#include "/usr2/wa1yyn/c/plot/plot.h"
#define BLKSIZE	512	/* disk block size (bytes) */
#define EOF	-1
#define FNSIZE	64	/* max size of pathname to file */
#define LBSIZE	512	/* max line length */
#define MAXERR	50	/* highest error number */
#define READ	0
#define SIGINT	2	/* Interrupt signal */
#define SIGQIT	3	/* Quit signal */
#define WRITE	1

#define ECHO 010
#define RAW 040
#define M 2
#define D 3
#define C 4
#define REFX 5160
#define REFY 3880
	extern boxdata[];
	char plotob[600];
	int pstat[20];
	int field;		/* current field	*/
	int place[3];		/* [0] = x, [1] = y	*/
	int lastep[3];		/* Last end point of a line */
	char crossterm;		/* croshair terminate char	*/
	char rotdir;		/* rotation direction	*/
	char rotscale;		/* scale factor		*/
	char command[200];	/* current command	*/
	char boxname[200];	/* current box name	*/
	int aposit;		/* a/n cursor position	*/
	int apositx;		/* a/n x cursor positon	*/
	int ttybuf[3],savebuf[3]; /* half/full dup tty buf*/

	char *boxkind[]{	/* kinds of boxes	*/
		"#42fE2z",	/* dummy string */
		"square",
		"nose",
		"and2",
		"nand2",
		"and3",
		"nand3",
		"and4",
		"nand4",
		"buf",
		"inv",
		"tbuf",
		"tinv",
		"or2",
		"nor2",
		"xor2",
		"ff",
		"trail",
		"lead",
		"ffp",
		"hyst",
		"arrow",
		"feather",
		"bi",
		"d",
		"res",
		"cap",
		"var",
		"bat",
		"npn",
		"pnp",
		"t",
		"mu",
		"ohm",
		"sigma",
		"int",
		"four",
		"diode",
		"zen",
		"ground",
		"earth",
		"photon",
		"fuse",
		"light",
		"coax",
		"varactor",
		"triac",
		"coil",
		"relay",
		"ind",
		"slug",
		"wiper",
		"ff7473",
		"ff7474",
		"ff7476",
		"ant",
		"crystal",
		"bus",
		"mud",
		"dot",
		"ic6",
		"ic8",
		"ic14",
		"ic16",
		"ic18",
		"ic24",
		"ic28",
		"ic40",
		"hop",
		"title",
		"circle",
		"diamond",
		"fet",
		"fetenh",
		-1
		};



psetup()
{

	pstat[HPBS] = plotob;
	pstat[DEBUG] = 1;
	plotsel(pstat,TEK);
	plotinit(pstat);
	pstat[DEBUG] = 0;
	lastep[0] = lastep[1] = 330;
	aposit = 7000;
	apositx = 160;
	crossterm = '\n';
	rotdir = '0';
	rotscale = '1';

	gtty(0,savebuf);	/* save original status	*/
	gtty(0,ttybuf);	/* play buffer		*/
	ttybuf[2] =& ~ECHO;	/* Echo off		*/
/*	ttybuf[2] =| RAW;	/* Raw mode on		*/

	plotpen(pstat,BLACK);
	place[0]=1000;place[1]=1000;

}
	extern	*zero,*dot,*dol;
getit()
{
	register char c;
	register char *p,*k;
again:	if(crossterm == ' '){
		c = 'p';
		goto wire;
		}
	c = getchar();
	if(c == 'j'){
		eatnl();
		if(zero == dol){
			printf("No line to adjust\n");
			foobar();
			goto again;
			}
		p = getline(*dot);
		k = command;
		while(*k++ = *p++);
		alcom();
		*dot = putline(command);
		foobar();
		goto again;
		}
	if(c == '/'){
		findit();
		goto again;
		}
wire:	getcom(c);
	if(crossterm == 'n'){
		foobar();
		goto again;
		}
	plotcom();
	foobar();
}

