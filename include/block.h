#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>

/* ── Data Structures (as specified in the project) ── */

typedef struct {
    unsigned char *data;         /* Pointer to block data            */
    size_t         size;         /* Current size of block            */
    size_t         original_size;/* Original size before compression */
} Block;

typedef struct {
    Block  *blocks;     /* Array of blocks          */
    int     num_blocks; /* Number of blocks         */
    size_t  block_size; /* Configurable block size  */
} BlockManager;

/* ── Function Prototypes ── */

/*
 * Reads the input file and splits it into blocks of block_size bytes.
 * Returns a heap-allocated BlockManager; caller must free with free_block_manager().
 */
BlockManager *divide_into_blocks(const char *filename, size_t block_size);

/*
 * Writes all block data sequentially to output_filename.
 * Returns 0 on success, -1 on failure.
 */
int reassemble_blocks(BlockManager *manager, const char *output_filename);

/* Frees all memory owned by the BlockManager (including the manager itself). */
void free_block_manager(BlockManager *manager);

#endif /* BLOCK_H */
