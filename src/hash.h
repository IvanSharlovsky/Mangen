// hash.h - Header for hash.c
#ifndef HASH_H
#define HASH_H

#include <stdint.h>

uint32_t simple_hash(const char *filepath);
void update_manifest_hash(const char *line);
uint32_t get_manifest_hash();
void reset_manifest_hash();

#endif // HASH_H