getcom(cm)
	char cm;
{
	register x,y;			/* gets a command line	*/
	register char c;		/* and puts it in 'command' */
	char *p,*k;
	int from[3],to[3];


	if(cm == 'r'){
		ruler();
		foobar();
		getcom(getchar());
		return;
		}
	if(cm == 'g'){			/* make work grid	*/
		while(getchar() != '\n');
		gridit();
		foobar();
		getcom(getchar());
		return;
		}
	p = command;
	*p++ = cm;
	*p = '\0';

    switch(cm){
	case 'w':			/* draw a 'W'ire	*/
		eatnl();
		getcross(from);
		hicke(from);
morelin:	getcross(to);
		if(crossterm == 'j'){
			place[0] = from[0];	place[1] = from[1];
			align();
			from[0] = place[0];	from[1] = place[1];
			goto morelin;
			}
		lastep[0] = to[0]; lastep[1] = to[1];
		hicke(to);
		make(from,p);
		seke(&p);
		*p++ = ':';
		make(to,p);
		seke(&p);
		*p++ = '\n';
		break;

	case 'z':			/* draw a 'Z'arrallagram	*/
		eatnl();
		getcross(from);
		hicke(from);
moreli:	getcross(to);
		if(crossterm == 'j'){
			place[0] = from[0];	place[1] = from[1];
			align();
			from[0] = place[0];	from[1] = place[1];
			goto moreli;
			}
		lastep[0] = to[0]; lastep[1] = to[1];
		make(from,p);
		seke(&p);
		*p++ = ':';
		make(to,p);
		seke(&p);
		*p++ = '\n';
		break;
	case 'c':			/* put a charactor	*/
		c = getchar();
		while(getchar() != '\n');
		getcross(to);
		make(to,p);
		seke(&p);
		*p++ = '-';
		*p++ = c;
		*p++ = '\n';
		break;

	case 'b':			/* draw a 'B'ox		*/
		c = getchar();
		k = boxname;
		if((c < '0') || (c > '7')){
			rotdir = '0';
			*k++ = c;
			goto boxno;
			}
		   else {
			rotdir = c;
			}
		c = getchar();
		if((c < '0') || (c > ':')){
			rotscale = '1';
			*k++ = c;
			goto boxno;
			}
		    else {
			rotscale = c;
			}
boxno:		while((*k++ = getchar()) != '\n');
		--k;	*k = '\0';
		putchar(ESC);	putchar(SUB);
		restart();
		move(pstat,REFX,REFY);
		putbox(boxname);
		putchar('\n');
		getrc(to);
		make(to,p);
		seke(&p);
		*p++ = '-';
		*p++ = rotdir;
		*p++ = rotscale;
		k = boxname;
		while(*p++ = *k++);
		p--;	*p++ = '\n';
		break;

	case 's':			/* 'S'tring plot	*/
		c = getchar();
		x = c;
		k = boxname;
		if((c >= '0') && (c <= '4')){
			}
		    else {
			*k++ = c;	c = x = '1';
			}
		while((*k++ = getchar()) != '\n');
		--k;
		*k = '\0';
		restart();
		move(pstat,5130,3860);
		goalpha(pstat);
		setchr(pstat,x);
		printf("%s\n",boxname);
		getrc(to);
		make(to,p);
		seke(&p);
		*p++ = '-';	*p++ = c;
		k = boxname;
		while(*p++ = *k++);
		p--;	*p++ = '\n';
		break;

	case 'p':			/* 'P'ath 		*/
		if(crossterm != ' ')eatnl();
		from[0] = lastep[0];
		from[1] = lastep[1];
		command[0] = 'w';
		goto morelin;
		break;

	case '.':			/* get out of input mode */
		command[0] = '\0';
		crossterm = '\n';
		eatnl();
		break;

	case 'v':			/* set vector type */
		if((c = getchar()) == '\n')*p++ = '`';
		else *p++ = c;
		if(c != '\n')eatnl();
		*p++ = '\n';
		break;

	case 't':			/* pen select	*/
		if((c = getchar()) == '\n')*p++ = '1';
		else *p++ = c;
		if(c != '\n')eatnl();
		*p++ = '\n';
		break;

	case '*':			/* comment	*/
		while((*p++ = getchar()) != '\n');
		break;

	case 'm':			/* 'M'ud dot */
		eatnl();
		getcross(to);
		make(to,p);
		seke(&p);
		*p++ = '\n';
		break;

	}
	*p = '\0';
	goalpha(pstat);
	rotdir = '0';
	rotscale = '1';
	if(command[0] != 'w')crossterm = '\n';
}

plotcom()				/* plots 'command'	*/
{
	register char *p,c;
	register x,y;
	int x2,y2,savdeb;
	char *glub,temp[200],*k,**ss;
	char temp2[20];

	glub = command;	k = temp;
	while((*glub) && (*glub != '\n'))*k++ = *glub++;
	*k = '\0';

	glub = temp;
	c = *glub++;
	ss = &glub;

	savdeb = pstat[DEBUG];
	pstat[DEBUG] = 0;

    switch(c){

	case 'w':			/* draw 'W'ire		*/
		setup(ss);
		sscan(temp2,ss);
		x = basin(temp2,10);
		sscan(temp2,ss);
		y = basin(temp2,10);
		draw(pstat,x,y);
		break;

	case 'z':			/* draw a 'Z'arralagram	*/
		sscan(temp2,ss);
		x = basin(temp2,10);
		sscan(temp2,ss);
		y = basin(temp2,10);
		sscan(temp2,ss);
		x2 = basin(temp2,10);
		sscan(temp2,ss);
		y2 = basin(temp2,10);
		move(pstat,x,y);
		draw(pstat,x,y2);
		draw(pstat,x2,y2);
		draw(pstat,x2,y);
		draw(pstat,x,y);
		break;

	case 'c':			/* plot 'C'haractor	*/
		setup(ss);
		(*ss)++;
		plotc(pstat,**ss);
		break;

	case 'b':			/* plot 'B'ox		*/
		setup(ss);
		p = boxname;
		k = (*ss)++;
		k++;
		rotdir = *k++;
		rotscale = *k++;
		while(*p++ = *k++);
		putbox(boxname);
		rotdir = '0';
		rotscale = '1';
		break;

	case 's':			/* plot 'S'tring	*/
		setup(ss);
		(*ss)++;
		c = *(*ss)++;
		setchr(pstat,c);
		plots(pstat,*ss);
		setchr(pstat,'1');
		break;

	case 'v':				/* set vector type */
		setvec(pstat,**ss);
		break;

	case 't':			/* pen select	*/
		plotpen(pstat,**ss);
		break;

	case '\n':			/* blank command	*/
		break;

	case '*':			/* comment	*/
		break;

	case 'm':			/* 'M'ud dot */
		setup(ss);
		drawr(pstat,-10,0);
		drawr(pstat,10,10);
		drawr(pstat,10,-10);
		drawr(pstat,-10,-10);
		drawr(pstat,-10,10);
		break;

	}
	pstat[DEBUG] = savdeb;
}

putbox(name)
	char *name;
{
	register i,*ii;
	int denote,x,y;

	denote = pstat[DEBUG];
	pstat[DEBUG] = 0;

	i = llu(name,boxkind);
	if(i == -1){
		putchar('\n');
		reclear();
		foobar();
		goalpha(pstat);
		pstat[DEBUG] = denote;
		printf("Unknown symbol- '%s'.\n",name);
		return;
		}

	i--;
	ii = boxdata;
	while(i && *ii){
		while((*ii != 1) && *ii)ii =+ 3;
		ii =+ 3;
		i--;
		}
	if(*ii == 0){
		goalpha(pstat);
		pstat[DEBUG] = denote;
		printf("Symbol not in table- '%s'.\n",name);
		return;
		}
	while((*ii != 1) && *ii){
	    switch(*ii++){
		case 2:					/* move	*/
			x = *ii++;
			y = *ii++;
			rotate(&x,&y);
			mover(pstat,x,y);
			break;
		case 3:					/* draw	*/
			x = *ii++;	y = *ii++;
			rotate(&x,&y);
			drawr(pstat,x,y);
			break;
		case 4:
			x = *ii++;	y = *ii++;
			goalpha(pstat);
			plotc(pstat,y);
			break;
		}
	}
	pstat[DEBUG] = denote;
}


