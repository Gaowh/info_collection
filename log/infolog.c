
#include "infolog.h"
#include <fcntl.h>

int open_log()
{
	int fd = open(LOGPATH, O_CREAT | O_WRONLY| O_APPEND, 0666);
	
	if(fd == -1){
		fprintf(stderr, "open logfile error: %s...\n", strerror(errno));
		return -1;
	}

	else return fd;
}

int do_log(char *comments, int len, int level)
{
	
	int fd;
	int wleft;
	int ret;
	char *wptr, logbuf[MAX_LOG_BUF];

	fd = open_log();
	if(fd == -1) return -1;
	
	struct timespec time;
	struct tm *now;
	time_t tmp;

	clock_gettime(CLOCK_REALTIME, &time);
	tmp = (time_t)time.tv_sec;
	now = localtime(&tmp);

	memset(logbuf, '\0', MAX_LOG_BUF);
	switch(level){
	
		case LEVEL_ERR:
			sprintf(logbuf, "\n###ERR### ");
			strncat(logbuf, asctime(now), strlen(asctime(now))-1);
			strncat(logbuf, " ### ", 5);
			strncat(logbuf, comments, len);
			break;

		case LEVEL_DEBUG:
			
			sprintf(logbuf, "\n###DEBUG### ");
			strncat(logbuf, asctime(now), strlen(asctime(now))-1);
			strncat(logbuf, " ### ", 5);
			strncat(logbuf, comments, len);
			break;

		case LEVEL_INFO:
			
			sprintf(logbuf, "\n###INFO### ");
			strncat(logbuf, asctime(now), strlen(asctime(now))-1);
			strncat(logbuf, " ### ", 5);
			strncat(logbuf, comments, len);
			break;

		default:
			fprintf(stderr, "unknown level\n");
			ret = -1;
	}
	strcat(logbuf, "\n");
	ret = writelog(fd, logbuf, strlen(logbuf));
	return ret;
}


int writelog(int fd, char * logbuf, int len)
{

	int wleft, ret;
	char *wptr;

	wleft = len;
	wptr = logbuf;

	while(wleft > 0){
				
		ret = write(fd, wptr, wleft);
		if(ret == -1){
			if(errno == EINTR)
				continue;
			else 
				return -1;
		}
		wleft -= ret;
		wptr += ret;
	}
	return len;
}

int clear_log( )
{
	int fd;

	fd = open(LOGPATH, O_TRUNC | O_WRONLY, NULL);
	if(fd == -1)
	{
		fprintf(stderr, "clear log failed %s...\n", strerror(errno));
		return -1;
	}

	close_log(fd);
	return 0;
}

int close_log(int fd)
{
	return close(fd);
}

