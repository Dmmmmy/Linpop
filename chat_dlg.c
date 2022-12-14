#include "linpop.h"
#include "chat_dlg.h"

#define MAX_BUF				1024 * 2
GData * widget_chat_dlg = NULL;// 聊天窗口集, 用于获取已经建立的聊天窗口
extern GArray* g_array_user; // 储存用户信息的全局变量
extern char my_id[15]; // 登录 user 的 id


void change_background(GtkWidget* widget, int w, int h, const gchar* path)
{
        //1.允许窗口可以绘图
        gtk_widget_set_app_paintable(widget, TRUE);
        gtk_widget_realize(widget);

        /* 更改背景图时，图片会重叠
        * 这时要手动调用下面的函数，让窗口绘图区域失效，产生窗口重绘制事件（即 expose 事件）。
        */
        gtk_widget_queue_draw(widget);
        GdkPixbuf* src = gdk_pixbuf_new_from_file(path, NULL);
        GdkPixbuf* dst = gdk_pixbuf_scale_simple(src, w, h, GDK_INTERP_BILINEAR);

        /* 创建pixmap图像;
        * NULL：不需要蒙版;
        * 123： 0~255，透明到不透明
        */
        GdkPixmap* pixmap = NULL;
        gdk_pixbuf_render_pixmap_and_mask(dst, &pixmap, NULL, 128);

        // 通过pixmap给widget设置一张背景图，最后一个参数必须为: FASL
        gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);


        g_object_unref(src);
        g_object_unref(dst);
        g_object_unref(pixmap);

        return;
}

