#include "bwt.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * ══════════════════════════════════════════════════════════════════
 *  FORWARD BWT  (matrix-based, O(n² log n) – good for blocks ≤ 1 MB)
 * ══════════════════════════════════════════════════════════════════
 *
 * How it works:
 *   1. Consider the input string S of length n.
 *   2. Build all n cyclic rotations:  S[i..n-1] + S[0..i-1]
 *   3. Sort those rotations lexicographically.
 *   4. The BWT output is the LAST character of each sorted rotation.
 *   5. Record which row contains the original string → primary_index.
 *
 * Memory trick: instead of storing n strings of length n (O(n²) memory),
 * we store only the STARTING INDEX of each rotation and do comparisons
 * on the fly using the original buffer.  This is still O(n² log n) time
 * but only O(n) extra memory.
 */

/* ── compare_rotations ── */

/*
 * We pass the input pointer via a global because qsort's comparator only
 * receives the two elements.  This is the standard C idiom for BWT.
 */
static const unsigned char *g_bwt_input = NULL;
static size_t               g_bwt_len   = 0;

int compare_rotations(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    size_t n = g_bwt_len;

    for (size_t k = 0; k < n; k++) {
        unsigned char ca = g_bwt_input[(ia + k) % n];
        unsigned char cb = g_bwt_input[(ib + k) % n];
        if (ca < cb) return -1;
        if (ca > cb) return  1;
    }
    return 0;
}

/* ── bwt_encode ── */

void bwt_encode(unsigned char *input,  size_t len,
                unsigned char *output, int   *primary_index)
{
    if (len == 0) { *primary_index = 0; return; }

    /* Build index array [0, 1, 2, ..., n-1] */
    int *idx = (int *)malloc(len * sizeof(int));
    if (!idx) { fprintf(stderr, "[bwt] malloc failed\n"); return; }
    for (size_t i = 0; i < len; i++) idx[i] = (int)i;

    /* Sort using the global comparator */
    g_bwt_input = input;
    g_bwt_len   = len;
    qsort(idx, len, sizeof(int), compare_rotations);

    /* Extract last column and find primary index */
    *primary_index = 0;
    for (size_t i = 0; i < len; i++) {
        /* Last character of rotation starting at idx[i] */
        output[i] = input[(idx[i] + len - 1) % len];
        if (idx[i] == 0) *primary_index = (int)i;
    }

    free(idx);
}

/* ── bwt_decode ── */

/*
 * Inverse BWT – standard "counting sort" reconstruction.
 *
 * Given the last column L (= bwt output) and primary_index, we
 * reconstruct the original string in O(n) time and O(n) space.
 *
 * Algorithm:
 *   1. Build the first column F by sorting L.
 *   2. Compute the T-mapping: T[i] = the row j such that F[j] is the
 *      same occurrence of character L[i].
 *      In practice this is done with two counting arrays.
 *   3. Start at row `primary_index` and follow T exactly n times,
 *      reading F[current] each step → that gives the original string
 *      in reverse order → reverse at the end.
 */
void bwt_decode(unsigned char *input,  size_t len,
                int primary_index,     unsigned char *output)
{
    if (len == 0) return;

    /* Step 1 – count frequencies */
    int freq[256] = {0};
    for (size_t i = 0; i < len; i++) freq[(unsigned char)input[i]]++;

    /* Step 2 – cumulative counts (gives starting position in F for each char) */
    int start[256] = {0};
    for (int c = 1; c < 256; c++) start[c] = start[c-1] + freq[c-1];

    /*
     * Step 3 – build the T array.
     * T[i] = position in F that corresponds to L[i].
     * We process L left-to-right; for each character we use a running
     * counter to assign the k-th occurrence of that char to its correct
     * position in F (F is just L sorted).
     */
    int *T = (int *)malloc(len * sizeof(int));
    if (!T) { fprintf(stderr, "[bwt] malloc failed\n"); return; }

    int used[256] = {0};
    for (size_t i = 0; i < len; i++) {
        unsigned char c = input[i];
        T[i] = start[c] + used[c];
        used[c]++;
    }

    /*
     * Step 4 – follow the T-chain starting at primary_index.
     *
     * T[i] maps position i in L (= BWT output) to the position in F
     * (sorted first column) of the same occurrence of that character.
     *
     * Following the chain once gives us the PREVIOUS character in the
     * original string each time.  So we walk n steps and write from
     * the END of the output buffer backward.
     *
     * F[row] = the character at position `row` in the first column,
     * which equals input[row] sorted — but we can look it up cheaply:
     * we just need to know which symbol lives at position `row` in F.
     * That's encoded in the `start` and `freq` arrays already.
     */

    /* Build F as a simple sorted copy of L (O(n) via counting sort). */
    unsigned char *F = (unsigned char *)malloc(len);
    if (!F) { free(T); fprintf(stderr, "[bwt] malloc failed\n"); return; }
    {
        int p[256];
        for (int c = 0; c < 256; c++) p[c] = start[c];
        for (size_t i = 0; i < len; i++) {
            unsigned char c = input[i];
            F[p[c]++] = c;
        }
    }

    /*
     * Walk the chain:
     *   current row = primary_index
     *   output[n-1] = F[primary_index]   (last char of original)
     *   output[n-2] = F[T[primary_index]]
     *   ...
     */
    /*
     * L[primary_index]         = last char of the original string
     * L[T[primary_index]]      = second-to-last char
     * L[T[T[primary_index]]]   = third-to-last char  … and so on.
     *
     * So we read from `input` (which IS the L-column) at each row,
     * walking backwards through the output buffer.
     */
    int row = primary_index;
    for (int i = (int)len - 1; i >= 0; i--) {
        output[i] = input[row];   /* L[row] = the character at this step */
        row = T[row];
    }

    free(T);
}