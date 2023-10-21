#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

#include <sys/types.h>
#include <ndir.h>

extern	long	lseek();	/* needed for pdp 11s -- ikonas!mcm */

/*
 * return a pointer into a directory
 */
long
telldir(dirp)
	DIR *dirp;
{
	return (lseek(dirp->dd_fd, 0L, 1) - dirp->dd_size + dirp->dd_loc);
}
