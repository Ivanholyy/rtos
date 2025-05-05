#include "lapic.h"

#include <cpu.h>

#include <stdio.h>

#include <interrupts/irq.h>
#include <interrupts/isr.h>
#include <pic/pic.h>

#include <pit/pit.h>
#include <time.h>
#include <tsc/tsc.h>

#include <scheduler/scheduler.h>

#include <memory/pmm/pmm.h>
#include <memory/vmm/vmm.h>

uint32_t lapic_timer_ticks_per_ms = 0;

bool lapic_status = false;

uint64_t lapic_base = 0;
bool is_lapic_enabled() {
    return lapic_status;
}

void set_lapic_base(uint64_t base) {
    lapic_base = base;
}

void lapic_write_reg(uint64_t reg, uint32_t value) {
    if (!lapic_base)
        return;

    cpu_reg_write((uint32_t *)(lapic_base + reg), value);
}

uint64_t lapic_read_reg(uint64_t reg) {
    if (!lapic_base)
        return 0;

    return cpu_reg_read((uint32_t *)(lapic_base + reg));
}

uint64_t lapic_get_id() {
    return lapic_read_reg(LAPIC_ID_REG);
}

void lapic_send_eoi() {
    lapic_write_reg(LAPIC_EOI_REG, 0);
}

void lapic_init() {
    // read LAPIC MSR to get its base address
    uint64_t lapic_msr_phys = (_cpu_get_msr(LAPIC_BASE_MSR) & LAPIC_BASE_MASK);
    if (!lapic_msr_phys) {
        debugf_warn("No LAPIC base was found! Quitting\n");
        return;
    }
    debugf_debug("LAPIC base: %#llx\n", lapic_msr_phys);

    lapic_base = PHYS_TO_VIRTUAL(lapic_msr_phys + HHDM_OFFSET);
    map_region_to_page((uint64_t *)PHYS_TO_VIRTUAL(_get_pml4()), lapic_msr_phys,
                       lapic_base, 0x1000, PMLE_KERNEL_READ_WRITE);

    pic_disable();
    // TODO: remap all PIC IRQs

    lapic_write_reg(LAPIC_TASKPR_REG, 0);
    lapic_write_reg(LAPIC_DEST_FMT_REG, 0xFFFFFFFF);
    debugf_debug("LAPIC is globally enabled and MSR is now %#llx\n",
                 _cpu_get_msr(0x1b));
    lapic_write_reg(LAPIC_SPURIOUS_REG, 0x1ff);
    debugf_debug("LAPIC_SPURIOUS_REG: %#lx\n",
                 lapic_read_reg(LAPIC_SPURIOUS_REG));

    lapic_write_reg(LAPIC_LINT0_REG, 0xfe);
    lapic_write_reg(LAPIC_LINT1_REG, 0xfe);
    lapic_write_reg(LAPIC_CMCI_REG, 0xfe);
    lapic_write_reg(LAPIC_ERR_REG, 0xfe);
    lapic_write_reg(LAPIC_PERFMON_COUNT_REG, 0xfe);
    lapic_write_reg(LAPIC_THERM_REG, 0xfe);
    lapic_write_reg(LAPIC_TIMER_REG, 0xfe);

    debugf_debug("LAPIC ID: %#hhx\n", lapic_get_id());

    lapic_status = true;
}

uint32_t lapic_timer_calibrate_pit(void) {
    // Set the divide value register
    lapic_write_reg(LAPIC_TIMER_DIV_REG, 0x3); // Divide by 16

    // Set initial counter to max value
    lapic_write_reg(LAPIC_TIMER_INIT_CNT, 0xFFFFFFFF);

    // Use PIT for calibration
    uint64_t start_tick = get_ticks();

    // Wait for a measurable time (10ms should be sufficient)
    pit_sleep(10);

    // Stop the timer by setting a mask bit
    lapic_write_reg(LAPIC_TIMER_REG, LAPIC_DISABLE);

    // Get elapsed time
    uint64_t end_tick      = get_ticks();
    uint64_t elapsed_ticks = end_tick - start_tick;

    // Read how many LAPIC timer ticks occurred
    uint32_t lapic_ticks = 0xFFFFFFFF - lapic_read_reg(LAPIC_TIMER_CURR_CNT);

    // Calculate ticks per ms
    uint32_t ticks_per_ms = lapic_ticks / elapsed_ticks;

    // Store this value for future use (can be static/global)
    return ticks_per_ms;
}

uint32_t calibrate_apic_timer_tsc(void) {

    lapic_write_reg(LAPIC_TIMER_INIT_CNT, 0xFFFFFFFF);

    tsc_sleep(100000);

    uint32_t end_count = lapic_read_reg(LAPIC_TIMER_CURR_CNT);

    uint32_t elapsed_lapic_ticks = 0xFFFFFFFF - end_count;

    uint64_t lapic_timer_frequency = (uint64_t)(elapsed_lapic_ticks * 10);

    uint32_t ticks_per_ms = lapic_timer_frequency / 1000;

    return ticks_per_ms;
}

void lapic_timer_init(void) {
    // Calibrate the timer
    if (check_tsc())
        lapic_timer_ticks_per_ms = calibrate_apic_timer_tsc();
    else
        lapic_timer_ticks_per_ms = lapic_timer_calibrate_pit();

    // Register the timer interrupt handler
    isr_registerHandler(LAPIC_IRQ_OFFSET + LAPIC_TIMER_VECTOR,
                        lapic_timer_handler);

    // Configure the timer in periodic mode
    // Set a reasonable value for your system (e.g., 10ms intervals)
    uint32_t count = lapic_timer_ticks_per_ms * 10; // 10ms intervals

    // Set the initial count
    lapic_write_reg(LAPIC_TIMER_INIT_CNT, count);

    // Set up the timer register with:
    // - The timer vector
    // - Periodic mode (bit 17)
    // - Not masked (bit 16 = 0)
    lapic_write_reg(LAPIC_TIMER_REG,
                    LAPIC_TIMER_VECTOR + LAPIC_IRQ_OFFSET | (1 << 17));

    debugf_debug("LAPIC Timer initialized: %u ticks per ms\n",
                 lapic_timer_ticks_per_ms);
}

void lapic_timer_handler(void *ctx) {

    if (get_ticks() >= MAX_LAPIC_TICKS)
        set_ticks(0);

    set_ticks(get_ticks() + 1);

    scheduler_schedule(ctx);

    lapic_send_eoi();
}
