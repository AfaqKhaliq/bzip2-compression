#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "block.h"
#include "rle.h"
#include "bwt.h"
#include "mtf.h"
#include "rle2.h"
#include "huffman.h"

/* ── helpers ── */

static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s encode <input_file> <output_file> [config.ini]\n", prog);
    printf("  %s decode <input_file> <output_file> [config.ini]\n", prog);
}

/*
 * Encode a single block through the pipeline:
 *   Block data → (optional) RLE-1 → BWT → (optional) MTF → (optional) RLE-2
 *   → (optional) Huffman
 *
 * Output format per block:
 *   [4 bytes: primary_index (little-endian int32)]
 *   [payload: stage2/3 output]
 */
static int encode_block(Block *block, const Config *cfg)
{
    unsigned char *buf   = block->data;
    size_t         size  = block->size;
    unsigned char *tmp   = NULL;
    size_t         tmp_len = 0;
    int            primary_index = 0;

    /* ── RLE-1 (optional) ── */
    if (cfg->rle1_enabled) {
        /* Worst-case encoded size: every byte emits 2 bytes → 2*size */
        tmp = (unsigned char *)malloc(size * 2 + 16);
        if (!tmp) return -1;
        rle1_encode(buf, size, tmp, &tmp_len);

        /* Swap buf/tmp so the next stage uses the encoded data */
        free(block->data);
        block->data = tmp;
        block->size = tmp_len;
        buf  = block->data;
        size = block->size;
        tmp  = NULL;
    }

    /* ── BWT ── */
    tmp = (unsigned char *)malloc(size);
    if (!tmp) return -1;

    bwt_encode(buf, size, tmp, &primary_index);

    free(block->data);
    block->data = tmp;
    block->size = size;
    buf  = block->data;
    size = block->size;
    tmp  = NULL;

    /* ── MTF (optional) ── */
    if (cfg->mtf_enabled) {
        tmp = (unsigned char *)malloc(size);
        if (!tmp) return -1;
        mtf_encode(buf, size, tmp);

        free(block->data);
        block->data = tmp;
        block->size = size;
        buf  = block->data;
        tmp  = NULL;
    }

    /* ── RLE-2 (optional) ── */
    if (cfg->rle2_enabled) {
        tmp = (unsigned char *)malloc(size * 2 + 16);
        if (!tmp) return -1;
        rle2_encode(buf, size, tmp, &tmp_len);

        free(block->data);
        block->data = tmp;
        block->size = tmp_len;
        buf  = block->data;
        size = block->size;
        tmp  = NULL;
    }

    /* ── Huffman (optional) ── */
    if (cfg->huffman_enabled) {
        size_t max_out = size * 5 + 4 + 256 + 512;
        tmp = (unsigned char *)malloc(max_out);
        if (!tmp) return -1;
        huffman_encode(buf, size, tmp, &tmp_len);

        free(block->data);
        block->data = tmp;
        block->size = tmp_len;
        buf  = block->data;
        size = block->size;
        tmp  = NULL;
    }

    /* ── Prepend primary_index ── */
    tmp = (unsigned char *)malloc(size + 4);
    if (!tmp) return -1;

    tmp[0] = (unsigned char)( primary_index        & 0xFF);
    tmp[1] = (unsigned char)((primary_index >>  8) & 0xFF);
    tmp[2] = (unsigned char)((primary_index >> 16) & 0xFF);
    tmp[3] = (unsigned char)((primary_index >> 24) & 0xFF);
    if (size > 0) memcpy(tmp + 4, buf, size);

    free(block->data);
    block->data = tmp;
    block->size = size + 4;

    return 0;
}

/*
 * Decode a single block through the inverse pipeline:
 *   (optional) Huffman → (optional) RLE-2 → (optional) MTF → BWT
 *   → (optional) RLE-1
 */
