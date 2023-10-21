#include "stdplt.h"

where(px, py, fname)
  float *px, *py;
  char *fname;
/*
 * The current pen position of the current frame is evaluated as
 * a position in the named frame and returned
 * to the FLOAT variables indicated by px and py.
 */
  {
	register FRAME *dotp, *evalp;
	register float *ctip;		/* ptr to inverse tcat */
	float itcat[2][2];		/* inverse tcat transform */
	char *evalfname;
	float relx, rely;		/* (x,y) relative to named origin */
	float junk;

	dotp = _dotp;
	evalfname = fname;
	evalp = _find(&evalfname);
	if (*evalfname)
		_err("where: %s not found\n", _cfname(fname));
	if (px == 0) px = &junk;
	if (py == 0) py = &junk;

	if (evalp == dotp)
	  {	*px = dotp->_x;
		*py = dotp->_y;
	  }
	else if (evalp == &_root)
	  {	*px = dotp->_xcat*PELSIZ;
		*py = dotp->_ycat*PELSIZ;
	  }
	else
	  {	if (_inv2(evalp->_tcat, ctip = itcat))
			_err("where: %s has non-invertible transformation\n",
				_cfname(fname));
		relx = dotp->_xcat - evalp->_xocat;
		rely = dotp->_ycat - evalp->_yocat;
		*px  = *ctip++ * relx;
		*px += *ctip++ * rely;
		*py  = *ctip++ * relx;
		*py += *ctip++ * rely;
	  }
  }
