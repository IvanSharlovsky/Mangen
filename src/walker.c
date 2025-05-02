// walker.c - Handles directory traversal and filtering logic
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "hash.h"
#include "walker.h"

// Static variables to store exclusion filters
static const char *exclude_name = NULL;
static const char *exclude_pattern = NULL;

// Set the name to exclude (exact match)
void set_exclude_name(const char *name) {
    exclude_name = name;
}

// Set the wildcard pattern to exclude (e.g., *.sh)
void set_exclude_pattern(const char *pattern) {
    exclude_pattern = pattern;
}

// Matches a filename against a pattern supporting:
// '*' = any sequence of characters, '.' = any single character
int match_pattern(const char *pattern, const char *str) {
    while (*pattern && *str) {
        if (*pattern == '*') {
            // Try both: advancing pattern or advancing string
            return match_pattern(pattern + 1, str) || match_pattern(pattern, str + 1);
        } else if (*pattern == '.' || *pattern == *str) {
            pattern++;
            str++;
        } else {
            return 0;
        }
    }
    // Skip trailing '*' in pattern
    while (*pattern == '*') pattern++;
    return *pattern == '\0' && *str == '\0';
}

// Recursively processes a directory and computes hash lines
void process_directory(const char *base_path, const char *rel_path) {
    char path[4096];
    // Build full path from base and relative components
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
        // Skip current and parent directory entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Apply exact name filter
        if (exclude_name && strcmp(entry->d_name, exclude_name) == 0) continue;

        // Apply pattern filter
        if (exclude_pattern && match_pattern(exclude_pattern, entry->d_name)) continue;

        char new_rel_path[4096];
        // Build new relative path depending on whether we're at the root
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
        // Build full path to the file or directory
        if (snprintf(full_path, sizeof(full_path), "%s/%s", base_path, new_rel_path) >= (int)sizeof(full_path)) {
            fprintf(stderr, "Full path too long: %s/%s\n", base_path, new_rel_path);
            continue;
        }

        struct stat st;
        // Get file or directory info
        if (stat(full_path, &st) == -1) {
            perror(full_path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            // Recursively process subdirectory
            process_directory(base_path, new_rel_path);
        } else if (S_ISREG(st.st_mode)) {
            // Process regular file: compute hash and output line
            uint32_t hash = simple_hash(full_path);
            char output_line[8192];
            snprintf(output_line, sizeof(output_line), "%s : %08X\n", new_rel_path, hash);
            printf("%s", output_line);
            update_manifest_hash(output_line); // Include in manifest checksum
        }
    }

    closedir(dir);
}
