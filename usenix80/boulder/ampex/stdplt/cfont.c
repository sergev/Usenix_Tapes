#include "stdplt.h"

char
cfont(font)
  char font;
/*
 * Set character font for textf calls in current frame.
 * The previous font is returned.
 */
  {	register FRAME *dotp;
	char oldfont;

	dotp = _dotp;
	dotp->_fflags =| _DOTXT;
	oldfont = dotp->_font;
	dotp->_font = font;
	return(oldfont);
  }
