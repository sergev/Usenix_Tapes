#include "defs.h"
#include "libextern.h"


char *
dirname(ngname, rbuf)
char *ngname;
char rbuf[FPATHLEN];
{
	register char *p;

	sprintf(rbuf, "%s/%s", SPOOL, ngname);
	for (p=rbuf+strlen(SPOOL); *p; p++)
		if (*p == '.')
			*p = '/';
	return rbuf;
}
