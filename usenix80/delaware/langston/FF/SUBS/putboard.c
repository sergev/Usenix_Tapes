#include    "../ffdef.h"
putboard()
{
	seek(bfh, 0, 0);
	if (write(bfh, board, sizeof board) != sizeof board)
	    printf("Oops!  Error writing board file\n");
	unlock(BOARD_LOCK);
}
