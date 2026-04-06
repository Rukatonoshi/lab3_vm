# Lama Static Frequency Analyzer

## Static analyzer for instruction sequence frequency in [Lama](https://github.com/PLTools/Lama.git) bytecode programs

This tool analyzes Lama bytecode files to count the frequency of instruction sequences (1-2 parameterized bytecodes) and outputs the results sorted by frequency.

## Task Requirements

- Reads bytecode from file in Lama format
- Performs reachability analysis using BFS from public symbols (entry points)
- Handles transfer labels by breaking instruction sequences at control flow points
- Assumes jumps to instruction interiors cannot occur (validated during BFS)
- Counts sequences of 1-2 parameterized bytecodes
- Outputs results sorted by frequency (descending), omitting zero-frequency sequences

## Build

In the project root directory execute:
```bash
make
```

## Run frequency analyzer

Analyze a Lama bytecode file:
```bash
./frequency_analyzer <path_to_bc_file>
```

Example for lama bubble-sort:
```bash
./frequency_analyzer Sort.bc
```

## Generate Lama bytecode

To generate Lama bytecode, use the Lama compiler:
```bash
lamac -b <path_to_lama_file>
```

Example for lama bubble-sort:
```bash
lamac -b Sort.lama
```

## Output Format

The analyzer outputs frequency results in the format:
```
<frequency> : <instruction sequence>
```

Instruction arguments are displayed as decimal values.

Example output:
```bash
31 : DROP
28 : DUP
21 : ELEM
16 : CONST 1
13 : CONST 1, ELEM
11 : CONST 0
```

## Algorithm

The analyzer uses a two-phase approach:

### Phase 1: Reachability Analysis
1. Start BFS from all public symbols (program entry points)
2. Propagate reachability through control flow
   - For control transfer instructions, enqueue jump targets
   - For non-terminal instructions, enqueue next sequential address
3. Validate enqueued addresses contain valid instruction opcodes
4. This prevents analysis of instruction arguments as if they were instructions

### Phase 2: Sequence Counting
1. Traverse reachable code linearly
2. Count single instruction sequences
3. Count instruction pair sequences (reset at control flow points)
4. Handle instruction sequence breaking at transfer labels

## Files

- `include/instructions.h` - Instruction metadata and type definitions
- `include/opcodes.def` - X-macro instruction definitions (single source of truth)
- `src/instructions.c` - Instruction table and length calculation
- `src/frequency_analyzer.c` - Main frequency analysis logic
- `src/byte_file.h` - Bytecode file parsing utilities
- `src/uthash.h` - Hash table implementation for sequence counting

## Performance

Optimized for large bytecode files (up to 1GB as per requirements):
- Linear scanning for reachability (BFS)
- Efficient hash table operations
- Minimal memory overhead beyond reachability tracking
- O(n) complexity for both phases

<details>

<summary>Instruction frequency analyzer results for Sort.bc</summary>

