md(argc,argv)
	char *argv[];
{
	if (-1 == mkdir(*(++argv)))
	{
		perror("mkdir");
		return -1;
	}
	return 0;
}
