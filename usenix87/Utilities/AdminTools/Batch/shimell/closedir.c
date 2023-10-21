#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

#include <sys/types.h>
#include <ndir.h>

/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	dirp->dd_fd = -1;
	dirp->dd_loc = 0;
	free(dirp);
}
