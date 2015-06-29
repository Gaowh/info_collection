#ifndef CODE_H
#define CODE_H
 
#ifndef uint_8 
typedef unsigned char  uint_8;
#endif

#ifndef uint_16 
typedef unsigned short uint_16;
#endif

#ifndef wchar
typedef uint_16 wchar;
#endif

#define MAXLEN 64

unsigned short gb_2_uni[95][95];

int base64_encode(char *gb2312string, char *base64string, int gb2312stringlen);

char getbase64value(char ch);

int base64_decode(char *gb2312string, char *base64string, int base64_len, int force);

int is_bigendian();

int gb2312_to_utf8(unsigned char *gb2312string,int gb2312string_len, unsigned char *utf8string);

int urldecode(char * url, int urllen, unsigned char *decodeurl, int force);

unsigned int getone(unsigned char ch1, unsigned char ch2);

int isutf8encode(unsigned char *utf8string, int utf8_len);

int isgb2312encode(unsigned char *gb2312string, int gb2312_len);

void gb_to_unicode();

int url_to_utf8(unsigned char *keyword, unsigned char *url, int urllen);

#endif
