	extern	fout;
main(argc,argv)
	int argc;	char **argv;
{
	register x,y,k;
	int tvec[2];


	shell("stty nl");
	fout = dup(1);
	putchar(26);
	time(tvec);
	srand(tvec);
	x = 40;	y = 12;
	while(1){
		k = rand();
		k =>> 6;
		k =& 7;
		switch(k){
			case 0:
				y++;
				break;
			case 1:
				x =+ 2;
				y++;
				break;
			case 2:
				x =+ 2;
				break;
			case 3:
				x =+ 2;	y--;
				break;
			case 4:
				y--;
				break;
			case 5:
				y--;	x =- 2;
				break;
			case 6:
				x =- 2;
				break;
			case 7:
				x =- 2;	y++;
				break;
			}
		if(x > 77)x = 0;
		if(x < 0)x = 77;
		if(y > 23)y = 0;
		if(y < 0)y = 23;
		printf(" \b");
		lsimov(x,y);
		printf("*\b");
		if(argc != 1)sleep(1);
		}
}

lsimov(x,y)
	int x,y;
{
	putchar('\033');	putchar('=');
	putchar(y + 040);	putchar(x + 040);
}
