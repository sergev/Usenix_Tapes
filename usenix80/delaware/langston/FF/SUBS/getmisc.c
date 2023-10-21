#include    "../ffdef.h"
getmisc(mode)
{
	if (mode == SAFE)
	    while (lock(MISC_LOCK) == LOCK_FAIL)
		sleep(10);
	if (mfh == 0) {
	    if ((mfh = open(miscfil, 2)) == -1) {
		perror(miscfil);
		return(-1);
	    }
	} else
	    seek(mfh, 0, 0);
	read(mfh, &misc, sizeof misc);
	return(0);
}
