	extern	fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i,j;

	if(argc == 2){
		i = open(argv[1],0);
		if(i == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = i;
		}
	    else fin = dup(0);
	fout = dup(1);
	c = 1;
loop:	while(c){

	c = getchar();
	if(c == '*'){
		while((c != '\n') && c)c = getchar();
		goto loop;
		}
	if(c == '\0')goto loop;
	if(c == '\n'){
		printf("	db	0\n");
		goto loop;
		}
	printf("	db	000");
	for(i=0;i!=5;i++){
		if(c == '*'){
			while((c != '\n')&& c)c = getchar();
			goto loop;
			}
		if(c == '\n'){
			while(i != 5){
				putchar('0');
				i++;
				}
			putchar(c);
			goto loop;
			}
		if(c == ' ')putchar('0');
		   else putchar('1');
		c = getchar();
		}
	while((c != '\n')&& c)c = getchar();
	putchar(c);
	}
	putchar('\n');
	flush();
}
