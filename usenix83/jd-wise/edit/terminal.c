/*
 * terminal control for screen editor
 */
#define testing 1

#include "conf.h"
#include "term.h"
#include "terminal.h"
#include "window.h"
#include "edit.h"
#include <stdio.h>
#ifdef eunice
#include <aout.h>
#else
#include <a.out.h>
#endif
#include "options.h"
#ifndef CERASE
#define CERASE '\0'
#define CKILL '\0'
#endif

#define flush() fflush(stdout)

char* termname;
char termfile[]="/usr/lib/termchar/termXXXXXX";
char mapname[]="MAPXXXXXX";

int curcol = -100;
int curline = -100;

getterm()		/* setup terminal char structure */
	{
	char *sp, *tp, *tp1;
	char **p;
	int i,j;
	int fd;
	struct exec header;
	char *getenv(), *sbrk();

	if((termname=getenv("TERM"))==(char *)0){
		printf("\007 environment variable TERM not set\n");
		if(dbflag==25)
			abort();
		exit(1);
		}
	for(sp=termname, tp= &termfile[22], tp1= &mapname[3]; *tp1 = *tp = *sp; sp++, tp++, tp1++)
		;	/* fix this up */
	if((fd=open(termfile,0))<0){
		printf("Can't open %s\n",termfile);
		exit(1);
		}
	read(fd,(char*)&header, sizeof header);
	read(fd,(char *)&t.a0, j=sizeof(int)*((int *)&t.zz - &t.a0));
	header.a_data -= j;
	tp=sbrk((int)header.a_data);
	lseek(fd,(long)t.aa + sizeof header,0);
	i=read(fd,tp,(int)header.a_data);
	j=tp-t.aa;
	for(p = &t.aa; p < &t.zz; p++){
		if(*p)
			*p += j;
		else	/** this should be omitted */
			*p = "";
		}
	termtype=t.ty;	/* temporary */
	nlines=t.li-3;
	/* setup maps */
	setmap(t.ma);
	setmap(getenv("MAP"));
	setmap(getenv(mapname));
	/* get original tty parameters */
	if(gtty(1,&sgtty)<0)	/* output not a tty */
#ifndef testing
		exit(1);
#else
		sgtty.sg_ospeed=14;
#endif
	ospeed=sgtty.sg_ospeed;
	if(gtty(0,(struct sgttyb*)&sgtty)<0){	/* input from script */
		sgtty.sg_erase=CERASE;
		sgtty.sg_kill=CKILL;
		}
	origmode=sgtty.sg_flags;
	ioctl(0,TIOCGETC,&tchars);	/* get special chars */
	}

deleteline(lineno)	/* delete line */
	{
	topt(lineno,0,(char*)0);
	dl();
	}

eraseline(lineno)	/* position cursor to beginning of line and clear */
	{
	topt(lineno,0,(char*)0);
	el();
	}

erasescreen()
	{
	if(*t.cl)
		dputs(t.cl);
	else{
		home();
		ep();
		}
	}

insertline(lineno)
	{
#ifdef debugging
debug(46,"insertline: lineno=%d",lineno);
#endif
	topt(lineno,0,(char*)0);
	il();
	}

restorloc()	/* restore cursor position */
	{
	if(--cursptr < 0){
		cursptr=3;
		errmsg("Cursor stack underflow");
		}
	topt((curstack[cursptr]>>8)&0377,curstack[cursptr]&0377,(char*)0);
	}

saveloc()	/* save current cursor location */
	{
	curstack[cursptr++]=curline*256+curcol;	/* temporary */
	if(cursptr > 128){
		cursptr=124;
		errmsg("Cursor stack overflow");
		}
	}

/*
 * terminal dependent primatives
 */

#ifdef cpu55
int dldelay=100;	/* was 100 */
int ildelay=100;	/* was 100 */
#else
#	ifdef vax
int dldelay=120;	/* was 60 */
int ildelay=120;	/* was 60 */
#	else
int dldelay=40;	/* delay for dl in tenths of ms */
int ildelay=40;	/* was 60 */
#	endif
#endif

short tmspc[16]={	/* tenths of a millisecond per char */
	0, 2000, 1333, 909, 743, 666, 500, 333, 166, 83, 55, 41, 20, 10, 5, 2};

delay(tms)	/* send nulls for tms/10 ms */
	register tms;
	{
	register mspc;

#ifdef debugging
debug(22,"delay: tms=%d",tms);
#endif
	if(tms==0) return;
	mspc=tmspc[ospeed];
	tms += mspc/2;
#ifdef debugging
debug(22,"delay: tms=%d mspc=%d",tms,mspc);
#endif
	for(tms /= mspc; tms>0; tms--)
		putchar('\0');
	}


dl()		/* delete current line */
	{
	dputs(t.dl);
	}

el()		/* erase to end of current line */
	{
	dputs(t.el);
	}

ep()		/* erase to end of page */
	{
if(dbflag==72) abort();
	dputs(t.ep);
	}

home()
	{
	dputs(t.ho);
	curline=curcol=0;
	}

il()		/* insert line at current location */
	{
	dputs(t.il);
	}

set4gnd()	/* set foreground intensity */
	{
	if(termtype == INFOTON || termtype==VISUAL)
		tputs("\0333");
	if(termtype == V50)
		tputs("\033T");
	}

setbgnd()	/* set background intensity */
	{
	if(termtype == INFOTON || termtype==VISUAL)
		tputs("\0334");
	if(termtype == V50)
		tputs("\033U");
	}

