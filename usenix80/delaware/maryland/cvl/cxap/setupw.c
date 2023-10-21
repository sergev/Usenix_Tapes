#
#include "/mnt/phil/cxap/area.define"

int setupw(area,name,iw,shift,size)
int     *area;                          /*work area for cxap            */
char    *name;                          /*picture file                  */
int     *iw;                            /*dimensions of output picture  */
int     shift;                          /*shift (in bits)               */
int     size;                           /*bytes per pixel               */
{
	int pichdr[HSIZE];              /*picture header                */
					/*create output file            */
	if ((FILE = creat(name,FMODE)) == ERROR)
	{
		printf(2,"Output file not opened\n");
		return(ERROR);
	}
					/*set write flag                */
	READWRITE = WRITE;
					/*create output header          */
	pichdr[1] = NCOL = iw[2];
	pichdr[3] = NROW = iw[3];
					/*check picture size            */
	if ((size > 5) || (size < 1))
	{
		printf(2,"Illegal byte size for %s\n",name);
		return(ERROR);
	}
	pichdr[5] = SIZE = size;
					/*write output header           */
	if (write(FILE,pichdr,HLEN) == ERROR)
	{
		printf(2,"Output header not written\n");
		return(ERROR);
	}

	SHIFT = shift;
					/*initialize parameters         */
	IW2 = NCOL;
	IW3 = NROW;
	ROWRITTEN = 0;

	return(SUCCES);

} /*end setupr*/
