#include "kmp.h"


int main()
{
	char p[] = {"aaaaa"};
	char str[] = {"xaaaaeafgaaaaaafkjagfg"};
	
	char * ret = kmp_strmt(str, p );
	if(ret != NULL){
		printf("Matched ....\n");
	}
	else 
		printf("Not matched...\n");
	return 0;
}
