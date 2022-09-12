#ifndef LINPOPWND_H_
#define LINPOPWND_H_

int gtk_wnd_init(int argc, char* argv[]);
GtkWidget* create_login_wnd();
GtkWidget* create_linpopwnd();
void enter_callback(GtkWidget* widget, GtkWidget* entry);
gboolean progress_login_timeout(GtkProgressBar* progress);
void login(GtkWidget* widget, gpointer data);
void set_user_info(GtkWidget* widget, gpointer data);
int show_info_msg_box(GtkMessageType msg_type, GtkButtonsType btn_type,
	gchar* text_error);
void linpop_quit(GtkWidget* widget, gpointer data);
GtkWidget* login_window, * linpop_window; //µÇÂ¼´°¿Ú£¬Ö÷´°¿Ú
#endif /* LINPOPWND_H_ */
