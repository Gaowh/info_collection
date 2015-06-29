#ifndef _TASK_H
#define _TASK_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<linux/netlink.h>
#include<errno.h>
#include<curses.h>
#include<netinet/in.h>
#include<signal.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<sys/un.h>
#include<pthread.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<linux/netfilter_ipv4/ipt_ULOG.h>
#include<pthread.h>


#include "./mylist.h"

typedef size_t type_t;

//每类网站对应不同的消息类型

#define TYPE_JD 1
#define TYPE_TM 2
#define TYPE_TB 3
#define TYPE_SN 4
#define TYPE_PP 5
#define TYPE_DD 6
#define TYPE_YX 7
#define TYPE_YHD 8
#define TYPE_JMYP 9

//信息来源类型： 点击、搜索
#define TYPE_CLICK 3
#define TYPE_SEARCH 1
#define TYPE_MSEARCH 2

#define MAX_BUF_SIZE 4096
#define NETLINK_GROUP_ID 12



#define WRONG_MESSAGE(message)\
	fprintf(stderr,"## @file %s @function %s @line %d ##",\
		__FILE__, __func__, __LINE__);\
	fprintf(stderr,message);\
	fprintf(stderr,"\n");

#define WRONG_MESSAGE_ERRNO(message, error)\
	fprintf(stderr,"## @file %s @function %s @line %d ##",\
		__FILE__, __func__, __LINE__);\
	fprintf(stderr,message, error);\
	fprintf(stderr,"\n");

 
#define MAX_INFO_SIZE sizeof(struct info_from_task)

int keep_max;
/*
 	任务队列的对头，用于管理所有未处理的任务
	主线程从内核收到任务后将任务加入该队列
	任务线程从该队列中获取任务
*/
struct task_queue{
		
	struct list_head queue_head;
	pthread_mutex_t task_mutex;
	pthread_cond_t task_cond;
	size_t task_in_queue;
	size_t thread_waiting;
};


/*
	代表每一个任务，收到任务时加入任务队列
*/
struct task{

	struct 	list_head list;
	char	buffer[MAX_BUF_SIZE];
	size_t 	buflen;
	size_t 	index;
};

/*
 用于保存从每个task中提取出来的信息
 */

struct info_from_task{
	
	struct list_head list;
	struct tm infotime;
	/*
		 消息类型，包括来自的网站，产生的事件
	 */	
	type_t 	addrtype; 
	type_t 	searchtype;
	size_t 	wordlen;
	char 	keyword[33];
	short 	redundancy;

};

/*
 用来缓存提取到的每个信息，待队列到达一定长度时就将队列上的信息发送给服务器
 */

struct info_queue{
	
	struct 	list_head list;
	pthread_mutex_t info_mutex;
	pthread_cond_t 	info_cond;
	size_t 	info_in_queue;
	struct 	sockaddr_in server1;
	struct 	sockaddr_in server2;
};

void init_task( struct task_queue *task_q);

void init_info_q(struct info_queue *info_q);

struct task_queue task_q;

struct info_queue info_q;

void readconfig();

char server1_ip[16];
int server1_port;

char server2_ip[16];
int server2_port;

#endif

