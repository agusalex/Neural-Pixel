#include <stdio.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "file_utils.h"
#include "structs.h"
#include "constants.h"
#include "str_utils.h"
#include "widgets_cb.h"

void create_cache(char *n)
{
	DIR* cd = opendir(".cache");
	if (cd == NULL) {
		#ifdef _WIN32
			if (mkdir(".cache") != 0) {
				perror("Error creating directory");
			}
		#else
			if (mkdir(".cache", 0777) != 0) {
				perror("Error creating directory");
			}
		#endif
	}
	closedir(cd);

	if (strcmp(n, ".cache/pp_cache") == 0) {
		FILE *pcf = fopen(".cache/pp_cache", "wb");
		if (pcf == NULL) {
			perror("fopen");
			exit(1);
		}
		fprintf(pcf, "%s", POSITIVE_PROMPT);
		fclose(pcf);
	}

	if (strcmp(n, ".cache/np_cache") == 0) {
		FILE *ncf = fopen(".cache/np_cache", "wb");
		if (ncf == NULL) {
			perror("fopen");
			exit(1);
		}
		fprintf(ncf, "%s", NEGATIVE_PROMPT);
		fclose(ncf);
	}
	
	if (strcmp(n, ".cache/img_cache") == 0) {
		FILE *imgcf = fopen(".cache/img_cache", "wb");
		if (imgcf == NULL) {
			perror("fopen");
			exit(1);
		}
		fprintf(imgcf, "%s", DEFAULT_IMG_PATH);
		fclose(imgcf);
	}

	if (strcmp(n, ".cache/sd_cache") == 0) {
		FILE *cf = fopen(".cache/sd_cache", "wb");
		if (cf == NULL) {
			perror("fopen");
			exit(1);
		}
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "%d\n", DEFAULT_SAMPLE);
		fprintf(cf, "%d\n", DEFAULT_SCHEDULE);
		fprintf(cf, "%d\n", DEFAULT_N_STEPS);
		fprintf(cf, "%d\n", DEFAULT_SIZE);
		fprintf(cf, "%d\n", DEFAULT_SIZE);
		fprintf(cf, "%d\n", DEFAULT_BATCH_SIZE);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%d\n", DEFAULT_OPT_VRAM);
		fprintf(cf, "%.1f\n", DEFAULT_CFG);
		fprintf(cf, "%.2f\n", DEFAULT_DENOISE);
		fprintf(cf, "%.1f\n", DEFAULT_SEED);
		fprintf(cf, "%.1f\n", DEFAULT_RP_UPSCALE);
		fclose(cf);
	}
}

void load_pp_cache(GtkTextBuffer *pos_tb)
{
	if (check_file_exists(".cache/pp_cache", 1) == 1) {
		FILE *pcf = fopen(".cache/pp_cache", "r");
		if (pcf == NULL) {
			perror("Failed to open file");
			exit(1);
		}
		char line[512];
		int i = 0;
		
		fseek(pcf, 0, SEEK_END);
		long pcf_size = ftell(pcf);
		fseek(pcf, 0, SEEK_SET);
		char *pb = (char *)malloc(pcf_size + 1);
		if (pb == NULL) {
			perror("Memory allocation failed");
			fclose(pcf);
			exit(1);
		}
		size_t pbr = fread(pb, 1, pcf_size, pcf);
		if (pbr != pcf_size) {
			perror("Error reading the file");
			free(pb);
			fclose(pcf);
			exit(1);
		}
		pb[pbr] = '\0';

		gtk_text_buffer_set_text (pos_tb, pb, -1);

		free(pb);
		fclose(pcf);
	} else {
		gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);
	}
}

void load_np_cache(GtkTextBuffer *neg_tb)
{
	if (check_file_exists(".cache/np_cache", 1) == 1) {
		FILE *ncf = fopen(".cache/np_cache", "r");
		if (ncf == NULL) {
			perror("Failed to open file");
			exit(1);
		}
		char line[512];
		int i = 0;
		
		fseek(ncf, 0, SEEK_END);
		long ncf_size = ftell(ncf);
		fseek(ncf, 0, SEEK_SET);
		char *nb = (char *)malloc(ncf_size + 1);
		if (nb == NULL) {
			perror("Memory allocation failed");
			fclose(ncf);
			exit(1);
		}
		size_t nbr = fread(nb, 1, ncf_size, ncf);
		if (nbr != ncf_size) {
			perror("Error reading the file");
			free(nb);
			fclose(ncf);
			exit(1);
		}
		nb[nbr] = '\0';

		gtk_text_buffer_set_text (neg_tb, nb, -1);

		free(nb);
		fclose(ncf);
	} else {
		gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);
	}
}

GtkWidget* load_img_cache(GtkWidget *img_wgt)
{
	if (check_file_exists(".cache/img_cache", 1) == 1) {
		FILE *imgf = fopen(".cache/img_cache", "r");
		if (imgf == NULL) {
			perror("Failed to open file");
			exit(1);
		}
		char line[128];
		int i = 0;
		
		while (fgets(line, sizeof(line), imgf) != NULL) {
			line[strcspn(line, "\n")] = '\0';
			if (i > 1) break;
			switch(i) {
				case 0: gtk_image_set_from_file(GTK_IMAGE(img_wgt), line); break;
				
				default: break;
			}
			i++;
		}
		fclose(imgf);
	} else {
		gtk_image_set_from_file(GTK_IMAGE(img_wgt), DEFAULT_IMG_PATH);
	}
}


