#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
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
	int nf = 1;
	
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && is_directory(entry->d_name) == 0) {
			nf++;
		}
	}
	rewinddir(dir);
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
			if (mkdir("models") != 0) {
				perror("Error creating directory");
			} else {
				if (mkdir("outputs") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/checkpoints") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/clips") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/controlnet") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/embeddings") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/loras") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/text_encoders") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/unet") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/upscale_models") != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/vae") != 0) {
					perror("Error creating directory");
				}
				closedir(dir);
				DIR* ndir = opendir(path);
				return ndir;
			}
		#else
			if (mkdir("models", 0777) != 0) {
				perror("Error creating directory");
			} else {
				if (mkdir("outputs", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/checkpoints", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/clips", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/controlnet", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/embeddings", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/loras", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/text_encoders", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/unet", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/upscale_models", 0777) != 0) {
					perror("Error creating directory");
				}
				if (mkdir("models/vae", 0777) != 0) {
					perror("Error creating directory");
				}
				closedir(dir);
				DIR* ndir = opendir(path);
				return ndir;
			}
		#endif
	} else {
		return dir;
	}
}

const char** get_files(const char* path)
{
	DIR* dir = check_create_dir(path);

	int nf = 0;
	nf = count_files(dir, NULL) + 1;
	const char** files = malloc(sizeof(char*) * (nf + 1));
	char** sort_files = malloc(sizeof(char*) * (nf + 1));
	if (files == NULL) {
		closedir(dir);
		return NULL;
	}
	if (sort_files == NULL) {
		closedir(dir);
		return NULL;
	}

	int i = 0;
	struct dirent* entry;

	sort_files[i] = "None";
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && is_directory(entry->d_name) == 0) {
			sort_files[i + 1] = strdup(entry->d_name); 
			i++;
		}
	}

	qsort(sort_files + 1, nf - 1, sizeof(const char *), compare_strings);

	for (int i = 0; i < nf; i++) {
		files[i] = malloc((strlen(sort_files[i]) + 1) * sizeof(char));
		if (files[i] == NULL) {
			printf("Memory allocation failed!\n");
			exit(1);
		}
		strcpy((char*)files[i], sort_files[i]);
	}

	free(sort_files);
	files[nf] = NULL;
	closedir(dir);

	return files;
}
