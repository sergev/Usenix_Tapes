	char *foo[]{
	" - | || |'_'",
	" |  |  |  | ",
	" - ' | - |_ ",
	"--   |-- __|",
	" /|/ |--|  |",
	" - |   -  _|",
	" - |   - |_|",
	"---  / |  | ",
	" - | | - '_'",
	" - | | -  _|",
	"            ",
	"    o  o    ",
	};

	int park;
	int bonzo();
	extern	fout;

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
	register i,j;
	register char *p;

	signal(2,bonzo);

	if(argc == 2){
		wait = argv[1][0] - 060;
		}
	   else wait = 1;

	shell("stty nl");
	fout = dup(1);

	clear = 26;	up = 11;	down = 10;
	over = 12;	back = 8;	equals = 61;
	esc = 27;	home = 30;	bell = 7;
	x79 = 110;	y16 = 55;	x51 = 83;
	y9 = 41;	x1 = 040;	y1 = 040;
	park = 0;

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

	printf("%c%c%c",clear,home,down);
	for(i=0;i!= 8;i++)putchar(down);
	printf("         ");
	pnum(hh);	rone();
	pnum(hl);	rone();
	pnum(11);
	rone();	pnum(mh);	rone();
	pnum(ml);	rone();
	pnum(11);
	rone();	pnum(sh);	rone();
	pnum(sl);
	i = 0;
	j = 0;
 loop:
	parkit();	flush();
	if(wait)sleep(wait);
	time(tvec);
	dtime = tvec[1] - ltime;
	ltime = tvec[1];
	printf("%c%c%c%c",esc,equals,y9,x51);
	mdp = 0;
  while(dtime--){
	dp = 0;
	sl++;
	if(sl == 10){
	    sl = 0;
	    sh++;
	    dp++;
	    if(sh == 6){
		sh = 0;
		ml++;
		dp++;
		    if(ml == 10){
			ml = 0;
			mh++;
			dp++;
			if(mh == 6){
			    mh = 0;
			    hl++;
			    dp++;
			    if(hl == 10){
				hl = 0;
				hh++;
				dp++;
				}
				else if((hl == 4)&&(hh == 2)){
				    hh = 0;	hl = 0;
				    dp++;
				    }
			    }
			}
		    }
		}
	if(dp > mdp)mdp = dp;
       }
	dp = mdp;
	j = dp;
	while(j){lone();j--;}
	if(dp > 1)lone();
	if(dp > 3)lone();
	if(dp == 5){pnum(hh);rone();}
	if(dp >= 4){pnum(hl);rone();rone();}
	if(dp >= 3){pnum(mh);rone();}
	if(dp >= 2){pnum(ml);rone();rone();}
	if(dp >= 1){pnum(sh);rone();}
	pnum(sl);
	goto loop;
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

parkit()
{

	int esc,equals,x1,y1,x79,y16;
	esc = 27;	equals = 61;	x1 = y1 = 040;
	x79 = 110;	y16 = 55;
	switch((park++) % 4){
		case 0:
			printf("%c%c%c%c",esc,equals,y1,x1);
			break;

		case 1:
			printf("%c%c%c%c",esc,equals,y1,x79);
			break;

		case 2:
			printf("%c%c%c%c",esc,equals,y16,x79);
			break;

		case 3:
			printf("%c%c%c%c",esc,equals,y16,x1);
			break;
		}
}

bonzo()
{
	shell("stty -nl");
	exit();
}
