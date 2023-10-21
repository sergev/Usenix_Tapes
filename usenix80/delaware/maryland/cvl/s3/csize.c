#
#include        "/mnt/phil/cxap/area.define"

main (argc,argv)
int     argc;
char    **argv;


{
	int     hbuf[HSIZE];

	register
	int     file,
		status;

	if (argc == 1)
	{
		printf("Usage:  csize <file> <size>\n");
		cexit();
	}

	if ((file = open(argv[1],IO)) == ERROR)
	{
		printf(2,"Picture %s not opened, status: %d\n",argv[1],file);
		cexit();
	}

	if (((status = read(file,hbuf,HLEN)) == ERROR) || (status == EOF))
	{
		printf(2,"Header in %s not read, status: %d\n",argv[1],status);
		cexit();
	}

	if ((status = seek(file,0,0)) == ERROR)
	{
		printf(2,"Seek to beginning of %s unsuccessful, status: %d\n",argv[1],status);
		cexit();
	}

	hbuf[5] = atoi(argv[2]);

	if ((status = write(file,hbuf,HLEN)) == ERROR)
	{
		printf(2,"Header not written to %s, status:\n",argv[1],status);
		cexit();
	}

	cexit();
}



