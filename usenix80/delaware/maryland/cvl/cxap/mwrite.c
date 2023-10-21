#
#include "/mnt/phil/cxap/area.define"

int mwrite(area,num,vector)
int     *area;                          /*work area for CXAP            */
int     num;                            /*number of rows to output      */
int     *vector;                        /*output buffer                 */
{
	register
	int     status;                 /*status of a function call     */

	if (num < 1)
		return(EOF);

	for (; num > 0; num--)
		if ((status = pwrite(area,num,vector)) != SUCCES)
			return(status);

	return(SUCCES);

} /*end mwrite*/
