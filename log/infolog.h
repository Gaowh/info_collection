#ifndef _INFOLOG_H
#define _INFOLOG_H

#include "../include/task.h"

#define LOGPATH "/bh_manage/info_collection/info.log"
//#define LOGPATH "./info.log"
#define MAX_LOG_BUF 200

#define LEVEL_ERR 1
#define LEVEL_DEBUG 2
#define LEVEL_INFO 3

int open_log();

int do_log(char *comments, int len, int level);

int clear_log();

int close_log(int fd);

#endif
