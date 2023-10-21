main(argc, argv)
char *argv[];
{
	register i;
	if(argc<2)
		i=3;
	else
		i=atoi(argv[1]);
	do {
		write(2, "\7", 1);
		nap(30);
	} while(--i);
}
