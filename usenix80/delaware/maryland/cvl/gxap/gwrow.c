#
#include        "./gparam"

/*
**      This routine will copy  one row from the user's buffer
**      into an internal temporary buffer. This buffer is then
**      written out to the correct window on the display as is
**      described by the  values in the area parameter sent by
**      the user.
*/

int gwrow(area, rbuf, rnum, rstart, npts, inc)
struct abuf *area;
int rbuf[];
{
	register struct abuf *rarea;
	register rrnum, rrstart;

	rarea = area;

	rrnum = rnum;
	rrstart = rstart;
	if(rarea->dsize) {      /* Double size output */
		rrnum =* 2;
		rrstart =* 2;
		inc =* 2;
	}
	if(gtestp(rrnum, rrstart, npts, inc, rarea->rown, rarea->coln))
		return(-1);

	return(grwcom(rarea->ldc, rarea->lwm, LUM | EBNEA, LEA | (rarea->fcol + rrstart),
	       LEB | (inc & MINUS1), LLA | (rarea->frow + rrnum), rbuf, npts, rarea->fdesc));
}
