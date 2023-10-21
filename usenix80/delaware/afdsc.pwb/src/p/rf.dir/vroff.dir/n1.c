#
#include <error.h>
#include "tv.h"
#include "tdef.h"
#include "t.h"
#include "tw.h"
/*
troff1.c

consume options, initialization, main loop,
input routines, escape function calling
*/

extern int stdi;
extern int waitf;
extern int nofeed;
extern int quiet;
extern int ptid;
extern int ascii;
extern int npn;
extern int stop;
extern char ibuf[IBUFSZ];
extern char xbuf[IBUFSZ];
extern char *ibufp;
extern char *xbufp;
extern char *eibuf;
extern char *xeibuf;
extern int cbuf[NC];
extern int *cp;
extern int *vlist;
extern int nx;
extern int mflg;
extern int ch;
extern int pto;
extern int pfrom;
extern int chbits;
extern int suffid;
extern int sufind[26];
extern char suftab[];
extern int ibf;
extern int ibfe;
extern int ttyod;
extern int ttys[3];
extern int iflg;
extern int ioff;
extern int init;
extern int rargc;
extern char **argp;
extern char trtab[256];
extern int blist[];
extern int lgf;
extern int copyf;
extern int eschar;
extern int ch0;
extern int cwidth;
extern int iseek;
extern int ip;
extern int nlflg;
extern int *nxf;
extern int *ap;
extern int *frame;
extern int *stk;
extern int donef;
extern int nflush;
extern int nchar;
extern int rchar;
extern int nfo;
extern int ifile;
extern int fc;
extern int padc;
extern int tabc;
extern int dotc;
extern int raw;
extern int tabtab[NTAB];
extern char nextf[];
extern char searchf[];
extern int nfi;
#ifdef NROFF
extern char termtab[];
extern int tti;
#endif
extern int ifl[NSO];
extern int offl[NSO];
extern int ipl[NSO];
extern int ifi;
extern int pendt;
extern int flss;
extern int fi;
extern int lg;
extern char ptname[];
extern int print;
extern int nonumb;
extern int pnlist[];
extern int *pnp;
extern int nb;
extern int trap;
extern int *litlev;
extern int tflg;
extern int ejf;
extern int *ejl;
extern int lit;
extern int cc;
extern int c2;
extern int spread;
extern int gflag;
extern int oline[];
extern int *olinep;
extern int dpn;
extern int noscale;
extern char *unlkev;
extern char *unlkms;
extern int pts;
extern int level;
extern int ttysave;
extern int tdelim;
extern int dotT;
extern int tabch, ldrch;
extern int eqflg;
extern no_out;
extern int hflg;
extern int xxx;
char ttyx[] "/dev/ttyx";
extern struct contab {
	int rq;
	int (*f)();
}contab[NM];
extern int ip, offset;
extern int *dip;
int ms[] {31,28,31,30,31,30,31,31,30,31,30,31};
int expt 0;
int exfont 0;

#define nextarg {++argv; --argc;}
#define lastarg {++argv; --argc;}

