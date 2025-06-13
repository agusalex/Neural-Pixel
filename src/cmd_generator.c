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

GString *gen_sd_string(GenerationData *data)
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

	GString *l1 = g_string_new(NULL);
	
	#ifdef G_OS_WIN32
		gchar *current_dir = g_get_current_dir();
		
		g_string_append(l1, current_dir);
		g_free(current_dir);
		
		if (*data->cpu_bool == 1) {
			g_string_append(l1, "\\sd-cpu");
		} else {
			g_string_append(l1, "\\sd");
		}
	#else
		if (*data->cpu_bool == 1) {
			g_string_append(l1, "./sd-cpu");
		} else {
			g_string_append(l1, "./sd");
		}
	#endif
	
	if (img2img_g_string != NULL && strcmp(img2img_g_string->str, "None") != 0) {
		g_string_append_printf(l1, "|-M|img2img|-i|%s", img2img_g_string->str);
	}

	g_string_append_printf(l1, "|-m|./models/checkpoints/%s", sel_model);

	g_string_append(l1, "|--lora-model-dir|./models/loras/|--embd-dir|./models/embeddings");

	if (sel_vae != NULL && strcmp(sel_vae, "None") != 0) {
		g_string_append_printf(l1, "|--vae|./models/vae/%s", sel_vae);
		if (*data->k_vae_bool == 1) {
			g_string_append(l1, "|--vae-on-cpu");
		}
	}
  
	if (sel_cnet != NULL && strcmp(sel_cnet, "None") != 0) {
		g_string_append_printf(l1, "|--control-net|./models/controlnet/%s", sel_cnet);
		if (*data->k_cnet_bool == 1) {
			g_string_append(l1, "|--control-net-cpu");
		}
	}

	if (sel_upscale != NULL && strcmp(sel_upscale, "None") != 0) {
		g_string_append_printf(l1, "|--upscale-model|./models/upscale_models/%s|--upscale-repeats|%s", sel_upscale, up_repeat_str);
	}
	
	if (sel_clip_l != NULL && strcmp(sel_clip_l, "None") != 0) {
		g_string_append_printf(l1, "|--clip_l|./models/clips/%s", sel_clip_l);
	}
	
	if (sel_clip_g != NULL && strcmp(sel_clip_g, "None") != 0) {
		g_string_append_printf(l1, "|--clip_g|./models/clips/%s", sel_clip_g);
	}
	
	if (sel_t5xxl != NULL && strcmp(sel_t5xxl, "None") != 0) {
		g_string_append_printf(l1, "|--t5xxl|./models/text_encoders/%s", sel_t5xxl);
	}
	
	if (*data->k_clip_bool == 1) {
		if (sel_clip_l != NULL || sel_clip_g != NULL || sel_t5xxl != NULL) {
			if (strcmp(sel_clip_l, "None") != 0 || strcmp(sel_clip_g, "None") != 0 || strcmp(sel_t5xxl, "None") != 0) {
				g_string_append(l1, "|--clip-on-cpu");
			}
		}
	}

	g_string_append_printf(l1, "|--strength|%s", denoise_str);

	if (*data->taesd_bool == 1) {
		g_string_append(l1, "|--taesd|.models/TAESD/taesd_decoder.safetensors");
	}

	g_string_append_printf(l1, "|--cfg-scale|%s", cfg_str);

	g_string_append_printf(l1, "|--sampling-method|%s", LIST_SAMPLES[(*data->sample_index)]);

	g_string_append_printf(l1, "|--schedule|%s", LIST_SCHEDULES[(*data->schedule_index)]);

	g_string_append_printf(l1,"|-s|%s", seed_str);

	g_string_append_printf(l1, "|--steps|%s", LIST_STEPS_STR[(*data->n_steps_index)]);

	g_string_append_printf(l1, "|-b|%s", LIST_STEPS_STR[(*data->bs_index)]);

	if (img2img_g_string == NULL || strcmp(img2img_g_string->str, "None") == 0) {
		g_string_append_printf(l1, "|-W|%s", LIST_RESOLUTIONS_STR[(*data->w_index)]);
		g_string_append_printf(l1, "|-H|%s", LIST_RESOLUTIONS_STR[(*data->h_index)]);
	}

	if (*data->vt_bool == 1) {
		g_string_append(l1, "|--vae-tiling");
	}

	if (*data->fa_bool == 1) {
		g_string_append(l1, "|--diffusion-fa");
	}

	if (p_text != NULL) {
		g_string_append_printf(l1, "|-p|\"%s\"", p_text);
	}

	if (n_text != NULL) {
		g_string_append_printf(l1, "|-n|\"%s\"", n_text);
	}

	int n_img_count = count_output_files();
	char *n_img_string = convert_int_to_string(n_img_count);
	
	#ifdef G_OS_WIN32
		g_string_append_printf(l1, "|-o|.\\outputs\\IMG_%s.png", n_img_string);
	#else
		g_string_append_printf(l1, "|-o|./outputs/IMG_%s.png", n_img_string);
	#endif

	update_cache(data, sel_model, sel_vae, sel_cnet, sel_upscale, sel_clip_l, sel_clip_g, sel_t5xxl, p_text, n_text, n_img_string);

	g_free(p_text);
	g_free(n_text);
	free(n_img_string);
	free(cfg_str);
	free(denoise_str);
	free(seed_str);
	free(up_repeat_str);
	array_strings_free(model_items);
	array_strings_free(vae_items);
	array_strings_free(cnet_items);
	array_strings_free(upscale_items);
	array_strings_free(clip_l_items);
	array_strings_free(clip_g_items);
	array_strings_free(t5xxl_items);
	
	if (*data->verbose_bool == 1) {
		GString *l1_copy = g_string_new_len(l1->str, l1->len);

		for (gsize i = 0; i < l1_copy->len; i++) {
			if (l1_copy->str[i] == '|') {
				l1_copy->str[i] = ' ';
			}
		}
		
		printf("%s\n", l1_copy->str);
		g_string_free(l1_copy, TRUE);
	}

	return l1;
}
