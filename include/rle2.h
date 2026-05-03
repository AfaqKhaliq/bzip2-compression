#ifndef RLE2_H
#define RLE2_H

#include <stddef.h>

/*
 * Encodes MTF output using specialized RLE for zero runs.
 * Uses 0x00 as a run marker followed by a 1-255 count.
 *
 * @param input   Input byte array (MTF output)
 * @param len     Length of input
 * @param output  Output buffer
 * @param out_len Pointer to store output length
 */
void rle2_encode(unsigned char *input, size_t len,
                 unsigned char *output, size_t *out_len);

/*
 * Decodes RLE-2 encoded data back to MTF output bytes.
 *
 * @param input   Encoded byte array
 * @param len     Length of encoded data
 * @param output  Output buffer
 * @param out_len Pointer to store output length
 */
void rle2_decode(unsigned char *input, size_t len,
                 unsigned char *output, size_t *out_len);

#endif /* RLE2_H */
