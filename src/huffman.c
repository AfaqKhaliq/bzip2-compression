#include "huffman.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    unsigned char symbol;
    unsigned char length;
} CodeLen;

static HuffmanNode *create_node(unsigned char symbol, int freq,
                                HuffmanNode *left, HuffmanNode *right)
{
    HuffmanNode *n = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    if (!n) return NULL;
    n->symbol = symbol;
    n->freq = freq;
    n->left = left;
    n->right = right;
    return n;
}

static void free_tree(HuffmanNode *root)
{
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

static void collect_lengths(HuffmanNode *root, unsigned char depth,
                            unsigned char *lengths)
{
    if (!root) return;
    if (!root->left && !root->right) {
        lengths[root->symbol] = depth ? depth : 1;
        return;
    }
    collect_lengths(root->left, (unsigned char)(depth + 1), lengths);
    collect_lengths(root->right, (unsigned char)(depth + 1), lengths);
}

void build_huffman_tree(int *frequencies, HuffmanNode **root)
{
    HuffmanNode *nodes[512];
    int count = 0;

    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            nodes[count++] = create_node((unsigned char)i, frequencies[i], NULL, NULL);
        }
    }

    if (count == 0) {
        *root = NULL;
        return;
    }

    if (count == 1) {
        *root = nodes[0];
        return;
    }

    while (count > 1) {
        int min1 = -1;
        int min2 = -1;
        for (int i = 0; i < count; i++) {
            if (min1 < 0 || nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1;
                min1 = i;
            } else if (min2 < 0 || nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        HuffmanNode *a = nodes[min1];
        HuffmanNode *b = nodes[min2];
        HuffmanNode *parent = create_node(0, a->freq + b->freq, a, b);

        if (min1 > min2) {
            int tmp = min1;
            min1 = min2;
            min2 = tmp;
        }

        nodes[min1] = parent;
        nodes[min2] = nodes[count - 1];
        count--;
    }

    *root = nodes[0];
}

static int compare_codelen(const void *a, const void *b)
{
    const CodeLen *ca = (const CodeLen *)a;
    const CodeLen *cb = (const CodeLen *)b;
    if (ca->length != cb->length) return (int)ca->length - (int)cb->length;
    return (int)ca->symbol - (int)cb->symbol;
}

void generate_canonical_codes(HuffmanNode *root, HuffmanCode *codes)
{
    unsigned char lengths[256];
    memset(lengths, 0, sizeof(lengths));

    if (root) collect_lengths(root, 0, lengths);

    CodeLen list[256];
    int count = 0;
    for (int i = 0; i < 256; i++) {
        if (lengths[i] > 0) {
            list[count].symbol = (unsigned char)i;
            list[count].length = lengths[i];
            count++;
        }
    }

    memset(codes, 0, sizeof(HuffmanCode) * 256);
    if (count == 0) return;

    qsort(list, count, sizeof(CodeLen), compare_codelen);

    unsigned int code = 0;
    unsigned char prev_len = list[0].length;
    for (int i = 0; i < count; i++) {
        unsigned char len = list[i].length;
        if (i == 0) {
            code = 0;
        } else if (len > prev_len) {
            code <<= (len - prev_len);
        }

        codes[list[i].symbol].code = code;
        codes[list[i].symbol].length = len;
        code++;
        prev_len = len;
    }
}

void write_header(HuffmanCode *codes, unsigned char *output,
                  size_t *out_len)
{
    size_t out = 0;
    for (int i = 0; i < 256; i++) {
        output[out++] = codes[i].length;
    }
    *out_len = out;
}

typedef struct {
    unsigned char *buf;
    size_t capacity;
    size_t byte_pos;
    int bit_pos;
} BitWriter;

static void bw_init(BitWriter *bw, unsigned char *buf, size_t cap)
{
    bw->buf = buf;
    bw->capacity = cap;
    bw->byte_pos = 0;
    bw->bit_pos = 0;
    if (cap > 0) bw->buf[0] = 0;
}

static void bw_write_bit(BitWriter *bw, int bit)
{
    if (bw->byte_pos >= bw->capacity) return;

    if (bit) bw->buf[bw->byte_pos] |= (unsigned char)(1 << (7 - bw->bit_pos));
    bw->bit_pos++;
    if (bw->bit_pos == 8) {
        bw->bit_pos = 0;
        bw->byte_pos++;
        if (bw->byte_pos < bw->capacity) bw->buf[bw->byte_pos] = 0;
    }
}

static void bw_write_bits(BitWriter *bw, unsigned int code, unsigned char len)
{
    for (int i = (int)len - 1; i >= 0; i--) {
        int bit = (code >> i) & 1U;
        bw_write_bit(bw, bit);
    }
}

static size_t bw_size(const BitWriter *bw)
{
    return bw->byte_pos + (bw->bit_pos > 0 ? 1 : 0);
}

void encode_data(unsigned char *input, size_t len,
                 HuffmanCode *codes, unsigned char *output,
                 size_t *out_len)
{
    BitWriter bw;
    bw_init(&bw, output, (size_t)(len * 5 + 512));

    for (size_t i = 0; i < len; i++) {
        HuffmanCode hc = codes[input[i]];
        bw_write_bits(&bw, hc.code, hc.length);
    }

    *out_len = bw_size(&bw);
}

typedef struct {
    const unsigned char *buf;
    size_t size;
    size_t byte_pos;
    int bit_pos;
} BitReader;

static void br_init(BitReader *br, const unsigned char *buf, size_t size)
{
    br->buf = buf;
    br->size = size;
    br->byte_pos = 0;
    br->bit_pos = 0;
}

static int br_read_bit(BitReader *br, int *bit)
{
    if (br->byte_pos >= br->size) return 0;
    *bit = (br->buf[br->byte_pos] >> (7 - br->bit_pos)) & 1U;
    br->bit_pos++;
    if (br->bit_pos == 8) {
        br->bit_pos = 0;
        br->byte_pos++;
    }
    return 1;
}

static HuffmanNode *build_decode_tree(const unsigned char *lengths)
{
    CodeLen list[256];
    int count = 0;
    for (int i = 0; i < 256; i++) {
        if (lengths[i] > 0) {
            list[count].symbol = (unsigned char)i;
            list[count].length = lengths[i];
            count++;
        }
    }

    if (count == 0) return NULL;

    qsort(list, count, sizeof(CodeLen), compare_codelen);

    HuffmanNode *root = create_node(0, 0, NULL, NULL);
    unsigned int code = 0;
    unsigned char prev_len = list[0].length;

    for (int i = 0; i < count; i++) {
        unsigned char len = list[i].length;
        if (i == 0) {
            code = 0;
        } else if (len > prev_len) {
            code <<= (len - prev_len);
        }

        HuffmanNode *cur = root;
        for (int bit = (int)len - 1; bit >= 0; bit--) {
            int b = (code >> bit) & 1U;
            if (b == 0) {
                if (!cur->left) cur->left = create_node(0, 0, NULL, NULL);
                cur = cur->left;
            } else {
                if (!cur->right) cur->right = create_node(0, 0, NULL, NULL);
                cur = cur->right;
            }
        }

        cur->symbol = list[i].symbol;
        code++;
        prev_len = len;
    }

    return root;
}

void huffman_encode(unsigned char *input, size_t len,
                    unsigned char *output, size_t *out_len)
{
    int freq[256] = {0};
    for (size_t i = 0; i < len; i++) freq[input[i]]++;

    HuffmanNode *root = NULL;
    build_huffman_tree(freq, &root);

    HuffmanCode codes[256];
    generate_canonical_codes(root, codes);

    size_t out = 0;

    unsigned int decoded_len = (unsigned int)len;
    output[out++] = (unsigned char)( decoded_len        & 0xFF);
    output[out++] = (unsigned char)((decoded_len >>  8) & 0xFF);
    output[out++] = (unsigned char)((decoded_len >> 16) & 0xFF);
    output[out++] = (unsigned char)((decoded_len >> 24) & 0xFF);

    size_t header_len = 0;
    write_header(codes, output + out, &header_len);
    out += header_len;

    size_t data_len = 0;
    encode_data(input, len, codes, output + out, &data_len);
    out += data_len;

    *out_len = out;
    free_tree(root);
}

void huffman_decode(unsigned char *input, size_t len,
                    unsigned char *output, size_t *out_len)
{
    if (len < 4 + 256) {
        *out_len = 0;
        return;
    }

    unsigned int decoded_len = (unsigned int)( input[0] |
                               (input[1] <<  8) |
                               (input[2] << 16) |
                               (input[3] << 24) );

    const unsigned char *lengths = input + 4;
    const unsigned char *stream = input + 4 + 256;
    size_t stream_len = len - 4 - 256;

    HuffmanNode *root = build_decode_tree(lengths);
    if (!root) {
        *out_len = 0;
        return;
    }

    BitReader br;
    br_init(&br, stream, stream_len);

    size_t out = 0;
    while (out < decoded_len) {
        HuffmanNode *cur = root;
        int bit = 0;
        while (cur->left || cur->right) {
            if (!br_read_bit(&br, &bit)) break;
            cur = bit == 0 ? cur->left : cur->right;
            if (!cur) break;
        }

        if (!cur) break;
        output[out++] = cur->symbol;
    }

    *out_len = out;
    free_tree(root);
}
