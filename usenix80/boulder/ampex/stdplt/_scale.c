#include "stdplt.h"

_scale(sx, sy)
  double sx, sy;
/*
 * Scale current frame by (sx,sy) relative to parent.
 * All coordinates will by multiplied by the scaling when
 * converted to the parent's coordinates.
 */
  {
	register FRAME *dotp;
	register float *tp;

	if ((dotp = _dotp) == &_root)
		_err("scale:  not valid in root frame\n");
/*
 * Update relative transform
 */
	dotp->_sx =* sx;
	dotp->_sy =* sy;
	tp = dotp->_t;
	*tp++ *= sx;
	*tp++ *= sy;
	*tp++ *= sx;
	*tp++ *= sy;
/*
 * Fix pen location.
 */
	dotp->_x /= sx;
	dotp->_y /= sy;

	_cat(dotp);
  }
