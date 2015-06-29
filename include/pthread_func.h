#ifndef _PTHREAD_FUNC_H
#define _PTHREAD_FUNC_H

#include "./mylist.h"
#include "./code.h"
#include "./task.h"

void * worker_func(void *arg);

int abstract_info(struct info_from_task * info, struct task * tsk);

int jd_get_keyword(char *keyword, char *buffer, int buflen);

int tm_get_keyword(char *keyword, char *buffer, int buflen);

int tb_get_keyword(char *keyword, char *buffer, int buflen);

int sn_get_keyword(char *keyword, char *buffer, int buflen);

int pp_get_keyword(char *keyword, char *buffer, int buflen);

int dd_get_keyword(char *keyword, char *buffer, int buflen);

int get_keyword(char *keyword, char *urlcode, size_t urllen);

int tb_get_keyword(char *keyword, char *buffer, int buflen);

//mobile
int m_jd_get_keyword(char *keyword, char *buffer, int buflen);

int m_tm__get_keyword(char *keyword, char *buffer, int buflen);

int m_dd_get_keyword(char *keyword, char *buffer, int buflen);

int m_yx_get_keyword(char *keyword, char *buffer, int buflen);

int m_yhd_get_keyword(char *keyword, char *buffer, int buflen);

int m_jymp_get_keyword(char *keyword, char *buffer, int buflen);

int m_sn_get_keyword(char *keyword, char *buffer, int buflen);

int m1_get_keyword(char *keyword, char *buffer, int buflen);
#endif

