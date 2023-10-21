#
#include        "./gparam"

/*
**      This routine will write out one point to the display
**      at window-relative location (cnum,rnum).
*/

int gwpnt(area, value, cnum, rnum)
struct abuf *area;
{
	register struct abuf *rarea;
	register rcnum;
	register *rbuf;
	int buff[5];

	rarea = area;

	rcnum = cnum;
	if(rarea->dsize) {      /* Double size output */
		rcnum =* 2;
		rnum =* 2;
	}

	if(rnum < 0 || rnum > rarea->rown - 1) return(-1);

	if(rcnum < 0 || rcnum > rarea->coln - 1) return(-1);

	rbuf = buff;

	*rbuf++ = rarea->ldc;
	*rbuf++ = rarea->lwm;
	*rbuf++ = LEA | (rarea->fcol + rcnum);
	*rbuf++ = LLA | (rarea->frow + rnum);
	*rbuf = value & ALL12;

	if(write(rarea->fdesc, buff, 10) != 10) return(-1);
	return(0);
}