Widgets_Chat* create_chat_dlg(gchar* text_id, int face_num, gboolean group)
{
	Widgets_Chat* w_chat = g_slice_new(Widgets_Chat);

	GtkWidget* chat_dlg;
	GtkWidget* hbox_main, * vbox_left, * vbox_right;
	gchar* title = g_strdup_printf("%s", text_id);

	hbox_main = gtk_hbox_new(FALSE, 2);
	vbox_left = gtk_vbox_new(FALSE, 0);
	vbox_right = gtk_vbox_new(FALSE, 40);
	gtk_widget_set_size_request(vbox_right, 180, 0);

	chat_dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(chat_dlg), GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(chat_dlg), "destroy",
		G_CALLBACK(close_dlg), chat_dlg);
	gtk_window_set_title(GTK_WINDOW(chat_dlg), title);
	gtk_container_set_border_width(GTK_CONTAINER(chat_dlg), 0);

	//本地信息
	gchar* face_path = g_strdup_printf("/mnt/hgfs/Image/tx/%d.jpg", face_num);
	GtkWidget* image_user = gtk_image_new_from_file(face_path);
	GtkWidget* button_image = gtk_button_new();
	GtkWidget* label_info = gtk_label_new(
		"Welcome to chat with me!");
	gtk_container_add(GTK_CONTAINER(button_image), image_user);
	gtk_widget_set_size_request(button_image, 52, 52);

	change_background(chat_dlg, 600, 400, "/mnt/hgfs/Image/2333.jpg");
	GtkWidget* hbox_head = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_head), button_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_head), label_info, TRUE, TRUE, 5);

	// 输出 text view
	GtkWidget* scrolled_win_output = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win_output),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget* hbox_output = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(hbox_output), scrolled_win_output, TRUE, TRUE, 0);

	GtkWidget* textview_output = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview_output), FALSE);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_output), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_output), GTK_WRAP_WORD);
	gtk_container_add(GTK_CONTAINER(scrolled_win_output), textview_output);
	gtk_widget_set_size_request(scrolled_win_output, 360, 310);
	/*
	 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(
	 scrolled_win_output), textview_output);
	 */
	 // 添加这句之后就无法自动滚动消息了

	 // 设置消息输出区域 text view 垂直滚动消息
	GtkTextBuffer* buffer;
	GtkTextIter iter;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_output));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_buffer_create_mark(buffer, "scroll", &iter, TRUE);

	// 输入 text view
	GtkWidget* scrolled_win_input = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win_input),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkWidget* hbox_input = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_input), scrolled_win_input, TRUE, TRUE, 0);

	GtkWidget* textview_input = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_input), GTK_WRAP_WORD);
	gtk_scrolled_window_add_with_viewport(
		GTK_SCROLLED_WINDOW(scrolled_win_input), textview_input);
	gtk_widget_set_size_request(scrolled_win_input, 350, 100);

	// 发送和关闭按钮
	GtkWidget* button_close = gtk_button_new_with_label("    关闭    ");
	g_signal_connect(G_OBJECT(button_close), "clicked", G_CALLBACK(
		close_dlg), chat_dlg);
	GtkWidget* button_send = gtk_button_new_with_label("    发送    ");
	g_signal_connect(G_OBJECT(button_send), "clicked", G_CALLBACK(
		send_text_msg), chat_dlg);

	GtkWidget* hbox_button = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox_button), button_send, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(hbox_button), button_close, FALSE, FALSE, 5);

	GtkWidget* table;
	GtkWidget* progress, * label_file;
	progress = NULL; // 注意初始化, 本身群聊窗口是没有进度条的
	
		// 文件传送进度条和文件名
		GtkWidget* hbox_progress, * hbox_label_file;

		progress = gtk_progress_bar_new(); // 右侧窗口进度条
		gtk_widget_set_size_request(progress, 170, 20);
		hbox_progress = gtk_hbox_new(FALSE, 15);
		gtk_box_pack_start(GTK_BOX(hbox_progress), GTK_WIDGET(progress), FALSE,
			TRUE, 0);
		if (!group) // 群窗口没有进度条显示文件和文件传送
		{
			// 右侧控件布局
			//有改动
			table = gtk_table_new(10, 1, TRUE);
			gtk_table_set_row_spacings(GTK_TABLE(table), 20);
			gtk_container_set_border_width(GTK_CONTAINER(table), 7);

			GtkWidget* frame = gtk_frame_new("对方信息");
			gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);//框字位置
			GtkWidget* label = gtk_label_new(get_info_from_id(text_id));

			gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_FILL);//居中
			gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);//自动换行
			GtkWidget* image = gtk_image_new_from_file(face_path);
			GtkWidget* vbox_face = gtk_vbox_new(FALSE, 15);
			gtk_container_set_border_width(GTK_CONTAINER(vbox_face), 0);
			gtk_box_pack_start(GTK_BOX(vbox_face), image, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(vbox_face), label);
			gtk_container_add(GTK_CONTAINER(frame), vbox_face);
			gtk_table_attach_defaults(GTK_TABLE(table), frame, 0, 1, 2, 6);//改动 0 1 2 5



			frame = gtk_frame_new("我的信息");
			gtk_frame_set_label_align(GTK_FRAME(frame), 0.5, 0.5);
			label = gtk_label_new(get_info_from_id(my_id));


			gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_FILL);
			gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);

			GtkWidget* my_image = gtk_image_new_from_file(image_user);
			GtkWidget* vbox = gtk_vbox_new(FALSE, 15);

			gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
			gtk_box_pack_start(GTK_BOX(vbox), my_image, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(vbox), label);
			gtk_container_add(GTK_CONTAINER(frame), vbox);
			//gtk_container_add(GTK_CONTAINER (frame), label);
			gtk_table_attach_defaults(GTK_TABLE(table), frame, 0, 1, 6, 10);//改动 0 1 6 9


			// 发送文件路径
			label_file = gtk_label_new(
				"This is an example of a line-wrapped label.  It ");
			gtk_widget_set_size_request(label_file, 170, 70);
			gtk_label_set_justify(GTK_LABEL(label_file), GTK_JUSTIFY_FILL);
			gtk_label_set_line_wrap(GTK_LABEL(label_file), TRUE);
			gtk_label_set_line_wrap_mode(GTK_LABEL(label_file), PANGO_WRAP_CHAR); // 设置自动换行
			hbox_label_file = gtk_hbox_new(FALSE, 10);
			gtk_box_pack_start(GTK_BOX(hbox_label_file), label_file, TRUE, FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(vbox_right), 2);
			gtk_box_pack_start(GTK_BOX(vbox_right), hbox_progress, FALSE, TRUE, 15);
			gtk_box_pack_start(GTK_BOX(vbox_right), hbox_label_file, TRUE, FALSE,
				0);
			gtk_table_attach_defaults(GTK_TABLE(table), vbox_right, 0, 1, 7, 9);
		}

	// 左侧窗口中间的工具栏
	GtkWidget* hbox_toolbar = gtk_vbox_new(FALSE, 0);
	GtkWidget* toolbar_middle = gtk_toolbar_new();
	gtk_box_pack_start(GTK_BOX(hbox_toolbar), toolbar_middle, TRUE, TRUE, 0);

	// 表情
	GtkWidget* toolbar_image = gtk_image_new_from_file(
		"/mnt/hgfs/Image/ico/IMSmallToolbarFace.ico");
	gtk_toolbar_append_item(GTK_TOOLBAR(toolbar_middle), NULL, "face",
		"Private", toolbar_image, G_CALLBACK(face_select), NULL);
	// 字体
	toolbar_image = gtk_image_new_from_file(
		"/mnt/hgfs/Image/ico/IMSmallToolbarFont.ico");
	gtk_toolbar_append_item(GTK_TOOLBAR(toolbar_middle), NULL, "font",
		"Private", toolbar_image, G_CALLBACK(font_select), textview_input);
	// 颜色
	toolbar_image = gtk_image_new_from_file(
		"/mnt/hgfs/Image/ico/IMSmallToolbarcolor.ico");
	gtk_toolbar_append_item(GTK_TOOLBAR(toolbar_middle), NULL, "color",
		"Private", toolbar_image, G_CALLBACK(color_select), textview_input);
	// 聊天记录
	toolbar_image = gtk_image_new_from_file(
		"/mnt/hgfs/Image/ico/IMSmallToolbarQuick.ico");
	gtk_toolbar_append_item(GTK_TOOLBAR(toolbar_middle), NULL, "history",
		"Private", toolbar_image, G_CALLBACK(show_history_msg), text_id);
	// 发送文件
	if (!group) // 群窗口没有进度条显示文件和文件传送
	{
		toolbar_image = gtk_image_new_from_file(
			"/mnt/hgfs/Image/ico/IMBigToolbarSendFile.ico");
		gtk_toolbar_append_item(GTK_TOOLBAR(toolbar_middle), NULL, "send file",
			"Private", toolbar_image, G_CALLBACK(send_file), chat_dlg);
	}

	// 左侧控件布局
	gtk_box_pack_start(GTK_BOX(vbox_left), hbox_head, FALSE, TRUE, 8);
	gtk_box_pack_start(GTK_BOX(vbox_left), hbox_output, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_left), hbox_toolbar, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_left), hbox_input, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_left), hbox_button, TRUE, TRUE, 5);

	gtk_box_pack_start(GTK_BOX(hbox_main), vbox_left, FALSE, FALSE, 3);
	if (!group)
		gtk_box_pack_start(GTK_BOX(hbox_main), table, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(chat_dlg), hbox_main);

	// 焦点置于输入 text view
	gtk_widget_grab_focus(textview_input);

	// send 按钮响应回车消息
	GtkAccelGroup* agChat;
	agChat = gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(chat_dlg), agChat);
	gtk_widget_add_accelerator(button_send, "clicked", agChat, GDK_Return, 0,
		GTK_ACCEL_VISIBLE);

	gtk_widget_show_all(chat_dlg);

	w_chat->chat_dlg = chat_dlg;
	w_chat->textview_intput = textview_input;
	w_chat->textview_output = textview_output;
	w_chat->progress = GTK_PROGRESS_BAR(progress);
	w_chat->label_file = label_file;
	w_chat->group = group;

	if (!group)
	{
		gtk_widget_hide(GTK_WIDGET(w_chat->progress));
		gtk_widget_hide(w_chat->label_file);
	}

	return w_chat;
}

