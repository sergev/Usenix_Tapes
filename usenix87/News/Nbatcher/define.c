/*
 *	define.c - global defines for nbatcher.
 *
 *	R.J. Esposito
 *	Bell of Penna.
 *	June 1986
 *
 */

#include <stdio.h>
#include "nbatcher.h"

FILE	*lfp,
	*tfp,
	*log = NULL;

long	n_bytes,
	cu_bytes;

char	*tfile = NULL;

short	vflg = 0,
	nfiles = 10;

int	fcnt = 0,
	scnt = 0;