main(argc,argv)
int argc;
char **argv;
{
	char *p, *q;
	char buf[36], home[64];
	int *fp;
	register i, j;
	extern catch(), fpecatch(), kcatch();

	nice(20);	/* Yawn */

	signal(SIGHUP,catch);
	if(signal(SIGINT,catch) & 01){
		signal(SIGHUP,1);
		signal(SIGINT,1);
		signal(SIGQUIT,1);
	}
	signal(SIGFPE,fpecatch);
	signal(SIGPIPE,catch);
	signal(SIGKILL,kcatch);
	init1(argv[0][0]);
	/*
	 * MAIN LOOP
	 */

options:
	nextarg;	/* skip arg 0 */
	while(argc > 0 && **argv == '-')
	{
		for(;;)
		{
		    switch(*++*argv){

		case 'i':
			stdi++;
			break;
		case 'q':
			quiet++;
			break;
		case 'n':
			nextarg;
			npn = cnum(*argv);
			goto nextflg;
		case 's':
			if(!(stop = cnum(&argv[0][2])))stop++;
			break;
		case 'r':
			nextarg;
			i = **argv & BMASK;
			i =| (*++*argv & BMASK) << BYTE;
			vlist[findr(i)] = cnum(&argv[1][0]);
			nextarg;
			goto nextflg;
		case 'm':
			/* note-  only one "-m" flag allowed */
			p = &searchf[nfi];
			nextarg;
			q = *argv;
			while((*p++ = *q++) != 0);
			/* DIRECTORY SEARCH FOR MACRO LIBRARY */
			if(stat(&searchf[nfi-5], buf) != -1)	/* tmac.___ in current directory */
				for(p = nextf, q = &searchf[nfi-5]; (*p++ = *q++) != 0;);
			else
			{
				homedir(home, *argv);	/* find home directory, concat lib name */
				if(stat(home,buf) != -1)	/* home-dir/mac/tmac.___ */
					for(p = nextf, q = home; (*p++ = *q++) != 0;);
				else
					if(stat(&searchf[4], buf) != -1)	/* /lib/mac/tmac.___ */
						for(p = nextf, q = &searchf[4]; (*p++ = *q++) != 0;);
					else
						for(p = nextf, q = &searchf; (*p++ = *q++) != 0;);	/* /usr/lib/mac/tmac.___ */
			}
			/* ------------------------------- */
			mflg++;
			goto nextflg;
		case 'o':
			nextarg;
			getpn(*argv);
			goto nextflg;
#ifndef NROFF
		case 'z':
			no_out++;
		case 'a':
			ascii = 1;
			nofeed++;
		case 't':
			ptid = 1;
			break;
/*		case 'w':
			waitf = 1;
			continue;
		case 'f':
			nofeed++;
			continue;
		case 'b':
			if(open(ptname,1) < 0)prstr("Busy.\n");
			else prstr("Available.\n");
			done3(0);
		case 'g':
			stop = ptid = gflag = 1;
			dpn = 0;
			continue;
*/
#endif
		case '\0':
			goto nextflg;
		default:
			prstr("Illegal Flag\n");
			ptid = 1;	/* to prevent flusho from trying to open typesetter */
			done3(EFLAG);
		}
	    }

nextflg:
	nextarg;
	}

start:
	argp = argv;
	rargc = argc;
	init2();
	setexit();
/*
 * MAIN LOOP through which all lines must pass
 */
loop:
	copyf = lgf = nb = nflush = nlflg = 0;
	fp = frame;
	if((ip != -1) &&  (rbf0(iseek, ip)==0) && ejf && (*++fp <= ejl)){
		nflush++;
		trap = 0;
		eject(0);
		goto loop;
	}
	i = getch();
	if(pendt)goto lt;
	if(lit && (frame <= litlev)){
		lit--;
		goto lt;
	}
	if((j = (i & CMASK)) == XPAR){
		copyf++;
		tflg++;
		for(;(i & CMASK) != '\n';)pchar(i = getch());
		tflg = 0;
		copyf--;
		goto loop;
	}
	if((j == cc) || (j == c2)){
		if(j == c2)nb++;
		copyf++;
		while(((j=((i=getch()) & CMASK)) == ' ') ||
			(j == '\t'));
		ch = i;
		copyf--;
		control(getrq(),1);
		flushi();
		goto loop;
	}
lt:
	ch = i;
	text();
	goto loop;
}

