// src/instructions.c
#include "instructions.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Instruction table
Instruction* instructions[256];

// Default instruction length computation for fixed-length instructions
// Format: 1 byte opcode + arg_size bytes of arguments
static size_t default_instruction_length(const uint8_t *code, size_t max_len, const Instruction *inst) {
    (void)code;  // Unused for fixed-length instructions
    (void)max_len;
    return 1 + inst->arg_size;
}

// Generic length computation from format descriptor
// This eliminates all magic numbers - format is entirely described in opcodes.def
static size_t length_from_format(const uint8_t *code, size_t max_len, const Instruction *inst) {
    if (!inst->format_desc) return 0;

    const FormatDesc *fmt = inst->format_desc;
    size_t total = 1;  // Start with opcode byte (offset 0)
    uint32_t repeats[MAX_REPEAT_FIELDS] = {0};  // Store count values

    // Process each field in the format descriptor
    for (size_t i = 0; i < fmt->field_count; i++) {
        const FieldDesc *f = &fmt->fields[i];

        if (f->type == FIELD_OP) {
            // Fixed-size operand field
            if (f->repeat_from < 0) {
                // Non-repeating field
                if (total + f->size > max_len) return 0;
                total += f->size;
            } else {
                // Repeating field: count is stored in repeats[f->repeat_from]
                uint32_t count = repeats[f->repeat_from];
                size_t field_size = (size_t)count * f->size;
                if (total + field_size > max_len) return 0;
                total += field_size;
            }
        }
        else if (f->type == FIELD_COUNT) {
            // Count field: read the value and store it for later fields
            if (total + f->size > max_len) return 0;  // Would exceed buffer

            uint32_t count = 0;
            switch (f->size) {
                case 1:
                    count = code[total];
                    break;
                case 2:
                    count = code[total] | (code[total + 1] << 8);
                    break;
                case 4:
                    memcpy(&count, code + total, f->size);
                    break;
                default:
                    return 0;  // Unsupported size
            }

            // Store the count value at the specified index
            if (f->repeat_from >= 0 && f->repeat_from < MAX_REPEAT_FIELDS) {
                repeats[f->repeat_from] = count;
            }
            total += f->size;
        }
        else {
            // Unknown field type
            return 0;
        }
    }

    // validity check: computed length must fit in buffer
    if (total > max_len) return 0;
    return total;
}

// Generic instruction length computation
// Returns length in bytes, or 0 if invalid
size_t get_instruction_length(const uint8_t *code, size_t max_len) {
    if (max_len < 1) return 0;

    uint8_t opcode = code[0];
    const Instruction* inst = get_instruction(opcode);
    if (!inst) return 0;

    // Use appropriate length computation based on instruction type
    if (inst->is_varlen) {
        return length_from_format(code, max_len, inst);
    } else {
        if (!inst->length_fn) return 0;
        return inst->length_fn(code, max_len, inst);
    }
}

// Format Descriptor Generation
// Pass 1: Generate field arrays
#define BEGIN_FORMAT(name) static const FieldDesc name##_fields[] = {
#define FORMAT_FIELD(type, size, repeat_from) { type, size, repeat_from },
#define FORMAT_END };

// Include opcodes.def to generate field arrays only
#undef INSTR
#define INSTR(code, name, arg_size, flags)

#undef VARLEN_DESCRIPTOR
#define VARLEN_DESCRIPTOR(code, name, flags, format_name, count_val)

#include "../include/opcodes.def"

// Undefine field array generation macros
#undef BEGIN_FORMAT
#undef FORMAT_FIELD
#undef FORMAT_END
#undef INSTR
#undef VARLEN_DESCRIPTOR

// Pass 2: Generate format descriptor wrappers using field_count from VARLEN_DESCRIPTOR
#undef INSTR
#define INSTR(code, name, arg_size, flags)

#undef VARLEN_DESCRIPTOR
#define VARLEN_DESCRIPTOR(code, name, flags, format_name, count_val) \
    static const FormatDesc format_name##_desc = { \
        .fields = format_name##_fields, \
        .field_count = count_val \
    };

#include "../include/opcodes.def"

// Cleanup
#undef INSTR
#undef VARLEN_DESCRIPTOR

// Instruction Table Generation
// Generate instruction table from X-macros
static Instruction instruction_table[] = {

    // Define INSTR to generate struct initializers for fixed-length instructions
    #undef INSTR
    #define INSTR(code, name, arg_size, flags) \
        { 0x##code, #name, arg_size, flags, .length_fn = default_instruction_length, .is_varlen = false },

    // Define VARLEN_DESCRIPTOR to generate struct initializers from format descriptors
    #undef VARLEN_DESCRIPTOR
    #define VARLEN_DESCRIPTOR(code, name, flags, format_name, count_val) \
        { 0x##code, #name, 0, flags, .format_desc = &format_name##_desc, .is_varlen = true },

    // Include the instruction definitions
    #include "../include/opcodes.def"

    #undef INSTR
    #undef VARLEN_DESCRIPTOR
};

// Instruction table initialization
static void init_instructions_table(void) {
    // Clear the instruction table
    for (int i = 0; i < 256; i++) {
        instructions[i] = NULL;
    }

    // Populate from generated table
    size_t table_size = sizeof(instruction_table) / sizeof(instruction_table[0]);
    for (size_t i = 0; i < table_size; i++) {
        uint8_t opcode = instruction_table[i].opcode;
        if (opcode < 256) {
            instructions[opcode] = &instruction_table[i];
        }
    }
}

// Initialize instructions (called automatically at program start)
__attribute__((constructor))
void init_instructions(void) {
    init_instructions_table();
}
