#include "stdplt.h"

double xdist(dx, fname)
  double dx;
  char *fname;
/*
 * The distance dx is evaluated along the x-axis in the coordinate
 * system named by fname.  The equivalent distance along the x-axis
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
		_err("xdist: %s not found\n", _cfname(fname));
	return(dx*evalp->_sxcat/dotp->_sxcat);
  }
