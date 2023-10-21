#
#include        "/mnt/phil/cxap/area.define"

int pack(buf,numpix,pixwd)
int     *buf;                           /*buffer to be packed           */
int     numpix;                         /*number of pixels              */
int     pixwd;                          /*pixels per word               */
{
	register
	int     mask,                   /*mask used for packing         */
		bitpix,                 /*bits per pixel                */
		ptr;                    /*points to current unpacked pix*/

	if ((pixwd < 0) || (pixwd > WDSIZE))
	{
		printf(2,"Illegal unpacking density\n");
		return(ERROR);
	}

	if ((pixwd == 0) || (pixwd == 1))
		return(SUCCES);

	bitpix = WDSIZE / pixwd;

	mask = ~(~0 << bitpix);

	for (ptr = 0; ptr <= (numpix - 1); ptr++)
					/*pack each pixel*/
		if (ptr % pixwd)
			buf[ptr / pixwd] =| ((mask & buf[ptr]) <<
					    ((ptr % pixwd) * bitpix));
		else
			buf[ptr / pixwd] = (mask & buf[ptr]);

	return(SUCCES);

} /*end pack*/
