#include "./include/task.h"
#include "./include/mylist.h"
#include "./include/code.h"
#include "./include/pthread_func.h"
#include "hash-table/hash.h"

int main()
{
	int nf_sock;
	struct msghdr msg;
	struct nlmsghdr nlhdr;
	struct sockaddr_nl local_addr;
	struct sockaddr_nl kpeer_addr;
	char 	buf[MAX_BUF_SIZE];
	int 	nres;
	int 	group_id = NETLINK_GROUP_ID;
	pid_t	pid;
	int addr_len;
	
	nf_sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_NFLOG);
	if(nf_sock < 0){
	
		WRONG_MESSAGE_ERRNO("Create nf_sock error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	bzero(&local_addr, sizeof(struct sockaddr_nl));
	bzero(&kpeer_addr, sizeof(struct sockaddr_nl));

	local_addr.nl_family = PF_NETLINK;
	local_addr.nl_pad = 0;
	local_addr.nl_pid = getpid();
	local_addr.nl_groups = group_id;

	if(nres =  bind(nf_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_nl)) != 0){
	
		WRONG_MESSAGE_ERRNO("Bind nf_sock error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	nres = setsockopt( nf_sock, 270, NETLINK_ADD_MEMBERSHIP, &group_id, sizeof(group_id));
	if(nres == -1){
	
		WRONG_MESSAGE_ERRNO("Set nf_sock option error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//设置所有线程都阻塞信号，用单独的线程来处理所有的信号
	sigset_t mask, oldmask;
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);

	pthread_sigmask(SIG_BLOCK, &mask, &oldmask);

	//初始化任务队列
	extern struct task_queue task_q;
	init_task(&task_q);

	//初始化发送任务队列
	extern struct info_queue info_q;
 	init_info_q(&info_q);

	//创建用于处理数据的线程组，线程从任务队列中获取任务
	pthread_t tid;
	nres = pthread_create( &tid, NULL, worker_func, NULL);
	nres = pthread_create(&tid, NULL, hash_func, NULL);

	//创建一个task结构，用于接收新的task
	struct task *new_task;

	int test_index = 0;

	char buffer[MAX_BUF_SIZE];

	while(1){
		
		
		bzero(buffer, MAX_BUF_SIZE);
		nres = recvfrom(nf_sock, buffer, MAX_BUF_SIZE, 0, (struct sockaddr *)&kpeer_addr, &addr_len);
		if( nres < 0){
			
			fprintf(stderr, "Recv data from kernel error: %s\n", strerror(errno));
			continue;
		}
		
		#ifdef DEBUGMAIN
		printf("\n***************Main thread start\n");
		#endif
		new_task = (struct task *)malloc(sizeof(struct task));
		
		new_task->index = test_index++;
		memcpy(new_task->buffer, buffer, nres);
		new_task->buflen = nres;
		
		/*
		struct ulog_packet_msg *msghdr;
		struct nlmsghdr *nlhdr;
		nlhdr = (struct nlmsghdr * )new_task->buffer;
		nlhdr++;
		msghdr = (struct ulog_packet_msg *)nlhdr;
		*/
		

		//锁住互斥量
		pthread_mutex_lock(&(task_q.task_mutex));
		
		//将新接收到的任务加入队列
		//list_add(&new_task->list, &task_q.queue_head);
		task_add(new_task, &task_q);
		
		#ifdef DEBUGMAIN
		printf("\nQueue Details:\n");
		printf("Task in queue: %d\n", task_q.task_in_queue);
		printf("Thread waiting: %d\n",task_q.thread_waiting);
		#endif

		int flag = 0;
		
		//判断是否有线程在等待队列上加入任务
		if(task_q.thread_waiting > 0) 
		{	
			flag = 1;
			task_q.thread_waiting--;
		}

		pthread_mutex_unlock(&(task_q.task_mutex));

		//如果有线程在等待，则在该条件变量上发信号
		//
		//
		//
		//
		if(flag == 1) {
			
			#ifdef DEBUGMAIN
			printf("Some thread are waiting task , now send signal...\n");
			#endif
			
			pthread_cond_signal( &(task_q.task_cond));
		}
		
		#ifdef DEBUGMAIN
		printf("****************Main thread end\n\n");
		#endif
	}
}

