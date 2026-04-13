#include "riscv.h"
#include "dispatch.h"
#include <stdio.h>
#include <string.h>

void riscv_init(RISCV *cpu, const char *program_filename, uint32_t mem_start, uint32_t entrypoint) {
    memset(cpu, 0, sizeof(RISCV));

    FILE *f = fopen(program_filename, "rb");
    if (!f) {
        fprintf(stderr, "failed to open program file: %s\n", program_filename);
        return;
    }
    fread(cpu->mem, 1, MEMSIZE, f);
    fclose(f);

    cpu->mem_start_addr = mem_start;
    cpu->pc = entrypoint;
}

void riscv_run(RISCV *cpu) {
    cpu->is_running = 1;
    cpu->regs[0] = 0;
    cpu->regs[2] = cpu->mem_start_addr + MEMSIZE - 4; // stack pointer

    while (cpu->is_running) {
        cpu->regs[0] = 0; // x0 is hardwired zero, enforce each cycle
        cpu->next_pc = cpu->pc + 4;

        uint32_t insn;
        if (riscv_read_u32(cpu, cpu->pc, &insn)) {
            fprintf(stderr, "failed to fetch instruction at pc=0x%x\n", cpu->pc);
            break;
        }

        if (insn == 0) {
            fprintf(stderr, "null instruction at pc=0x%x\n", cpu->pc);
            break;
        }

        if (DEBUG_PRINT)
            riscv_print_context(cpu);

        riscv_interpret_insn(cpu, insn);

        cpu->pc = cpu->next_pc;
    }
}

void riscv_interpret_insn(RISCV *cpu, uint32_t insn) {
    // bits [1:0] must be 11 for a 32-bit instruction
    // if not, it's a C extension compressed instruction
    if ((insn & 0x3) != 0x3) {
        fprintf(stderr, "compressed instruction not supported: 0x%08x at pc=0x%x\n", insn, cpu->pc);
        cpu->is_running = 0;
        return;
    }
    dispatch_insn(cpu, insn);
}

void riscv_print_context(RISCV *cpu) {
    for (int i = 0; i < 32; i++)
        printf("| x%d: %u(%d) ", i, cpu->regs[i], (int32_t)cpu->regs[i]);
    printf("| pc: 0x%x\n", cpu->pc);
}


int riscv_read_u8(RISCV *cpu, uint32_t addr, uint8_t *out) {
    uint32_t offset = addr - cpu->mem_start_addr;
    if (offset >= MEMSIZE) {
        fprintf(stderr, "illegal read8 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        *out = 0;
        return 1;
    }
    *out = cpu->mem[offset];
    return 0;
}

int riscv_read_u16(RISCV *cpu, uint32_t addr, uint16_t *out) {
    if (addr & 1) {
        fprintf(stderr, "misaligned read16 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    uint32_t offset = addr - cpu->mem_start_addr;
    if (offset >= MEMSIZE - 1) {
        fprintf(stderr, "illegal read16 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        *out = 0;
        return 1;
    }
    *out = (uint16_t)(cpu->mem[offset] | (cpu->mem[offset + 1] << 8));
    return 0;
}

int riscv_read_u32(RISCV *cpu, uint32_t addr, uint32_t *out) {
    if (addr & 3) {
        fprintf(stderr, "misaligned read32 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    uint32_t offset = addr - cpu->mem_start_addr;
    if (offset >= MEMSIZE - 3) {
        fprintf(stderr, "illegal read32 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        *out = 0;
        return 1;
    }
    //little endian assembly
    *out = (uint32_t)(cpu->mem[offset]
        | (cpu->mem[offset + 1] << 8)
        | (cpu->mem[offset + 2] << 16)
        | (cpu->mem[offset + 3] << 24));
    return 0;
}

int riscv_write_u8(RISCV *cpu, uint32_t addr, uint8_t in) {
    uint32_t offset = addr - cpu->mem_start_addr;
    if (offset >= MEMSIZE) {
        fprintf(stderr, "illegal write8 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    cpu->mem[offset] = in;
    return 0;
}

int riscv_write_u16(RISCV *cpu, uint32_t addr, uint16_t in) {
    if (addr & 1) {
        fprintf(stderr, "misaligned write16 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    uint32_t offset = addr - cpu->mem_start_addr;
    if (offset >= MEMSIZE - 1) {
        fprintf(stderr, "illegal write16 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    cpu->mem[offset]     = in & 0xFF;
    cpu->mem[offset + 1] = (in >> 8) & 0xFF;
    return 0;
}

int riscv_write_u32(RISCV *cpu, uint32_t addr, uint32_t in) {
    if (addr & 3) {
        fprintf(stderr, "misaligned write32 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    uint32_t offset = addr - cpu->mem_start_addr;
    if (offset >= MEMSIZE - 3) {
        fprintf(stderr, "illegal write32 addr=0x%x pc=0x%x\n", addr, cpu->pc);
        return 1;
    }
    cpu->mem[offset]     = in & 0xFF;
    cpu->mem[offset + 1] = (in >> 8) & 0xFF;
    cpu->mem[offset + 2] = (in >> 16) & 0xFF;
    cpu->mem[offset + 3] = (in >> 24) & 0xFF;
    return 0;
}
