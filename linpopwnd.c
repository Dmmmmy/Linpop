#include "linpop.h"
#include "linpopwnd.h"
#include <pango/pango.h>
#include <gdk/gdk.h>

extern GData *widget_chat_dlg;// 聊天窗口集, 用于获取已经建立的聊天窗口

GArray *g_array_user = NULL; // 储存用户信息的全局变量
//GtkWidget *login_window, *linpop_window;登录窗口，主窗口的定义放在linpopwnd.h
GtkWidget *vbox_login; //存放登录界面的图片和信息label
int utimer_login, utimer_count = 0;
GtkWidget *g_treeview; //建立分组的借口

extern GtkWidget *button_image, *button_preview; //主界面图片按键——显示更改用户信息界面 显示按键
extern GtkWidget *image_user, *image_user_old, *image_user_new; //用户头像，用户旧头像，用户新头像
extern gchar *str_image_user,*str_image_user_new;

GtkWidget *entry_user_id, *entry_pass; // 用户 id 和密码输入 entry
char my_id[15];

int socket_ret;// socket 连接 server 返回值

gboolean thread_quit = FALSE; // 主窗口退出时退出 recv_msg thread


// 同步所用, 主窗口创建后再收到用户在线数据
int gnum = 0;
pthread_mutex_t mutex; // 互斥量
pthread_cond_t cond; // 条件变量



gboolean gtk_wnd_init(int argc, char *argv[]) //主窗口初始化
{
        // 同步所用
        pthread_mutex_init(&mutex, NULL); // 互斥初始化
        pthread_cond_init(&cond, NULL); // 条件变量初始化

        gtk_init(&argc, &argv);

        login_window = create_login_wnd(); //创建登录窗口

        if (!g_thread_supported())
                g_thread_init(NULL);

        gdk_threads_init();

        // 关键之处, gdk_threads_enter 和 gdk_threads_leave, 否则在线程中创建窗口会崩溃

        gdk_threads_enter();
        gtk_main();
        gdk_threads_leave();

        return TRUE;
}


GtkWidget *create_login_wnd() //创建登录窗口
{
        //      GtkWidget *entry_user_id, *entry_pass; // 提取到全局变量
        GtkWidget *label;
        GtkWidget *button_login;
        GtkWidget *hbox_user_id, *hbox_pass, *hbox_button;
        GtkWidget *align;
        GtkWidget *image;


        // 创建登录主窗口
        login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(login_window), "Linpop");//标题
        gtk_widget_set_size_request(login_window, 500, 300);//最小的尺寸
        gtk_container_set_border_width(GTK_CONTAINER (login_window), 2);//边界宽度
        gtk_window_set_position(GTK_WINDOW(login_window), GTK_WIN_POS_CENTER_ALWAYS);//窗口在显示器里的位置

        vbox_login = gtk_vbox_new(FALSE, 15); //新建一个vbox
        gtk_container_set_border_width(GTK_CONTAINER (vbox_login), 0); //没有边界宽度


        image = gtk_image_new_from_file("/mnt/hgfs/Image/25.jpg"); //找到这个图片并存放 
        gtk_box_pack_start(GTK_BOX(vbox_login),image,FALSE,FALSE,0); //把图片放进vbox里面



        label = gtk_label_new("用户账号:   "); //设置标签
        PangoFontDescription *font1 = pango_font_description_from_string("Simsun 20");
        gtk_widget_modify_font (label, font1); //修改标签字体和大小



        entry_user_id = gtk_entry_new(); //键入ID
        gtk_entry_set_text(GTK_ENTRY (entry_user_id), "1120170001"); //默认值
        hbox_user_id = gtk_hbox_new(FALSE, 0); //键入ID的框
        gtk_box_pack_start(GTK_BOX (hbox_user_id), label, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX (hbox_user_id), entry_user_id, TRUE, FALSE, 0); //分别把标签和键入功能放入框中


        //同理如上，制作password的标签和框
        label = gtk_label_new("用户密码:   ");
        entry_pass = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY (entry_pass), "000000");
        gtk_entry_set_visibility(GTK_ENTRY (entry_pass), FALSE);
        gtk_entry_set_invisible_char(GTK_ENTRY (entry_pass), '*');


        //修改标签password的字体和大小
        PangoFontDescription *font2 = pango_font_description_from_string("Simsun 20");
        gtk_widget_modify_font (label, font2);


        change_background(login_window, 500, 300, "/mnt/hgfs/Image/0.jpg"); //修改背景图片



        // 响应回车消息
        g_signal_connect (G_OBJECT (entry_user_id), "activate",
                        G_CALLBACK (enter_callback), NULL);
        g_signal_connect (G_OBJECT (entry_pass), "activate",
                        G_CALLBACK (enter_callback), NULL);

        hbox_pass = gtk_hbox_new(FALSE, 5);
        gtk_box_pack_start(GTK_BOX (hbox_pass), label, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX (hbox_pass), entry_pass, TRUE, FALSE, 0);

        button_login = gtk_button_new_with_label("   Login   "); //制作login按键
        g_signal_connect(G_OBJECT(button_login), "clicked",
                        G_CALLBACK(login), NULL); //点击后调用void login函数
        hbox_button = gtk_hbox_new(FALSE, 2);

        // 创建一个居中对齐的对象
        align = gtk_alignment_new(0.5, 0.7, 0, 0);
        gtk_box_pack_start(GTK_BOX (hbox_button), align, TRUE, TRUE, 0);
        gtk_widget_show(align);
        gtk_container_add(GTK_CONTAINER (align), button_login);

        gtk_box_pack_start(GTK_BOX (vbox_login), hbox_user_id, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX (vbox_login), hbox_pass, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX (vbox_login), hbox_button, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(login_window), vbox_login);

        g_signal_connect (G_OBJECT (login_window), "destroy",
                        G_CALLBACK (gtk_main_quit), NULL);

        gtk_widget_show_all(login_window);

        return login_window;

}

