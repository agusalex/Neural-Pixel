#ifndef PNG_UTILS_H
#define PNG_UTILS_H

static void read_png_metadata(GObject* client, GAsyncResult* res, gpointer user_data);

static void set_file_path(GObject* client, GAsyncResult* res, gpointer user_data);

static void read_png_metadata_deprecated(GtkDialog* dialog, int response, gpointer user_data);

static void set_file_path_deprecated(GObject* client, GAsyncResult* res, gpointer user_data);

void open_png_dialog(GtkWidget *btn, gpointer user_data);

void get_img_path_dialog(GtkWidget *btn, gpointer user_data);

#endif // PNG_UTILS_H