restart()
{
	putchar(ESC);
	printf("=1");
}
reend()
{	putchar('\n');
}
reclear()
{
	putchar(ESC);	printf("=0");
	setchr(pstat,'1');
	flush();
}
restore()
{
	putchar(ESC);	printf("=3");
	getchar(0);		/* flush line terminator	*/
}
setup(ss)
	int ss;
{
	register x,y;
	char temp[20];

	sscan(temp,ss);
	x = basin(temp,10);
	sscan(temp,ss);
	y = basin(temp,10);
	if((x == pstat[X]) && (y == pstat[Y]))return;	/* dont
					move if already there*/
	move(pstat,x,y);
}
make(loc,p)				/* put x,y locations into  */
	int *loc;	char *p;	/* into string		*/
{
	basesp(loc[0],p,10);
	seke(&p);
	*p++ = ',';
	basesp(loc[1],p,10);
}
seke(p)					/* seek 'p' to end of string */
	int **p;
{
	char *q;

	q = *p;			/* have *char	*/
	while(*q != '\0')q++;		/* find end of string	*/
	*p = q;			/* store it in pointer	*/
}
hicke(loc)				/* put a hicke at current */
	int loc[2];			/* location on paper	*/
{
	move(pstat,loc[0] - 30,loc[1]);
	drawr(pstat,30,30);
	drawr(pstat,30,-30);
	drawr(pstat,-30,-30);
	drawr(pstat,-30,30);
	mover(pstat,30,0);
}

