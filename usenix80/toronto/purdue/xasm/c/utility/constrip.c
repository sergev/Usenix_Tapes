	extern fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	int foo;

/*	This program strips out nasty control charactors from a file	*/

	if(argc == 2){
		foo = open(argv[1],0);
		if(foo == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = foo;
		}
	    else fin = dup(0);
	fout = dup(1);
	while((foo=getchar())!='\0'){
	if((foo=='\t')||(foo>=' ')||(foo=='\n'))putchar(foo);
	}
	flush();
}
