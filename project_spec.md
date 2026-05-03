# Data Compression Project — BZip2 Implementation

**Instructor:** Dr. Faisal Aslam  
**Date:** April 14, 2026  

---

## 1. Introduction

### 1.1 Project Overview
Implement a simplified version of the BZip2 compression algorithm.

Pipeline includes:
- Run-Length Encoding (RLE)
- Burrows-Wheeler Transform (BWT)
- Move-to-Front (MTF)
- Huffman Coding

⚠️ You must write all code yourself. Only prototypes are provided.

---

### 1.2 Timeline
- Total Duration: 3 Weeks
- Stage 1: 1.5 Weeks (50%)
- Stage 2: 6 Days (25%)
- Stage 3: 1 Week (25%)

---

### 1.3 Evaluation
- Encoding + Decoding required
- Extra features: +10%
- Better than BZip2: +10% (limited)

---

## 2. System Architecture

### Encoding Pipeline

Input File → Block Division → RLE-1 → BWT → MTF → RLE-2 → Huffman → Output

---

### Configuration File (config.ini)

[General]
block_size = 500000
rle1_enabled = true
bwt_type = matrix
mtf_enabled = true
rle2_enabled = true
huffman_enabled = true

[Performance]
benchmark_mode = false
output_metrics = true

[Paths]
input_directory = ./benchmarks/
output_directory = ./results/

---

## 3. Stage 1

### Block Division

typedef struct {
    unsigned char *data;
    size_t size;
    size_t original_size;
} Block;

typedef struct {
    Block *blocks;
    int num_blocks;
    size_t block_size;
} BlockManager;

Functions:
- divide_into_blocks
- reassemble_blocks
- free_block_manager

---

### RLE-1

Example:
Input:  ABBBCCCCD  
Output: A3B4C1D  

---

### BWT

typedef struct {
    char *rotation;
    int index;
} Rotation;

---

## 4. Stage 2

### MTF
- mtf_encode
- mtf_decode

### RLE-2
- rle2_encode
- rle2_decode

---

## 5. Stage 3 — Huffman Coding

typedef struct {
    unsigned short code;
    unsigned char length;
} HuffmanCode;

---

## 6. Build System

Makefile must support:
- make all
- make clean
- make windows

---

## 7. Benchmarking

Datasets:
- Canterbury Corpus
- Calgary Corpus
- Silesia Corpus
- Large files

CSV Format:
File, Size, BlockSize, CompressionRatio, Time, Memory

---

## 8. Extra Features

- Advanced RLE
- Suffix Array BWT
- ANS / Range Coding

---

## 9. Repository Structure

project-bzip2/
│── src/
│── include/
│── benchmarks/
│── results/
│── Makefile
│── config.ini
│── README.md

---

## 10. Important Notes

- No plagiarism
- Write all code yourself
- Submit GitHub repo

---

## 11. Summary

Full pipeline:
RLE → BWT → MTF → RLE → Huffman
