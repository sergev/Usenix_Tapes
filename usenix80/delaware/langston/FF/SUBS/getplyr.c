#include    "../ffdef.h"
getplyr(pnum, mode)
{
	if (mode == SAFE)
	    while (lock(PLAYER_LOCK) == LOCK_FAIL)
		sleep(10);
	if (pfh == 0) {
	    if ((pfh = open(plyrfil, 2)) == -1) {
		perror(plyrfil);
		return(-1);
	    }
	}
	seek(pfh, pnum * sizeof plyr, 0);
	if (read(pfh, &plyr, sizeof plyr) != sizeof plyr)
	    return(-1);
}
