#ifndef __FILE_H__
#define __FILE_H__
#include "ttimer.h"

char *readFile(const char *filename);
TTimerSetting *parseSetting(char *jsonString);
TTimerTask **parseTask(char *jsonString);

int writeSetting(const char *fileName, const TTimerSetting *setting);
int writeTask(const char *fileName, TTimerTask **tasks, const int numTasks);

#endif