#include "../include/pthread_func.h"
#include "../include/code.h"
#include "../include/task.h"
#include "../include/message.h"
#include "../hash-table/hash.h"
#include "../log/infolog.h"
#include "../kmp-mt/kmp.h"

void *worker_func(void *arg)
{
	extern struct task_queue task_q;
	extern struct info_queue info_q;
	int 	ret;

	//创建 gb2312编码转换到unicode编码的编码表，这里用一个二维数组表示
	gb_to_unicode();
	do_log("create encode table success", 27, LEVEL_INFO);

	//初始化哈希表
	hashtable_init();
	do_log("init hash table success",23, LEVEL_INFO);
	
	//初始化用于操作哈希表的互斥变量
	pthread_mutex_init(&hash_mutex, NULL);
	//hash_mutex = PTHREAD_MUTEX_INITIALIZER;
	while(1){
		pthread_mutex_lock( &(task_q.task_mutex));
		
		#ifdef DEBUGTASKTHREAD
			printf("\n***************Thread %d\n",pthread_self());
		#endif

		while( task_q.task_in_queue <= 0) {
			(task_q.thread_waiting)++;
			
			#ifdef DEBUGTASKTHREAD
			printf("Wating task...\n");
			#endif
			
			pthread_cond_wait(&(task_q.task_cond), &(task_q.task_mutex));
		
			#ifdef DEBUGTASKTHREAD
				printf("Thread %u wake\n", pthread_self());
			#endif
		}

		(task_q.task_in_queue)--;

	
		struct task *new_task = list_first_entry(&task_q.queue_head, struct task, list);
		
		#ifdef DEBUGTASKCHECK 

			struct ulog_packet_msg *msghdr;
			struct nlmsghdr *nlhdr;
			char *prefix;

			nlhdr = (struct nlmsghdr * )new_task->buffer;
			nlhdr++;
			msghdr = (struct ulog_packet_msg *)nlhdr;
			prefix = msghdr->prefix;

			printf("New_task index: %d\n", new_task->index);
			printf("len: %d\n",new_task->buflen);
			printf("prefix: %s\n", prefix);
		#endif

		task_del(&new_task->list);
		
		pthread_mutex_unlock( &(task_q.task_mutex));

			
		struct info_from_task *info = (struct info_from_task *)malloc(sizeof(struct info_from_task));

		ret = abstract_info(info, new_task);
		if (ret == 0 || ret == -1){
			
			free(info);
			continue;
		}
		
			
		//判断是否出现冗余
		//
		pthread_mutex_lock(&hash_mutex);
		if(hash_find_keyword(info->keyword, info->wordlen)){
			printf("keyword Already exist...\n");
			free(info);
			pthread_mutex_unlock(&hash_mutex);
			continue;
		}

		hash_insert_keyword(info->keyword, info->wordlen);
		pthread_mutex_unlock(&hash_mutex);
	
		#ifdef CHECKINFO
			printf("Info time:	  %s,",asctime(&info->infotime));
			printf("Info addrtype:    %d\n", info->addrtype);
			printf("Info searchtype:  %d\n", info->searchtype);
			printf("Info wordlength:  %d\n", info->wordlen);
			printf("Info keyword:     %s\n\n", info->keyword);
		#endif


		pthread_mutex_lock(&info_q.info_mutex);
		info_q_add(info, &info_q);
	
		#ifdef CHECKINFOQ
			printf("Info in queue: %d\n", info_q.info_in_queue);
		#endif
	
		if(info_q.info_in_queue > keep_max ){
			printf("keep_Max: %d\n", keep_max);	
			int ret = 0;
			void  *messagepack = malloc(sizeof(struct messagehdr) + info_q.info_in_queue * MAX_INFO_SIZE);

			ret = message_pack(&info_q, messagepack);
			if(ret == -1){
				WRONG_MESSAGE("PACKET  ERROR");
				goto out;
			}

			ret = message_send(&info_q.server1, messagepack, ret);

			out:
				INIT_LIST_HEAD(&info_q.list);
				info_q.info_in_queue = 0;
				free(messagepack);
		}
		pthread_mutex_unlock(&info_q.info_mutex);

		free(new_task);
		new_task = NULL;
		#ifdef DEBUGTASKTHREAD
			printf("\n*************Thread %d end\n", pthread_self());
		#endif
	}
	return (void *)0;
}