homedir(str, filename)
	char *str, *filename;
{
	register char *p;
	register int i;
	char buf[100];
	pwuid(getuid()&0377, buf);
	p = buf;
	for(i=0;i < 5 && *p != '\0'; i++)
		while(*p != '\0' && *p++ != ':');
	while((*str = *p++) && *str++ != ':');
	--str;
	/* add library and file name */
	for(p = "/mac/tmac."; (*str++ = *p++) != 0;);
	--str;
	for(p = filename; (*str++ = *p++) != 0;);
}
catch(){
/*
	prstr("Interrupt\n");
*/
	done3(01);
}
fpecatch(){
	prstrfl("Floating Exception.\n");
	signal(SIGFPE,fpecatch);
}
kcatch(){
	signal(SIGKILL,1);
	done3(01);
}
init1(a)
char a;	/* first name of formatter name from command line */
{
	register char *p, *q;
	register i;

	if((suffid=open(suftab,0)) < 0){
		prstr("Cannot open suftab.\n");
		exit(-1);
	}
	seek(suffid,16,0);
	read(suffid,sufind,2*26);

	/* create macro/string temp file */
	p = mktemp("/tmp/taXXXXX");
	if(a == 'a')p = &p[5];
	if((close(creat(p, 0600))) < 0){
		prstr("Cannot create macro/string temp file.\n");
		exit(-1);
	}
	ibf = open(p, 2);

	/* Create environment temp file */
	q = mktemp("/tmp/evXXXXX");
	if(a == 'a')q = &q[5];
	if(close(creat(q,0600)) < 0){
		prstr("Cannot create environment temp file.\n");
		exit(-1);
	}
	ibfe = open(q,2);

	/* init output translation table */
	for(i=256; --i;)trtab[i]=i;
	trtab[UNPAD] = ' ';
	mchbits();

	/* init macro/string temp file block list */
	for(i=0;i<NBLIST;i++)blist[i] = -1;

	/* init macro/string temp file ptrs */
	offset = -1;  /* write ptr */
	ip = -1;  /* read ptr */

	/* level 0 diversion has output ptr = -1 */
	dip->op = -1;

	/* init macro/string name table ptrs to definitions */

	for(i=0;i < NM;i++)
		if(contab[i].f == 0) contab[i].f = -1;
	/* save temp file names */
	if(a != 'a'){
		unlkms = p;  /* to unlink macro/string temp file */
		unlkev = q;  /* to unlink environment temp file */
	}
}
init2()
{
	register i,j;
	extern int block;

	ttyod = 2;
	if(((i=ttyn(j=0)) != 'x') ||
	   ((i=ttyn(j=1)) != 'x') ||
	   ((i=ttyn(j=2)) != 'x')
	  )ttyx[8]=i;
	iflg = j;
	gtty(j,ttys);
	ttysave = ttys[2];
	if(ascii)mesg(0);	/* only squelch messages when going to a terminal as ascii */

/*
	if((!ptid) && (!waitf)){
		if((ptid = open(ptname,1)) < 0){
			prstr("Typesetter busy.\n");
			done3(-2);
		}
	}
*/
	for(i=NEV; i--;)write(ibfe, &block, EVS*2);
	olinep = oline;
	ibufp = eibuf = ibuf;
	v.hp = ioff = init = 0;
	v.nl = -1;
	cvtime();
	frame = stk = setbrk(DELTA);
	nxf = frame + STKSIZE;
	nx = mflg;
}
cvtime(){

	int tt[2];
	register i;

	time(tt);
	sub1(tt,3600*ZONE);	/*5hrs for EST*/
	v.dy = ldiv(tt[0],tt[1],60*60*8)/3 + 1;
	v.dw = (v.dy + 3)%7 + 1;
	for(v.yr=70;; v.yr++){
		if((v.yr)%4)ms[1]=28;else ms[1]=29;
		for(i=0;i<12;){
			if(v.dy<=ms[i]){
				v.mo = i+1;
				return;
			}
			v.dy =- ms[i++];
		}
	}
}
cnum(a)
char *a;
{
	register i;

	ibufp = a;
	eibuf = -1;
	i = atoin();
	ch = 0;
	return(i);
}
mesg(f)
int f;
{
	static int mode;

	if(!f){
		stat(ttyx,cbuf);
		mode = cbuf[2];
		chmod(ttyx,mode & ~077);
	}else{
		chmod(ttyx,mode);
	}
}
prstrfl(s)
char *s;
{
	flusho();
	prstr(s);
}
prstr(s)
char *s;
{
	register i;

	for(i=0;*s;i++)s++;
	write(ttyod,s-i,i);
}
control(a,b)
int a,b;
{
	register i,j;
	int nisk, nip;

	i = a;
	/* find macro name */
	if((i == 0) || ((j = findmn(i)) == -1))return(0);
	if(contab[j].rq & MMASK){
		*nxf = 0;
		if(b)collect();
		flushi();
		/* sets macro as new input */
		boff(contab[j].f, &nisk, &nip);
		pushi(nisk, nip);
		/* return positive if macro exists, zero if not */
		if(contab[j].f == -1)
			return(0);
		else
			return(1);
	}else{
		if(!b)return(0);
		return((*contab[j].f)(0));
	}
}

