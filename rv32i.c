#include "decode.h"
#include "dispatch.h"
#include "riscv.h"
#include <stdint.h>
#include <stdio.h>

#define OP_IMM   0x04   // 0x13 >> 2  = 0x04
#define OP       0x0C   // 0x33
#define OP_LUI   0x0D   // 0x37
#define OP_AUIPC 0x05   // 0x17
#define OP_JAL   0x1B   // 0x6F
#define OP_JALR  0x19   // 0x67
#define OP_BR    0x18   // 0x63
#define OP_LOAD  0x00   // 0x03
#define OP_STORE 0x08   // 0x23
#define OP_FENCE 0x03   // 0x0F
#define OP_SYS   0x1C   // 0x73

// op_imm insn

//add immediate 
static void insn_addi(RISCV *cpu, uint32_t insn){
 uint32_t rd = RD(insn), rs1 = RS1(insn);
  int32_t imm = IMM_I(insn);
  DEBUG(printf("ADDI x%d, x%d, %d\n", rd, rs1, imm));

  if (rd)  cpu->regs[rd] = (int32_t)cpu->regs[rs1] + imm;
}
  

// set less than immediate
static void insn_slti(RISCV *cpu, uint32_t insn){
  uint32_t rd = RD(insn), rs1 = RS1(insn);
  int32_t imm = IMM_I(insn);
  DEBUG(printf("SLTI x%d, x%d, %d\n", rd, rs1, imm));
  if (rd) cpu->regs[rd] = (int32_t)cpu->regs[rs1] < imm;
}

//set less than immediate unsigned
static void insn_sltiu(RISCV *cpu, uint32_t insn){
  uint32_t rd = RD(insn), rs1 = RS1(insn);
  uint32_t imm = (uint32_t)IMM_I(insn);
  DEBUG(printf("SLTIU x%d, x%d, %d\n", rd, rs1, imm));
  if (rd) cpu->regs[rd] = (int32_t)cpu->regs[rs1] < imm;
}

// XOR immediate
static void insn_xori(RISCV *cpu, uint32_t insn) {
  uint32_t rd = RD(insn), rs1 = RS1(insn);
  int32_t imm = IMM_I(insn);
  DEBUG(printf("XORI x%d, x%d, %d\n", rd, rs1, imm));
  if (rd) cpu->regs[rd] = cpu->regs[rs1] ^ (uint32_t)imm;
}

// OR immediate
static void insn_ori(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t imm = IMM_I(insn);
    DEBUG(printf("ORI x%d, x%d, %d\n", rd, rs1, imm));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] | (uint32_t)imm;
}

//AND immediate
static void insn_andi(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t imm = IMM_I(insn);
    DEBUG(printf("ANDI x%d, x%d, %d\n", rd, rs1, imm));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] & (uint32_t)imm;
}

//shift left logical immediate
static void insn_slli(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    // shamt is imm[4:0] (bits 24:20)
    // The upper bits of the immediate (funct7) must be 0x00
    uint32_t shamt = (insn >> 20) & 0x1F; 
    uint32_t funct7 = FUNCT7(insn); 
    if (funct7 != 0x00) {
        DEBUG(printf("SLLI x%d, x%d, %d\n", rd, rs1, shamt));
        cpu->is_running = 0; 
        return;
    }
    DEBUG(printf("SLLI x%d, x%d, %u\n", rd, rs1, shamt));

    if (rd != 0) {
        cpu->regs[rd] = cpu->regs[rs1] << shamt;
    }
}

//shift right logical/arithmetic immediate
static void insn_srli_srai(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    uint32_t shamt = RS2(insn);
    uint32_t shtyp = FUNCT7(insn);
    if (shtyp == 0x20) {
        DEBUG(printf("SRAI x%d, x%d, %d\n", rd, rs1, shamt));
        if (rd) cpu->regs[rd] = (int32_t)cpu->regs[rs1] >> shamt;
    } else if (shtyp == 0x00) {
        DEBUG(printf("SRLI x%d, x%d, %d\n", rd, rs1, shamt));
        if (rd) cpu->regs[rd] = cpu->regs[rs1] >> shamt;
    } else {
        fprintf(stderr, "illegal SRLI/SRAI funct7=0x%x\n", shtyp);
        cpu->is_running = 0;
    }
}

static void handle_op_imm(RISCV *cpu, uint32_t insn) {
    dispatch_secondary(cpu, insn);
}


// op insn

