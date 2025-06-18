#include <gtk/gtk.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "global.h"
#include "structs.h"

void add_dropdown_selected_item_textview (GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (GTK_IS_DROP_DOWN(wgt)) {
		DropDownTextBufferData *data = user_data;
		GtkDropDown *dd = GTK_DROP_DOWN(wgt);

		int tb_type = data->tb_type;
		GtkTextBuffer *tv_tb = data->textbuffer;

		GtkStringObject *selected_item = gtk_drop_down_get_selected_item(dd);
		const char* item_string = gtk_string_object_get_string(selected_item);

		if (strcmp(item_string, "None") != 0) {
			char *text = format_lora_embedding_string(item_string, tb_type);
			GtkTextIter si;
			GtkTextIter ei;
			GtkTextIter sel_i;
			gtk_text_buffer_get_bounds (tv_tb, &si, &ei);
			if (tb_type == 0) {
				sel_i = ei;
			} else {
				sel_i = si;
			}
			gtk_text_buffer_insert (tv_tb, &sel_i, text, -1);
			free(text);
		}
	}
}

void array_strings_free (const char **list)
{
	if (list != NULL) {
		for (int i = 0; list[i] != NULL; i++) {
			free((char*)list[i]);
		}
		free(list);
	}
}

void clear_img2img_btn_cb (GtkWindow *wgt, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	GString *gstr = data->img2img_file_path;
	g_string_assign(gstr, "None");
	GtkImage *prev_img = GTK_IMAGE(data->image_wgt);
	gtk_image_set_from_file(prev_img, "./resources/example.png");
}

gboolean close_app_callback (GtkWindow *win, gpointer user_data)
{
	AppStartData *data = user_data;
	if (data == NULL) return 0;
	g_string_free(data->model_string, TRUE);
	data->model_string = NULL;
	g_string_free(data->vae_string, TRUE);
	data->vae_string = NULL;
	g_string_free(data->cnet_string, TRUE);
	data->cnet_string = NULL;
	g_string_free(data->upscale_string, TRUE);
	data->upscale_string = NULL;
	g_string_free(data->clip_l_string, TRUE);
	data->clip_l_string = NULL;
	g_string_free(data->clip_g_string, TRUE);
	data->clip_g_string = NULL;
	g_string_free(data->t5xxl_string, TRUE);
	data->t5xxl_string = NULL;
	g_string_free(data->img2img_file_path, TRUE);
	data->img2img_file_path = NULL;
	gtk_window_destroy (win);
	g_free(data);
}

void dropdown_items_update (const char *path, GtkWidget *dd)
{
	GtkStringList *new_dd_items = get_files(path);
	gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(new_dd_items));
}

void free_cache_data (MyCacheData *s)
{
	if (s == NULL) {
		return;
	}
	free(s->pos_p);
	free(s->neg_p);
	free(s->img_name);
	free(s);
}

void free_app_start_data (gpointer data)
{
	AppStartData *app_start_d = (AppStartData *)data;
	g_string_free(app_start_d->img2img_file_path, TRUE);
	g_free(app_start_d);
}


void free_preview_data (gpointer data)
{
	PreviewImageData *preview_d = (PreviewImageData *)data;
	g_free(preview_d);
}
void kill_stable_diffusion_process (GtkButton *btn, gpointer user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);

	#ifdef _WIN32
		HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)globalSDPID);
		if (hp == NULL) {
			fprintf(stderr, "Failed to open process %d. Error: %lu\n", globalSDPID, GetLastError());
		} else {
			if (TerminateProcess(hp, 1)) {
				printf("Process %d terminated successfully.\n", globalSDPID);
				globalSDPID = 0;
			} else {
				fprintf(stderr, "Failed to terminate process %d. Error: %lu\n", globalSDPID, GetLastError());
			}
			CloseHandle(hp);
		}
	#else
		if (kill(globalSDPID, SIGKILL) == 0) {
			printf("Process %d killed successfully.\n", globalSDPID);
			globalSDPID = 0;
		} else {
			perror("Error killing process");
		}
	#endif
}