void load_cache(gpointer user_data)
{
	if (check_file_exists(".cache/sd_cache", 1) == 1) {
		FILE *cf = fopen(".cache/sd_cache", "r");
		if (cf == NULL) {
			perror("Failed to open file");
			exit(1);
		}
		char line[128];
		int i = 0;
		
		AppStartData *data = user_data;

		while (fgets(line, sizeof(line), cf) != NULL) {
			line[strcspn(line, "\n")] = '\0';
			if (i > 24) break;
			switch(i) {
				case 0: g_string_assign(data->model_string, line); break;

				case 1: g_string_assign(data->vae_string, line); break;

				case 2: g_string_assign(data->cnet_string, line); break;

				case 3: g_string_assign(data->upscale_string, line); break;
				
				case 4: g_string_assign(data->clip_l_string, line); break;
				
				case 5: g_string_assign(data->clip_g_string, line); break;
				
				case 6: g_string_assign(data->t5xxl_string, line); break;

				case 7: sscanf(line, "%d", &data->sample_index); break;
				
				case 8: sscanf(line, "%d", &data->schedule_index); break;
				
				case 9: sscanf(line, "%d", &data->n_steps_index); break;
				
				case 10: sscanf(line, "%d", &data->w_index); break;
				
				case 11: sscanf(line, "%d", &data->h_index); break;
				
				case 12: sscanf(line, "%d", &data->bs_index); break;
				
				case 13: sscanf(line, "%d", &data->cpu_bool); break;
				
				case 14: sscanf(line, "%d", &data->vt_bool); break;
				
				case 15: sscanf(line, "%d", &data->k_clip_bool); break;
				
				case 16: sscanf(line, "%d", &data->k_cnet_bool); break;
				
				case 17: sscanf(line, "%d", &data->k_vae_bool); break;
				
				case 18: sscanf(line, "%d", &data->fa_bool); break;
				
				case 19: sscanf(line, "%d", &data->taesd_bool); break;
				
				case 20: sscanf(line, "%d", &data->verbose_bool); break;
				
				case 21: char *endptr1; data->cfg_value = strtod(line, &endptr1); break;
				
				case 22: char *endptr2; data->denoise_value = strtod(line, &endptr2); break;
				
				case 23: char *endptr3; data->seed_value = strtod(line, &endptr3); break;
				
				case 24: char *endptr4; data->up_repeat_value = strtod(line, &endptr4); break;
				
				default: break;
			}
			i++;
		}
		fclose(cf);
	}
}

void update_cache(GenerationData *data, gchar *sel_model, gchar *sel_vae, gchar *sel_cnet, gchar *sel_upscale, gchar *sel_clip_l, gchar *sel_clip_g, gchar *sel_t5xxl, char *pp, char *np, char *img_num)
{
	FILE *pcf = fopen(".cache/pp_cache", "wb");
	FILE *ncf = fopen(".cache/np_cache", "wb");
	FILE *imgcf = fopen(".cache/img_cache", "wb");
	FILE *cf = fopen(".cache/sd_cache", "wb");
	if (pcf == NULL || ncf == NULL || imgcf == NULL || cf == NULL) {
		perror("fopen");
		exit(1);
	}

	fprintf(pcf, "%s", pp);
	fclose(pcf);

	fprintf(ncf, "%s", np);
	fclose(ncf);

	fprintf(imgcf, "./outputs/IMG_%s.png\n", img_num);
	fclose(imgcf);

	fprintf(cf, "%s\n", sel_model);
	fprintf(cf, "%s\n", sel_vae);
	fprintf(cf, "%s\n", sel_cnet);
	fprintf(cf, "%s\n", sel_upscale);
	fprintf(cf, "%s\n", sel_clip_l);
	fprintf(cf, "%s\n", sel_clip_g);
	fprintf(cf, "%s\n", sel_t5xxl);
	fprintf(cf, "%d\n", *data->sample_index);
	fprintf(cf, "%d\n", *data->schedule_index);
	fprintf(cf, "%d\n", *data->n_steps_index);
	fprintf(cf, "%d\n", *data->w_index);
	fprintf(cf, "%d\n", *data->h_index);
	fprintf(cf, "%d\n", *data->bs_index);
	fprintf(cf, "%d\n", *data->cpu_bool);
	fprintf(cf, "%d\n", *data->vt_bool);
	fprintf(cf, "%d\n", *data->k_clip_bool);
	fprintf(cf, "%d\n", *data->k_cnet_bool);
	fprintf(cf, "%d\n", *data->k_vae_bool);
	fprintf(cf, "%d\n", *data->fa_bool);
	fprintf(cf, "%d\n", *data->taesd_bool);
	fprintf(cf, "%d\n", *data->verbose_bool);
	fprintf(cf, "%.1f\n", *data->cfg_value);
	fprintf(cf, "%.2f\n", *data->denoise_value);
	fprintf(cf, "%.1f\n", *data->seed_value);
	fprintf(cf, "%.1f\n", *data->up_repeat_value);
	fclose(cf);
}
