#include "bastrans.h"

varsrch(fp,s)
int fp;
char s[CLEN];
{
	char var[20],t[10];
	int got_one;
	register i,q;
	int j;
	for (got_one=i=j=q=0;*(s+i) != '\0';i++) {
		if (*(s+i)=='"')
			if (++q == 2) q=0;
		if (!q) {
			if (issupper(*(s+i)) || (j && (*(s+i)=='\$' || issdigit(*(s+i))))) {
				*(var+j) = *(s+i);
				*(t+j) = lower(*(s+i));
				got_one++;
				j++;
			}
			else got_one=0;
			if (j && !got_one) {
				*(var+j) = *(t+j) = '\0';
				replace(t,"\$","str",0,1);
				fprintf(fp,"%s\n",t);
				replace(s,var,t,0,1);
				j=0;
			}
		}
	}
}
