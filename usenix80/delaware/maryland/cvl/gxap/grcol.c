#
#include        "./gparam"

/*
**      This routine will input one column from the user's
**      window  into an  internal buffer.   This buffer is
**      then copied to the user's column buffer,  deleting
**      the header information.
*/

int grcol(area, cbuf, cnum, cstart, npts, inc)
struct abuf *area;
int cbuf[];
{
	register struct abuf *rarea;
	register rcnum, rcstart;

	rarea = area;

	rcnum = cnum;
	rcstart = cstart;

	if(rarea->dsize) {      /* Double size output */
		rcnum =* 2;
		rcstart =* 2;
		inc =* 2;
	}

	if(gtestp(rcnum, rcstart, npts, inc, rarea->coln, rarea->rown))
		return(-1);

	return(grwcom(rarea->ldc, rarea->lsm, LUM | LBNLA, LLA | (rarea->frow + rcstart),
	       LLB | (inc & MINUS1), LEA | (rarea->fcol + rcnum), cbuf, npts, rarea->fdesc));
}
