#include "stdplt.h"

double cos(), sin(), fmod();

_rotate(theta)
  double theta;
/*
 * Rotate frame counter-clockwise theta radians about origin.
 */
  {
	register FRAME *dotp;
	register float *tp, *ctp;
	float c, s, tmp;
	int i;

	if ((dotp = _dotp) == &_root)
		_err("rotate: not valid in root frame\n");
	theta = fmod(theta, 2.0*PI);
	dotp->_rot = fmod(dotp->_rot + theta, 2.0*PI);
	c = cos(theta);
	s = sin(theta);
/*
 * Update relative transform (put in _tcat momentarily).
 */
	tp = dotp->_t;
	ctp = dotp->_tcat;
	*ctp++ = *tp++ * c;
	*ctp++ = *tp++ * c;
	*ctp++ = *tp++ * c;
	*ctp++ = *tp   * c;

	tp = &dotp->_t[0][1];
	ctp = dotp->_tcat;
	*ctp++ +=   *tp * s;
	*ctp++ -= *--tp * s;
	tp = &(dotp->_t[1][1]);
	*ctp++ +=   *tp * s;
	*ctp++ -= *--tp * s;
/*
 * Move new transform back to _t.
 */
	for (tp = dotp->_t, ctp = dotp->_tcat, i = 4; i--; )
		*tp++ = *ctp++;
/*
 * Fix pen location (de-rotate it).
 */
	tmp = c * dotp->_x + s * dotp->_y;
	dotp->_y = c * dotp->_y - s * dotp->_x;
	dotp->_x = tmp;

	_cat(dotp);
  }
