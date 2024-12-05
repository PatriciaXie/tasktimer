#include <gtk/gtk.h>
#include "gui.h"
#include "file.h"
#include "ttimer.h"

// char global_buffer[200];

void waiting_debug(GtkWidget *widget, const gpointer user_data) {

    TTimerData *data = (TTimerData *)user_data;
    TTimerSetting *setting = data->setting;
    GtkWidget *main_window = data->window;

    // Create the widgets
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Sorry",
                                                    GTK_WINDOW(main_window),
                                                    flags,
                                                    ("_OK"),
                                                    GTK_RESPONSE_NONE,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("This function have not been finished.");
    gtk_widget_set_size_request(label, 300, 100);

    // Ensure that the dialog box is destroyed when the user responds
    g_signal_connect_swapped (dialog,
                              "response",
                              G_CALLBACK (gtk_widget_destroy),
                              dialog);

    // Add the label, and show everything we’ve added
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
}


// gpointer data是setting
void on_clicked_toolbar_about_button(GtkWidget *widget, const gpointer user_data) {

    TTimerData *data = (TTimerData *)user_data;
    TTimerSetting *setting = data->setting;
    GtkWidget *main_window = data->window;

    char str[100];
    sprintf(str, "\n%s v%s\n\nby %s\n%s", setting->app_name, setting->version, setting->author, setting->date);

    // Create the widgets
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
    GtkWidget *dialog = gtk_dialog_new_with_buttons("About",
                                                    GTK_WINDOW(main_window),
                                                    flags,
                                                    ("_OK"),
                                                    GTK_RESPONSE_NONE,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new(str);
    gtk_widget_set_size_request(label, 300, 200);

    // Ensure that the dialog box is destroyed when the user responds
    g_signal_connect_swapped (dialog,
                              "response",
                              G_CALLBACK (gtk_widget_destroy),
                              dialog);

    // Add the label, and show everything we’ve added
    gtk_container_add(GTK_CONTAINER(content_area), label);
    gtk_widget_show_all(dialog);
}

void on_clicked_toolbar_save_button(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *main_window = data->window;

    if (!setting->change_unsaved) {
        return;
    }

    if (setting->change_unsaved) {
        writeTask("task.json", all_task, setting->num_task);
        writeSetting("setting.json", setting);
        setting->change_unsaved = false;
        printf("file saved\n");
        char buffer[100];
        sprintf(buffer, "%s v%s", setting->app_name, setting->version);
        gtk_window_set_title(GTK_WINDOW(main_window), buffer);
    }
}

void show_tasks(gpointer user_data, GtkListStore *list_store) {


}
void load_task(gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *main_window = data->window;

    for (int i = 0; i < setting->num_task; i++) {
        for (int j = 0; j < all_task[i]->num_execution; j++) {
            int real_duration = (int)difftime(all_task[i]->execution_end_time[j], all_task[i]->execution_start_time[j]);
            int saved_duration = all_task[i]->execution_duration[j];
            if ( real_duration != saved_duration ) {
                all_task[i]->execution_duration[j] = real_duration;
                // setting->change_unsaved = true;
                printf("file saved\n");
                char buffer[100];
                sprintf(buffer, "*%s v%s", setting->app_name, setting->version);
                gtk_window_set_title(GTK_WINDOW(main_window), buffer);
            }
        }
    }

    for (int i = 0; i < setting->num_task; i++) {
        TTimerTask *task = all_task[i];
        // 向 ListStore 中添加新的任务
        GtkTreeIter iter;

        if (task->state == 2) {
            continue;
        }
        gtk_list_store_append(list_store, &iter);
        int spent_second = 0;
        for (int j = 0; j < task->num_execution; j++) {
            spent_second = spent_second + task->execution_duration[j];
        }
        char *state;
        switch (task->state) {
            case 0: state = "待办"; break;
            case 1: state = "完成"; break;
            case 2: state = "归档"; break;
            case 3: state = "删除"; break;
            case 4: state = "ing"; break;
            default: state = "未知"; break;
        }

        char buffer2[20];
        int hour = spent_second / (3600);
        int minute = (spent_second - hour * 3600) / 60;
        sprintf(buffer2, "%dh%dm", hour, minute);

        gtk_list_store_set(list_store, &iter,
            0, task->id,
            1, state,
            2, setting->name_category[task->category],
            3, task->name,
            4, task->importance_degree,
            5, task->emergency_degree,
            6, task->plan_progress,
            7, task->num_execution,
            8, buffer2, -1);
    }
}

void on_add_task_button_clicked(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;

    setting->selected_id = -1;
    revise_task(widget, user_data);
    setting = data->setting;
    all_task = data->all_task;

    int idx = get_idx(all_task, setting->num_task, setting->selected_id);
    TTimerTask *task = select_task_idx(setting, all_task, idx);

    GtkTreeIter iter;
    gtk_list_store_append(list_store, &iter);
    char *state;
    switch (task->state) {
        case 0: state = "待办"; break;
        case 1: state = "完成"; break;
        case 2: state = "归档"; break;
        case 3: state = "删除"; break;
        case 4: state = "ing"; break;
        default: state = "未知"; break;
    }
    gtk_list_store_set(list_store, &iter,
        0, task->id,
        1, state,
        2, setting->name_category[task->category],
        3, task->name,
        4, task->importance_degree,
        5, task->emergency_degree,
        6, task->plan_progress,
        7, task->num_execution,
        8, "0h0m", -1);
    setting->change_unsaved = true;
    GtkWidget *main_window = data->window;
    char buffer[100];
    sprintf(buffer, "*%s v%s", setting->app_name, setting->version);
    gtk_window_set_title(GTK_WINDOW(main_window), buffer);
}


static void on_button_toggled(GtkToggleButton *toggle_button, gpointer user_data) {
    // 如果当前按钮是按下状态，其他按钮需要释放
    if (gtk_toggle_button_get_active(toggle_button)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(user_data), FALSE);
    }
}

