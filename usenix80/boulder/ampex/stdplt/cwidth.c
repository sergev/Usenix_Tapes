#include "stdplt.h"

double cwidth(width, fname)
  double width;
  char *fname;
/*
 * Set character width for textf calls in current frame.
 * The frame named by fname provides the coordinate system
 * used to evaluate width.  The width in the current coordinate
 * system is returned.
 */
  {	register FRAME *dotp, *evalp;
	char *evalfname;

	dotp = _dotp;
	evalfname = fname;
	evalp = _find(&evalfname);
	if (*evalfname)
		_err("cwidth: %s not found\n", _cfname(fname));
	dotp->_fflags =| _DOTXT;
	dotp->_cw = width*evalp->_sxcat;
	return(dotp->_cw/dotp->_sxcat);
  }
