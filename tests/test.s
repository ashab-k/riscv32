# test.S
.section .text
.global _start
_start:
    li a0, 5          # a0 = 5
    li a1, 3          # a1 = 3
    add a0, a0, a1    # a0 = 8  (also exit code)
    li a7, 93         # exit syscall
    ecall
