#ifndef __TTIMER_H__
#define __TTIMER_H__

#include <stdbool.h>
#include <time.h>


typedef struct {
    char *app_name;
    char *author;
    char *version;
    char *date;
    char *introduction;
    bool active;
    bool change_unsaved;
    int selected_id;
    int num_task;
    int num_category;
    int *key_category;
    char **name_category;
} TTimerSetting;

typedef struct {
    int id;
    char *name;
    int category;
    int importance_degree;
    int emergency_degree;
    time_t create_time;
    time_t revise_time;
    time_t plan_start_time;
    time_t plan_due_time;
    int num_roadmap;
    int *id_roadmap;
    char **plan_roadmap;
    int plan_progress;

    int num_execution;
    time_t *execution_start_time;
    time_t *execution_end_time;
    int *execution_duration;
    char **execution_note;
    bool *execution_roadmap;

    int state; // 0 存档
} TTimerTask;

void print_task(const TTimerTask *task, const TTimerSetting *setting);
void print_task_inline(const TTimerTask *task, const TTimerSetting *setting);
void print_category(const TTimerSetting *setting);
void print_about(const TTimerSetting *setting);
void print_about_inline(const TTimerSetting *setting);
// 输入时间区间内的 (duration) id-task_name: notes (start_time->end_time)
// flag=0, 按照时间顺序
// flag=1, 按照任务id属性
// week, 显示周几
int print_time_interval(TTimerTask **all_task, const TTimerSetting *setting, time_t start_time, time_t end_time, int flag, const int week);

// 获得当前命令状态
char *get_app_name(char *app_name, const TTimerSetting *setting);

// 给定id获得任务idx
int get_idx(TTimerTask **all_task, int num_task, int id);
// 给定idx，获得任务指针
TTimerTask *select_task_idx(TTimerSetting *setting, TTimerTask **all_task, int idx);

// 开始任务
int start_task(TTimerSetting *setting, TTimerTask *task);
// 结束任务
int stop_task(TTimerSetting *setting, TTimerTask *task);

int print_doing(const TTimerSetting *setting, const TTimerTask *task);

void sort_time(const time_t *time_list, int *idxes, const int num_idx);
void sort_time2(int *time_list, int *idxes, const int num_idx);
#endif