static void insn_add_sub(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    uint32_t funct7 = FUNCT7(insn);
    if (funct7 == 0x20) {
        if (rd) cpu->regs[rd] = (int32_t)(cpu->regs[rs1] - cpu->regs[rs2]);
    } else {
        if (rd) cpu->regs[rd] = (int32_t)(cpu->regs[rs1] + cpu->regs[rs2]);
    }
}

 
static void insn_sll(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    DEBUG(printf("SLL x%d, x%d, x%d\n", rd, rs1, rs2));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] << (cpu->regs[rs2] & 0x1F);
}
 
static void insn_slt(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    DEBUG(printf("SLT x%d, x%d, x%d\n", rd, rs1, rs2));
    if (rd) cpu->regs[rd] = (int32_t)cpu->regs[rs1] < (int32_t)cpu->regs[rs2];
}
 
static void insn_sltu(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    DEBUG(printf("SLTU x%d, x%d, x%d\n", rd, rs1, rs2));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] < cpu->regs[rs2];
}
 
static void insn_xor(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    DEBUG(printf("XOR x%d, x%d, x%d\n", rd, rs1, rs2));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] ^ cpu->regs[rs2];
}
 
static void insn_srl_sra(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    uint32_t shamt = cpu->regs[rs2] & 0x1F;
    if (FUNCT7(insn) == 0x20) {
        DEBUG(printf("SRA x%d, x%d, x%d\n", rd, rs1, rs2));
        if (rd) cpu->regs[rd] = (int32_t)cpu->regs[rs1] >> shamt;
    } else {
        DEBUG(printf("SRL x%d, x%d, x%d\n", rd, rs1, rs2));
        if (rd) cpu->regs[rd] = cpu->regs[rs1] >> shamt;
    }
}
 
static void insn_or(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    DEBUG(printf("OR x%d, x%d, x%d\n", rd, rs1, rs2));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] | cpu->regs[rs2];
}
 
static void insn_and(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn), rs2 = RS2(insn);
    DEBUG(printf("AND x%d, x%d, x%d\n", rd, rs1, rs2));
    if (rd) cpu->regs[rd] = cpu->regs[rs1] & cpu->regs[rs2];
}

static void handle_op(RISCV *cpu, uint32_t insn) {
    dispatch_secondary(cpu, insn);
}


//load unsigned immediate
static void handle_lui(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn);
    uint32_t imm = IMM_U(insn);
    DEBUG(printf("LUI x%d, 0x%x\n", rd, imm));
    if (rd) cpu->regs[rd] = imm;
}
 
static void handle_auipc(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn);
    uint32_t imm = IMM_U(insn);
    DEBUG(printf("AUIPC x%d, 0x%x\n", rd, imm));
    if (rd) cpu->regs[rd] = cpu->pc + imm;
}

//jump and link
static void handle_jal(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn);
    int32_t offset = IMM_J(insn);
    uint32_t addr = cpu->pc + (uint32_t)offset;
    DEBUG(printf("JAL x%d, pc%+d\n", rd, offset));
    if (addr & 3) {
        fprintf(stderr, "misaligned JAL target 0x%x\n", addr);
        cpu->is_running = 0; return;
    }
    if (rd) cpu->regs[rd] = cpu->next_pc;
    cpu->next_pc = addr;
}


//jump and link register 
static void handle_jalr(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t offset = IMM_I(insn);
    if (FUNCT3(insn) != 0x0) {
        fprintf(stderr, "illegal JALR funct3=0x%x\n", FUNCT3(insn));
        cpu->is_running = 0; return;
    }
    uint32_t addr = ((uint32_t)((int32_t)cpu->regs[rs1] + offset)) & ~1u;
    DEBUG(printf("JALR x%d, x%d%+d\n", rd, rs1, offset));
    if (addr & 3) {
        fprintf(stderr, "misaligned JALR target 0x%x\n", addr);
        cpu->is_running = 0; return;
    }
    if (rd) cpu->regs[rd] = cpu->next_pc;
    cpu->next_pc = addr;
}


//branch instructions 

//branch if equal
static void insn_beq(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_B(insn);
    DEBUG(printf("BEQ x%d, x%d, pc%+d\n", rs1, rs2, offset));
    if (cpu->regs[rs1] == cpu->regs[rs2]) cpu->next_pc = cpu->pc + offset;
}
 
//branch if not equal
static void insn_bne(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_B(insn);
    DEBUG(printf("BNE x%d, x%d, pc%+d\n", rs1, rs2, offset));
    if (cpu->regs[rs1] != cpu->regs[rs2]) cpu->next_pc = cpu->pc + offset;
}

//branch if less than 
static void insn_blt(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_B(insn);
    DEBUG(printf("BLT x%d, x%d, pc%+d\n", rs1, rs2, offset));
    if ((int32_t)cpu->regs[rs1] < (int32_t)cpu->regs[rs2]) cpu->next_pc = cpu->pc + offset;
}
 