getcross(place)				/* read in the crosshairs */
	int *place;
{
	register x,i;
	register char c;

	stty(0,ttybuf);	/* go no echo	*/
	putchar(ESC);	putchar(SUB);
	flush();
	crossterm = getchar();
	place[0] = getcrxx();
	place[1] = getcrxx();
	getchar();		/* eat up \n terminator	*/
	stty(0,savebuf);	/* go full dup	*/
	putchar(7);
}
getcrxx()
{
/* Note that the resolution of the crosshairs is 512 lines/page.
  This is to aid with allignment. 				*/
	register x,temp;

	x = ((getchar() & 037) << 5);
	temp = getchar() & 036;
	if(temp & 2)temp =+ 2;
	x =+ temp;
	return(x * 10);
}
getrc(place)				/* read in crosshairs */
	int *place;			/* with refresh mem.  */
{					/* running	    */
	int i;
	register char c;
	char pos;		/* user must prompt with n */
	stty(0,ttybuf);	/* go no echo	*/
	crossterm = '\n';
	flush();
/* Wait for request for crosshair postion read		*/
	while((c = getchar()) != '\n'){
		if(c == 'n')crossterm = c;
		}
	reclear();
	putchar(ESC);	putchar(SUB);
	for(i = 0;i != 19;i++)putchar(1);
	putchar(ESC);	putchar(ENQ);	/* request cursor position */
	flush();
	place[0] = getcrjs();
	place[1] = getcrjs();
	place[0] =- (REFX/2);
	place[1] =- (REFY/2);
	place[0] =* 2;
	place[1] =* 2;
	place[1] =/ 10;		place[0] =/ 10;
	place[1] =& 037776;	place[0] =& 037776;
	if(place[0] & 2)place[0] =+ 2;
	if(place[1] & 2)place[1] =+ 2;
	place[0] =* 10;		place[1] =* 10;
	place[0] =+ 80; place[1] =+ 40;	/* funny hardware corections */
	if((pos = getchar()) != '\n'){reclear();printf("I am out of sync'%c'.\n",pos);}
	stty(0,savebuf);	/* go with echo	*/
	reclear();
}
getcrjs()
{
/* Note that the resolution of the joystick is 256 lines/page.
  This is to aid with allignment and correct for joystick drift. */
	register x,temp;

	x = ((getchar() & 037) << 5);
	temp = getchar() & 037;
	x =+ temp;
	return(x * 10);
}
foobar()
{
	move(pstat,apositx,aposit =- 130);
	if(aposit < 0){
		aposit = 7400;
		apositx =+ 2500;
		}
	goalpha(pstat);
	flush();
}
rotate(x,y)
	int *x,*y;
{
	register xt,yt,fac;
	char temp;

	temp = rotdir;
	if(rotdir > '3'){
		*y = -*y;
		temp =- 4;
		}
    switch(temp){
	case '0':					/* no rotate	*/
		xt = *x;	yt = *y;
		break;
	case '1':					/* -90 deg.	*/
		xt = *y;
		yt = -*x;
		break;
	case '2':					/* 180 deg.	*/
		xt = -*x;
		yt = *y;
		break;
	case '3':					/* +90 deg.	*/
		xt = -*y;
		yt = *x;
		break;
	}
	if(rotscale == '0'){
		*x = xt / 2;
		*y = yt / 2;
		}
	    else {
		fac = rotscale - '0';
		*x = xt * fac;
		*y = yt * fac;
		}
}
ruler()
{
	register n,x,y;
	int x1,y1;
	int to[3],linit;
	char c,temp[22];

	linit = 0;
	c = getchar();
	if(c == 'l'){
		linit = 1;
		c = getchar();
		}
	if(c == 'i'){
		linit = 2;
		c = getchar();
		}
	if(c == 'r')goto rulread;
	n = 0;	temp[n++] = c;
	while((c = getchar()) != '\n')temp[n++] = c;
	temp[n] = '\0';
	n = basin(temp,10);
/* n has # of points	*/
	if(n < 2)return;
	n--;
	getcross(to);
	move(pstat,to[0],to[1]);
	hickx();
	x1 = to[0];	y1 = to[1];
	getcross(to);
	x = to[0] - x1;	y = to[1] - y1;
	move(pstat,x1,y1);
	if(linit == 2){
		x = x/n;	y = y/n;
		}
	while(n--){
		if(linit == 1)
			drawr(pstat,x,y);
		    else
			mover(pstat,x,y);
		hickx();
		}
	move(pstat,x1,y1);
	crossterm = '\n';
	return;
rulread:
	scanin(temp);	x1 = basin(temp,10) * 40;
	scanin(temp);	y1 = basin(temp,10) * 40;
	getcross(to);
	move(pstat,to[0] + x1,to[1] + y1);
	hickx();
	crossterm = '\n';
}
eatnl()
{
	while(getchar() != '\n');
}
hickx()
{
	drawr(pstat,20,20);
	drawr(pstat,-40,-40);
	mover(pstat,0,40);
	drawr(pstat,40,-40);
	mover(pstat,-20,20);
}
scanin(world)
	char *world;
{
	register char *p,c;
	register i;
	p = world;
	*p = '\0';

	while((island(c = getchar())!= 1)&&(c!= '\n')&&(c != '\0')&&(c != ';'));

	if(c == '\n') return(-1);
	if(c == '\0') return(-2);
	if(c == ';')return(c);
	*p++ = c;
	i = 1;
	while(island(*p++ = getchar()))
			if(++i > 20) --p;
		if(*(--p) == '\0') return(-2);
		if(*p == '\n'){
			*p = '\0';
			return(-1);
			}
	else {i = *p;	*p = '\0';}
	return(i);
}
sscan(world,string)
	char *world,**string;
{
	register char *p,**c;
	register i;
	p = world;
	*p = '\0';
	c = string;

	while((island(**c)!= 1)&&(**c!= '\n')&&(**c != '\0')&&(**c != ';'))(*c)++;

	if(**c == '\n') return(-1);
	if(**c == '\0') return(-2);
	if(**c == ';')return(**c);
	*p++ = *(*c)++;
	i = 1;
	while(island(*p++ = *(*c)++))
			if(++i > 20) --p;
	(*c)--;
		if(*(--p) == '\0') return(-2);
		if(*p == '\n'){
			*p = '\0';
			return(-1);
			}
	else {i = *p;	*p = '\0';}
	return(i);
}
basin(string,base)
	char *string;
	int base;
{
	char s[21];
	register number;
	register p;
	int i;
	register exponent;
	for(i=0;(s[i]= string[i])!='\0';i++);
	for(p=0;s[p]!='\0';p++);
	for(i=0;(s[i]!='\0');i++){
		if((s[i] == 'o') || (s[i] == 'O'))s[i] = '0';
		if((s[i] >= 'A') && (s[i] <= 'Z'))s[i] =+ 32;
		if(s[i] > '9')s[i]=
			(s[i]-('a'-':'));
		}
	number=0;exponent=1;p=p-1;
		while(p!=(-1)){
			number=number+(((s[p--])-48)*exponent);
			exponent=exponent*base;}
	return(number);
}
llu(world,keylist)		/*   List Look Up	*/
	char *world,*keylist[];
{
	register char *p,*m;
	register i;

	i = 0;				/*	point to first keylist			*/
	while(keylist[i] != -1){	/*	and if not all done with them:	*/
		p = world;		/*	point to input string		*/
		m = keylist[i++];	/*	point to current keylist string	*/

		while((*p++ == *m++)&&(*(p-1)));	/* eat up equal char.	*/

		if((*--p == '\0')&&(*--m == '\0')) return(--i);
	}
	return(-1);
}
basesp(n,t,base)
	int n,base;
	char *t;
{
int p[10];
register i;
register char *j;
j = t;
p[0]= 0;
for(i=0;n != 0;i++){
	p[i] = lrem(0,n,base);
	n = ldiv(0,n,base);
	}
if(i!=0)--i;	/* remove this line if allways want leading 0 */
for(;i >= 0;i--)*j++ = (((p[i] > 9)?(p[i]+87):(p[i]+'0')));
*j = '\0';
}
island(ch)
	char ch;
{
	if(((ch >= 'A')&&(ch <= 'Z'))||((ch >= 'a')&&(ch <= 'z'))) return (1);
	if((ch >= '0')&&(ch <= '9')) return(1);
	if(ch == '_')return(1);
	return(0);
}
islet(ch)
	char ch;
{
	if(((ch >= 'A')&&(ch <= 'Z'))||((ch >= 'a')&&(ch <= 'z'))) return (1);
	if(ch == '_')return(1);
	if((ch >= '0')&&(ch <= '9')) return(0);
	if((ch >= '!')&&(ch <= '/')) return(2);
	if((ch >= ':')&&(ch <= '?')) return(2);
	if((ch == '`')||(ch == '@')) return(2);
	if((ch >= '[')&&(ch <= '_')) return(2);
	if((ch >= '{')&&(ch <= '~')) return(2);
	return(3);
}
/* align and x,y co-ordinate.  CR terminates the allign-
   ment process, ' ' continues it, and 'v'  views  the
   current position.  Crosshairs are positioned in the
   direction of point movment.  Only x OR y can be changed
   (there is no 45 degree movement allowed).		*/
align()
{					/* place passes args */
	register dx,dy,i;
	int tx,ty,x,y,to[3];

	crossterm = ' ';
	x = place[0];	y = place[1];
	move(pstat,x-60,y);
	drawr(pstat,-60,0);
	mover(pstat,120,60);
	drawr(pstat,0,60);
	mover(pstat,60,-120);
	drawr(pstat,60,0);
	mover(pstat,-120,-60);
	drawr(pstat,0,-60);
	mover(pstat,0,120);
    while(1){
	getcross(to);
	if(crossterm == '\n'){
		place[0] = x;	place[1] = y;
		return;
		}
	if(crossterm == 'v'){
		goto alignv;
		}
	dx = to[0] - x;
	dy = to[1] - y;
	tx = dx;	if(tx < 0)tx = -tx;
	ty = dy;	if(ty < 0)ty = -ty;
	if(tx > ty){
		if(dx > 0)x =+ 10;
		    else x=- 10;
		}
	    else{
		if(dy > 0)y=+ 10;
		    else y=- 10;
		}
alignv:	point(pstat,x,y);
	}
}
/*	Read x & y from a command string pointed to by
	**ss type.  Return x & y in 'place' (global).
	*ss is modifed and left at end of the command. */
