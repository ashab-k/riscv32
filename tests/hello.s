# tests/hello.s
.section .text
.global _start
_start:
    li a7, 64
    li a0, 1
    la a1, msg
    li a2, 13
    ecall
    li a7, 93
    li a0, 0
    ecall

.section .data
msg: .ascii "Hello RISC-V\n"
