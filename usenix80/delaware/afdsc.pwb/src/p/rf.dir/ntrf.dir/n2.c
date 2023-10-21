#include "tnt.h"
#include "tdef.h"
#include "t.h"

/*
 * troff2.c
 *
 *
 * There are two formats for output words:
 *
 * Motion Word:  15 - motion indicator (on)
 *               14 - vertical motion indicator
 *               13 - negative motion indicator
 *               12 - rule motion indicator
 *               11 - amount of motion in units (bits 0-11)
 *
 *
 * Character Word:  15 - motion indicator (off)
 *                  14 - point size (bits 11-14)
 *                  13 -
 *                  12 -
 *                  11 -
 *                  10 - font (bits 9-10)
 *                   9 -
 *                   8 - zero width indicator
 *                   7 - actual character (bits 0-7)
 *
 *
 * output, cleanup
 *
 *		R E V I S I O N   H I S T O R Y
 *
 *	MJM	08/09/77	/usr/jfo/tr.act renamed as 
 *				/usr/txt/tr.act
 *
 *	MJM	10/03/77	Additional commenting added.
 *
 *	JCP	1/1/78		Word Markers added, these are
 *				ignored by pchar().  
 *
 *	DFC	7/26/78		"Pipe not created" message removed.
 *
 */

extern char obuf[OBUFSZ];
extern char *obufp;
extern int dilev;
extern int *dip;
extern int eschar;
extern int tlss;
extern int tflg;
extern int ascii;
extern int print;
extern char trtab[];
extern int waitf;
extern char ptname[];
extern int ptid;
extern int offset;
extern int em;
extern int ds;
extern int ip;
extern int mflg;
extern int woff;
extern int nflush;
extern int lgf;
extern int app;
extern int nfo;
extern int donef;
extern int *frame;
extern int *stk;
extern int *pendw;
extern int nofeed;
extern int trap;
extern int ttys[3];
extern int quiet;
extern int pendnf;
extern int ndone;
extern int lead;
extern int ralss;
extern int paper;
extern int gflag;
extern int *nxf;
extern char *unlkms;
extern char *unlkev;
extern char *nextf[];
extern int pipeflg;
extern int ejf;
extern int no_out;
extern int level;
extern int xxx;
extern int stop;
int toolate;
int error;

pchar(c)
int c;
{
	register i, j;

	if((i=c) & MOT) {
		pchar1(i);
		return;
	}
	/* Toth Diversion bug code */
	if((i == ENDWORD) && !(dip -> op)) {
		return;
	}
	switch(j = i & CMASK){
		case 0:
		case IMP:
		case RIGHT:
		case LEFT:
			return;
		case HX:
			j = (tlss>>9) | ((i&~0777)>>3);
			if(i & 040000){
				j =& ~(040000>>3);
				if(j > dip->blss)dip->blss = j;
			}else{
				if(j > dip->alss)dip->alss = j;
				ralss = dip->alss;
			}
			tlss = 0;
			return;
		case LX:
			tlss = i;
			return;
		case PRESC:
			if(dip->op == -1)j = eschar;
		default:
			i = (trtab[j] & BMASK) | (i & ~CMASK);
	}
	pchar1(i);
}

/*
 *	Output the requested motion for the character
 */
pchar1(c)
int c;
{
	register i, j, *k;
	extern int chtab[];

	j = (i = c) & CMASK;
	/* write to diversion */
	if(dip->op > -1){
		wbf(i);
		dip->op = offset;
		return;
	}
	if(!tflg && !print){
		if(j == '\n')dip->alss = dip->blss = 0;
		return;
	}
	if(no_out || (j == FILLER))return;

/*	Fix to George Toth Ligature Bug is to remove the conditional
 *	compilation here.
 */
#ifndef NROFF
	if(ascii){
		if(i & MOT){
			oput(' ');
			return;
		}
		if(j < 0177){	/* Normal ASCII characters */
			oput(i);
			return;
		}
		switch(j){	/* special characters */
			case 0200:
			case 0210:
				oput('-');
				break;
			case 0211:
				oputs("fi");
				break;
			case 0212:
				oputs("fl");
				break;
			case 0213:
				oputs("ff");
				break;
			case 0214:
				oputs("ffi");
				break;
			case 0215:
				oputs("ffl");
				break;
			default:
				for(k=chtab; *++k != j; k++)
					if(*k == 0)return;
				oput('\\');
				oput('(');
				oput(*--k & BMASK);
				oput(*k >> BYTE);
		}
	}else
#endif
	/* send out to printing device (via n10.c ) */
	ptout(i);
}