void revise_task(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview;
    GtkWidget *main_window = data->window;

    TTimerTask *task;
    int idx = get_idx(all_task, setting->num_task, setting->selected_id);
    if (idx == -1) {
        task = (TTimerTask *)malloc(sizeof(TTimerTask));
        task->id = setting->num_task;
        char *name = malloc(100*sizeof(char));
        sprintf(name, "Untitled-%d", task->id);
        task->name = name;
        task->category = 0;
        task->importance_degree = 0;
        task->emergency_degree = 0;
        time_t cur_time;
        time(&cur_time);
        task->create_time = cur_time;
        task->revise_time = cur_time;
        task->plan_start_time = cur_time;
        task->plan_due_time = (cur_time + 3600*24*2);
        task->num_roadmap = 0;
        task->plan_progress = 0;
        task->num_execution = 0;
        task->state = 0;
    } else {
       task = select_task_idx(setting, all_task, idx);
    }

    // 创建一个新的对话框
    GtkWidget *dialog = gtk_dialog_new_with_buttons("revise", GTK_WINDOW(main_window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "OK", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    // gtk_widget_set_size_request(content_area, 400, 400);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); //用一个box装grid

    // 创建任务文本输入框
    GtkWidget *task_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(task_entry), task->name);

    // 创建 importance 单选按钮，三个Toggle按钮
    GtkWidget *importance_button0 = gtk_toggle_button_new_with_label("无关");
    GtkWidget *importance_button1 = gtk_toggle_button_new_with_label("一般");
    GtkWidget *importance_button2 = gtk_toggle_button_new_with_label("重要");
    // 设置初始按钮状态
    switch (task->importance_degree) {
        case 0: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(importance_button0), TRUE);  // 默认选中“待办”按钮
            break;
        }
        case 1: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(importance_button1), TRUE);  // 默认选中“待办”按钮
            break;
        }
        case 2: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(importance_button2), TRUE);  // 默认选中“待办”按钮
            break;
        }
        default: ;
    }
    // 连接信号：当某个按钮的状态变化时，回调 on_button_toggled
    g_signal_connect(importance_button0, "toggled", G_CALLBACK(on_button_toggled), importance_button1);  // 设置互斥
    g_signal_connect(importance_button0, "toggled", G_CALLBACK(on_button_toggled), importance_button2);  // 设置互斥
    g_signal_connect(importance_button1, "toggled", G_CALLBACK(on_button_toggled), importance_button0);
    g_signal_connect(importance_button1, "toggled", G_CALLBACK(on_button_toggled), importance_button2);
    g_signal_connect(importance_button2, "toggled", G_CALLBACK(on_button_toggled), importance_button0);
    g_signal_connect(importance_button2, "toggled", G_CALLBACK(on_button_toggled), importance_button1);
    // 将按钮添加到按钮盒子中
    GtkWidget *importance_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); //用一个box装grid
    gtk_container_add(GTK_CONTAINER(importance_box), importance_button0);
    gtk_container_add(GTK_CONTAINER(importance_box), importance_button1);
    gtk_container_add(GTK_CONTAINER(importance_box), importance_button2);

    // 创建 importance 单选按钮，三个Toggle按钮
    GtkWidget *emergency_button0 = gtk_toggle_button_new_with_label("不慌");
    GtkWidget *emergency_button1 = gtk_toggle_button_new_with_label("一般");
    GtkWidget *emergency_button2 = gtk_toggle_button_new_with_label("紧急");
    // 设置初始按钮状态
    switch (task->emergency_degree) {
        case 0: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(emergency_button0), TRUE);  // 默认选中“待办”按钮
            break;
        }
        case 1: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(emergency_button1), TRUE);  // 默认选中“待办”按钮
            break;
        }
        case 2: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(emergency_button2), TRUE);  // 默认选中“待办”按钮
            break;
        }
        default: ;
    }
    // 连接信号：当某个按钮的状态变化时，回调 on_button_toggled
    g_signal_connect(emergency_button0, "toggled", G_CALLBACK(on_button_toggled), emergency_button1);  // 设置互斥
    g_signal_connect(emergency_button0, "toggled", G_CALLBACK(on_button_toggled), emergency_button2);  // 设置互斥
    g_signal_connect(emergency_button1, "toggled", G_CALLBACK(on_button_toggled), emergency_button0);
    g_signal_connect(emergency_button1, "toggled", G_CALLBACK(on_button_toggled), emergency_button2);
    g_signal_connect(emergency_button2, "toggled", G_CALLBACK(on_button_toggled), emergency_button0);
    g_signal_connect(emergency_button2, "toggled", G_CALLBACK(on_button_toggled), emergency_button1);
    // 将按钮添加到按钮盒子中
    GtkWidget *emergency_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); //用一个box装grid
    gtk_container_add(GTK_CONTAINER(emergency_box), emergency_button0);
    gtk_container_add(GTK_CONTAINER(emergency_box), emergency_button1);
    gtk_container_add(GTK_CONTAINER(emergency_box), emergency_button2);

    // 创建 class 下拉框
    GtkWidget *class_combobox = gtk_combo_box_text_new();
    for (int i = 0; i < setting->num_category; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(class_combobox), setting->name_category[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(class_combobox), task->category);


    // 创建 state 单选按钮，三个Toggle按钮
    GtkWidget *state_button0 = gtk_toggle_button_new_with_label("待办");
    GtkWidget *state_button1 = gtk_toggle_button_new_with_label("归档");
    GtkWidget *state_button2 = gtk_toggle_button_new_with_label("无效");
    // 设置初始按钮状态
    switch (task->state) {
        case 0: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state_button0), TRUE);  // 默认选中“待办”按钮
            break;
        }
        case 2: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state_button1), TRUE);  // 默认选中“待办”按钮
            break;
        }
        case 3: {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state_button2), TRUE);  // 默认选中“待办”按钮
            break;
        }
        default: ;
    }
    // 连接信号：当某个按钮的状态变化时，回调 on_button_toggled
    g_signal_connect(state_button0, "toggled", G_CALLBACK(on_button_toggled), state_button1);  // 设置互斥
    g_signal_connect(state_button0, "toggled", G_CALLBACK(on_button_toggled), state_button2);  // 设置互斥
    g_signal_connect(state_button1, "toggled", G_CALLBACK(on_button_toggled), state_button0);
    g_signal_connect(state_button1, "toggled", G_CALLBACK(on_button_toggled), state_button2);
    g_signal_connect(state_button2, "toggled", G_CALLBACK(on_button_toggled), state_button0);
    g_signal_connect(state_button2, "toggled", G_CALLBACK(on_button_toggled), state_button1);
    // 将按钮添加到按钮盒子中
    GtkWidget *state_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); //用一个box装grid
    gtk_container_add(GTK_CONTAINER(state_box), state_button0);
    gtk_container_add(GTK_CONTAINER(state_box), state_button1);
    gtk_container_add(GTK_CONTAINER(state_box), state_button2);


    // 创建一个水平的滑块 (进度条)
    GtkWidget *progress_bar = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1); // 最小值、最大值、步长
    gtk_range_set_value(GTK_RANGE(progress_bar), task->plan_progress); // 初始值设为50
    // g_signal_connect(progress_bar, "value-changed", G_CALLBACK(on_scale_value_changed), NULL); // 连接滑动条值变化信号

    // 向对话框通过网格添加控件
    GtkWidget *grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, TRUE, 20); // 将grid放进main_box
    gtk_container_add(GTK_CONTAINER(content_area), main_box); // 将box放进对话框的content_area

    // 添加标签到网格
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("       name"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("importance"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" emergence"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("  category"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("     state"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" progress"), 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), task_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), importance_box, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), emergency_box, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), class_combobox, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), state_box, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), progress_bar, 1, 5, 1, 1);

    // 显示对话框
    gtk_widget_show_all(dialog);

    // 等待用户响应
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        // 更新变量
        strcpy(task->name, gtk_entry_get_text(GTK_ENTRY(task_entry)));
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(importance_button0))) {
            task->importance_degree = 0;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(importance_button1))) {
            task->importance_degree = 1;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(importance_button2))) {
            task->importance_degree = 2;
        }

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(emergency_button0))) {
            task->emergency_degree = 0;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(emergency_button1))) {
            task->emergency_degree = 1;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(emergency_button2))) {
            task->emergency_degree = 2;
        }

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state_button0))) {
            task->state = 0;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state_button1))) {
            task->state = 2;
        }
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state_button2))) {
            task->state = 3;
        }

        task->category = gtk_combo_box_get_active(GTK_COMBO_BOX(class_combobox));
        task->plan_progress = (int)gtk_range_get_value(GTK_RANGE(progress_bar));
        // if (task->plan_progress == 100) {
        //     task->state = 1;
        // };
        if (setting->selected_id == -1) {
            data->all_task = realloc(all_task, sizeof(TTimerTask *) * (setting->num_task + 1));
            data->all_task[setting->num_task] = task;
            setting->num_task++;
            setting->selected_id = task->id;
        }
    }
    if (response == GTK_RESPONSE_CANCEL) {
        free(task);
    }
    // 销毁对话框
    gtk_widget_destroy(dialog);
}

