#ifndef HANDLE_CACHE_H
#define HANDLE_CACHE_H

#include "structs.h"

void create_cache(char *n, GError **error);

void load_pp_cache(GtkTextBuffer *pos_tb);

void load_np_cache(GtkTextBuffer *neg_tb);

void load_img_cache(GtkWidget *img_wgt);

void load_cache_fallback(gpointer user_data);

void load_cache(gpointer user_data);

void update_cache(GenerationData *data, gchar *sel_model, gchar *sel_vae, gchar *sel_cnet, gchar *sel_upscale, gchar *sel_clip_l, gchar *sel_clip_g, gchar *sel_t5xxl, char *pp, char *np, char *img_num);

#endif // HANDLE_CACHE_H
