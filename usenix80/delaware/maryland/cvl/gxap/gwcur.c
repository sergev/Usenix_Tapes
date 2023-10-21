#
#include        "./gparam"

/*
**      This function will write  out the selected cursor to the
**      window-relative position (col,row), either on or off and
**      black or white depending on the color parameter.
*/

int gwcur(area, curnum, col, row, on, color)
struct abuf *area;
{
	register struct abuf *rarea;
	register rcol;
	register *rbuf;
	int buff[8];

	rarea = area;

	if(curnum < 1 || curnum > 2) return(-1);
	rcol = col;

	if(rcol < 0 || rcol >= rarea->coln) return(-1);
	if(row < 0 || row >= rarea->rown) return(-1);
	rcol = curnum - 1;

	if(color)
		rarea->acolr =| 1 << rcol;
	else
		rarea->acolr =& ~(1 << rcol);
	if(on)
		rarea->acurs =| 1 << rcol;
	else
		rarea->acurs =& ~(1 << rcol);

	rbuf = buff;

	*rbuf++ = SPD | MCURSR;
	*rbuf++ = LPA | (rcol << 1);
	*rbuf++ = LPD | ABS | (rarea->fcol + col);
	*rbuf++ = LPA | (rcol << 1) | 1;
	*rbuf++ = LPD | ABS | (rarea->frow + row);
	*rbuf++ = LPR | CCOLOR | rarea->acolr;
	*rbuf++ = LPR | rarea->acurs;
	*rbuf = SPD;

	if(write(rarea->fdesc,buff,16) != 16) return(-1);
	return(0);
}
