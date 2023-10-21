	char	page[81][25];
	int fool;
	int fout,fin;

main(argc,argv)
	int argc;	char **argv;
{
	register x,y,k;
	int tvec[2];
	char	c;
	int done,x1,y1;
	int count;
	int ifd;		/* tty input file descriptor */
	int heof;		/* = 1 if have end of file */

	fout = dup(1);
	fool = 0;
	heof = 0;

	if(argc == 2){
		x = open(argv[1],0);
		if(x == -1){
			printf("Can't read '%s'.\n",argv[1]);
			return;
			}
		fin = x;
		}

	shell("stty nl");
	time(tvec);
	srand(tvec[1]);

npage:	for(x = 0;x != 80;x++)
	    for(y = 0;y != 24;y++)page[x][y] = ' ';

	x = 0;	y = 0;
	while((c = getchar()) && (y < 23)){
		if((c == '\n') || (c == '\r')){
			y++;	x = 0;
			}
		else {if(c == '\t'){
			x =+ 010;
			x =& 0370;
			if(x > 79)x--;
			}
		else {
			if(c < 32)c = ' ';
			page[x][y] = c;
			x++;
			if(x > 79)x--;
			}
		    }
		}

	if(c == '\0')heof = 1;
	putchar(26);

	while(1){
		if((fool % 200) == 0){
			flush();
			done = 0;
			x =+ x%13;	y =+ x%3;
			for(x1 = 0;x1 != 80;x1++)
			    for(y1 = 0;y1 != 24;y1++)
				if(page[x1][y1] != ' ')done++;
			if(done == 0){
				lsimov(0,22);
				flush();
				if(heof){
					shell("stty -nl");
					flush();
					return;
					}
				read(2,&c,1);
				goto npage;
				}
			}
		k = rand();
		count = k;
		count =>> fool % 4;
		count =& 7;
		k =>> fool++ % 9;
		k =& 7;
		switch(k){
			case 0:
				y =+ count;
				break;
			case 1:
				x =+ count + 1;
				y =+ count;
				break;
			case 2:
				x =+ count;
				break;
			case 3:
				x =+ count - 3;	y =- count;
				break;
			case 4:
				y =- count;
				break;
			case 5:
				y =- count;	x =- count;
				break;
			case 6:
				x =- count;
				break;
			case 7:
				x =- count;	y =+ count;
				break;
			}
		if(x > 80)x =% 80;
		if(x < 0)x = 80;
		if(y > 22)y =% 23;
		if(y < 0)y = 22;
		if(page[x][y] != ' '){
			lsimov(x,y);
			putchar(page[x][y]);
			page[x][y] = ' ';
			}
		}
}

lsimov(x,y)
	int x,y;
{
	putchar('\033');	putchar('=');
	putchar(y + 040);	putchar(x + 040);
}
