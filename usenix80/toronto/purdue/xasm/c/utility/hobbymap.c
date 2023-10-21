	extern fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i,x;
	char save;
	int fd;

	if(argc == 2){
		fd = open(argv[1],0);
		if(fd == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = fd;
		}
	    else fin = dup(0);
	fout = dup(1);


next:
    while(c = getchar()){
	if(c == '*'){putchar(c);	goto dumpit;}
	if(c == '\n'){putchar(c);	goto next;}
	if((c == ' ')||(c == '\t'))putchar('\t');
	    else {
		putchar(c);
		while((c = getchar()) > ' '){
		    putchar(c);
		    }
		putchar('\t');
		}
	while(((c = getchar()) == ' ')||(c == '\t'));
	putchar(c);
	if(c == '\n')goto next;
	while((c = getchar()) > ' ')putchar(c);	/* print instruction */
	while((c == ' ')||(c == '\t'))c = getchar();
	if(c == '\n'){putchar(c);	goto next;}
	if((c == 'a')||(c == 'b')){
	    save = c;
	    if(((c = getchar()) == ' ')||(c == '\t')||(c == '\n')){
		if(c == '\n'){
			printf(" %c\n",save);
			goto next;
			}
		printf(" %c\t",save);
		while(((c = getchar()) == ' ')||(c == '\t'));
		putchar(c);
		if(c == '\n')goto next;
		}
	      else {
		printf("\t%c%c",save,c);
		if(c == '\n')goto next;
		}
	    }
	  else printf("\t%c",c);
	while(((c = getchar()) != ' ')&&(c != '\t')){
		if(c == '\n'){putchar(c);goto next;}
		putchar(c);
		}
	putchar('\t');
dumpit:	while(((c = getchar()) != '\n')&& c)putchar(c);
	putchar('\n');
	}
	flush();
}
