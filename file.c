#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "file.h"


time_t time_json2c(const char *json_time) {
    struct tm tm_time = {0};
    // 解析时间
    int year, month, day, hour, minute, second;

    // 使用 sscanf() 提取时间字段
    sscanf(json_time, "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);

    // 填充 struct tm 结构
    tm_time.tm_year = year - 1900;  // tm_year 是从 1900 年开始的
    tm_time.tm_mon = month - 1;     // tm_mon 是从 0 月份开始的
    tm_time.tm_mday = day;
    tm_time.tm_hour = hour;
    tm_time.tm_min = minute;
    tm_time.tm_sec = second;

    // 将 struct tm 转换为 time_t
    time_t c_time = mktime(&tm_time);
    return c_time;
};

char *readFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("TT> cannot open file %s\n", filename);
        return NULL;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存并读取文件内容
    char *content = (char *)malloc(fileSize + 1);  // +1 为了末尾加上 '\0'
    if (content) {
        fread(content, 1, fileSize, file);
        content[fileSize] = '\0';  // 确保字符串结束
    }

    if (content == NULL) {
        printf("TT> cannot read file %s\n", filename);
    }

    fclose(file);
    return content;
}


TTimerSetting *parseSetting(char *jsonString) {
    // 解析 JSON 字符串
    cJSON *root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("TT> cannot parse setting file \n");
        free(jsonString);
        return NULL;
    }

    TTimerSetting *setting = (TTimerSetting *)malloc(sizeof(TTimerSetting));

    cJSON *json_setting = cJSON_GetObjectItem(root, "setting");
    cJSON *json_app_name = cJSON_GetObjectItem(json_setting, "app_name");
    cJSON *json_author = cJSON_GetObjectItem(json_setting, "author");
    cJSON *json_version = cJSON_GetObjectItem(json_setting, "version");
    cJSON *json_date = cJSON_GetObjectItem(json_setting, "date");
    cJSON *json_introduction = cJSON_GetObjectItem(json_setting, "introduction");
    cJSON *json_active = cJSON_GetObjectItem(json_setting, "active");
    cJSON *json_change_unsaved = cJSON_GetObjectItem(json_setting, "change_unsaved");
    cJSON *json_selected_id = cJSON_GetObjectItem(json_setting, "selected_id");
    cJSON *json_num_task = cJSON_GetObjectItem(json_setting, "num_task");

    setting->app_name = json_app_name->valuestring;
    setting->author = json_author->valuestring;
    setting->version = json_version->valuestring;
    setting->date = json_date->valuestring;
    setting->introduction = json_introduction->valuestring;
    setting->active = cJSON_IsTrue(json_active) ? true : false;
    setting->change_unsaved = cJSON_IsTrue(json_change_unsaved) ? true : false;
    setting->selected_id = json_selected_id->valueint;
    setting->num_task = json_num_task->valueint;

    cJSON *json_num_category = cJSON_GetObjectItem(json_setting, "num_category");
    cJSON *json_name_category = cJSON_GetObjectItem(json_setting, "name_category");
    setting->num_category = json_num_category->valueint;
    setting->key_category = (int *)malloc(setting->num_category * sizeof(int));
    setting->name_category = (char **)malloc(setting->num_category * sizeof(char *));

    for (int i = 0; i < setting->num_category; i++) {
        cJSON *name_category_item = cJSON_GetArrayItem(json_name_category, i);
        cJSON *json_name_category_item_key = cJSON_GetObjectItem(name_category_item, "key");
        setting->key_category[i] = json_name_category_item_key->valueint;
        cJSON *json_name_category_item_name = cJSON_GetObjectItem(name_category_item, "name");
        setting->name_category[i] = json_name_category_item_name->valuestring;
    }
    return setting;
}


