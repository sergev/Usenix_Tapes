/*
 * printstat - print name and stat structure 
 * 	       output similar to 'ls -l'
 */

#include "cfs.h"

printstat(name, sbuf)
char           *name;
struct stat    *sbuf;
{
	register char  *cmode;
	register char  *ctp;
	char           *mkmode();
	char           *ctime();



	cmode = mkmode(sbuf -> st_mode);
	printf("%6d %10s %2d %3d/%-3d ",
	       sbuf -> st_ino, cmode, sbuf -> st_nlink, sbuf -> st_uid,
	       sbuf -> st_gid);

	if (*cmode == 'c' || *cmode == 'b')
	       printf("%2d, %2d ", major(sbuf->st_rdev), minor(sbuf->st_rdev));
	else
	       printf("%6d ", sbuf->st_size);

	ctp = ctime (&(sbuf -> st_mtime));
	printf("%12.12s %4.4s ", ctp + 4, ctp + 20);

	ctp = ctime (&(sbuf -> st_ctime));
	printf("%12.12s %4.4s ", ctp + 4, ctp + 20);
	printf("%s\n", name);
}