getxy(ss)
	char **ss;
{
	char temp[20];
	sscan(temp,ss);	/* read x co-ord	*/
	place[0] = basin(temp,10);
	sscan(temp,ss);	/* read y co-ord	*/
	place[1] = basin(temp,10);
}
/*	Take 'command' and align the last data point
	in it, leaving a new command in 'command'.	*/
alcom()
{
	register char *p,*k,c;
	char *glub,**ss,*n,ncomm[200];

	glub = command;	n = ncomm;
	c = *glub++;	*n++ = c;
	ss = &glub;
	if(c == 'w'){
		while((*n++ = *(*ss)++) != ':');
		}
	getxy(ss);
	align();
	make(place,n);
	seke(&n);
	if(c == 'w'){
		*n++ = '\n';
		*n++ = '\0';
		goto alcomf;
		}
	while(*n++ = *(*ss)++);
alcomf:	p = command;	k = ncomm;
	while(*p++ = *k++);
}
gridit(){
	register x,y,n;
	int nn,sx,sy;
	int to[3];

	getcross(to);
	x = to[0]-160;	y = to[1]-160;

	sx = x;
	for(n = 0;n != 9;n++){
		x = sx;
		for(nn = 0;nn != 9;nn++){
			point(pstat,x,y);
			goalpha(pstat);
			x =+ 40;
			}
		y =+ 40;
		}
}
findit()
{
	register *a;
	register char *p,*k;
	int i,n;
	char c;

again:	getcom(getchar());
	plotcom();
	foobar();
	if((c = getchar()) == 'n'){
		eatnl();
		goto again;
		}
	if(c != '\n')eatnl();
	foobar();
	p = command;
	while(*p++);
	p[-2] = '\0';
	a = zero + 1;
	i = dol - zero;
	n = 1;
	while(i--){
		p = getline(*a++);
		k = command;
		while((*p == *k++) && *p)p++;
		if((*p == '\0') && (*--k == '\0')){
			printf("  %d",n);	foobar();
			return;
			}
		n++;
		}
	printf("Element not found");
	foobar();
}

#define error	errfunc()

char	file[FNSIZE];
char	ibuff[BLKSIZE];
char	linebuf[LBSIZE];  /* text line */
char	obuff[BLKSIZE];
char	tfname[]  "/tmp/plot00000";  /* "buffer" name */
char	fmtlno[]  "%7l=";  /* format for line-number output */
char	*charac;	/* command line pointer */
char	*linebp	linebuf;  /* text line pointer */
char	*nextip;
char	appflg	0;	/* append flag (if "w>file") */
char	badf	0;	/* bad read on temp file */
char	io	0;	/* file descriptor for "r", "w", "e" */
char	listf	0;	/* print line after doing command */
char	lnflag	0;	/* line numbering on plot */
char	pflag	0;	/* plot line after doing command */
char	tfile	-1;	/* file des for "buffer" */

int	*addr1;		/* lower line bound */
int	*addr2;		/* upper line bound */
int	*dol;		/* last line in file */
int	*dot;		/* "current" line */
int	*endcore;	/* current end of memory */
int	*fendcore;	/* start of dynamic area */
int	*zero;		/* anchor line for all other lines */
int	iblock	-1;	/* block number of input buffer */
int	ichanged;	/* ibuf has been changed */
int	line_num;	/* integer for line number on output */
int	ninbuf;		/* ??? */
int	nleft;		/* byte count remaining in output buffer */
int	oblock	-1;	/* output buffer block number */
int	tline;		/* ??? */

int	fout	1;
int	_opos	0;
char	_obuf[512];

/*
 * error messages
 *
 *	(there are more than these)
 */

char	*errtext[]	{
	/*  0 */	"syntax is k[a-z]",
	/*  1 */	"illegal command format",
	/*  2 */	"no command",
	/*  3 */	"no tab character",
	/*  4 */	"can't change filename",
	/*  5 */	"file name syntax",
	/*  6 */	"recursive \"use\" command",
	/*  7 */	"null file name illegal",
	/*  8 */	"unrecognized command",
	/*  9 */	"no tabs set",
	/* 10 */	"global command not allowed with huge file",
	/* 11 */	"file name too long",
	/* 12 */	"expanded line too long",
	/* 13 */	0,
	/* 14 */	"can't fork",
	/* 15 */	"can't write to process",
	/* 16 */	"no lines",
	/* 17 */	"backup(FILE) error (?)",
	/* 18 */	"string not found",
	/* 19 */	"  '  must be followed by [a-z]",
	/* 20 */	"address syntax error",
	/* 21 */	"lower address bound > upper one",
	/* 22 */	"address illegal here",
	/* 23 */	"non-existent line number",
	/* 24 */	"bottom of file reached",
	/* 25 */	"command syntax error",
	/* 26 */	"\"advance\" error (?)",
	/* 27 */	"null string illegal",
	/* 28 */	"destination not found",
	/* 29 */	"INTERRUPT!",
	/* 30 */	"line too long",
	/* 31 */	"missing destination address",
	/* 32 */	"I/O error--file not saved!",
	/* 33 */	"file overflows available memory",
	/* 34 */	"file too large (TMPERR)",
	/* 35 */	"I/O error on temp file (TMPERR)",
	/* 36 */	"open error on temp file (TMPERR)",
	/* 37 */	"recursive global command",
	/* 38 */	"global command list too long",
	/* 39 */	"substitute pattern not found",
	/* 40 */	"missing substring",
	/* 41 */	"string2 too long",
	/* 42 */	"substring too long",
	/* 43 */	"substituted string too long",
	/* 44 */	"too many  \\(",
	/* 45 */	"unbalanced  \\(  \\)",
	/* 46 */	"\\n illegal in pattern",
	/* 47 */	"illegal use of  *",
	/* 48 */	"missing  ]",
	/* 49 */	"pattern too complicated",
	/* 50 */	"string too long",
};

