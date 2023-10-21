#include "config.h"
#if BSDREL < 42
#include <sys/types.h>
#include <sys/param.h>
#include "ndir.h"

/*
 * open a directory.
 */
DIR *
opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;
	char *malloc();

	if ((fd = open(name, 0)) == -1)
		return NULL;
	if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) {
		close (fd);
		return NULL;
	}
	dirp->dd_fd = fd;
	dirp->dd_nleft = 0;
	return dirp;
}



/*
 * close a directory.
 */
void
closedir(dirp)
	register DIR *dirp;
{
	close(dirp->dd_fd);
	free(dirp);
}



/*
 * read an old style directory entry and present it as a new one
 */
#define	ODIRSIZ	14

struct	olddirect {
	ino_t	od_ino;
	char	od_name[ODIRSIZ];
};

/*
 * get next entry in a directory.
 */
struct direct *
readdir(dirp)
	register DIR *dirp;
{
	register struct olddirect *dp;
	static struct direct dir;

	for (;;) {
		if ((dirp->dd_nleft -= sizeof(struct olddirect)) < 0) {
			dirp->dd_nleft = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
			if (dirp->dd_nleft <= 0)
				return NULL;
			dirp->dd_nextc = dirp->dd_buf;
		}
		dp = (struct olddirect *) dirp->dd_nextc;
		dirp->dd_nextc += sizeof(struct olddirect);
		if (dp->od_ino == 0)
			continue;
		dir.d_ino = dp->od_ino;
		strncpy(dir.d_name, dp->od_name, ODIRSIZ + 1);
		dir.d_namlen = strlen(dir.d_name);
		return (&dir);
	}
}
#endif
