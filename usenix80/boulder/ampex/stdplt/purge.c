#include "stdplt.h"

purge()
  {
	putc(_CMD|'p', stdplt);
	fflush(stdplt);
  }
