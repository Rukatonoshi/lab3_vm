// src/instructions.c
#include "instructions.h"
#include <stddef.h>

// Instruction table
Instruction* instructions[256];

// Instruction table initialization
static void init_instructions_table(void) {
    for (int i = 0; i < 256; i++) {
        instructions[i] = NULL;
    }
    for (int i = 0; i < sizeof(init_table) / sizeof(init_table[0]); i++) {
        int op = init_table[i].opcode;
        if (op >= 0 && op < 256) {
            instructions[op] = (Instruction*)&init_table[i];
        }
    }
}

// Initialize instructions
__attribute__((constructor))
void init_instructions(void) {
    init_instructions_table();
}
