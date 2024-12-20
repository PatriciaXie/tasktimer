#include <gtk/gtk.h>
#include "gui.h"
#include "file.h"
#include "ttimer.h"

gboolean close_welcome_window(GtkWidget *window) {

    gtk_widget_destroy(window); // 销毁窗口
    gtk_main_quit(); // 退出主循环
    return FALSE; // 返回 FALSE 以停止定时器
}

static void activate(GtkApplication *app, gpointer user_data0) {

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "icon.ico", NULL);

    // // 欢迎界面
    // GtkWidget *welcome_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // // gtk_window_set_title(GTK_WINDOW(welcome_window), "Welcome");
    // gtk_window_set_decorated(GTK_WINDOW(welcome_window), FALSE); // 去除标题栏
    // gtk_window_set_default_size(GTK_WINDOW(welcome_window), 350, 225);
    // gtk_window_set_position(GTK_WINDOW(welcome_window), GTK_WIN_POS_CENTER);
    // gtk_window_set_resizable(GTK_WINDOW(welcome_window), FALSE); // 设置窗口不可调整大小
    //
    // GtkWidget *image = gtk_image_new_from_file("splash.png"); // 加载图片
    // gtk_container_add(GTK_CONTAINER(welcome_window), image); // 添加图片到窗口
    // gtk_widget_show_all(welcome_window); // 显示窗口和图片
    // // 设置定时器，2秒后关闭欢迎窗口
    // g_timeout_add_seconds(1, (GSourceFunc)close_welcome_window, welcome_window);




    char *setting_filename = "setting.json";
    char *task_filename =  "task.json";
    char *jsonString_setting = readFile(setting_filename);
    char *jsonString_task = readFile(task_filename);

    TTimerSetting *setting = parseSetting(jsonString_setting);
    TTimerTask **all_task = parseTask(jsonString_task);

    // 创建一个窗口

    TTimerData *user_data = g_new0(TTimerData, 1);
    user_data->window = window;
    user_data->setting = setting;
    user_data->all_task = all_task;
    user_data->detail_time_flag = false;
    user_data->detail_week_flag = false;

    char buffer[100];
    sprintf(buffer, "%s%s v%s", setting->change_unsaved ? "*":"",setting->app_name, setting->version);
    gtk_window_set_title(GTK_WINDOW(window), buffer);
    const int width = 1600;
    const int height = 900;
    gtk_window_set_default_size(GTK_WINDOW(window), width, height+50);
    gtk_window_maximize(GTK_WINDOW(window));

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // 1. 工具栏
    GtkWidget *toolbar = gtk_toolbar_new();// 创建工具栏
    GtkToolItem *toolbar_open_button = gtk_tool_button_new(NULL, "reload"); // 创建打开按钮
    GtkToolItem *toolbar_save_button = gtk_tool_button_new(NULL, "save"); // 创建保存按钮
    GtkToolItem *toolbar_about_button = gtk_tool_button_new(NULL, "about"); // 创建关于按钮
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbar_open_button, -1); // 将打开按钮放进工具栏
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbar_save_button, -1); // 将保存按钮放进工具栏
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolbar_about_button, -1); // 将关于按钮放进工具栏
    g_signal_connect(toolbar_open_button, "clicked", G_CALLBACK(on_clicked_toolbar_open_button), user_data); // 连接保存按钮函数
    g_signal_connect(toolbar_save_button, "clicked", G_CALLBACK(on_clicked_toolbar_save_button), user_data); // 连接保存按钮函数
    g_signal_connect(toolbar_about_button, "clicked", G_CALLBACK(on_clicked_toolbar_about_button), user_data); // 连接关于按钮函数
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0); // 将工具栏放进主box

    // 2. 主窗口界面
    GtkWidget *v_paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL); // 创建垂直分隔控件
    GtkWidget *top_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL); // 创建上部分水平分隔
    GtkWidget *bottom_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL); // 创建下部分水平分隔

    // 2.1 task界面
    GtkWidget *task_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // task界面用上下两个box，上面box是按钮，下面box是表格
    // 表格box
    // 7 列，分别是0id(%d)、1状态(%s)、2类别(%s)、3任务名(%s)、4重要性(%d)、5紧急度(%d)、6进度(%d)、7次数(%d)，8已用时(%s)
    GtkListStore *list_store = gtk_list_store_new(9, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
    user_data->list_store = list_store;
    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store)); // 创建 TreeView 控件，并与模型绑定
    user_data->treeview = treeview;

    // 0添加 "id" 列
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("id", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 1添加 "state" 列
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("state", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 2添加 "class" 列
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("class", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 3添加 "task" 列
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("task", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 4添加 "importance" 列
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("importance", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 5添加 "emergency" 列
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("emergency", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 6添加 "progress" 列
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("progress", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, NULL, NULL); // 完成的话就是灰色背景
    // 7添加 "times" 列
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("times", renderer, "text", 7, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    // 8添加 "spent" 列
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("spent", renderer, "text", 8, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    g_signal_connect(treeview, "button-press-event", G_CALLBACK(on_task_treeview_right_click), user_data);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_GRID_LINES_BOTH);
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);
    gtk_box_pack_start(GTK_BOX(task_box), scrolled_window, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // 按钮box
    GtkWidget *task_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *start_task_button = gtk_button_new_with_label("start");
    g_signal_connect(start_task_button, "clicked", G_CALLBACK(on_start_task_button_clicked), user_data);
    GtkWidget *stop_task_button = gtk_button_new_with_label("stop");
    g_signal_connect(stop_task_button, "clicked", G_CALLBACK(on_stop_task_button_clicked), user_data);
    GtkWidget *add_task_button = gtk_button_new_with_label("add");
    g_signal_connect(add_task_button, "clicked", G_CALLBACK(on_add_task_button_clicked), user_data);
    GtkWidget *edit_task_button = gtk_button_new_with_label("edit");
    g_signal_connect(edit_task_button, "clicked", G_CALLBACK(on_edit_task_button_clicked), user_data);
    GtkWidget *delete_task_button = gtk_button_new_with_label("delete");
    g_signal_connect(delete_task_button, "clicked", G_CALLBACK(on_delete_task_button_clicked), user_data);
    GtkWidget *detail_task_button = gtk_button_new_with_label("detail");
    g_signal_connect(detail_task_button, "clicked", G_CALLBACK(on_detail_task_button_clicked), user_data);

    // 将按钮添加到按钮框
    gtk_box_pack_start(GTK_BOX(task_button_box), start_task_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_button_box), stop_task_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_button_box), add_task_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_button_box), edit_task_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_button_box), delete_task_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_button_box), detail_task_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_box), task_button_box, FALSE, FALSE, 0);

    // 2.2 today界面
    GtkWidget *today_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // task界面用上下两个box，上面box是按钮，下面box是表格
    // 表格box
    // 是0id(%d)、1用时(%d)、2周几，3开始时间(%s)、4结束时间(%s)、5任务名(%s)、6note(%s)、8执行的第几次
    GtkListStore *list_store_today = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
    user_data->list_store_today = list_store_today;
    GtkWidget *treeview_today = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store_today)); // 创建 TreeView 控件，并与模型绑定
    user_data->treeview_today = treeview_today;
    // 2开始时间(%s)
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("week", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);
    // 3开始时间(%s)
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("start", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);
    // 4结束时间(%s)
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("stop", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);// 1用时(%d)
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("spent", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);
    // 0添加 "id" 列
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("id", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);
    // 5任务名
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("task", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);
    // 6note
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("no.", renderer, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);
    // 6note
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("note", renderer, "text", 7, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_today), column);


    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(treeview_today), GTK_TREE_VIEW_GRID_LINES_BOTH);
    GtkWidget *scrolled_window_today = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window_today), treeview_today);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_today), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);


    // 按钮box
    GtkWidget *today_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *today_today_button = gtk_button_new_with_label("today");
    g_signal_connect(today_today_button, "clicked", G_CALLBACK(on_today_today_button_clicked), user_data);
    GtkWidget *today_week_button = gtk_button_new_with_label("week");
    g_signal_connect(today_week_button, "clicked", G_CALLBACK(on_today_week_button_clicked), user_data);


    // 将按钮添加到按钮框
    gtk_box_pack_start(GTK_BOX(today_button_box), today_today_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(today_button_box), today_week_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(today_box), today_button_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(today_box), scrolled_window_today, TRUE, TRUE, 0);

    // 2.3 detail界面
    GtkWidget *detail_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // task界面用上下两个box，上面box是按钮，下面box是表格
    // 表格box
    // 是0执行的第几次(%d)、1用时(%d)、2开始时间(%s)、3结束时间(%s)、4note(%s)
    GtkListStore *list_store_detail = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    user_data->list_store_detail = list_store_detail;
    GtkWidget *treeview_detail = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store_detail)); // 创建 TreeView 控件，并与模型绑定
    user_data->treeview_detail = treeview_detail;
    // 0添加 "id" 列
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("no.", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_detail), column);
    // 1用时(%d)
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("spent", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_detail), column);
    // 2开始时间(%s)
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("start", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_detail), column);
    // 3开始时间(%s)
    renderer = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column = gtk_tree_view_column_new_with_attributes("stop", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_detail), column);
    // 4结束时间(%s)
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("note", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_detail), column);


    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(treeview_detail), GTK_TREE_VIEW_GRID_LINES_BOTH);
    GtkWidget *scrolled_window_detail = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window_detail), treeview_detail);
    gtk_box_pack_start(GTK_BOX(detail_box), scrolled_window_detail, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_detail), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);


    // 按钮box
    GtkWidget *detail_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *detail_revise_button = gtk_button_new_with_label("edit");
    g_signal_connect(detail_revise_button, "clicked", G_CALLBACK(on_detail_revise_button_clicked), user_data);
    GtkWidget *detail_delete_button = gtk_button_new_with_label("delete");
    g_signal_connect(detail_delete_button, "clicked", G_CALLBACK(on_detail_delete_button_clicked), user_data);
    GtkWidget *start_calendar = gtk_calendar_new();
    // gtk_calendar_set_display_options(GTK_CALENDAR(start_calendar), GTK_CALENDAR_SHOW_HEADING);
    time_t detail_start_time = time(NULL);
    user_data->detail_start_time = detail_start_time;
    g_signal_connect(start_calendar, "day-selected", G_CALLBACK(on_detail_start_calendar_selected), user_data);
    g_signal_connect(start_calendar, "day-selected-double-click", G_CALLBACK(on_detail_week_calendar_selected), user_data);
    // 将按钮添加到按钮框
    gtk_box_pack_start(GTK_BOX(detail_button_box), detail_revise_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(detail_button_box), detail_delete_button, FALSE, FALSE, 0);
    // gtk_box_pack_start(GTK_BOX(detail_button_box), start_calendar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(detail_box), detail_button_box, FALSE, FALSE, 0);


    // 创建四个区域（标签作为占位符）
    // GtkWidget *area1 = gtk_label_new("任务");
    // GtkWidget *area2 = gtk_label_new("今日/本周");
    // GtkWidget *area3 = gtk_label_new("执行");
    GtkWidget *area4 = gtk_label_new("calendar");

    // 在右下角box中增加archive列表
    GtkWidget *calendar_archive_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(calendar_archive_box), start_calendar, FALSE, FALSE, 0);
    // 表格box
    // 7 列，分别是0id(%d)、1状态(%s)、2类别(%s)、3任务名(%s)、4重要性(%d)、5紧急度(%d)、6进度(%d)、7次数(%d)，8已用时(%s)
    GtkListStore *list_store_archive = gtk_list_store_new(9, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
    user_data->list_store_archive = list_store_archive;
    GtkWidget *treeview_archive = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store_archive)); // 创建 TreeView 控件，并与模型绑定
    user_data->treeview_archive = treeview_archive;

    // 0添加 "id" 列
    GtkCellRenderer *renderer_archive = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    GtkTreeViewColumn *column_archive = gtk_tree_view_column_new_with_attributes("id", renderer_archive, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 1添加 "state" 列
    renderer_archive = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column_archive = gtk_tree_view_column_new_with_attributes("state", renderer_archive, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 2添加 "class" 列
    renderer_archive = gtk_cell_renderer_text_new(); // 创建列并设置渲染器
    column_archive = gtk_tree_view_column_new_with_attributes("class", renderer_archive, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    gtk_tree_view_column_set_cell_data_func(column_archive, renderer_archive, cell_data_func2, NULL, NULL); // 完成的话就是灰色背景
    // 3添加 "task" 列
    renderer_archive = gtk_cell_renderer_text_new();
    column_archive = gtk_tree_view_column_new_with_attributes("task", renderer_archive, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 4添加 "importance" 列
    renderer_archive = gtk_cell_renderer_text_new();
    column_archive = gtk_tree_view_column_new_with_attributes("importance", renderer_archive, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 5添加 "emergency" 列
    renderer_archive = gtk_cell_renderer_text_new();
    column_archive = gtk_tree_view_column_new_with_attributes("emergency", renderer_archive, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 6添加 "progress" 列
    renderer_archive = gtk_cell_renderer_text_new();
    column_archive = gtk_tree_view_column_new_with_attributes("progress", renderer_archive, "text", 6, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 7添加 "times" 列
    renderer_archive = gtk_cell_renderer_text_new();
    column_archive = gtk_tree_view_column_new_with_attributes("times", renderer_archive, "text", 7, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    // 8添加 "spent" 列
    renderer_archive = gtk_cell_renderer_text_new();
    column_archive = gtk_tree_view_column_new_with_attributes("spent", renderer_archive, "text", 8, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_archive), column_archive);
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(treeview_archive), GTK_TREE_VIEW_GRID_LINES_BOTH);

    // 右键
    g_signal_connect(treeview_archive, "button-press-event", G_CALLBACK(on_archive_treeview_right_click), user_data);


    GtkWidget *scrolled_window_archive = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window_archive), treeview_archive);
    gtk_box_pack_start(GTK_BOX(calendar_archive_box), scrolled_window_archive, TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_archive), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);



    // 将区域添加进paned中
    gtk_paned_add1(GTK_PANED(top_paned), task_box); // 将task表格放进上方水平分隔左侧
    gtk_paned_add2(GTK_PANED(top_paned), detail_box); // 将放进上方水平分隔右侧
    gtk_paned_add1(GTK_PANED(bottom_paned), today_box); // 将放进下方水平分隔左侧
    gtk_paned_add2(GTK_PANED(bottom_paned), calendar_archive_box); // 将放进下方水平分隔右侧
    // gtk_paned_add2(GTK_PANED(bottom_paned), area4); // 将放进下方水平分隔右侧gtk_container_add(GTK_CONTAINER(window), area4);
    gtk_paned_add1(GTK_PANED(v_paned), top_paned); // 将上面的水平分隔添加到垂直分隔
    gtk_paned_add2(GTK_PANED(v_paned), bottom_paned); // 将下面的水平分隔添加到垂直分隔

    // 初始化paned大小
    int left_width = (int)((double)width * 0.55);
    int right_width = width - left_width;
    gtk_paned_set_position(GTK_PANED(v_paned), height/2);  // 设置垂直分隔控件，均分上下区域
    gtk_paned_set_position(GTK_PANED(top_paned), left_width); // 设置水平分隔控件，均分左右区域
    gtk_paned_set_position(GTK_PANED(bottom_paned), left_width); // 设置水平分隔控件，均分左右区域

    gtk_box_pack_start(GTK_BOX(main_box), v_paned, TRUE, TRUE, 0); // 将垂直分隔放进主box
    gtk_container_add(GTK_CONTAINER(window), main_box); // 将主box放进主窗口

    load_task(user_data);
    // g_signal_connect(treeview, "row-activated", G_CALLBACK(test_signal), user_data);

    // g_signal_connect(window, "delete-event", G_CALLBACK(on_close_event), user_data);
    // 显示所有控件
    gtk_widget_show_all(window);
}



int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("xienianhao.ttimer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}