void on_edit_task_button_clicked(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview;
    GtkWidget *main_window = data->window;

    GtkTreeIter iter;
    GtkTreeModel *model;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)); // 获取当前选中的行

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        gtk_tree_model_get(model, &iter, 0, &id, -1);

        setting->selected_id = id;
        revise_task(widget, user_data);

        int idx = get_idx(all_task, setting->num_task, setting->selected_id);
        TTimerTask *task = select_task_idx(setting, all_task, idx);
        // 刷新列表
        char *state;
        switch (task->state) {
            case 0: state = "待办"; break;
            case 1: state = "完成"; break;
            case 2: state = "归档"; break;
            case 3: state = "删除"; break;
            case 4: state = "ing"; break;
            default: state = "未知"; break;
        }

        int spent_second = 0;
        for (int j = 0; j < task->num_execution; j++) {
            spent_second = spent_second + task->execution_duration[j];
        }
        char buffer2[20];
        int hour = spent_second / (3600);
        int minute = (spent_second - hour * 3600) / 60;
        sprintf(buffer2, "%dh%dm", hour, minute);

        gtk_list_store_set(list_store, &iter,
            0, task->id,
            1, state,
            2, setting->name_category[task->category],
            3, task->name,
            4, task->importance_degree,
            5, task->emergency_degree,
            6, task->plan_progress,
            7, task->num_execution,
            8, buffer2, -1);

        setting->change_unsaved = true;
        char buffer[100];
        sprintf(buffer, "*%s v%s", setting->app_name, setting->version);
        gtk_window_set_title(GTK_WINDOW(main_window), buffer);
    }
}

