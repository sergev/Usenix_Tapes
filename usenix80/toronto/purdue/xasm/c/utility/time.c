	char *foo[]{
	" - | || ||_|",
	" |  |  |  | ",
	" - ' | - |_ ",
	"--   |-- __|",
	" /|/ |--|  |",
	" - |   -  _|",
	" - |   - |_|",
	"---  / |  | ",
	" - | | - |_|",
	" - | | -  _|",
	"            ",
	"    o  o    ",
	};
	extern	fout;

main(argc,argv)
	int argc;
	char **argv;
{
	int	up,down,over,back;
	int	clear,equals,esc,home,bell;
	int sh,sl,mh,ml,hh,hl,dp,mdp;
	int x79,x51,y16,y9;
	int *bar;
	int	tvec[2];
	int	ltime,dtime;
	int wait;
	register i,j;
	register char *p;

	shell("stty nl");
	fout = dup(1);

	clear = 26;	up = 11;	down = 10;
	over = 12;	back = 8;	equals = 61;
	esc = 27;	home = 30;	bell = 7;
	x79 = 110;	y16 = 55;	x51 = 83;
	y9 = 41;

	time(tvec);
	ltime = tvec[1];
	p = ctime(tvec);
	p=+ 11;
	hh = *p++ -060;
	hl = *p++ - 060;
	p++;
	mh = *p++ - 060;
	ml = *p++ - 060;
	p++;
	sh = *p++ - 060;
	sl = *p++ - 060;

	printf("%c%c",down,015);
	printf("         ");
	pnum(hh);	rone();
	pnum(hl);	rone();
	pnum(11);
	rone();	pnum(mh);	rone();
	pnum(ml);	rone();
	pnum(11);
	rone();	pnum(sh);	rone();
	pnum(sl);
	for(i = 0;i != 5;i++)putchar(down);
	putchar(015);
	flush();
	shell("stty -nl");
}


pnum(n)
	int n;
{
	register i,j;
	int down,over,back,up;
	register char *k;

	down = 10;	up = 11;	back = 8;
	over = 12;
	putchar(over);
	k = foo[n];
	for(i=0;i!=3;i++)putchar(*k++);
	printf("%c%c%c%c",down,back,back,back);
	for(i=0;i!=3;i++)putchar(*k++);
	printf("%c%c%c%c",down,back,back,back);
	for(i=0;i!=3;i++)putchar(*k++);
	printf("%c%c%c%c",down,back,back,back);
	for(i=0;i!=3;i++)putchar(*k++);
	printf("%c%c%c%c%c%c%c%c",down,back,back,back,up,up,up,up);
	putchar(back);
}

lone()
{
	printf("%c%c%c%c%c%c%c%c",11,8,8,8,8,8,8,10);
}

rone()
{
	printf("%c%c%c%c%c%c%c%c",11,12,12,12,12,12,12,10);
}
