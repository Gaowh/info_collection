#include "../include/code.h"
#include "../include/gb2312_to_unicode.h"
#include "../include/task.h"


void gb_to_unicode()
{
	//这个函数将生成一个二维数组
	//数组中的元素是unicode编码
	//下标可以同过gb2312编码得到
	//所以这个数组是用来进行gb2312 --->   unicode 转换的码表
	int j;

	unsigned short tmp;
	unsigned int index1;
	unsigned int index2;

	for(j=0;j<7445;j++){
		//取gb2312编码， 减去0xa0a0便得到数组的下标
		tmp = gb2312_to_unicode[j][1]-0xa0a0;
	
		index1 = tmp>>8;
		index2 = tmp&0xff;

		if(!is_bigendian()){
	
			index1 = tmp&0xff;
			index2 = tmp>>8;
			
		}


		gb_2_uni[index1][index2] = gb2312_to_unicode[j][0];
	}/*end of coding table*/
}


//判断当前使用的是打断还是小端系统
int is_bigendian(){

	union{
		
		short s;
		char c[sizeof(short)];
	}un;
	un.s = 0x0102;
	
	if(un.c[0] == 1 && un.c[1] == 2) return 1; //big_endian
	if(un.c[0] == 2 && un.c[1] == 1) return 0; //little_endian

	return -1;

}


//gb2312 到utf8 的转码函数   当前的函数支持1个字节和两个字节的utf8转换  如有必要可以扩充
int gb2312_to_utf8(unsigned char *gb2312string, int gb2312string_len, unsigned char *utf8string)
{
	
	if(! isgb2312encode(gb2312string, gb2312string_len)){
		
		//判断gb2312编码是否有效
		WRONG_MESSAGE("Invalid Gb2312 encode");
		return -1;
	
	}

	int 	i;
	unsigned int 	index1,index2;
	
	wchar	tmp;
	wchar 	*p;
	
	uint_8	*gb2312_char;
	uint_8 *utf8_char;
	int	utf8string_len = 0;

	wchar 	*twochar;

	gb2312_char  =(uint_8*)gb2312string;
	utf8_char = (uint_8 *)utf8string;

	i = 0;
	while(i<gb2312string_len){


		if(*gb2312_char >= 0x0 && *gb2312_char <= 0x7f){
			
			//case   当汉字中插有英文字符是，字符一般不编码，直接就是ASCII码
			//而utf8中英文字符也直接用ASCII码表示就可以了
			//所以这种情况直接存放就行
			*utf8_char = *gb2312_char;
			utf8_char++;
			utf8string_len++;
			gb2312_char++;
			i++;
		}
		else{
			twochar = (wchar *)gb2312_char;
			
				

			tmp = *twochar - 0xa0a0;
			index1 = tmp >> 8;
			index2 = tmp & 0xff;

			tmp = gb_2_uni[index1][index2];

			// case 110xxxxx 10xxxxxx
			if (tmp >= 0x0 && tmp <= 0x7ff){
				
				*(utf8_char) = (tmp >> 6 & 0x1f) | 0xc0;
				*(utf8_char +1) = (tmp & 0x3f) | 0x80;
				
				utf8_char += 2;
				utf8string_len +=2;
				gb2312_char += 2;
			}

			//case 110xxxxx 10xxxxxx 10xxxxxx
			else if (tmp >= 0x800 && tmp <= 0xffff){
				
				*(utf8_char) = ((tmp >> 12) & 0x0f) | 0xe0;
			
				*(utf8_char+2) = (tmp & 0x3f) | 0x80;
			
				*(utf8_char+1) = ((tmp >> 6) & 0x3f) |0x80;
			
			
							
				#ifdef DEBUG_ENCODE
			
				printf("%x\n",*twochar);
				printf("index1: %d index2: %d\n",index1,index2);
				printf("unicode: %x\n", gb_2_uni[index1][index2]);
				printf("%x ",*utf8_char);
				printf("%x ",*(utf8_char+2));
				printf("%x\n\n",*(utf8_char+1));
			
				#endif
			
			
				utf8_char += 3;
				utf8string_len += 3;
				gb2312_char += 2;
				i +=2;
			}
			
			//还有些范围在utf8编码内，但是已经不是汉字的编码范围，所以不予考虑
			else  {
				WRONG_MESSAGE("Invalid encode");
				return -1;
			}
		}
	}
	if(!isutf8encode(utf8string, utf8string_len));
	{
		WRONG_MESSAGE("Invalid utf8 encode");
		return -1;
	}
	return utf8string_len;
}
