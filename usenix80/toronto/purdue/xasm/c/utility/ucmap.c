	extern	fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	int foo;

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
	if((foo>='a')&&(foo<='z'))foo=foo-('a'-'A');
	putchar(foo);
	}
	flush();
}
