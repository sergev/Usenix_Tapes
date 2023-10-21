#include "errnos.h"

char xwrt_____[] "~|^`xwrite.c	1.1";

/*
 * Interface to write(II) which handles all error conditions.
 */
xwrite(fildes,buffer,nbytes)
char *buffer;
{
	extern int errno;
	register int n;

	if (nbytes>0 && (n=write(fildes,buffer,nbytes))!=nbytes)
		if (errno==EFBIG) fatal("write error (350)");
		else if (errno==ENOSPC) fatal("no space! (351)");
		else fatal(stringf("write error %d (352)",errno));
}
