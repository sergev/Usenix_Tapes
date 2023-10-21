do_prog(argc,argv)
char *argv[];
{
	int result;
	if (666 == (result = fexecvp(argv[0],argv)))
	{
		invalid(argc,argv);
		perror("");
		return -1;
	}
	return result; 
}
