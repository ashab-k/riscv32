#pragma once 

#include <stdio.h>
#include <stdint.h>

#define XLEN 32
#define MEMSIZE 0x10000

#define DEBUG_PRINT 0
#define SINGLE_STEP 0

#define DEBUG(x) do { \
    if (DEBUG_PRINT) { \
        (x); \
        if (SINGLE_STEP) getchar(); \
    } \
} while (0)


typedef struct RISCV  {
  uint8_t mem[MEMSIZE];
  uint32_t mem_start_addr;
  
  uint32_t regs[32];
  uint32_t pc;
  uint32_t next_pc;

  uint32_t csrs[4096];

  int is_running;
} RISCV;


void riscv_init(RISCV* cpu, const char* program_filename, uint32_t mem_start, uint32_t entrypoint);

void riscv_run(RISCV* cpu);

void riscv_interpret_insn(RISCV* cpu, uint32_t insn);
void riscv_print_context(RISCV* cpu);

int riscv_read_u8(RISCV* cpu, uint32_t addr, uint8_t* out);
int riscv_read_u16(RISCV* cpu, uint32_t addr, uint16_t* out);
int riscv_read_u32(RISCV* cpu, uint32_t addr, uint32_t* out);

int riscv_write_u8(RISCV* cpu, uint32_t addr, uint8_t in);
int riscv_write_u16(RISCV* cpu, uint32_t addr, uint16_t in);
int riscv_write_u32(RISCV* cpu, uint32_t addr, uint32_t in);
