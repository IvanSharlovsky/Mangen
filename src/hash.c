// hash.c - Implements file and manifest hash functions with internal checksum tracking
#include <stdio.h>
#include <stdint.h>
#include "hash.h"

// Internal variable that accumulates the checksum of all manifest lines
static uint32_t manifest_hash = 1;

// Computes a simple hash for a file by reading its contents byte-by-byte.
// The algorithm is inspired by a simplified Adler-32 variant.
// Returns 0 on failure, non-zero 32-bit hash otherwise.
uint32_t simple_hash(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        // Print error if file cannot be opened
        perror(filepath);
        return 0;
    }

    uint32_t hash = 1;
    int c;
    // Read each byte of the file and update the hash
    while ((c = fgetc(file)) != EOF) {
        hash = (hash + (uint8_t)c) * 65521 % 0xFFFFFFFF;
    }

    fclose(file);
    return hash;
}

// Updates the manifest hash by iterating through each character of a line.
// Called for every line added to the manifest except the checksum line.
void update_manifest_hash(const char *line) {
    for (; *line; ++line) {
        manifest_hash = (manifest_hash + (uint8_t)(*line)) * 65521 % 0xFFFFFFFF;
    }
}

// Returns the current accumulated manifest hash value.
uint32_t get_manifest_hash() {
    return manifest_hash;
}

// Resets the internal manifest hash accumulator to its initial state.
void reset_manifest_hash() {
    manifest_hash = 1;
}
