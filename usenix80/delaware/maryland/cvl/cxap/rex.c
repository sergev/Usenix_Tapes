#
#define ERROR   0               /*error flag                            */
#define EOF    -1               /*end of file                           */
#include        "/mnt/phil/cxap/area.define"

int rex(area,buf,ptr)
int     *area;                  /*work area                             */
int     *buf;                   /*input buffer                          */
int     *ptr;                   /*vector of pointers                    */
{
	register
	int     *base,          /*base addresse                         */
		status,         /*read status                           */
		i;              /*counter                               */

	status = pread(area,1);

	if (status == EOF || status == ERROR)
		return(status);

	base = buf + ptr[DEPTH - 1] * RLENGTH;

	if (LEFTBOR > 0)
		for (i = 0; i < LEFTBOR; i++)
		       *(base + LEFTBOR - i - 1) = *(base + LEFTBOR + i);

	if (RIGHTBOR > 0)
		for (i = 0; i < RIGHTBOR; i++)
		       *(base + RBORLOC + i) = *(base + RBORLOC - i - 1);

	return(SUCCES);

} /*end rex*/
