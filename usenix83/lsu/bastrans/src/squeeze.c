
#include "bastrans.h"

squeeze(s,c,n,qt)
char s[CLEN];
int c,n,qt;
{
	register i,j,k;
	int q;
	for (q=k=i=j=0;*(s+i)!='\0';i++) {
		if (*(s+i) == '\"') q++;
		if (*(s+i) != c || (qt && q) || (n && k>=n))
			*(s+j++) = *(s+i);
		else k++;
		if (q==2) q=0;
	}
	*(s+j)='\0';
}
