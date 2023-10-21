#
#include        "/mnt/phil/cxap/area.define"

int bread(area,buf,ptr,bias)
int     *area;                  /*work area                             */
int     *buf;                   /*input buffer                          */
int     *ptr;                   /*vector of pointers                    */
int     *bias;                  /*left border                           */
{
	register
	int     status;         /*status of read operation              */

	*bias = LEFTBOR;
				/*increment row input count             */
	POSITION++;
				/*check for end-of-file                 */
	if (POSITION > (WROWS + BOTBOR))
		return(EOF);
				/*read next row and determine how row
				  is to be copied into user buffer      */
	if (POSITION <= WROWS)
	{
		status = rex(area,buf,ptr);
		if ((status ==  EOF) || (status == ERROR))
			return(status);
	}

	if (POSITION <= TOPBOR)
		copyup(area,buf,ptr);

	if (POSITION > WROWS)
		copydn(area,buf,ptr);

	return(SUCCES);

} /*end bread*/
