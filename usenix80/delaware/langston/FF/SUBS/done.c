#include    "../ffdef.h"
/*
**      DONE -- Release any locks and exit
** Compile: cc -O -c -q done.c
*/

done(stat)            /* note unlock() will only unlink if we set the lock */
{
	register int i;

	for (i = FIRST_LOCK; i <= LAST_LOCK; i++)
	    unlock(i);                        /* avoid deadly embraces */
        exit(stat);
}
