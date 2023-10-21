#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <ndir.h>

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
	struct stat sbuf;

	if ((fd = open(name, 0)) == -1)
		return NULL;
	fstat(fd, &sbuf);
	if (((sbuf.st_mode & S_IFDIR) == 0) ||
	    ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL)) {
		close (fd);
		return NULL;
	}
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return dirp;
}
