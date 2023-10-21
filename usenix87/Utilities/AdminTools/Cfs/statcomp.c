/*
 *  Compare two stat structures
 */

#include "cfs.h"

statcomp (old, new)
struct stat *old;
struct stat *new;
{
    int     errflg = 0;
    register u_short mode;

    errflg += (old -> st_ino	!= new -> st_ino);
    errflg += (old -> st_mode	!= new -> st_mode);
    errflg += (old -> st_nlink	!= new -> st_nlink);
    errflg += (old -> st_uid	!= new -> st_uid);
    errflg += (old -> st_gid	!= new -> st_gid);
    errflg += (old -> st_ctime	!= new -> st_ctime);

    mode = old -> st_mode & S_IFMT;
    if (mode == S_IFCHR || mode == S_IFBLK) {
	errflg += (old -> st_rdev	!= new -> st_rdev);
    }
    else {
	errflg += (old -> st_size	!= new -> st_size);
	errflg += (old -> st_mtime	!= new -> st_mtime);
    }

    return (errflg);
}
