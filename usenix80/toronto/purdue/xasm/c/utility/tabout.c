	extern fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register i,j;
	register char c;
	char c1;

/*	This program takes a file and tabs it out by one tab	*/
	if(argc == 2){
		i = open(argv[1],0);
		if(i == -1){
			printf("Can't read %s.\n",argv[1]);
			return;
			}
		fin = i;
		}
	    else fin = dup(0);
	fout = dup(1);
	j = 1;
	while(c = getchar()){
		if(c == '\n'){
			j = 0;
			while((c = getchar()) == '\n')putchar('\n');
			if(c == '\0'){
				putchar('\n');
				flush();
				return;
				}
			printf("\n\t");
			}
		if(j){printf("\t%c",c);j = 0;}
		   else putchar(c);
		}
	flush();
}
