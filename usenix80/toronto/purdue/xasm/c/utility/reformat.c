	extern fin,fout;
main(argc,argv)
	int argc;	char **argv;
{
	register char c;
	register i,x;
	int fd;

	fout = dup(1);
	if(argc == 2){
		fd = open(argv[1],0);
		if(fd == -1){
			printf("Can't find '%s'.\n",argv[1]);
			return;
			}
		fin = fd;
		}
	    else fin = dup(0);

next:	x = 0;
	while(x < 18){
		c = getchar();
		if(c == '\0'){
			putchar(c);
			flush();
			return;
			}
		if(c == '\n'){
			putchar(c);
			goto next;
			}
		if(c == '\t')x = tab(x);
		    else x++;
		putchar(c);
		}
	c = getchar();
	if(c < '!'){
		if(c == '\n'){
			putchar(c);
			goto next;
			}
		while((c = getchar()) < '!'){
			if(c == '\n'){
				putchar(c);
				goto next;
				}
			}
		}
	    else {
		putchar(c);
		while((c = getchar()) > ' '){
			putchar(c);
			x++;
			}
		if(c == '\n'){
			putchar(c);
			goto next;
			}
		while((c = getchar()) < '!'){
			if(c == '\n'){
				putchar(c);
				goto next;
				}
			}
		x++;
		}
	while(x < 29){
		putchar(' ');
		x++;
		}

	putchar(c);
	x = 31;
	while((c = getchar()) != '\n'){
	    if(c == '\t'){
		i = 8 - ((x-30) % 8);
		x =+ i;
		while(i--)putchar(' ');
		}
	  else {
		putchar(c);
		x++;
		}
	    }
	putchar(c);
	goto next;

}
tab(pos)
	int pos;
{
	pos =& ~7;
	pos =+ 8;
	return(pos);
}