GtkWidget *create_linpopwnd()
{

        GtkWidget *label_info;
        // 便于修改头像
        GtkWidget *scrolled_win; // tree view 自动滚动
        GtkWidget *vbox_main, *hbox_head;

        // 创建主窗口
        linpop_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(linpop_window), "Linpop");//主窗口标题
        g_signal_connect (G_OBJECT (linpop_window), "destroy",
                        G_CALLBACK (linpop_quit), NULL); //点击结束，调用linpop_quit函数
        gtk_widget_set_size_request(linpop_window, 300, 625); //主窗口大小
        gtk_window_set_position(GTK_WINDOW(linpop_window), GTK_WIN_POS_CENTER);//位置 居中

        vbox_main = gtk_vbox_new(FALSE, 10);
        hbox_head = gtk_hbox_new(FALSE, 10);
        //建两个vbox


        change_background(linpop_window, 300, 625, "/mnt/hgfs/Image/2333.jpg"); //更改主窗口背景



        // 用户头像
        image_user = gtk_image_new_from_file("/mnt/hgfs/Image/tx/1.jpg");
        // 保存原来头像信息
        str_image_user = g_strdup("/mnt/hgfs/Image/tx/1.jpg");
        button_image = gtk_button_new();
        label_info = gtk_label_new(
                        "Welcome to use this app! \n       Have fun every day!"); //创建一个文字label
        gtk_container_add(GTK_CONTAINER(button_image), (GtkWidget *) image_user); //创建带头像的按键
        g_signal_connect(G_OBJECT(button_image), "clicked",
                        G_CALLBACK(set_user_info), NULL);//点击后调用set_user_info函数，出现更改用户信息界面
        gtk_widget_set_size_request(button_image, 64, 64);//设置头像按键大小

        // 头像和文本信息
        gtk_box_pack_start(GTK_BOX(hbox_head), button_image, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox_head), label_info, FALSE, FALSE, 5);//按键和label放入顶部盒子当中

        // 创建树视图
        g_treeview = create_tree_view();
        setup_tree_view_model(g_treeview, NULL, "");

        scrolled_win = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_win),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);//自动
        gtk_container_add(GTK_CONTAINER (scrolled_win), g_treeview);

        gtk_container_add(GTK_CONTAINER(linpop_window), vbox_main);
        gtk_box_pack_start(GTK_BOX(vbox_main), hbox_head, FALSE, FALSE, 2);//把hbox_head放入vbox_main中
        gtk_box_pack_start(GTK_BOX(vbox_main), scrolled_win, TRUE, TRUE, 1);//把可以滚动放入

        // 显示主窗口
        gtk_widget_show_all(linpop_window);

        return linpop_window;


}



void enter_callback(GtkWidget *widget, GtkWidget *entry)
{
        const char *text_id, *text_pass;
        text_id = gtk_entry_get_text(GTK_ENTRY (entry_user_id));
        text_pass = gtk_entry_get_text(GTK_ENTRY (entry_pass));

        login(login_window, NULL);
}