main(argc, argv)
char **argv;
{
	int onintr(), hangup(), mail(), shutdown(), getfile();
	register char *p1, *p2;
	register n;
	int savint, num;

	signal(SIGQIT, 1);
	savint = signal(SIGINT, 1);

	fendcore = sbrk(0);
	n = getpid();
	for (num = 13; num > 8; num--) {
		tfname[num] = n % 10 + '0';
		n =/ 10;
	}
	setexit();
	if ((savint & 01) == 0)
		signal(SIGINT, onintr);
	init();
	setexit();
	while (cmds(0));
	delexit(0);
}

address() {
	extern *zero, *dot, *dol;
	extern char *charac;
	register *a1, minus, c;
	int n, relerr, *start;

	minus = 0;
	a1 = 0;
	for (;;) {
		c = *charac++;
		if ('0' <= c && c <= '9') {
			--charac;
			n = getnum();
			if (a1 == 0)
				a1 = zero;
			if (minus < 0)
				n = -n;
			a1 =+ n;
			minus = 0;
			continue;
		}
		relerr = 0;
		if (a1 || minus)
			relerr++;
		switch (c) {
		case '+':
			minus++;
			if (a1 == 0)
				a1 = dot;
			continue;

		case '-':
			minus--;
			if (a1 == 0)
				a1 = dot;
			continue;

		case '$':
			a1 = dol;
			break;

		case '.':
			a1 = dot;
			break;

		default:
			--charac;
			if (a1 == 0)
				if (c == ',' || c == ';')
					if (dot + 1 > dol)
						return(dol);
					else
						return(dot + 1);
				else
					return(0);
			a1 =+ minus;
			if (a1 < zero)
				a1 = dol == zero? zero : zero + 1;
			else if (a1 > dol)
				a1 = dol;
			return(a1);
		}
		if (relerr)
			errmsg(20);
	}
}

append(f, a, single, bkpf)
int (*f)();
{
	extern *dot, *dol, *endcore;
	extern char linebuf[];
	register *a1, *a2, *rdot;
	int nline, tl;
	struct { int integer; };

	nline = 0;
	dot = a;
	while ((*f)(single) == 0) {
		if (dol >= endcore) {
			if (sbrk(1024) == -1)
				errmsg(33);
			endcore.integer =+ 1024;
		}
		tl = putline(linebuf);
		nline++;
		a1 = ++dol;
		a2 = a1 + 1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
		if (single)
			break;
	}
	return(nline);
}

blkio(b, buf, iofcn)
int (*iofcn)();
{
	extern char *tfile, badf;

	seek(tfile, b, 3);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		badf++;
		errmsg(-35);
	}
}

cmds(rflg) {
	extern getfile(), gettty(), onintr(), getit();
	extern *addr1, *addr2, *dot, *dol, *zero, io, line_num, ninbuf, pstat[];
	extern char *charac, file[], fmtlno[], command[];
	extern char appflg, lnflag, pflag, listf;
	register *a1, c;
	register char *p;
	int r, num;

	addr1 = 0;
	addr2 = 0;
	if (listf || pflag) {
		if (dot <= zero)
			dot = zero + 1;
		if (dot > dol)
			dot = dol;
		addr1 = addr2 = dot;
		if (listf) {
			listf = 0;
			goto print;
		}
		pflag = 0;
		goto plot;
	}
	foobar();	/* go to next text line position */
	if (rflg == 0) {
		puts2("? ");
		flush();
		getecom();	/* read an edit command from tty */
	}
	charac = command;
	r = 1;
	do {
		addr1 = addr2;
		if ((a1 = address()) == 0) {
			c = *charac++;
			break;
		}
		addr2 = a1;
		if ((c = *charac++) == ';') {
			c = ',';
			dot = a1;
		}
	} while (c == ',' && r-- > 0);
	if (addr1 == 0)
		addr1 = addr2;
	line_num = (addr1? addr1 : dot) - zero;

	switch (c) {
	case 'a':	/* append */
		setdot();
		line_num++;
		num = addr2;
	caseadd:
		newline();
		r = 0;	/* prompting for the moment */
		append(gettty, num, r, 1);
		break;

	case 'c':	/* change (replace) */
		newline();
		setdot();
		nonzero();
		delete();
		append(gettty, addr1 - 1, 0, 0);
		break;

	case 'd':	/* delete */
#define DEBUG
#ifdef DEBUG
/*		*du	Dump command (testing only)	*/
		if (*charac == 'u') {
			charac++;
			dump();
			break;
		}
#endif
		newline();
		setdot();
		nonzero();
		delete();
		break;

	case 'h':	/* plot on hp */
		setall();
		nonzero();
		hpplot();
		break;

	case 'i':	/* insert */
		setdot();
		nonzero();
		num = addr2 - 1;
		goto caseadd;

	case 'm':	/* move */
		r = 0;
	casemove:
		setdot();
		nonzero();
		tmove(r);
		break;

	case 'n':	/* line number toggle (plot labels) */
		setnoaddr();
		newline();
		lnflag = !lnflag;
		foobar();	/* set cursor position */
		printf("%sline numbers\n", (lnflag? "" : "no "));
		break;

	case '\n':	/* plot (next) line */
		if (addr2 > dol)
			break;
		if (addr2 == 0) {
			addr1 = addr2 = dot + 1;
			if (addr2 <= dol)
				line_num++;
		}
		if (addr2 <= dol) {
			pflag = 0;
			goto plot;
		}
		break;

	case 'l':	/* list lines (text) */
		if (*charac == 'l') {
			charac++;
			addr1 = zero + 1;
			addr2 = dol;
		}
		newline();
		pflag = 0;
		setdot();
	print:
		nonzero();
		a1 = addr1;
		do {
			line_num = a1 - zero;
			foobar();	/* set cursor pos */
			printf(fmtlno, line_num);
			puts(getline(*a1++));
		} while (a1 <= addr2);
		dot = addr2;
		break;

	case 'p':	/* plot */
		setdot();
		if (*charac == 'p') {
			charac++;
			addr1 = zero + 1;
			addr2 = dol;
			plotcls(pstat);
			aposit = 7400;	apositx = 160;
		}
		newline();
	plot:
		nonzero();
		tekplot();
		break;

	case 'q':	/* quit */
		setnoaddr();
		newline();
		return(0);

	case 'r':	/* read */
		filename();
		if ((io = open(file, 0)) < 0) {
			foobar();	/* set cruiser pos */
			puts2("can't read ");
			puts(file);
			io = 0;
			error;	/* errmsg(9) */
		}
		setall();
		ninbuf = 0;
		r = append(getfile, addr2, 0, 0);
		foobar();	/* set cursor pos */
		printf("%l line%s\n", r, (r == 1? "" : "s"));
		exfile();
		break;

	case 's':	/* stop */
		if (zero == dol)
			return(0);
		addr1 = zero + 1;
		addr2 = dol;
		r = 0;
		goto casewrite;

	case 't':	/* transfer (copy) */
		r = 1;
		goto casemove;

	case 'w':	/* write */
		setall();
		r = 1;
	casewrite:
		filename();
		if (dol == zero) {
			foobar();	/* set cursor */
			puts("[nothing written]");
			break;
		}
		if (appflg)
			io = open(file, 1);
		if (!appflg || io < 0)
			io = creat(file, 0644);
		if (io < 0) {
			io = 0;
			foobar();	/* set cursor */
			puts2("can't create ");
			puts(file);
			error;
		}
		if (appflg)
			seek(io, 0, 2);	/* append onto file */
		putfile();
		exfile();
		return(r);

	case '!':	/* shell command */
		setnoaddr();
		unix();
		break;

	case 0:	/* exit */
		return(0);

	default:
		errmsg(8);
		skip_rest();
		break;
	}
	return(1);
}

