#include "mtf.h"

#include <string.h>

void mtf_encode(unsigned char *input, size_t len,
                unsigned char *output)
{
    unsigned char list[256];
    for (int i = 0; i < 256; i++) list[i] = (unsigned char)i;

    for (size_t i = 0; i < len; i++) {
        unsigned char sym = input[i];
        int idx = 0;
        while (list[idx] != sym && idx < 256) idx++;

        output[i] = (unsigned char)idx;

        if (idx > 0) {
            memmove(&list[1], &list[0], (size_t)idx);
            list[0] = sym;
        }
    }
}

void mtf_decode(unsigned char *input, size_t len,
                unsigned char *output)
{
    unsigned char list[256];
    for (int i = 0; i < 256; i++) list[i] = (unsigned char)i;

    for (size_t i = 0; i < len; i++) {
        unsigned char idx = input[i];
        unsigned char sym = list[idx];
        output[i] = sym;

        if (idx > 0) {
            memmove(&list[1], &list[0], idx);
            list[0] = sym;
        }
    }
}