void on_delete_task_button_clicked(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview;

    GtkTreeIter iter;
    GtkTreeModel *model;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)); // 获取当前选中的行

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        // 获取选中的id
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        int idx = get_idx(all_task, setting->num_task, id);
        TTimerTask **new_all_task = malloc((setting->num_task-1) * sizeof(TTimerTask *));
        int k1 = 0;
        int k2 = 0;
        while (k1 < setting->num_task) {
            if (k1 == idx) {
                k1++;
            } else {
                new_all_task[k2] = all_task[k1];
                k1++;
                k2++;
            }
        }
        data->all_task = new_all_task;
        setting->num_task--;
        setting->change_unsaved = true;
        gtk_list_store_remove(list_store, &iter);  // 在表格中删除选中的任务
    }


    GtkWidget *main_window = data->window;
    char buffer[100];
    sprintf(buffer, "*%s v%s", setting->app_name, setting->version);
    gtk_window_set_title(GTK_WINDOW(main_window), buffer);
}

void on_start_task_button_clicked(GtkWidget *widget, gpointer user_data){
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview;

    GtkTreeIter iter;
    GtkTreeModel *model;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)); // 获取当前选中的行
    if (selection == NULL) {
        printf("no selection\n");
        return;
    }
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        // 获取选中的id
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        int idx = get_idx(all_task, setting->num_task, id);
        TTimerTask *task = select_task_idx(setting, all_task, idx);
        int err_inf = start_task(setting, task);
        if (err_inf == 0) {
            char *state;
            switch (task->state) {
                case 0: state = "待办"; break;
                case 1: state = "完成"; break;
                case 2: state = "归档"; break;
                case 3: state = "删除"; break;
                case 4: state = "ing"; break;
                default: state = "未知"; break;
            }
            gtk_list_store_set(list_store, &iter, 1, state, 7, task->num_execution, -1);
            GtkWidget *main_window = data->window;
            char buffer[100];
            sprintf(buffer, "*%s v%s", setting->app_name, setting->version);
            gtk_window_set_title(GTK_WINDOW(main_window), buffer);
        }
    }
}

