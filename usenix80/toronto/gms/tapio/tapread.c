/*
 * This file contains the source for _tapread.
 * compile: cc -c -v -O tapread.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * _tapread attempts to read the next record from the specified tape into
 * that tape's input buffer.  Its argument is a pointer to the _tapio
 * structure for the tape to be read.  It returns the number of bytes
 * actually read or ERR on an error.
 */

unsigned _tapread(tp)
register TAPE	*tp; {
	tp->_tapptr = tp->_tapbuf;
	tp->_tapcnt = read(tp->_tapfd,tp->_tapbuf,tp->_taprec);
	if((tp->_tapcnt == 0) || (tp->_tapcnt == ERR)) {
		tp->_tapcnt = 0;
		return(ERR);
	}
	return(tp->_tapcnt);
}
