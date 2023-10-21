#
#include        "./gparam"

/*
**      This routine will input one row from the user's
**      window into an internal buffer.  This buffer is
**      then copied to the user's row buffer,  deleting
**      the header information.
*/

int grrow(area, rbuf, rnum, rstart, npts, inc)
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

	return(grwcom(rarea->ldc, rarea->lsm, LUM | EBNEA, LEA | (rarea->fcol + rrstart),
	       LEB | (inc & MINUS1), LLA | (rarea->frow + rrnum), rbuf, npts, rarea->fdesc));
}