delete() {
	extern *addr1, *addr2, *dol, *dot;
	register *a1, *a2, *a3;

	a1 = addr1;
	a2 = addr2 + 1;
	a3 = dol;
	dol =- a2 - a1;
	do
		*a1++ = *a2++;
	while (a2 <= a3);
	a1 = addr1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
}

delexit(aretcode) {
	extern char tfname[];

	unlink(tfname);
	exit(aretcode);
}

dump() {
	extern *dot, *zero, *addr1, *addr2, *line_num;
	extern char fmtlno[];
	register *i, b, o;

	setdot();
	nonzero();
	newline();
	line_num = addr1 - zero;
	for (i = addr1; i <= addr2; i++) {
		b = (*i >> 8) & 0377;
		o = (*i << 1) & 0774;
		printf(fmtlno, line_num);
		printf("%6o: %6o  %4d/%d\n", i, *i, b, o);
		line_num++;
	}
	dot = addr2;
}

errfunc() {
	extern char pflag;

	exfile();
	pflag = 0;
	skip_rest();
	reset();
}

errmsg(n) {
	extern char *errtext[];

	foobar();	/* set cursor position */
	if (n < 0) {
		puts2("******** ");
		n = -n;
	}
	if (0 <= n && n <= MAXERR)
		puts(errtext[n]);
	error;
}

exfile() {
	extern char io;

	if (io) {
		close(io);
		io = 0;
	}
}

filename() {
	extern char appflg, file[], *charac;
	register char *p1, *p2;
	register c;

	appflg = 0;
	c = *charac;
	if (c == '\n' || c == 0)
		goto nofil;
	if (c != ' ' && c != ',' && c != '>')
    nofil:	errmsg(5);
	if (c == ',')
		c = *++charac;
	while (c == ' ')
		c = *++charac;
	if (c == '>') {
		while ((c = *++charac) == '>');
		appflg++;
		if (c == '\n')
			goto nofil;
		while (c == ' ')
			c = *++charac;
	}
	if (c == '\n')
		errmsg(7);
	p1 = file;
	do {
		*p1++ = c;
		if (p1 >= &file[FNSIZE - 2])
			errmsg(11);
	} while ((c = *++charac) && c != '\n');
	*p1++ = 0;
}

flush() {
	if (_opos) {
		write(fout, _obuf, _opos);
		_opos = 0;
	}
}

getblock(atl, iof) {
	extern read(), write();
	extern iblock, oblock, nleft;
	extern char ibuff[], obuff[], badf, ichanged;
	register bno, off;

	bno = (atl >> 8) & 0377;
	off = (atl << 1) & 0774;
	if (bno >= 255)
		errmsg(34);
	nleft = 512 - off;
	if (bno == iblock && !badf) {
		ichanged =| iof;
		return(ibuff + off);
	}
	if (bno == oblock)
		return(obuff + off);
	if (iof == READ) {
		if (ichanged) {
			blkio(iblock, ibuff, write);
			ichanged = 0;
		}
		iblock = bno;
		blkio(bno, ibuff, read);
		return(ibuff + off);
	}
	if (oblock >= 0)
		blkio(oblock, obuff, write);
	oblock = bno;
	return(obuff + off);
}

getecom() {
	extern command[];
	register char *p, c;

	for (p = command; c = getchar();) {
		if (c == EOF)
			break;
		*p++ = c;
		if (c == '\n')
			break;
	}
	*p = 0;
}

getfile() {
	extern ninbuf;
	extern char io, linebuf[], *nextip;
	register c;
	register char *lp, *fp;
	static char genbuf[LBSIZE];

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io,genbuf,LBSIZE) - 1) < 0)
				return(EOF);
			fp = genbuf;
		}
		if (lp >= &linebuf[LBSIZE - 2]) {
			puts("line in file too long");
			*lp++ = '>';
			*lp = 0;
			nextip = fp;
			return(0);
			break;
		}
		if ((*lp++ = c = *fp++ & 0177) == 0) {
			lp--;
			continue;
		}
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

getline(tl) {
	extern nleft;
	extern char linebuf[];
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, READ);
	nl = nleft;
	tl =& ~0377;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl =+ 0400, READ);
			nl = nleft;
		}
	return(linebuf);
}

getnum() {
	extern char *charac;
	register n;
	register char c;

	n = 0;
	while ('0' <= (c = *charac++) && c <= '9')
		n = n * 10 + c - '0';
	--charac;
	return(n);
}