// void entry_to_global_buffer(GtkWidget *widget, gpointer user_data) {
//     GtkWidget *entry = GTK_WIDGET(user_data);
//     const char *note = gtk_entry_get_text(GTK_ENTRY(entry));
//     strcpy(global_buffer, note);
// }
void on_stop_task_button_clicked(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview;
    GtkWidget *main_window = data->window;

    GtkTreeIter iter;
    GtkTreeModel *model;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)); // 获取当前选中的行
    if (selection == NULL) {
        printf("no selection\n");
        return;
    }
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        // 获取选中的id
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        int idx = get_idx(all_task, setting->num_task, id);
        TTimerTask *task = select_task_idx(setting, all_task, idx);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("note", GTK_WINDOW(main_window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "OK", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        // gtk_widget_set_size_request(content_area, 400, 400);

        GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); //用一个box装grid
        // 创建任务文本输入框
        GtkWidget *note_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(note_entry), "=task");
        gtk_widget_set_size_request(note_entry, 400, -1);

        // 创建一个水平的滑块 (进度条)
        GtkWidget *progress_bar = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1); // 最小值、最大值、步长
        gtk_range_set_value(GTK_RANGE(progress_bar), task->plan_progress); // 初始值设为50
        // 向对话框通过网格添加控件
        GtkWidget *grid = gtk_grid_new();
        gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, TRUE, 20); // 将grid放进main_box
        gtk_container_add(GTK_CONTAINER(content_area), main_box); // 将box放进对话框的content_area

        // 添加标签到网格
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new("    note"), 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new("progress"), 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), note_entry, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), progress_bar, 1, 1, 1, 1);

        // 显示对话框
        gtk_widget_show_all(dialog);

        // 等待用户响应
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_OK) {
            int err_inf = stop_task(setting, task);
            if (err_inf == 0) {
                task->plan_progress = (int)gtk_range_get_value(GTK_RANGE(progress_bar));
                printf("note: %s\n", gtk_entry_get_text(GTK_ENTRY(note_entry)));
                task->execution_note[task->num_execution-1] = (char *) malloc(300*sizeof(char));
                strcpy(task->execution_note[task->num_execution-1], gtk_entry_get_text(GTK_ENTRY(note_entry)));
                char *state;
                switch (task->state) {
                    case 0: state = "待办"; break;
                    case 1: state = "完成"; break;
                    case 2: state = "归档"; break;
                    case 3: state = "删除"; break;
                    case 4: state = "ing"; break;
                    default: state = "未知"; break;
                }
                int spent_second = 0;
                for (int j = 0; j < task->num_execution; j++) {
                    spent_second = spent_second + task->execution_duration[j];
                }
                printf("backend stopped\n");
                char buffer2[20];
                int hour = spent_second / (3600);
                int minute = (spent_second - hour * 3600) / 60;
                sprintf(buffer2, "%dh%dm", hour, minute);

                gtk_list_store_set(list_store, &iter, 1, state, 7, task->num_execution, 8, buffer2, -1);
                printf("fore-end stopped\n");
                char buffer[100];
                sprintf(buffer, "*%s v%s", setting->app_name, setting->version);
                gtk_window_set_title(GTK_WINDOW(main_window), buffer);

                char buffer3[20];
                int hour3 = spent_second / (3600);
                int minute3 = (spent_second - hour3 * 3600) / 60;
                sprintf(buffer3, "%dh%dm", hour3, minute3);

                // 更新task表的用时和进度
                gtk_list_store_set(list_store, &iter,
                    6, task->plan_progress,
                    7, task->num_execution,
                    8, buffer3, -1);
            }
        }
        gtk_widget_destroy(dialog);
    }
}