char* get_info_from_id(char* id_buf)
{
	int i;
	for (i = 0; i < g_array_user->len; i++)
	{
		if (strcmp(g_array_index(g_array_user, s_user_info, i).id, id_buf) == 0)
			break;
	}

	s_user_info uinfo;
	if (i < g_array_user->len)
		uinfo = g_array_index(g_array_user, s_user_info, i);

	return g_strdup_printf("账号:\t%s\n\n昵称: %s\n\nIP地址:\t%s", uinfo.id,
		uinfo.name, inet_ntoa(uinfo.ip_addr));
}

void face_select(GtkWidget* widget, gpointer data)
{
	GtkWidget* face_dialog;
	GtkWidget* scrolled_window;
	GtkWidget* table;
	GtkWidget* button_gif;
	GtkWidget* image_gif;
	int i, j;
	gchar* buf_path;

	face_dialog = gtk_dialog_new();
	// 设置为模态对话框
	gtk_window_set_modal(GTK_WINDOW(face_dialog), TRUE);
	// 设置固定大小
	gtk_window_set_resizable((GtkWindow*)face_dialog, FALSE);
	gtk_window_set_title(GTK_WINDOW(face_dialog), "选择表情");
	gtk_container_set_border_width(GTK_CONTAINER(face_dialog), 0);
	gtk_widget_set_size_request(face_dialog, 15 * 24, 160);
	gtk_window_set_position((GtkWindow*)face_dialog, GTK_WIN_POS_MOUSE);

	// 创建一个新的滚动窗口
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// 对话框窗口内部包含一个 vbox
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(face_dialog)->vbox),
		scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show(scrolled_window);

	// 创建一个包含 15 × 9 个格子的表格, 用于存放 gif 表情
	table = gtk_table_new(15, 9, TRUE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 2);
	gtk_table_set_col_spacings(GTK_TABLE(table), 2);

	// 将表格组装到滚动窗口中
	gtk_scrolled_window_add_with_viewport(
		GTK_SCROLLED_WINDOW(scrolled_window), table);
	gtk_widget_show(table);

	// 开始添加 gif 表情
	for (i = 0; i < 15; i++)
		for (j = 0; j < 9; j++)
		{
			buf_path = g_strdup_printf("/mnt/hgfs/Image/face/%d.gif", i * 9 + j);
			image_gif = gtk_image_new_from_file(buf_path);

			button_gif = gtk_button_new();
			g_signal_connect(G_OBJECT(button_gif), "button_press_event",
				G_CALLBACK(face_click), buf_path);

			gtk_widget_set_size_request(button_gif, 32, 32);
			gtk_container_add(GTK_CONTAINER(button_gif), image_gif);
			gtk_table_attach_defaults(GTK_TABLE(table), button_gif, j, j + 1,
				i, i + 1);
			gtk_widget_show(button_gif);
		}

	g_free(buf_path);

	gtk_widget_show_all(face_dialog);
}

