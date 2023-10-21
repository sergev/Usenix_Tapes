#
#include        "./gparam"

/*
**      This function will return as its value the data
**      located at window-relative location (cnum,rnum)
*/

int grpnt(area, cnum, rnum)
struct abuf *area;
{
	register struct abuf *rarea;
	register *rbuf;
	register rcnum;
	int buff[7];

	rarea = area;

	rcnum = cnum;
	if(rarea->dsize) {      /* Double size output */
		rcnum =* 2;
		rnum =* 2;
	}

	if(rnum < 0 || rnum >= rarea->rown) return(-1);

	if(rcnum < 0 || rcnum >= rarea->coln) return(-1);

	rbuf = buff;

	*rbuf++ = rarea->ldc;
	*rbuf++ = rarea->lsm;
	*rbuf++ = LEA | (rarea->fcol + rcnum);
	*rbuf++ = LLA | (rarea->frow + rnum);
	*rbuf++ = SPD | RDBAK;
	*rbuf++ = RPD;

	if(read(rarea->fdesc, buff, 14) != 14) return(-1);

	return(*rbuf & ALL12);
}
