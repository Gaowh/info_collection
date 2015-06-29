#include "../include/task.h"
#include "../include/code.h"
#include "../log/infolog.h"

void init_task( struct task_queue *task_q)
{	
	INIT_LIST_HEAD(&task_q->queue_head);
	
	pthread_mutex_init( &(task_q->task_mutex), NULL);
	pthread_cond_init(&(task_q->task_cond), NULL);
	task_q->task_in_queue = 0;
	task_q->thread_waiting = 0;
}

void init_info_q(struct info_queue * info_q)
{
	
	readconfig();
	//初始化链表节点
	INIT_LIST_HEAD(&info_q->list);

	//初始化互斥量和条件变量
	pthread_mutex_init(&(info_q->info_mutex), NULL);
	pthread_cond_init(&(info_q->info_cond), NULL);


	//信息队列中的信息总长度置0	
	info_q->info_in_queue = 0;
	
	//初始化一个服务器的地址结构，队列满时消息发往的服务器
	if(server1_ip != NULL){
		bzero(&info_q->server1, sizeof(struct sockaddr_in));
		info_q->server1.sin_family = AF_INET;
		info_q->server1.sin_port = htons(server1_port);
		inet_pton(AF_INET, server1_ip, &info_q->server1.sin_addr);
	}
}

//向任务队列加入一个任务
void task_add(struct task *newtask, struct task_queue *task_q)
{

	struct list_head *new;
	struct list_head *head;

	new = &newtask->list;
	head = &task_q->queue_head;

	list_add(new, head);

	task_q->task_in_queue++;
}

//在任务队列中取出一个任务后将其删除
void task_del(struct list_head *list)
{

	list_del(list);
}

//信息队列加入一个已处理信息
void info_q_add(struct info_from_task *info , struct info_queue * info_q)
{

	struct list_head *new;
	struct list_head *head;

	new = &info->list;
	head = &info_q->list;

	list_add(new, head);

	info_q->info_in_queue++;

}

//信息队列删除一个已处理信息
void info_q_del(struct list_head * del)
{

	list_del(del);
}

void readconfig()
{
	int fd ;
	FILE * fp;
	fp = fopen("/bh_manage/info_collection/config", "r");
	if(fp ==NULL){
		do_log("fopen error", 11, LEVEL_ERR);
		return;
	}

	char *buf = (char *)malloc(200);
	memset(buf, '\0', 200);
	int ret = 0;
	int len=200;

	char *s, *e;
	
	char tmp[5];
	bzero(tmp,5);

	bzero(server2_ip, 16);
	bzero(server1_ip, 16);
	while(ret = getline(&buf, &len, fp)!=-1)
	{
		//
		if( (s=strstr(buf, "SERVER1"))!= NULL || (s=strstr(buf, "SERVER2"))!=NULL){
			
			do_log(buf, len, LEVEL_INFO);
			if((s = strstr(buf, "SERVER1"))!=NULL && s==buf){
		
				s =strstr(s, "=");
				s+=1;

				e = strstr(s,":");
				if(!e){
					continue;
				}

				memcpy(server1_ip, s, (int)(e-s));

				e +=1;
				s = strstr(e, "\n");
				bzero(tmp, 5);
				memcpy(tmp, e, s-e);
				server1_port = atoi(tmp);
			}

			else if((s = strstr(buf, "SERVER2")) !=NULL && s == buf){
				
				s =strstr(s, "=");
				s+=1;
				e = strstr(s,":");
				if(!e){
					continue;
				}

				memcpy(server2_ip, s, (int)(e-s));

				e +=1;
				s = strstr(e, "\n");
				bzero(tmp,5);
				memcpy(tmp, e, s-e);
				server2_port = atoi(tmp);
			}
		}

		if((s = strstr(buf, "KEEP_MAX=")) != NULL && s==buf){
			
			do_log(buf, len, LEVEL_INFO);
			s += 9;
			e = strstr(s, "\n");

			bzero(tmp, 5);
			memcpy(tmp, s, e-s);

			keep_max  =atoi(tmp);
		}
	}
}
