	extern	fin,fout;
main(argc,argv)
	int argc;	char **argv;
	{
	int foo,bar;

/*	This program filters out blank lines in a file	*/

	if(argc == 2){
		bar = open(argv[1],0);
		if(bar == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = bar;
		}
	    else fin = dup(0);
	fout = dup(1);
	bar=1;
	while((foo=getchar())!=('\0')){
	    if((foo!='\n')||(bar!='\n')){
			bar=(putchar(foo));
	    }
	}
	flush();
}
