#ifndef TIME_H
#define TIME_H 1

#include <stdint.h>

#include <interrupts/isr.h>

uint64_t get_ticks();
void set_ticks(uint64_t new);

void pit_tick(void *ctx);
void sched_timer_tick(void *ctx);

#endif