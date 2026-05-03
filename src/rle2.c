#include "rle2.h"

#include <stddef.h>

void rle2_encode(unsigned char *input, size_t len,
                 unsigned char *output, size_t *out_len)
{
    size_t i = 0;
    size_t out = 0;

    while (i < len) {
        if (input[i] == 0) {
            size_t run = 1;
            while (i + run < len && input[i + run] == 0 && run < 255)
                run++;

            output[out++] = 0;
            output[out++] = (unsigned char)run;
            i += run;
        } else {
            output[out++] = input[i++];
        }
    }

    *out_len = out;
}

void rle2_decode(unsigned char *input, size_t len,
                 unsigned char *output, size_t *out_len)
{
    size_t i = 0;
    size_t out = 0;

    while (i < len) {
        unsigned char c = input[i++];
        if (c == 0) {
            if (i >= len) break;
            unsigned char run = input[i++];
            for (unsigned char k = 0; k < run; k++)
                output[out++] = 0;
        } else {
            output[out++] = c;
        }
    }

    *out_len = out;
}
