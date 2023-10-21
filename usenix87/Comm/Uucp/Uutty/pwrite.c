#include "uutty.h"
/* 
** Write a character string to the port, stopping at the first null.
** This will be done slowly iff slowfl is turned on.
*/
pwrite(msg)
	char*msg;
{	int  i, n;
	char c, *p;

	D5("pwrite(%08lX)",msg);
	n = strlen(msg);
	if (debug) {
		dbgtimep = getime();
		if (debug >= 2) Ascdnm(msg,n,"Send:");
		if (debug >= 4) Hexdnm(msg,n,"Send:");
	}
	if (slowfl) {
		D8("port_wr:slow=%d",slow);
		p = msg;
		while (c = *p++) {
			Slowly;
			D9("port_wr:before write(%d,%06lX,%d)",dev,&c,1);
			i = write(dev,&c,1);
			D9("port_wr: after write(%d,%06lX,%d)=%d",dev,&c,1,i);
			if (i <= 0) {
				if (debug) P("%s: write failed, quitting.",getime());
				die(2);
			}
		}
	} else {
		D9("port_wr:before write(%d,\"%s\",%d)",dev,msg,n);
		i = write(dev,msg,n);
		D9("port_wr: after write()=%d\t[errno=%d]",i,errno);
	}
}
