#
#include        "./gparam"

/*
**      This function will set up the user's area region
**      with certain default values, as well as open the
**      display and assign the window wanted.
*/

int gopflg 0;
int gopfd;

int gopen(area, fc, fr, ncol, nrow)
struct abuf *area;
{
	register struct abuf *rarea;

	rarea = area;

	if(((fc | fr | ncol | nrow) < 0) || (fc + ncol) > 512 || (fr + nrow) > 512)
		return(rarea->fdesc = -1);

	rarea->fcol = fc;               /* First column */
	rarea->frow = fr;               /* First row */
	rarea->coln = ncol;             /* Number of columns */
	rarea->rown = nrow;             /* Number of rows */

	rarea->dsize = 0;
	rarea->acurs = 0;

	rarea->ldc = LDC | IMAGE;       /* Overlay not enabled */
	rarea->lsm = LSM | ALL12;       /* All subchannels enabled */
	rarea->lwm = LWM | ZERO;        /* Write zeroes */

	if(gopflg)  /* Device already open for this run */
		rarea->fdesc = gopfd;
	else	/* The first open of this run */
		if((rarea->fdesc = gopfd = open(DISPLAY, 2)) < 0) return(-1);

	gopflg++;	/* One more open has taken place */
	return(0);
}
