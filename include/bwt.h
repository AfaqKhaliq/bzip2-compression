#ifndef BWT_H
#define BWT_H

#include <stddef.h>

/* ── Data Structure (as specified in the project) ── */

typedef struct {
    char *rotation;  /* Rotation string  */
    int   index;     /* Original index   */
} Rotation;

/* ── Function Prototypes ── */

/*
 * Compares two Rotation pointers for qsort().
 */
int compare_rotations(const void *a, const void *b);

/*
 * Forward BWT.
 *
 * Creates all cyclic rotations of `input`, sorts them lexicographically,
 * writes the last column into `output`, and stores the row index of the
 * original string in *primary_index.
 *
 * Both input and output must have length `len`.
 */
void bwt_encode(unsigned char *input,  size_t len,
                unsigned char *output, int   *primary_index);

/*
 * Inverse BWT.
 *
 * Reconstructs the original string from the BWT output and primary_index.
 * `output` must have length `len`.
 */
void bwt_decode(unsigned char *input,  size_t len,
                int primary_index,     unsigned char *output);

#endif /* BWT_H */
