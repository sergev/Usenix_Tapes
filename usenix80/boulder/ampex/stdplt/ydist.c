#include "stdplt.h"

double ydist(dy, fname)
  double dy;
  char *fname;
/*
 * The distance dy is evaluated along the y-ayis in the coordinate
 * system named by fname.  The equivalent distance along the y-ayis
 * in the current coordinate system is returned.  Note that the
 * distances are measured independently of the rotations of the two
 * coordinate systems.
 */
  {	register FRAME *dotp, *evalp;
	char *evalfname;

	dotp = _dotp;
	evalfname = fname;
	evalp = _find(&evalfname);
	if (*evalfname)
		_err("ydist: %s not found\n", _cfname(fname));
	return(dy*evalp->_sycat/dotp->_sycat);
  }
