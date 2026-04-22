#include "rle.h"

#include <stddef.h>

/*
 * BZip2's actual RLE-1 scheme
 * ───────────────────────────
 * We scan the input looking for runs of the SAME byte.
 *
 * Rules:
 *   • A run shorter than 4 bytes → copy bytes verbatim.
 *   • A run of 4 or more bytes   → emit exactly 4 copies of the byte,
 *     then emit ONE extra byte whose value is (run_length - 4).
 *     Because a single byte can hold 0–255, the maximum run we can encode
 *     in one shot is 4 + 255 = 259 bytes.  Longer runs are split.
 *
 * Example (from the spec's spirit, adapted to actual BZip2 RLE-1):
 *   Input : A A A A A A   (6 identical bytes)
 *   Output: A A A A \x02  (4 copies + count byte 2 meaning 4+2=6)
 *
 *   Input : A A A         (3 identical bytes, run < 4)
 *   Output: A A A         (copied verbatim)
 */

void rle1_encode(unsigned char *input,  size_t  len,
                 unsigned char *output, size_t *out_len)
{
    size_t i   = 0;
    size_t out = 0;

    while (i < len) {
        unsigned char c  = input[i];
        size_t        run = 1;

        /* Count how many identical bytes follow (up to 259) */
        while (i + run < len && input[i + run] == c && run < 259)
            run++;

        if (run < 4) {
            /* Short run – copy verbatim */
            for (size_t k = 0; k < run; k++)
                output[out++] = c;
        } else {
            /* Long run – emit 4 copies then a count byte */
            output[out++] = c;
            output[out++] = c;
            output[out++] = c;
            output[out++] = c;
            output[out++] = (unsigned char)(run - 4);   /* 0 means run of 4 */
        }

        i += run;
    }

    *out_len = out;
}


/*
 * Decoder – exact inverse of rle1_encode.
 *
 * We track how many identical bytes we have written consecutively.
 * The moment we have emitted 4 of the same byte in a row, the very
 * next byte in the stream is a count byte, not data.
 */
void rle1_decode(unsigned char *input,  size_t  len,
                 unsigned char *output, size_t *out_len)
{
    size_t        i         = 0;
    size_t        out       = 0;
    int           run_count = 0;   /* consecutive identical bytes written so far */
    unsigned char prev      = 0;   /* the last byte we wrote                     */

    while (i < len) {
        unsigned char c = input[i++];

        if (run_count == 4) {
            /*
             * The encoder emitted 4 identical bytes and then this count byte.
             * Write `c` more copies of `prev` (0 means no extra copies).
             */
            for (int k = 0; k < (int)c; k++)
                output[out++] = prev;

            /* After consuming the count byte the run is fully closed. */
            run_count = 0;
            /* prev keeps its value; the NEXT byte will be evaluated fresh. */
        } else {
            /* Normal data byte */
            output[out++] = c;

            if (run_count > 0 && c == prev) {
                run_count++;
            } else {
                run_count = 1;
            }
            prev = c;
        }
    }

    *out_len = out;
}
