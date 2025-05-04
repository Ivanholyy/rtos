#include "arch.h"

#include <stdio.h>

#include <gdt/gdt.h>
#include <idt/idt.h>
#include <interrupts/irq.h>
#include <interrupts/isr.h>

#include <pic/pic.h>
#include <pit/pit.h>
#include <tsc/tsc.h>

#include <paging/paging.h>

#include <time.h>

#include <cpu.h>

#define PIT_TICKS 1000 / 1 // 1 ms

bool pit;
bool tsc;

void arch_base_init() {
    pit = false;
    tsc = false;

    gdt_init();
    kprintf_ok("Initialized GDT\n");
    idt_init();
    kprintf_ok("Initialized IDT\n");
    isr_init();
    kprintf_ok("Initialized ISRs\n");
    isr_registerHandler(14, pf_handler);

    // _crash_test();

    irq_init();
    kprintf_ok("Initialized PIC and IRQs\n");

    if (check_tsc()) {
        tsc_init();
        tsc = true;
        kprintf_ok("Initialized TSC\n");
    } else {
        pit_init(PIT_TICKS);
        pit = true;
        kprintf_ok("Initialized PIT\n");
    }

    irq_registerHandler(0, pit_tick);
}

void sleep(unsigned long ms) {
    if (!(tsc || pit) || !ms) {
        return;
    }

    if (pit) {
        pit_sleep(ms);
    } else {
        tsc_sleep(ms);
    }
}

void mark_time(uint64_t *marker) {
    if (!(tsc || pit) || !marker) {
        return;
    }

    if (pit) {
        *marker = get_ticks();
    } else {
        *marker = _get_tsc();
    }
}

uint64_t get_ms(uint64_t start) {
    uint64_t now;

    if (!(tsc || pit) || !start) {
        return 0;
    }

    if (pit) {
        now = get_ticks();

        return (now - start) / 1000;
    } else {
        now = _get_tsc();

        return (now - start) / 1000000;
    }
}