#
#include        "./gparam"

/*
**      This function will change the values in the 'area'
**      buffer according to the parameters sent.   If the
**      parameter is zero, that value is left untouched.
*/

int genter(area, channel, subchan, bakgnd, zwrite, dsize)
struct abuf *area;
{
	register struct abuf *rarea;
	register temp;

	rarea = area;

	if(channel) {
		if((temp = channel & 7) == 0) return(-1);
		rarea->ldc = LDC | temp;
	}

	if(subchan) {
		rarea->lsm = LSM | (subchan & ALL12);
	}

	temp = rarea->lwm;

	if(bakgnd) {
		if(bakgnd == 1) /* Select light background */
			temp =| BAKGND;
		else            /* Select dark background (default) */
			temp =& ~BAKGND;
	}

	if(zwrite) {
		if(zwrite == 1) /* No write for zero bits */
			temp =& ~ZERO;
		else            /* Write zero bits as zeroes (default) */
			temp =| ZERO;
	}

	if(dsize) {
		if(dsize == 1) { /* Double size I/O */
			temp =| DHGHT | DWDTH;
			rarea->dsize = 1;
		}
		else {  /* Standard size image */
			temp =& ~(DHGHT | DWDTH);
			rarea->dsize = 0;
		}
	}

	rarea->lwm = temp;

	return(0);
}