getrq(){
	register i,j;

	if(((i=getach()) == 0) ||
	   ((j=getach()) == 0))goto rtn;
	i =| (j<<BYTE);
rtn:
	return(i);
}
getch(){
	register int i, j, k;

	level++;
g0:
	if(ch){
		if(((i = ch) & CMASK) == '\n')nlflg++;
		ch = 0;
		level--;
		return(i);
	}

	if(nlflg){
		level--;
		return('\n');
	}

	if((k = (i = getch0()) & CMASK) != ESC){
		if(i & MOT)goto g2;
		if(k == FLSS){
			copyf++; raw++;
			i = getch0();
			if(!fi)flss = i;
			copyf--; raw--;
			goto g0;
		}
		if(k == RPT){
			setrpt();
			goto g0;
		}
		if(!copyf){
			if((k == 'f') && lg && !lgf){
				i = getlg(i);
				goto g2;
			}
			if((k == fc) || (k == tabch) || (k == ldrch)){
				if((i=setfield(k)) == 0)goto g0; else goto g2;
			}
			if(k == 010){
				i = makem(-width(' ' | chbits));
				goto g2;
			}
		}
		goto g2;
	}
	k = (j = getch0()) & CMASK;
	if(j & MOT){
		i = j;
		goto g2;
	}
/*
	if(k == tdelim){
		i = TDELIM;
		tdelim = IMP;
		goto g2;
	}
*/
	switch(k){

		case '\n':	/*concealed newline*/
			goto g0;
		case 'n':	/*number register*/
			setn();
			goto g0;
		case '*':	/*string indicator*/
			setstr();
			goto g0;
		case '$':	/*argument indicator*/
			seta();
			goto g0;
		case '{':	/*LEFT*/
			i = LEFT;
			goto gx;
		case '}':	/*RIGHT*/
			i = RIGHT;
			goto gx;
		case '"':	/*comment*/
			while(((i=getch0()) & CMASK ) != '\n');
			goto g2;
		case ESC:	/*double backslash*/
			i = eschar;
			goto gx;
		case 'e':	/*printable version of current eschar*/
			i = PRESC;
			goto gx;
		case ' ':	/*unpaddable space*/
			i = UNPAD;
			goto gx;
		case '|':	/*narrow space*/
			i = NARSP;
			goto gx;
		case '^':	/*half of narrow space*/
			i = HNSP;
			goto gx;
		case '\'':	/*\(aa*/
			i = 0222;
			goto gx;
		case '`':	/*\(ga*/
			i = 0223;
			goto gx;
		case '_':	/*\(ul*/
			i = 0224;
			goto gx;
		case '-':	/*current font minus*/
			i = 0210;
			goto gx;
		case '&':	/*filler*/
			i = FILLER;
			goto gx;
		case 'c':	/*to be continued*/
			i = CONT;
			goto gx;
		case ':':	/*lem's char*/
			i = COLON;
			goto gx;
		case '!':	/*transparent indicator*/
			i = XPAR;
			goto gx;
		case 't':	/*tab*/
			i = '\t';
			goto g2;
		case 'a':	/*leader (SOH)*/
			i = LEADER;
			goto g2;
		case '%':	/*ohc*/
			i = OHC;
			goto g2;
		case '.':	/*.*/
			i = '.';
		gx:
			i = (j & ~CMASK) | i;
			goto g2;
	}
g1:
	if(!copyf)
		switch(k){

			case 'p':	/*spread*/
				spread++;
				goto g0;
			case '(':	/*special char name*/
				if((i=setch()) == 0)goto g0;
				break;
			case 's':	/*size indicator*/
				setps();
				goto g0;
			case 'f':	/*font indicator*/
				setfont(0);
				goto g0;
			case 'w':	/*width function*/
				setwd();
				goto g0;
			case 'v':	/*vert mot*/
				vmot();
				goto g0;
			case 'h': 	/*horiz mot*/
				hmot();
				goto g0;
			case 'z':	/*zero with char*/
				i = setz();
				break;
			case 'l':	/*hor line*/
				setline();
				goto g0;
			case 'L':	/*vert line*/
				setvline();
				goto g0;
			case 'b':	/*bracket*/
				setbra();
				goto g0;
			case 'o':	/*overstrike*/
				setov();
				goto g0;
			case 'k':	/*mark hor place*/
				if((i=findr(getsn())) == -1)goto g0;
				vlist[i] = v.hp;
				goto g0;
			case 'j':	/*mark output hor place*/
				if(!(i=getach()))goto g0;
				i = (i<<BYTE) | JREG;
				break;
			case '0':	/*number space*/
				i = makem(width('0' | chbits));
				break;
			case 'x':	/*extra line space*/
				if(i = xlss())break;
				goto g0;
			case 'u':	/*half em up*/
			case 'r':	/*full em up*/
			case 'd':	/*half em down*/
				i = sethl(k);
				break;
			default:
				i = j;
		}
	else{
		ch0 = j;
		i = eschar;
	}
g2:
	if((i & CMASK) == '\n'){
		nlflg++;
		v.hp = 0;
		if(ip == -1)v.cd++;
	}
	if(!--level){
		j = width(i);
		v.hp =+ j;
		cwidth = j;
	}
	return(i);
}
char ifilt[32] {0,001,002,003,0,005,006,007,010,011,012};
getch0(){
	register int i, j, k;

	if(ch0){i=ch0; ch0=0; return(i);}
	if(nchar){nchar--; return(rchar);}

again:
	if(cp){
		if((i = *cp++) == 0){
			cp = 0;
			goto again;
		}
	}else if(ap){
		if((i = *ap++) == 0){
			ap = 0;
			goto again;
		}
	}else if(ip != -1){
		if(ip == -2)i = rdtty();
		else i = rbf();
	}else{
		if(donef)done(0);
		if(nx || ((ibufp >= eibuf) && (ibufp != -1))){
			if(nfo)goto g1;
		g0:
			if(nextfile()){
				if(ip != -1)goto again;
				if(ibufp < eibuf)goto g2;
			}
		g1:
			nx = 0;
			if((j=read(ifile,ibuf,IBUFSZ)) <= 0)goto g0;
			ibufp = ibuf;
			eibuf = ibuf + j;
			if(ip != -1)goto again;
		}
	g2:
		i = *ibufp++ & 0177;
		ioff++;
		if(i >= 040)goto g4; else i = ifilt[i];
	}
	if(raw)return(i);
	if((j = i & CMASK) == IMP)goto again;
	if((i == 0) && !init)goto again;
g4:
	if((copyf == 0) && ((i & ~BMASK) == 0) && ((i & CMASK) < 0370))
		i =| chbits;
	if(exfont && !(i & MOT)) {
		i =& ~FONT_CNTRL;
		i =| (FONT_CNTRL & chbits);
	}
	if(expt && !(i & MOT)) {
		i =& ~PT_CNTRL;
		i =| (PT_CNTRL & chbits);
	}
	if((i & CMASK) == eschar)i = (i & ~CMASK) | ESC;
	return(i);
}
nextfile(){
	register char *p;

n0:
	if(ifile)close(ifile);
	if(nx){
		p = nextf;
		if(*p != 0)goto n1;
	}
	if(ifi > 0){
		if(popf())goto n0; /*popf error*/
		return(1); /*popf ok*/
	}
	if(rargc-- <= 0)goto n2;
	p = (argp++)[0];
n1:
	if((p[0] == '-') && (p[1] == 0)){
		ifile = 0;
	}else if((ifile=open(p,0)) < 0){
		prstr("Cannot open ");
		prstr(p);
		prstr("\n");
		nfo =- mflg;
		done(02);
	}
	nfo++;
	ioff = v.cd = 0;
	return(0);
n2:
	if((nfo =- mflg) && !stdi)done(0);
	nfo++;
	v.cd = ioff = ifile =  stdi = mflg = 0;
	return(0);
}
popf(){
	register i, *p, *q;

	ioff = offl[--ifi];
	ip = ipl[ifi];
	if((ifile = ifl[ifi]) == 0){
		p = xbuf;
		q = ibuf;
		ibufp = xbufp;
		eibuf = xeibuf;
		while(q < eibuf)*q++ = *p++;
		return(0);
	}
	if((seek(ifile,ioff & ~(IBUFSZ-1),0) < 0) ||
	   ((i = read(ifile,ibuf,IBUFSZ)) < 0))return(1);
	eibuf = ibuf + i;
	ibufp = ibuf;
	i = ttyn(ifile);
	if(i == '?') {
		prstrfl("ttyn error\n");
		exit(10);
	}
	if(i == 'x')
		if((ibufp = ibuf + (ioff & (IBUFSZ-1)))  >= eibuf)return(1);
	return(0);
}
flushi(){
	if(nflush)return;
	ch = 0;
	if((ch0 & CMASK) == '\n')nlflg++;
	ch0 = 0;
	copyf++;
	while(!nlflg){
		if(donef && (frame == stk))break;
		getch();
	}
	copyf--;
	v.hp = 0;
}
getach(){
	register i;

	lgf++;
	if(((i = getch()) & MOT) ||
	    ((i&CMASK) == ' ') ||
	    ((i&CMASK) == '\n')||
	    (i & 0200)){
			ch = i;
			i = 0;
	}
	lgf--;
	return(i & 0177);
}
casenx(){
	lgf++;
	skip();
	getname();
	nx++;
	nextfile();
	nlflg++;
	ip = -1;
	ap = nchar = pendt = 0;
	frame = stk;
	nxf = frame + STKSIZE;
}
/* Extend Font -- Superimpose present font over previous one from
  diversion */