void face_click(GtkWidget* widget, GdkEventButton* event, gpointer data)
{
	const gchar* buf_path = g_strdup(data);

	GtkWidget* MSGBox;
	MSGBox = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL
		| GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		"%s", buf_path);
	if (gtk_dialog_run(GTK_DIALOG(MSGBox)) == GTK_RESPONSE_OK)
		gtk_widget_destroy(MSGBox);
}

void font_select(GtkWidget* widget, gpointer data)
{
	GtkWidget* textview_input = (GtkWidget*)data;

	GtkWidget* fontdlg = gtk_font_selection_dialog_new("选择字体");
	GtkResponseType ret = gtk_dialog_run(GTK_DIALOG(fontdlg));

	// 设置为模态对话框
	gtk_window_set_modal(GTK_WINDOW(fontdlg), TRUE);

	if (ret == GTK_RESPONSE_OK || ret == GTK_RESPONSE_APPLY)
	{
		gchar* font;
		PangoFontDescription* desc;

		font = gtk_font_selection_dialog_get_font_name(
			GTK_FONT_SELECTION_DIALOG(fontdlg));
		desc = pango_font_description_from_string(font);

		gtk_widget_modify_font(GTK_WIDGET(textview_input), desc);

		g_free(font);
	}
	gtk_widget_destroy(fontdlg);
}

