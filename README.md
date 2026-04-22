# BZip2 Compression - Stage 1 Implementation

## Project Overview

This is **Stage 1** of a three-stage implementation of the BZip2 compression algorithm. Stage 1 focuses on the foundational components: **Block Division**, **RLE-1 Encoding**, and the **Burrows-Wheeler Transform (BWT)**.

### Project Timeline

- **Stage 1** (1.5 Weeks): Block division, RLE-1, BWT - **✓ COMPLETED**
- **Stage 2** (6 Days): Move-to-Front (MTF), RLE-2
- **Stage 3** (1 Week): Huffman Coding

---

## Stage 1: Architecture & Components

### 1. Block Division

**Purpose:** Handle very large files by splitting them into manageable blocks.

- **Configurable block size:** 100KB - 900KB (default: 500KB)
- **Reads** input file sequentially
- **Divides** into fixed-size chunks (last block may be smaller)
- **Stores** each block with size metadata

**Benefits:**

- Enables processing of files larger than available RAM
- Allows parallel processing of blocks (future optimization)
- Configurable based on available memory

### 2. RLE-1 Encoding (Run-Length Encoding)

**Purpose:** Compress runs of identical bytes before BWT. This is the **actual BZip2 RLE-1 scheme**, not a simplified version.

**Algorithm:**

- Scans input for sequences of identical bytes
- **Runs < 4 bytes:** Copy verbatim (no encoding)
- **Runs ≥ 4 bytes:** Emit 4 copies of the byte, then a count byte (count = run_length - 4)
- **Maximum run:** 259 bytes (4 + 255)

**Example:**

```
Input:  AAAAAAA      (7 identical A's)
Output: A A A A \x03 (4 copies + count byte 3, meaning 4+3=7 total)

Input:  ABC          (no runs)
Output: A B C        (unchanged)
```

**Advantages:**

- Bounded output size: never exceeds input + 25% overhead
- Pairs naturally with BWT output (which has many runs)
- Reduces entropy for Huffman coding

### 3. Burrows-Wheeler Transform (BWT)

**Purpose:** Transform data into a form with more repeating patterns (better compressibility).

**Algorithm:**

1. **Create all cyclic rotations** of the input string
2. **Sort rotations lexicographically** (dictionary order)
3. **Output the last column** of sorted rotations
4. **Record primary_index:** which row contains the original string

**Example:**

```
Original: "BANANA"

All rotations:
  BANANA
  ANANAB
  NANABA
  ANABAN
  NABANA
  ABANAN

After sorting:
  ABANAN  ← row 0
  ANABAN  ← row 1
  ANANAB  ← row 2
  BANANA  ← row 3 (original - primary_index = 3)
  NABANA  ← row 4
  NANABA  ← row 5

Last column: ANNBAA
Output: [3, ANNBAA]  (primary_index=3, BWT data="ANNBAA")
```

**Implementation Details:**

- **Matrix-based approach:** O(n² log n) time, O(n) space
- Uses index array [0,1,2,...,n-1] for rotations
- Compares rotations on-the-fly using modulo arithmetic
- Suitable for blocks ≤ 1MB

**Why BWT is Powerful:**

- Groups similar characters together
- Creates long runs of identical bytes
- Output is perfectly reversible with primary_index
- Dramatically improves compressibility when combined with RLE

### 4. Full Stage 1 Pipeline

```
Input File
    ↓
[Block Division]
    ↓
For each block:
    ├─ [RLE-1 Encode] (if enabled)
    │   ├─ Compress runs of 4+ bytes
    │   └─ Output has up to 25% overhead
    │
    └─ [BWT Transform]
        ├─ Create cyclic rotations
        ├─ Sort lexicographically
        ├─ Extract last column
        └─ Store primary_index (4 bytes header)
    ↓
[Reassemble Blocks]
    ↓
Compressed File
```

---

## Building the Project

### Prerequisites

- **Compiler:** GCC (MinGW on Windows, or WSL)
- **Tools:** Make (optional, can compile manually)

### Build Commands

#### Using Make (Linux/WSL):

```bash
cd stage1
make clean
make all
```

#### Using Make for Windows Cross-Compilation:

```bash
make windows    # Creates bzip2_stage1.exe
```

#### Manual Compilation (Any Platform):

