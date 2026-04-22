#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    size_t block_size;       /* bytes, 100KB–900KB */
    bool   rle1_enabled;
    char   bwt_type[32];     /* "matrix" or "suffix_array" */
    bool   mtf_enabled;
    bool   rle2_enabled;
    bool   huffman_enabled;

    bool   benchmark_mode;
    bool   output_metrics;

    char   input_directory[256];
    char   output_directory[256];
} Config;

/*
 * Loads config from the given .ini file path.
 * Returns 0 on success, -1 on failure.
 * On failure the struct is filled with safe defaults.
 */
int  config_load(const char *filename, Config *cfg);

/* Print the currently loaded config (useful for debugging). */
void config_print(const Config *cfg);

#endif /* CONFIG_H */