void color_select(GtkWidget* widget, gpointer data)
{
	GtkWidget* textview_input = (GtkWidget*)data;

	GtkWidget* colordlg = gtk_color_selection_dialog_new("选择颜色");
	GtkResponseType ret = gtk_dialog_run(GTK_DIALOG(colordlg));

	// 设置为模态对话框
	gtk_window_set_modal(GTK_WINDOW(colordlg), TRUE);

	if (ret == GTK_RESPONSE_OK)
	{
		GdkColor color;
		GtkColorSelection* colorsel;

		colorsel = GTK_COLOR_SELECTION(
			GTK_COLOR_SELECTION_DIALOG(colordlg)->colorsel);
		gtk_color_selection_get_current_color(colorsel, &color);

		gtk_widget_modify_text(textview_input, GTK_STATE_NORMAL, &color);
	}
	gtk_widget_destroy(colordlg);
}

/*void change_background(GtkWidget* widget, int w, int h, const gchar* path)
{
	//1.允许窗口可以绘图
	gtk_widget_set_app_paintable(widget, TRUE);
	gtk_widget_realize(widget);

	 更改背景图时，图片会重叠
	* 这时要手动调用下面的函数，让窗口绘图区域失效，产生窗口重绘制事件（即 expose 事件）。
	
	gtk_widget_queue_draw(widget);
	GdkPixbuf* src = gdk_pixbuf_new_from_file(path, NULL);
	GdkPixbuf* dst = gdk_pixbuf_scale_simple(src, w, h, GDK_INTERP_BILINEAR);

	 创建pixmap图像;
	* NULL：不需要蒙版;
	* 123： 0~255，透明到不透明
	
	GdkPixmap* pixmap = NULL;
	gdk_pixbuf_render_pixmap_and_mask(dst, &pixmap, NULL, 128);

	// 通过pixmap给widget设置一张背景图，最后一个参数必须为: FASL
	gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);


	g_object_unref(src);
	g_object_unref(dst);
	g_object_unref(pixmap);

	return;
}
*/
void show_history_msg(GtkWidget* widget, gpointer data)
{
	GtkWidget* win_history, * scrolled_win, * vbox, * searchbar;
	GtkWidget* find;
	GtkWidget* text_view_history, * entry_search;
	gchar* title;
	GtkTextBuffer* buffer;
	gchar* content, * file;

	win_history = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	title = g_strdup_printf("与%s聊天", (char*)data);
	gtk_window_set_title(GTK_WINDOW(win_history), title);//窗口名
	gtk_container_set_border_width(GTK_CONTAINER(win_history), 10);//设置边框的宽度
	gtk_widget_set_size_request(win_history, 600, 400);//设置构件大小
	change_background(win_history, 300, 600, "/mnt/hgfs/Image/2333.jpg");
	gtk_window_set_position(GTK_WINDOW(win_history), GTK_WIN_POS_CENTER);//设置窗口位置

	text_view_history = gtk_text_view_new();//生成text view
	scrolled_win = gtk_scrolled_window_new(NULL, NULL);//
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),//同上一起控制滚条
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view_history), FALSE);//设置光标可见
	gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_history), FALSE);//允许text view内容修改
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view_history), GTK_WRAP_WORD);//处理多行显示的模式
	gtk_container_add(GTK_CONTAINER(scrolled_win), text_view_history);//包装text view到滚动条窗口

	entry_search = gtk_entry_new();//建立entry
	gtk_entry_set_text(GTK_ENTRY(entry_search), "请输入查找内容");//

	find  = gtk_button_new_with_label(" 查找 ");
	g_signal_connect(G_OBJECT(find), "clicked", G_CALLBACK(
		NULL), (gpointer)NULL);


	searchbar = gtk_hbox_new(FALSE, 5);//创建组装盒—横向
	gtk_box_pack_start(GTK_BOX(searchbar), entry_search, FALSE, FALSE, 0);//加入entry控件放在左部
	gtk_box_pack_start(GTK_BOX(searchbar), find, FALSE, FALSE, 0);//加入find控件在左部

	vbox = gtk_vbox_new(FALSE, 5);//创建组装盒—纵向
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, TRUE, TRUE, 0);//加入scrolled_win放在顶端或左部
	gtk_box_pack_start(GTK_BOX(vbox), searchbar, FALSE, FALSE, 0);//加入横向组装盒在顶端或者左部

	gtk_container_add(GTK_CONTAINER(win_history), vbox);//包装纵向组装盒到历史记录窗口

	file = g_strdup_printf("%s/%s.txt", my_id, (char*)data);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_history));
	g_file_get_contents(file, &content, NULL, NULL);
	gtk_text_buffer_set_text(buffer, content, -1);
	g_free(content);

	gtk_widget_show_all(win_history);
}


