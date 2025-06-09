#include <gtk/gtk.h>
#include <png.h>
#include <stdio.h>
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

#if GTK_CHECK_VERSION(4, 10, 0)
static void read_png_metadata(GObject* client, GAsyncResult* res, gpointer user_data)
{
	GFile *png_file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(client), res, NULL);

	if (png_file != NULL) {
		char *path = g_file_get_path(png_file);

		FILE *fp = fopen(path, "rb");
		if (!fp) {
			perror("File opening failed");
			exit(1);
		}

		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png) {
			perror("png_create_read_struct failed");
			fclose(fp);
			return;
		}

		png_infop info = png_create_info_struct(png);
		if (!info) {
			perror("png_create_info_struct failed");
			png_destroy_read_struct(&png, NULL, NULL);
			fclose(fp);
			return;
		}

		if (setjmp(png_jmpbuf(png))) {
			perror("Error during PNG creation");
			png_destroy_read_struct(&png, &info, NULL);
			fclose(fp);
			return;
		}

		png_init_io(png, fp);
		png_read_info(png, info);

		png_textp text;
		int num_text;
		png_get_text(png, info, &text, &num_text);

		char *l1 = malloc(sizeof(char));
		if (l1 == NULL) {
			perror("Failed to allocate memory for array1");
		}
		l1[0] = '\0';

		for (int i = 0; i < num_text; i++) {
			add_strings(&l1, text[i].key);
			add_strings(&l1, text[i].text);
		}

		LoadPNGData *data = user_data;
		
		char *pos_start = strchr(l1, '"');
		char *neg_start = strstr(l1, "\"\nNegative prompt: \"");
		char *steps_start = strstr(l1, "\"\nSteps: ");
		char *cfg_start = strstr(l1, ", CFG scale: ");
		char *guidance_start = strstr(l1, ", Guidance: ");
		char *seed_start = strstr(l1, ", Seed: ");
		char *size_start = strstr(l1, ", Size: ");
		char *model_start = strstr(l1, ", Model: ");
		char *rng_start = strstr(l1, ", RNG: ");
		char *sampler_start = strstr(l1, ", Sampler: ");
		char *version_start = strstr(l1, ", Version: ");

		//Set Positive Prompt
		if(pos_start != NULL && neg_start != NULL) {
			size_t pos_len = neg_start - pos_start - 1;
			GtkTextBuffer *ptb = data->pos_tb;
			if(pos_len > 0) {
				char *pprompt = malloc((pos_len + 1) *sizeof(char));
				pprompt[0] = '\0';
				strncpy(pprompt, pos_start + 1, pos_len);
				pprompt[pos_len] = '\0';

				gtk_text_buffer_set_text (ptb, pprompt, -1);
				free(pprompt);
			} else {
				gtk_text_buffer_set_text (ptb, POSITIVE_PROMPT, -1);
			}
		}
		
		//Set Negative Prompt
		if(neg_start != NULL && steps_start != NULL) {
			size_t neg_len = steps_start - neg_start - 20;
			GtkTextBuffer *ntb = data->neg_tb;
			if(neg_len > 0) {
				char *nprompt = malloc((neg_len + 1) *sizeof(char));
				nprompt[0] = '\0';
				strncpy(nprompt, neg_start + 20, neg_len);
				nprompt[neg_len] = '\0';
				gtk_text_buffer_set_text (ntb, nprompt, -1);
				free(nprompt);
			} else {
				gtk_text_buffer_set_text (ntb, NEGATIVE_PROMPT, -1);
			}
		}
		
		//Set Steps
		if(steps_start != NULL && cfg_start != NULL) {
			size_t steps_len = cfg_start - steps_start - 9;
			char *steps_str = malloc((steps_len + 1) *sizeof(char));
			steps_str[0] = '\0';
			strncpy(steps_str, steps_start + 9, steps_len);
			steps_str[steps_len] = '\0';
			int steps_i = check_list_contains_item(LIST_STEPS_STR, steps_str);
			GtkWidget *steps_dd = data->steps_dd;
			gtk_drop_down_set_selected(GTK_DROP_DOWN(steps_dd), steps_i);
			free(steps_str);
		}
		
		//Set CFG Value
		if(cfg_start != NULL && guidance_start != NULL) {
			size_t cfg_len = guidance_start - cfg_start - 13;
			char *cfg_str = malloc((cfg_len + 1) *sizeof(char));
			cfg_str[0] = '\0';
			strncpy(cfg_str, cfg_start + 13, cfg_len);
			cfg_str[cfg_len] = '\0';
			double cfg_double;
			GtkWidget *cfg_spin = data->cfg_spin;
			if (sscanf(cfg_str, "%lf", &cfg_double) == 1) {
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), cfg_double);
			} else {
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), DEFAULT_CFG);
			}
			free(cfg_str);
		}
		
		//Set Guidance Value
		if(guidance_start != NULL && seed_start != NULL && 1 > 2) {
			size_t guidance_len = seed_start - guidance_start - 12;
			char *guidance_str = malloc((guidance_len + 1) *sizeof(char));
			strncpy(guidance_str, guidance_start + 12, guidance_len);
			guidance_str[guidance_len] = '\0';
			double guidance_double;
			if (sscanf(guidance_str, "Guidance: %lf", &guidance_double) == 1) {
				printf("Guidance: %.1f\n", guidance_double);
			}
		}
		
		//Set Seed Value
		if(seed_start != NULL && size_start != NULL) {
			size_t seed_len = size_start - seed_start - 8;
			char *seed_str = malloc((seed_len + 1) *sizeof(char));
			seed_str[0] = '\0';
			strncpy(seed_str, seed_start + 8, seed_len);
			seed_str[seed_len] = '\0';
			long long int seed_long;
			GtkWidget *seed_spin = data->seed_spin;
			if (sscanf(seed_str, "%lld", &seed_long) == 1) {
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(seed_spin), seed_long);
			} else {
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(seed_spin), DEFAULT_SEED);
			}
			free(seed_str);
		}
		
		//Set IMG Size
		if(size_start != NULL && model_start != NULL) {
			size_t size_len = model_start - size_start - 8;
			char *width_str = malloc((size_len + 1) *sizeof(char));
			width_str[0] = '\0';
			strncpy(width_str, size_start + 8, size_len);
			width_str[size_len] = '\0';
			char *heigth_str = strchr(width_str, 'x') + 1;

			size_t width_len = strchr(width_str, 'x') - width_str;
			size_t heigth_len = strlen(heigth_str);

			char *final_w = malloc((width_len + 1) *sizeof(char));
			final_w[0] = '\0';
			char *final_h = malloc((heigth_len + 1) *sizeof(char));
			final_h[0] = '\0';

			GtkWidget *width_dd = data->width_dd;
			GtkWidget *height_dd = data->height_dd;

			strncpy(final_w, width_str, width_len);
			strncpy(final_h, width_str + width_len + 1, heigth_len);

			final_w[width_len] = '\0';
			final_h[heigth_len] = '\0';

			int width_i = check_list_contains_item(LIST_RESOLUTIONS_STR, final_w);
			int height_i = check_list_contains_item(LIST_RESOLUTIONS_STR, final_h);

			gtk_drop_down_set_selected(GTK_DROP_DOWN(width_dd), width_i);
			gtk_drop_down_set_selected(GTK_DROP_DOWN(height_dd), height_i);

			free(width_str);
			free(final_w);
			free(final_h);
		}
		
		//Set SD Model
		if(model_start != NULL && rng_start != NULL) {
			size_t model_len = rng_start - model_start - 9;
			char *model_str = malloc((model_len + 1) *sizeof(char));
			model_str[0] = '\0';
			strncpy(model_str, model_start + 9, model_len);
			model_str[model_len] = '\0';
			const char** models_files = get_files(MODELS_PATH);
			int model_i = check_list_contains_item(models_files, model_str);
			GtkWidget *model_dd = data->model_dd;
			gtk_drop_down_set_selected(GTK_DROP_DOWN(model_dd), model_i);
			array_strings_free(models_files);
			free(model_str);
		}
		
		//Set RNG Value
		if(rng_start != NULL && sampler_start != NULL && 1 > 2) {
			size_t rng_len = sampler_start - rng_start - 7;
			char *rng_str = malloc((rng_len + 1) *sizeof(char));
			strncpy(rng_str, rng_start + 7, rng_len);
			rng_str[rng_len] = '\0';
			printf("RNG: %s\n", rng_str);
		}
		
		//Set Sampler and Schedule
		if(version_start != NULL) {
			size_t sampler_len = version_start - sampler_start - 11;
			char *sampler_str = malloc((sampler_len + 1) *sizeof(char));
			sampler_str[0] = '\0';
			strncpy(sampler_str, sampler_start + 11, sampler_len);
			sampler_str[sampler_len] = '\0';

			char *schedule_str = strchr(sampler_str, ' ') + 1;

			size_t samp_len = strchr(sampler_str, ' ') - sampler_str;
			size_t sche_len = strlen(schedule_str);

			char *final_sampler = malloc((samp_len + 1) *sizeof(char));
			final_sampler[0] = '\0';
			char *final_schedule = malloc((sche_len + 1) *sizeof(char));
			final_sampler[0] = '\0';

			GtkWidget *sample_dd = data->sample_dd;
			GtkWidget *schedule_dd = data->schedule_dd;

			if (sscanf(sampler_str, "%s %s", final_sampler, final_schedule) == 2) {
				final_sampler[samp_len] = '\0';
				final_schedule[sche_len] = '\0';
				int sample_i = check_list_contains_item(LIST_SAMPLES, final_sampler);
				int schedule_i = check_list_contains_item(LIST_SCHEDULES, final_schedule);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(sample_dd), sample_i);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(schedule_dd), schedule_i);
			} else {
				gtk_drop_down_set_selected(GTK_DROP_DOWN(sample_dd), DEFAULT_SAMPLE);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(schedule_dd), DEFAULT_SCHEDULE);
			}
			free(sampler_str);
			free(final_sampler);
			free(final_schedule);
		}
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		free(l1);
	}
}

