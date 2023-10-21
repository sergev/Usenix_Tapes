#

/*      MAX - find maximum value in a picture                           */

#define SHIFT           0
#define DEPTH           1
#define RLENGTH         512

extern  scanf();

main(argc,argv)
int     argc;
char    **argv;
{
	int     min;                    /*minimum value                 */
	int     ncol;                   /*# of columns in picture       */
	int     nrow;                   /*# of rows in picture          */
	int     ptr;                    /*subscript for input buffer    */

	int     hbuf[6];                /*header buffer                 */
	int     buffer[RLENGTH];        /*io buffer                     */
	int     iarea[30];              /*io picture work areas         */
	int     iw[4];                  /*input window                  */

	register
	int     i,j;

	if (argc < 2)
	{
		printf(2,"Usage: min picture\n");
		cexit();
	}

	header(argv[1],hbuf);
	iw[0] = iw[1] = 1;
	ncol  = iw[2] = hbuf[1];
	nrow  = iw[3] = hbuf[3];

	setupr(iarea,argv[1],iw,SHIFT,DEPTH,ptr,buffer,RLENGTH);

	min = 0;

	for (i = 0; i < nrow; i++)
	{
		pread(iarea,1);
		for (j = 0; j < ncol; j++)
			min = min <= buffer[j] ? min : buffer[j];
	}

	xclose(iarea);

	printf(2,"Minimum value of %s is %d\n",argv[1],min);

	cexit();
}
