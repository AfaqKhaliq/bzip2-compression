#include "block.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── divide_into_blocks ── */

BlockManager *divide_into_blocks(const char *filename, size_t block_size)
{
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("[block] fopen");
        return NULL;
    }

    /* Determine file size */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);

    if (file_size < 0) {
        perror("[block] ftell");
        fclose(f);
        return NULL;
    }

    /* How many blocks do we need? */
    int num_blocks = (int)((file_size + (long)block_size - 1) / block_size);
    if (num_blocks == 0) num_blocks = 1;   /* handle empty file gracefully */

    /* Allocate BlockManager */
    BlockManager *mgr = (BlockManager *)malloc(sizeof(BlockManager));
    if (!mgr) { fclose(f); return NULL; }

    mgr->blocks     = (Block *)calloc(num_blocks, sizeof(Block));
    mgr->num_blocks = num_blocks;
    mgr->block_size = block_size;

    if (!mgr->blocks) {
        free(mgr);
        fclose(f);
        return NULL;
    }

    /* Read each block */
    for (int i = 0; i < num_blocks; i++) {
        /* Last block may be smaller than block_size */
        size_t this_size = block_size;
        long remaining   = file_size - (long)i * (long)block_size;
        if ((long)this_size > remaining) this_size = (size_t)remaining;

        mgr->blocks[i].data = (unsigned char *)malloc(this_size);
        if (!mgr->blocks[i].data) {
            /* clean up what we already allocated */
            for (int j = 0; j < i; j++) free(mgr->blocks[j].data);
            free(mgr->blocks);
            free(mgr);
            fclose(f);
            return NULL;
        }

        size_t n_read = fread(mgr->blocks[i].data, 1, this_size, f);
        mgr->blocks[i].size          = n_read;
        mgr->blocks[i].original_size = n_read;
    }

    fclose(f);
    return mgr;
}

/* ── reassemble_blocks ── */

int reassemble_blocks(BlockManager *manager, const char *output_filename)
{
    if (!manager || !output_filename) return -1;

    FILE *f = fopen(output_filename, "wb");
    if (!f) {
        perror("[block] fopen output");
        return -1;
    }

    for (int i = 0; i < manager->num_blocks; i++) {
        Block *b = &manager->blocks[i];
        if (b->size == 0) continue;
        size_t written = fwrite(b->data, 1, b->size, f);
        if (written != b->size) {
            fprintf(stderr, "[block] write error on block %d\n", i);
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

/* ── free_block_manager ── */

void free_block_manager(BlockManager *manager)
{
    if (!manager) return;
    for (int i = 0; i < manager->num_blocks; i++) {
        free(manager->blocks[i].data);
    }
    free(manager->blocks);
    free(manager);
}
