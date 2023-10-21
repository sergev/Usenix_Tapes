#
#include        "./gparam"

/*
**      This function will scroll the image up
**      or down "num" rows.  The  image may be
**      scrolled up to 512 lines.
*/

int gscrl(area, num, updwn)
struct abuf *area;
{
	register *rbuf;
	register i, com;
	int buff[512];

	rbuf = buff;

	if(num <= 0 || num > 512) return(-1);

	com = SLU | SINHBT | (updwn ? SDOWN : SUP);

	for(i=0;i<num;i++)
		*rbuf++ = com;

	if(write(area->fdesc, buff, com = i * 2) != com)
		return(-1);
	return(0);
}
