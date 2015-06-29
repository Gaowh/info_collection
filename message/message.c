#include "../include/message.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "../log/infolog.h"

int message_pack(struct info_queue *infoq, void *message)
{

	struct info_from_task *info;
	struct info_from_task *iter_info = NULL;

	struct messagehdr *mshdr;
	struct list_head *tlist;

	mshdr = (struct messagehdr *)message;

	
	mshdr->message_type = htonl(MESSAGE_PUSH);
	//mshdr->message_count = infoq->info_in_queue;

	char *payload =(char *)(mshdr + 1);
	char *pptr = payload;

	info = list_first_or_null(&infoq->list, struct info_from_task,list);
	
	while(info){
		
		list_del(&info->list);
		if(info->redundancy == 1)
			goto cont;

		list_for_each_entry(iter_info, &infoq->list, list){

			if (info->wordlen < iter_info->wordlen){
				
				if (word_cmp(info->keyword, iter_info->keyword, info->wordlen) == 0)
					goto cont;
			}

			else if (info->wordlen > iter_info->wordlen){
			
				if(word_cmp(iter_info->keyword, info->keyword, iter_info->wordlen) == 0){
					iter_info->redundancy = 1;			
				}
			}
		}

		#ifdef DEBUGPACK
			printf("\nPack one info\n");
			printf("Info time: %s", asctime(&info->infotime));
			printf("Info addrtypr: %d\n", info->addrtype);
			printf("Info searchtype: %d\n", info->searchtype);
			printf("Info keyword len: %d\n", info->wordlen);
			printf("Info keyword: %s\n", info->keyword);
		#endif
		
		memcpy(pptr, &info->infotime, sizeof(info->infotime));
		pptr += sizeof(struct tm);
		
		memcpy(pptr, &info->addrtype, sizeof(info->addrtype));
		pptr += sizeof(info->addrtype);

		memcpy(pptr, &info->searchtype, sizeof(info->searchtype));
		pptr += sizeof(info->searchtype);

		memcpy(pptr, &info->wordlen, sizeof(info->wordlen));
		pptr += sizeof(info->wordlen);

		memcpy(pptr, info->keyword, info->wordlen);
		pptr += info->wordlen;

		memcpy(pptr, "\r\n", 2);
		pptr += 2;

	cont:
		free(info);
		info = NULL;
		info = list_first_or_null(&infoq->list, struct info_from_task,list);
	}	
		
	mshdr->message_total_length = htonl(pptr - payload);
	#ifdef DEBUGPACK
	printf("Message length: %d\n", pptr - payload);
	#endif
	
	char buf[MAX_LOG_BUF];
	memset(buf, '\0', MAX_LOG_BUF);
	sprintf(buf, "packet info success and packet length is: %d", (int)(pptr-payload));
	do_log(buf, strlen(buf),LEVEL_INFO);

	return pptr - payload;
}

int message_send(struct sockaddr_in *serveraddr, void *message, int messagelen)
{
	int 	ret;
	int 	sockfd;
	int 	nleft,nwrite;
	char 	*ptr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		WRONG_MESSAGE_ERRNO("Sock error", strerror(errno));
		return -1;
	}

	ret = connect(sockfd,(struct sockaddr *)serveraddr, sizeof(struct sockaddr));
	if(ret == -1){
		
		char buf[MAX_LOG_BUF];
		char ip[16];
		memset(buf, '\0', MAX_LOG_BUF);

		sprintf(buf, "connect to server error: %s, serveraddr: %s:%d", strerror(errno), server1_ip, server1_port);
		do_log(buf, strlen(buf), LEVEL_ERR);

		//连接备用服务器
		if (server2_ip != NULL){
		
			struct sockaddr_in server2;

			memset(&server2, 0, sizeof(server2));
			server2.sin_family = AF_INET;
			server2.sin_port = htons(server2_port);
			inet_pton(AF_INET, server2_ip, &server2.sin_addr.s_addr);

			ret = connect(sockfd, (struct sockaddr*)&server2, sizeof(server2));
			if(ret == -1){
				char buf[MAX_LOG_BUF];
				char ip[16];
				memset(buf, '\0', MAX_LOG_BUF);

				sprintf(buf, "connect to server error: %s, serveraddr: %s:%d", strerror(errno), server2_ip, server2_port);
				do_log(buf, strlen(buf), LEVEL_ERR);
				WRONG_MESSAGE_ERRNO("Connect server2 error",strerror(errno));
				return -1;
			}
			else{
				char buf[MAX_LOG_BUF];
				char ip[16];
				memset(buf, '\0', MAX_LOG_BUF);
				sprintf(buf, "connect to server success, serveraddr: %s:%d", server2_ip, server2_port);
				do_log(buf, strlen(buf), LEVEL_INFO);
			}
		}
		else{
			WRONG_MESSAGE_ERRNO("Connect error", strerror(errno));
			return -1;
		}
	}

	else {
		char buf[MAX_LOG_BUF];
		char ip[16];
		memset(buf, '\0', MAX_LOG_BUF);
		sprintf(buf, "connect to server success, serveraddr: %s:%d", server1_ip, server1_port);
		do_log(buf, strlen(buf), LEVEL_INFO);
	}


//	sockfd = open("/bh_manage/info_collection/logfile", O_RDWR | O_CREAT| O_APPEND, S_IRUSR | S_IWUSR);
	
	fprintf(stderr,"Message length: %d\n", messagelen); 
	nleft = messagelen + 8;
	ptr = message;
	
	int count =0;

	while(nleft > 0){
		nwrite = write(sockfd, ptr, nleft);
		if(nwrite <= 0 ){
			if(nwrite == -1 && errno == EINTR){
				nwrite = 0;
			}
			else{
				char buf[MAX_LOG_BUF];
				char ip[16];
				memset(buf, '\0', MAX_LOG_BUF);
				sprintf(buf, "write to server failed: %s", strerror(errno));
				do_log(buf, strlen(buf), LEVEL_ERR);

				return -1;
			}
		}
		count +=  nwrite;

		nleft -= nwrite;
		ptr += nwrite;
	}

	fprintf(stderr,"write to server success  %d\n...\n", count);
	char buf[MAX_LOG_BUF];
	char ip[16];
	memset(buf, '\0', MAX_LOG_BUF);
	sprintf(buf, "write to server success, write %d bytes", messagelen);
	do_log(buf, strlen(buf), LEVEL_INFO);

	close(sockfd);
	return 1;
}