void on_today_today_button_clicked(GtkWidget *widget, gpointer user_data){
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store = data->list_store_today;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview_today;
    GtkWidget *main_window = data->window;

    gtk_list_store_clear(data->list_store_today);

    time_t cur_time;
    time(&cur_time);
    time_t start_time = (int)cur_time / (3600 * 24) * (3600 * 24) - 8*3600; // 世界时
    time_t end_time = start_time + 3600 * 24 - 1;

    // 计算time_interval的idx
    int num_task = 0; // 被选中的task数
    int num_execution = 0; // 被选中的execution数量
    for (int i = 0; i < setting->num_task; i++) {
        bool task_not_add = true;
        TTimerTask *task = all_task[i];
        for (int j = 0; j < task->num_execution; j++) {
            time_t exc_start_time = task->execution_start_time[j];
            time_t exc_end_time = task->execution_end_time[j];
            if (exc_start_time >= start_time && exc_end_time <= end_time) {
                num_execution++;
                if (task_not_add) {
                    num_task++;
                    task_not_add = false;
                }
            }
        }
    }

    int idx_task[num_execution]; // 被选中的task，数组长度=sum(num_execution)
    int idx_execution[num_execution]; // 被选中的execution，数组长度=sum(num_execution)
    time_t time_execution[num_execution]; //被选中的execution的开始时间，数组长度=sum(num_execution)
    int k = 0;
    int sum_duration = 0;

    for (int i = 0; i < setting->num_task; i++) {
        TTimerTask *task = all_task[i];
        for (int j = 0; j < task->num_execution; j++) {
            time_t exc_start_time = task->execution_start_time[j];
            time_t exc_end_time = task->execution_end_time[j];
            if (exc_start_time >= start_time && exc_end_time <= end_time) {
                idx_task[k] = i;
                idx_execution[k] = j;
                time_execution[k] = exc_start_time;
                k++;
                sum_duration = sum_duration + task->execution_duration[j];
            }
        }
    }

    if (num_execution == 0) {
        printf("No task found.\n");
        return;
    }
    int early_to_late_idx[num_execution];
    for (int i = 0; i < num_execution; i++) {
        early_to_late_idx[i] = i;
    }
    // early_to_late_idx，idx_task、idx_execution时间从小到大的idx
    sort_time(time_execution, early_to_late_idx, num_execution);

    int hour = sum_duration / (3600);
    int minute = (sum_duration - hour * 3600) / 60;
    printf("%d work on %d task (%dh%dm): \n", num_execution, num_task, hour, minute);
    for (int i = 0; i < num_execution; i++) {
        int idx = early_to_late_idx[i];
        int task_idx = idx_task[idx];
        int execution_idx = idx_execution[idx];
        TTimerTask *task = all_task[task_idx];

        char str_start_time[80];
        char str_end_time[80];
        strftime(str_start_time, 80, "%H:%M", localtime(&task->execution_start_time[execution_idx]));
        strftime(str_end_time, 80, "%H:%M", localtime(&task->execution_end_time[execution_idx]));
        struct tm *tm_info = localtime(&task->execution_start_time[execution_idx]);
        int weekday = tm_info->tm_wday;
        char *str_week;
        switch (weekday) {
            case 0:  str_week = "Sun"; break;
            case 1:  str_week = "Mon"; break;
            case 2:  str_week = "Tue"; break;
            case 3:  str_week = "Wed"; break;
            case 4:  str_week = "Thu"; break;
            case 5:  str_week = "Fri"; break;
            case 6:  str_week = "Sat"; break;
            default: str_week = "";
        }

        // 向 ListStore 中添加新的任务
        GtkTreeIter iter;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter,
        4, task->id,
        3, task->execution_duration[execution_idx] / 60,
        0, str_week,
        1, str_start_time,
        2, str_end_time,
        5, task->name,
        7, task->execution_note[execution_idx],
        6, execution_idx+1, -1);
    }
    GtkTreeIter iter;
    gtk_list_store_append(list_store, &iter);
    char buffer[20];
    sprintf(buffer, "%dh%dm", hour, minute);
    char buffer2[20];
    sprintf(buffer2, "%d ex", num_execution);
    gtk_list_store_set(list_store, &iter,
    1, buffer,
    0, buffer2, -1);
}

void on_today_week_button_clicked(GtkWidget *widget, gpointer user_data){
    waiting_debug(widget, user_data);
}

