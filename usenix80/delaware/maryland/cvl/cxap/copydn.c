#
#include        "/mnt/phil/cxap/area.define"

int copydn(area,buf,ptr)
int     *area;                  /*work area                             */
int     *buf;                   /*input buffer                          */
int     *ptr;                   /*vector of pointers                    */
{

	int     i;
	int     temp;
	register
	int     *base1,
		*base2,
		dif;

	base1 = buf + ptr[0] * RLENGTH;
	dif   = DEPTH - 2 * (POSITION - 1 - WROWS);
	base2 = buf + ptr[dif - 1] * RLENGTH;

	for (i = 0; i < OUTCOL; i++)
		*(base1 + i) = *(base2 + i);

	temp = ptr[0];
	for (i = 1; i < DEPTH; i++)
		ptr[i - 1] = ptr[i];
	ptr[DEPTH - 1] = temp;

	return(SUCCES);

} /*end copydn*/
