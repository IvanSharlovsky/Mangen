// verify.c - Implements --verify logic for mangen
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "hash.h"

// Verifies that a manifest file ends with correct checksum
int verify_manifest(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror(filename);
        return 1;
    }

    char line[8192];
    uint32_t parsed_hash = 0;
    reset_manifest_hash();
    int checksum_found = 0;

    while (fgets(line, sizeof(line), file)) {
        // If we detect checksum line, extract value and skip hashing
        if (!checksum_found && sscanf(line, "Manifest checksum: %X", &parsed_hash) == 1) {
            checksum_found = 1;
            continue;
        }
        update_manifest_hash(line);
    }

    fclose(file);

    if (!checksum_found) {
        fprintf(stderr, "Invalid or missing checksum line\n");
        return 1;
    }

    if (parsed_hash == get_manifest_hash()) {
        printf("Valid\n");
        return 0;
    } else {
        printf("Corrupted\n");
        return 1;
    }
}
