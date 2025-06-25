#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_list_contains_item(const char* const* list, const char* item)
{
	int i = 0;
	while (list[i] != NULL) {
		if (strcmp(list[i], item) == 0) {
			return i;
		}
		i++;
	}
	return 0;
}

int check_gtk_list_contains_item(GtkStringList *list, const char* item)
{
	guint i = 0;
	guint n = g_list_model_get_n_items((GListModel *)list);
	while(i < n) {
		const char *s = gtk_string_list_get_string(list, i);
		if (strcmp(s, item) == 0) {
			return i;
		}
		i++;
	}
	return 0;
}

int compare_strings(const void *a, const void *b)
{
	return strcasecmp(*(const char **)a, *(const char **)b);
}

int count_digits(double n)
{
	long long int i = (int)n;
	if (i < 0) {
		i = -i;
	}
	int l = 0;
	if (i == 0) {
		l = 1;
	} else {
		while (i > 0) {
			i /= 10;
			l++;
		}
	}
	return l;
}

char *convert_int_to_string(int x)
{
	char* str = malloc(12);

	if (str == NULL) {
	return NULL;
	}

	snprintf(str, 12, "%d", x);
	return str;
}

char *convert_double_to_string(double n, char *str_format)
{
	int str_size = 0;

	if (strcmp(str_format, "%.2f") == 0) {
		str_size = count_digits(n) + 4;
	}
	if (strcmp(str_format, "%.1f") == 0) {
		str_size = count_digits(n) + 3;
	}
	if (strcmp(str_format, "%.0f") == 0) {
		if (n < 0) {
			str_size = count_digits(n) + 2;
		} else {
			str_size = count_digits(n) + 1;
		}
	}

	char *str = malloc(str_size * sizeof(char));
	snprintf(str, str_size, str_format, n);
	return str;
}

char *format_lora_embedding_string(const gchar* item, int tb_type)
{
	if (item == NULL) return NULL;

	char *n_item = strdup(item); 
	if (n_item == NULL) return NULL;

	char *dot = strrchr(n_item, '.'); 
	if (dot != NULL) {
		*dot = '\0'; 
	}

	int t_len[2];

	if (tb_type == 0) {
		t_len[0] = 3;
		t_len[1] = 4;
	} else {
		t_len[0] = 7;
		t_len[1] = 5;
	}

	char text_pf[t_len[0]];
	char text_sf[t_len[1]];

	if (tb_type == 0) {
		strcpy(text_pf, ", (embedding:");
		strcpy(text_sf, ")");
	} else {
		strcpy(text_pf, "<lora:");
		strcpy(text_sf, ":1> ");
	}


	int f_len = strlen(n_item) + strlen(text_pf) + strlen(text_sf) + 1;
	char *f_str = malloc(f_len);
	if (f_str == NULL) {
		free(n_item);
		return NULL;
	}
	f_str[0] = '\0';
	strcat(f_str, text_pf);
	strcat(f_str, n_item);
	strcat(f_str, text_sf);
	f_str[f_len -1] = '\0';
	free(n_item);

	return f_str;
}

char *get_time_str()
{
	time_t rt;
	struct tm *timeinfo;
	time(&rt);
	timeinfo = localtime(&rt);
	char f_time[20];
	strftime(f_time, 20, "%d-%m-%Y_%H:%M:%S", timeinfo);
	char *t = malloc(strlen(f_time) + 1);
	strcpy(t, f_time);
	return t;
}

const char **trim_dropdown_strings(const char* const* items, int n_files, int str_size)
{
	const char **n_items = malloc(sizeof(char*) * (n_files + 1));
	if (n_items == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}

	for (int i = 0; items[i] != NULL; i++) {
		int str_len = strlen(items[i]);
		char *str = malloc(str_len + 1);
		if (str == NULL) {
			fprintf(stderr, "Memory allocation failed\n");
			exit(1);
		}
		if (str_len > str_size) {
			strncpy(str, items[i], str_size - 1);
			str[str_size - 1] = '\0';
			if (str_size > 3) {
				str[str_size - 3] = '.';
				str[str_size - 2] = '.';
			}
		} else {
			strncpy(str, items[i], str_len + 1);
			str[str_len] = '\0';
		}
		n_items[i] = strdup(str);
		free(str);
	}

	n_items[n_files] = NULL;
	return n_items;
}
