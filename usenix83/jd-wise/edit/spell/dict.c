#include "dict.h"

int xflag;

dict(tabno,bp,ep)
char *bp, *ep;
	{
	register char *wp;
	long h;
	register long *lp;
	long *p=P[tabno];
	register i;
	if(xflag)
		printf("=%.*s\n",ep-bp,bp);
	for(i=0; i<NP; i++) {
		for (wp = bp, h = 0, lp = POW2[tabno][i]; wp < ep; ++wp, ++lp)
			h += *wp * *lp;
		h += '\n' * *lp;
		h %= p[i];
		if(get(tab[tabno],h)==0)
			return(0);
		}
	return(1);
	}

accept(word)
	char *word;
	{
	register i,j;
	register long *lp;
	register char *wp;
	long h;
	long *p=P[0];

	for (i=0; i<NP; i++) {
		for (wp = word, h = 0, lp = POW2[0][i];
			 (j = *wp) != '\0'; ++wp, ++lp)
			h += j * *lp;
		h += '\n' * *lp;
		h %= p[i];
		set(dicttab,h);
		}
	}