casexf() {
	register i;


	i = 0;
	skip();
	i = max(atoin(), 0);
	if(nonumb)
		i = 0;
	if(i)
		i = 1;
	exfont = i;
}

casexp() {
	register i;

	i = 0;
	skip();
	i = max(atoin(),0);
	if(nonumb)
		i = 0;
	if(i)
		i = 1;
	expt = i;
}

getname(){
	register int i, j, k;

	lgf++;
	for(k=0; k < (NS-1); k++){
		if(((j=(i=getch()) & CMASK) <= ' ') ||
			(j > 0176))break;
		nextf[k] = j;
	}
	nextf[k] = 0;
	ch = i;
	lgf--;
	return(nextf[0]);
}
caseso(){
	register i, *p, *q;

	lgf++;
	if(skip() || !getname() || ((i=open(nextf,0)) <0) ||
		(ifi >= NSO))return;
	flushi();
	ifl[ifi] = ifile;
	ifile = i;
	offl[ifi] = ioff;
	ioff = 0;
	ipl[ifi] = ip;
	ip = -1;
	nx++;
	nflush++;
	if(!ifl[ifi++]){
		p = ibuf;
		q = xbuf;
		xbufp = ibufp;
		xeibuf = eibuf;
		while(p < eibuf)*q++ = *p++;
	}
}
getpn(a)
char *a;
{
	register i, neg;
	LONG0 atoi1();

	if((*a & 0177) == 0)return;
	neg = 0;
	ibufp = a;
	eibuf = -1;
	noscale++;
	while((i = getch() & CMASK) != 0)switch(i){
		case '+':
		case ',':
			continue;
		case '-':
			neg = MOT;
			goto d2;
		default:
			ch = i;
		d2:
			i = atoi1();
			if(nonumb)goto fini;
			else{
				*pnp++ = i | neg;
				neg = 0;
				if(pnp >= &pnlist[NPN-2]){
					prstr("Too many page numbers\n");
					done3(-3);
				}
			}
		}
fini:
	if(neg)*pnp++ = -2;
	*pnp = -1;
	ch = noscale = print = 0;
	pnp = pnlist;
	if(*pnp != -1)chkpn();
}
setrpt(){
	register i, j;

	copyf++;raw++;
	i = getch0();
	copyf--;raw--;
	if((i < 0) ||
	   (((j = getch0()) & CMASK) == RPT))return;
	rchar = j;
	nchar = i & BMASK;
}

