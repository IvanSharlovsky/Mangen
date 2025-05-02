// walker.h - Header for walker.c
#ifndef WALKER_H
#define WALKER_H

void process_directory(const char *base_path, const char *rel_path);
void set_exclude_name(const char *name);
void set_exclude_pattern(const char *pattern);

#endif // WALKER_H
