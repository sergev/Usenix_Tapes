#include    "../ffdef.h"
putmisc()
{
	seek(mfh, 0, 0);
	if (write(mfh, &misc, sizeof misc) != sizeof misc)
	    printf("Oops!  Error writing misc file\n");
	unlock(MISC_LOCK);
}
