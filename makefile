vpath %.h include hash-table log kmp-mt

vpath %.c decode  task pthread-func message  hash-table log kmp-mt


obj=main.o urldecode.o  gb2312_to_utf8.o checkencode.o  base64.o task.o pthread_func.o message.o hash.o infolog.o kmp.o



info_collection: $(obj) 
	$(CC) -o info_collection $(obj) -lpthread -lrt 

main.o:main.c
	$(CC) -o main.o -D DEBUGMAIN -c main.c

pthread_func.o:pthread_func.c
	$(CC) -o pthread_func.o -c pthread-func/pthread_func.c 

#urldecode.o:urldecode.c
#	$(CC) -DDEBUG_URLDECODE -o urldecode.o -c decode/urldecode.c
message.o:message.c
	$(CC) -DDEBUGPACK -o message.o -c message/message.c


clean:
	rm -rf  info_collection $(obj)
