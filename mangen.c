// mangen.c
// Утилита генерации манифеста каталога с контролем целостности

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

const char *exclude_name = NULL; // Имя файла/каталога для исключения
const char *exclude_pattern = NULL; // Шаблон для исключения

uint32_t manifest_hash = 1; // Хэш для контроля целостности манифеста

// Вычисляет простой хэш-функцию для содержимого файла
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

// Обновляет хэш манифеста по строке
void update_manifest_hash(const char *line) {
    while (*line) {
        manifest_hash = (manifest_hash + (uint8_t)(*line)) * 65521 % 0xFFFFFFFF;
        line++;
    }
}

// Сопоставление строки с шаблоном
int match_pattern(const char *pattern, const char *str) {
    while (*pattern && *str) {
        if (*pattern == '*') {
            if (match_pattern(pattern + 1, str) || match_pattern(pattern, str + 1))
                return 1;
            else
                return 0;
        } else if (*pattern == '.' || *pattern == *str) {
            pattern++;
            str++;
        } else {
            return 0;
        }
    }

    while (*pattern == '*')
        pattern++;

    return *pattern == '\0' && *str == '\0';
}

// Рекурсивно обходит каталог и выводит пары <относительный путь> : <хэш>
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
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (exclude_name && strcmp(entry->d_name, exclude_name) == 0)
            continue;

        if (exclude_pattern && match_pattern(exclude_pattern, entry->d_name))
            continue;

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

        if (S_ISDIR(st.st_mode)) {
            process_directory(base_path, new_rel_path);
        } else if (S_ISREG(st.st_mode)) {
            uint32_t hash = simple_hash(full_path);
            char output_line[8192];
            snprintf(output_line, sizeof(output_line), "%s : %08X\n", new_rel_path, hash);
            printf("%s", output_line);
            update_manifest_hash(output_line);
        }
    }

    closedir(dir);
}

// Выводит справку об использовании утилиты
void print_help() {
    printf("Usage: ./mangen [DIR_PATH] [OPTIONS]\n");
    printf("Generate manifest of directory files with hashes.\n\n");
    printf("Options:\n");
    printf("  -h         Show help message and exit.\n");
    printf("  -v         Show git commit hash and exit.\n");
    printf("  -e NAME    Exclude files or directories with the specified NAME.\n");
    printf("  -E PATTERN Exclude files or directories matching the PATTERN (supports '*' and '.').\n");
}

// Выводит информацию о версии утилиты
void print_version() {
    printf("commit: %s\n", GIT_COMMIT_HASH);
}

int main(int argc, char *argv[]) {
    const char *dir_path = ".";

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
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            dir_path = argv[i];
        }
    }

    process_directory(dir_path, "");

    printf("Manifest checksum: %08X\n", manifest_hash);

    return 0;
}