settabs(start,space)	/* set tabs every space cols, starting at start */
	{
	register i;

#ifdef debugging
debug(16,"settabs: flag=0%o",sgtty.sg_flags);
#endif
	tabstart=start;
	tabspace=space;
	switch(termtype){
	    case INFOTON:
		tputs("\033g");	/* clear all tabs */
	    comm:
		saveloc();
		for(i=start; i<80; i+=space)
			topt(0,i,"\0331");
		restorloc();
		hastabs++;
		sgtty.sg_flags &= ~XTABS;
/*		stty(0,(struct sgttyb*)&sgtty);
*/		ioctl(0,TIOCSETN,(struct sgttyb*)&sgtty);
		break;

	    case HP2621:
		tputs("\0333");
		goto comm;

	    default:
		hastabs=0;
	    }
	flush();
#ifdef debugging
debug(16,"settabs: flags=0%o",sgtty.sg_flags);
#endif
	}

char *
showctl(c)	/* returns string to make control char visible */
	{
	static char defstr[]="~";
	static char okstr[]=" ";
	static char string[8];

	if(' '<=c && c<0177){
		okstr[0]=c;
		return(okstr);
		}
#ifdef debugging
debug(40,"showctl: c=0%o", c&0377);
if(dbflag==42) abort();
#endif
	switch(termtype){
	    case INFOTON:
		return("\016v\017");	/* cent sign */
/*		return(defstr);	/* punt because bkgnd chars are ineradicable */

	    case HP2621:	/* underline because function display disable prints */
		if(c==0177) c='~'-'@';
		sprintf(string,"\033&dA%c\033&d@",c+'@');
		return(string);

	    case VISUAL:
	    case V50:
		return("\033Fq\033G");	/* cent sign */

	    case HEATH19:
		if(c==0177)
			sprintf(string,"\033Fi\033g");
		else
			sprintf(string,"\033p%c\033q",c+'@');
		return(string);

	    default:
		return(defstr);
	    }
	}

tinit()
	{
	register l;

	if(termtype == INFOTON)
		printf("\014");		/* this clears protected lines */
	if(termtype == HP2621){
		for(l=0; l<24; l++)
			printf("\n");	/* scroll current screen to top memory */
		printf("\033&jB");	/* display f1-f8 labels */
		flush();
		}
	if(termtype==VISUAL || termtype==V50){
		printf("\033v");	/* clear page */
		printf("\033=");	/* set alt keypad mode */
		}
	}

psync()			/* make physical position agree with virtual */
	{
	Topt(curline,curcol,(char*)0);
	}

topt(line,col,string)	/* update virtual position */
	char *string;
	{
	curline=line;
	curcol=col;
	if(string && *string)
		Topt(curline,curcol,string);
	}

Topt(line,col,string)	/* update physical position */
	int line,col;
	char *string;
	{
	register i;

/*goto tgoto;		/* something's wrong here */
	if(line==Curline && col==Curcol)	/* already there */
		goto printstr;
	else if(line==Curline+1 && col==Curcol && !(sgtty.sg_flags&CRMOD))
		putchar('\n');
	else if(line==Curline && col==0 && !(sgtty.sg_flags&CRMOD))
		putchar('\r');
	else if(line==Curline+1 && col==0 && (sgtty.sg_flags&CRMOD))
		putchar('\n');
	else if(line==Curline && col==Curcol-1)
		putchar('\b');
	else{
    tgoto:
		if(t.cu.cf & curev)
			printf(t.ca, col+t.cu.of, line+t.cu.of);
		else
			printf(t.ca, line+t.cu.of, col+t.cu.of);
		}
    printstr:
	curline=Curline=line;
	curcol=Curcol=col;
	if(string != 0){
		printf("%s",string);
		for(i=0; *string++; i++);
		Curcol += i;
		while(Curcol>79){
			Curcol -= 80;
			if(++Curline >= t.li)
				errfunc("Screen overflow");
			}
		}
	}

tputs(string)	/* necessary because puts appends a linefeed */
	register char *string;
	{
	register c;

	psync();
	while(c = *string++)
		putchar(c);
	}

trestor()
	{
	extern origmode;

	if(hastabs && (origmode&XTABS)!=XTABS)
		settabs(0,8);
	if(termtype==HP2621){
		tputs("\033&j@");	/* clear screen labels */
		}
	if(termtype==VISUAL || termtype==V50){
		tputs("\033>");	/* reset alt keypad mode */
		}
	psync();
	flush();
	}


dputs(cp)
	register char *cp;
	{
	register int i = 0;
	register int mspc10;

	if (cp == 0)
		return;

	psync();

	/*
	 * Convert the number representing the delay.
	 */
	if (isdigit(*cp)) {
		do
			i = i * 10 + *cp++ - '0';
		while (isdigit(*cp));
	}
	i *= 10;
	if (*cp == '.') {
		cp++;
		if (isdigit(*cp))
			i += *cp - '0';
		/*
		 * Only one digit to the right of the decimal point.
		 */
		while (isdigit(*cp))
			cp++;
	}

	/*
	 * The guts of the string.
	 */
	while (*cp)
		putchar(*cp++);

	/*
	 * If no delay needed, or output speed is
	 * not comprehensible, then don't try to delay.
	 */
	if (i == 0)
		return;
	delay(i);
	}

isdigit(c)
	register c;
	{
	return(c>='0' && c<='9');
	}
