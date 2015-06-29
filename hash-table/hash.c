#include "hash.h"
#include "../log/infolog.h"


int hashfunc(unsigned char *keyword, int wordlen)
{

	//将一段连续的内存散列成一个哈希值
	//
	//具体的方法是，将每个字节叠加起来，然后在mod哈希表的长度           
	
	unsigned char *ptr;
	int hashkey =  0;

	ptr = keyword;
	while(wordlen--) hashkey += *ptr++;
	hashkey %= MAX_HASH_NODE;
	return hashkey;
}


void hashtable_init()
{
	int i;
	
	for(i=0; i<MAX_HASH_NODE; i++){
	
		hashtable[i].next = NULL;
		hashtable[i].keyword = NULL;
		hashtable[i].keylen = 0;
	}
}

int hash_find_keyword(unsigned char *keyword, int wordlen)
{
	int index;
	struct node *tmp;

	index = hashfunc(keyword, wordlen);
	tmp = hashtable[index].next;

	while(tmp){
		
		if(tmp->keylen != wordlen){
			tmp = tmp->next;
			continue;
		}

		if(!memcmp(keyword, tmp->keyword, wordlen))
			return 1;
		tmp = tmp->next;
	}
	return 0;
}

void hash_insert_keyword(unsigned char *keyword, int wordlen)
{

	if(hash_find_keyword(keyword, wordlen)) return;

	int index = hashfunc(keyword, wordlen);

	struct node *tmp;
	struct node *new = (struct node *)malloc(sizeof(struct node));

	tmp = hashtable[index].next;
	new->keyword = keyword;
	new->keylen =wordlen;
	new->next = tmp;
	hashtable[index].next = new;
}

int hash_del_keyword(unsigned char *keyword, int wordlen)
{

	//删除成功返回1，  没有该节点返回0
	int index = hashfunc(keyword, wordlen);
	struct node *tmp, *pretmp;
	pretmp = &hashtable[index];
	tmp = pretmp->next;

	while(tmp){
		if(tmp->keylen != wordlen)
		{	
			tmp = tmp->next;
			continue;
		}
		if(!memcmp(keyword, tmp->keyword, wordlen))
		{
			//删除成功
			pretmp->next = tmp->next;
			free(tmp);
			return 1;
		}
		pretmp = tmp;
		tmp = tmp->next;
	}
	return 0;
}

void clear_hashtable()
{
	
	int i=0;
	struct node *tmp, *aftertmp;

	for(i=0; i<MAX_HASH_NODE; i++){
	
		tmp = hashtable[i].next;
		while(tmp){
			aftertmp = tmp->next;
			free(tmp);
			tmp = aftertmp;
		}
		hashtable[i].next = NULL;
	}
}

void *hash_func(void *arg)
{

	//没有进行错误检查
	//
	//
	//主要做用是定期清理hash表中的内容
	//
	//
	//次要作用是在程序运行初设置好iptables
	//
	//
	//在程序收到SIGTERM信号时清除iptables
	


	struct itimerspec newtime;
	int timerfd;

	//同步时间  时间服务器是复旦的服务器
	system("ntpdate ntp.fudan.edu.cn >> /dev/null");

	//设置iptables的规则
	
	system("/bh_manage/info_collection/scripts/set.sh");
	do_log("start running, setiptables ok", 29, LEVEL_INFO);

	timerfd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);

	clock_gettime(CLOCK_REALTIME, &newtime.it_value);

	newtime.it_value.tv_sec += 1;

	newtime.it_interval.tv_sec = CLEAR_TIMESLOT;
	newtime.it_interval.tv_nsec = 0;

	timerfd_settime(timerfd, TFD_TIMER_ABSTIME, &newtime, NULL);

	//向epoll注册定时器描述符
	int epfd, ret;
	struct epoll_event timerevent;

	epfd = epoll_create(5);
	assert(epfd > 0);

	timerevent.events = EPOLLIN;
	timerevent.data.fd = timerfd;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, timerfd, &timerevent);
	assert (ret != -1);

	//向epoll注册新号描述符
	int sigfd;
	sigset_t mask;
	struct epoll_event sigevent;

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigfd = signalfd(-1, &mask, SFD_CLOEXEC);
	
	sigevent.events = EPOLLIN;
	sigevent.data.fd = sigfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sigfd, &sigevent);
	
	int nfds, i;
	long long buf;
	struct epoll_event ev[3];
	struct signalfd_siginfo siginfo;
	while(1){
	
		nfds = epoll_wait(epfd, ev, 3, -1);
		if (nfds == -1) continue;
	
		for(i=0; i<nfds; i++){
		
			int fd = ev[i].data.fd;
			if(fd == timerfd){
				
				ret = read(timerfd , &buf, sizeof(buf));
			
				//处理哈希表
				pthread_mutex_lock(&hash_mutex);
				clear_hashtable();
				pthread_mutex_unlock(&hash_mutex);
			}	
			else if(fd == sigfd){
				ret = read(sigfd, &siginfo, sizeof(siginfo));

				int sig = siginfo.ssi_signo;
				if(sig == SIGTERM){
					
					//清理iptables并退出整个进程
					fprintf(stderr, "Catched SIGTERM, now clear iptables...\n");
					
					system("/bh_manage/info_collection/scripts/unset.sh");
					fprintf(stderr, "iptables clear success...\n");
					
					fprintf(stderr, "now quit...\n");
					do_log("catched SIGTERM, quit",21, LEVEL_ERR);
					exit(EXIT_SUCCESS);
				}

				else if(sig == SIGUSR1){
						
					//重新读取配置文件（这里可以做很多事，但现在只是重新设置一下iptables）
					fprintf(stderr, "Catched SIGUSR1, now reset iptables...\n");
					
					system("/bh_manage/info_collection/scripts/unset.sh");
					fprintf(stderr, "iptables clear succeess...\n");

					system("/bh_manage/info_collection/scripts/set.sh");
					fprintf(stderr, "reset iptables success...\n");
					do_log("reset iptables", 14, LEVEL_INFO);
				}

				else if(sig == SIGUSR2){
					//重新读入服务器的地址
				}

				else {
					fprintf(stderr, "unknow signo...\n");
					do_log("catched unknown sig", 19, LEVEL_ERR);
				}
			}
		}
	}
}