void login(GtkWidget *widget, gpointer data)
{
        const char *text_id, *text_pass;
        text_id = gtk_entry_get_text(GTK_ENTRY (entry_user_id));
        text_pass = gtk_entry_get_text(GTK_ENTRY (entry_pass));

        strcpy(my_id, text_id);

        if (text_id == NULL || strlen(text_id) == 0)
        {
                show_info_msg_box(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK,
                                "请输入您的用户账号!");
                return;
        }
        else if (text_pass == NULL || strlen(text_pass) == 0)
        {
                show_info_msg_box(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK,
                                "请输入您的用户密码!");
                return;
        }

        // 连接服务器, 进行登录

        char server_ip[16];
        memset(server_ip, 0, sizeof(server_ip));
        int server_port;
        if (access("server.conf", 0) == -1)
        {
                show_info_msg_box(GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                "文件 server.conf 不存在!");
                return;
        }

        FILE *fp = fopen("server.conf", "rb");
        fscanf(fp, "%s %d", server_ip, &server_port);
        fclose(fp);

        socket_ret = init_socket(server_ip, server_port);

        // 要求服务器传回上线用户列表
        char *pro_buf = g_strdup(PROTOCOL_ONLINE); // 要求服务器传回在线用户
        gchar *msg = g_strconcat(pro_buf, text_id, NULL);
        socket_ret = send_msg(msg, strlen(msg));

        GtkWidget *progress, *image;
        GtkWidget *vbox_progress, *align, *align1, *vbox_image;

        gtk_container_remove(GTK_CONTAINER (login_window), vbox_login);
        vbox_progress = gtk_vbox_new(FALSE, 0);
        gtk_container_set_border_width(GTK_CONTAINER (vbox_progress), 0);
        gtk_container_add(GTK_CONTAINER (login_window), vbox_progress);
        gtk_widget_show(vbox_progress);
	image = gtk_image_new_from_file("/mnt/hgfs/Image/25.jpg");
        // 创建一个居中对齐的对象
	gtk_widget_show(image);
        align = gtk_alignment_new(0.5, 0.2, 0, 0);
	align1 = gtk_alignment_new(0.5, 0.7, 0, 0);
        gtk_box_pack_start(GTK_BOX (vbox_progress), align1, TRUE, TRUE, 5);
        gtk_widget_show(align);
	gtk_box_pack_start(GTK_BOX (vbox_progress), align, TRUE, TRUE, 5);
        gtk_widget_show(align1);
        progress = gtk_progress_bar_new();
        gtk_widget_set_size_request(progress, 180, 20);
        utimer_login = gtk_timeout_add(60, (GtkFunction) progress_login_timeout,
                        progress);
	gtk_container_add(GTK_CONTAINER (align1), image);
        gtk_container_add(GTK_CONTAINER (align), progress);
	gtk_widget_show(vbox_image);
        gtk_widget_show(progress);
	
        

}
gboolean progress_login_timeout(GtkProgressBar *progress)
{
        utimer_count++;
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR (progress));

        if (utimer_count >= 20 && socket_ret >= 0)
        {
                gtk_timeout_remove(utimer_login);
                gtk_widget_hide_all(login_window);

                pthread_mutex_lock(&mutex); // 获取互斥锁

                // 创建主窗口
                create_linpopwnd();
                gnum++;
                if (gnum == 1)
                        pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex); // 释放互斥锁

                if (access(my_id, 0) == -1) // 创建一个以自己 id 为名的文件夹用于保存聊天记录
                {
                        if (mkdir(my_id, 0777))//如果不存在就用mkdir函数来创建
                        {
                                perror("creat folder failed");
                        }
                }
                return FALSE;
        }
        else if (utimer_count >= 50 && socket_ret == -1)
        {
                gdk_threads_enter();
                show_info_msg_box(GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                "连接服务器失败!");
                gdk_threads_leave();
                // 不这样加上就会死机

                linpop_quit(NULL, NULL);

                return FALSE;
        }

        return TRUE;
}
void set_user_info(GtkWidget *widget, gpointer data)//接口
{
        GtkWidget *dialog = create_user_info_dlg();

        GtkResponseType result = gtk_dialog_run(GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK)
        {
                // 更新头像
                gtk_container_remove(GTK_CONTAINER(button_image), image_user);
                image_user_new = gtk_image_new_from_file(str_image_user_new);
                image_user = image_user_new;
                gtk_container_add(GTK_CONTAINER(button_image), image_user_new);
                gtk_widget_show_all(button_image);
        }

        gtk_widget_destroy(dialog);
}

int show_info_msg_box(GtkMessageType msg_type, GtkButtonsType btn_type,
                gchar *text_error)//接口
{
        GtkWidget *MSGBox = gtk_message_dialog_new(NULL,
                        GTK_DIALOG_DESTROY_WITH_PARENT, msg_type, btn_type, "%s",
                        text_error);
        int ret = gtk_dialog_run(GTK_DIALOG(MSGBox));
        gtk_widget_destroy(MSGBox);
        return ret;
}

void linpop_quit(GtkWidget *widget, gpointer data)
{
        //退出主窗口
        gtk_timeout_remove(utimer_login);
        thread_quit = TRUE;
        gtk_main_quit();
}

