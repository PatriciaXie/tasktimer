#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ttimer.h"

#include <string.h>


void print_task(const TTimerTask *task, const TTimerSetting *setting) {
    int category = task->category;
    char *revise_time = (char *)malloc(80*sizeof(char));
    strftime(revise_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->revise_time));
    char *plan_start_time = (char *)malloc(80*sizeof(char));
    strftime(plan_start_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->plan_start_time));
    char *plan_due_time = (char *)malloc(80*sizeof(char));
    strftime(plan_due_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->plan_due_time));

    char *category_name = "";
    for (int i = 0; i < setting->num_category; i++) {
        if (setting->key_category[i] == task->category) {
            category_name = setting->name_category[category];
            break;
        }
    }
    printf("  [id-name-update] (%s)%d-%s (revise %s)\n", category_name, task->id, task->name, revise_time);
    printf("  [importance-emergency] %d-%d\n", task->importance_degree, task->emergency_degree);
    printf("  [Plan] %s -> %s\n", plan_start_time, plan_due_time);

    char *state;
    switch (task->state) {
        case 0: state = "待办"; break;
        case 1: state = "完成"; break;
        case 2: state = "归档"; break;
        case 3: state = "删除"; break;
        case 4: state = "计时"; break;
        default: state = "未知"; break;
    }
    printf("  [state] %s \n", state);
    printf("  [execution %d%%]\n", task->plan_progress);
    char *execution_start_time = (char *)malloc(80*sizeof(char));
    char *execution_end_time = (char *)malloc(80*sizeof(char));
    int i_iter = task->num_execution;
    if (task->state == 4) {
        i_iter = task->num_execution - 1;
    }
    for (int i = 0; i < i_iter; i++) {
        strftime(execution_start_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->execution_start_time[i]));
        strftime(execution_end_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->execution_end_time[i]));
        printf("   %d> (%-3d') %s (%s -> %s) \n", i+1, task->execution_duration[i] / 60, task->execution_note[i], execution_start_time, execution_end_time);
    }
    if (task->state == 4) {
        int i = task->num_execution - 1;
        strftime(execution_start_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->execution_start_time[i]));
        strftime(execution_end_time, 80, "%Y-%m-%d %H:%M:%S", localtime(&task->execution_end_time[i]));
        printf("   %d> (%s) %s (%s -> ) \n", i+1, "ing", task->execution_note[i], execution_start_time);
    }
    free(revise_time);
    free(plan_start_time);
    free(plan_due_time);
    free(execution_start_time);
    free(execution_end_time);
}

void print_task_inline(const TTimerTask *task, const TTimerSetting *setting) {
    int category = task->category;
    char *revise_time = (char *)malloc(80*sizeof(char));
    // strftime(revise_time, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->revise_time));
    strftime(revise_time, 80, "%m-%d %H:%M:%S", localtime(&task->revise_time));

    char *category_name = "";
    for (int i = 0; i < setting->num_category; i++) {
        if (setting->key_category[i] == task->category) {
            category_name = setting->name_category[category];
            break;
        }
    }

    char *state;
    switch (task->state) {
        case 0: state = "进行"; break;
        case 1: state = "完成"; break;
        case 2: state = "归档"; break;
        case 3: state = "删除"; break;
        default: state = "未知"; break;
    }
    printf("  [%s] (%s)%d-%s [%d:%d] (%d%% %s)\n", state, category_name, task->id, task->name, task->importance_degree, task->emergency_degree, task->plan_progress, revise_time);
    free(revise_time);
}

void print_category(const TTimerSetting *setting) {
    printf("Category statistic: %d \n", setting->num_category);
    for (int i = 0; i < setting->num_category; i++) {
        printf("  %d : %s\n", setting->key_category[i], setting->name_category[i]);
    }
}

void print_about(const TTimerSetting *setting) {
    printf("%s v%s (%s) by %s\n", setting->app_name, setting->version, setting->date, setting->author);
}

// 对一组时间进行排序，获得顺序idx
void sort_time(const time_t *time_list, int *idxes, const int num_idx) {
    // 冒泡算法
    time_t list[num_idx];
    for (int i = 0; i < num_idx; i++) {
        list[i] = time_list[i];
    }
    bool not_sorted = true;
    while (not_sorted) {
        not_sorted = false;
        for (int i = 0; i < num_idx-1; i++) {
            if (list[i] > list[i+1]) {
                time_t tmp_time = list[i];
                list[i] = list[i+1];
                list[i+1] = tmp_time;
                int tmp_idx = idxes[i];
                idxes[i] = idxes[i+1];
                idxes[i+1] = tmp_idx;
                not_sorted = true;
            }
        }
    }
}


void sort_time2(int *time_list, int *idxes, const int num_idx) {
    // 冒泡算法
    int list[num_idx];
    for (int i = 0; i < num_idx; i++) {
        list[i] = time_list[i];
    }
    bool not_sorted = true;
    while (not_sorted) {
        not_sorted = false;
        for (int i = 0; i < num_idx-1; i++) {
            if (list[i] > list[i+1]) {
                int tmp_time = list[i];
                list[i] = list[i+1];
                list[i+1] = tmp_time;
                int tmp_idx = idxes[i];
                idxes[i] = idxes[i+1];
                idxes[i+1] = tmp_idx;
                not_sorted = true;
            }
        }
    }
}

