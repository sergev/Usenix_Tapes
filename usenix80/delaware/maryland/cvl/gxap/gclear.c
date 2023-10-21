#
#include        "./gparam"

/*
**      This function will clear out a subwindow of the user's
**      physical window.  The subwindow can be as large as the
**      whole window or as small as one point. The subchannels
**      cleared are determined by the value of area->lsm.
*/

int gclear(area, fc, fr, ncol, nrow, subchan)
struct abuf *area;
{
	register struct abuf *rarea;
	register temp;
	register *rbuf;
	int buff[8];

	rarea = area;

	rbuf = buff;

	if(((fc | fr) < 0) || (ncol <= 0) || (nrow <= 0))
		return(-1);

	if(((fc + ncol) > rarea->coln) || ((fr + nrow) > rarea->rown))
		return(-1);

	*rbuf++ = rarea->ldc;           /* Whatever is there is used */
	if(temp = subchan & ALL12)      /* Then use parameter */
		temp =| LSM;
	else                            /* Else, use what is in area */
		temp = rarea->lsm;
	*rbuf++ = temp;
	*rbuf++ = (rarea->lwm & ~(DHGHT | DWDTH | ZERO)) | BAKGND;  /* Double size disabled */
	*rbuf++ = LEA | rarea->fcol + fc; /* Set up the first column and */
	*rbuf++ = LLA | rarea->frow + fr; /* the first row and */
	*rbuf++ = LEB | ncol - 1;       /* the last column and */
	*rbuf++ = LLB | nrow - 1;       /* the last row. */
	*rbuf = EGW;                  /* The command that will do it */

	if(write(rarea->fdesc,buff,16) != 16) return(-1);    /* Clear the window */

	return(0);
}
