#include "stdplt.h"

char *_fmatch(fname, fpath)
  register char *fname, *fpath;
/*
 * If fname matches the front of fpath, return a ptr to
 * the unmatched part of fpath.  If no match, return 0.
 * The '/' separating names is stripped off the front of the
 * unmatched part of fpath.
 */
  {
	while (*fname && *fname++ == *fpath++) ;
	if (*--fname != *--fpath)
		return(0);
	return(*++fpath? (*fpath == '/'? fpath+1: 0): fpath);
  }
