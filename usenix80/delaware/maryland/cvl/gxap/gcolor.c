#
#include        "./gparam"

/*
**      This function allows the user to change the display
**      from  black and white  to color and back again,  as
**      determined by the 'gray' argument.
*/

int gcolor(area, gray)
struct abuf *area;
{
	register *rbuf;
	int buff[3];

	rbuf = buff;

	*rbuf++ = SPD | VCNTRL;
	*rbuf = LPR;
	if(gray)
		*rbuf =| GRAY;  /* Black and white if nonzero */
	*++rbuf = SPD;

	if(write(area->fdesc, buff, 6) != 6) return(-1);

	return(0);
}
