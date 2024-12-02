#ifndef ISR_H
#define ISR_H 1

#include <stdint.h>

typedef struct {
    // registers in the reverse order they are pushed:

    uint64_t ds;

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;

    uint64_t rbp;

    uint64_t rdi;
    uint64_t rsi;

    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t interrupt;
    uint64_t error;

    uint64_t rip;
    uint64_t cs;
    uint64_t eflags;
    uint64_t rsp;
    uint64_t ss;

} __attribute__((packed)) registers;

typedef void (*isrHandler)(registers* regs);

void print_reg_dump(registers* regs);

void isr_init();
void isr_registerHandler(int interrupt, isrHandler handler);

extern void _panic();

#endif