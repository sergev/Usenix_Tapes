#

/*      CHEAD - changes the header of a CXAP picture.                   */
/*                                                                      */
/*              Usage: chead picture [fc] [fr] [size]                   */
/*                                                                      */
/*              Load: load chead -lp -lz                                */

#define FAILURE -1
#define EOF     0
#define SUCCESS 1

#define WRITE   1

extern  scanf();

main(argc,argv)
int     argc;
char    **argv;
{
	int     file;           /*file descriptor for picture file*/

	int     hbuf[6];        /*buf for picture header*/

	if (argc < 2)
		printf(2,"Usage: chead picture [nc] [nr] [size]\n");
	else
	{
		if (header(argv[1],hbuf) == SUCCESS)
		{
			if (argc > 2)
			{
				hbuf[1] = atoi(argv[2]);
				if (argc > 3)
				{
					hbuf[3] = atoi(argv[3]);
					if (argc > 4)
						hbuf[5] = atoi(argv[4]);
				}
			}
			if ((hbuf[1] > 0) && (hbuf[3] > 0) && ((hbuf[5] >= 1) && (hbuf[5] <=5)))
			{
				if ((file = open(argv[1],WRITE)) != SUCCESS)
					write(file,hbuf,12);
				else
					printf(2,"Header not changed in %s\n",argv[1]);
			}
			else
				printf(2,"Error in header update - check parameters\n");
		}
		else
			printf(2,"Header of %s not read\n",argv[1]);
	}

	close(file);

	cexit();

} /*end CHEAD*/
