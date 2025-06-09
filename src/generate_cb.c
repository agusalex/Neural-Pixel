#include <gtk/gtk.h>
#include <gio/gio.h>
#include "cmd_generator.h"
#include "file_utils.h"
#include "global.h"
#include "structs.h"
#include "str_utils.h"

static int img_n = 1;
static int img_t = 1;

static void show_progress(GObject* stream_obj, GAsyncResult* res, gpointer user_data)
{
	HandleOutputData *data = user_data;
	
	if(globalSDPID != 0) {
		char *line = NULL;
		gsize length;
		GError *error = NULL;

		if((line = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(stream_obj), res, &length, &error)) != NULL) {
			if (data->verbose_bool == 1) {
				printf("%s\n", line);
			}
			if (strstr(line, "sampling completed") != NULL && img_n == img_t) {
				gtk_button_set_label(GTK_BUTTON(data->button), "Decoding latent(s)...");
			} else {
				if (strstr(line, "generating image:") != NULL) {
					long int img_seed;
					const char *last_colon = strrchr(line, ':');
					if (sscanf(last_colon + 1, " %i/%i - seed %ld\n", &img_n, &img_t, &img_seed) != 0) {
						g_data_input_stream_set_newline_type(
							G_DATA_INPUT_STREAM(data->pipe_stream),
							G_DATA_STREAM_NEWLINE_TYPE_CR
						);
					} else {
						printf("Error getting batch size\n");
					}
				} else {
					int step, steps;
					float time_or_speed;
					char unit[20];
					const char *last_pipe = strrchr(line, '|');

					if (last_pipe != NULL) {
						if (sscanf(last_pipe + 1, " %i/%i - %f%19s\n", &step, &steps, &time_or_speed, unit) != 0) {
							if (steps < 61) {
								int pp = (int)(((float)step / steps) * 100 + 0.5);
								int pnd = count_digits((float)pp) +
									count_digits((float)img_n) +
									count_digits((float)img_t);
								char pstr[18 + pnd];
								snprintf(pstr, sizeof(pstr), "Sampling... %d%% %d/%d", pp, img_n, img_t);
								pstr[sizeof(pstr)] = '\0';
								gtk_button_set_label(GTK_BUTTON(data->button), pstr);
								if (step == steps - 1) {
									g_data_input_stream_set_newline_type(
										G_DATA_INPUT_STREAM(data->pipe_stream),
										G_DATA_STREAM_NEWLINE_TYPE_LF
									);
								}
							}
						}
					}
					
					g_free(line);
				}
			}
		}
		if (error != NULL) {
			g_printerr("Error reading line: %s\n", error->message);
			g_error_free(error);
		}
		g_data_input_stream_read_line_async(
			data->pipe_stream,
			G_PRIORITY_DEFAULT,
			NULL,
			show_progress,
			user_data
		);
	} else {
		gboolean s_closed = g_input_stream_close(G_INPUT_STREAM(data->pipe_stream), NULL, NULL);
	}
}

static void on_subprocess_end(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
	EndGenerationData *data = user_data;
	globalSDPID = 0;
	const char *icon_n = gtk_button_get_icon_name (GTK_BUTTON(data->show_img_btn));
	
	if (g_strcmp0 (icon_n, "view-conceal-symbolic") != 0) {
		GtkImage *prev_img = GTK_IMAGE(data->image_widget);
		if (check_file_exists(data->img_name, 0) == 1) {
			gtk_image_set_from_file(prev_img, data->img_name);
		} else {
			gtk_image_set_from_file(prev_img, "./resources/example.png");
		}
	}
	
	gtk_button_set_label(GTK_BUTTON(data->button), "Generate");
	gtk_widget_set_sensitive(GTK_WIDGET(data->button), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), FALSE);
	free(data->cmd);
	g_object_unref(source_object);
}

static void start_reading_output(gpointer user_data)
{
	HandleOutputData *data = user_data;
	GDataInputStream *data_stream = data->pipe_stream;
	g_data_input_stream_read_line_async(
		data_stream,
		G_PRIORITY_DEFAULT,
		NULL,
		show_progress,
		user_data
	);
}

void generate_cb(GtkButton *gen_btn, gpointer user_data)
{
	GenerationData *data = user_data;
	if (data == NULL) {
		printf("data is null\n");
		exit(1);
	}
	if (g_strcmp0 (gtk_button_get_label (gen_btn), "Generate") == 0) {
		gtk_button_set_label (gen_btn, "Loading Files...");
		gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), TRUE);
	}

	char *c_line = gen_sd_string(data);

	char *argv[50];
	int argc = 0;

	char *cmd = strdup(c_line);
	if (cmd == NULL) {
		fprintf(stderr, "Memory allocation failed!\n");
	}

	free(c_line);

	char *t;
	char* str = cmd;

	while ((t = strtok_r(str, "|", &str))) {
		argv[argc] = t; 
		argc++;
	}

	argv[argc] = '\0';
	const gchar * const *argv_const = (const gchar * const *)argv;

	char *img_name = argv[argc -1];

	GSubprocessFlags sd_flags = G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE;
	GError *error = NULL;

	GSubprocess* sd_process = g_subprocess_newv(argv_const, sd_flags, &error);

	if (sd_process == NULL) {
		g_print("Error spawning process: %s\n", error->message);
		g_clear_error(&error);
	} else {
		const gchar* sd_pid = g_subprocess_get_identifier(sd_process);
		char *endptr;
		int npid = strtol(sd_pid, &endptr, 10);

		if (*endptr == '\0') {
			globalSDPID = npid;
		} else {
			globalSDPID = 0;
		}

		GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(sd_process);
		GDataInputStream *data_stream = g_data_input_stream_new(stdout_stream);
		g_data_input_stream_set_newline_type (data_stream, G_DATA_STREAM_NEWLINE_TYPE_LF);

		EndGenerationData *check_d = g_new0 (EndGenerationData, 1);
		check_d->pid = npid;
		check_d->cmd = cmd;
		check_d->button = GTK_WIDGET(gen_btn);
		check_d->image_widget = data->image_widget;
		check_d->show_img_btn = data->show_img_btn;
		check_d->halt_btn = data->halt_btn;
		check_d->img_name = img_name;
		check_d->channel = NULL;
		HandleOutputData *output_d = g_new0 (HandleOutputData, 1);
		output_d->verbose_bool = *data->verbose_bool;
		output_d->button = GTK_WIDGET(gen_btn);
		output_d->sdpid = npid;
		output_d->pipe_stream = data_stream;

		printf("Binary launched in subprocess: %d\n", npid);

		GCancellable* cnlb = g_cancellable_new ();

		start_reading_output(output_d);
		g_subprocess_wait_async(sd_process, cnlb, on_subprocess_end, check_d);
	}
}
