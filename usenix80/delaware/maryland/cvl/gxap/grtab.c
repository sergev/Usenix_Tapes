#
#include        "./gparam"

/*
**      This function will read back the 'num'  values
**      from the lookup table int 'ubuf',  starting at
**      location 'loc'.
*/

int grtab(area, ubuf, loc, num)
struct abuf *area;
int ubuf[];
{
	register i;
	register *rbuf;
	register struct abuf *rarea;
	int buf[4100];

	rarea = area;

	if((loc | num) <= 0 || (loc + num) > 4096)
		return(-1);

	rbuf = buf;

	*rbuf++ = SPD | LKUPTAB;/* Select lookup table */
	*rbuf++ = LPA | loc;    /* Select lookup address */
	*rbuf++ = RPD;          /* Read back the values */
	if(read(rarea->fdesc, buf, i = num * 2 + 6) != i)
		return(-1);

	for(i=0;i<num;i++)
		ubuf[i] = *rbuf++ & ALL12;

	buf[0] = SPD;   /* Deselect the lookup table */
	write(rarea->fdesc, buf, 2);

	return(0);
}
