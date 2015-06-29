#include "kmp.h"

void getnext(char *p, int *next)
{
	int n =strlen(p);
	int i,j;
	next[0]  = 0;

	for(j=1; j<n; j++){
		
		i = next[j-1];
		while(p[j] != p[i] && i!=0){
			i = next[i-1];
		}

		if(p[j] == p[i])
			next[j] = i+1;
		else
			next[j] = 0;
	}

	for(j=n-1; j>0; j--)
		next[j] = next[j-1];
}

void pkt_getnext(char *p, int plen, int *next)
{
	int n = plen;
	int i,j;
	next[0]  = 0;

	for(j=1; j<n; j++){
		
		i = next[j-1];
		while(p[j] != p[i] && i!=0){
			i = next[i-1];
		}

		if(p[j] == p[i])
			next[j] = i+1;
		else
			next[j] = 0;
	}

	for(j=n-1; j>0; j--)
		next[j] = next[j-1];
}

char *kmp_pktmt(char *target, int len, char *p)
{
	int m, n;
	m = len;
	n = strlen(p);

	int *next = (int *)malloc(sizeof(int)*n);
	getnext(p, next);

	int i,j=0;

	for(i=0; i<m; i++){
		while(p[j] != target[i] && j > 0)
			j  = next[j];
		if(p[j] == target[i])
			j += 1;
		if(j == n)
			return (char *)(target +i-n+1);
	}
	return (char*)0;
}


char *kmp_strmt(char *target, char *p)
{
	int m = strlen(target);
	char *ret = kmp_pktmt(target, m ,p);
	return ret;
}

char *pktstr(char *pkt, int pktlen ,char *str){
	
	char *ret;
	ret = kmp_pktmt(pkt, pktlen, str);
	return ret;
}

int word_cmp(char *target , char *p, int len)
{
	return memcmp(target, p ,len);
}
