#include "stdplt.h"

_err(fmt, args)
  char *fmt;
/*
 * Simulate printf on stderr, followed by abort().
 */
  {
	fflush(stdplt);
	fputs(stderr, "\n");		/* force Tektronix to ascii */
	_doprnt(fmt, &args, stderr);
	_cleanup();
	abort();			/* cause core dump */
	_exit(0177);
  }
_warn(fmt, args)
  char *fmt;
/*
 * Simulate printf on stderr.
 */
  {
	fflush(stdplt);
	fputs(stderr, "\n");		/* force Tektronix to ascii */
	_doprnt(fmt, &args, stderr);
	_cleanup();
  }
