#
#include        "./gparam"

/*
**      This is a common routine for the row and column
**      read and write routines.
*/

int grwcom(b0,b1,b2,b3,b4,b5,ubuf,npnts,fdesc)
int ubuf[];
{
	register *rc, *urc;
	register cnt;
	int buff[520];

	rc = buff;

	*rc++ = b0;   /* LDC */
	*rc++ = b1;   /* LWM or LSM */
	*rc++ = b2;   /* LUM */
	*rc++ = b3;   /* LEA or LLA */
	*rc++ = b4;   /* LEB or LLB */
	*rc++ = b5;   /* LLA or LEA */

	urc = ubuf;

	if((b1 &  LWM) == LWM) {        /* Row or column write */
		for(cnt=0;cnt<npnts;cnt++)
			*rc++ = *urc++ & ALL12;
		if(write(fdesc, buff, cnt = 12 + npnts * 2) != cnt)
			return(-1);
	}
	else {                          /* Row or column read */
		*rc++ = SPD | RDBAK;    /* Memory select */
		*rc++ = RPD;
		if(read(fdesc, buff, cnt = 16 + (npnts * 2)) != cnt)
			return(-1);
		for(cnt=0;cnt<npnts;cnt++)
			*urc++ = *rc++;
	}
	return(0);
}
