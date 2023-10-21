#define NULL ((char *)0)
main(argc, argv)
	char **argv;
{
	static int ll = 65;	/* line length */
	static buf[100];

	if(argc > 1 && **++argv == '-' && argv[0][1] == 'w')
			ll = atoi(*++argv);
	while(gets(buf) != NULL)
		printf("%*s%s\n", (ll-strlen(buf))/2, "", buf);
}