void send_file(GtkButton* button, gpointer data)
{
	GtkWidget* chatdlg = (GtkWidget*)data;
	const char* text_title = gtk_window_get_title(GTK_WINDOW(chatdlg));
	Widgets_Chat* w_chatdlg =
		(Widgets_Chat*)g_datalist_get_data(&widget_chat_dlg, text_title);

	GtkWidget* dialog;
	GSList* filenames;
	gchar* str_file;

	dialog = gtk_file_chooser_dialog_new("选择文件（夹）", NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
		GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);//创建对话框


	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),//设置当前读入文件或文件夹目录（点击之后弹出的）
		g_get_home_dir());// 获取密码数据库中定义的当前用户的主目录。 




	gint result = gtk_dialog_run(GTK_DIALOG(dialog));//显示对话框
	if (result == GTK_RESPONSE_ACCEPT)
	{
		filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));//列出选择器当前文件夹中的所有选定文件和子文件夹
		gchar* file = (gchar*)filenames->data;

		str_file = g_strdup(file);

		gtk_label_set_text(GTK_LABEL(w_chatdlg->label_file), str_file);//改变标签文本
		gtk_widget_show(GTK_WIDGET(w_chatdlg->progress));//显示
		gtk_widget_show(w_chatdlg->label_file);//显示

		// 发送文件时, 先发送询问消息, 对方是否接收, 若对方接收, 则建立 TCP Socket 进行真正的传输
		// 格式为 PROTOCOL_RECV_FILE + 对方 id + 文件名(包含路径) + "-" + 文件大小

		struct stat stat_buf;
		stat(str_file, &stat_buf); // 获取文件大小
		char* size_buf = g_strdup_printf("-%ld", stat_buf.st_size);//显示文件大小

		char* send_buf, * psend;
		int send_size;

		send_size = strlen(PROTOCOL_RECV_FILE) + sizeof(my_id) + strlen(
			str_file) + strlen(size_buf);//发送大小
		send_buf = malloc(send_size);
		memset(send_buf, 0, send_size);
		psend = send_buf;
		char* pro_buf = strdup(PROTOCOL_RECV_FILE);
		memcpy(send_buf, pro_buf, strlen(PROTOCOL_RECV_FILE));
		psend += strlen(PROTOCOL_RECV_FILE);

		memcpy(psend, (char*)text_title, sizeof(my_id));
		psend += sizeof(my_id);
		memcpy(psend, (char*)str_file, strlen(str_file));
		psend += strlen(str_file);
		memcpy(psend, (char*)size_buf, strlen(size_buf));

		// 发送给服务器消息, 通过服务器转发
		send_msg(send_buf, send_size);

	}
	gtk_widget_destroy(dialog);
}