int print_time_interval(TTimerTask **all_task, const TTimerSetting *setting, const time_t start_time, const time_t end_time, const int flag, const int week) {
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
    }
    int early_to_late_idx[num_execution];
    for (int i = 0; i < num_execution; i++) {
        early_to_late_idx[i] = i;
    }
    // early_to_late_idx，idx_task、idx_execution时间从小到大的idx
    sort_time(time_execution, early_to_late_idx, num_execution);

    int err = 5;
    if (flag == 0) { // 按时间顺序统计今日任务
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
            printf("  %2d> (%3d') (%s: %s -> %s) %d-%s: %s\n", i, task->execution_duration[execution_idx] / 60, str_week, str_start_time, str_end_time,
                task->id, task->name, task->execution_note[execution_idx]);
        }
        err = 0;
    }
    return err;
}

char *get_app_name(char *app_name, const TTimerSetting *setting) {
    char select[100];
    char save[100];
    char active[100];

    if (setting->selected_id != -1) {
        sprintf(select, "(%d)", setting->selected_id);
    } else {
        sprintf(select, "");
    }
    if (setting->change_unsaved) {
        sprintf(save, "*");
    } else {
        sprintf(save, "");
    }
    if (setting->active) {
        sprintf(active, "=");
    } else {
        sprintf(active, ">");
    }
    sprintf(app_name, "%sTT%s%s", select, save, active);
    return app_name;
}

int get_idx(TTimerTask **all_task, const int num_task, const int id) {
    int idx = -1;
    for (int i = 0; i < num_task; i++) {
        TTimerTask *task = all_task[i];
        if (task->id == id) {
            idx = i;
            break;
        }
    }
    return idx;
}

TTimerTask *select_task_idx(TTimerSetting *setting, TTimerTask **all_task, int idx) {
    TTimerTask *task = all_task[idx];
    setting->selected_id = task->id;
    return task;
}

int start_task(TTimerSetting *setting, TTimerTask *task) {
    if (task->state == 4) {
        return 1; // 当前任务正在进行
    }
    if (setting->active) {
        return 2; // 已有其他任务正在进行
    }
    time_t cur_time;
    time(&cur_time);
    if (task->num_execution == 0) {
        task->execution_start_time = (time_t *)malloc(sizeof(time_t));
        task->execution_end_time = (time_t *)malloc(sizeof(time_t));
        task->execution_duration = (int *)malloc(sizeof(int));
        task->execution_note = (char **)malloc(sizeof(char*));
    } else {
        void* temp1 = realloc(task->execution_start_time, sizeof(time_t) * (task->num_execution + 1));
        if (temp1 != NULL) {
            task->execution_start_time = temp1;
        } else {
            return 4; // 内存分配错误
        }
        void* temp2 = realloc(task->execution_end_time, sizeof(time_t) * (task->num_execution + 1));
        if (temp2 != NULL) {
            task->execution_end_time = temp2;
        } else {
            return 4; // 内存分配错误
        }
        void* temp3 = realloc(task->execution_duration, sizeof(int) * (task->num_execution + 1));
        if (temp3 != NULL) {
            task->execution_duration = temp3;
        } else {
            return 4; // 内存分配错误
        }
        void* temp4 = realloc(task->execution_note, sizeof(char*) * (task->num_execution + 1));
        if (temp4 != NULL) {
            task->execution_note = temp4;
        } else {
            return 4; // 内存分配错误
        }
    }
    task->num_execution = task->num_execution + 1;
    task->execution_start_time[task->num_execution-1] = cur_time;
    task->execution_end_time[task->num_execution-1] = cur_time;
    task->execution_duration[task->num_execution-1] = 0;
    task->execution_note[task->num_execution-1] = "n/a";
    task->state = 4; // 把任务设为正在进行
    setting->active = true;

    setting->change_unsaved = true;

    char *app_name = (char *)malloc(80*sizeof(char));
    get_app_name(app_name, setting);
    char *time = (char *)malloc(80*sizeof(char));
    strftime(time, 80, "%H:%M:%S", localtime(&cur_time));
    printf("%s Task started at %s\n", app_name, time);
    free(app_name);
    free(time);

    return 0;
}

int stop_task(TTimerSetting *setting, TTimerTask *task) {
    if (!setting->active || task->state != 4) {
        return 3; // 当前任务没有在进行
    }
    time_t cur_time;
    time(&cur_time);
    task->execution_end_time[task->num_execution-1] = cur_time;
    int duration_min = (int)difftime(cur_time, task->execution_start_time[task->num_execution-1]) / 60;
    int duration_sec = (int)difftime(cur_time, task->execution_start_time[task->num_execution-1]) - duration_min * 60;
    task->execution_duration[task->num_execution-1] = (int)difftime(cur_time, task->execution_start_time[task->num_execution-1]);

    char *app_name = (char *)malloc(80*sizeof(char));
    get_app_name(app_name, setting);

    task->state = 0; // 把任务设为待办
    setting->active = false;

    char *time = (char *)malloc(80*sizeof(char));
    strftime(time, 80, "%H:%M:%S", localtime(&cur_time));
    printf("%s Task stopped at %s: %dm%ds\n", app_name, time, duration_min, duration_sec);
    free(time);
    free(app_name);
    setting->change_unsaved = true;
    return 0;
}

int print_doing(const TTimerSetting *setting, const TTimerTask *task) {
    if (!setting->active || task->state != 4) {
        return 3; // 当前任务没有在进行
    }
    time_t cur_time;
    time(&cur_time);

    int duration_min = (int)difftime(cur_time, task->execution_start_time[task->num_execution-1]) / 60;
    int duration_sec = (int)difftime(cur_time, task->execution_start_time[task->num_execution-1]) - duration_min * 60;
    char *app_name = (char *)malloc(80*sizeof(char));
    get_app_name(app_name, setting);
    printf("%s [%s] Passed: %dm%ds\n", app_name, task->name, duration_min, duration_sec);
    free(app_name);
    return 0;
}

