	/*	Alarmclk.c  Compact reminder program.  Normally run
		in background.  S. Klyce - LSU/NO		*/
main(argc,argv)
int argc;
char **argv;
{
	int tbuf[2]; char *c;
	printf("Set for %s\n",*++argv);
	++argv;
	printf("Message: %s\n",*argv);
	--argv;
	if (argc < 3 || **argv < '0' || **argv > '9') useage();
	for (;;)
	{
		time(tbuf);
		c = ctime(tbuf);
		if (*(c+11) == **argv && *(c+12) == (*argv)[1] && *(c+14) == (*argv)[3] && *(c+15) == (*argv)[4])
		{
			printf("\007%s\n",*argv);
			printf("%s\n",*++argv);
			exit(0);
		}
		else sleep(15);
	}
}

useage()
{
	printf("\007USE:  alarmclk hh:mm \"message string\"\n");
	exit(0);
}
