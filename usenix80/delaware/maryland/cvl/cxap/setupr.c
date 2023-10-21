#
#include "/mnt/phil/cxap/area.define"

int setupr(area,name,iw,shift,depth,ptr,array,rlength)
int     *area;                          /*work area for cxap            */
char    *name;                          /*input file                    */
int     *iw;                            /*input picture window          */
int     shift;                          /*shift (in bits)               */
int     depth;                          /*number of rows in input buffer*/
int     *ptr;                           /*pointer vector                */
int     *array;                         /*input buffer                  */
int     rlength;                        /*length of input buffer        */
{
	int size;                       /*byte size                     */

	int     pichdr[HSIZE];          /*picture header                */

	register
	int     i,                      /*counter                       */
		pixwd,                  /*pixel per word                */
		frow;                   /*first row                     */
					/*open file                     */
	if ((FILE = open(name,READ)) == ERROR)
	{
		printf(2,"Input file not opened\n");
		return(ERROR);
	}
					/*set read flag                 */
	READWRITE = READ;

	if (read(FILE,pichdr,HLEN) == ERROR)
	{
		printf(2,"Input header not read\n");
		return(ERROR);
	}

	NCOL = pichdr[1];
	NROW = pichdr[3];
	SIZE = size = pichdr[5];
					/*check input window against pic*/
	if ((iw[0] > NCOL) || (iw[1] > NROW) ||
	    ((iw[0] + iw[2] - 1) > NCOL) ||
	    ((iw[1] + iw[3] - 1) > NROW))
	{
		printf(2,"Window invalid\n");
		return(ERROR);
	}
					/*save parameters               */
	SHIFT   = shift;
	IW0     = iw[0];
	IW2     = iw[2];
	IW3     = iw[3];
	RLENGTH = rlength;

	pixwd   = WDSIZE / (1 << (SIZE - 1));
	OFFSET  = (((NCOL - IW2) / pixwd) * 2) +
		   (((NCOL - IW2) % pixwd) > 0 ? 1 : 0);

	ARRAY   = array;
	PTR     = ptr;
	DEPTH   = depth;

	frow = (1 << (SIZE - 1)) * NCOL;
	frow = frow / 8;
	frow = frow % 8 > 0 ? 1:0;
	frow = frow * (iw[1] - 1);

	if (seek(FILE,frow,CRLOCPO) == ERROR)
	{
		printf(2,"Seek error\n");
		return(ERROR);
	}
					/*initialize pointer vector     */
	for (i = 0; i < depth; i++)
		ptr[i] = i;

	return(SUCCES);

} /*end setupr*/
