	extern	fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register sc;
	register char c;
	register i;

/*	This program takes spaces/tabs clusters and turns them
	into one tab.  It is good for files from PUCC where there are
	spaces to get into a given field, and the users wants tabs
	there.							*/

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
    c = getchar();
    while(c){
	sc = 0;
	if(c == '\t'){putchar(c); c = getchar();goto loop;}
	if(c == ' '){
		while(c == ' ')c = getchar();
		putchar('\t');
		}
  loop: while((c != '\n')&& c){
		if(c == ' '){
		    if(sc){
			while(c == ' '){
			    c = getchar();
			    }
			putchar('\t');
			sc = 0;
			}
		      else {
			sc++;
			c = getchar();
			}
		    goto loop;
		    }
	    if(sc){
		putchar(' ');
		}
	    sc = 0;
	    putchar(c);
	    c = getchar();
	    }
	if(c)putchar(c);
	c = getchar();
	}
	flush();
}
