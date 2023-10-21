#include "stdplt.h"

double
crotate(theta, fname)
  double theta;
  char *fname;
/*
 * Set character baseline rotation for textf calls in current frame.
 * The frame named by fname provides the coordinate system
 * used to evaluate theta.  The  character rotation in the current coordinate
 * system is returned.  The  character rotation is stored in terms
 * of the current frame's x-axis rotation.
 */
  {	register FRAME *dotp, *evalp;
	char *evalfname;
	extern double fmod();

	dotp = _dotp;
	evalfname = fname;
	evalp = _find(&evalfname);
	if (*evalfname)
		_err("cwidth: %s not found\n", _cfname(fname));
	dotp->_fflags =| _DOTXT;
	dotp->_crot = fmod(theta + evalp->_rotcat - dotp->_rotcat, 2.*PI);
	return(dotp->_crot);
  }
