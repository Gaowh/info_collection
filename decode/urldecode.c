/*
 * 将从url中提取到的字符串提取出其中的重要信息
 * 
 * 如输入字符串：%3B%4D%8C%7D%36%20...
 *
 * 需要提取出一个字符串s[LEN]， (int)s[0] = 0x3b, (int)s[1] = 0x4d.....
 * 
*/

#include "../include/code.h"

int  urldecode(char *url, int urllen, unsigned char *decodeurl,int force){

	if(url == 0) return -1;
	
	int i;
	int len = 0;

	unsigned char ch1;
	unsigned char ch2;
	unsigned int one;

	unsigned char 	*uptr = url;	
	char *dptr = decodeurl;

	i=0;
	while( *uptr != '\0' ){
		if(*uptr == '%'){
			
			ch1 = *++uptr;
			ch2 = *++uptr;

			one = getone(ch1, ch2);

			if(one == -1){
				*dptr = '\0';
				return force? len:-1;
			}
			else	*dptr++ = one;
		}
		
		else *dptr++ = *uptr;

		uptr++;
		len += 1;
	}
		
	*dptr = '\0';
	
	#ifdef DEBUG_URLDECODE
		dptr = decodeurl;
		for(i=0; i<len ;i++){
			printf("%x ", *dptr++);
		} printf("\n");
		printf("%s\n", decodeurl);
	#endif

	return len;
	
}

/* 将两个char字符转换为一个unsigned int    返回的int放在一个char中*/
unsigned int getone(unsigned char ch1, unsigned char ch2){
	
	unsigned int one;

	if(ch1 >= '0' && ch1 <= '9'){
		one = ch1 - '0';	
	}
	else if(ch1 >= 'a' && ch1 <= 'z'){
		one = ch1 - 'a' + 10;
	}
	else if(ch1 >= 'A' && ch1 <= 'Z'){
		one = ch1 - 'A' + 10;
	}
	else  return -1;
	one *= 16;


	if(ch2 >= '0' && ch2 <= '9'){
		one += ch2 - '0';
	}
	else if(ch2 >= 'a' && ch2 <= 'z'){
		one += ch2 - 'a' + 10;
	}
	else if(ch2 >= 'A' && ch2 <= 'Z'){
		one += ch2 -'A' +10;
	}
	else return -1;
	return one;
}
