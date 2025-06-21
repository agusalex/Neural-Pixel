#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

int is_file_empty(const char *fn);

int is_directory(const char *path);

int count_files(DIR* dir, const char* const* array);

int count_output_files();

int check_file_exists(char *fn, int tf);

int has_files(const char *directory);

DIR* check_create_dir(const char* path);

int check_create_base_dirs();

GtkStringList* get_files(const char* path);

#endif // FILE_UTILS_H
