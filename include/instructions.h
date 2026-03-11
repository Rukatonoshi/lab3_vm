#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

// Instruction flags
// INSTR_FLAG_JUMP: Instructions that transfer control to a new address in the file
// JMP (0x15), CALL (0x56), CJMPz (0x50), CJMPnz (0x51)
#define INSTR_FLAG_JUMP     0x01

// INSTR_FLAG_HALT: Instructions that terminate the current basic block (do not continue to 'next')
// JMP (0x15), END (0x16), RET (0x17), FAIL (0x59)
#define INSTR_FLAG_HALT     0x02

// INSTR_FLAG_BREAK: Instructions that break the sequence of idioms (but don't necessarily halt)
// Used for CALL (0x56), CALLC (0x55), and CALL_Builtins (0x70-0x74) to prevent them from forming pairs
// with the next instruction.
#define INSTR_FLAG_BREAK    0x04

// Instruction structure
typedef struct {
    uint8_t opcode;
    const char* name;
    int arg_size;
    uint8_t flags;
} Instruction;

// DEFINITION
static const Instruction init_table[] = {
    // --- BINOP (0x01 - 0x0D) ---
    {0x01, "binop_add", 0, 0}, // PLUS
    {0x02, "binop_sub", 0, 0}, // MINUS
    {0x03, "binop_mul", 0, 0}, // MULTIPLY
    {0x04, "binop_div", 0, 0}, // DIVIDE
    {0x05, "binop_rem", 0, 0}, // REMAINDER
    {0x06, "binop_lt", 0, 0}, // LESS
    {0x07, "binop_le", 0, 0}, // LESS_EQUAL
    {0x08, "binop_gt", 0, 0}, // GREATER
    {0x09, "binop_ge", 0, 0}, // GREATER_EQUAL
    {0x0A, "binop_eq", 0, 0}, // EQUAL
    {0x0B, "binop_ne", 0, 0}, // NOT_EQUAL
    {0x0C, "binop_and", 0, 0}, // AND
    {0x0D, "binop_or", 0, 0}, // OR

    // --- Constants & Strings ---
    {0x10, "const", 4, 0},
    {0x11, "xstring", 4, 0},
    {0x12, "sexp", 8, 0},
    {0x13, "sti", 4, 0},
    {0x14, "sta", 4, 0},

    // --- Jumps & Control ---
    {0x15, "jmp", 4, INSTR_FLAG_JUMP | INSTR_FLAG_HALT | INSTR_FLAG_BREAK},
    {0x16, "end", 0, INSTR_FLAG_HALT | INSTR_FLAG_BREAK},
    {0x17, "ret", 0, INSTR_FLAG_HALT | INSTR_FLAG_BREAK},
    {0x18, "drop", 0, 0},
    {0x19, "dup", 0, 0},
    {0x1A, "swap", 0, 0},
    {0x1B, "elem", 0, 0},

    // --- LD, LDA, ST (0x20-0x43) ---
    // Subtypes: G=0, L=1, A=2, C=3
    {0x20, "ld_g", 4, 0},
    {0x21, "ld_l", 4, 0},
    {0x22, "ld_a", 4, 0},
    {0x23, "ld_c", 4, 0},
    
    {0x30, "lda_g", 4, 0},
    {0x31, "lda_l", 4, 0},
    {0x32, "lda_a", 4, 0},
    {0x33, "lda_c", 4, 0},
    
    {0x40, "st_g", 4, 0},
    {0x41, "st_l", 4, 0},
    {0x42, "st_a", 4, 0},
    {0x43, "st_c", 4, 0},

    // --- Conditional Jumps ---
    {0x50, "cjmp_z", 4, INSTR_FLAG_JUMP | INSTR_FLAG_BREAK},
    {0x51, "cjmp_nz", 4, INSTR_FLAG_JUMP | INSTR_FLAG_BREAK},

    // --- Procedure & Closure ---
    {0x52, "begin", 8, 0},
    {0x53, "cbegin", 8, 0},
    {0x54, "closure", 8, 0},

        // --- CALL (0x56) is the ONLY one that changes flow in file ---
    // It has a target offset in file, so it IS a control transfer.
    {0x56, "call", 8, INSTR_FLAG_JUMP | INSTR_FLAG_BREAK},

    // --- Other instructions ---
    {0x55, "callc", 4, INSTR_FLAG_BREAK},      // Calls closure, NO target offset
    {0x57, "tag", 8, 0},
    {0x58, "array", 4, 0},
    {0x59, "fail", 8, INSTR_FLAG_HALT | INSTR_FLAG_BREAK},
    {0x5A, "line", 4, 0},

    // --- Pattern Matching ---
    {0x60, "patt_str", 1, 0},
    {0x61, "patt_string", 1, 0},
    {0x62, "patt_array", 1, 0},
    {0x63, "patt_sexp", 1, 0},
    {0x64, "patt_ref", 1, 0},
    {0x65, "patt_val", 1, 0},
    {0x66, "patt_fun", 1, 0},

    // --- Built-in Calls (0x70-0x74) ---
    // These do NOT have target offsets. Do NOT add to queue.
    {0x70, "call_read", 0, INSTR_FLAG_BREAK},
    {0x71, "call_write", 0, INSTR_FLAG_BREAK},
    {0x72, "call_length", 0, INSTR_FLAG_BREAK},
    {0x73, "call_string", 0, INSTR_FLAG_BREAK},
    {0x74, "call_array", 4, INSTR_FLAG_BREAK}
};

extern Instruction* instructions[256];

void init_instructions(void);

#endif // INSTRUCTIONS_H
