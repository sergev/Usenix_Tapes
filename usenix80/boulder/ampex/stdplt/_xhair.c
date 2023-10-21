#include "stdplt.h"

#define TEKSCL	21	/* STDSIZ/781 */

int _xhair(ax, ay)
  int *ax, *ay;
/*
 * Internal routine to read Tektronix 4012 crosshair position.
 * Turns on crosshairs, waits for user to type a single command
 * key, at which time the crosshair is read and turned off.
 * The return value is the key typed, and the coordinates are
 * returned in standard pixels (0 to 1023*TEKSCL).
 * If either the EOT key is typed, a -1 is returned.
 * The Tektronix strap options must be set to terminate the GIN-mode
 * sequence with the CR code alone (this allows cooked tty to be used).
 */
  {	register int i, n;
	int c;
	char ginbuf[256];

	do
	  {
		putc(_CMD|'x', stdplt);
		fflush(stdplt);
		n = read(2, ginbuf, sizeof ginbuf);
		if (n <= 0)
			break;
		if (n != 6)
			continue;
		for (i = 1; i <= 5; i++)	/* actual char, 4 addrs, \n */
		  {
			c = ginbuf[i]&0177;
			if (i < 5 && 040 <= c && c <= 077)	/* valid byte */
				continue;
			if (i == 5 && c == '\n') /* end of correct sequence */
				continue;
			n = 0;		/* bad sequence; restart */
			break;
		  }
	  } while (n != 6);
	*ax = (((ginbuf[1]&037)<<5) | (ginbuf[2]&037))*TEKSCL;
	*ay = (((ginbuf[3]&037)<<5) | (ginbuf[4]&037))*TEKSCL;
	return((n == 6)? ginbuf[0]&0177: EOF);
  }
