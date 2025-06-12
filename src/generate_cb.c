#include <gtk/gtk.h>
#include <gio/gio.h>
#include "cmd_generator.h"
#include "file_utils.h"
#include "global.h"
#include "structs.h"
#include "str_utils.h"

static int img_n = 1;
static int img_t = 1;

static void handle_stderr(GObject* stream_obj, GAsyncResult* res, gpointer user_data)
{
	SDProcessErrorData *data = user_data;

	if(globalSDPID != 0) {
		char *err_string = NULL;
		gsize err_length;
		GError *err_error = NULL;

		if((err_string = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(stream_obj), res, &err_length, &err_error)) != NULL) {
			if (data->verbose_bool == 1) {
				printf("%s\n", err_string);
			}
			
			int n_error;
			char file_name[256];
			
			if (sscanf(err_string, "[ERROR] stable-diffusion.cpp:%i  - init model loader from file failed: '%255[^']'", &n_error, file_name) == 2) {
				char error_dialog_text[16 + strlen(file_name)];
				strcpy(error_dialog_text, "Error loading: ");
				strcat(error_dialog_text, file_name);

				GtkAlertDialog *file_loading_error_dialog = gtk_alert_dialog_new ("Error loading file");
				gtk_alert_dialog_set_detail (file_loading_error_dialog, error_dialog_text);
				gtk_alert_dialog_show (file_loading_error_dialog, GTK_WINDOW(data->win));
			}
			g_free(err_string);
		}
		
		if (err_error != NULL) {
			g_printerr("Error reading line: %s\n", err_error->message);
			g_error_free(err_error);
		}
		
		g_data_input_stream_read_line_async(
			data->err_pipe_stream,
			G_PRIORITY_DEFAULT,
			NULL,
			handle_stderr,
			user_data
		);
		
	} else {
		gboolean s_closed = g_input_stream_close(G_INPUT_STREAM(data->err_pipe_stream), NULL, NULL);
		g_object_unref(data->err_pipe_stream);
		data->err_pipe_stream = NULL;
		g_free(data);
	}
}

static void show_progress(GObject* stream_obj, GAsyncResult* res, gpointer user_data)
{
	SDProcessOutputData *data = user_data;
	
	if(globalSDPID != 0) {
	
		char *out_string = NULL;
		gsize out_length;
		GError *out_error = NULL;

		if((out_string = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(stream_obj), res, &out_length, &out_error)) != NULL) {
			if (data->verbose_bool == 1) {
				printf("%s\n", out_string);
			}
			if (strstr(out_string, "[ERROR] stable-diffusion.cpp:242  - get sd version from file failed: './models/checkpoints/anyloraCheckpoint_bakedvaeBlessedFp16'")) {
				
			}
			if (strstr(out_string, "sampling completed") != NULL && img_n == img_t) {
				gtk_button_set_label(GTK_BUTTON(data->button), "Decoding latent(s)...");
			} else {
				if (strstr(out_string, "generating image:") != NULL) {
					long int img_seed;
					const char *last_colon = strrchr(out_string, ':');
					if (sscanf(last_colon + 1, " %i/%i - seed %ld\n", &img_n, &img_t, &img_seed) != 0) {
						g_data_input_stream_set_newline_type(
							G_DATA_INPUT_STREAM(data->out_pipe_stream),
							G_DATA_STREAM_NEWLINE_TYPE_CR
						);
					} else {
						printf("Error getting batch size\n");
					}
				} else {
					int step, steps;
					float time_or_speed;
					char unit[20];
					const char *last_pipe = strrchr(out_string, '|');

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
										G_DATA_INPUT_STREAM(data->out_pipe_stream),
										G_DATA_STREAM_NEWLINE_TYPE_LF
									);
								}
							}
						}
					}
				}
			}
			g_free(out_string);
		}
		
		if (out_error != NULL) {
			g_printerr("Error reading line: %s\n", out_error->message);
			g_error_free(out_error);
		}
		g_data_input_stream_read_line_async(
			data->out_pipe_stream,
			G_PRIORITY_DEFAULT,
			NULL,
			show_progress,
			user_data
		);
	} else {
		gboolean s_closed = g_input_stream_close(G_INPUT_STREAM(data->out_pipe_stream), NULL, NULL);
		g_object_unref(data->out_pipe_stream);
		data->out_pipe_stream = NULL;
		g_free(data);
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
	g_free(data);
	g_object_unref(source_object);
}

static void start_reading_error(gpointer user_data)
{
	SDProcessErrorData *data = user_data;
	GDataInputStream *data_err_stream = data->err_pipe_stream;
	g_data_input_stream_read_line_async(
		data_err_stream,
		G_PRIORITY_DEFAULT,
		NULL,
		handle_stderr,
		user_data
	);
}

static void start_reading_output(gpointer user_data)
{
	SDProcessOutputData *data = user_data;
	GDataInputStream *data_out_stream = data->out_pipe_stream;
	g_data_input_stream_read_line_async(
		data_out_stream,
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
		GInputStream *stderr_stream = g_subprocess_get_stderr_pipe(sd_process);
		
		GDataInputStream *data_out_stream = g_data_input_stream_new(stdout_stream);
		GDataInputStream *data_err_stream = g_data_input_stream_new(stderr_stream);
		
		g_data_input_stream_set_newline_type (data_out_stream, G_DATA_STREAM_NEWLINE_TYPE_LF);
		g_data_input_stream_set_newline_type (data_err_stream, G_DATA_STREAM_NEWLINE_TYPE_LF);

		EndGenerationData *check_d = g_new0 (EndGenerationData, 1);
		check_d->pid = npid;
		check_d->cmd = cmd;
		check_d->button = GTK_WIDGET(gen_btn);
		check_d->image_widget = data->image_widget;
		check_d->show_img_btn = data->show_img_btn;
		check_d->halt_btn = data->halt_btn;
		check_d->img_name = img_name;
		
		SDProcessOutputData *output_d = g_new0 (SDProcessOutputData, 1);
		output_d->verbose_bool = *data->verbose_bool;
		output_d->button = GTK_WIDGET(gen_btn);
		output_d->sdpid = npid;
		output_d->out_pipe_stream = data_out_stream;
		
		SDProcessErrorData *error_d = g_new0 (SDProcessErrorData, 1);
		error_d->verbose_bool = *data->verbose_bool;
		error_d->win = data->win;
		error_d->sdpid = npid;
		error_d->err_pipe_stream = data_err_stream;

		printf("Binary launched in subprocess: %d\n", npid);

		GCancellable* cnlb = g_cancellable_new ();

		start_reading_output(output_d);
		start_reading_error(error_d);
		g_subprocess_wait_async(sd_process, cnlb, on_subprocess_end, check_d);
	}
}
