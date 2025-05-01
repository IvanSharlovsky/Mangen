// mangen.c
// Directory manifest generator with integrity checking and verification
// Author: Ivan Sharlovskii

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

const char *exclude_name = NULL;       // File or directory name to exclude by -e
const char *exclude_pattern = NULL;    // Pattern to exclude by -E

uint32_t manifest_hash = 1;            // Global manifest checksum accumulator

// Computes a simple file hash by reading byte-by-byte and accumulating.
// This uses a simplified Adler-32-like method.
uint32_t simple_hash(const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror(filepath);
        return 0;
    }

    uint32_t hash = 1;
    int c;
    while ((c = fgetc(file)) != EOF) {
        hash = (hash + (uint8_t)c) * 65521 % 0xFFFFFFFF;
    }

    fclose(file);
    return hash;
}

// Updates the global manifest_hash based on a full line of output text.
// Called for every printed manifest line except checksum.
void update_manifest_hash(const char *line) {
    for (; *line; ++line) {
        manifest_hash = (manifest_hash + (uint8_t)(*line)) * 65521 % 0xFFFFFFFF;
    }
}

// Matches a filename against a pattern with '*' (wildcard) and '.' (any single character)
int match_pattern(const char *pattern, const char *str) {
    while (*pattern && *str) {
        if (*pattern == '*') {
            // '*' matches any number of characters, so we try both advancing pattern or str
            return match_pattern(pattern + 1, str) || match_pattern(pattern, str + 1);
        } else if (*pattern == '.' || *pattern == *str) {
            pattern++;
            str++;
        } else {
            return 0;
        }
    }

    // Consume remaining '*' if any
    while (*pattern == '*') pattern++;
    return *pattern == '\0' && *str == '\0';
}

// Recursively walks through a directory and prints the manifest for each file
void process_directory(const char *base_path, const char *rel_path) {
    char path[4096];
    if (snprintf(path, sizeof(path), "%s/%s", base_path, rel_path) >= (int)sizeof(path)) {
        fprintf(stderr, "Path too long: %s/%s\n", base_path, rel_path);
        return;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Check exclusion by name
        if (exclude_name && strcmp(entry->d_name, exclude_name) == 0) continue;

        // Check exclusion by pattern
        if (exclude_pattern && match_pattern(exclude_pattern, entry->d_name)) continue;

        char new_rel_path[4096];
        if (rel_path[0] != '\0') {
            if (snprintf(new_rel_path, sizeof(new_rel_path), "%s/%s", rel_path, entry->d_name) >= (int)sizeof(new_rel_path)) {
                fprintf(stderr, "Relative path too long: %s/%s\n", rel_path, entry->d_name);
                continue;
            }
        } else {
            if (snprintf(new_rel_path, sizeof(new_rel_path), "%s", entry->d_name) >= (int)sizeof(new_rel_path)) {
                fprintf(stderr, "Relative path too long: %s\n", entry->d_name);
                continue;
            }
        }

        char full_path[4096];
        if (snprintf(full_path, sizeof(full_path), "%s/%s", base_path, new_rel_path) >= (int)sizeof(full_path)) {
            fprintf(stderr, "Full path too long: %s/%s\n", base_path, new_rel_path);
            continue;
        }

        struct stat st;
        if (stat(full_path, &st) == -1) {
            perror(full_path);
            continue;
        }

        // If it's a directory — process recursively
        if (S_ISDIR(st.st_mode)) {
            process_directory(base_path, new_rel_path);
        }
        // If it's a regular file — print hash line
        else if (S_ISREG(st.st_mode)) {
            uint32_t hash = simple_hash(full_path);
            char output_line[8192];
            snprintf(output_line, sizeof(output_line), "%s : %08X\n", new_rel_path, hash);
            printf("%s", output_line);
            update_manifest_hash(output_line);
        }
    }

    closedir(dir);
}

// Verifies that a manifest file ends with correct checksum
int verify_manifest(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror(filename);
        return 1;
    }

    char line[8192];
    uint32_t parsed_hash = 0;
    manifest_hash = 1;
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

    if (parsed_hash == manifest_hash) {
        printf("Valid\n");
        return 0;
    } else {
        printf("Corrupted\n");
        return 1;
    }
}

// Prints command-line usage help
void print_help() {
    printf("Usage: ./mangen [DIR_PATH] [OPTIONS]\n");
    printf("Generate manifest of directory files with hashes.\n\n");
    printf("Options:\n");
    printf("  -h           Show help message and exit.\n");
    printf("  -v           Show git commit hash and exit.\n");
    printf("  -e NAME      Exclude files or directories with the specified NAME.\n");
    printf("  -E PATTERN   Exclude files or directories matching the PATTERN (supports '*' and '.').\n");
    printf("  --verify F   Verify manifest file integrity.\n");
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
                exclude_name = argv[++i];
            } else {
                fprintf(stderr, "Option -e requires a file name argument\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-E") == 0) {
            if (i + 1 < argc) {
                exclude_pattern = argv[++i];
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
    printf("Manifest checksum: %08X\n", manifest_hash);
    return 0;
}