char* get_cur_time()
{
	time_t timep;
	struct tm* time_cur;
	char* time_str;
	time(&timep);
	time_cur = localtime(&timep);

	time_str = g_strdup_printf("%02d:%02d:%02d", time_cur->tm_hour,
		time_cur->tm_min, time_cur->tm_sec);

	return time_str;
}

void send_text_msg(GtkWidget* widget, gpointer data)
{
	GtkWidget* chatdlg = (GtkWidget*)data;
	const char* text_title = gtk_window_get_title(GTK_WINDOW(chatdlg));

	// 获得相关的聊天窗口集所关联的数据, 获得当前聊天对话框的 textview_intput
	Widgets_Chat* w_chatdlg =
		(Widgets_Chat*)g_datalist_get_data(&widget_chat_dlg, text_title);

	GtkTextIter start_in, end_in, start_out, end_out;
	gchar* text_msg = malloc(MAX_BUF);

	GtkTextBuffer* buff_input, * buff_output;
	buff_input = gtk_text_view_get_buffer(
		GTK_TEXT_VIEW(w_chatdlg->textview_intput));
	buff_output = gtk_text_view_get_buffer(
		GTK_TEXT_VIEW(w_chatdlg->textview_output));

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buff_input), &start_in, &end_in);
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buff_output), &start_out,
		&end_out);

	text_msg = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buff_input), &start_in,
		&end_in, FALSE);

	char* msg_title;
	if (text_msg == NULL || strlen(text_msg) == 0)
	{
		printf("no message\n");
		return;
	}
	else
	{
		// 获取消息数据, 封装, 发送
		char* send_buf, * psend;
		int send_size;

		if (!w_chatdlg->group) // 单对单消息
		{
			send_size = strlen(text_msg) + strlen(PROTOCOL_RECV_MSG)
				+ sizeof(my_id);
			send_buf = malloc(send_size);
			memset(send_buf, 0, send_size);
			psend = send_buf;
			char* pro_buf = strdup(PROTOCOL_RECV_MSG);
			memcpy(send_buf, pro_buf, strlen(PROTOCOL_RECV_MSG));
			psend += strlen(PROTOCOL_RECV_MSG);

			memcpy(psend, (char*)text_title, sizeof(my_id));
			psend += sizeof(my_id);
			memcpy(psend, (char*)text_msg, strlen(text_msg));
		}
		else if (w_chatdlg->group) // 群消息
		{
			send_size = strlen(text_msg) + strlen(PROTOCOL_RECV_GROUP_MSG) + 2
				* sizeof(my_id);
			send_buf = malloc(send_size);
			memset(send_buf, 0, send_size);
			psend = send_buf;
			char* pro_buf = strdup(PROTOCOL_RECV_GROUP_MSG);
			memcpy(send_buf, pro_buf, strlen(PROTOCOL_RECV_GROUP_MSG));
			psend += strlen(PROTOCOL_RECV_GROUP_MSG);

			// 发送到哪个群 id
			memcpy(psend, (char*)text_title, sizeof(my_id));
			psend += sizeof(my_id);

			// 哪个 id 发的消息
			memcpy(psend, (char*)my_id, sizeof(my_id));
			psend += sizeof(my_id);

			memcpy(psend, (char*)text_msg, strlen(text_msg));
		}

		// 发送给服务器消息, 通过服务器转发
		send_msg(send_buf, send_size);

		// 清除输入 text view 数据
		gtk_text_buffer_delete(GTK_TEXT_BUFFER(buff_input), &start_in, &end_in);

		char* time_str = get_cur_time();
		msg_title = g_strdup_printf("%s  %s%s", my_id, time_str, "\n");

		// 插入文本到消息接收 text view
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out,
			msg_title, strlen(msg_title));
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out,
			text_msg, strlen(text_msg));
		gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, "\n",
			strlen("\n"));

		scroll_textview((char*)text_title);

		// 写入聊天记录文件
		char* history_file = g_strdup_printf("%s/%s.txt", my_id, text_title);
		FILE* fp = fopen(history_file, "ab");
		fseek(fp, 0L, SEEK_END);
		fprintf(fp, "%s", g_locale_from_utf8(msg_title, strlen(msg_title),
			NULL, NULL, NULL));
		fprintf(fp, "%s\n\n", g_locale_from_utf8(text_msg, strlen(text_msg),
			NULL, NULL, NULL));
		fclose(fp);
	}
}

