	extern	fout;
	char *gp,*rp;
main(argc,argv)
	int argc;
	char **argv;
{
	int	up,down,over,back;
	int	clear,equals,esc,home,bell;
	int sh,sl,mh,ml,hh,hl,dp,mdp;
	int x79,x51,y16,y9,y1,x1;
	int *bar;
	int	tvec[2];
	int	ltime,dtime;
	int wait;
	register x,y;
	register i;

	shell("stty nl");
	clear = 26;	up = 11;	down = 10;
	over = 12;	back = 8;	equals = 61;
	esc = 27;	home = 30;	bell = 7;
	x79 = 110;	y16 = 55;	x51 = 83;
	y9 = 41;	x1 = 040;	y1 = 040;

	fout = dup(1);
	putchar(clear);
	putchar(home);
	rp = gp = argv[1];
	x = 78;	y = 23;

	while(y > 0){
		for(i = x;i;i--)putit();
		for(i = y;i;i--){
			putit();
			printf("%c%c",back,down);
			}
		y--;
		for(i = x;i;i--){
			putit();
			printf("%c%c",back,back);
			}
		for(i = y;i;i--){
			putit();
			printf("%c%c",back,up);
			}
		putit();
		x =- 2;
		y--;
		}
	shell("stty -nl");
}
putit()
{
	register char c;

	if(*gp)putchar(*gp++);
	    else {
		gp = rp;
		putchar(*gp++);
		}
}
