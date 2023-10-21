#
#include        "/mnt/phil/cxap/area.define"

int copyup(area,buf,ptr)
int     *area;                  /*work area                             */
int     *buf;                   /*input buffer                          */
int     *ptr;                   /*vector of pointers                    */
{
	int     *base1;
	int     *base2;
	int     i;

	base1 = buf + ptr[DEPTH - 2 * POSITION] * RLENGTH;
	base2 = buf + ptr[DEPTH - 1] * RLENGTH;

	for (i = 0; i < OUTCOL; i++)
		*(base1 + i) = *(base2 + i);

	return(SUCCES);

} /*end copyup*/
