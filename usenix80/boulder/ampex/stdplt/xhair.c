#include "stdplt.h"

int xhair(px, py, afname)
  register float *px, *py;
  char *afname;
/*
 * Turn on Tektronix crosshairs, wait for user to type single key.
 * This key is returned as the value,
 * unless it was EOT in which case -1 is returned.
 * The position of the crosshairs is read, evaluated as a point in
 * the named frame, and stored in the float variables indicated by
 * px and py.
 */
  {
	FRAME *fp;
	register float *ctip;	/* ptr to inverse tcat */
	float itcat[2][2];	/* inverse tcat */
	int x, y;		/* integer crosshair coords */
	int key;		/* key typed */
	char *fname;
	float junk;

	fname = afname;
	fp = _find(&fname);
	if (*fname)
		_err("xhair: %s not found\n", _cfname(afname));
	if (px == 0)
		px = &junk;
	if (py == 0)
		py = &junk;
	if (_inv2(fp->_tcat, ctip = itcat))
		_err("xhair: %s has non-invertible transformation\n",
			_cfname(afname));
	key = _xhair(&x, &y);
	*px  = *ctip++ *x;
	*px += *ctip++ *y;
	*py  = *ctip++ *x;
	*py += *ctip   *y;
	return(key);
  }
