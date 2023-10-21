
echo(argc,argv)
	char *argv[];
{
	register int i;
	for (i = 1; i < argc;i++)
	{
		write(2,argv[i],strlen(argv[i]));
		if (i < argc-1)
			write(2," ",1);
	}
	crlf();
	return 0;
}
