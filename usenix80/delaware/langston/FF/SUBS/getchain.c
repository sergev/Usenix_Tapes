#include    "../ffdef.h"
getchain(cnum, mode)
{
	if (cnum > LIM_CHAINS)
	    return(-1);
	if (mode == SAFE)
	    while (lock(CHAIN_LOCK) == LOCK_FAIL)
		sleep(10);
	if (cfh == 0) {
	    if ((cfh = open(chainfil, 2)) == -1) {
		perror(chainfil);
		return(-1);
	    }
	}
	seek(cfh, cnum * sizeof chain, 0);
	if (read(cfh, &chain, sizeof chain) != sizeof chain)
	    return(-1);
}