```bash
# Compile each source file
gcc -Wall -Wextra -O2 -Iinclude -c src/config.c -o src/config.o
gcc -Wall -Wextra -O2 -Iinclude -c src/block.c -o src/block.o
gcc -Wall -Wextra -O2 -Iinclude -c src/rle.c -o src/rle.o
gcc -Wall -Wextra -O2 -Iinclude -c src/bwt.c -o src/bwt.o
gcc -Wall -Wextra -O2 -Iinclude -c src/main.c -o src/main.o

# Link into executable
gcc -Wall -Wextra -O2 -o bzip2_stage1 src/main.o src/config.o src/block.o src/rle.o src/bwt.o
```

### Build Output

```
bzip2_stage1              (or bzip2_stage1.exe on Windows)
```

---

## Usage Guide

### Encoding a File

**Command Format:**

```bash
./bzip2_stage1 encode <input_file> <output_file> [config.ini]
```

**Examples:**

```bash
# Basic encoding with default config
./bzip2_stage1 encode input.txt output.bz

# Encoding with custom config
./bzip2_stage1 encode document.pdf compressed.bz custom_config.ini

# Batch encode multiple files
for file in *.txt; do
  ./bzip2_stage1 encode "$file" "$file.bz"
done
```

**Output:**

```
[Config]
  block_size       = 500000
  rle1_enabled     = true
  bwt_type         = matrix
  mtf_enabled      = true
  rle2_enabled     = true
  huffman_enabled  = true
  benchmark_mode   = false
  output_metrics   = true
  input_directory  = ./benchmarks/
  output_directory = ./results/

Read 1 block(s) from 'input.txt'
  Encoding block 1/1 (1024 bytes)...
Output written to 'output.bz'
```

### Decoding a File

**Command Format:**

```bash
./bzip2_stage1 decode <compressed_file> <output_file> [config.ini]
```

**Examples:**

```bash
# Basic decoding
./bzip2_stage1 decode output.bz recovered.txt

# Decoding with custom config
./bzip2_stage1 decode compressed.bz recovered.pdf config.ini

# Batch decode
for file in *.bz; do
  ./bzip2_stage1 decode "$file" "${file%.bz}"
done
```

**Output:**

```
[Config]
  block_size       = 500000
  ...
Read 1 block(s) from 'output.bz'
  Decoding block 1/1 (512 bytes)...
Output written to 'recovered.txt'
```

---

## Configuration

### config.ini Format

```ini
[General]
block_size = 500000        # Bytes (100KB to 900KB)
rle1_enabled = true        # Enable RLE-1 in Stage 1
bwt_type = matrix          # matrix or suffix_array (Stage 2+)
mtf_enabled = true         # Enable MTF (Stage 2)
rle2_enabled = true        # Enable RLE-2 (Stage 2)
huffman_enabled = true     # Enable Huffman (Stage 3)

[Performance]
benchmark_mode = false     # Enable detailed benchmarking
output_metrics = true      # Output compression metrics

[Paths]
input_directory = ./benchmarks/   # Default input directory
output_directory = ./results/     # Default output directory
```

### Configuration Notes

- **block_size:** Automatically clamped to [100KB, 900KB] range
- **Stage 1 only uses:** `block_size`, `rle1_enabled`, `bwt_type`
- Missing config file → uses safe defaults
- Inline comments (after `#`) are stripped

---

## Test Results

### Test 1: Simple Text File

**Input:** `"Hello World! This is a test file for BZip2 compression."`  
**File size:** 57 bytes  
**Result:** ✓ **PASS** - Round-trip encode/decode verified

```bash
./bzip2_stage1 encode test_input.txt test_encoded.bin config.ini
./bzip2_stage1 decode test_encoded.bin test_decoded.txt config.ini
# Verification: Original == Decoded ✓
```

### Test 2: Repeated Data

**Input:** Pattern `"AAABBBCCCDDDEEEFFFGGGHHH"` repeated 100 times  
**File size:** 2402 bytes  
**Compressed size:** 2406 bytes  
**Compression ratio:** 100.17%  
**Result:** ✓ **PASS** - Handles non-compressible data gracefully

```bash
./bzip2_stage1 encode test_repeated.txt test_repeated.bin config.ini
./bzip2_stage1 decode test_repeated.bin test_repeated_decoded.txt config.ini
# Verification: Original == Decoded ✓
```

### Test 3: File Integrity

**Test:** Encode and decode random binary files  
**Result:** ✓ **PASS** - 100% byte-for-byte accuracy

---

## Project Structure

