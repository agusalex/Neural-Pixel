#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "constants.h"
#include "handle_cache.h"
#include "file_utils.h"
#include "str_utils.h"

int is_file_empty(const char *fn)
{
	FILE *f = fopen(fn, "r");
	if (f == NULL) {
		perror("Error opening file");
		return -1;
	}
	fseek(f, 0, SEEK_END);
	long fs = ftell(f);
	fclose(f);
	if (fs == 0) {
		return 1;
	}
	return 0;
}


int is_directory(const char *path)
{
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) return 0;
	return S_ISDIR(path_stat.st_mode);
}

int count_files(DIR* dir, const char* const* array)
{
	int nf = 0;
	if (dir != NULL && array == NULL) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) {
			if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && is_directory(entry->d_name) == 0) {
				nf++;
			}
		}
		rewinddir(dir);
	}
	if (dir == NULL && array != NULL) {
		while (*array != NULL) {
			array++;
			nf++;
		}
	}
	return nf;
}

int count_output_files()
{
	const char *outputs_path = "./outputs/";
	DIR* dir = opendir(outputs_path);
	if (dir == NULL) {
		return 0;
	}
	int nf = 0;
	
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && is_directory(entry->d_name) == 0) {
			nf++;
		}
	}
	closedir(dir);
	return nf;
}

int check_file_exists(char *fn, int tf)
{
	if (access(fn, F_OK) == 0) {
		if (tf == 1) {
			if (is_file_empty(fn) == 1) {
				create_cache(fn);
			}
		}
		return 1;
	} else {
		if (tf == 1) {
			create_cache(fn);
			return 1;
		}
		return 0;
	}
}

int has_files(const char *directory)
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir(directory);
	if (dir == NULL) {
		return 0;
	}
	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

DIR* check_create_dir(const char* path)
{
	DIR* dir = opendir(path);
	if (dir == NULL) {
		#ifdef _WIN32
			if (mkdir(path) != 0) {
				perror("Error creating directory");
				return NULL;
			}
			
			DIR* ndir = opendir(path);
			return ndir;
		#else
			if (mkdir(path, 0777) != 0) {
				perror("Error creating directory");
				return NULL;
			}
			
			DIR* ndir = opendir(path);
			return ndir;
		#endif
	} else {
		return dir;
	}
}

int check_create_base_dirs() {
	DIR* cache_dir = opendir(CACHE_PATH);
	if (cache_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(CACHE_PATH) != 0) {
				perror("Error creating required \".cache\" directory.");
				return 1;
			}
		#else
			if (mkdir(CACHE_PATH, 0777) != 0) {
				perror("Error creating required \".cache\" directory.");
				return 1;
			}
		#endif
	}
	closedir(cache_dir);
	
	DIR* models_dir = opendir(MODELS_PATH);
	if (models_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(MODELS_PATH) != 0) {
				perror("Error creating required \"models\" directory.");
				return 1;
			}
		#else
			if (mkdir(MODELS_PATH, 0777) != 0) {
				perror("Error creating required \"models\" directory.");
				return 1;
			}
		#endif
	}
	closedir(models_dir);
	
	DIR* outputs_dir = opendir(OUTPUTS_PATH);
	if (outputs_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(OUTPUTS_PATH) != 0) {
				perror("Error creating required \"outputs\" directory.");
				return 1;
			}
		#else
			if (mkdir(OUTPUTS_PATH, 0777) != 0) {
				perror("Error creating required \"outputs\" directory.");
				return 1;
			}
		#endif
	}
	closedir(outputs_dir);
	return 0;
}

GtkStringList* get_files(const char* path)
{
	DIR* dir = check_create_dir(path);
	if (dir == NULL) {
		GtkStringList *empty_list = gtk_string_list_new((const char*[]){"None", NULL});
		return empty_list;
	}

	int nf = 0;
	nf = count_files(dir, NULL) + 1;
	GtkStringList *files = gtk_string_list_new(NULL);
	char** sort_files = malloc(sizeof(char*) * (nf + 1));
	
	if (sort_files == NULL) {
		closedir(dir);
		GtkStringList *empty_list = gtk_string_list_new((const char*[]){"None", NULL});
		return empty_list;
	}

	int i = 0;
	struct dirent* entry;

	sort_files[0] = strdup("None");
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && is_directory(entry->d_name) == 0) {
			sort_files[i + 1] = strdup(entry->d_name); 
			i++;
		}
	}

	qsort(sort_files + 1, nf - 1, sizeof(const char *), compare_strings);

	for (int x = 0; x < nf; x++) {
		gtk_string_list_append(files, sort_files[x]);
		free(sort_files[x]);
	}
	
	free(sort_files);
	closedir(dir);

	return files;
}
