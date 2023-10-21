#
#define BSIZE 1024

/*      ioinfo - input picture info                                     */

extern scanf();

ioinfo(min,msg,argc,argv,ifile,ofile,iw,ci,ri,shift,size)
int     min;                    /*minimum number of arguments*/
char    *msg;                   /*usage message*/
int     argc;                   /*argument count*/
char    **argv;                 /*argument vector*/
char    *ifile;                 /*input file*/
char    *ofile;                 /*output file*/
int     *iw;                    /*input window*/
int     *ci;                    /*column increment*/
int     *ri;                    /*row increment*/
int     *shift;                 /*pixel shift*/
int     *size;                  /*byte size of picture*/
{
	int	hbuf[6];	/*header buffer*/

	if (argc < min)
	{
		printf(2,"%s",msg);
		cexit();
	}
	else
	{                       /*initialize parameters*/
		iw[0] = 1;
		iw[1] = 1;
		iw[2] = BSIZE;
		iw[3] = BSIZE;
		*ci    = 1;
		*ri    = 1;
		*shift = 0;
		*size  = 4;

		if (argc > 1)
		{
			cvswap(ifile,argv[1]);
			if (!(cveql(ifile,"/dev",4)))
			{
				if (header(ifile,hbuf) == -1)
				{
					printf(2,"%s header not read\n",ifile);
					cexit();
				}
				else
				{

					iw[2]  = hbuf[1];
					iw[3]  = hbuf[3];
					*size  = hbuf[5];
				}
			}
			if (argc > 2)
			{
				cvswap(ofile,argv[2]);
				if (argc > 3)
				{
					iw[0] = atoi(argv[3]);
					if (argc > 4)
					{
						iw[1] = atoi(argv[4]);
						if (argc > 5)
						{
							iw[2] = atoi(argv[5]);
							if (argc > 6)
							{
								iw[3] = atoi(argv[6]);
								if (argc > 7)
								{
									*ci = atoi(argv[7]);
									if (argc > 8)
									{
										*ri = atoi(argv[8]);
										if (argc > 9)
											*shift = atoi(argv[9]);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return;

} /*end IOINFO*/