```bash

--- Function Calls ---
Function: main at file offset 0x0000001e (code offset 0x00000000)

--- Reachability Stats ---
Total code size: 764 bytes
Reachable instructions: 204
Reachable code: 26.70%
31 : drop
28 : dup
21 : elem
16 : const 1
13 : const 1, elem
11 : const 0
11 : drop, dup
11 : dup, const 1
10 : drop, drop
8 : const 0, elem
7 : dup, const 0
7 : elem, drop
7 : ld_a 0
5 : end
4 : sexp 0 2
4 : dup, dup
3 : jmp 762
3 : dup, array 2
3 : elem, st_l 0
3 : ld_l 0
3 : ld_l 3
3 : st_l 0
3 : st_l 0, drop
3 : call 351 1
3 : array 2
3 : call_array 2
3 : call_array 2, jmp 762
2 : binop_eq
2 : sexp 0 2, call_array 2
2 : jmp 350
2 : jmp 116
2 : dup, tag 0 2
2 : elem, const 0
2 : elem, const 1
2 : ld_l 1
2 : begin 1 0
2 : call 43 1
2 : call 351 1, dup
2 : call 151 1
2 : tag 0 2
1 : binop_sub
1 : binop_sub, call 43 1
1 : binop_gt
1 : binop_gt, cjmp_z 600
1 : binop_eq, cjmp_z 274
1 : binop_eq, cjmp_z 191
1 : const 0, binop_eq
1 : const 0, jmp 116
1 : const 0, line 9
1 : const 1, binop_sub
1 : const 1, binop_eq
1 : const 1, line 6
1 : const 1000
1 : const 1000, call 43 1
1 : sexp 0 2, jmp 116
1 : sexp 0 2, call 351 1
1 : jmp 262
1 : jmp 336
1 : jmp 386
1 : jmp 715
1 : jmp 734
1 : drop, const 0
1 : drop, jmp 262
1 : drop, jmp 336
1 : drop, jmp 386
1 : drop, jmp 715
1 : drop, jmp 734
1 : drop, ld_l 5
1 : drop, line 5
1 : drop, line 15
1 : drop, line 16
1 : dup, drop
1 : elem, sexp 0 2
1 : elem, dup
1 : elem, st_l 1
1 : elem, st_l 2
1 : elem, st_l 3
1 : elem, st_l 4
1 : elem, st_l 5
1 : ld_l 0, sexp 0 2
1 : ld_l 0, jmp 350
1 : ld_l 0, call 151 1
1 : ld_l 1, binop_gt
1 : ld_l 1, ld_l 3
1 : ld_l 2
1 : ld_l 2, call 351 1
1 : ld_l 3, ld_l 0
1 : ld_l 3, ld_l 1
1 : ld_l 3, ld_l 4
1 : ld_l 4
1 : ld_l 4, sexp 0 2
1 : ld_l 5
1 : ld_l 5, ld_l 3
1 : ld_a 0, const 1
1 : ld_a 0, dup
1 : ld_a 0, ld_a 0
1 : ld_a 0, cjmp_z 106
1 : ld_a 0, call 351 1
1 : ld_a 0, call 151 1
1 : ld_a 0, call_array 2
1 : st_l 1
1 : st_l 1, drop
1 : st_l 2
1 : st_l 2, drop
1 : st_l 3
1 : st_l 3, drop
1 : st_l 4
1 : st_l 4, drop
1 : st_l 5
1 : st_l 5, drop
1 : cjmp_z 274
1 : cjmp_z 600
1 : cjmp_z 106
1 : cjmp_z 191
1 : cjmp_nz 280
1 : cjmp_nz 637
1 : cjmp_nz 392
1 : cjmp_nz 428
1 : cjmp_nz 197
1 : begin 1 0, line 18
1 : begin 1 0, line 24
1 : begin 1 1
1 : begin 1 1, line 14
1 : begin 1 6
1 : begin 1 6, line 3
1 : begin 2 0
1 : begin 2 0, line 25
1 : call 43 1, sexp 0 2
1 : call 43 1, call 117 1
1 : call 351 1, const 1
1 : call 117 1
1 : call 117 1, end
1 : call 151 1, jmp 350
1 : call 151 1, end
1 : tag 0 2, cjmp_nz 392
1 : tag 0 2, cjmp_nz 428
1 : array 2, cjmp_nz 280
1 : array 2, cjmp_nz 637
1 : array 2, cjmp_nz 197
1 : fail 7 17
1 : fail 14 9
1 : line 3
1 : line 3, ld_a 0
1 : line 5
1 : line 5, ld_l 3
1 : line 6
1 : line 6, ld_l 1
1 : line 7
1 : line 7, ld_l 2
1 : line 9
1 : line 9, ld_a 0
1 : line 14
1 : line 14, ld_a 0
1 : line 15
1 : line 15, ld_l 0
1 : line 16
1 : line 16, ld_l 0
1 : line 18
1 : line 18, line 20
1 : line 20
1 : line 20, ld_a 0
1 : line 24
1 : line 24, ld_a 0
1 : line 25
1 : line 25, line 27
1 : line 27
1 : line 27, const 1000
```

</details>