int abstract_info( struct info_from_task *info, struct task * tsk)
{	
	int ret;
	struct tm *timenow;
	time_t now;

	
	struct ulog_packet_msg *msghdr;
	struct nlmsghdr *nlhdr;

	nlhdr = (struct nlmsghdr *)tsk->buffer;
	nlhdr++;
	msghdr = (struct ulog_packet_msg *)nlhdr;

	//从数据包种得到信息产生的时间                                                              
	now = (time_t)msghdr->timestamp_sec;
	timenow = localtime(&now);
	info->infotime = *timenow;

	//从prefix中得到信息的类型                                                                  
	info->addrtype = ((msghdr->prefix[0]) - '0');
	info->searchtype = ((msghdr->prefix[1]) - '0');
	info->redundancy = 0;	
	//电脑端的搜索                                                            
	if(info->searchtype == TYPE_SEARCH){
		switch(info->addrtype){
			case TYPE_JD:
				
				ret = jd_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				
				#ifdef DEBUGINFO
				printf(("Search from www.jd.com\n"));
				#endif
				break;

			case TYPE_TM:
				ret = tm_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				
				#ifdef DEBUGINFO
				printf(("Search from www.tmall.com\n"));
				#endif
				break;

			case TYPE_TB:
				ret = tb_get_keyword(info->keyword, tsk->buffer,tsk->buflen);
				
				#ifdef DEBUGINFO
				printf(("Search from www.taobao.com\n"));
				#endif
				break;

			case TYPE_SN:
				ret = sn_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				#ifdef DEBUGINFO
				printf("Search from www.suning.com");
				#endif
				break;
			
			case TYPE_PP:
			 	ret = pp_get_keyword(info->keyword,tsk->buffer, tsk->buflen);
				break;
			
			case TYPE_DD:
				ret = dd_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;
			case TYPE_YX:
				ret = yx_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;
			
			default:
				printf("desktop known Type...\n");
		}
	}

	//移动端的搜索
	else if (info->searchtype == TYPE_MSEARCH) {
		
		switch(info->addrtype){
			
			case TYPE_JD:
				printf("Search from mobile jd.com...\n");
				ret = m_jd_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;

			case TYPE_TM:
				printf("Search from mobile tm.com...\n");
				ret = m_tm_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;

			case TYPE_SN:
				printf("Search from mobile suning.com...\n");
				ret = m_sn_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;
			
			case TYPE_YX:
				printf("Search from mobile yixun.com...\n");
				ret = m_yx_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;

			case TYPE_YHD:
				printf("Search from mobile yihaodian.com...\n");
				ret = m_yhd_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;

			case TYPE_DD:
				printf("Search from mobile dangdang.com...\n");
				ret = m_dd_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;

			case TYPE_PP:
				printf("Search from mobile paipai.com...\n");
				ret = m_pp_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;
			
			case TYPE_JMYP:
				printf("Search from mobile jumeiyouping.com...\n");
				ret = m_jmyp_get_keyword(info->keyword, tsk->buffer, tsk->buflen);
				break;

			default:
				printf(" type: %d   mobile known Type...\n", info->addrtype);
		
		}
	}
	info->wordlen = ret;
	return ret;
}


int get_keyword(char *keyword, char *urlcode, size_t urllen)
{
	
	printf("Url: %s\n", urlcode);
	int ret;
	
	if(urllen == 0){
		WRONG_MESSAGE("Invalid url");
		return 0;
	}

	char *durl = (char *)malloc(urllen);
	
	ret = urldecode(urlcode, urllen, durl, 0);
	if(ret == -1){
		WRONG_MESSAGE("Bad url");
		goto out;
	}
	ret = url_to_utf8(keyword, durl, ret);
	if(ret == -1){
		WRONG_MESSAGE("Uknown edcode");
		goto out;
	}
	
	out:
		free(durl);
		return ret;
}


int jd_get_keyword(char *keyword, char *buffer, int buflen)
{
	return m1_get_keyword(keyword, buffer, buflen);
}

int m_jd_get_keyword(char *keyword, char *buffer, int buflen)
{
	char *ptr1, *ptr2;
	char *url;
	int  ret;
	int urllen;
	int flag = 0;

	ptr1 = pktstr(buffer, buflen, "keyword=");
	if(!ptr1){
		ptr1 = (pktstr(buffer, buflen, "keyword"));
		if(! ptr1){

			WRONG_MESSAGE("PTR1 null");
			keyword = NULL;
			return 0;
		}
		else ptr1 += 16;
		flag = 1;
	}

	else  ptr1 += 8;

	ptr2 = strstr(ptr1, "&");
	if(!ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}

	if(flag == 1) urllen = ptr2 - ptr1 - 6;
	else urllen = ptr2-ptr1;

	url = (char *)malloc(urllen+1);
	memcpy(url,ptr1, urllen);
	url[urllen] = '\0';

	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out :
		free(url);
		return ret;
}

int tm_get_keyword(char *keyword, char *buffer, int buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "search_product.htm?q=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 21;
	ptr2 = strstr(ptr1, "&");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	url = memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;

}

