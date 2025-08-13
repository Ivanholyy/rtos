format ELF64
extrn isr_handler

section '.text' executable

isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rbp, ds
    push rbp

    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov ss, bx

    mov rdi, rsp
    call isr_handler

    pop rbp
    mov bx, bp
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov ss, bx

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

macro isr_err_stub num
{
  public isr_stub_#num
  isr_stub_#num:
    push num
    jmp isr_common
    iretq
}

macro isr_no_err_stub num
{
  public isr_stub_#num
  isr_stub_#num:
    push 0
    push num
    jmp isr_common
    iretq
}

rept 256 i:0 {
  if i = 8 | i = 10 | i = 11 | i = 12 | i = 13 | i = 14 | i = 17 | i = 21 | i = 29 | i = 30
    isr_err_stub i
  else
    isr_no_err_stub i
  end if
}

public isr_stub_table
isr_stub_table:
rept 256 i:0 {
    dq isr_stub_#i
}

public _hcf
_hcf:
    cli
    hlt
