#include <stdio.h>

cred_(mdevin,jbuf,ls)

long ls;
char *jbuf;
long *mdevin;

{
	int fin, nc;

	fin = *mdevin;

	nc = read(fin,jbuf,28);

	for(nc--; nc<ls; nc++)
		jbuf[nc] = ' ';
	jbuf[ls-1] = '\n';
	return(0);
}
