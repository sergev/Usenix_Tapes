#
#include "/mnt/phil/cxap/area.define"

int pwrite(area,num,buf)
int     *area;                          /*work area used by cxap        */
int     num;                            /*number of rows to be written  */
int     *buf;                           /*buffer cont'ng  row to be outp*/
{
	int file;                       /*output file                   */
	int iw2;                        /*number of columns             */
	int nbyte;                      /*number of bytes to output     */
	int pixwd;                      /*pixels per word               */
	int size;                       /*byte size                     */
	int shift;                      /*shift                         */
	register int i,
		     j,                 /*counters                      */
		     loc;               /*location of beginning of a row*/
					/*check write status of file    */
	if (READWRITE == READ)
	{
		printf(2,"File not to be written \n");
		return(ERROR);
	}
	if (num < 1)
		return(EOF);

	file  = FILE;
	iw2   = IW2;
	shift = SHIFT;
	size  = SIZE;
	pixwd = WDSIZE / (1 << (size - 1));
					/*calcu num of bytes output     */
	nbyte   = ((NCOL / pixwd) * 2) + (NCOL % pixwd > 0 ? 1:0);
	loc = 0;
	for (i = num; i > 0; i--)
	{
		if (shift)
			for (j = loc; j < loc + iw2; j++)
				buf[j] =<< shift;
					/*pack output row               */
		pack(buf + loc,iw2,pixwd);
					/*write output row              */
		if (write(file,buf + loc,nbyte) == ERROR)
		{
			printf(2,"Output file not written\n");
			return(ERROR);
		}
		loc =+ iw2;
					/*increment row written param   */
		ROWRITTEN++;
	}

	return(SUCCES);

} /*end pwrite*/