int m_tm_get_keyword(char *keyword, char *buffer, int buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;

	ptr1 = pktstr(buffer, buflen, "q=");
	if(!ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}

	ptr1 += 2;
	
	ptr2 = ptr1;
	while(*ptr2 != '&' && *ptr2 != ' '){
		ptr2 ++;
	}
	
	if(!ptr2){
		ptr2 = strstr(ptr1, " ");
		if(!ptr2){

			WRONG_MESSAGE("PTR2 NULL");
			keyword = NULL;
			return 0;
		}
	}

	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';

	ret = get_keyword(keyword, url, urllen);

	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

out:
	free(url);
	return ret;
}


int tb_get_keyword(char *keyword, char *buffer, int buflen)
{
	
	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "search?q=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 9;
	ptr2 = strstr(ptr1, "&");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;

}


int sn_get_keyword(char *keyword, char *buffer, int buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;

	ptr1 = pktstr(buffer, buflen, "mobileSearch");
	if(ptr1 == NULL) {
		return 0;
	}

	ptr1 = pktstr(buffer, buflen, "GET /");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 5;
	ptr2 = strstr(ptr1, "/");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;

}

int m_sn_get_keyword(char *keyword, char *buffer, int buflen)
{
	return m1_get_keyword(keyword, buffer, buflen);
}

int m1_get_keyword(char *keyword, char *buffer, int buflen)
{
	
	char *ptr1, * ptr2, *url;
	int ret, urllen;

	ptr1 = pktstr(buffer, buflen, "keyword=");
	if(!ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}

	ptr1 += 8;
	ptr2 = strstr(ptr1, "&");
	if(!ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}

	urllen =ptr2 - ptr1;
	url = (char *)malloc(urllen +1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';

	ret = get_keyword(keyword, url ,urllen);

	if(ret == -1){
	
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

out:
	free(url);
	return ret;
}


int pp_get_keyword(char *keyword, char *buffer, int buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "KeyWord=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 8;
	ptr2 = strstr(ptr1, "&");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';

	printf("Url: %s\n", url);
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;

}

int dd_get_keyword(char *keyword, char *buffer, int buflen)
{
	
	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "key=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 4;
	ptr2 = strstr(ptr1, " ");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;
}

int m_yx_get_keyword(char *keyword, char *buffer, int  buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "q=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 2;
	ptr2 = strstr(ptr1, "&");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;
}


int yx_get_keyword(char *keyword, char *buffer, int  buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "key=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 4;
	ptr2 = strstr(ptr1, "&");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;
}

int m_yhd_get_keyword(char *keyword, char *buffer, int  buflen)
{
	
	int ret;
	ret = m1_get_keyword(keyword, buffer, buflen);
	if(ret == 0){

		char *ptr1, *ptr2;
		char *url;
		int ret;
		int urllen;

		ptr1 = pktstr(buffer, buflen, "getkeyword/");
		if(! ptr1){
			WRONG_MESSAGE("PTR1 NULL");
			keyword = NULL;
			return 0;
		}
	
		ptr1 += 11;
		ptr2 = strstr(ptr1, "?");
		if(! ptr2){
			WRONG_MESSAGE("PTR2 NULL");
			keyword = NULL;
			return 0;
		}
		
		urllen = ptr2 - ptr1;
		url = (char *)malloc(urllen + 1);
		memcpy(url, ptr1, urllen);
		url[urllen] = '\0';
		ret = get_keyword(keyword, url, urllen);
		if(ret == -1){
			WRONG_MESSAGE("GET KEYWORD FAILED");
			goto out;
		}

	out:
		free(url);
		return ret;

	}	
	else 
		return ret;
}

int m_dd_get_keyword(char *keyword, char *buffer, int  buflen)
{

	return m1_get_keyword(keyword, buffer, buflen);
}

int m_pp_get_keyword(char *keyword, char *buffer, int  buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "KeyWord=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 8;
	ptr2 = strstr(ptr1, " ");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;
}

int m_jmyp_get_keyword(char *keyword, char *buffer, int  buflen)
{

	char *ptr1, *ptr2;
	char *url;
	int ret;
	int urllen;


	ptr1 = pktstr(buffer, buflen, "keyword=");
	if(! ptr1){
		WRONG_MESSAGE("PTR1 NULL");
		keyword = NULL;
		return 0;
	}
	
	ptr1 += 8;
	ptr2 = strstr(ptr1, " ");
	if(! ptr2){
		WRONG_MESSAGE("PTR2 NULL");
		keyword = NULL;
		return 0;
	}
	
	urllen = ptr2 - ptr1;
	url = (char *)malloc(urllen + 1);
	memcpy(url, ptr1, urllen);
	url[urllen] = '\0';
	ret = get_keyword(keyword, url, urllen);
	if(ret == -1){
		WRONG_MESSAGE("GET KEYWORD FAILED");
		goto out;
	}

	out:
		free(url);
		return ret;
}
