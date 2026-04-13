#include "riscv.h"
#include "dispatch.h"
#include "rv32i.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: %s <program.bin> <mem_start> <entrypoint>\n", argv[0]);
        fprintf(stderr, "  example: %s program.bin 0x80000000 0x80000000\n", argv[0]);
        return 1;
    }

    uint32_t mem_start   = (uint32_t)strtoul(argv[2], NULL, 16);
    uint32_t entrypoint  = (uint32_t)strtoul(argv[3], NULL, 16);

    dispatch_init();
    rv32i_init();

    RISCV cpu;
    riscv_init(&cpu, argv[1], mem_start, entrypoint);
    riscv_run(&cpu);

    return 0;
}
