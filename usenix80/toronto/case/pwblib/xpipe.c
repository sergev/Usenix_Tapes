#include "errnos.h"

char xpipe__[] "~|^`xpipe.c 1.1";

xpipe(t)
int *t;
{
	extern int errno;

	if (pipe(t)==0) return;
	fatal(stringf("pipe error %d",errno));
}

