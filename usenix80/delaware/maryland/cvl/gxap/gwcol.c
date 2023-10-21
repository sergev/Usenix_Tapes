#
#include        "./gparam"

/*
**      This routine will copy one column from the user's buffer
**      into an internal temporary buffer.   This buffer is then
**      written out to the  correct window on  the display as is
**      described by the   values in the area parameter  sent by
**      the user.
*/

int gwcol(area, cbuf, cnum, cstart, npts, inc)
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

	return(grwcom(rarea->ldc, rarea->lwm, LUM | LBNLA, LLA | (rarea->frow + rcstart),
	       LLB | (inc & MINUS1), LEA | (rarea->fcol + rcnum), cbuf, npts, rarea->fdesc));
}