void on_detail_task_button_clicked(GtkWidget *widget, gpointer user_data) {
    TTimerData *data = (TTimerData *)user_data;
    GtkListStore *list_store_detail = data->list_store_detail;
    GtkListStore *list_store = data->list_store;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *treeview = data->treeview;
    GtkWidget *treeview_detail = data->treeview_detail;
    GtkWidget *main_window = data->window;

    gtk_list_store_clear(data->list_store_detail);

    GtkTreeIter iter;
    GtkTreeModel *model;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)); // 获取当前选中的行
    if (selection == NULL) {
        printf("no selection\n");
        return;
    }
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        // 获取选中的id
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        int idx = get_idx(all_task, setting->num_task, id);
        TTimerTask *task = select_task_idx(setting, all_task, idx);

        int sum_duration = 0;

        for (int j = 0; j < task->num_execution; j++) {
            sum_duration = sum_duration + task->execution_duration[j];

            char str_start_time[80];
            char str_end_time[80];
            strftime(str_start_time, 80, "%m-%d %H:%M", localtime(&task->execution_start_time[j]));
            strftime(str_end_time, 80, "%H:%M", localtime(&task->execution_end_time[j]));

            GtkTreeIter iter_detail;
            gtk_list_store_append(list_store_detail, &iter_detail);
            gtk_list_store_set(list_store_detail, &iter_detail,
            0, j+1,
            1, task->execution_duration[j] / 60,
            2, str_start_time,
            3, str_end_time,
            4, task->execution_note[j], -1);
        }
        GtkTreeIter iter_detail;
        gtk_list_store_append(list_store_detail, &iter_detail);
        char buffer[20];
        int hour = sum_duration / (3600);
        int minute = (sum_duration - hour * 3600) / 60;
        sprintf(buffer, "%dh%dm", hour, minute);
        gtk_list_store_set(list_store_detail, &iter_detail,
            1,sum_duration / 60,
            2, buffer, -1);
    }
}

void on_detail_revise_button_clicked(GtkWidget *widget, gpointer user_data){
    TTimerData *data = (TTimerData *)user_data;
    TTimerSetting *setting = data->setting;
    TTimerTask **all_task = data->all_task;
    GtkWidget *main_window = data->window;
    GtkListStore *list_store = data->list_store;
    GtkWidget *treeview = data->treeview;
    GtkListStore *list_store_detail = data->list_store_detail;
    GtkWidget *treeview_detail = data->treeview_detail;

    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeIter iter_detail;
    GtkTreeModel *model_detail;

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)); // 获取当前选中的行
    GtkTreeSelection *selection_detail = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_detail)); // 获取当前选中的行
    if (selection == NULL || selection_detail == NULL) {
        printf("no selection\n");
        return;
    }
    if (gtk_tree_selection_get_selected(selection, &model, &iter) && gtk_tree_selection_get_selected(selection_detail, &model_detail, &iter_detail)) {
        gint id, idx_execution;
        // 获取选中的id
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        gtk_tree_model_get(model_detail, &iter_detail, 0, &idx_execution, -1);
        idx_execution--;
        int idx = get_idx(all_task, setting->num_task, id);
        TTimerTask *task = select_task_idx(setting, all_task, idx);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("revise", GTK_WINDOW(main_window), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "OK", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        // gtk_widget_set_size_request(content_area, 400, 400);

        GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20); //用一个box装grid
        // 创建任务文本输入框
        GtkWidget *start_entry = gtk_entry_new();
        char *old_start_time = (char *)malloc(80*sizeof(char));
        strftime(old_start_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->execution_start_time[idx_execution]));
        GtkWidget *stop_entry = gtk_entry_new();
        char *old_stop_time = (char *)malloc(80*sizeof(char));
        strftime(old_stop_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->execution_end_time[idx_execution]));
        GtkWidget *note_entry = gtk_entry_new();

        gtk_entry_set_text(GTK_ENTRY(start_entry), old_start_time);
        gtk_entry_set_text(GTK_ENTRY(stop_entry), old_stop_time);
        gtk_entry_set_text(GTK_ENTRY(note_entry), task->execution_note[idx_execution]);
        gtk_widget_set_size_request(note_entry, 400, -1);

        // 向对话框通过网格添加控件
        GtkWidget *grid = gtk_grid_new();
        gtk_box_pack_start(GTK_BOX(main_box), grid, TRUE, TRUE, 20); // 将grid放进main_box
        gtk_box_pack_start(GTK_BOX(main_box), note_entry, TRUE, TRUE, 0); // 将box放进对话框的content_area
        gtk_container_add(GTK_CONTAINER(content_area), main_box); // 将box放进对话框的content_area


        // 添加标签到网格
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new("start "), 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" stop "), 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" note "), 0, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), start_entry, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), stop_entry, 1, 1, 1, 1);

        // 显示对话框
        gtk_widget_show_all(dialog);

        // 等待用户响应
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_OK) {
            strcpy(task->execution_note[idx_execution], gtk_entry_get_text(GTK_ENTRY(note_entry)));

            struct tm tm_start, tm_stop;
            int start_num =  sscanf(gtk_entry_get_text(GTK_ENTRY(start_entry)), "%d-%d-%d %d:%d:%d",
                &tm_start.tm_year, &tm_start.tm_mon, &tm_start.tm_mday,
                &tm_start.tm_hour, &tm_start.tm_min, &tm_start.tm_sec);
            int stop_num =  sscanf(gtk_entry_get_text(GTK_ENTRY(stop_entry)), "%d-%d-%d %d:%d:%d",
                &tm_stop.tm_year, &tm_stop.tm_mon, &tm_stop.tm_mday,
                &tm_stop.tm_hour, &tm_stop.tm_min, &tm_stop.tm_sec);

            if (start_num == 6 && stop_num == 6) {
                tm_start.tm_year -= 1900;  // 年份是从1900年开始的
                tm_start.tm_mon -= 1;   // 月份是从0开始的
                tm_stop.tm_year -= 1900;  // 年份是从1900年开始的
                tm_stop.tm_mon -= 1;   // 月份是从0开始的
            }

            // 使用 mktime 将 struct tm 转换为 time_t
            task->execution_start_time[idx_execution] = mktime(&tm_start);
            task->execution_end_time[idx_execution] = mktime(&tm_stop);
            task->execution_duration[idx_execution] = (int)difftime(task->execution_end_time[idx_execution], task->execution_start_time[idx_execution]);

            int task_duration = 0;
            for (int i = 0; i < task->num_execution; i++) {
                task_duration += task->execution_duration[i];
            }

            char buffer2[20];
            int hour = task_duration / (3600);
            int minute = (task_duration - hour * 3600) / 60;
            sprintf(buffer2, "%dh%dm", hour, minute);

            // 更新task表和detail表
            gtk_list_store_set(list_store, &iter, 8, buffer2, -1);

            char str_start_time[80];
            char str_end_time[80];
            strftime(str_start_time, 80, "%m-%d %H:%M", localtime(&task->execution_start_time[idx_execution]));
            strftime(str_end_time, 80, "%H:%M", localtime(&task->execution_end_time[idx_execution]));

            gtk_list_store_set(list_store_detail, &iter_detail,
            1, task->execution_duration[idx_execution] / 60,
            2, str_start_time,
            3, str_end_time,
            4, task->execution_note[idx_execution], -1);

            // 最后一行
            gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(list_store_detail), &iter_detail, NULL, task->num_execution);
            char buffer[20];
            int hour0 = task_duration / (3600);
            int minute0 = (task_duration - hour0 * 3600) / 60;
            sprintf(buffer, "%dh%dm", hour0, minute0);
            gtk_list_store_set(list_store_detail, &iter_detail,
                1,task_duration / 60,
                2, buffer, -1);

            setting->change_unsaved = true;
            char app_buffer[100];
            sprintf(app_buffer, "*%s v%s", setting->app_name, setting->version);
            gtk_window_set_title(GTK_WINDOW(main_window), app_buffer);
        }
        gtk_widget_destroy(dialog);
    }

}
void on_detail_delete_button_clicked(GtkWidget *widget, gpointer user_data){
    waiting_debug(widget, user_data);
}

