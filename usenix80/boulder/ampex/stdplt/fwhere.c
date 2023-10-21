#include "stdplt.h"

fwhere(px, py, fname)
  float *px, *py;
  char *fname;
/*
 * The current pen position of the named frame is evaluated as
 * a position in the current frame and returned
 * to the FLOAT variables indicated by px and py.
 */
  {
	register FRAME *dotp, *posp;
	register float *ctip;		/* ptr to inverse tcat */
	float itcat[2][2];		/* inverse tcat transform */
	char *posfname;
	float relx, rely;		/* (x,y) relative to named origin */
	float junk;

	dotp = _dotp;
	posfname = fname;
	posp = _find(&posfname);
	if (*posfname)
		_err("fwhere: %s not found\n", _cfname(fname));
	if (px == 0) px = &junk;
	if (py == 0) py = &junk;

	if (posp == dotp)
	  {	*px = dotp->_x;
		*py = dotp->_y;
	  }
	else
	  {	if (_inv2(dotp->_tcat, ctip = itcat))
			_err("fwhere: %s has non-invertible transformation\n",
				_cfname("."));
		relx = posp->_xcat - dotp->_xocat;
		rely = posp->_ycat - dotp->_yocat;
		*px  = *ctip++ * relx;
		*px += *ctip++ * rely;
		*py  = *ctip++ * relx;
		*py += *ctip++ * rely;
	  }
  }
