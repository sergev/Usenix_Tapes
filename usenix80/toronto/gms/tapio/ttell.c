/*
 * This file contains the source for ttell.
 * compile: cc -c -v -O ttell.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * Ttell returns the current position (in bytes) on the tape for the
 * _tapio buffer passed as its argument.  It returns ERR on an error.
 */

long	ttell(tp)
register TAPE	*tp; {
	if(_TPCHK) {
		return((long)ERR);
	}
	return(tp->_tappos);
}
