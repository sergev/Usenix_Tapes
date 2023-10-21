	extern	fout;
main(argc,argv)
	int	argc;	char	**argv;
{
	register i,j;
	register char c;
/*	this program fills the page with a given charactor.	*/

	fout = dup(1);
	c = argv[1][0];
	for(i = 0;i != 24;i++){
		putchar('\n');
		for(j = 0; j != 79; j++){
			putchar(c);
		}
	}
	putchar('\n');
	flush();
}
