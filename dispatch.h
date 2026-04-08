#pragma once
#include "riscv.h"

typedef void (*insn_handler)(RISCV *cpu, uint32_t insn);

void dispatch_init(void);
void dispatch_insn(RISCV *cpu, uint32_t insn);

// register a top-level handler for an opcode 
void register_primary(uint32_t opcode, insn_handler fn);

// register a funct3-level handler under an opcode
void register_secondary(uint32_t opcode, uint32_t funct3, insn_handler fn);

void dispatch_secondary(RISCV *cpu, uint32_t insn);
