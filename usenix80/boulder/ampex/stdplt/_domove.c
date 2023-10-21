#include "stdplt.h"

_domove()
/*
 * Actually move pen to current position if necessary.
 */
  {
	register FRAME *dotp;
	register unsigned xc, yc;

	dotp = _dotp;
	xc = (long) dotp->_xcat;
	yc = (long) dotp->_ycat;
	if (_x != xc || _y != yc || (dotp->_fflags&_TXTMODE))
	  {	dotp->_fflags =& ~_TXTMODE;
		putc(_CMD|'m', stdplt);
		putw(_x = xc, stdplt);
		putw(_y = yc, stdplt);
	  }
  }
