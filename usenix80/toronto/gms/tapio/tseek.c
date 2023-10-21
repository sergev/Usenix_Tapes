/*
 * This file contains the source for tseek.
 * compile: cc -c -v -O tseek.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * Tseek attempts to seek to the position offset (in bytes) on the tape
 * for the _tapio buffer passed as its argument.  It returns OK if
 * successful or ERR on an error.  Note that only forward seeks are allowed.
 */

tseek(tp,offset)
register TAPE	*tp;
long	offset; {
	if(_TPCHK) {
		return(ERR);
	}
	if(offset < tp->_tappos) {
		return(ERR);
	}
	while(offset > tp->_tappos) {
		if(tp->_tapptr >= &tp->_tapbuf[tp->_tapcnt]) {
			break;
		}
		if(tgetc(tp) == ERR) {
			return(ERR);
		}
	}
	if(offset == tp->_tappos) {
		return(OK);
	}
	if(_tapread(tp) == ERR) {
		return(ERR);
	}
	while(offset > (tp->_tappos + tp->_tapcnt)) {
		tp->_tappos += tp->_tapcnt;
		if(_tapread(tp) == ERR) {
			return(ERR);
		}
	}
	while(offset > tp->_tappos) {
		if(tgetc(tp) == ERR) {
			return(ERR);
		}
	}
	return(OK);
}
