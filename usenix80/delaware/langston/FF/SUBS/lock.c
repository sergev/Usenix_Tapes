#include    "../ffdef.h"
lock(n)
{
	if (locks[n] != LOCK_OK)
	    if (link(locknode, lockfile[n]) != -1)
		locks[n] = LOCK_OK;
	return(locks[n]);
}