void on_clear_img2img_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_dd_const_destroy (GtkWidget* wgt, gpointer user_data)
{
	DropDownModelsNameData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_dd_path_destroy (GtkWidget* wgt, gpointer user_data)
{
	DropDownTextBufferData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_dropdown_destroy (GtkWidget* wgt, gpointer user_data)
{
	const char** di = (const char**)user_data;
	array_strings_free(di);
}

void on_generate_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	GenerationData *data = user_data;
	if (data == NULL) return;
	//g_string_free(data->img2img_file_path, TRUE);
	//data->img2img_file_path = NULL;
	g_free(data);
}

void on_hide_img_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	PreviewImageData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_load_from_img_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	LoadPNGData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_reload_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	ReloadDropDownData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_reset_default_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	ResetCbData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void quit_btn_callback (GtkWidget *wgt, GtkWidget *win)
{
	gtk_window_close(GTK_WINDOW(win));
}

void reload_dropdown(GtkWidget* wgt, gpointer user_data)
{
	ReloadDropDownData *data = user_data;
	dropdown_items_update(MODELS_PATH, GTK_WIDGET(data->model_dd));
	dropdown_items_update(VAES_PATH, GTK_WIDGET(data->vae_dd));
	dropdown_items_update(CONTROLNET_PATH, GTK_WIDGET(data->cnet_dd));
	dropdown_items_update(UPSCALES_PATH, GTK_WIDGET(data->upscale_dd));
	dropdown_items_update(CLIPS_PATH, GTK_WIDGET(data->clip_l_dd));
	dropdown_items_update(CLIPS_PATH, GTK_WIDGET(data->clip_g_dd));
	dropdown_items_update(TEXT_ENCODERS_PATH, GTK_WIDGET(data->t5xxl_dd));
	dropdown_items_update(LORAS_PATH, GTK_WIDGET(data->lora_dd));
	dropdown_items_update(EMBEDDINGS_PATH, GTK_WIDGET(data->embedding_dd));
}

void reset_default_btn_cb (GtkWidget* btn, gpointer user_data)
{
	ResetCbData *data = user_data;

	GtkTextBuffer *pos_tb = data->pos_tb;
	gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);

	GtkTextBuffer *neg_tb = data->neg_tb;
	gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);

	GtkWidget *model_dd = data->model_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(model_dd), DEFAULT_MODELS);

	GtkWidget *vae_dd = data->vae_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(vae_dd), DEFAULT_MODELS);

	GtkWidget *cnet_dd = data->cnet_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(cnet_dd), DEFAULT_MODELS);

	GtkWidget *upscale_dd = data->upscale_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(upscale_dd), DEFAULT_MODELS);
	
	GtkWidget *clip_l_dd = data->clip_l_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(clip_l_dd), DEFAULT_MODELS);
	
	GtkWidget *clip_g_dd = data->clip_g_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(clip_g_dd), DEFAULT_MODELS);
	
	GtkWidget *t5xxl_dd = data->t5xxl_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(t5xxl_dd), DEFAULT_MODELS);

	GtkWidget *cfg_spin = data->cfg_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), DEFAULT_CFG);

	GtkWidget *denoise_spin = data->denoise_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(denoise_spin), DEFAULT_DENOISE);

	GtkWidget *seed_spin = data->seed_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(seed_spin), DEFAULT_SEED);

	GtkWidget *upscale_spin = data->upscale_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(upscale_spin), DEFAULT_RP_UPSCALE);

	GtkWidget *lora_dd = data->lora_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(lora_dd), DEFAULT_MODELS);

	GtkWidget *embedding_dd = data->embedding_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(embedding_dd), DEFAULT_MODELS);

	GtkWidget *sample_dd = data->sample_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(sample_dd), DEFAULT_SAMPLE);

	GtkWidget *schedule_dd = data->schedule_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(schedule_dd), DEFAULT_SCHEDULE);

	GtkWidget *steps_dd = data->steps_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(steps_dd), DEFAULT_N_STEPS);

	GtkWidget *width_dd = data->width_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(width_dd), DEFAULT_SIZE);

	GtkWidget *height_dd = data->height_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(height_dd), DEFAULT_SIZE);

	GtkWidget *batch_dd = data->batch_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(batch_dd), DEFAULT_BATCH_SIZE);
	
	GtkWidget *cpu_check = data->cpu_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cpu_check), DEFAULT_OPT_VRAM == 1 ? TRUE : FALSE);

	GtkWidget *tiling_check = data->tiling_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(tiling_check), DEFAULT_OPT_VRAM == 1 ? TRUE : FALSE);

	GtkWidget *clip_check = data->clip_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(clip_check), DEFAULT_OPT_VRAM == 1 ? TRUE : FALSE);

	GtkWidget *cnet_check = data->cnet_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cnet_check), DEFAULT_OPT_VRAM == 1 ? TRUE : FALSE);

	GtkWidget *vae_check = data->vae_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(vae_check), DEFAULT_OPT_VRAM == 1 ? TRUE : FALSE);

	GtkWidget *flash_check = data->flash_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(flash_check), DEFAULT_OPT_VRAM == 1 ? TRUE : FALSE);
}

void set_dropdown_selected_const_item(GtkWidget* wgt, GParamSpec *pspec, int *i1)
{
	GtkDropDown *dd = GTK_DROP_DOWN(wgt);
	guint s = gtk_drop_down_get_selected(dd);
	*i1 = (int)s;
}

void set_dropdown_selected_item (GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (GTK_IS_DROP_DOWN(wgt)) {
		DropDownModelsNameData *data = user_data;
		GString *var_string = data->dd_item_str;
		int is_req = data->req_int;
		GtkDropDown *dd = GTK_DROP_DOWN(wgt);
		guint s = gtk_drop_down_get_selected(dd);
		GtkStringObject *dd_string_obj = gtk_drop_down_get_selected_item(dd);
		const char *dd_string = gtk_string_object_get_string(dd_string_obj);
		g_string_assign(var_string, dd_string);
		if (is_req == 1) {
			GtkWidget *gen_btn = data->g_btn;
			if (g_strcmp0(var_string->str, "None") == 0) {
				gtk_button_set_label (GTK_BUTTON(gen_btn), "Select a model first.");
				gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), FALSE);
			} else {
				gtk_button_set_label (GTK_BUTTON(gen_btn), "Generate");
				gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), TRUE);
			}
		}
	}
}

void set_spin_value_to_var (GtkWidget *w, double *v)
{
	double value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
	*v = value;
}

void hide_img_btn_cb (GtkButton *btn, gpointer user_data)
{
	PreviewImageData *data = user_data;
	GtkImage *img = GTK_IMAGE(data->image_widget);
	const char *i;
	i = gtk_button_get_icon_name (btn);
	if (g_strcmp0 (i, "view-reveal-symbolic") == 0) {
		gtk_button_set_icon_name (btn, "view-conceal-symbolic");
		gtk_image_clear(GTK_IMAGE(img));
	} else {
		gtk_button_set_icon_name (btn, "view-reveal-symbolic");
	}
}

void toggle_extra_options (GtkCheckButton *btn, gpointer user_data)
{
	int *i = (int *)user_data;
	*i = gtk_check_button_get_active(btn) == TRUE ? 1 : 0;
}
