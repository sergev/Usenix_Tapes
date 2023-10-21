#include    "../ffdef.h"
unlock(n)
{
	if (locks[n] == LOCK_OK) {
	    if (unlink(lockfile[n]) == -1) {
		printf("Unlock failed -- ");
		perror(lockfile[n]);
	    } else
		locks[n] = NOT_LOCKED;
	}
	return(locks[n]);
}