```
stage1/
├── src/
│   ├── main.c           # Main program, encode/decode orchestration
│   ├── config.c         # Configuration file parsing
│   ├── block.c          # Block division and reassembly
│   ├── rle.c            # RLE-1 encoding/decoding
│   └── bwt.c            # BWT forward/inverse transform
├── include/
│   ├── main.h           # (implied by main.c includes)
│   ├── config.h         # Config struct and functions
│   ├── block.h          # Block and BlockManager structs
│   ├── rle.h            # RLE-1 function prototypes
│   └── bwt.h            # BWT function prototypes
├── Makefile             # Build automation (Linux/WSL)
├── config.ini           # Configuration file
├── README.md            # This file
└── test files:
    ├── test_input.txt
    ├── test_encoded.bin
    ├── test_decoded.txt
    ├── test_repeated.txt
    ├── test_repeated.bin
    └── test_repeated_decoded.txt
```

---

## File Format

### Encoded File Structure (per block)

```
[4 bytes: primary_index (little-endian int32)]
[N bytes: BWT output]

Total: 4 + N bytes
```

**Example for 57-byte input:**

- Primary index: 4 bytes
- BWT output: ~57 bytes
- Total: ~61 bytes (4 bytes overhead)

---

## Performance Notes

### Stage 1 Compression

- **Text files:** Modest compression (RLE-1 + BWT prepare data for later stages)
- **Repetitive data:** Limited compression at this stage
- **Binary data:** May not compress well until Stage 2 (MTF) and Stage 3 (Huffman)

**Expected ratios:**

- English text: ~20-30% after all 3 stages
- Code files: ~15-25% after all 3 stages
- Random data: ~100% (cannot compress)

### Memory Usage

- **Per block:** O(n) where n = block_size
- BWT creates n rotation indices (~500KB × 4 bytes = 2MB for default block size)
- Total working memory: ~3-4MB for default configuration

### Time Complexity

- **RLE-1:** O(n) linear
- **BWT:** O(n² log n) due to lexicographic sorting
- **BWT Inverse:** O(n) linear

---

## Next Steps: Stage 2

Stage 1 prepares the data through RLE-1 and BWT. Stage 2 will add:

1. **Move-to-Front (MTF):** Transforms repeated characters to small numbers
2. **RLE-2:** Specialized RLE targeting MTF output (many zeros)
3. **Significant compression improvements** on most file types

Expected compression ratio improvement: **30-50%** better after Stage 2.

---

## Troubleshooting

### Compilation Errors

- **GCC not found:** Install MinGW (Windows) or use WSL
- **Header not found:** Ensure you're in `stage1/` directory
- **Undefined reference:** Make sure all 5 `.c` files are compiled

### Runtime Issues

- **"Cannot open input file":** Check file path and permissions
- **Config file not found:** Uses defaults automatically (safe fallback)
- **Output file empty:** Check available disk space and write permissions

### Common Commands

```bash
# Clean build
make clean
make all

# Get help
./bzip2_stage1              # Shows usage

# Verify integrity after round-trip
diff original.txt decoded.txt
# (Should output nothing if files match)
```

---

## Implementation Details

### Key Algorithms

#### RLE-1 State Machine

```c
for each byte in input:
  if byte is same as previous AND we have < 4 of them:
    increment count
  else if we had a run of exactly 4:
    output count byte
    reset count
  else:
    output byte
```

#### BWT Sorting

Uses **qsort** with a custom comparator that:

1. Compares cyclic rotations on-the-fly
2. Uses modulo arithmetic to wrap around string
3. Returns -1, 0, or 1 for lexicographic ordering

#### BWT Inverse (Counting Sort)

```
1. Count frequency of each byte
2. Build T-mapping (where each occurrence appears in sorted order)
3. Follow chain starting from primary_index
4. Read characters in reverse order
```

---

## References

- **BZip2 Algorithm:** Seward, J. (1996)
- **Burrows-Wheeler Transform:** Burrows & Wheeler (1994)
- **RLE-1 in BZip2:** Optimized to guarantee ≤125% output size

---

## Testing Checklist

- [x] Block division works for various file sizes
- [x] RLE-1 encode/decode round-trip verified
- [x] BWT encode/decode round-trip verified
- [x] Configuration file parsing works
- [x] Simple text files compress and decompress
- [x] Complex patterns handled correctly
- [x] File integrity maintained (100% accurate)

---

## Future Enhancements (Stage 2 & 3)

- Move-to-Front transform for byte frequency clustering
- RLE-2 for MTF zero runs
- Canonical Huffman coding for entropy encoding
- Suffix array-based BWT (O(n log n) for larger blocks)
- Parallel block processing
- Performance benchmarking suite

---

**Status:** ✓ Stage 1 Complete and Verified  
**Last Updated:** April 22, 2026  
**Team:** Data Compression Project