oput(i)
char i;
{
	*obufp++ = i;
	if(obufp == (obuf + OBUFSZ + ascii - 1))flusho();
}

oputs(i)
char *i;
{
	while(*i != 0)oput(*i++);
}

flusho(){
	if(!ascii)*obufp++ = 0;
	if(!ptid){
		while((ptid=open(ptname,1)) < 0){
			if(++waitf <=2)prstr("Waiting for Typesetter.\n");
			sleep(15);
		}
	}
	if(no_out == 0){
	/* write on typesetter */
	toolate =+ write(ptid, obuf, obufp-obuf);
	}
	obufp = obuf;	/* reset output pointer */
}
/*
 *	Also called by '.ex' with arg of 0.
 */

done(x) int x;{
	register i;

	error =| x;
	level = 0;
	app = ds = lgf = 0;

	/*
	 * If end macro set in "em" by '.em' command, force execution
	 * end-macro.
	 */
	if(i=em){
		donef = -1;
		em = 0;
		if(control(i,0))reset();
		/* no return if valid macro */
	}
	if(!nfo)done3(0);
	mflg = 0;
	dip = &d[0];
	if(woff)wbt(0);
	if(pendw)getword(1);

	pendnf = 0;
	if(donef == 1)done1(0);
	donef = 1;
	ip = 0;
	frame = stk;
	nxf = frame + STKSIZE;

	if(!ejf)tbreak();
	nflush++;
	eject(0);
	reset();
}

done1(x)
int x;
{
	error =| x;
	if(v.nl){
		trap = 0;
		eject(0);
		reset();
	}
	if(nofeed){
		ptlead();
		flusho();
		done3(0);
	}else{
		if(!gflag)lead =+ TRAILER;
		done2(0);
	}
}

done2(x) int x; {
	register i;

	ptlead();
#ifndef NROFF
	if(!ascii){
		oput(T_INIT);
		oput(T_STOP);
		if(!gflag)for(i=8; i>0; i--)oput(T_PAD);
	}
#endif
	flusho();
	done3(x);
}
done3(x) int x;{
	error =| x;
	signal(SIGINT, 1);
	signal(SIGKILL,1);
	unlink(unlkms);  /* clean up macro/string temp file */
	unlink(unlkev);  /* and environment temp file */
#ifdef NROFF
	twdone();
#endif
	if(quiet){
		ttys[2] =| ECHO;
		stty(0,ttys);
	}
	if(ascii && ttyn(1) != 'x')mesg(1);
#ifndef NROFF
	report();
#endif
	exit(error);
}
edone(x) int x;{
	frame = stk;
	nxf = frame + STKSIZE;
	ip = 0;
	done(x);
}
#ifndef NROFF
report(){
	register i;
	struct {int use; char uid;} a;

	if((ptid != 1) && paper && ((i=open("/usr/txt/tr.acct",1)) >0)){
		seek(i,0,2);
		a.use = paper;
		a.uid = getuid();
		write(i,&a,3);
		close(i);
	}
}
#endif
#ifdef NROFF
casepi(){
	register i;
	int id[2];
	char *cmdarg[10], cmdbuf[100], **cmdptr;
	register char *f, *p;

	/* initialize */
	f = &cmdbuf[0];	/* pointer to free spot in cmdbuf */
	cmdptr = &cmdarg[0];	/* command arguments pointer and base address */

	/* get arguments */

	if(skip() || !getname())	return;

	*cmdptr++ = f;
	p = nextf;
	while(*f++ = *p++);

	while(!skip() && getname())	/* until newline */
	{
		*cmdptr++ = f;
		p = nextf;
		while(*f++ = *p++);
	}
	*cmdptr = 0;
	if(toolate ||  (pipe(id) == -1) ||
	   ((i=fork()) == -1)){
/*		prstr("Pipe not created.\n");	*/	/* not needed - too much of a nuisance */
		return;
	}
	ptid = id[1];
	if(i>0){
		close(id[0]);
		toolate++;
		pipeflg++;
		stop = 0;	/* dont stop between pages going to a pipe */
		return;
	}
	close(0);
	dup(id[0]);
	close(id[1]);
	execv(*cmdarg, cmdarg);
	prstr("Cannot exec: ");
	prstr(cmdarg[0]);
	prstr("\n");
	exit(-4);
}
strcpy(to,from)
char *to, *from;
{
	while(*to++ = *from++);
}
#endif
