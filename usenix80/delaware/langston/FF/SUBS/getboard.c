#include    "../ffdef.h"
getboard(mode)
{
	if (mode == SAFE)
	    while (lock(BOARD_LOCK) == LOCK_FAIL)
		sleep(10);
	if (bfh == 0) {
	    if ((bfh = open(boardfil, 2)) == -1) {
		perror(boardfil);
		return(-1);
	    }
	} else
	    seek(bfh, 0, 0);
	read(bfh, board, sizeof board);
	return(0);
}
