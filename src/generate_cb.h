#ifndef GENERATE_CB_H
#define GENERATE_CB_H

static void show_progress(GObject* stream_obj, GAsyncResult* res, gpointer user_data);

static void on_subprocess_end(GObject* source_object, GAsyncResult* res, gpointer user_data);

static void start_reading_output(gpointer user_data);

void generate_cb(GtkButton *gen_btn, gpointer user_data);

#endif // GENERATE_CB_H
