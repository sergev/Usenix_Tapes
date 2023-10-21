main(argc,argv)
	int argc;
	char *argv[];
	{
	int foo;
	foo=0;
	while((argv[1][foo])!=(0)){
	printf("   %c is %o octal, %d decimal, ",argv[1][foo],argv[1][foo],argv[1][foo]);
	printf("and ");
	basout(argv[1][foo++],16);
	printf(" hex.\n");
	}
}