//branch if greaterthan or equal
static void insn_bge(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_B(insn);
    DEBUG(printf("BGE x%d, x%d, pc%+d\n", rs1, rs2, offset));
    if ((int32_t)cpu->regs[rs1] >= (int32_t)cpu->regs[rs2]) cpu->next_pc = cpu->pc + offset;
}
 
//branch if less than unsigned
static void insn_bltu(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_B(insn);
    DEBUG(printf("BLTU x%d, x%d, pc%+d\n", rs1, rs2, offset));
    if (cpu->regs[rs1] < cpu->regs[rs2]) cpu->next_pc = cpu->pc + offset;
}
 
// branch if greaterthan or equal unsigned
static void insn_bgeu(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_B(insn);
    DEBUG(printf("BGEU x%d, x%d, pc%+d\n", rs1, rs2, offset));
    if (cpu->regs[rs1] >= cpu->regs[rs2]) cpu->next_pc = cpu->pc + offset;
}


static void handle_branch(RISCV *cpu, uint32_t insn) {
    dispatch_secondary(cpu, insn);
}

//loads
//load byte
static void insn_lb(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t offset = IMM_I(insn);
    uint8_t val;
    DEBUG(printf("LB x%d, x%d%+d\n", rd, rs1, offset));
    if (riscv_read_u8(cpu, cpu->regs[rs1] + offset, &val)) return;
    if (rd) cpu->regs[rd] = (int32_t)(int8_t)val;
}
 
//load halfword 
static void insn_lh(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t offset = IMM_I(insn);
    uint16_t val;
    DEBUG(printf("LH x%d, x%d%+d\n", rd, rs1, offset));
    if (riscv_read_u16(cpu, cpu->regs[rs1] + offset, &val)) return;
    if (rd) cpu->regs[rd] = (int32_t)(int16_t)val;
}
 
//load word
static void insn_lw(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t offset = IMM_I(insn);
    uint32_t val;
    DEBUG(printf("LW x%d, x%d%+d\n", rd, rs1, offset));
    if (riscv_read_u32(cpu, cpu->regs[rs1] + offset, &val)) return;
    if (rd) cpu->regs[rd] = val;
}

//load byte unsigned  
static void insn_lbu(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t offset = IMM_I(insn);
    uint8_t val;
    DEBUG(printf("LBU x%d, x%d%+d\n", rd, rs1, offset));
    if (riscv_read_u8(cpu, cpu->regs[rs1] + offset, &val)) return;
    if (rd) cpu->regs[rd] = (uint32_t)val;
}

//load halfword unsigned 
static void insn_lhu(RISCV *cpu, uint32_t insn) {
    uint32_t rd = RD(insn), rs1 = RS1(insn);
    int32_t offset = IMM_I(insn);
    uint16_t val;
    DEBUG(printf("LHU x%d, x%d%+d\n", rd, rs1, offset));
    if (riscv_read_u16(cpu, cpu->regs[rs1] + offset, &val)) return;
    if (rd) cpu->regs[rd] = (uint32_t)val;
}
 
static void handle_load(RISCV *cpu, uint32_t insn) {
    dispatch_secondary(cpu, insn);
} 


//stores

//store byte
static void insn_sb(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_S(insn);
    DEBUG(printf("SB x%d, x%d%+d\n", rs2, rs1, offset));
    riscv_write_u8(cpu, cpu->regs[rs1] + offset, cpu->regs[rs2] & 0xFF);
}
 
//store halfword
static void insn_sh(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_S(insn);
    DEBUG(printf("SH x%d, x%d%+d\n", rs2, rs1, offset));
    riscv_write_u16(cpu, cpu->regs[rs1] + offset, cpu->regs[rs2] & 0xFFFF);
}

//store word 
static void insn_sw(RISCV *cpu, uint32_t insn) {
    uint32_t rs1 = RS1(insn), rs2 = RS2(insn);
    int32_t offset = IMM_S(insn);
    DEBUG(printf("SW x%d, x%d%+d\n", rs2, rs1, offset));
    riscv_write_u32(cpu, cpu->regs[rs1] + offset, cpu->regs[rs2]);
}
 
static void handle_store(RISCV *cpu, uint32_t insn) {
    dispatch_secondary(cpu, insn);
}


//fence
static void handle_fence(RISCV *cpu, uint32_t insn) {
    (void)cpu; (void)insn;
    DEBUG(printf("FENCE (nop)\n"));
}


//system insn

