#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <time.h>
#include "constants.h"
#include "file_utils.h"
#include "structs.h"
#include "handle_cache.h"
#include "str_utils.h"
#include "widgets_cb.h"

char *gen_sd_string(GenerationData *data)
{
	GtkTextBuffer *pos_tb = data->pos_p;
	GtkTextIter psi;
	GtkTextIter pei;
	gtk_text_buffer_get_bounds (pos_tb, &psi, &pei);
	char *p_text = gtk_text_buffer_get_text(pos_tb, &psi, &pei, FALSE);

	GtkTextBuffer *neg_tb = data->neg_p;
	GtkTextIter nsi;
	GtkTextIter nei;
	gtk_text_buffer_get_bounds (neg_tb, &nsi, &nei);
	char *n_text = gtk_text_buffer_get_text(neg_tb, &nsi, &nei, FALSE);
	
	const char** model_items = get_files(MODELS_PATH);
	const char** vae_items = get_files(VAES_PATH);
	const char** cnet_items = get_files(CONTROLNET_PATH);
	const char** upscale_items = get_files(UPSCALES_PATH);
	const char** clip_l_items = get_files(CLIPS_PATH);
	const char** clip_g_items = get_files(CLIPS_PATH);
	const char** t5xxl_items = get_files(TEXT_ENCODERS_PATH);

	const char *sel_model = model_items[*data->model_index];
	const char *sel_vae = vae_items[*data->vae_index];
	const char *sel_cnet = cnet_items[*data->cnet_index];
	const char *sel_upscale = upscale_items[*data->upscale_index];
	const char *sel_clip_l = clip_l_items[*data->clip_l_index];
	const char *sel_clip_g = clip_g_items[*data->clip_g_index];
	const char *sel_t5xxl = t5xxl_items[*data->t5xxl_index];
	
	char *cfg_str = convert_double_to_string(*data->cfg_value, "%.1f");
	char *denoise_str = convert_double_to_string(*data->denoise_value, "%.2f");
	char *seed_str = convert_double_to_string(*data->seed_value, "%.0f");
	char *up_repeat_str = convert_double_to_string(*data->up_repeat_value, "%.0f");
	
	GString *img2img_g_string = data->img2img_file_path;

	char *l1 = malloc(10 * sizeof(char));

	if (l1 == NULL) {
		perror("Failed to allocate memory for cmd string");
	}
	l1[0] = '\0';
	
	#ifdef G_OS_WIN32
		const gchar *sd_vulkan_name = "sd.exe";
		
		gchar *current_dir = g_get_current_dir();
		
		add_strings(&l1, current_dir);
		
		if (*data->cpu_bool == 1) {
			add_strings(&l1, "\\sd-cpu");
		} else {
			add_strings(&l1, "\\sd");
		}
	#else
		if (*data->cpu_bool == 1) {
			add_strings(&l1, "./sd-cpu");
		} else {
			add_strings(&l1, "./sd");
		}
	#endif
	
	if (img2img_g_string != NULL && strcmp(img2img_g_string->str, "None") != 0) {
		add_strings(&l1, "|-M|img2img|-i|");
		add_strings(&l1, img2img_g_string->str);
	}

	add_strings(&l1, "|-m|./models/checkpoints/");
	add_strings(&l1, sel_model);

	add_strings(&l1, "|--lora-model-dir|./models/loras/|--embd-dir|./models/embeddings");

	if (sel_vae != NULL && strcmp(sel_vae, "None") != 0) {
		add_strings(&l1, "|--vae|./models/vae/");
		add_strings(&l1, sel_vae);
		if (*data->k_vae_bool == 1) {
			add_strings(&l1, "|--vae-on-cpu");
		}
	}
  
	if (sel_cnet != NULL && strcmp(sel_cnet, "None") != 0) {
		add_strings(&l1, "|--control-net|./models/controlnet/");
		add_strings(&l1, sel_cnet);
		if (*data->k_cnet_bool == 1) {
			add_strings(&l1, "|--control-net-cpu");
		}
	}

	if (sel_upscale != NULL && strcmp(sel_upscale, "None") != 0) {
		add_strings(&l1, "|--upscale-model|./models/upscale_models/");
		add_strings(&l1, sel_upscale);
		add_strings(&l1, "|--upscale-repeats|");
		add_strings(&l1, up_repeat_str);
	}
	
	if (sel_clip_l != NULL && strcmp(sel_clip_l, "None") != 0) {
		add_strings(&l1, "|--clip_l|./models/clips/");
		add_strings(&l1, sel_clip_l);
	}
	
	if (sel_clip_g != NULL && strcmp(sel_clip_g, "None") != 0) {
		add_strings(&l1, "|--clip_g|./models/clips/");
		add_strings(&l1, sel_clip_g);
	}
	
	if (sel_t5xxl != NULL && strcmp(sel_t5xxl, "None") != 0) {
		add_strings(&l1, "|--t5xxl|./models/text_encoders/");
		add_strings(&l1, sel_t5xxl);
	}
	
	if (*data->k_clip_bool == 1) {
		if (sel_clip_l != NULL || sel_clip_g != NULL || sel_t5xxl != NULL) {
			if (strcmp(sel_clip_l, "None") != 0 || strcmp(sel_clip_g, "None") != 0 || strcmp(sel_t5xxl, "None") != 0) {
				add_strings(&l1, "|--clip-on-cpu");
			}
		}
	}

	add_strings(&l1, "|--strength|");
	add_strings(&l1, denoise_str);

	if (*data->taesd_bool == 1) {
		add_strings(&l1, "|--taesd|");
		add_strings(&l1, ".models/TAESD/taesd_decoder.safetensors");
	}

	add_strings(&l1, "|--cfg-scale|");
	add_strings(&l1, cfg_str);

	add_strings(&l1, "|--sampling-method|");
	add_strings(&l1, LIST_SAMPLES[(*data->sample_index)]);

	add_strings(&l1, "|--schedule|");
	add_strings(&l1, LIST_SCHEDULES[(*data->schedule_index)]);

	add_strings(&l1, "|-s|");
	add_strings(&l1, seed_str);

	add_strings(&l1, "|--steps|");
	add_strings(&l1, LIST_STEPS_STR[(*data->n_steps_index)]);

	add_strings(&l1, "|-b|");
	add_strings(&l1, LIST_STEPS_STR[(*data->bs_index)]);

	if (img2img_g_string == NULL || strcmp(img2img_g_string->str, "None") == 0) {
		add_strings(&l1, "|-W|");
		add_strings(&l1, LIST_RESOLUTIONS_STR[(*data->w_index)]);

		add_strings(&l1, "|-H|");
		add_strings(&l1, LIST_RESOLUTIONS_STR[(*data->h_index)]);
	}

	if (*data->vt_bool == 1) {
		add_strings(&l1, "|--vae-tiling");
	}

	if (*data->fa_bool == 1) {
		add_strings(&l1, "|--diffusion-fa");
	}

	if (p_text != NULL) {
		add_strings(&l1, "|-p|\"");
		add_strings(&l1, p_text);
	}

	if (n_text != NULL) {
		add_strings(&l1, "\"|-n|\"");
		add_strings(&l1, n_text);
		add_strings(&l1, "\"");
	}

	int n_img_count = count_output_files();
	char *n_img_string = convert_int_to_string(n_img_count);
	
	#ifdef G_OS_WIN32
		add_strings(&l1, "|-o|.\\outputs\\IMG_");
	#else
		add_strings(&l1, "|-o|./outputs/IMG_");
	#endif
	
	add_strings(&l1, n_img_string);
	add_strings(&l1, ".png");

	update_cache(data, sel_model, sel_vae, sel_cnet, sel_upscale, sel_clip_l, sel_clip_g, sel_t5xxl, p_text, n_text, n_img_string);

	free(n_img_string);
	free(cfg_str);
	free(denoise_str);
	free(seed_str);
	free(up_repeat_str);
	array_strings_free(model_items);
	array_strings_free(vae_items);
	array_strings_free(cnet_items);
	array_strings_free(upscale_items);
	
	if (*data->verbose_bool == 1) {
		char *str = malloc((strlen(l1) + 1) * sizeof(char));
		int i = 0;
		while (l1[i] != '\0') {
			if (l1[i] == '|') {
				str[i] = ' ';
			} else {
				str[i] = l1[i];
			}
			i++;
		}
		str[i] = '\0';
		
		printf("%s\n", str);
		free(str);
	}

	return l1;
}
