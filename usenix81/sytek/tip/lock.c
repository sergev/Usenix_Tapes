/*	lock.c	4.1	81/05/09	*/
#include "tip.h"

#ifdef ACULOG
/*
 * Locking mechanism for files
 */

static int fd = -1;

lock(f)
char *f;
{
	int timeout = 0;

	while ((fd = creat(f, 0444)) < 0) {
		if (++timeout == 6)
			break;
		sleep(5);
	}
	if (timeout == 6 && fd >= 0)
		unlock(); 
	else
		unlink(f);
	return(fd >= 0);
}

unlock()
{
	if (fd != -1)
		close(fd);
	fd = -1;
}
#endif
