#include "tnt.h"
#include "tdef.h"
#include "t.h"
#include "tw.h"

/*
 * troff1.c
 * 
 * consume options, initialization, main loop,
 * input routines, escape function calling
 *
 *
 *	R E V I S I O N   H I S T O R Y
 *
 *	-who-	-when-		-why-
 *
 *	MJM 8/17/77		Commenting & debug on '.so'
 *
 *	MJM 8/19/77		Debug option '.db'
 *
 *	JCP 1/1/78		Expand Font and Expand Point added
 *				These allow point and fonts to be
 *				changed to current parameters when
 *				dumping diversions.  Previously 
 *				impossible to change parameters of
 *				diversion.
 *
 *	DFC 7/26/78		Changed flag gathering, modified -T and
 *				added -P flags.  Added Nice call.
 *
 */

/* JHU variables */
extern debug;
extern bugmode;


/* Traditional Variables */
extern int stdi;
extern int waitf;
extern int nofeed;
extern int quiet;
extern int ptid;
extern int ascii;
extern int npn;
extern int xflg;
extern int stop;
extern int stopval;
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
extern int cps;
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
extern char batchf[];   /*  Batch Variables  */
extern char *endbname;
extern int batchflg;  /*  --------------  */
extern int nfi;
#ifdef NROFF
extern char termhold[];
extern char pitchold[];
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
extern int upid;	/* for .U number register */

int ms[] {31,28,31,30,31,30,31,31,30,31,30,31};
int expt    0;
int exfont  0;

#define nextarg {++argv;--argc;}	/* function to move to next argument */
#define lastarg {--argv;++argc;}	/* function to move to last argument */



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
	 * Main loop to consume options
	 */

