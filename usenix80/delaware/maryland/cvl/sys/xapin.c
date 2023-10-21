#

/*      XAPIN - read a raw formatted picture                            */

#define ERROR -1                /*error detection*/
#define EOF 0                   /*end-of-file*/
#define READ 0                  /*read operation*/
#define FMODE 0644              /*file mode*/
#define NSIZE 72                /*name size*/
#define BSIZE 1024              /*buffer size*/

extern scanf();

main(argc,argv)
int     argc;                   /*argument count*/
char    **argv;                 /*argument vector*/
{
	int     ci,ri;          /*column and row increment*/
	int     nrec;           /*record counter*/
	int     status;         /*status of function call*/

	int     fc,fr,nc,nr;    /*window parameters*/
	int     shift;          /*pixel shift*/
	int     size;           /*byte size of picture*/

	int     file,tape;      /*disk and tape file descriptors*/

	int     head[6];        /*output header*/
	int     iw[4];          /*input window vector*/

	char    buf[BSIZE];     /*input buffer*/
	char    ofile[NSIZE];   /*output file name*/
	char    ifile[NSIZE];   /*tape file name*/

	register
	int     cicntr,         /*column and row counters*/
		ricntr,
		subcntr;        /*subscript counter*/
				/*get user input*/
	ioinfo(2,"Usage: xapin tape file [fc] [fr] [nc] [nr] [ci] [ri]\n",
	       argc,argv,ifile,ofile,iw,&ci,&ri,&shift,&size);
				/*extract picture window*/
	fc = iw[0];
	fr = iw[1];
	nc = iw[2];
	nr = iw[3];
				/*open tape drive and output file*/
	if ((tape = open(ifile,READ)) == ERROR)
	{
		printf(2,"%s not opened, status %d\n",ifile,tape);
		cexit();
	}
	else
		if ((file = creat(ofile,FMODE)) == ERROR)
		{
			printf(2,"%s not opened/created, status %d\n",ofile,file);
			cexit();
		}
		else
			if ((status = write(file,buf,12)) == ERROR)
			{
				printf(2,"Blank header not written to %s, status %d\n",ofile,status);
				cexit();
			}
				/*default number of columns*/
	status = read(tape,buf,BSIZE);
	if ((argc < 6) || (nc > status))
		nc = status;
				/*skip to first actual row to read      */
	for (nrec = 2; (nrec < fr) && (status != EOF); nrec++)
		if ((status = read(tape,buf,BSIZE)) < EOF)
		{
			printf(2,"Initial record not skipped, status %d\n",status);
			cexit();
		}

	for (nrec = 0; (nrec < nr) && (status != EOF); nrec++)
	{
		subcntr = 0;
		for (cicntr = fc - 1; subcntr < nc; cicntr = cicntr + ci)
		{
			buf[subcntr] = buf[cicntr];
			subcntr++;
		}
		if (write(file,buf,nc) == ERROR)
		{
			printf(2,"Row %d not written, status %d\n",nrec+1,status);
			cexit();
		}
				/*skip rows*/
		for (ricntr = 1;(ricntr < ri) && (status != EOF); ricntr++)
			if ((status = read(tape,buf,BSIZE)) == ERROR)
			{
				printf(2,"Row %d not read, status\n",nrec+2,status);
				cexit();
			}
				/*read next picture row*/
		if (status != EOF)
			if ((status = read(tape,buf,BSIZE)) == ERROR)
			{
				printf(2,"Row %d not read, status\n",nrec+2,status);
				cexit();
			}
	}
				/*create output header*/
	head[1] = nc;
	head[3] = nrec;
	head[5] = 4;
				/*write new output header and close files*/
	if ((status = seek(file,0,0)) == ERROR)
		printf(2,"Error in seek to file beginning, status %d\n");
	else
		if ((status = write(file,head,12)) == ERROR)
			printf(2,"Final header not written, status\n",status);
		else
			if ((status = close(file)) == ERROR)
				printf(2,"%s not closed, status %d\n",ofile,status);
			else
				if ((status = close(tape)) == ERROR)
					printf(2,"%s not closed, status %d\n",ifile,status);

	cexit();

} /*end XAPIN*/
