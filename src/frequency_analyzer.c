// src/frequency_analyzer.c
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "instructions.h"
#include "byte_file.h"
#include "uthash.h"

// Hash table entry for counting byte sequences
typedef struct {
    uint8_t *bytes;   // Copy of the byte sequence
    size_t len;       // Length of the sequence in bytes
    uint32_t count;   // Frequency of this sequence
    UT_hash_handle hh; // Hash table handle
} CountEntry;

// Global hash table for sequence counts
static CountEntry *counts = NULL;

// Helper: copy bytes from src to dst
static inline void copy_bytes(uint8_t *dst, const uint8_t *src, size_t len) {
    memcpy(dst, src, len);
}

// Increment count for a given byte sequence
static void increment_count(const uint8_t *data, size_t len) {
    CountEntry *entry;
    HASH_FIND(hh, counts, data, len, entry);
    if (!entry) {
        entry = (CountEntry*)calloc(1, sizeof(CountEntry));
        entry->bytes = (uint8_t*)calloc(len, 1);
        copy_bytes((uint8_t*)entry->bytes, data, len);
        entry->len = len;
        entry->count = 0;
        HASH_ADD_KEYPTR(hh, counts, entry->bytes, len, entry);
    }
    entry->count++;
}

// Check if instruction is a control transfer (jump, call, etc.)
static inline bool is_control_transfer(const Instruction* inst) {
    return (inst->flags & (INSTR_FLAG_JUMP | INSTR_FLAG_CJUMP));
}

// Check if instruction is terminal (ends execution)
static inline bool is_terminal(const Instruction* inst) {
    return (inst->flags & INSTR_FLAG_HALT);
}

// Check if instruction breaks a sequence (jump, call, halt)
static inline bool split_after(const Instruction* inst) {
    return (inst->flags & INSTR_FLAG_HALT);
}

// Get instruction by opcode; returns NULL if unknown
static inline const Instruction* get_instruction(uint8_t opcode) {
    if (opcode >= 256) return NULL;
    return instructions[opcode];
}

// Return length of instruction in bytes, or 0 if unknown
static inline size_t instruction_length(const u_int8_t *code, size_t max_len) {
    if (max_len < 1) return 0;
    uint8_t opcode = code[0];
    const Instruction* inst = get_instruction(opcode);
    if (!inst) return 0;
    return 1 + inst->arg_size;
}

// Compare two entries: by frequency (descending), then by byte sequence (lexicographic)
static int compare_entries(const void *a, const void *b) {
    const CountEntry *ea = *(const CountEntry**)a;
    const CountEntry *eb = *(const CountEntry**)b;

    // Sort by frequency (descending)
    if (ea->count != eb->count) {
        return (ea->count < eb->count) ? 1 : -1;
    }

    // Then by byte sequence (lexicographic)
    size_t min_len = ea->len < eb->len ? ea->len : eb->len;
    int cmp = memcmp(ea->bytes, eb->bytes, min_len);
    if (cmp != 0) return cmp;

    // If one is shorter, it comes first
    if (ea->len < eb->len) return -1;
    if (ea->len > eb->len) return 1;
    return 0;
}

// Read a 32-bit little-endian integer from raw bytes
static uint32_t read_uint32(const uint8_t *data) {
    return (data[0] << 0) |
           (data[1] << 8) |
           (data[2] << 16) |
           (data[3] << 24);
}

// Print a sequence of instructions with names and arguments
static void print_sequence(const uint8_t *data, size_t len) {
    size_t pos = 0;
    int first = 1;
    while (pos < len) {
        const Instruction* inst = get_instruction(data[pos]);
        if (!inst) {
            if (first) {
                printf("<unknown>");
            } else {
                printf(", <unknown>");
            }
            pos++;
            continue;
        }

        if (!first) {
            printf(", ");
        }
        printf("%s", inst->name);

        const uint8_t *args = data + pos + 1;
        size_t arg_size = inst->arg_size;

        // Print arguments as 32-bit integers (little-endian)
        for (size_t i = 0; i < arg_size; i += 4) {
            if (i + 4 <= arg_size) {
                uint32_t val = read_uint32(args + i);
                printf(" %u", val);
            }
        }

        pos += 1 + arg_size;
        first = 0;
    }
}

