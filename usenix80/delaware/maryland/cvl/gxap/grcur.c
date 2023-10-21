#
#include        "./gparam"

/*
**      This function will  read  in the position of  both cursors
**      to an internal buffer,   change the values  to  be between
**      0 and 511 and place them in the user's buffer in the order
**      (x0, y0, x1, y1).
*/

int grcur(area, ubuf, asynch)
struct abuf *area;
int ubuf[4];
{
	register struct abuf *rarea;
	register *rubuf;
	register *rbuf;
	int junk[1];
	int buff[7];

	rarea = area;

	rbuf = buff;
	rubuf = ubuf;

	*rbuf++ = SPD | MCURSR;
	*rbuf++ = LPA;  /* Cursor number 0 (or 1 if you like) */
	*rbuf++ = RPD;

	if(asynch)   /* Wait for an asynchronous interrupt (TRACK or ENTER) */
		read(rarea->fdesc,junk,2);

	if(read(rarea->fdesc, buff, 14) != 14) return(-1);      /* Read in cursors */

	*rubuf++ = *rbuf++ & ALL9;
	*rubuf++ = *rbuf++ & ALL9;
	*rubuf++ = *rbuf++ & ALL9;
	*rubuf = *rbuf & ALL9;

	return(0);
}