void receive_text_msg(char* text_title, char* id_from, int face_num,
	gboolean group, char* text_msg, int msg_len)
{
	// 不重复建立聊天对话窗口, 已经建立的通过 g_datalist_get_data 获得窗口
	Widgets_Chat* w_chatdlg =
		(Widgets_Chat*)g_datalist_get_data(&widget_chat_dlg, text_title);
	if (w_chatdlg == NULL)
	{
		w_chatdlg = create_chat_dlg(text_title, face_num, group);
		g_datalist_set_data(&widget_chat_dlg, text_title, w_chatdlg);
	}
	else
		gtk_widget_show(w_chatdlg->chat_dlg);

	// 显示消息
	GtkTextIter start_out, end_out;

	GtkTextBuffer* buff_output;
	buff_output = gtk_text_view_get_buffer(
		GTK_TEXT_VIEW(w_chatdlg->textview_output));

	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buff_output), &start_out,
		&end_out);

	char* time_str = get_cur_time();
	char* msg_title = g_strdup_printf("%s  %s%s", id_from, time_str, "\n");

	// 插入文本到消息接收 text view
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, msg_title,
		strlen(msg_title));
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, text_msg,
		msg_len);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buff_output), &end_out, "\n",
		strlen("\n"));

	scroll_textview(text_title);

	// 写入聊天记录文件
	char* history_file = g_strdup_printf("%s/%s.txt", my_id, text_title);
	FILE* fp = fopen(history_file, "ab");
	fseek(fp, 0L, SEEK_END);
	fprintf(fp, "%s", g_locale_from_utf8(msg_title, strlen(msg_title), NULL,
		NULL, NULL));
	fprintf(fp, "%s\n\n", g_locale_from_utf8(text_msg, msg_len, NULL, NULL,
		NULL));
	fclose(fp);
}

void scroll_textview(char* text_title)
{
	Widgets_Chat* w_chatdlg =
		(Widgets_Chat*)g_datalist_get_data(&widget_chat_dlg, text_title);
	GtkWidget* textview = w_chatdlg->textview_output;

	GtkTextBuffer* buffer;
	GtkTextIter iter;
	GtkTextMark* mark;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_end_iter(buffer, &iter);
	gtk_text_iter_set_line_offset(&iter, 0);
	mark = gtk_text_buffer_get_mark(buffer, "scroll");
	gtk_text_buffer_move_mark(buffer, mark, &iter);

	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(textview), mark);
}

void close_dlg(GtkWidget* widget, gpointer data)
{
	GtkWidget* chatdlg = (GtkWidget*)data;
	const gchar* text_title = gtk_window_get_title(GTK_WINDOW(chatdlg));

	// 设置聊天窗口集所关联的数据, 释放窗口数据, 下次再双击 tree item 的时候可以再创建聊天窗口
	g_datalist_set_data(&widget_chat_dlg, text_title, NULL);
	gtk_widget_destroy(chatdlg);
}
