#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── helpers ── */

/* Strip leading and trailing whitespace in-place. */
static void trim(char *s)
{
    /* leading */
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    /* trailing */
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) s[--n] = '\0';
}

/* Remove everything from '#' onward (inline comments). */
static void strip_comment(char *s)
{
    char *p = strchr(s, '#');
    if (p) *p = '\0';
}

static bool parse_bool(const char *val)
{
    return (strcmp(val, "true") == 0 || strcmp(val, "1") == 0 ||
            strcmp(val, "yes")  == 0);
}

/* Fill cfg with safe defaults. */
static void set_defaults(Config *cfg)
{
    cfg->block_size       = 500000;   /* 500 KB */
    cfg->rle1_enabled     = true;
    strncpy(cfg->bwt_type, "matrix", sizeof(cfg->bwt_type));
    cfg->mtf_enabled      = true;
    cfg->rle2_enabled     = true;
    cfg->huffman_enabled  = true;
    cfg->benchmark_mode   = false;
    cfg->output_metrics   = true;
    strncpy(cfg->input_directory,  "./benchmarks/", sizeof(cfg->input_directory));
    strncpy(cfg->output_directory, "./results/",    sizeof(cfg->output_directory));
}

/* ── public API ── */

int config_load(const char *filename, Config *cfg)
{
    set_defaults(cfg);

    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "[config] Warning: could not open '%s', using defaults.\n",
                filename);
        return -1;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        strip_comment(line);
        trim(line);

        /* Skip blank lines and section headers */
        if (line[0] == '\0' || line[0] == '[') continue;

        /* Split on '=' */
        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;
        trim(key);
        trim(val);

        if      (strcmp(key, "block_size")      == 0) cfg->block_size      = (size_t)atol(val);
        else if (strcmp(key, "rle1_enabled")    == 0) cfg->rle1_enabled    = parse_bool(val);
        else if (strcmp(key, "bwt_type")        == 0) strncpy(cfg->bwt_type, val, sizeof(cfg->bwt_type)-1);
        else if (strcmp(key, "mtf_enabled")     == 0) cfg->mtf_enabled     = parse_bool(val);
        else if (strcmp(key, "rle2_enabled")    == 0) cfg->rle2_enabled    = parse_bool(val);
        else if (strcmp(key, "huffman_enabled") == 0) cfg->huffman_enabled = parse_bool(val);
        else if (strcmp(key, "benchmark_mode")  == 0) cfg->benchmark_mode  = parse_bool(val);
        else if (strcmp(key, "output_metrics")  == 0) cfg->output_metrics  = parse_bool(val);
        else if (strcmp(key, "input_directory") == 0) strncpy(cfg->input_directory,  val, sizeof(cfg->input_directory)-1);
        else if (strcmp(key, "output_directory")== 0) strncpy(cfg->output_directory, val, sizeof(cfg->output_directory)-1);
    }

    /* Clamp block_size to the allowed range: 100 KB – 900 KB */
    if (cfg->block_size < 100000)  cfg->block_size = 100000;
    if (cfg->block_size > 900000)  cfg->block_size = 900000;

    fclose(f);
    return 0;
}

void config_print(const Config *cfg)
{
    printf("[Config]\n");
    printf("  block_size       = %zu\n",  cfg->block_size);
    printf("  rle1_enabled     = %s\n",   cfg->rle1_enabled    ? "true" : "false");
    printf("  bwt_type         = %s\n",   cfg->bwt_type);
    printf("  mtf_enabled      = %s\n",   cfg->mtf_enabled     ? "true" : "false");
    printf("  rle2_enabled     = %s\n",   cfg->rle2_enabled    ? "true" : "false");
    printf("  huffman_enabled  = %s\n",   cfg->huffman_enabled ? "true" : "false");
    printf("  benchmark_mode   = %s\n",   cfg->benchmark_mode  ? "true" : "false");
    printf("  output_metrics   = %s\n",   cfg->output_metrics  ? "true" : "false");
    printf("  input_directory  = %s\n",   cfg->input_directory);
    printf("  output_directory = %s\n",   cfg->output_directory);
}