// Main frequency analysis function
void analyze_frequency(byte_file *bf) {
    // Track reachable code bytes
    uint8_t *reachable = (uint8_t*)calloc(bf->code_size, 1);
    if (!reachable) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    // Track jump targets (to enqueue them)
    uint8_t *jump_target = (uint8_t*)calloc(bf->code_size, 1);
    if (!jump_target) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    // BFS queue for reachability analysis
    uint32_t *queue = (uint32_t*)malloc(bf->code_size * sizeof(uint32_t));
    if (!queue) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    uint32_t qhead = 0, qtail = 0;

    // Enqueue all public symbols (entry points)
    for (uint32_t i = 0; i < bf->public_symbols_number; i++) {
        uint32_t addr = get_public_offset(bf, i);
        if (addr >= bf->code_size) {
            fprintf(stderr, "Public symbol offset %u out of code bounds\n", addr);
            continue;
        }
        if (!reachable[addr]) {
            reachable[addr] = 1;
            queue[qtail++] = addr;
        }
    }

    // BFS: propagate reachability
    while (qhead < qtail) {
        uint32_t addr = queue[qhead++];
        const uint8_t *code = (const uint8_t*)bf->code_ptr + addr;
        size_t remaining = bf->code_size - addr;
        size_t len = instruction_length(code, remaining);
        if (len == 0) {
            fprintf(stderr, "Unknown instruction at offset 0x%08x\n", addr);
            continue;
        }

        const Instruction* inst = get_instruction(code[0]);
        if (!inst) {
            fprintf(stderr, "Unknown instruction at offset 0x%08x\n", addr);
            continue;
        }

        // If it's a control transfer (jump, call), enqueue target
        if (is_control_transfer(inst)) {
            uint32_t target = 0;
            memcpy(&target, code + 1, 4); // 4-byte target offset
            if (target >= bf->code_size) {
                fprintf(stderr, "Jump target 0x%08x out of bounds at offset 0x%08x\n", target, addr);
            } else if (!reachable[target]) {
                reachable[target] = 1;
                queue[qtail++] = target;
            }
        }

        // If not terminal, enqueue next instruction
        if (!is_terminal(inst)) {
            uint32_t next = addr + len;
            if (next < bf->code_size && !reachable[next]) {
                reachable[next] = 1;
                queue[qtail++] = next;
            }
        }
    }

    free(queue);

    // Traverse reachable code and count sequences of length 1 and 2
    uint32_t i = 0;
    uint8_t *prev_bytes = NULL;
    size_t prev_len = 0;

    while (i < bf->code_size) {
        if (!reachable[i]) {
            i++;
            continue;
        }

        const uint8_t *code = (const uint8_t*)bf->code_ptr + i;
        size_t remaining = bf->code_size - i;
        size_t len = instruction_length(code, remaining);
        if (len == 0) {
            i++;
            continue;
        }

        const Instruction* inst = get_instruction(code[0]);
        if (!inst) {
            i++;
            continue;
        }

        // Count current instruction (length 1)
        increment_count(code, len);

        // Count pair with previous instruction (length 2)
        if (prev_bytes) {
            size_t pair_len = prev_len + len;
            uint8_t *pair_bytes = (uint8_t*)malloc(pair_len);
            memcpy(pair_bytes, prev_bytes, prev_len);
            memcpy(pair_bytes + prev_len, code, len);
            increment_count(pair_bytes, pair_len);
            free(pair_bytes);
        }

        // If instruction breaks the sequence, reset previous
        if (split_after(inst)) {
            free(prev_bytes);
            prev_bytes = NULL;
            prev_len = 0;
        } else {
            // Otherwise, save current as previous
            free(prev_bytes);
            prev_bytes = (uint8_t*)malloc(len);
            memcpy(prev_bytes, code, len);
            prev_len = len;
        }

        i += len;
    }

    free(prev_bytes);

    // Collect all entries from hash table
    CountEntry *entry, *tmp;
    size_t n_entries = HASH_COUNT(counts);
    CountEntry **array = (CountEntry**)malloc(n_entries * sizeof(CountEntry*));
    if (!array) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    size_t idx = 0;
    HASH_ITER(hh, counts, entry, tmp) {
        array[idx++] = entry;
    }

    // Sort by frequency (descending), then by byte sequence (lexicographic)
    qsort(array, n_entries, sizeof(CountEntry*), compare_entries);

    // Output results
    for (size_t j = 0; j < n_entries; j++) {
        entry = array[j];
        if (entry->count == 0) continue;
        printf("%u : ", entry->count);
        print_sequence(entry->bytes, entry->len);
        printf("\n");
    }

    // Cleanup
    HASH_ITER(hh, counts, entry, tmp) {
        HASH_DEL(counts, entry);
        free(entry->bytes);
        free(entry);
    }
    free(array);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <lambc-file>\n", argv[0]);
        return 1;
    }

    byte_file *bf = read_file(argv[1]);
    if (!bf) {
        fprintf(stderr, "Failed to read file\n");
        return 1;
    }

    analyze_frequency(bf);
    free(bf);
    return 0;
}