void on_clicked_toolbar_open_button(GtkWidget *widget, gpointer user_data) {
    char *setting_filename = "setting.json";
    char *task_filename =  "task.json";
    char *jsonString_setting = readFile(setting_filename);
    char *jsonString_task = readFile(task_filename);

    TTimerSetting *setting = parseSetting(jsonString_setting);
    TTimerTask **all_task = parseTask(jsonString_task);
    TTimerData *data = (TTimerData *)user_data;

    gtk_list_store_clear(data->list_store);
    gtk_list_store_clear(data->list_store_detail);
    gtk_list_store_clear(data->list_store_today);

    data->setting = setting;
    data->all_task = all_task;

    load_task(user_data);
}

void test_signal(GtkWidget *widget, gpointer user_data) {
    printf("get signal\n");
}


// int on_close_event(GtkWidget *widget, gpointer user_data) {
//     TTimerData *data = (TTimerData *)user_data;
//     TTimerSetting *setting = data->setting;
//
//     // 创建一个确认对话框
//     GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(widget),
//                                                GTK_DIALOG_MODAL,
//                                                GTK_MESSAGE_QUESTION,
//                                                GTK_BUTTONS_YES_NO,
//                                                "Save?");
//
//     // 显示对话框并获取用户的选择
//     gint result = gtk_dialog_run(GTK_DIALOG(dialog));
//
//     // 根据用户的选择做出反应
//     if (result == GTK_RESPONSE_YES) {
//         if (setting->change_unsaved) {
//             on_clicked_toolbar_save_button(widget, user_data);
//         }
//         // g_print("User chose to save\n");
//         // 这里可以调用保存函数
//     } else if (result == GTK_RESPONSE_NO) {
//         printf("no save\n");
//         // g_print("User chose not to save\n");
//         // 可以在这里处理不保存的情况
//     }
//
//     // 销毁对话框
//     gtk_widget_destroy(dialog);
//
//     // 返回 FALSE 以允许窗口关闭
//     return FALSE;
// }