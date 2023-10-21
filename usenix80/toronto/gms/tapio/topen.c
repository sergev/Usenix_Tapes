/*
 * This file contains the source for topen.
 * compile: cc -c -v -O topen.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * Topen attempts to open the file tapdev (presumably a raw magtape device)
 * for reading.  The second argument specifies the maximum size of a record
 * on that tape and is used to set the size of the input buffer.  It returns
 * a pointer to the _tapio structure for the tape or NULL on an error.
 */

TAPE	*topen(tapdev,recsiz)
register char	*tapdev;
register unsigned recsiz; {
	register TAPE	*tp;

	for(tp = &_tapio[0];tp < &_tapio[_NTAPES];tp++) {
		if(tp->_tapbuf == NULL) {
			break;
		}
	}
	if(tp == &_tapio[_NTAPES]) {
		return(NULL);
	}
	tp->_tapbuf = calloc(recsiz,1);
	if(tp->_tapbuf == NULL) {
		return(NULL);
	}
	tp->_tapfd = open(tapdev,0);
	if(tp->_tapfd == ERR) {
		cfree(tp->_tapbuf);
		tp->_tapbuf = NULL;
		return(NULL);
	}
	tp->_taprec = recsiz;
	tp->_tappos = 0L;
	tp->_tapcnt = 0;
	tp->_tapptr = tp->_tapbuf;
	return(tp);
}