static int decode_block(Block *block, const Config *cfg)
{
    unsigned char *buf  = block->data;
    size_t         size = block->size;

    if (size < 4) return -1;

    /* ── Read primary_index ── */
    int primary_index = (int)( buf[0]        |
                               (buf[1] <<  8) |
                               (buf[2] << 16) |
                               (buf[3] << 24) );

    size_t payload_len = size - 4;
    unsigned char *payload = buf + 4;

    /* ── Huffman (optional) ── */
    if (cfg->huffman_enabled) {
        if (payload_len < 4 + 256) return -1;
        unsigned int decoded_len = (unsigned int)( payload[0] |
                                   (payload[1] <<  8) |
                                   (payload[2] << 16) |
                                   (payload[3] << 24) );
        unsigned char *tmp = (unsigned char *)malloc(decoded_len + 16);
        if (!tmp) return -1;

        size_t decoded_out = 0;
        huffman_decode(payload, payload_len, tmp, &decoded_out);

        free(block->data);
        block->data = tmp;
        block->size = decoded_out;
        buf  = block->data;
        size = block->size;
    } else {
        memmove(block->data, payload, payload_len);
        block->size = payload_len;
        buf  = block->data;
        size = block->size;
    }

    /* ── RLE-2 (optional) ── */
    if (cfg->rle2_enabled) {
        size_t decoded_max = 0;
        for (size_t i = 0; i < size; i++) {
            if (buf[i] == 0 && i + 1 < size) {
                decoded_max += buf[i + 1];
                i++;
            } else {
                decoded_max += 1;
            }
        }

        unsigned char *tmp = (unsigned char *)malloc(decoded_max + 16);
        if (!tmp) return -1;

        size_t decoded_len = 0;
        rle2_decode(buf, size, tmp, &decoded_len);

        free(block->data);
        block->data = tmp;
        block->size = decoded_len;
        buf  = block->data;
        size = block->size;
    }

    /* ── MTF (optional) ── */
    if (cfg->mtf_enabled) {
        unsigned char *tmp = (unsigned char *)malloc(size);
        if (!tmp) return -1;
        mtf_decode(buf, size, tmp);

        free(block->data);
        block->data = tmp;
        block->size = size;
        buf  = block->data;
    }

    /* ── Inverse BWT ── */
    unsigned char *tmp = (unsigned char *)malloc(size);
    if (!tmp) return -1;
    bwt_decode(buf, size, primary_index, tmp);

    /* Replace block data with BWT-decoded data */
    free(block->data);
    block->data = tmp;
    block->size = size;

    /* ── Inverse RLE-1 (optional) ── */
    if (cfg->rle1_enabled) {
        /* Decoded could be up to ~255x the encoded size in pathological
           cases, but in practice ≤ original_size.  We use 4× as a safe bound. */
        size_t decoded_max = (block->size * 259 > cfg->block_size ? block->size * 259 : cfg->block_size) + 16;
        tmp = (unsigned char *)malloc(decoded_max);
        if (!tmp) return -1;

        size_t decoded_len = 0;
        rle1_decode(block->data, block->size, tmp, &decoded_len);

        free(block->data);
        block->data = tmp;
        block->size = decoded_len;
    }

    return 0;
}

/* ── main ── */

int main(int argc, char *argv[])
{
    if (argc < 4) { print_usage(argv[0]); return 1; }

    const char *mode        = argv[1];
    const char *input_file  = argv[2];
    const char *output_file = argv[3];
    const char *config_file = (argc >= 5) ? argv[4] : "config.ini";

    /* Load configuration */
    Config cfg;
    config_load(config_file, &cfg);
    config_print(&cfg);

    /* Read input file into blocks */
    BlockManager *mgr = divide_into_blocks(input_file, cfg.block_size);
    if (!mgr) {
        fprintf(stderr, "Error: could not read input file '%s'\n", input_file);
        return 1;
    }

    printf("Read %d block(s) from '%s'\n", mgr->num_blocks, input_file);

    int ok = 0;

    if (strcmp(mode, "encode") == 0) {
        for (int i = 0; i < mgr->num_blocks; i++) {
            printf("  Encoding block %d/%d (%zu bytes)...\n",
                   i+1, mgr->num_blocks, mgr->blocks[i].size);
            if (encode_block(&mgr->blocks[i], &cfg) != 0) {
                fprintf(stderr, "  Error encoding block %d\n", i);
                ok = 1;
            }
        }
    } else if (strcmp(mode, "decode") == 0) {
        for (int i = 0; i < mgr->num_blocks; i++) {
            printf("  Decoding block %d/%d (%zu bytes)...\n",
                   i+1, mgr->num_blocks, mgr->blocks[i].size);
            if (decode_block(&mgr->blocks[i], &cfg) != 0) {
                fprintf(stderr, "  Error decoding block %d\n", i);
                ok = 1;
            }
        }
    } else {
        fprintf(stderr, "Unknown mode '%s'\n", mode);
        print_usage(argv[0]);
        free_block_manager(mgr);
        return 1;
    }

    if (ok == 0) {
        if (reassemble_blocks(mgr, output_file) != 0) {
            fprintf(stderr, "Error writing output file\n");
            ok = 1;
        } else {
            printf("Output written to '%s'\n", output_file);
        }
    }

    free_block_manager(mgr);
    return ok;
}
