/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
#define SIGINT 2
#define SIGQIT 3
/*
C version of tcatsim
*/

#define OBSZ 512
#define MAXY 3071
#define US 037
#define GS 035
#define ESC 033
#define FF 014
#define DBL 0200

int pl 11*144;
int mpy 1;
int div 1;
char *ap;
int ch;
int nonumb;
int psize 10;
int dfact 1;
int ibuf[259];
char obuf[OBSZ];
char *obufp obuf;
int esc;
int escd;
int verd;
int esct;
int osize 02;
int size 02;
int rx;
int xx 0;
int yy MAXY+62+48;
int leadtot -31;
int ohy -1;
int ohx -1;
int oxb -1;
int oly -1;
int olx -1;
int tflag;
int railmag;
int lead;
int skip;
int pgskip;
int ksize ';';
int mcase;
int stab[]{010,0,01,07,02,03,04,05,0211,06,0212,0213,0214,0215,0216,0217};
int rtab[]{6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 28, 26, 18};
int ktab[]{';',';',';',';',';',';',':',':','9','9','9','9','8','8','8','9'};
int od 1;
int first 1;
int alpha;
extern char *asctab[128];
extern char *spectab[128];
int erase 1;
int xxx;
int sigint;
int sigqit;

main(argc,argv)
int argc;
char **argv;
{
	register i, j;
	register char *k;
	extern ex();

	while((--argc > 0) && ((++argv)[0][0]=='-')){
		switch(argv[0][1]){
			case 'p':
				ap = &argv[0][2];
				dfact = 72;
				if(i = atoi())pl = i/3;
				continue;
			case 't':
				tflag++;
				continue;
			case 's':
				ap = &argv[0][2];
				dfact = 1;
				pgskip = atoi();
				continue;
			default:
				dfact = 1;
				ap = &argv[0][1];
				if(i = atoi())mpy = i;
				if(i = atoi())div = i;
				continue;
		}
	}
	if(argc){
		if(fopen(argv[0], ibuf) < 0){
			prstr("Cannot open: ");
			prstr(argv[0]);
			prstr("\n");
			exit(1);
		}
	}
	sigint=signal(SIGINT, ex);
	sigqit=signal(SIGQIT,1);
	while((i = getc(ibuf)) >= 0){
		if(!i)continue;
		if(i & 0200){
			esc =+ (~i) & 0177;
			continue;
		}
		if(esc){
			if(escd)esc = -esc;
			esct =+ esc;
			xx =+ (esc*mpy + rx)/div;
			rx = (esc*mpy + rx)%div;
			sendpt();
			esc = 0;
		}
		switch(i){
			case 0100:	/*init*/
				escd = verd = mcase = railmag = xx = 0;
				yy = MAXY + 48;
				leadtot = -31;
				ohy = oxb = oly = ohx = olx = -1;
				oput(US);
				flusho();
				if(!first && !tflag)kwait();
				if(first){
					first = 0;
					yy =+ 62;
				}
				init();
				continue;
			case 0101:	/*lower rail*/
				railmag =& ~01;
				continue;
			case 0102:	/*upper rail*/
				railmag =| 01;
				continue;
			case 0103:	/*upper mag*/
				railmag =| 02;
				continue;
			case 0104:	/*lower mag*/
				railmag =& ~02;
				continue;
			case 0105:	/*lower case*/
				mcase = 0;
				continue;
			case 0106:	/*upper case*/
				mcase = 0100;
				continue;
			case 0107:	/*escape forward*/
				escd = 0;
				continue;
			case 0110:	/*escape backward*/
				escd = 1;
				continue;
			case 0111:	/*stop*/
				continue;
			case 0112:	/*lead forward*/
				verd = 0;
				continue;
			case 0113:	/*undefined*/
				continue;
			case 0114:	/*lead backward*/
				verd = 1;
				continue;
			case 0115:	/*undefined*/
			case 0116:
			case 0117:
				continue;
		}
		if((i & 0340) == 0140){	/*leading*/
			lead = (~i) & 037;
			if(verd)lead = -lead;
			if((leadtot =+ lead) > pl){
				leadtot = lead;
				oput(US);
				flusho();
				if(!tflag)kwait();
				yy = MAXY;
				if(pgskip)--pgskip;
				init();
				continue;
			}
			if(skip)continue;
			if((yy =- (lead<<1)) < 0){
				skip++;
				yy = 0;
			}else sendpt();
			continue;
		}
		if((i & 0360) == 0120){	/*size change*/
			i =& 017;
			for(j = 0; i != (stab[j] & 017); j++);
			osize = size;
			size = stab[j];
			psize = rtab[j];
			ksize = ktab[j];
			oput(ESC);
			oput(ksize);
			i = 0;
			if(!(osize & DBL) && (size & DBL))i = -55;
			else if((osize & DBL) && !(size & DBL))i = 55;
			if(escd)i = -i;
			esc =+ i;
			continue;
		}
		if(i & 0300)continue;
		i = (i & 077) | mcase;
		if(railmag != 03)k = asctab[i];
		else k = spectab[i];
		if(alpha)sendpt();
		if(k != -1){
			oput(US);
			while(*k & 0377)oput(*k++);
			alpha++;
			continue;
		}else{
			if(railmag != 03){
				switch(i){
				case 0124: lig("fi"); break;
				case 0125: lig("fl"); break;
				case 0126: lig("ff"); break;
				case 0130: lig("ffl"); break;
				case 0131: lig("ffi"); break;
				default: continue;
				}
			}
			continue;
		}
	}
	ex();
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
lig(x)
char *x;
{
	register i, j;
	register char *k;

	j = 0;
	k = x;
	oput(US);
	oput(*k++);
	i = psize * 8 * mpy / (div * 6); /* 8/36 em */
	while(*k){
		xx =+ i;
		j =+ i;
		sendpt();
		oput(US);
		oput(*k++);
	}
	xx =- j;
	sendpt();
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
init(){
	register i;

	flusho();
	if(erase){
		oput(ESC);
		oput(FF);
	}else erase = 1;
	oput(ESC);
	oput(ksize);
	/*delay about a second*/
	for(i = 960; i > 0; i--)oput(GS);
	skip = 0;
	sendpt();
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
ex(){
	yy = MAXY;
	xx = 0;
	sendpt();
	oput(ESC);
	oput(';');
	oput(US);
	flusho();
	exit(0);
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
kwait(){
	char buf[128]; char *bptr; char c;
	if(pgskip) return;
next:
	bptr=buf;
	while((c=readch())&&(c!='\n')) *bptr++=c;
	*bptr=0;
	if(bptr!=buf){
		bptr = buf;
		if(*bptr == '!'){unix(&buf[1]); prstr("!\n"); goto next;}
		else switch(*bptr++){
			case 'e':
				erase = 0;
				goto next;
			case 's':
				ap = &buf[1];
				dfact = 1;
				pgskip = atoi() + 1;
				goto next;
			default:
				prstr("?\n");
				goto next;
		}
	}
	else if (c==0) ex();
	else	return;
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
unix(line)
char line[];
{
	int rc, status, unixpid;
	if( (unixpid=fork())==0 ) {
		signal(SIGINT,sigint); signal(SIGQIT,sigqit);
		close(0); dup(2);
		execl("/bin/sh", "-sh", "-c", line, 0);
		exit(255);
	}
	else if(unixpid == -1)
		return;
	else{	signal(SIGINT,1); signal(SIGQIT,1);
		while( (rc = wait(&status)) != unixpid && rc != -1 ) ;
		signal(SIGINT,ex); signal(SIGQIT,sigqit);
	}
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
readch(){
	char c;
	if (read(2,&c,1)<1) c=0;
	return(c);
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
oput(i)
char i;
{
	if(pgskip)return;
	*obufp++ = i;
	if(obufp >= (obuf + OBSZ))flusho();
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
flusho(){
	write(od,obuf,obufp-obuf);
	obufp = obuf;
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
sendpt(){
	int hy,xb,ly,hx,lx;

	oput(GS);
	hy = ((yy>>7) & 037);
	xb = ((xx & 03) + ((yy<<2) & 014) & 017);
	ly = ((yy>>2) & 037);
	hx = ((xx>>7) & 037);
	lx = ((xx>>2) & 037);
	if(hy != ohy)oput(hy | 040);
	if(xb != oxb)oput(xb | 0140);
	if((ly != oly) || (hx != ohx) || (xb != oxb))
		oput(ly | 0140);
	if(hx != ohx)oput(hx | 040);
	oput(lx | 0100);
	ohy = hy;
	oxb = xb;
	oly = ly;
	ohx = hx;
	olx = lx;
	alpha = 0;
	return;
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
prstr(s)
char *s;
{
	register i;

	for(i=0;*s;i++)s++;
	write(2,s-i,i);
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
atoi()
{
	register i, j, acc;
	int field, digits, *dd, *tscale();

	field = digits = acc = 0;
a1:
	while(((j = (i = getch()) - '0') >= 0) && (j <= 9)){
		field++;
		digits++;
		acc = 10*acc + j;
	}
	if(i == '.'){
		field++;
		digits = 0;
		goto a1;
	}
	if(!(ch = i))ch = 'x';
	dd = tscale(acc);
	acc = dd[0];
	if((field != digits) && (digits > 0)){
		j = 1;
		while(digits--)j =* 10;
		acc = ldiv(dd[1],dd[0],j);
	}
	nonumb = !field;
	ch = 0;
	return(acc);
}
int *tscale(n)
int n;
{
	register i, j;
	static int aa[2];

	switch(i = getch()){
		case 'u':
			j = 1;
			break;
		case 'p':	/*Points*/
			j = 6;
			break;
		case 'i':	/*Inches*/
			j = 432;
			break;
		case 'c':	/*Centimeters; should be 170.0787*/
			j = 170;
			break;
		case 'P':	/*Picas*/
			j = 72;
			break;
		default:
			j = dfact;
			ch = i;
	}
	aa[0] = n * j;
	aa[1] = hmul(n,j);
	return(aa);
}
/*
Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:

Installation Instructions:

History:

*/
getch(){
	register i;

	if(ch){
		i = ch;
		ch = 0;
		return(i);
	}
	return(*ap++);
}
