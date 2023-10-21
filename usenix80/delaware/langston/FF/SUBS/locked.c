#include    "../ffdef.h"
locked(n)              /* test whether someone else has lock; don't set it */
{
	int sbuf[18];

	if (locks[n] == LOCK_OK)
	    return(0);                                       /* we have it */
	if (stat(lockfile[n], sbuf) == -1)
	    return(0);                               /* lock doesn't exist */
	return(1);                                  /* someone else has it */
}




