#include "stdplt.h"

_origin(xo, yo)
  double xo, yo;
/*
 * Set origin (0,0) of current frame to (xo, yo) where xo and yo
 * are evaluated in the current coordinate frame.
 */
  {
	register FRAME *dotp, *dotdotp;
	register float *tp;

	if ((dotp = _dotp) == &_root)
		_err("origin: not valid in root frame\n");
/*
 * Fix pen location.
 */
	dotp->_x -= xo;
	dotp->_y -= yo;
/*
 * Translate origin to parent's frame.
 */
	tp = dotp->_t;
	dotp->_xo += *tp++ * xo;
	dotp->_xo += *tp++ * yo;
	dotp->_yo += *tp++ * xo;
	dotp->_yo += *tp++ * yo;
/*
 * Update concatenated origin.
 */
	dotdotp = dotp->_parent;
	tp = dotdotp->_tcat;
	dotp->_xocat = _T(tp, dotp->_xo, dotp->_yo) + dotdotp->_xocat;
	dotp->_yocat = _T(tp, dotp->_xo, dotp->_yo) + dotdotp->_yocat;
/*
 * Mark child concatenated origins invalid.
 */
	for (dotp = dotp->_child; dotp; dotp = dotp->_sibling)
		dotp->_fflags |= _DOCAT;
  }
