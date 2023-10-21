#

/*      XAPOUT - write a picture to tape                                */


#define ERROR   -1              /*error flag*/
#define EOF     0               /*end-of-file*/
#define READ    0               /*read operation*/
#define DEPTH   1               /*number of rows kept in core*/
#define WRITE   1               /*write operation*/
#define ASIZE   30              /*area size*/
#define NSIZE   72              /*name size*/
#define RLENGTH 10000           /*length of longest row*/

extern  scanf();

main(argc,argv)
int     argc;                   /*argument count*/
char    **argv;                 /*argument vector*/
{
	int     ci,ri;          /*column and row increment (unused)*/
	int     ncol,nrow;      /*# of columns and rows in input window*/
	int     shift;          /*output shift*/
	int     size;           /*byte size of input picture*/
	int     source;         /*source file*/
	int     tape;           /*tape file*/

	int     area[ASIZE];    /*cxap work area*/
	int     buffer[RLENGTH];/*I/O buffer*/
	int     iw[4];          /*input window*/
	int     ptr[DEPTH];     /*pointer vector*/

	char    ifile[NSIZE];
	char    ofile[NSIZE];

	register
	int     i,
		status;         /*function status*/

	ioinfo(3,"Usage: xapout picture tape [fc] [fr] [nc] [nr] [shift]\n",
	       argc,argv,ifile,ofile,iw,&ci,&ri,&shift,&size);
				/*open output tape file*/
	if ((tape = open(ofile,WRITE)) == ERROR)
	{
		printf(2,"%s not opened, status %d\n",ofile,tape);
		cexit();
	}
				/*open input picture file*/
	if ((status = setupr(area,ifile,iw,shift,DEPTH,ptr,buffer,RLENGTH) == 0))
	{
		printf(2,"%s not opened, status %d\n",ifile,status);
		cexit();
	}
				/*extract window parameters*/
	ncol = iw[2];
	nrow = iw[3];
				/*write picture to output tape file*/
	for (i = 0; i < nrow; i++)
	{
		if ((status = pread(area,1)) == 0)
		{
			printf(2,"Row %d of %s not read, status %d\n",i+1,ifile,status);
			cexit();
		}
				/*pack and output picture row*/
		pack(buffer,ncol,2);
		if ((status = write(tape,buffer,ncol)) == ERROR)
		{
			printf(2,"Row %d not written to %s, status %d\n",i,ofile,status);
			cexit();
		}
	}
				/*close files*/
	if ((status = close(tape)) == ERROR)
		printf(2,"%s not closed, status %d\n",ofile,status);
	else
		if ((status = xclose(area)) == ERROR)
			printf(2,"%s not closed, status %d\n",ifile,status);

	cexit();

} /*end XAPOUT*/