static void set_file_path(GObject* client, GAsyncResult* res, gpointer user_data)
{
	GFile *png_file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(client), res, NULL);

	if (png_file != NULL) {
		char *path = g_file_get_path(png_file);
		if (path != NULL) {
			LoadImg2ImgData *data = user_data;
			GString *str = data->img2img_file_path;
			g_string_assign(str, path);
			GtkImage *prev_img = GTK_IMAGE(data->image_wgt);
			if (check_file_exists(path, 0) == 1) {
				gtk_image_set_from_file(prev_img, path);
			} else {
				gtk_image_set_from_file(prev_img, "./resources/example.png");
			}
			g_free(path);
		}
	}
}

#else
static void read_png_metadata_deprecated(GtkDialog* dialog, int response, gpointer user_data)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

		g_autoptr(GFile) png_file = gtk_file_chooser_get_file (chooser);

		if (png_file != NULL) {
			char *path = g_file_get_path(png_file);

			FILE *fp = fopen(path, "rb");
			if (!fp) {
				perror("File opening failed");
				exit(1);
			}

			png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if (!png) {
				perror("png_create_read_struct failed");
				fclose(fp);
				return;
			}

			png_infop info = png_create_info_struct(png);
			if (!info) {
				perror("png_create_info_struct failed");
				png_destroy_read_struct(&png, NULL, NULL);
				fclose(fp);
				return;
			}

			if (setjmp(png_jmpbuf(png))) {
				perror("Error during PNG creation");
				png_destroy_read_struct(&png, &info, NULL);
				fclose(fp);
				return;
			}

			png_init_io(png, fp);
			png_read_info(png, info);

			png_textp text;
			int num_text;
			png_get_text(png, info, &text, &num_text);

			char *l1 = malloc(sizeof(char));
			if (l1 == NULL) {
				perror("Failed to allocate memory for array1");
			}
			l1[0] = '\0';

			for (int i = 0; i < num_text; i++) {
				add_strings(&l1, text[i].key);
				add_strings(&l1, text[i].text);
			}

			LoadPNGData *data = user_data;
			
			char *pos_start = strchr(l1, '"');
			char *neg_start = strstr(l1, "\"\nNegative prompt: \"");
			char *steps_start = strstr(l1, "\"\nSteps: ");
			char *cfg_start = strstr(l1, ", CFG scale: ");
			char *guidance_start = strstr(l1, ", Guidance: ");
			char *seed_start = strstr(l1, ", Seed: ");
			char *size_start = strstr(l1, ", Size: ");
			char *model_start = strstr(l1, ", Model: ");
			char *rng_start = strstr(l1, ", RNG: ");
			char *sampler_start = strstr(l1, ", Sampler: ");
			char *version_start = strstr(l1, ", Version: ");

			//Set Positive Prompt
			if(pos_start != NULL && neg_start != NULL) {
				size_t pos_len = neg_start - pos_start - 1;
				GtkTextBuffer *ptb = data->pos_tb;
				if(pos_len > 0) {
					char *pprompt = malloc((pos_len + 1) *sizeof(char));
					pprompt[0] = '\0';
					strncpy(pprompt, pos_start + 1, pos_len);
					pprompt[pos_len] = '\0';

					gtk_text_buffer_set_text (ptb, pprompt, -1);
					free(pprompt);
				} else {
					gtk_text_buffer_set_text (ptb, POSITIVE_PROMPT, -1);
				}
			}
			
			//Set Negative Prompt
			if(neg_start != NULL && steps_start != NULL) {
				size_t neg_len = steps_start - neg_start - 20;
				GtkTextBuffer *ntb = data->neg_tb;
				if(neg_len > 0) {
					char *nprompt = malloc((neg_len + 1) *sizeof(char));
					nprompt[0] = '\0';
					strncpy(nprompt, neg_start + 20, neg_len);
					nprompt[neg_len] = '\0';
					gtk_text_buffer_set_text (ntb, nprompt, -1);
					free(nprompt);
				} else {
					gtk_text_buffer_set_text (ntb, NEGATIVE_PROMPT, -1);
				}
			}
			
			//Set Steps
			if(steps_start != NULL && cfg_start != NULL) {
				size_t steps_len = cfg_start - steps_start - 9;
				char *steps_str = malloc((steps_len + 1) *sizeof(char));
				steps_str[0] = '\0';
				strncpy(steps_str, steps_start + 9, steps_len);
				steps_str[steps_len] = '\0';
				int steps_i = check_list_contains_item(LIST_STEPS_STR, steps_str);
				GtkWidget *steps_dd = data->steps_dd;
				gtk_drop_down_set_selected(GTK_DROP_DOWN(steps_dd), steps_i);
				free(steps_str);
			}
			
			//Set CFG Value
			if(cfg_start != NULL && guidance_start != NULL) {
				size_t cfg_len = guidance_start - cfg_start - 13;
				char *cfg_str = malloc((cfg_len + 1) *sizeof(char));
				cfg_str[0] = '\0';
				strncpy(cfg_str, cfg_start + 13, cfg_len);
				cfg_str[cfg_len] = '\0';
				double cfg_double;
				GtkWidget *cfg_spin = data->cfg_spin;
				if (sscanf(cfg_str, "%lf", &cfg_double) == 1) {
					gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), cfg_double);
				} else {
					gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), DEFAULT_CFG);
				}
				free(cfg_str);
			}
			
			//Set Guidance Value
			if(guidance_start != NULL && seed_start != NULL && 1 > 2) {
				size_t guidance_len = seed_start - guidance_start - 12;
				char *guidance_str = malloc((guidance_len + 1) *sizeof(char));
				strncpy(guidance_str, guidance_start + 12, guidance_len);
				guidance_str[guidance_len] = '\0';
				double guidance_double;
				if (sscanf(guidance_str, "Guidance: %lf", &guidance_double) == 1) {
					printf("Guidance: %.1f\n", guidance_double);
				}
			}
			
			//Set Seed Value
			if(seed_start != NULL && size_start != NULL) {
				size_t seed_len = size_start - seed_start - 8;
				char *seed_str = malloc((seed_len + 1) *sizeof(char));
				seed_str[0] = '\0';
				strncpy(seed_str, seed_start + 8, seed_len);
				seed_str[seed_len] = '\0';
				long long int seed_long;
				GtkWidget *seed_spin = data->seed_spin;
				if (sscanf(seed_str, "%lld", &seed_long) == 1) {
					gtk_spin_button_set_value (GTK_SPIN_BUTTON(seed_spin), seed_long);
				} else {
					gtk_spin_button_set_value (GTK_SPIN_BUTTON(seed_spin), DEFAULT_SEED);
				}
				free(seed_str);
			}
			
			//Set IMG Size
			if(size_start != NULL && model_start != NULL) {
				size_t size_len = model_start - size_start - 8;
				char *width_str = malloc((size_len + 1) *sizeof(char));
				width_str[0] = '\0';
				strncpy(width_str, size_start + 8, size_len);
				width_str[size_len] = '\0';
				char *heigth_str = strchr(width_str, 'x') + 1;

				size_t width_len = strchr(width_str, 'x') - width_str;
				size_t heigth_len = strlen(heigth_str);

				char *final_w = malloc((width_len + 1) *sizeof(char));
				final_w[0] = '\0';
				char *final_h = malloc((heigth_len + 1) *sizeof(char));
				final_h[0] = '\0';

				GtkWidget *width_dd = data->width_dd;
				GtkWidget *height_dd = data->height_dd;

				strncpy(final_w, width_str, width_len);
				strncpy(final_h, width_str + width_len + 1, heigth_len);

				final_w[width_len] = '\0';
				final_h[heigth_len] = '\0';

				int width_i = check_list_contains_item(LIST_RESOLUTIONS_STR, final_w);
				int height_i = check_list_contains_item(LIST_RESOLUTIONS_STR, final_h);

				gtk_drop_down_set_selected(GTK_DROP_DOWN(width_dd), width_i);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(height_dd), height_i);

				free(width_str);
				free(final_w);
				free(final_h);
			}
			
			//Set SD Model
			if(model_start != NULL && rng_start != NULL) {
				size_t model_len = rng_start - model_start - 9;
				char *model_str = malloc((model_len + 1) *sizeof(char));
				model_str[0] = '\0';
				strncpy(model_str, model_start + 9, model_len);
				model_str[model_len] = '\0';
				const char** models_files = get_files(MODELS_PATH);
				int model_i = check_list_contains_item(models_files, model_str);
				GtkWidget *model_dd = data->model_dd;
				gtk_drop_down_set_selected(GTK_DROP_DOWN(model_dd), model_i);
				array_strings_free(models_files);
				free(model_str);
			}
			
			//Set RNG Value
			if(rng_start != NULL && sampler_start != NULL && 1 > 2) {
				size_t rng_len = sampler_start - rng_start - 7;
				char *rng_str = malloc((rng_len + 1) *sizeof(char));
				strncpy(rng_str, rng_start + 7, rng_len);
				rng_str[rng_len] = '\0';
				printf("RNG: %s\n", rng_str);
			}
			
			//Set Sampler and Schedule
			if(version_start != NULL) {
				size_t sampler_len = version_start - sampler_start - 11;
				char *sampler_str = malloc((sampler_len + 1) *sizeof(char));
				sampler_str[0] = '\0';
				strncpy(sampler_str, sampler_start + 11, sampler_len);
				sampler_str[sampler_len] = '\0';

				char *schedule_str = strchr(sampler_str, ' ') + 1;

				size_t samp_len = strchr(sampler_str, ' ') - sampler_str;
				size_t sche_len = strlen(schedule_str);

				char *final_sampler = malloc((samp_len + 1) *sizeof(char));
				final_sampler[0] = '\0';
				char *final_schedule = malloc((sche_len + 1) *sizeof(char));
				final_sampler[0] = '\0';

				GtkWidget *sample_dd = data->sample_dd;
				GtkWidget *schedule_dd = data->schedule_dd;

				if (sscanf(sampler_str, "%s %s", final_sampler, final_schedule) == 2) {
					final_sampler[samp_len] = '\0';
					final_schedule[sche_len] = '\0';
					int sample_i = check_list_contains_item(LIST_SAMPLES, final_sampler);
					int schedule_i = check_list_contains_item(LIST_SCHEDULES, final_schedule);
					gtk_drop_down_set_selected(GTK_DROP_DOWN(sample_dd), sample_i);
					gtk_drop_down_set_selected(GTK_DROP_DOWN(schedule_dd), schedule_i);
				} else {
					gtk_drop_down_set_selected(GTK_DROP_DOWN(sample_dd), DEFAULT_SAMPLE);
					gtk_drop_down_set_selected(GTK_DROP_DOWN(schedule_dd), DEFAULT_SCHEDULE);
				}
				free(sampler_str);
				free(final_sampler);
				free(final_schedule);
			}
			png_destroy_read_struct(&png, &info, NULL);
			fclose(fp);
			free(l1);
		}
	}
	gtk_window_destroy (GTK_WINDOW (dialog));
}

