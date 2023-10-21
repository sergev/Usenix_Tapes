#include "stdplt.h"

erase()
  {
	putc(_CMD|'e', stdplt);
	fflush(stdplt);
	_dotp->_fflags =| _TXTMODE;
	_domove();		/* force position recovery */
  }
