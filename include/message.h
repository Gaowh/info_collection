#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "mylist.h"
#include "task.h"

#define MESSAGE_PUSH 5
/*
	可以扩充另外的消息类型
*/

struct messagehdr{
	size_t message_total_length;
	type_t message_type;
};

int message_pack(struct info_queue *infoq, void *message);

int message_send(struct sockaddr_in *serveraddr, void  *message, int messagelen);

#endif


