#include "stdplt.h"

/*
 * Set window for named frame in "." coords.  If the pointer is set to NULL,
 * set global window in "/" coords.
 */
_window(xmin, xmax, ymin, ymax, fp)
  double xmin, xmax, ymin, ymax;
  register FRAME *fp;
  {
	if (fp == NULL)
	  {	if (xmin < 0. || xmax > 4. || xmin >= xmax)
			fprintf(stderr,"window: bad x-boundary\n");
		else
		  {	_rwind[0] = xmin * STDSIZ;
			_rwind[1] = xmax * STDSIZ;
		  }
		if (ymin < 0. || ymax > 4. || ymin >= ymax)
			fprintf(stderr,"window: bad y-boundary\n");
		else
		  {	_rwind[2] = ymin * STDSIZ;
			_rwind[3] = ymax * STDSIZ;
		  }
	  }
	else
	  {	if (xmin > xmax)
			fprintf(stderr,"_window: xmin > xmax\n");
		else if (ymin > ymax)
			fprintf(stderr,"_window: ymin > ymax\n");
		else
		  {	fp->_fflags |= _DOWIND;
			fp->_wind[0] = xmin;
			fp->_wind[1] = xmax;
			fp->_wind[2] = ymin;
			fp->_wind[3] = ymax;
		  }
	  }
  }

_rmwindow(fp)
  register FRAME *fp;
  {
	if (fp)
		fp->_fflags &= ~_DOWIND;
  }
