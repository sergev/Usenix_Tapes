#include "stdplt.h"

_cat(dotp)
  register FRAME *dotp;
/*
 * Concatenate all transform data which is relative to parent
 * with parent's concatenated version.
 */
  {
	extern double fmod();
/*
 * Concatenate the 2x2 transform matrix.
 * _tcat = ..->_tcat * _t .
 */
    {	register float *tp, *ctp;

	ctp = dotp->_tcat;
	tp = dotp->_parent->_tcat;

	*ctp    = *tp++ * dotp->_t[0][0];
	*ctp++ += *tp   * dotp->_t[1][0];
	tp--;
	*ctp    = *tp++ * dotp->_t[0][1];
	*ctp++ += *tp++ * dotp->_t[1][1];
	*ctp    = *tp++ * dotp->_t[0][0];
	*ctp++ += *tp   * dotp->_t[1][0];
	tp--;
	*ctp    = *tp++ * dotp->_t[0][1];
	*ctp   += *tp   * dotp->_t[1][1];
    }
/*
 * Concatenate the scale and rotation.
 */
    {	register FRAME *dotdotp;
	register float *ctp;

	dotdotp = dotp->_parent;
	dotp->_sxcat = dotdotp->_sxcat*dotp->_sx;
	dotp->_sycat = dotdotp->_sycat*dotp->_sy;
	dotp->_rotcat = fmod(dotdotp->_rotcat + dotp->_rot, 2.*PI);
/*
 * Concatenate the origin.
 */
	ctp = dotdotp->_tcat;
	dotp->_xocat = _T(ctp, dotp->_xo, dotp->_yo) + dotdotp->_xocat;
	dotp->_yocat = _T(ctp, dotp->_xo, dotp->_yo) + dotdotp->_yocat;
    }
/*
 * Concatenate the current pen location.
 */
    {	register float *ctp;

	ctp = dotp->_tcat;
	dotp->_xcat = _T(ctp, dotp->_x, dotp->_y) + dotp->_xocat;
	dotp->_ycat = _T(ctp, dotp->_x, dotp->_y) + dotp->_yocat;
    }
/*
 * Mark child concatenations invalid.
 */
	dotp->_fflags =& ~_DOCAT;
	dotp->_fflags =| _DOTXT;
	for (dotp = dotp->_child; dotp; dotp = dotp->_sibling)
		dotp->_fflags =| (_DOCAT|_DOTXT);
  }
