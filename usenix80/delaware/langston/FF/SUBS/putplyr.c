#include    "../ffdef.h"
putplyr(pnum)
{
	seek(pfh, pnum * sizeof plyr, 0);
	if (write(pfh, &plyr, sizeof plyr) != sizeof plyr)
	    printf("Oops!  Error writing player %d\n", pnum);
	unlock(PLAYER_LOCK);
}
