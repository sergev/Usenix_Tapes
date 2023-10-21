/*
 * This file contains the source for tgetc.
 * compile: cc -c -v -O tgetc.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * Tgetc attempts to read the next character from the tape specified by the
 * _tapio buffer passed as its argument.  It returns an integer representing
 * the character (no sign extension) or ERR on an error.
 */

tgetc(tp)
register TAPE	*tp; {
	if(_TPCHK) {
		return(ERR);
	}
	if(tp->_tapptr >= &tp->_tapbuf[tp->_tapcnt]) {
		if(_tapread(tp) == ERR) {
			return(ERR);
		}
		tp->_tapptr = tp->_tapbuf;
	}
	tp->_tappos++;
	return(*tp->_tapptr++ & 0377);
}
