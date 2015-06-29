#include "../include/code.h"
#include "../include/task.h"


/*
    默认的情况是utf8编码中只包含汉字和常用的字符
 
    如果判断到不是这些字符的编码的话便认为这不是一个有效的utf8编码
  
*/

/*
       code符号范围    |   UTF-8编码方式
      (十六进制)       |   （二进制）
   --------------------+---------------------------------------------
   0000 0000-0000 007F | 0xxxxxxx   (字符所在的编码范围)
   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx   (汉字所在的编码范围)			
   0001 0000-001F FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx   (utf8 6.1定义范围 0x0-0x10ffff，后面两类已不属于utf8)
   0020 0000-03FF FFFF | 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx					
   0400 0000-7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
int isutf8encode(unsigned char *utf8string, int utf8_len){

	//只处理上面表格中的前4中情况
	int 	i;
	int 	len = utf8_len;
	unsigned char *uptr = utf8string;

	i = 0;
	while(i < len){
		
		//case: 0xxxxxxx
		if(*uptr >= 0x0 && *uptr <= 0x7f)  {
			i++;
			uptr++;
		}

		//case: 110xxxxx 10xxxxxx
		else if (*uptr >> 5 == 0x6){
			
			if(*++uptr >> 6 != 0x2){
				return 0;
			}
			i+=2;
			uptr++;
		
		}

		//case: 1110xxxx   10xxxxxx 10xxxxxx  (汉字所在的区域)
		else if( *uptr >> 4 == 0xe){
		
			if( (*(++uptr) >> 6 != 0x2) || (*++uptr)>>6 != 0x2) 
			{
				return 0;
			}

			i = i+3;
			uptr++;
		}
			
		//case: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx   其实只有0x1 0000 ~  0x 10 ffff
		else if (*uptr >> 3 == 0x1e){

			if(*(++uptr) >> 6 != 0x2 || *(++uptr) >> 6 != 0x2 || *(++uptr) >> 6 !=0x2){
				return 0;
			}

			i+=4;
			uptr++;
		}
		else return 0;
	}

	return 1;
}



/*
 	通过汉字和字符的编码范围来判断该字符串是有效的gb2312编码。

*/
int isgb2312encode(unsigned char *gb2312string, int gb2312_len)
{

	int 	i;
	int 	len = gb2312_len;

	unsigned char *gptr = (unsigned char *)gb2312string;
	

	while(*gptr != '\0'){
		
		if(*gptr >= 0x0 && *gptr <= 0x7f)  {
			gptr++;
		}
		
		//这里只能通过判断每个字节的范围来判断是否是gb2312编码

		else{
			unsigned short *ptr = (unsigned short *)gptr;
			if(*ptr <= 0xa0a0 && *ptr >= 0xf7ff)
				return 0;
			else
				gptr += 2;
		}
	}
	return 1;
}

int url_to_utf8(unsigned char *keyword, unsigned char *url, int urllen)
{
	int keywordlen;
	
	bzero(keyword, urllen);
	if( isutf8encode(url, urllen)) {
		
		#ifdef DEBUG_URL_TO_UTF8
		printf("Is utf8 encode\n");
		#endif
		
		memcpy(keyword, url, urllen);
		keyword[urllen] = '\0';
		return urllen;
	}
	else if(isgb2312encode(url, urllen)){
		
		#ifdef DEBUG_URL_TO_UTF8
		printf("Is gb2312 encode\n");
		#endif

		keywordlen = gb2312_to_utf8(url, urllen, keyword);
		keyword[keywordlen] = '\0';
		return keywordlen;
	}
	else{
		WRONG_MESSAGE("Unknown encode form\n");
		keyword = NULL;
		return -1;
	}
}
