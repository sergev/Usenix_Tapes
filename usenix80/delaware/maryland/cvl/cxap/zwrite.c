#
#include "/mnt/phil/cxap/area.define"

int zwrite(area,num)
int     *area;                          /*work area used for CXAP       */
int     num;                            /*number of zeroes row to write */
{
	int     zbuf[MAXCOL];           /*zero buffer for output        */

	int     iw2;                    /*number of columns in out pic  */

	register
	int     i;                      /*increment                     */

	iw2 = IW2;

	if (iw2 > MAXCOL)
	{
		printf(2,"Too many columns in picture\n");
		return(ERROR);
	}

					/*initialize zero buffer        */
	for (i = 0; i < iw2; i++)
		zbuf[i] = 0;
					/*call pwrite                   */
	return (pwrite(area,num,zbuf));

} /*end zwrite*/
