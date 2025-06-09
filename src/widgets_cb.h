#ifndef WIDGETS_CB_H
#define WIDGETS_CB_H

void add_dropdown_selected_item_textview(GtkWidget* wgt, GParamSpec *pspec, gpointer user_data);

void array_strings_free(const char **list);

void clear_img2img_path (GtkWindow *wgt, gpointer user_data);

gboolean close_app_callback (GtkWindow *win, gpointer user_data);

void dropdown_items_update(const char *path, GtkWidget *dd);

void free_cache_data(MyCacheData *s);

void kill_stable_diffusion_process(GtkButton *btn, gpointer user_data);

void on_dropdown_destroy(GtkWidget* wgt, gpointer user_data);

void quit_btn_callback (GtkWidget *wgt, GtkWidget *win);

void reload_dropdown(GtkWidget* wgt, gpointer user_data);

void reset_view_callback (GtkWidget* btn, gpointer user_data);

void set_dropdown_selected_const_item(GtkWidget* wgt, GParamSpec *pspec, int *i1);

void set_dropdown_selected_item(GtkWidget* wgt, GParamSpec *pspec, gpointer user_data);

void set_spin_value_to_var(GtkWidget *w, double *v);

void toggle_image_visibility(GtkButton *btn, gpointer user_data);

void toggle_extra_options(GtkCheckButton *btn, gpointer user_data);

#endif // WIDGETS_CB_H
