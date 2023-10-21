#include "stdplt.h"

_move(x, y)
  double x, y;
/*
 * Move pen to (x,y) in current coordinate frame.
 */
  {
	register FRAME *dotp;
	register float *ctp;

	dotp = _dotp;
	ctp = dotp->_tcat;
	dotp->_x = x;
	dotp->_y = y;
	dotp->_xcat = _T(ctp, x, y) + dotp->_xocat;
	dotp->_ycat = _T(ctp, x, y) + dotp->_yocat;
	dotp->_dphase = 0.;
	dotp->_fflags =& ~_TXTMODE;
  }
