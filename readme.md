# RISCV Emulator
 
The sole purpose of this project is to learn about the various instructions in the rv32i ISA by implementing them.

An attempt has been made to structure this in a way where in the future if I do decide to implement extensions, it won't be too much of a hassle.

I made the decision to use a function pointer table to dispatch the various instructions instead of using a giant switch statement, although that may be an overkill.
 
## Building
 
```bash
gcc -o emulator main.c riscv.c dispatch.c rv32i.c -I.
```
 
## Running
 
Expects a flat raw binary. To produce one from an assembly file:
 
```bash
riscv64-unknown-elf-as -march=rv32i -mabi=ilp32 program.s -o program.o
riscv64-unknown-elf-ld -m elf32lriscv -Ttext=0x80000000 program.o -o program.elf
riscv64-unknown-elf-objcopy -O binary program.elf program.bin
```
 
Then run:
 
```bash
./emulator program.bin 80000000 80000000
```
 
The two addresses are `mem_start` and `entrypoint`. Both are `0x80000000` by convention, this is the standard RISC-V RAM base address used by QEMU and most RISC-V boards.
 
## Project Structure
 
```
riscv.h      — CPU struct, memory size, debug macros
riscv.c      — run loop, memory read/write, binary loader
decode.h     — bit extraction macros (OPCODE, FUNCT3, RD, IMM_* etc)
dispatch.h   — function pointer table API
dispatch.c   — primary and secondary dispatch table implementation
rv32i.c      — RV32I instruction handlers and registration
main.c       — entry point, wires everything together
```
