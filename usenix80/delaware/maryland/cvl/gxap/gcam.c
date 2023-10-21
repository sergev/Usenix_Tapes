#
#include        "./gparam"

/*
**      This function is used to input an image with
**      the television camera.   Frame averaging can
**      be done if wanted by setting the appropriate
**      parameters.
*/

int gcam(area, nfrms, shift)
struct abuf *area;
{
	register *rbuf;
	int buff[6];

	rbuf = buff;

	*rbuf++ = LDC | IMAGE;
	*rbuf++ = LSM | ALL12;
	*rbuf++ = SPD | DIGIT;
	*rbuf++ = LPR | (shift & 7);
	*rbuf++ = LPD | (nfrms & 0377);
	*rbuf = SPD;

	if(write(area->fdesc, buff, 12) != 12) return(-1);
	return(0);
}