gettty(single) {
	extern line_num;
	extern char fmtlno[], linebuf[], *charac, command[];
	register c;
	register char *p;

	p = linebuf;
	if (!single) {
		foobar();	/* reset cursor appropriately */
		printf(fmtlno, line_num);
	}
	line_num++;
	flush();
	getit();	/* get the line to append */
	charac = command;
	while ((c = *charac++) != '\n') {
		*p++ = c;
		if (c == 0) {
			--charac;
			puts2("\n");
			return(EOF);
		}
		if (p >= &linebuf[LBSIZE-2])
			errmsg(30);
	}
	*p = 0;
	return(0);
}

hpplot() {
	extern *addr1, *addr2, *dot, pstat[];
	extern char command[];
	register *a;
	register char *p1, *p2;

	plotsel(pstat, HP);
	plotinit(pstat);
	a = addr1;
	while (a <= addr2) {
		p1 = getline(*a++);
		p2 = command;
		while (*p1)
			*p2++ = *p1++;
		*p2++ = '\n';
		*p2 = 0;
		plotcom();
	}
	plotend(pstat);
	plotsel(pstat, TEK);
	plotinit(pstat);
	aposit = 7400;	apositx = 160;
	dot = addr2;
}

init() {
	extern tline, iblock, oblock, *fendcore, *addr1, *addr2;
	extern *dol, *dot, *zero, *endcore;
	extern char tfile, badf, ichanged, tfname[];
	register pid, n;
	register char *p;

	close(tfile);
	exfile();
	tline = 0;
	iblock = -1;
	oblock = -1;
	badf = 0;
	ichanged = 0;
	close(creat(tfname, 0600));
	if ((tfile = open(tfname, 2)) < 0)
		errmsg(-36);
	brk(fendcore);
	addr1 = addr2 = dot = zero = dol = fendcore;
	endcore = fendcore - 2;
	psetup();
	return(0);
}

newline() {
	extern char pflag, *charac, listf;
	register c;

	if ((c = *charac++) == '\n')
		return;
	if (c == 'p' || c == 'l') {
		if (c == 'p')
			pflag++;
		else
			listf++;
		if (*charac++ == '\n')
			return;
	}
	errmsg(25);
}

nonzero() {
	extern *addr1, *addr2, *dol, *zero;

	if (addr1 <= zero)
		errmsg(23);
	if (addr2 > dol)
		errmsg(24);
}

onintr() {
	extern char *charac, command[];
	extern savebuf[];

	signal(SIGINT, onintr);
	*(charac = command) = 0;
	reclear();
	stty(0, savebuf);
	errmsg(29);
}

putchar(c) {
	_obuf[_opos++] = c;
	if (_opos >= 512)
		flush();
	return(c);
}

putfile() {
	extern *addr1, *addr2;
	extern char io;
	register char *fp, *lp;
	register nib;
	int *a1;
	char genbuf[LBSIZE];

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		for (;;) {
			if (--nib < 0) {
				wte(io, genbuf, fp - genbuf);
				nib = 511;
				fp = genbuf;
			}
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	wte(io, genbuf, fp - genbuf);
}

putline(abuf)
char abuf[];
{
	extern tline, nleft;
	extern char *linebp;
	register char *bp, *lp;
	register nl;
	int tl;

	lp = abuf;
	tl = tline;
	bp = getblock(tl, WRITE);
	nl = nleft;
	tl =& ~0377;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl =+ 0400, WRITE);
			nl = nleft;
		}
	}
	nl = tline;
	tline =+ (((lp - abuf) + 03) >> 1) & 077776;
	return(nl);
}

puts(as)
char *as;
{
	puts2(as);
	puts2("\n");
}

puts2(as)
char *as;
{
	register char *p;

	for (p = as; *p;)
		putchar(*p++);
}

setall() {
	extern *addr1, *addr2, *dol, *zero;

	if (addr2 == 0) {
		addr1 = dol == zero? zero : zero + 1;
		addr2 = dol;
	}
	setdot();
}

setdot() {
	extern *addr1, *addr2, *dot;

	if (addr2 == 0)
		addr1 = addr2 = dot;
	if (addr1 > addr2)
		errmsg(21);
}

setnoaddr() {
	extern *addr2;

	if (addr2)
		errmsg(22);
}

skip_rest() {
	extern char *charac, command[];

	charac = command;
	*charac = 0;
}

tekplot() {
	extern *addr1, *addr2, *dot;
	extern char command[];
	register *a;
	register char *p1, *p2;

	a = addr1;
	while (a <= addr2) {
		p1 = getline(*a++);
		p2 = command;
		while (*p1)
			*p2++ = *p1++;
		*p2++ = '\n';
		*p2 = 0;
		plotcom();
	}
	dot = addr2;
}

tmove(cflag) {
	extern getcopy(), *dot;
	extern *addr1, *addr2, *dol;
	register *adt, *ad1, *ad2;

	setdot();
	nonzero();
	if ((adt = address()) == 0)
		errmsg(31);
	ad1 = addr1;
	ad2 = addr2;
	newline();
	if (cflag) {
		ad1 = dol;
		append(getcopy, ad1++, 0, 0);
		ad2 = dol;
	}
	ad2++;
	if (adt < ad1) {
		dot = adt + (ad2 - ad1);
		if (++adt == ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		errmsg(28);
}

reverse(aa1, aa2) {
	register *a1, *a2, t;

	a1 = aa1;
	a2 = aa2;
	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy() {
	extern *addr1, *addr2;

	if (addr1 > addr2)
		return(EOF);
	getline(*addr1++);
	return(0);
}

unix() {
	extern char command[];
	register pid, wpid, savint;

	savint = signal(SIGINT, 1);
	if ((pid = fork()) < 0) {
		signal(SIGINT, savint);
		errmsg(14);
	}
	if (pid == 0) {
		signal(SIGINT, 0);
		signal(SIGQIT, 0);
		execl("/bin/sh", "e!", "-c", &command[1], 0);
		puts("No shell!");
		exit(1);
	}
	while ((wpid = wait()) != pid && wpid != -1);
	signal(SIGINT, savint);
	puts("!");
}

wte(fd, buf, len)
char *buf;
{
	if (write(fd, buf, len) != len)
		errmsg(-32);
}
