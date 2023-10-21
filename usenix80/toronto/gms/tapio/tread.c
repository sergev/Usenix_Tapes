/*
 * This file contains the source for tread.
 * compile: cc -c -v -O tread.c
 */

#include <tapio.h>		/* header file for the tape i/o library */

/*
 * Tread reads cnt objects of size siz (in bytes) from the tape specified
 * by the _tapio buffer tp.  There is no error indication.  The number of
 * objects actually read is returned.
 */

unsigned tread(buf,siz,cnt,tp)
register char	*buf;
unsigned siz,cnt;
register TAPE	*tp; {
	register c;
	long	i;

	if(_TPCHK) {
		return(0);
	}
	i = cnt;
	i *= siz;
	while(i) {
		c = tgetc(tp);
		if(c == ERR) {
			break;
		}
		*buf++ = c;
		i--;
	}
	i /= siz;
	return((unsigned)(cnt - i));
}
