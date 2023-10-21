#
#include "/mnt/phil/cxap/area.define"

int pread(area,num)
int     *area;                          /*work area for cxap            */
int     num;                            /*number of rows to read        */
{
	int     *array;                 /*input buffer                  */
	int     *ptr;                   /*pointer vector                */

	int     depth;                  /*number of rows in input buffer*/
	int     file;                   /*picture file                  */
	int     iw0;                    /*first column of user window   */
	int     iw2;                    /*number of col in user window  */
	int     nbyte;                  /*number of bytes               */
	int     offset;                 /*offset                        */
	int     pixwd;                  /*pixels per word               */
	int     rlength;                /*maximum length of a row       */
	int     shift;                  /*shift                         */
	int     size;                   /*size of pixels                */
	int     status;                 /*status of read operation      */
	int     temp;                   /*temporary                     */

	register
	int     i,
		j,
		*base;

	if (READWRITE == WRITE)
	{
		printf(2,"File not to be read\n");
		return(ERROR);
	}
	if (num < 1)
		return(EOF);

	array   = ARRAY;
	depth   = DEPTH;
	file    = FILE;
	iw0     = IW0;
	iw2     = IW2;
	offset  = OFFSET;
	rlength = RLENGTH;
	ptr     = PTR;
	shift   = SHIFT;
	size    = SIZE;
	pixwd   = WDSIZE / (1 << (size - 1));
					/*calculate num of bytes input  */
	nbyte   = ((NCOL / pixwd) * 2) + (NCOL % pixwd > 0 ? 1:0);

	for (i = 0; (i < num) && (IW3 > 0); i++)
	{
		if (IW3 == EOF)
			return(EOF);

		base = &array[0] + (ptr[0] * rlength);
					/*read a row                    */
		status = read(file,base,nbyte);

		if (status == EOF)
			return(EOF);
		else
			if (status == ERROR)
			{
				printf(2,"Read error\n");
				return(ERROR);
			}
					/*unpack current row            */
		unpack(base,NCOL,pixwd);

		for (j = 0;j < iw2; j++)
			base[j] = base[j + iw0 - 1];
					/*shift current row             */
		if (shift)
			for (j = 0; j < iw2; j++)
			       base[j] =<< shift;
					/*rotate pointer vector         */
		temp = ptr[0];
		for (j = 0; j < depth - 1; j++)
		   ptr[j] = ptr[j+1];
		ptr[depth - 1] = temp;
					/*decrement user window row cnt */
		IW3 =- 1;
	}

	return(SUCCES);

} /*end pread*/
