#include "dispatch.h"
#include "decode.h"
#include <stdio.h>

static insn_handler primary_table[32];
static insn_handler secondary_table[32][8];

void dispatch_init(void) {
    for (int i = 0; i < 32; i++) {
        primary_table[i] = NULL;
        for (int j = 0; j < 8; j++)
            secondary_table[i][j] = NULL;
    }
}

void register_primary(uint32_t opcode, insn_handler fn) {
    primary_table[opcode] = fn;
}

void register_secondary(uint32_t opcode, uint32_t funct3, insn_handler fn) {
    secondary_table[opcode][funct3] = fn;
}

void dispatch_insn(RISCV *cpu, uint32_t insn) {
    insn_handler fn = primary_table[OPCODE(insn)];
    if (!fn) {
        fprintf(stderr, "illegal instruction: 0x%08x at pc 0x%08x\n", insn, cpu->pc);
        cpu->is_running = 0;
        return;
    }
    fn(cpu, insn);
}

void dispatch_secondary(RISCV *cpu, uint32_t insn) {
    insn_handler fn = secondary_table[OPCODE(insn)][FUNCT3(insn)];
    if (!fn) {
        fprintf(stderr, "illegal instruction: 0x%08x at pc 0x%08x\n", insn, cpu->pc);
        cpu->is_running = 0;
        return;
    }
    fn(cpu, insn);
}
