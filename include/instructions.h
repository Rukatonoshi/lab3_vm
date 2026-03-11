#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

// Instruction flags
#define INSTR_FLAG_JUMP   0x01
#define INSTR_FLAG_CJUMP  0x02
#define INSTR_FLAG_HALT   0x10

// Instruction structure
typedef struct {
    uint8_t opcode;
    const char* name;
    int arg_size;
    uint8_t flags;
} Instruction;

// DEFINITION
static const Instruction init_table[] = {
    {0x01, "binop_add", 1, 0},      // PLUS
    {0x02, "binop_sub", 1, 0},      // MINUS
    {0x03, "binop_mul", 1, 0},      // MULTIPLY
    {0x04, "binop_div", 1, 0},      // DIVIDE
    {0x05, "binop_rem", 1, 0},      // REMAINDER
    {0x06, "binop_lt", 1, 0},       // LESS
    {0x07, "binop_le", 1, 0},       // LESS_EQUAL
    {0x08, "binop_gt", 1, 0},       // GREATER
    {0x09, "binop_ge", 1, 0},       // GREATER_EQUAL
    {0x0A, "binop_eq", 1, 0},       // EQUAL
    {0x0B, "binop_ne", 1, 0},       // NOT_EQUAL
    {0x0C, "binop_and", 1, 0},      // AND
    {0x0D, "binop_or", 1, 0},       // OR
    {0x10, "const", 4, 0},
    {0x11, "xstring", 4, 0},
    {0x12, "sexp", 8, 0},
    {0x13, "sti", 4, 0},
    {0x14, "sta", 4, 0},
    {0x15, "jmp", 4, INSTR_FLAG_JUMP},
    {0x16, "end", 0, INSTR_FLAG_HALT},
    {0x17, "ret", 0, INSTR_FLAG_HALT},
    {0x18, "drop", 0, 0},
    {0x19, "dup", 0, 0},
    {0x1A, "swap", 0, 0},
    {0x1B, "elem", 0, 0},
    {0x20, "ld", 4, 0},
    {0x30, "lda", 4, 0},
    {0x40, "st", 4, 0},
    {0x50, "cjmp_z", 4, INSTR_FLAG_CJUMP},
    {0x51, "cjmp_nz", 4, INSTR_FLAG_CJUMP},
    {0x52, "begin", 8, 0},
    {0x53, "cbegin", 8, 0},
    {0x54, "closure", 8, 0},
    {0x55, "callc", 4, 0},
    {0x56, "call", 8, 0},
    {0x57, "tag", 8, 0},
    {0x58, "array", 4, 0},
    {0x59, "fail", 8, INSTR_FLAG_HALT},
    {0x5A, "line", 4, 0},
    {0x60, "patt", 1, 0},
    {0x70, "call_read", 0, 0},
    {0x71, "call_write", 0, 0},
    {0x72, "call_length", 0, 0},
    {0x73, "call_string", 0, 0},
    {0x74, "call_array", 4, 0}
};

extern Instruction* instructions[256];

void init_instructions(void);

#endif // INSTRUCTIONS_H
