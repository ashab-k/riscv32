// macros for extraction 

#pragma once
#include <stdint.h>

#define OPCODE(insn)   (((insn) >> 2) & 0x1F)
#define FUNCT3(insn)   (((insn) >> 12) & 0x7)
#define FUNCT7(insn)   (((insn) >> 25) & 0x7F)
#define RD(insn)       (((insn) >> 7)  & 0x1F)
#define RS1(insn)      (((insn) >> 15) & 0x1F)
#define RS2(insn)      (((insn) >> 20) & 0x1F)

// sign extend val from bits width to 32
#define SEXT(val, bits) \
    ((int32_t)(((uint32_t)(val)) << (32 - (bits))) >> (32 - (bits)))

// immediate extraction
#define IMM_I(insn)    SEXT((insn) >> 20, 12)

#define IMM_S(insn)    SEXT((((insn) >> 7) & 0x1F) | (((insn) >> 25) << 5), 12)

#define IMM_B(insn)    SEXT((((insn) >> 8) & 0xF) | ((((insn) >> 25) & 0x3F) << 4) | ((((insn) >> 7) & 0x1) << 10) | ((((insn) >> 31) & 0x1) << 11), 12) << 1

#define IMM_U(insn)    ((insn) & 0xFFFFF000)

#define IMM_J(insn) (SEXT((((insn)>>21)&0x3FF) | ((((insn)>>20)&0x1)<<10) | ((((insn)>>12)&0xFF)<<11) | ((((insn)>>31)&0x1)<<19), 20) << 1)
