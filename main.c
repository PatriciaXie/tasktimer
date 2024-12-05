#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "file.h"


int main(const int argc, char *argv[]) {
    // 读取文件
    char *setting_filename = "../setting.json";
    char *task_filename =  "../task.json";
    // char *setting_filename = "setting.json";
    // char *task_filename =  "task.json";
    if (argc == 3) {
        setting_filename = argv[1];
        task_filename = argv[2];
    }
    char *jsonString_setting = readFile(setting_filename);
    char *jsonString_task = readFile(task_filename);

    if (jsonString_setting == NULL || jsonString_task == NULL) {
        return 1;
    }

    TTimerSetting *setting = parseSetting(jsonString_setting);
    TTimerTask **all_task = parseTask(jsonString_task);

    // 显示
    print_about(setting);

    // TTimerTask *task = all_task[0];
    // print_task(task, setting);

    char *app_name = (char *)malloc(80*sizeof(char));
    get_app_name(app_name, setting);
    while (1) {
        get_app_name(app_name, setting);
        char command[100];
        printf("%s ", app_name);
        scanf("%s", command);
        if (strcmp(command, "about") == 0) {
            print_about(setting);
            continue;
        }
        if (strcmp(command, "help") == 0) {
            printf(" about: 关于本软件\n");
            printf("  help: 帮助\n");
            printf("    ls: 所有任务\n");
            printf("select: 选择任务\n");
            printf("  show: 显示任务详情\n");
            printf("  save: 保存\n");
            printf("  exit: 退出\n");
            printf(" start: 开始\n");
            printf("  stop: 结束\n");
            printf("     ?: 当前已计时\n");
        }
        if (strcmp(command, "reload") == 0) {
            if (setting->change_unsaved) {
                setting->change_unsaved = false;

                writeTask(task_filename, all_task, setting->num_task);
                writeSetting(setting_filename, setting);
            }
            jsonString_setting = readFile(setting_filename);
            jsonString_task = readFile(task_filename);

            if (jsonString_setting == NULL || jsonString_task == NULL) {
                return 1;
            }

            setting = parseSetting(jsonString_setting);
            all_task = parseTask(jsonString_task);
            continue;
        }
        if (strcmp(command, "list_class") == 0) {
            print_category(setting);
            continue;
        }
        if (strcmp(command, "ls") == 0) {
            for (int i = 0; i < setting->num_task; i++) {
                TTimerTask *task = all_task[i];
                print_task_inline(task, setting);
            }
            continue;
        }
        if (strcmp(command, "show") == 0) {
            int id = -1;
            if (setting->selected_id != -1) {
                id = setting->selected_id;
            } else {
                char buffer[10];
                printf("%s input id: ", app_name);
                scanf("%s", buffer);
                id = atoi(buffer);
            }

            int idx = get_idx(all_task, setting->num_task, id);
            // printf("%d\n", idx);
            if (idx == -1) {
                printf("%s has no (%d)\n", app_name, id);
            } else {
                print_task(all_task[idx], setting);
            }
            continue;
        }
        if (strcmp(command, "add") == 0) {
            char name[100];
            char buffer[100];
            TTimerTask *task = (TTimerTask *)malloc(sizeof(TTimerTask));

            printf("%s input name: ", app_name);
            scanf("%s", name);
            task->id = setting->num_task;
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

            print_task_inline(task, setting);

            printf("%s addition confirm? [y/n]: ", app_name);
            scanf("%s", buffer);
            if (strcmp(buffer, "y") == 0) {
                all_task = realloc(all_task, sizeof(TTimerTask *) * (setting->num_task + 1));
                if (all_task == NULL) {
                    printf("%s realloc failed!", app_name);
                    return 1;
                }
                all_task[setting->num_task] = task;
                setting->num_task++;
                setting->selected_id = task->id;
                setting->change_unsaved = true;
                get_app_name(app_name, setting);
            }

        }

        if (strcmp(command, "select") == 0) {
            char buffer[10];
            printf("%s input id: ", app_name);
            scanf("%s", buffer);
            setting->selected_id = atoi(buffer);
            get_app_name(app_name, setting);
            continue;
        }

        if (strcmp(command, "start") == 0) {
            int id = setting->selected_id;
            if (setting->selected_id == -1) {
                char buffer[10];
                printf("%s input id: ", app_name);
                scanf("%s", buffer);
                id = atoi(buffer);
            }
            int idx = get_idx(all_task, setting->num_task, id);
            TTimerTask *task = select_task_idx(setting, all_task, idx);
            int err_inf = start_task(setting, task);
            if (err_inf != 0) {
                printf("error\n");
            }
            get_app_name(app_name, setting);
            continue;
        }

        if (strcmp(command, "stop") == 0) {
            int id = setting->selected_id;
            if (setting->selected_id == -1) {
                char buffer[10];
                printf("%s input id: ", app_name);
                scanf("%s", buffer);
                id = atoi(buffer);
            }
            int idx = get_idx(all_task, setting->num_task, id);
            TTimerTask *task = select_task_idx(setting, all_task, idx);
            int err_inf = stop_task(setting, task);
            if (err_inf != 0) {
                printf("error\n");
            }
            get_app_name(app_name, setting);

            char *note = malloc(100*sizeof(char));
            printf("%s input note: ", app_name);
            scanf("%s", note);
            task->execution_note[task->num_execution-1] = note;
            continue;
        }
        if (strcmp(command, "recal") == 0) { //重新计算所有任务的执行时间
            for (int i = 0; i < setting->num_task; i++) {
                for (int j = 0; j < all_task[i]->num_execution; j++) {
                    int real_duration = (int)difftime(all_task[i]->execution_end_time[j], all_task[i]->execution_start_time[j]);
                    int saved_duration = all_task[i]->execution_duration[j];
                    if ( real_duration != saved_duration ) {
                        all_task[i]->execution_duration[j] = real_duration;
                        char *start_time = (char *)malloc(80*sizeof(char));
                        strftime(start_time, 80, "%H:%M:%S", localtime(&all_task[i]->execution_start_time[j]));
                        char *end_time = (char *)malloc(80*sizeof(char));
                        strftime(end_time, 80, "%H:%M:%S", localtime(&all_task[i]->execution_end_time[j]));
                        printf("Correct [%d%s:%d%s] %s -> %s (%d -> %d)\n", all_task[i]->id, all_task[i]->name, j, all_task[i]->execution_note[j], start_time, end_time, saved_duration, real_duration);
                        free(start_time);
                        free(end_time);
                        setting->change_unsaved = true;
                    }
                }
            }
            if (!setting->change_unsaved == true) {
                printf("%s all duration correct\n", app_name);
            }

        }
        if (strcmp(command, "?") == 0) { // 正在进行的任务过去多久了
            int id = setting->selected_id;
            if (setting->selected_id == -1) {
                char buffer[10];
                printf("%s input id: ", app_name);
                scanf("%s", buffer);
                id = atoi(buffer);
            }
            int idx = get_idx(all_task, setting->num_task, id);
            TTimerTask *task = select_task_idx(setting, all_task, idx);
            print_doing(setting, task);
            continue;
        }

        if (strcmp(command, "revise") == 0) {
            if (setting->selected_id == -1) {
                char buffer[10];
                printf("%s input id: ", app_name);
                scanf("%s", buffer);
                setting->selected_id = atoi(buffer);
            }
            int idx = get_idx(all_task, setting->num_task, setting->selected_id);
            if (idx == -1) {
                printf("%s has no (%d)\n", app_name, setting->selected_id);
                continue;
            }
            printf("%s what to revise: ", app_name);
            char buffer[100];
            // fgets(buffer, sizeof(buffer), stdin);
            // buffer[strcspn(buffer, "\n")] = '\0'; // 去除换行符（如果存在）
            scanf("%s", buffer);
            if (strcmp(buffer, "name") == 0) {
                printf("%s new name: ", app_name);
                scanf("%s", buffer);
                TTimerTask *task = all_task[idx];

                char char_buffer[10];
                printf("%s change [%s] to [%s]? [y/n]: ", app_name, task->name, buffer);
                scanf("%s", char_buffer);
                if (char_buffer[0] == 'y') {
                    task->name = buffer;
                    setting->change_unsaved = true;
                    get_app_name(app_name, setting);
                }

            } else {
                printf("%s no such attribute \n", app_name);
            }
            continue;
        }
        if (strcmp(command, "today") == 0) {
            time_t cur_time;
            time(&cur_time);
            time_t start_time = (int)cur_time / (3600 * 24) * (3600 * 24) - 8*3600; // 世界时
            time_t end_time = start_time + 3600 * 24 - 1;
            // char *tmp1 = (char *)alloca(80*sizeof(char));
            // strftime(tmp1, 80, "%Y-%m-%d %H:%M:%S", localtime(&start_time));
            // char *tmp2 = (char *)alloca(80*sizeof(char));
            // strftime(tmp2, 80, "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            print_time_interval(all_task, setting, start_time, end_time, 0, 0);
        }
        if (strcmp(command, "week") == 0) {
            time_t cur_time;
            time(&cur_time);

            struct tm *tm_info = localtime(&cur_time);
            int weekday = tm_info->tm_wday;
            weekday = (weekday != 0)? weekday : 7;

            // 本周的第一天, weekday = 1
            time_t start_time = ((int)cur_time / (3600 * 24) - weekday + 1) * (3600 * 24) - 8*3600;
            time_t end_time = start_time + 7 * 3600 * 24 - 1;
            char *tmp1 = (char *)alloca(80*sizeof(char));
            strftime(tmp1, 80, "%Y-%m-%d %H:%M:%S", localtime(&start_time));
            char *tmp2 = (char *)alloca(80*sizeof(char));
            strftime(tmp2, 80, "%Y-%m-%d %H:%M:%S", localtime(&end_time));
            print_time_interval(all_task, setting, start_time, end_time, 0, 1);
        }
        if (strcmp(command, "delete") == 0) { //一般不用delete，否则可能造成新任务的id与老任务重复
            if (setting->selected_id == -1) {
                char buffer[10];
                printf("%s input id: ", app_name);
                scanf("%s", buffer);
                setting->selected_id = atoi(buffer);
            }
            int idx = get_idx(all_task, setting->num_task, setting->selected_id);
            if (idx == -1) {
                printf("%s has no (%d)\n", app_name, setting->selected_id);
                continue;
            }
            TTimerTask *task = select_task_idx(setting, all_task, idx);
            char char_buffer[10];
            printf("%s sure to delete [%s]? [y/n]: ", app_name, task->name);
            scanf("%s", char_buffer);
            if (char_buffer[0] == 'y') {
                setting->num_task = setting->num_task - 1;
                // free(task->name);
                all_task = realloc(all_task, sizeof(TTimerTask *) * (setting->num_task));
                setting->change_unsaved = true;
                setting->selected_id = -1;
                get_app_name(app_name, setting);
                printf("[%s] deleted\n", task->name);
            }
        }
        if (strcmp(command, "save") == 0) {
            if (setting->change_unsaved) {

                writeTask(task_filename, all_task, setting->num_task);
                writeSetting(setting_filename, setting);
                setting->change_unsaved = false;
                printf("%s file saved\n", app_name);
                get_app_name(app_name, setting);
            } else {
                printf("%s no change\n", app_name);
            }
            continue;
        }
        if (strcmp(command, "exit") == 0) {
            // 退出时不选中任何任务
            if (setting->change_unsaved) {
                char char_buffer[10];
                printf("%s change unsaved, will save? [y/n]: ", app_name);
                scanf("%s", char_buffer);
                if (char_buffer[0] == 'y') {
                    setting->change_unsaved = false;
                    writeTask(task_filename, all_task, setting->num_task);
                    writeSetting(setting_filename, setting);
                    get_app_name(app_name, setting);
                    printf("%s file saved\n", app_name);
                }
            }
            get_app_name(app_name, setting);
            printf("%s bye\n", app_name);
            break;
        }
    }

    // 清理内存
    free(app_name);
    free(jsonString_setting);
    free(jsonString_task);
    return 0;
}
