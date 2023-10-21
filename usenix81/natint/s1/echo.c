/*
 * Echo args to stdout
 * decoding usual syntax for non-printing chars
 */
main(argc,argv) char **argv;{
	register int i;
	for(i=1; i<argc; i++){
		if(i>1) putchar(' ');
		putstr(argv[i]); }
	putchar('\n'); }

putstr(s) char *s;{
	char c;
	while(*s){
		s=+ cscan(s, &c);
		putchar(c); }
	} 
