// main.c - Entry point for mangen
#include <stdio.h>
#include <string.h>
#include "walker.h"
#include "verify.h"
#include "hash.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

// Prints command-line usage help
void print_help() {
    printf("Usage: ./mangen [DIR_PATH] [OPTIONS]\n");
    printf("Generate manifest of directory files with hashes.\n\n");
    printf("Options:\n");
    printf("  -h              Show help message and exit.\n");
    printf("  -v              Show git commit hash and exit.\n");
    printf("  -e NAME         Exclude files or directories with the specified NAME.\n");
    printf("  -E PATTERN      Exclude files or directories matching the PATTERN (supports '*' and '.').\n");
    printf("  --verify FILE   Verify manifest file integrity.\n");
}

// Prints version information
void print_version() {
    printf("commit: %s\n", GIT_COMMIT_HASH);
}

int main(int argc, char *argv[]) {
    const char *dir_path = ".";  // Default path is current directory

    // Command-line argument parsing loop
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 < argc) {
                set_exclude_name(argv[++i]);
            } else {
                fprintf(stderr, "Option -e requires a file name argument\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-E") == 0) {
            if (i + 1 < argc) {
                set_exclude_pattern(argv[++i]);
            } else {
                fprintf(stderr, "Option -E requires a pattern argument\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--verify") == 0) {
            if (i + 1 < argc) {
                return verify_manifest(argv[++i]);
            } else {
                fprintf(stderr, "--verify requires a file name\n");
                return 1;
            }
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            // If it's a path and not an option, treat as target directory
            dir_path = argv[i];
        }
    }

    // Begin manifest generation
    process_directory(dir_path, "");

    // Print final checksum line
    printf("Manifest checksum: %08X\n", get_manifest_hash());
    return 0;
}