static void set_file_path_deprecated(GtkDialog* dialog, int response, gpointer user_data)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

		g_autoptr(GFile) png_file = gtk_file_chooser_get_file (chooser);

		if (png_file != NULL) {
			char *path = g_file_get_path(png_file);
			if (path != NULL) {
				LoadImg2ImgData *data = user_data;
				GString *str = data->img2img_file_path;
				g_string_assign(str, path);
				GtkImage *prev_img = GTK_IMAGE(data->image_wgt);
				if (check_file_exists(path, 0) == 1) {
					gtk_image_set_from_file(prev_img, path);
				} else {
					gtk_image_set_from_file(prev_img, "./resources/example.png");
				}
				g_free(path);
			}
		}
	}
	gtk_window_destroy (GTK_WINDOW (dialog));
}

#endif

void open_png_dialog(GtkWidget *btn, gpointer user_data)
{
	#if GTK_CHECK_VERSION(4, 10, 0)
		LoadPNGData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkFileDialog* load_img_dialog = gtk_file_dialog_new();
		gtk_file_dialog_set_title(load_img_dialog, "Choose the image:");
		gtk_file_dialog_set_modal(load_img_dialog, TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_add_suffix (load_img_filter, "png");
		gtk_file_filter_set_name(load_img_filter,"PNG");

		GListStore* l = g_list_store_new (GTK_TYPE_FILE_FILTER);
		g_list_store_append(l, load_img_filter);
		gtk_file_dialog_set_filters(load_img_dialog, G_LIST_MODEL(l));

		GCancellable* png_cancellable = g_cancellable_new();

		gtk_file_dialog_open(load_img_dialog, win, png_cancellable, (GAsyncReadyCallback)read_png_metadata, user_data);
	#else
		LoadPNGData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkWidget *load_img_dialog = gtk_file_chooser_dialog_new ("Choose the image:",
						win,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						"Cancel",
						GTK_RESPONSE_CANCEL,
						"Open",
						GTK_RESPONSE_ACCEPT,
						NULL);

		gtk_window_present (GTK_WINDOW (load_img_dialog));
		
		g_signal_connect (load_img_dialog, "response",
				G_CALLBACK (read_png_metadata_deprecated),
				user_data);

	#endif
}

void get_img_path_dialog(GtkWidget *btn, gpointer user_data)
{
	#if GTK_CHECK_VERSION(4, 10, 0)
		LoadImg2ImgData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkFileDialog* load_img_dialog = gtk_file_dialog_new();
		gtk_file_dialog_set_title(load_img_dialog, "Choose the image file:");
		gtk_file_dialog_set_modal(load_img_dialog, TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_add_suffix (load_img_filter, "png");
		gtk_file_filter_set_name(load_img_filter,"PNG");
		gtk_file_filter_add_suffix (load_img_filter, "jpg");
		gtk_file_filter_set_name(load_img_filter,"JPG");

		GListStore* l = g_list_store_new (GTK_TYPE_FILE_FILTER);
		g_list_store_append(l, load_img_filter);
		gtk_file_dialog_set_filters(load_img_dialog, G_LIST_MODEL(l));

		GCancellable* png_cancellable = g_cancellable_new();

		gtk_file_dialog_open(load_img_dialog, win, png_cancellable, (GAsyncReadyCallback)set_file_path, user_data);
	#else
		LoadPNGData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkWidget *load_img_dialog = gtk_file_chooser_dialog_new ("Choose the image:",
						win,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						"Cancel",
						GTK_RESPONSE_CANCEL,
						"Open",
						GTK_RESPONSE_ACCEPT,
						NULL);

		gtk_window_present (GTK_WINDOW (load_img_dialog));
		
		g_signal_connect (load_img_dialog, "response",
				G_CALLBACK (set_file_path_deprecated),
				user_data);

	#endif
}
