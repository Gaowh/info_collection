#ifndef _HASH_H
#define _HASH_H


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#define MAX_HASH_NODE 20
#define CLEAR_TIMESLOT 10

pthread_mutex_t hash_mutex;

struct node {
	
	struct node *next;
	void *keyword;
	size_t keylen;
};



struct node hashtable[MAX_HASH_NODE];

int hashfunc(unsigned char * keyword, int wordlen);

void hashtable_init();

int hash_find_keyword(unsigned char *keyword, int wordlen);

void hash_insert_keyword(unsigned char *keyword, int wordlen);

int hash_del_keyword(unsigned char *keyword, int wordlen);

void clear_hashtable();

void *hash_func(void *arg);


#endif

