/*
 * This file contains the source for tclose.
 * compile: cc -c -v -O tclose.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * Tclose attempts to close the tape and free the buffer associated with
 * the _tapio buffer passed as its argument.  It returns OK if successful
 * or ERR on an error.
 */

tclose(tp)
register TAPE	*tp; {
	if(_TPCHK) {
		return(ERR);
	}
	if(tp->_tapbuf == NULL) {
		return(ERR);
	}
	cfree(tp->_tapbuf);
	tp->_tapbuf = NULL;
	if(close(tp->_tapfd) == ERR) {
		return(ERR);
	}
	return(OK);
}
