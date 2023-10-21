/*	SCAME io.c			*/

/*	Revision 1.0.2  1985-02-19	*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

/* This file contains functions for input/ouput,
 * and other miscellaneous primitives.
 */

# include "scame.h"
# include "k_funcs.h"
#ifdef S5
#  include <termio.h>
#else
#  include <sgtty.h>
#endif

/* Variables for saving original terminal parameters */
#ifdef S5
  struct termio templ, ttparms;
#else
  struct sgttyb templ, ttparms;
  struct tchars oldtchars, tmptchars = { -1, 28, -1, -1, -1, -1 };
  struct ltchars oldltchars, tmpltchars = { -1, -1, -1, -1, -1, -1 };
#endif

/* Terminal attributes */
char tbuf[1024];
Bool BS;
char *AL, *BC, *CD, *CL, *CE, *CM, *CR, *CS, *DL, *UP, *HO, *KE, *KS, *ND, *NL,
	*SE, *SO, *SR, *TI, *TE;
 
short ospeed;

pchar(c)
char c;
{
/*
int ii,jj;
	ii=0;
	for (jj=0; jj<=7;jj++) if (c & (1<<jj)) ii++;
	if (~ (ii & 1)) c += 128;
*/
	c &= (char) 127;
	write(1, &c, 1);
}

putchar_x(c)
char c;
{
	putchar(c);
}

tputs_x(s)
char *s;
{
	tputs(s, 0, putchar_x);
	fflush(stdout);
}

pstring(s)
char *s;
{
	write(1, s, strlen(s));
}

int hpos, vpos;

ttyraw()
{
	/* set tty raw mode */
#ifdef S5
	ioctl(0, TCGETA, &templ);
	ttparms = templ;
	ttparms.c_iflag &= ~(ICRNL | IXON);
	ttparms.c_oflag &= ~ONLCR;
	ttparms.c_lflag &= ~(ISIG | ICANON | ECHO);
	ttparms.c_cc[VEOF] = 1;	/* When ICANON is off, EOF char value means */
				/* number of characters to buffer on input */
	ioctl(0, TCSETA, &ttparms);
#else
	gtty(0,&templ);
	ttparms = templ;
	ttparms.sg_flags |= RAW;
	ttparms.sg_flags &= ~(CRMOD | ECHO);
	stty(0,&ttparms);
	ioctl(0, TIOCGETC, &oldtchars);
	ioctl(0, TIOCGLTC, &oldltchars);
#endif
	signal(SIGHUP, hangup);
	signal(SIGINT,SIG_IGN);
#ifdef SIGTSTP
	signal(SIGTSTP, stop);
#endif
}

push()
{
int status;
register int i;
char *shell;
	if (fork()==0) {
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		setupterm(FALSE);
		echo("Return to SCAME with ");
#ifdef TIOCGETC
		outchar(oldtchars.t_eofc, TRUE);
#else
#ifdef S5
		outchar(templ.c_cc[VEOF], TRUE);
#else
		outchar('\004', TRUE);	/* ^D */
#endif
#endif
		pchar('\n');
		shell = getenv("SHELL");
		if (shell == NIL || *shell == '\0') shell = "/bin/sh";
		execl(shell,"sh <from scame>",0);
		echo("no shell?");
		exit(1);
	}
	wait(&status);
	setupterm(TRUE);
	cls();
	vpos=0; hpos=0;
	upd2();
	modeline();
	for (i=0; i<=screenlen-2; i++) *oldscreen[i]='\0';
	killing=FALSE;
}

ttycbreak()
{
	/* set tty cbreak mode */
#ifdef S5
	if (gvars.cbreak_mode)
		ttparms.c_lflag |= ISIG;
	else	ttparms.c_lflag &= ~ISIG;
#else
#ifdef CBREAK
	if (gvars.cbreak_mode)
		ttparms.sg_flags = CBREAK | (ttparms.sg_flags & ~ RAW);
	else	ttparms.sg_flags = RAW | (ttparms.sg_flags & ~ CBREAK);
	stty(0,&ttparms);
	ioctl(0, TIOCSETC, &tmptchars);
	ioctl(0, TIOCSLTC, &tmpltchars);
#endif
#endif
}

