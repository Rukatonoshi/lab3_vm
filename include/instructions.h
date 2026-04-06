#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stddef.h>

// Instruction flags
// INSTR_FLAG_JUMP (0x01): Instructions that transfer control to a new address in the file
//   JMP (0x15), CALL (0x56), CALLC (0x55), CJMPz (0x50), CJMPnz (0x51)
// INSTR_FLAG_HALT (0x02): Instructions that terminate a basic block (do not continue to 'next')
//   JMP (0x15), END (0x16), RET (0x17), FAIL (0x59)
// INSTR_FLAG_BREAK (0x04): Instructions that break instruction sequences (reset sequence tracking)
//   JMP (0x15), END (0x16), RET (0x17), FAIL (0x59), CJMPz (0x50), CJMPnz (0x51)

#define INSTR_FLAG_JUMP     0x01
#define INSTR_FLAG_HALT     0x02
#define INSTR_FLAG_BREAK    0x04

// Field descriptor types for variable-length instruction formats
#define FIELD_OP        0   // Fixed-size operand field
#define FIELD_COUNT     1   // Count field that determines repetitions of other fields

// Maximum number of repeating fields we support (for storing count values)
#define MAX_REPEAT_FIELDS  16

// Field descriptor: describes a single field in instruction format
typedef struct {
    uint8_t type;           // FIELD_OP or FIELD_COUNT
    uint8_t size;           // Size in bytes
    // For FIELD_OP, index of count field to repeat from (-1 = no repeat)
    // For FIELD_COUNT, index where count is stored
    int8_t repeat_from;
} FieldDesc;

// Format descriptor: array of FieldDesc entries describing instruction format
typedef struct {
    const FieldDesc* fields;
    size_t field_count;
} FormatDesc;

// Forward declaration needed for function pointer type
typedef struct Instruction Instruction;

// Function pointer type for instruction length computation
typedef size_t (*InstructionLengthFn)(const uint8_t *code, size_t max_len, const Instruction *inst);

// Instruction structure
struct Instruction {
    uint8_t opcode;
    const char* name;
    int arg_size;
    uint8_t flags;
    union {
        InstructionLengthFn length_fn;      // For fixed-length (function pointer)
        const FormatDesc* format_desc;     // For variable-length (format descriptor)
    };
    bool is_varlen;           // True if format_desc is valid, false if length_fn is valid
};

// Typedef for convenience
typedef struct Instruction Instruction;

// Instruction table (indexed by opcode)
extern Instruction* instructions[256];

// Function to compute instruction length (handles variable-length instructions)
// Returns length in bytes, or 0 if invalid
extern size_t get_instruction_length(const uint8_t *code, size_t max_len);

// Get instruction by opcode; returns NULL if unknown
static inline const Instruction* get_instruction(uint8_t opcode) {
    if (opcode >= 256) return NULL;
    return instructions[opcode];
}

// Initialize instruction table
void init_instructions(void);

#endif // INSTRUCTIONS_H
