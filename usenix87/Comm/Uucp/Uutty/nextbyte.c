#include "uutty.h"
/*
** Extract one byte from the port's input buffer, triggering
** a read if necessary.  If no data is available within the
** timeout limit, -1 is returned.  All other return values
** should be small positive integers, in [1,255].
*/
nextbyte()
{	int i;

	D9("nextbyte()");
loop:
	if (ibfa >= ibfz) {
		errno = 0;
		lockwait();
		D9("nextbyte:before read(%d,%06lX,%d)",dev,ibuf,IBUF);
		if ((i = read(dev,ibuf,IBUF)) <= 0) {
			D9("nextbyte: read(%d,%06lX,%d)=%d\t[errno=%d]"
				,dev,ibuf,IBUF,i,errno);
			Fail;
		}
		D8("nextbyte: after read(%d,%06lX,%d)=%d\t[errno=%d]"
			,dev,ibuf,IBUF,i,errno);
		if (debug >= 4) {
			dbgtimep = getime();
			Hexdnm(ibuf,i,"Read:");
		}
		if (echoing) {
			write(dev,ibuf,i);
		}
		ibfa = ibuf;
		ibfz = ibuf + i;
		*ibfz = '\0';
	}
	i = ASCII(*ibfa++);
	D9("nextbyte()=%02X='%c'",i,dsp(i));
	if (i == 0) Loop;		/* Don't return nulls */
	return i;
fail:
	D8("nextbyte()=-1 [FAILURE]");
	return -1;
}