char *termtyp;

char tBuf[1024];	/*HMS: added */
char *tBufptr = tBuf;
char **tbufptr = &tBufptr;

gettermtype()
{
	char tptr[1024];
	int rc;

	termtyp=getenv("TERM");
	switch((rc = tgetent(tptr,termtyp))) {
	    case -1:
		printf("Can't read termcap\n"); exitscame(1);
	    case 0:
		printf("Can't find your terminal type (%s) in termcap\n", termtyp);
		exitscame(1);
	}
printf("tgetent returned %d\r\n");
/*	tbufptr=tbuf;	/* original */
	gvars.hackmatic = tgetflag("km", tbufptr);	/* terminal has META */
printf("tbufptr=%#o\n", *tbufptr);
	gvars.system_output_holding = tgetflag("xx"/* , tbufptr */);	/* no ^S/^Q */
	AL = tgetstr("al", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	BC = tgetstr("bc", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	BS = tgetflag("bs" /* ,tbufptr */);
	CD = tgetstr("cd", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	CL = tgetstr("cl", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	CE = tgetstr("ce", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	CM = tgetstr("cm", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	CS = tgetstr("cs", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	DL = tgetstr("dl", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	HO = tgetstr("ho", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	KE = tgetstr("ke", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	KS = tgetstr("ks", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	ND = tgetstr("nd", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	NL = tgetstr("nl", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	SO = tgetstr("so", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	SE = tgetstr("se", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	SR = tgetstr("sr", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	TI = tgetstr("ti", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	TE = tgetstr("te", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	UP = tgetstr("up", tbufptr);
printf("tbufptr=%#o\n", *tbufptr);
	screenwidth = min(tgetnum("co"), SCRDIMX);
	screenlen = min(tgetnum("li"), SCRDIMY);

printf("screenwidth: %d, screenlen: %d, CL=%#o, CM=%#o, HO=%#o, ND=%#o, UP=%#o\n", screenwidth, screenlen, CL, CM, HO, ND, UP);
	if (screenwidth < 2 || screenlen < 4 ||
	    !CL || (!CM && !(HO && ND)) || !UP) {
		printf("Sorry, but your terminal appears to be too dumb.\n");
		exitscame(1);
	}
}

setupterm(flg)			/* If flg==TRUE, set line in raw mode and */
Bool flg;			/* initialize the terminal,otherwise restore */
{
    if (flg) {
    	ttyraw();
	if (KS) tputs_x(KS);	/* Keypad on */
	if (TI) tputs_x(TI);	/* start CM mode */
	if (!gvars.hackmatic) {
		gvars.cbreak_mode = 1L;
		ttycbreak();
	}
    }
    else {
	if (KE) tputs_x(KE);	/* Restores Keypad */
	if (TE) tputs_x(TE);	/* exit CM mode */
#ifdef S5
	ioctl(0, TCSETA, &templ);
#else
	stty(0,&templ);
	ioctl(0, TIOCSETC, &oldtchars);
	ioctl(0, TIOCSLTC, &oldltchars);
#endif
    }
}

cls()				/* Clear screen */
{
	tputs_x(CL);
	hpos = vpos = 0;
}

cleol(cd_ok)
Bool cd_ok;
{				/* Clear to end of line */
	if (CE != NIL) tputs_x(CE);
	else if (cd_ok && CD != NIL) tputs_x(CD);  /* Clear to end of screen */
	else { int i;
		/* A very poor simulation, should be avoided */
		for (i=0; i<15; i++) pchar(' ');
		for (i=0; i<15; i++) pchar('\b');
	}
}

do_CM(y,x)
register int y,x;
{
int i;
extern char *tgoto();
	if (CM != NIL) tputs_x(tgoto(CM,x,y));
	else {
		tputs_x(HO);
		for (i=0; i<y; i++) {
			if (NL) tputs_x(NL);
			else pchar('\n');
		}
		for (i=0; i<x; i++) tputs_x(ND);
	}
}

cur(y,x)			/* Move cursor to line y, column x */
 register int y,x;		/* where 0 < y < screenlen-1	   */
{				/* 	 0 < x < screenwidth-1	   */
	if (x != hpos || y != vpos) {
		if (x == 0 && y == 0 && HO) tputs_x(HO);
		else if (y == vpos) {
			if (x == 0 && CR) tputs_x(CR);
			else if (x == hpos-1 && (BC || BS)) {
				if (BC) tputs_x(BC);
				else pchar('\b');
			}
			else if (x == hpos+1 && ND) tputs_x(ND);
			else do_CM(y,x);
		}
		else if (x == hpos) {
			if (y == vpos-1 && UP) tputs_x(UP);
			else if (y == vpos+1) {
				if (NL) tputs_x(NL);
				else pchar('\n');
			}
			else do_CM(y,x);
		}
		else do_CM(y,x);
		hpos = x;
		vpos = y;
	}
}

vtscroll(i,lin1)		/* Scroll region between lin1 and */
int i,lin1;			/* winbot i lines upwards */
{
register int j;
char str[10];
extern char *tgoto();
	if (CS != NIL) {
		tputs_x(tgoto(CS,winbot,lin1));
		hpos = vpos = 0;
		if (i>0) {
			cur(winbot,0);
			for (j = 0; j < i; j++)
				tputs_x(NL);
			for (j = lin1; j <= winbot-i; j++)
				strcpy(oldscreen[j],oldscreen[j+i]);
			for (j = winbot+1-i; j <= winbot; j++)
				*oldscreen[j] = '\0';
		}
		else if (i<0) {
			cur(lin1,0);
			for (j = 0; j > i; j--)
				tputs_x(SR);
			for (j = winbot; j >= lin1-i; j--)
				strcpy(oldscreen[j],oldscreen[j+i]);
			for (j = lin1; j < lin1-i; j++) *oldscreen[j] = '\0';
		}
		tputs_x(tgoto(CS,screenlen-1,0));
		hpos = vpos = 0;
	}
	else if (winbot == screenlen-3) {
		if (i > 0 && DL != NIL) {
			cur(lin1,0);
			for (j = 0; j < i; j++)
				tputs_x(DL);
			for (j = lin1; j <= screenlen-2-i; j++)
				strcpy(oldscreen[j],oldscreen[j+i]);
			for (j = screenlen-1-i; j <= screenlen-2; j++)
				*oldscreen[j] = '\0';
		}
		else if (i < 0 && AL != NIL) {
			cur(lin1,0);
			for (j = 0; j > i; j--)
				tputs_x(AL);
			if (CD != NIL) {
				cur(screenlen-2,0);
				tputs_x(CD);
			}
			for (j = screenlen-3; j >= lin1-i; j--)
				strcpy(oldscreen[j],oldscreen[j+i]);
			*oldscreen[screenlen-2] = '\0';
			for (j = lin1; j < lin1-i; j++) *oldscreen[j] = '\0';
		}
	}
}


/* Check if there is typeahead */

#ifdef PDP11
int kmemfd;

inittypeaheadcheck()
{
int c;
	if ((kmemfd = open("/dev/kmem",0))<0){
		write(2,"Can't sense KBD\r\n", 16);
		return;
	}
	lseek(kmemfd, 0140544L, 0);
	read(kmemfd, &c, 2); /* tty */
	lseek(kmemfd, (long) c + 2, 0);
}
#endif

Bool typeahead()
{
int n;
# ifdef FIONREAD
	ioctl(0, FIONREAD, &n);
# else
# ifdef PDP11
	lseek(kmemfd, -2L, 1);
	read(kmemfd,&n,2);
# else
	n = 0;
# endif
# endif
	return(n != 0 || stdin->_cnt || quiet);
}


outchar(c, verbose)
char c;
Bool verbose;
{
	if (verbose) {
		if (c & 0400) { 
			pstring("Control-");
 			c &= 0377;
		}
		if (c & 0200) { 
			pstring("Meta-");
 			c &= 0177;
		}
		switch (c) {
			case '\010': pstring("Backspace"); break;
			case '\011': pstring("Tab"); break;
			case '\012': pstring("Linefeed"); break;
			case '\r'  : pstring("Return"); break;
			case '\033': pstring("Escape"); break;
			case  ' '  : pstring("Space"); break;
			case  DEL  : pstring("Rubout"); break;
			default    : if (c < ' ' || c==DEL) { 
					pchar('^');
					hpos++;
					c = (c+64) & 0177;
				     }
				     pchar(c);
				     hpos++;
		}
	}
	else {
		if ((c < ' ' && c != '\t') || c==DEL) {
			pchar('^');
			hpos++;
			c = (c+64) & 0177;
 		}
		pchar(c);
		hpos++;
	}
}

strnout(str,len)
char *str;
int len;
{
	while(len--) outchar(*str++, FALSE);
}

strout(str)
char *str;
{
	while(*str) outchar(*str++, FALSE);
}


invmod(flg)
Bool flg;
{
	switch (flg) {
 		case TRUE: if (SO != NIL) tputs_x(SO); break;
		case FALSE:if (SE != NIL) tputs_x(SE); break;
	}
}

vt100a(asciiflg)		/* Change character set */
Bool asciiflg;
{
	if (strneq(termtyp,"VT100",5,TRUE)) {
		if (asciiflg) pchar('\017');
		else pstring("\033)1\016");
	}
}

errmes(code)
enum errcode_t code;
{
char *s;
	pchar(BELL);
	switch (code) {
		case A2W: s = "A2W Already 2 Windows"; break;
		case CCF: s = "CCF Can't Create File"; break;
		case BF:  s = "BF  Buffer Full"; break;
		case BTF: s = "BTF Buffer Table Full"; break;
		case FTB: s = "FTB File Too Big"; break;
		case FTF: s = "FTF File Table Full"; break;
		case ILA: s = "ILA Illegal Argument"; break;
		case ILQ: s = "ILQ Illegal Quantity"; break;
		case MTF: s = "MTF Macro Table Full"; break;
		case NIB: s = "NIB Character Not In Buffer"; break;
		case NKM: s = "NKM No Kbd Macro"; break;
		case NSB: s = "NSB No Such Buffer"; break;
		case NSF: s = "NSF No Such File"; break;
		case NYI: s = "NYI Function is Not Yet Implemented"; break;
		case O1W: s = "O1W Only One Window"; break;
		case RKM: s = "RKM Recursive Kbd Macro"; break;
		case SSF: s = "SSF String Space Full"; break;
		case UBK: s = "UBK Unbound Key"; break;
	}
	echo(s);
	pchar('?');
}


echo(str)
char *str;
{
	alarm(0);
	cur(screenlen-1,0);
	cleol(TRUE);
	if (str != NIL) {
		strout(str);
		echobusy = TRUE;
	}
	else echobusy = FALSE;
}

short yesorno(str)
char *str;
{
int c;
	*commandprompt = '\0';
yquery:	if (!quiet) {
		echo(str);
		strout(" (Y or N)? ");
	}
	c = upcasearr[inchar(FALSE)];
	if (!quiet)
		outchar(c, TRUE);
	if (c != 'Y' && c != 'N' && c != '\007') {
		pchar(BELL);
		goto yquery;
	}
	switch (c) {
		case 'N': return(0);
		case 'Y': return(1);
		default: return(2);
	}
}

int instring(prompt, strp, pos, notallowed, exitstr)
char *prompt, *strp;
int pos;
char *notallowed,*exitstr;
{
int c;
int i;
Bool gotit;
char tstr[80];
	*commandprompt='\0';
	gotit=FALSE;
	strcpy(tstr,strp);
	tstr[pos]= '\0';
	if (!quiet) {
		echo(prompt);
		strnout(tstr,pos);
	}
	while (!gotit) {
		c=inchar(FALSE);
		if (c == EOF)
			return c;
		if (notallowed != NIL && index(notallowed,c) != 0)
			pchar(BELL);
		else if (exitstr != NIL && index(exitstr,c) != 0) {
			tstr[pos]='\0';
			gotit = TRUE;
			if (c=='\007') { pchar(BELL); outchar(7, FALSE); }
			else if (c == '\r' && !quiet) {
				pchar(c); hpos = 0; }
		}
		else if (c == gvars.quote_char) {	/* ^Q */
			c=inchar(FALSE);
			if (pos < 80 - 1) {
				if (!quiet)
					outchar(c, FALSE);
				tstr[pos++]=c;
			}
			else pchar(BELL);
		}
		else switch (c) {
			case '\006':			/* ^F */
				if (pos < 80 - strlen(cb.filename)) {
					strcpy(&tstr[pos],cb.filename);
					if (!quiet) {
						strout(cb.filename);
						pos += strlen(cb.filename);
					}
				}
				else pchar(BELL); break;

			case '\037':			/* ^_ */
				vfile(scamelib,"instring", FALSE,FALSE,NIL);
			case '\014':			/* ^L */
				echo(prompt);
				strnout(tstr,pos);
				break;
			case '\025':			/* ^U */
			case DEL:			/* RubOut */
				i = (c == DEL) ? min(1, pos) : pos;
				while (i-- > 0)  {
					if (!quiet) {
						pchar('\b');
						pchar(' ');
						pchar('\b');
					}
					pos--;
					hpos--;
					if (tstr[pos] < ' ' ||
 					    tstr[pos] == '\0177') {
						if (!quiet) {
							pchar('\b');
							pchar(' ');
							pchar('\b');
						}
						hpos--;
					}
				} break;
			default:
				if (pos < 80 - 1) {
  					if (!quiet)
						outchar(c, FALSE);
					tstr[pos++]=c;
				}
				else pchar(BELL);
		}
	}
	strcpy(strp,tstr);
	return c;
}


static Bool timedout;

catchalarm()
{
	echo(commandprompt);
	timedout = TRUE;
}

int inchar(metaallowed)
Bool metaallowed;
{
char pipestr[1000];
char c, *p1, *p2;
struct stat s;
int ic,l;

ignore:
	if (*commandprompt != '\0' && !quiet) {
		signal(SIGALRM, catchalarm);
		alarm(2);
	}
	if (execfd >= 0) {
nextc:		if (read(execfd,&c,1) == 0) return(EOF);
		if (c == '\\') {
			if (read(execfd,&c,1) == 0) return(EOF);
			if (c == '\n') goto nextc;
		}

	}
	else {
		timedout = FALSE;
		if ((ic=getchar()) == -1)
			c = (char) getchar();
		else {
			c = (char) ic;
			alarm(0);
		}
		if (timedout) {
			if (commandprompt[strlen(commandprompt)-1] == '-')
				outchar(nupcasarr[c] & 0177, TRUE);
			else outchar(c & 0177, TRUE);
		}
	}
	if (gvars.system_output_holding && (c == '\023' || c == '\021'))
		goto ignore;
	if (!metaallowed || gvars.cbreak_mode || !gvars.hackmatic)
		c &= 0177;   	/* Strip parity */
	lastinput[lstindex] = c;
	lstindex = (lstindex + 1) % LASTINPUTSIZE;
	if (defining_kbd_mac)
		write(kbdmfd,&c,1);
	return (c & 0377);
}

unget(c)
char c;
{
	ungetc(c,stdin);
	lstindex--;
	if (lstindex < 0) lstindex = LASTINPUTSIZE - 1;
}

getline(f, s, lim, eof)
int f;
char s[];
int lim;
Bool *eof;
{
register int i, j, l;
static char c='\0';
	i=0;
	if (c != '\0' && lim--) {
		if (c == '\t') {
			lim++;
			j = i;
			while(i < j + 8 -(j & 7) && lim--) s[i++]=' ';
		}
		else s[i++]=c;
	}
	while((lim-- > 0)
 && ((l=read(f,&c,1))==1)
 && (c != '\n'))
		if (c == '\t') {
			lim++;
			j = i;
			while(i < j + 8 -(j & 7) && lim--) s[i++]=' ';
		}
		else s[i++]=c;
	*eof = (!l);
	c = s[i] = '\0';
	if (lim == 0 && c != '\n') {
 		read(f,&c,1);
		if (c == '\n') c='\0';
	}
	return(i);
}


blockmove(src, dst, len)
char *src;
register char *dst;
long len;
{
register char *p1, *p2;
#ifdef VAX
		if (len < 65536L) {
			asm("movc3 12(ap),*4(ap),*8(ap)");
			return;
		}
#endif
	if (dst > src) {
		p1 = src + len - 1;
		p2 = dst + len - 1;
		while (p2 >= dst) *p2-- = *p1--;
	}
	else if (dst < src) {
		p1 = dst + len - 1;
		p2 = src;
		while (dst <= p1) *dst++ = *p2++;
	}
}

suspend(msecs, break_if_typeahead)
long msecs;
Bool break_if_typeahead;
{
long t;
	t = msecs + time(0) * 1000;
	while(time(0) * 1000 < t && !(break_if_typeahead && typeahead()));
}

int b_getkey(s)
char *s;
{
int c;
	if(!quiet) {
		echo(s);
		strout(": ");
	}
	*commandprompt = '\0';
	c = inchar(TRUE);
	if ((c & 0177) >= 0 && (c & 0177) < 32) c = (c + '@') | 0400;
	if (disptab[c] == k_c_prefix) {
		if (!quiet)
			strout("C-");
		c = inchar(TRUE);
		if (!quiet)
			outchar(c, TRUE);
		c = b_control(c);
	}
	else if (disptab[c] == k_m_prefix) {
		if (!quiet)
			strout("M-");
		c = inchar(TRUE);
		if (!quiet)
			outchar(c, TRUE);
		c = b_meta(c);
	}
	else if (disptab[c] == k_c_m_prefix) {
		if (!quiet)
			strout("C-M-");
		c = inchar(TRUE);
		if (!quiet)
			outchar(c, TRUE);
		c = b_control(b_meta(c));
	}
	return(c);
}
		

int b_control(c)		/* Set bit nine. */
int c;
{
	if ((c & 0177) >= 0 && (c & 0177) < 32) c += '@';
	return(nupcasarr[c] | 0400);
}

int b_meta(c)			/* Set bit eight. */
int c;
{
	if ((c & 0177) >= 0 && (c & 0177) < 32) c = (c + '@') | 0400;
	return(nupcasarr[c] | 0200);
}

char *key_name(c, s, verbose)
int c;
char *s;
Bool verbose;
{
	*s = '\0';
	if (c & 0400) {
		if (verbose) strcat(s, "Control-");
		else strcat(s, "C-");
		c = c & 0377;
	}
	if (c & 0200) {
		if (verbose) strcat(s, "Meta-");
		else strcat(s, "M-");
		c = c & 0177;
	}
	if	(c == '\010') strcat(s,"Backspace");
	else if (c == '\011') strcat(s,"Tab");
	else if (c == '\012') strcat(s,"Linefeed");
	else if (c == '\r'  ) strcat(s,"Return");
	else if (c == '\033') strcat(s,"Escape");
	else if (c ==  ' '  ) strcat(s,"Space");
	else if (c ==  DEL  ) strcat(s,"Rubout");
	else {
		if (c < ' ' || c==DEL) { 
			strcat(s,"^");
				c = (c+64) & 0177;
		}
		sprintf(&s[strlen(s)],"%c",c);
	}
	return(s);
}

funcp getfuncname(prompt)
char *prompt;
{
char str[80], tstr[80], *p;
int c, i;

	/* This code should be merged with setvariable() */
	*str = '\0';
	do {
		c=instring(prompt, str, strlen(str), "\t", "\007\022\033\r ?");
		if (c == 7 || (c == 13 && *str=='\0')) {
			typing = FALSE;
			return(0);
		}
		switch (c) {
		    case '\022':		/* ^R */
			strcat(str, "^R"); break;
		    case '?':
			listmatching((struct tabstruct *)x_comtab,str); break;
		    case ' ':
			strcpy(tstr,str);
			strcat(str, " ");
			if (tablk((struct tabstruct *)x_comtab,str,0) <0) {
			    strcpy(str,tstr);
			    (void) tablk((struct tabstruct *)x_comtab,str,0);
			    if (strlen(str) == strlen(tstr))
				pchar(BELL);
			}
			if ((p=index(str+strlen(tstr), ' ')) != NIL)
				*(p+1) = '\0';
			break;
		    default:
			if ((i = tablk((struct tabstruct *)x_comtab,str,0)) <0)
				pchar(BELL);
		}
	} while (i<0 || index("\022\033 ?", c) != NIL);
	return(x_comtab[i].funcp);
}