#

/*
 *				T T Y N
 *
 *	     Routine to find the teletype name corresponding to
 *	an open file descriptor by comparing device and inode
 *	numbers with the teletypes listed in /dev.  If no match
 *	is found, return 'x'.  If /dev is inaccessible, return '?'.
 *
 *	Gerhard A. Harrop, 16 Aug. 1977
 */

# define TYPE	060000
# define CHAR	020000

struct	statbuf {
	int	device;
	int	inumber;
	int	flags;
	char	nlinks;
	char	uid;
	char	gid;
	char	size0;
	int	size1;
	int	addr[8];
	long	actime;
	long	modtime;
};

struct	direct {
	int	d_inum;
	char	name[14];
};

ttyn(fildes)
int	fildes;
{
	int		dev, tty_dev;
	register int	tty_inum, count;
	struct direct	dir[32];	/* 512 bytes. */
	register struct direct	*dirp;
	struct	statbuf sbuf;
	extern	char	ttyc();

	if (fstat(fildes, &sbuf) < 0 || (sbuf.flags & TYPE) != CHAR)
					return('x');

	tty_dev = sbuf.device;
	tty_inum = sbuf.inumber;   /* Save these for later. */

	if (stat("/dev", &sbuf) < 0) return('?');   /* No /dev ! */
	if (tty_dev != sbuf.device) return('x');
		/* To fix bug in which files not on the root
		   were made to look like teletypes. */
	if ((dev = open("/dev", 0)) < 0) return('?');	/* No /dev ! */

	while (count = read(dev, &dir, 512)) {
		count =/ 16;	/* How many entries? */
		for (dirp = dir; count--; dirp++) {
			if (dirp->d_inum != tty_inum) continue;

			/* This is it -- we've got the right inode. */
			if (dirp->name[0] != 't' ||
			    dirp->name[1] != 't' ||
			    dirp->name[2] != 'y' ||	   /* We'll check
						    name[3] in a moment. */
			    dirp->name[4] != 0) return('x');
			/* But it's something other than a tty. */
			if (dirp->name[3]) return(dirp->name[3]);
				else	   return(0);
				/* To handle the case of /dev/tty  */
		}
	}

	return('x');	/* Doesn't appear in /dev at all. */
}