static void handle_ecall(RISCV *cpu, uint32_t insn) {
    (void)insn;
    uint32_t syscall_num = cpu->regs[17]; // a7
    uint32_t a0 = cpu->regs[10];
    uint32_t a1 = cpu->regs[11];
    uint32_t a2 = cpu->regs[12];

    switch (syscall_num) {
    case 64: { // write(fd, buf, len)
        for (uint32_t i = 0; i < a2; i++) {
            uint8_t byte;
            if (riscv_read_u8(cpu, a1 + i, &byte)) break;
            fputc(byte, a0 == 2 ? stderr : stdout);
        }
        cpu->regs[10] = a2; // return value = bytes written
        break;
    }
    case 93:  // exit(code)
        printf("program exited with code %d\n", a0);
        cpu->is_running = 0;
        break;
    case 214: // brk(addr)
        cpu->regs[10] = a0;
        break;
    default:
        fprintf(stderr, "unhandled syscall %d at pc=0x%x\n", syscall_num, cpu->pc);
        cpu->regs[10] = -1;
        break;
    }
}

static void handle_ebreak(RISCV *cpu, uint32_t insn) {
    (void)insn;
    DEBUG(printf("EBREAK\n"));
    cpu->is_running = 0;
}
 
static void handle_priv(RISCV *cpu, uint32_t insn) {
    if (RD(insn) != 0 || RS1(insn) != 0) {
        fprintf(stderr, "illegal SYSTEM insn at pc=0x%x\n", cpu->pc);
        cpu->is_running = 0; return;
    }
    uint32_t funct12 = (insn >> 20) & 0xFFF;
    switch (funct12) {
    case 0x000: handle_ecall(cpu, insn);  break;
    case 0x001: handle_ebreak(cpu, insn); break;
    default:
        fprintf(stderr, "unknown SYSTEM funct12=0x%x at pc=0x%x\n", funct12, cpu->pc);
        cpu->is_running = 0;
    }
}
 
static void handle_system(RISCV *cpu, uint32_t insn) {
    if (FUNCT3(insn) == 0x0)
        handle_priv(cpu, insn);
    else
        dispatch_secondary(cpu, insn); // CSR slots filled by zicsr_init()
}

void rv32i_init(void) {
    // primary table
    register_primary(0x04, handle_op_imm); // 0x13 OP-IMM
    register_primary(0x0C, handle_op);     // 0x33 OP
    register_primary(0x0D, handle_lui);    // 0x37 LUI
    register_primary(0x05, handle_auipc);  // 0x17 AUIPC
    register_primary(0x1B, handle_jal);    // 0x6F JAL
    register_primary(0x19, handle_jalr);   // 0x67 JALR
    register_primary(0x18, handle_branch); // 0x63 BRANCH
    register_primary(0x00, handle_load);   // 0x03 LOAD
    register_primary(0x08, handle_store);  // 0x23 STORE
    register_primary(0x03, handle_fence);  // 0x0F FENCE
    register_primary(0x1C, handle_system); // 0x73 SYSTEM
 
    // OP-IMM funct3
    register_secondary(0x04, 0x0, insn_addi);
    register_secondary(0x04, 0x2, insn_slti);
    register_secondary(0x04, 0x3, insn_sltiu);
    register_secondary(0x04, 0x4, insn_xori);
    register_secondary(0x04, 0x6, insn_ori);
    register_secondary(0x04, 0x7, insn_andi);
    register_secondary(0x04, 0x1, insn_slli);
    register_secondary(0x04, 0x5, insn_srli_srai);
 
    // OP funct3
    register_secondary(0x0C, 0x0, insn_add_sub);
    register_secondary(0x0C, 0x1, insn_sll);
    register_secondary(0x0C, 0x2, insn_slt);
    register_secondary(0x0C, 0x3, insn_sltu);
    register_secondary(0x0C, 0x4, insn_xor);
    register_secondary(0x0C, 0x5, insn_srl_sra);
    register_secondary(0x0C, 0x6, insn_or);
    register_secondary(0x0C, 0x7, insn_and);
 
    // BRANCH funct3
    register_secondary(0x18, 0x0, insn_beq);
    register_secondary(0x18, 0x1, insn_bne);
    register_secondary(0x18, 0x4, insn_blt);
    register_secondary(0x18, 0x5, insn_bge);
    register_secondary(0x18, 0x6, insn_bltu);
    register_secondary(0x18, 0x7, insn_bgeu);
 
    // LOAD funct3
    register_secondary(0x00, 0x0, insn_lb);
    register_secondary(0x00, 0x1, insn_lh);
    register_secondary(0x00, 0x2, insn_lw);
    register_secondary(0x00, 0x4, insn_lbu);
    register_secondary(0x00, 0x5, insn_lhu);
 
    // STORE funct3
    register_secondary(0x08, 0x0, insn_sb);
    register_secondary(0x08, 0x1, insn_sh);
    register_secondary(0x08, 0x2, insn_sw);
}
