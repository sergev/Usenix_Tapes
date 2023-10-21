#
#include        "./gparam"

/*
**      This function will load the lookup table  with
**      the values in 'ubuf' starting at location 'loc'
**      and entering 'num' values.
*/

int gwtab(area, ubuf, loc, num)
struct abuf *area;
int ubuf[];
{
	register i;
	register *rbuf;
	register struct abuf *rarea;
	int buff[4100];

	rarea = area;

	if((loc | num) <= 0 || (loc + num) > 4096)
		return(-1);

	rbuf = buff;

	*rbuf++ = SPD | LKUPTAB; /* Select lookup table */
	*rbuf++ = LPA | loc;     /* Select the lookup address */

	for(i=0;i<num;i++)
		*rbuf++ = (ubuf[i] & ALL12) | LPD;

	*rbuf = 0120000;       /* SPD */

	if(write(rarea->fdesc, buff, i = num*2+6) != i)
		return(-1);

	return(0);
}
