#
#include        "./gparam"

/*
**      This function will write a vector from (fcol, frow) to
**      (lcol,lrow)  into  the  selected  subchannels  of  the
**      enabled channels.
*/

int gwvec(area, fcol, frow, lcol, lrow, rctngl)
struct abuf *area;
{
	register struct abuf *rarea;
	register t1;
	register *rbuf;
	int t2;
	int buff[8];

	rarea = area;

	if((t1 = fcol & ALL9) >= rarea->coln || (t2 = frow & ALL9) >= rarea->rown ||
	   (lcol =& ALL9) >= rarea->coln || (lrow =& ALL9) >= rarea->rown)
		return(-1);

	rbuf = buff;

	*rbuf++ = rarea->ldc;
	*rbuf++ = rarea->lsm;
	*rbuf++ = (rarea->lwm | (rctngl ? RCTNGL : VECTOR)) & ~(DHGHT | DWDTH);
	*rbuf++ = LEA | (rarea->fcol + t1);
	*rbuf++ = LEB | ((lcol - t1) & MINUS1);
	*rbuf++ = LLA | (rarea->frow + t2);
	*rbuf++ = LLB | ((lrow - t2) & MINUS1);
	*rbuf = EGW;

	if(write(rarea->fdesc, buff, 16) != 16) return(-1);
	return(0);
}
