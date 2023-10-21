#
#include        "/mnt/phil/cxap/area.define"

int unpack(buf,numpix,pixwd)
int     *buf;                           /*buffer to be unpacked         */
int     numpix;                         /*number of pixels              */
int     pixwd;                          /*pixel per word                */
{
	register
	int     bitpix,                 /*bit per pixel                 */
		pnt,                    /*points to physical word addr  */
		mask;                   /*mask used for unpacking       */

	if ((pixwd < 0) || (pixwd > WDSIZE))
	{
		printf(2,"Illegal unpacking density\n");
		return(ERROR);
	}

	if ((pixwd == 0) || (pixwd == 1))
		return(SUCCES);

	bitpix = WDSIZE / pixwd;

	mask = ~(~0 << bitpix);

	for (pnt = numpix - 1; pnt >= 0; pnt--)
					/*unpack each pixel             */
		buf[pnt] = mask & ((buf[pnt / pixwd]) >>
			   ((pnt % pixwd) * bitpix));

	return(SUCCES);

} /*end unpack*/
