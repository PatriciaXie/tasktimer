#ifndef __GUI_H__
#define __GUI_H__
#include <gtk/gtk.h>
#include "ttimer.h"

typedef struct {
    TTimerSetting *setting;
    TTimerTask **all_task;
    GtkWidget *window;
    GtkListStore *list_store;
    GtkListStore *list_store_today;
    GtkListStore *list_store_detail;
    GtkListStore *list_store_archive;

    GtkWidget *treeview;
    GtkWidget *treeview_today;
    GtkWidget *treeview_detail;
    GtkWidget *treeview_archive;

    time_t detail_start_time;
    bool detail_time_flag;
    bool detail_week_flag;
} TTimerData;

void load_task(gpointer user_data);
void revise_task(GtkWidget *widget, gpointer user_data);

void on_clicked_toolbar_open_button(GtkWidget *widget, gpointer user_data);
void on_clicked_toolbar_about_button(GtkWidget *widget, gpointer user_data);
void on_clicked_toolbar_save_button(GtkWidget *widget, gpointer user_data);

void on_add_task_button_clicked(GtkWidget *widget, gpointer user_data);
void on_edit_task_button_clicked(GtkWidget *widget, gpointer user_data);
void on_delete_task_button_clicked(GtkWidget *widget, gpointer user_data);
void on_detail_task_button_clicked(GtkWidget *widget, gpointer user_data);
void on_start_task_button_clicked(GtkWidget *widget, gpointer user_data);
void on_stop_task_button_clicked(GtkWidget *widget, gpointer user_data);

void on_today_today_button_clicked(GtkWidget *widget, gpointer user_data);
void on_today_week_button_clicked(GtkWidget *widget, gpointer user_data);

void on_detail_revise_button_clicked(GtkWidget *widget, gpointer user_data);
void on_detail_delete_button_clicked(GtkWidget *widget, gpointer user_data);

void test_signal(GtkWidget *widget, gpointer user_data);

void on_detail_start_calendar_selected(GtkCalendar *calendar, gpointer user_data);
void on_detail_week_calendar_selected(GtkCalendar *calendar, gpointer user_data);
gboolean  on_task_treeview_right_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
gboolean  on_archive_treeview_right_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

// gint sort_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);
// int on_close_event(GtkWidget *widget, gpointer user_data);

#endif