TTimerTask **parseTask(char *jsonString){
    cJSON *root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("TT> cannot parse task file \n");
        free(jsonString);
        return NULL;
    }
    cJSON *json_task = cJSON_GetObjectItem(root, "task");
    int num_task = cJSON_GetArraySize(json_task);


    TTimerTask **all_task = (TTimerTask **)malloc(num_task * sizeof(TTimerTask));
    for (int i = 0; i < num_task; i++) {
        all_task[i] = (TTimerTask *)malloc(sizeof(TTimerTask));
        cJSON *json_item = cJSON_GetArrayItem(json_task, i);
        cJSON *json_id = cJSON_GetObjectItem(json_item, "id");
        all_task[i]->id = json_id->valueint;
        cJSON *json_name = cJSON_GetObjectItem(json_item, "name");
        all_task[i]->name = json_name->valuestring;
        cJSON *json_category = cJSON_GetObjectItem(json_item, "category");
        all_task[i]->category = json_category->valueint;
        cJSON *json_importance_degree = cJSON_GetObjectItem(json_item, "importance_degree");
        all_task[i]->importance_degree = json_importance_degree->valueint;
        cJSON *json_emergency_degree = cJSON_GetObjectItem(json_item, "emergency_degree");
        all_task[i]->emergency_degree = json_emergency_degree->valueint;
        cJSON *json_create_time = cJSON_GetObjectItem(json_item, "create_time");
        all_task[i]->create_time = time_json2c(json_create_time->valuestring);
        cJSON *json_revise_time = cJSON_GetObjectItem(json_item, "revise_time");
        all_task[i]->revise_time = time_json2c(json_revise_time->valuestring);
        cJSON *json_plan_start_time = cJSON_GetObjectItem(json_item, "plan_start_time");
        all_task[i]->plan_start_time = time_json2c(json_plan_start_time->valuestring);
        cJSON *json_plan_due_time = cJSON_GetObjectItem(json_item, "plan_due_time");
        all_task[i]->plan_due_time = time_json2c(json_plan_due_time->valuestring);
        cJSON *json_num_roadmap = cJSON_GetObjectItem(json_item, "num_roadmap");
        all_task[i]->num_roadmap = json_num_roadmap->valueint;

        cJSON *json_plan_roadmap = cJSON_GetObjectItem(json_item, "plan_roadmap");
        all_task[i]->id_roadmap = (int *)malloc(all_task[i]->num_roadmap * sizeof(int));
        all_task[i]->plan_roadmap = (char **)malloc(all_task[i]->num_roadmap * sizeof(char *));
        all_task[i]->execution_roadmap = (bool *)malloc(all_task[i]->num_roadmap * sizeof(bool));
        for (int j = 0; j < all_task[i]->num_roadmap; j++) {
            cJSON *json_roadmap_item = cJSON_GetArrayItem(json_plan_roadmap, j);
            cJSON *json_id_roadmap = cJSON_GetObjectItem(json_roadmap_item, "key");
            all_task[i]->id_roadmap[j] = json_id_roadmap->valueint;
            cJSON *json_name_roadmap = cJSON_GetObjectItem(json_roadmap_item, "name");
            all_task[i]->plan_roadmap[j] = json_name_roadmap->valuestring;
            cJSON *json_bool_roadmap = cJSON_GetObjectItem(json_roadmap_item, "done");
            all_task[i]->execution_roadmap[j] = cJSON_IsTrue(json_bool_roadmap) ? true : false;
        }

        cJSON *json_plan_progress = cJSON_GetObjectItem(json_item, "plan_progress");
        all_task[i]->plan_progress = json_plan_progress->valueint;
        cJSON *json_num_execution = cJSON_GetObjectItem(json_item, "num_execution");
        all_task[i]->num_execution = json_num_execution->valueint;

        cJSON *json_execution = cJSON_GetObjectItem(json_item, "execution");
        all_task[i]->execution_start_time = (time_t *)malloc(all_task[i]->num_execution * sizeof(time_t *));
        all_task[i]->execution_end_time = (time_t *)malloc(all_task[i]->num_execution * sizeof(time_t *));
        all_task[i]->execution_duration = (int *)malloc(all_task[i]->num_execution * sizeof(int));
        all_task[i]->execution_note = (char **)malloc(all_task[i]->num_execution * sizeof(char *));
        for (int j = 0; j < all_task[i]->num_execution; j++) {
            cJSON *json_execution_item = cJSON_GetArrayItem(json_execution, j);
            cJSON *json_execution_start_time = cJSON_GetObjectItem(json_execution_item, "start");
            all_task[i]->execution_start_time[j] = time_json2c(json_execution_start_time->valuestring);
            cJSON *json_execution_end_time = cJSON_GetObjectItem(json_execution_item, "end");
            all_task[i]->execution_end_time[j] = time_json2c(json_execution_end_time->valuestring);
            cJSON *json_execution_duration = cJSON_GetObjectItem(json_execution_item, "duration");
            all_task[i]->execution_duration[j] = json_execution_duration->valueint;
            cJSON *json_execution_note = cJSON_GetObjectItem(json_execution_item, "note");
            all_task[i]->execution_note[j] = json_execution_note->valuestring;
        }

        cJSON *json_state = cJSON_GetObjectItem(json_item, "state");
        all_task[i]->state = json_state->valueint;

    }
    return all_task;
}

