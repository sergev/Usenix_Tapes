	char	page[128][128];
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


	putchar(033);	putchar(':');	putchar('3');
	putchar(033);	putchar(':');	putchar('$');
	time(tvec);
	srand(tvec[1]);

npage:	for(x = 0;x != 128;x++)
	    for(y = 0;y != 128;y++)page[x][y] = 0;


	while(1){
		if((fool % 200) == 0){
			flush();
			done = 0;
			x =+ x%13;	y =+ x%3;
			for(x1 = 0;x1 != 128;x1++)
			    for(y1 = 0;y1 != 128;y1++)
				if(page[x1][y1] != 1 )done++;
			if(done == 0){
				flush();
				return;
				}
			}
		k = rand();
		count = k;
		count =>> fool % 4;
		count =& 037;
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
		if(x > 127)x =% 127;
		if(x < 0)x = 127;
		if(y > 127)y =% 128;
		if(y < 0)y = 127;
		if(page[x][y] == 0){
			point(x,y,1,'3');
			page[x][y] = 1;
			}
		}
}

point(x,y,onoff,page)
	int x,y,onoff;	char page;
{
	register char c;

	c = (y >> 3) & 017;
	c =+ ' ';
	putchar(033);	putchar('<');
	putchar(c);
	putchar(' ' + (y & 7));
	c = (x >> 3) & 017;
	c =+ ' ';
	putchar(c);
	putchar(' ' + (x & 7));
	putchar(onoff + ' ' + (((page - '1') << 1) & 6));
}
