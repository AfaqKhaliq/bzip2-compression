#ifndef MTF_H
#define MTF_H

#include <stddef.h>

/*
 * Forward MTF transform.
 * @param input  Input byte array
 * @param len    Length of input
 * @param output Output buffer for MTF indices
 */
void mtf_encode(unsigned char *input, size_t len,
                unsigned char *output);

/*
 * Inverse MTF transform.
 * @param input  MTF encoded indices
 * @param len    Length of input
 * @param output Output buffer for decoded data
 */
void mtf_decode(unsigned char *input, size_t len,
                unsigned char *output);

#endif /* MTF_H */
