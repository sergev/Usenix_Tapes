#
#include "/mnt/phil/cxap/area.define"

int xclose(area)
int     *area;                          /*work area for CXAP            */
{
	int pichdr[HSIZE];              /*picture header                */

	if (READWRITE)
					/*insure header is correct      */
		if (ROWRITTEN != IW3)
		{
			printf(2,"Warning --- Output file prematurely closed, header corrected\n");
			pichdr[1] = NCOL;
			pichdr[3] = ROWRITTEN;
			pichdr[5] = SIZE;

			if ((seek(FILE,0,0)) == ERROR)
			{
				printf(2,"Error on header seek\n");
				return(ERROR);
			}
			else
				if (write(FILE,pichdr,2 * HSIZE) == ERROR)
				{
					printf(2,"New header not written\n");
					return(ERROR);
				}
		}
					/*close file                    */
	if (close(FILE) == ERROR)
	{
		printf(2,"File not closed\n");
		return(ERROR);
	}
	else
		return(SUCCES);

} /*end xclose*/
