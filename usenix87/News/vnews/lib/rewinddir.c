#include "config.h"
#if BSDREL < 42
#include <sys/types.h>
#include <sys/param.h>
#include "ndir.h"

/*
 * Go back to the beginning of a directory.
 */

rewinddir(dirp)
	DIR *dirp;
	{
	dirp->dd_nleft = 0;
	lseek(dirp->dd_fd, 0L, 0);
}
#endif