options:
	nextarg;	/* skip arg 0 */
	while(argc > 0 &&  **argv == '-')	/* While they are args & start with dash */
	{
	   for(;;)
	   {
		switch(*++*argv){

		case 'v':			/* videocomp-500 (GPO) */
			argv[argc] = 0;
			execv("/usr/bin/vroff",argv);
			prstr("Unable to find /usr/bin/vroff\n");
			done3(0);
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
		case 'p':
			nextarg;
			xflg = 0;
			cps = cnum(*argv);
			goto nextflg;
		case 's':
			nextarg;
			if(notnum(*argv))
			{
				lastarg;
				stop = 1;	/* default */
			}
			else
				if(!(stop = cnum(*argv)))stop++;
			if(ttyn(1) == 'x')
				stop = 0;	/* ignore if output not a tty */
			stopval = stop;		/* .S number register */
			goto nextflg;
		case 'r':
			nextarg;
			i = **argv & BMASK;
			i =| (*++*argv & BMASK)<<BYTE;
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

		/* JHU debugging option */
		case 'D':
			debug++;
			break;
#ifdef NROFF
		case 'h':
			hflg++;
			break;
		case 'z':
			no_out++;
			break;
		case 'e':
			eqflg++;
			break;
		case 'T':
			p = termhold;
			nextarg;
			q = *argv;
			if(!((*q) & 0177))break;
			while((*p++ = *q++) != 0);
			dotT++;
			goto nextflg;
		case 'P':
			contab[findmn('ph')].rq = -1;	/* disable (i.e. remove) the .ph command */
			p = pitchold;
			nextarg;
			q = *argv;
			if(!((*q)&0177)) break;
			while((*p++ = *q++)!=0);
			goto nextflg;
#endif
#ifndef NROFF
		case 'z':
			no_out++;
		case 'a':
			ascii = 1;
			nofeed++;
		case 't':
			ptid = 1;
			break;
		case 'w':
			waitf = 1;
			break;
		case 'f':
			nofeed++;
			break;
		case 'x':
			xflg = 0;
			break;
		case 'b':
			if(open(ptname,1) < 0)prstr("Busy.\n");
			else prstr("Available.\n");
			done3(0);

		/*  Batch Code -- Get file name  */
		case 'c':
			/*  last "-c" wins -- like "-m"  */
			{
			int k = 0;
			p = batchf;
			nextarg;
			q = *argv;
			while((k++ <= 40) && (*p++ = *q++) != 0);
			endbname = p = p-1;
			*p++ = ++batchflg/10 + '0';
			*p++ = batchflg%10 + '0';
			*p++ = '\0';
			}
			goto nextflg;
		/*  ------------------  */
#endif
		case '\0':
			goto nextflg;
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
	 * MAIN LOOP through which all lines pass
	 */

loop:
	copyf = lgf = nb = nflush = nlflg = 0;
	fp = frame;
	if((ip != -1) && (rbf0(iseek,ip)==0) && ejf && (*++fp <= ejl)){
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
	j = (i & CMASK);
	if( j == XPAR ) {
		copyf++;
		tflg++;
		for(;(i & CMASK) != '\n';)pchar(i = getch());
		tflg = 0;
		copyf--;
		goto loop;
	}
	if((j == cc) || (j == c2)){
		/* control character, or non-br control ch */
		if( j == c2 ) nb++;		/* set no break */

		/* consume leading spaces and tabs */
		copyf++;
		while(((j=((i=getch()) & CMASK)) == ' ') ||
			(j == '\t'));
		ch = i;		/* "un-read" this char */
		copyf--;

		/* Process macro */
		control(getrq(),1);
		flushi();	/* consume rest of line */
		goto loop;
	}

	/*
	 * Load text for a while (1 line)
	 */
lt:
	ch = i;		/* unread char just examined */
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


notnum(str)
	char *str;
{
	for(; *str != 0; str++)
		if(*str<'0' || *str>'9')
			return(1);	/* not a number */
	return(0);	/* it is a num */
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
char a;  /* first char of formatter name from command line */
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
	/* if formatter name indicates that it is a debug version
	   (first letter is 'a'), then put the temp files in
	   the current working directory and don't clean them up
	   when death sets in  */
	if(a == 'a')p = &p[5];
	if((close(creat(p, 0600))) < 0){
		prstr("Cannot create macro/string temp file.\n");
		exit(-1);
	}
	ibf = open(p, 2);

	/* create environment temp file */
	q = mktemp("/tmp/evXXXXX");
	if(a == 'a')q = &q[5];
	if(close(creat(q,0600)) < 0){
		prstr("Cannot create environment temp file.\n");
		exit(-1);
	}
	ibfe = open(q,2);

	/* init output translation table */
	for(i=256; --i;)trtab[i]=i;  /* all chars translate to themselves */
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
#ifndef NROFF
	stopval = 15;	/* default number of pages per troff batch output file */
#endif
}


init2()
{
	register i,j;
	extern int block;

	upid = getpid();	/* for .U register */
	ttyod = 2;
	if(((i=ttyn(j=0)) != 'x') ||
	   ((i=ttyn(j=1)) != 'x') ||
	   ((i=ttyn(j=2)) != 'x')
	  )ttyx[8]=i;
	iflg = j;
	gtty(j,ttys);
	ttysave = ttys[2];
	if(ascii && ttyn(1) != 'x')mesg(0);	/* don't squelch write command when going to block device */

	/*  Batch Code -- Open first file  */
	if(batchflg && !ascii)
		if((ptid = creat(batchf,0666)) == -1){
			prstr("Cannot create batch file : ");
			prstr(batchf);
			prstr("\n");
			done3(-2);
		}
	/*  ---------------  */

	if((!ptid) && (!waitf)){
		if((ptid = open(ptname,1)) < 0){
			prstr("Typesetter busy.\n");
			done3(-2);
		}
	}
	ptinit();

	/* Initialize the virtual environments */
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

/* squelch 'write' command */
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

/* Print string with output flush */
prstrfl(s)
char *s;
{
	flusho();
	prstr(s);
}


/* print string without output flush */
prstr(s)
char *s;
{
	register i;

	for(i=0;*s;i++)s++;
	write(ttyod,s-i,i);
}

char B_msg1[] = "Entry macro 'xx':  ";
char B_msg2[] = "Entry primf 'xx'\n";

/* Arrange to have a control request processed */
control(a,b)
int a,b;
{
	register i,j;
	int nisk, nip;

	i = a;

	/* find macro name */
	if((i == 0) || ((j = findmn(i)) == -1))return(0);

	/* processing is different for macros and primitives */
	if(contab[j].rq & MMASK){

		/* Macro */
		*nxf = 0;

		/* collect args */
		if(b) collect();

		/* eat rest of line */
		flushi();
		/* sets macro as new input */
		boff(contab[j].f,&nisk,&nip);
		pushi(nisk,nip);

		/* return positive if macro exists, zero if not */

		if(contab[j].f == -1)
			return(0);
		else 	return(1);
	}else{

		/* Primitive */
		if(!b)return(0);
		j = (*contab[j].f)(0);	/* Invoke magic function */
		return( j );	/* return = return of primitive func */
	}
}


/* Mash a macro request name into one word */
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
			/* control/y */
			copyf++; raw++;
			i = getch0();
			if(!fi)flss = i;
			copyf--; raw--;
			goto g0;
		}

		if(k == RPT){
			/* control/l */
			setrpt();
			goto g0;
		}

		if(!copyf){
			/* 'lg' - liigature option selected
			 * 'lgf' - THIS IS SIGNIFICANT!!!
			 *         only only test of "ligature usage
			 *         appropriate" flag.
			 */
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


	/* All characters go thru this translation */
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
	/* only do this if copy mode off */
	if(!copyf)
		switch(k){

			case 'p':	/*spread*/
				spread++;	/* used only in text() */
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
				if(i = vmot())break;
				goto g0;
			case 'h': 	/*horiz mot*/
				if(i = hmot())break;
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

/* This is what REALLY gets the characters */
getch0(){
	register int i, j, k;

	/* return re-read character */
	if(ch0){
		i=ch0;
		ch0=0;
		return(i);
	}

	/* filling out to tab position on input line */
	if(nchar){
		nchar--; 
		/* return tab or leader character */
		return(rchar);
	}

again:
	if(cp){
		/* read from number register (\n interpolation) */
		if((i = *cp++) == 0){
			/* read end marker */
			cp = 0;
			/* get a real character */
			goto again;
		}
	}else if(ap){
		/* read from macro argument stack (\$ interpolation) */
		if((i = *ap++) == 0){
			/* read end marker */
			ap = 0;
			/* get a real character */
			goto again;
		}

	}else if(ip != -1){
		/* read from standard-input (.rd request) */
		if(ip == -2)i = rdtty();
		/* read from macro/string def on temp file */
		else i = rbf();
	}else{
		/* read from current input file (ip == -1) */
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
	/* extend font mode, -- superimpose new font over old */
	/* don't due for motion chars */
	if(exfont && !(i & MOT)) {
		/* And out present font control chars */
		i =& ~FONT_CNTRL;
		/* or in new ones */
		i =| (FONT_CNTRL & chbits);
	}

	if(expt && !(i & MOT)) {
		/* and out the point bits */
		i =& ~PT_CNTRL;
		/* or in the new ones */
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

/*
 * Pop back to file which issued '.so' command
 */
popf(){
	register i, *p, *q;

	/* Restore previous file offset and input pointer in buf */
	ioff = offl[--ifi];
	ip = ipl[ifi];

	/* Process a restore to the standard input */
	if((ifile = ifl[ifi]) == 0){
		p = xbuf;
		q = ibuf;
		ibufp = xbufp;
		eibuf = xeibuf;
		while(q < eibuf)*q++ = *p++;
		return(0);
	}

	/*
	 * Process restore to file.
	 *   position disk file as before,
	 *   restore all pointer & such
	 */
	if( seek( ifile, ioff & ~(IBUFSZ-1), 0 ) < 0 ) return(1);
	if( ( i = read( ifile, ibuf, IBUFSZ )) < 0 )   return(1);

	eibuf = ibuf + i;	/* end input buffer addr */
	ibufp = ibuf;		/* input buffer pointer */

	/*
	 * ensure disk file buffer pointer does not exceed
	 * amount of data read into buffer.
	 */
	i = ttyn(ifile);
	if( i == '?' ) {
		prstrfl("ttyn error\n");
		exit(10);
	}
	if( i == 'x' ) {
		ibufp = ibuf + (ioff & (IBUFSZ-1) );
		if( ibufp > eibuf )	/* origional had ">=", but its OK if it's equal. -- DFC @AFDSC, 4/29/80 */
			return(1);	/* error */
	}

	return(0);
}



/* Consume rest of line */
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
	v.hp = 0;	/* set horiz line ptr to begin of line */
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


/* Process '.so <file>' command */
caseso(){
	register i, *p, *q;

	lgf++;

	/*
	 * skip to next field (hopefully file name)
	 * move name to 'nextf' buffer
	 * open file
	 * ensure max num of SOs not exceeded
	 */
	if(skip() || !getname() || ((i=open(nextf,0)) <0) ||
		(ifi >= NSO))return;
	flushi();

	/* save the current file status */

	/* save & load file descriptors */
	ifl[ifi] = ifile;
	ifile = i;

	/* save and reset file offset */
	offl[ifi] = ioff;
	ioff = 0;
	ipl[ifi] = ip;
	ip = -1;

	nx++;
	nflush++;

	/* if the current input file is the standard input,
	 * then save rest of buffer, as we can't seek back later.
	 */
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
