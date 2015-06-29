#ifndef _KMP_H
#define _KMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getnext(char *p, int *next);

char *kmp_pktmt(char *target ,int len, char *p);

char *kmp_strmt(char *target , char *p);

char *pktstr( char *pkt, int pktlen, char *str);

void pkt_getnext(char *p, int plen, int *next);

int word_cmp(char *target , char *p, int len);

#endif