int writeSetting(const char *fileName, const TTimerSetting *setting) {
    // 创建一个JSON对象
    cJSON *root = cJSON_CreateObject();
    cJSON *json_setting = cJSON_CreateObject();

    cJSON_AddStringToObject(json_setting, "app_name", setting->app_name);
    cJSON_AddStringToObject(json_setting, "author", setting->author);
    cJSON_AddStringToObject(json_setting, "version", setting->version);
    cJSON_AddStringToObject(json_setting, "date", setting->date);
    cJSON_AddStringToObject(json_setting, "introduction", setting->introduction);

    cJSON_AddNumberToObject(json_setting, "num_task", setting->num_task);
    cJSON_AddNumberToObject(json_setting, "num_category", setting->num_category);

    cJSON *name_category = cJSON_CreateArray();
    for (int i = 0; i < setting->num_category; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "key", setting->key_category[i]);
        cJSON_AddStringToObject(item, "name", setting->name_category[i]);
        cJSON_AddItemToArray(name_category, item);
    }
    cJSON_AddItemToObject(json_setting, "name_category", name_category);
    cJSON_AddBoolToObject(json_setting, "active", setting->active);
    cJSON_AddBoolToObject(json_setting, "change_unsaved", false);
    cJSON_AddNumberToObject(json_setting, "selected_id", setting->selected_id);

    // 将setting添加到root下，键名为 "setting"
    cJSON_AddItemToObject(root, "setting", json_setting);

    // 打开文件准备写入
    FILE *f = fopen(fileName, "w");
    if (f == NULL) {
        perror("Unable to open file for writing");
        return 1;
    }

    // 将JSON对象转化为字符串并写入文件
    char *json_string = cJSON_Print(root);  // 获取格式化的JSON字符串
    fprintf(f, "%s", json_string);

    // 关闭文件
    fclose(f);

    // 释放内存
    cJSON_Delete(root);
    free(json_string);

    // printf("setting saved.\n");
    return 0;
}


int writeTask(const char *fileName, TTimerTask **tasks, const int numTasks) {
    // 创建一个JSON对象
    cJSON *root = cJSON_CreateObject();

    cJSON *json_task_array = cJSON_CreateArray();

    char *time_buffer = (char *)malloc(80*sizeof(char));
    for (int i = 0; i < numTasks; i++) {
        cJSON *item = cJSON_CreateObject();
        TTimerTask *task = tasks[i];
        cJSON_AddNumberToObject(item, "id", task->id);
        cJSON_AddStringToObject(item, "name", task->name);
        cJSON_AddNumberToObject(item, "category", task->category);
        cJSON_AddNumberToObject(item, "importance_degree", task->importance_degree);
        cJSON_AddNumberToObject(item, "emergency_degree", task->emergency_degree);
        strftime(time_buffer, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->create_time));
        cJSON_AddStringToObject(item, "create_time", time_buffer);
        strftime(time_buffer, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->revise_time));
        cJSON_AddStringToObject(item, "revise_time", time_buffer);
        strftime(time_buffer, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->plan_start_time));
        cJSON_AddStringToObject(item, "plan_start_time", time_buffer);
        strftime(time_buffer, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->plan_due_time));
        cJSON_AddStringToObject(item, "plan_due_time", time_buffer);

        cJSON_AddNumberToObject(item, "num_roadmap", task->num_roadmap);
        cJSON *json_roadmap_array = cJSON_CreateArray();
        for (int j = 0; j < task->num_roadmap; j++) {
            cJSON *json_roadmap_item = cJSON_CreateObject();
            cJSON_AddNumberToObject(json_roadmap_item, "key", task->id_roadmap[j]);
            cJSON_AddStringToObject(json_roadmap_item, "name", task->plan_roadmap[j]);
            cJSON_AddBoolToObject(json_roadmap_item, "done", task->execution_roadmap[j]);
            cJSON_AddItemToArray(json_roadmap_array, json_roadmap_item);
        }
        cJSON_AddItemToObject(item, "plan_roadmap", json_roadmap_array);

        cJSON_AddNumberToObject(item, "plan_progress", task->plan_progress);
        cJSON_AddNumberToObject(item, "num_execution", task->num_execution);

        cJSON *json_execution_array = cJSON_CreateArray();
        for (int j = 0; j < task->num_execution; j++) {
            cJSON *json_execution_item = cJSON_CreateObject();
            cJSON_AddNumberToObject(json_execution_item, "key", j);
            strftime(time_buffer, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->execution_start_time[j]));
            cJSON_AddStringToObject(json_execution_item, "start", time_buffer);
            strftime(time_buffer, 80, "%Y-%m-%dT%H:%M:%S", localtime(&task->execution_end_time[j]));
            cJSON_AddStringToObject(json_execution_item, "end", time_buffer);
            cJSON_AddNumberToObject(json_execution_item, "duration", task->execution_duration[j]);
            cJSON_AddStringToObject(json_execution_item, "note", task->execution_note[j]);
            cJSON_AddItemToArray(json_execution_array, json_execution_item);
        }
        cJSON_AddItemToObject(item, "execution", json_execution_array);

        cJSON_AddNumberToObject(item, "state", task->state);
        cJSON_AddItemToArray(json_task_array, item);
    }
    free(time_buffer);

    // 将task添加到root下，键名为 "task"
    cJSON_AddItemToObject(root, "task", json_task_array);

    // 打开文件准备写入
    FILE *f = fopen(fileName, "w");
    if (f == NULL) {
        perror("Unable to open file for writing");
        return 1;
    }

    // 将JSON对象转化为字符串并写入文件
    char *json_string = cJSON_Print(root);  // 获取格式化的JSON字符串
    fprintf(f, "%s", json_string);

    // 关闭文件
    fclose(f);

    // 释放内存
    cJSON_Delete(root);
    free(json_string);

    // printf("task saved.\n");
    return 0;
}