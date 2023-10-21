main(argc,argv)
int argc;
char **argv;
{
	extern int fout;
	int i,len,fc;
	register j,nc;
	fout = dup(1);
	if(argc == 2)
		len=atoi(argv[1]);
	else
		len=79;
	fc = ' ';
	for(i=0; i<200; i++) {
		if(++fc == 0177) fc = ' ';
		nc = fc;
		for(j=0; j<len; j++) {
			putchar(nc);
			if(++nc == 0177) nc = ' ';
		}
		putchar('\n');
	}
	flush();
}
