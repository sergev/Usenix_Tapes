/*
 *
 *				N O T I C E
 *
 * This file is NOT a copyrighted part of the UNaXcess distribution.  These
 * are directory-reading routines which are compatible with the Berkeley Unix
 * (4.2BSD, 4.3BSD) directory routines.  They come from the Usenet news
 * distribution and are in the public domain.
 *
 * To get the best use of them:  install the file "dir.h" in /usr/include
 * -- standard usage calls it "ndir.h", and make a random archive of dir.o and
 * put it in /usr/lib/libndir.a .  It is then available with "-lndir".
 *
 * Bell System {III, V} sites, just make an archive -- it is only one file
 * anyway.  Other sites will have to run ranlib on the archive to keep ld
 * happy.
 */

#include <sys/types.h>
#include "dir.h"

#ifndef BSD

extern char *malloc();

/*
 * close a directory.
 */
closedir(dirp)
        register DIR *dirp;
{
        close(dirp->dd_fd);
        dirp->dd_fd = -1;
        dirp->dd_loc = 0;
        free(dirp);
}



/*
 * open a directory.
 */
DIR *
opendir(name)
        char *name;
{
        register DIR *dirp;
        register int fd;

        if ((fd = open(name, 0)) == -1)
                return NULL;
        if ((dirp = (DIR *)malloc(sizeof(DIR))) == NULL) {
                close (fd);
                return NULL;
        }
        dirp->dd_fd = fd;
        dirp->dd_loc = 0;
        return dirp;
}



/*
 * read an old style directory entry and present it as a new one
 */
#define ODIRSIZ 14

struct  olddirect {
        ino_t   od_ino;
        char    od_name[ODIRSIZ];
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
                if (dirp->dd_loc == 0) {
                        dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
                            DIRBLKSIZ);
                        if (dirp->dd_size <= 0)
                                return NULL;
                }
                if (dirp->dd_loc >= dirp->dd_size) {
                        dirp->dd_loc = 0;
                        continue;
                }
                dp = (struct olddirect *)(dirp->dd_buf + dirp->dd_loc);
                dirp->dd_loc += sizeof(struct olddirect);
                if (dp->od_ino == 0)
                        continue;
                dir.d_ino = dp->od_ino;
                strncpy(dir.d_name, dp->od_name, ODIRSIZ);
                dir.d_name[ODIRSIZ] = '\0'; /* insure null termination */
                dir.d_namlen = strlen(dir.d_name);
                dir.d_reclen = DIRBLKSIZ;
                return (&dir);
        }
}

#endif BSD
