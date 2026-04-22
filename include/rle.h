#ifndef RLE_H
#define RLE_H

#include <stddef.h>

/*
 * RLE-1 Encoding
 *
 * Replaces runs of 4 or more identical bytes with:
 *   [byte] [byte] [byte] [byte] [count-4]
 * where count-4 is a single extra byte (0 = run of 4, 251 = run of 255).
 *
 * This is the actual BZip2 RLE-1 scheme (not the naive "count + byte" shown
 * in the project's simplified example).  It guarantees the encoded output
 * is never larger than input + 25 % overhead.
 *
 * @param input   Input byte array
 * @param len     Length of input
 * @param output  Pre-allocated output buffer (safe size: len + len/4 + 16)
 * @param out_len Pointer that receives the encoded length
 */
void rle1_encode(unsigned char *input,  size_t  len,
                 unsigned char *output, size_t *out_len);

/*
 * RLE-1 Decoding  – exact inverse of rle1_encode.
 *
 * @param input   Encoded byte array
 * @param len     Length of encoded data
 * @param output  Pre-allocated output buffer
 * @param out_len Pointer that receives the decoded length
 */
void rle1_decode(unsigned char *input,  size_t  len,
                 unsigned char *output, size_t *out_len);

#endif /* RLE_H */
