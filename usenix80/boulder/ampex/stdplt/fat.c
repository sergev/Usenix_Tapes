#include "stdplt.h"

fat(extra)
  int extra;
/*
 * Set line thickness to 1 + 'extra' line widths.
 */
  {	register FRAME *dotp;

	if ((dotp = _dotp) == &_root)
		_err("fat: not valid in root\n");
	dotp->_fat = extra;
  }
