#ifndef HANDLE_CACHE_H
#define HANDLE_CACHE_H

#include "structs.h"

void create_cache(char *n);

void load_pp_cache(GtkTextBuffer *pos_tb);

void load_np_cache(GtkTextBuffer *neg_tb);

void load_img_cache(GtkWidget *img_wgt);

void load_cache(gpointer user_data);

void update_cache(GenerationData *data, const char *sel_model, const char *sel_vae, const char *sel_cnet, const char *sel_upscale, const char *sel_clip_l, const char *sel_clip_g, const char *sel_t5xxl, char *pp, char *np, char *img_num);

#endif // HANDLE_CACHE_H
