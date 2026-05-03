#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>

/*
 * Huffman code representation.
 */
typedef struct {
    unsigned int  code;   /* Huffman code bits */
    unsigned char length; /* Code length in bits */
} HuffmanCode;

/*
 * Huffman tree node.
 */
typedef struct Node {
    unsigned char symbol; /* Byte value (0-255) */
    int freq;             /* Frequency count */
    struct Node *left;    /* Left child */
    struct Node *right;   /* Right child */
} HuffmanNode;

/*
 * Builds Huffman tree from frequency counts.
 * @param frequencies Array of 256 frequency counts
 * @param root        Pointer to store root of Huffman tree
 */
void build_huffman_tree(int *frequencies, HuffmanNode **root);

/*
 * Generates canonical Huffman codes from tree.
 * @param root  Root of Huffman tree
 * @param codes Array of 256 to store generated codes
 */
void generate_canonical_codes(HuffmanNode *root, HuffmanCode *codes);

/*
 * Encodes data using Huffman coding.
 * Output format: [4 bytes: decoded length][256 bytes: code lengths][bitstream]
 *
 * @param input   Input byte array
 * @param len     Length of input
 * @param output  Output buffer for compressed data
 * @param out_len Pointer to store output length
 */
void huffman_encode(unsigned char *input, size_t len,
                    unsigned char *output, size_t *out_len);

/*
 * Decodes Huffman encoded data.
 * Input format: [4 bytes: decoded length][256 bytes: code lengths][bitstream]
 *
 * @param input   Huffman encoded data
 * @param len     Length of encoded data
 * @param output  Output buffer for decoded data
 * @param out_len Pointer to store output length
 */
void huffman_decode(unsigned char *input, size_t len,
                    unsigned char *output, size_t *out_len);

/*
 * Writes Huffman header (code lengths) to output.
 * @param codes   Array of Huffman codes
 * @param output  Output buffer
 * @param out_len Pointer to update with header size
 */
void write_header(HuffmanCode *codes, unsigned char *output,
                  size_t *out_len);

/*
 * Encodes data using generated codes.
 * @param input   Input data
 * @param len     Length of input
 * @param codes   Huffman codes for each symbol
 * @param output  Output buffer
 * @param out_len Pointer to update with encoded size
 */
void encode_data(unsigned char *input, size_t len,
                 HuffmanCode *codes, unsigned char *output,
                 size_t *out_len);

#endif /* HUFFMAN_H */
