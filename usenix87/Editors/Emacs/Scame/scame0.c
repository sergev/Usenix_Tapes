/*	SCAME scame0.c				*/

/*	Revision 1.0.0	1985-01-30		*/

static char *cpyrid = "@(#)Copyright (C) 1985 by Leif Samuelsson";

# include "scame.h"
# include <ctype.h>

Bool insertc(c,i)
char c;
long i;
{
register char *p1, *p2;
	killing=FALSE;
	if ((gaps > dot + GAPDIST || (long) (gape-gaps) < i) &&
		!gapto(dot + GAPDIST, GAPSIZE + i)) {
		errmes(BF);
		return (FALSE);
 	}
	blockmove(dot, dot+i, (long)(gaps-dot));
	if (gape == z) gape += i;
	z += i;
	gaps += i;
	p1 = dot;
	dot = p2 = dot+i;
	while (p1 < p2) *p1++ = c;
	updateflag = TRUE;
	cb.modified = TRUE;
	return (TRUE);
}

insertstr(str,l)
char *str;
long l;
{
register char *tdot;
register int midwindow;
	if (l == 0L) return;
	midwindow = (winbot-wintop)/3;
	tdot=dot;
	if (insertc('\0',l)) {
		while (tdot < dot) {
			*(tdot++) = *str++;
			if (*str == '\n' && (CS != NIL || DL != NIL) && cury >= winbot) {
				if (dot==z && cury==winbot+1)
					vtscroll(2*midwindow+1,wintop);
				else if  (dot != z && cury == winbot)
					vtscroll(2*midwindow,wintop);
				cury = midwindow;
			}
		}
		cb.modified = TRUE;
	}
}

indent(i)
long i;
{
char *tdot;
	if (!xarg) switch (cb.majormode) {
		case ADA:
		case C:
		case LISP:
		case PASCAL:
			tdot=dot;
			passline(-1L);
			backtoindent();
			findxy();
			i = curx+1;
			xarg = TRUE;
			dot = tdot;
			findxy();
	}
	else i++;
	if (xarg) {
		if (i > 0) {
			delhorizspace();
			findxy();
			if (curx < i) {
				while (curx+1 < i && insertc('\t',1L))
					findxy();
				if (curx >= i) {
					*(dot-1) = ' ';
					findxy();
					insertc(' ', i-curx-1);
				}
			}
		}
		else errmes(ILA);
	}
	else insertc('\t',1L);
}

indentregion(i)
long i;
{
char *tdot, *ttdot;
	tdot = min(dot, *mark);
	adjustmark();
	if (dot > *mark) exchgdotmark();
	while (dot < *mark) {
		backtoindent();
		findxy();
		xarg = TRUE;
		ttdot = dot;
		indent(curx + i);
		*mark += dot-ttdot;
		passline(1L);
	}
	*mark = tdot;
}


scroll(lines)
long lines;
{
register int x;
register int y=0;
char *tdot0;
register char *tdot;
	closegap();
	if (lines > 0){
		while (lines-- > 0 && home<z) {
			x=0;
			while(home<z && *home++ != '\n' && x++< screenwidth-1);
		}
	}
	else if (lines < 0) {
		tdot = dot;
		lines = -lines;
		while (lines-- && home > buf) {
			dot=home;
			do {
				home--;
				x = 0;
				while (home > buf && *(home-1) != '\n'
				    && x++ < screenwidth)
					home--;
				findxy();
			} while (cury == wintop && home > buf);
			while (cury > wintop + 1) {
				home++;
				findxy();
			}
		}
		dot = tdot;
 	}
}

center(i)
int i;
/* move line with cursor to be near center of window */
{
	home=dot;
	findxy();
	scroll((long)((wintop-winbot)*4/10));
}

newwindow(i)
long i;
/* Refresh window and let current line be line i if arg given */
{
int j, k;
	if (xxarg) {
		if (i < 0) i = winbot-wintop+1 + i;
		if (i >= 0 && i < winbot-wintop+1 && wintop + i != cury) {
			vtscroll((int)(cury - wintop-i),wintop);
			scroll(cury-wintop-i);
			upd2();
			modeline();
		}
	}
	else if (xarg) {
		for (j=0; j<screenwidth; j++) oldscreen[cury][j] = 128;
	}
	else {
		if (windows == 1) {
			cls(); 
			for (j=wintop; j<=winbot+1; j++) *oldscreen[j]='\0';
		}
		else for(j=wintop; j<=winbot+1; j++)
			for  (k=0; k<screenwidth; k++) oldscreen[j][k]=128;
		center();
		modeline();
	}
}

forwchar(i)
register long i;
{
	if (dot + i > z) {
		errmes(NIB);
		i = z - dot;
	}
	if (dot + i >= gaps)
		gapto(dot+i+GAPDIST, GAPSIZE);
	dot += i;
	killing=FALSE;
}

long backchar(i)
register long i;
{
	if (dot - i < buf) {
		errmes(NIB);
		i = dot - buf;
	}
	dot -= i;
	killing=FALSE;
	return (i);
}


passline(i)
register long i;
{
register char *tdot, *end;
	tdot = dot;
	if (i > 0) {
		closegap();
		end = z;
		while (i-- > 0 && tdot < end)
			while (tdot < end && *tdot++ != '\n');
	}
	else {
		end = buf;
		while (i++ < 0 && tdot > end) {
			while (tdot > end && *--tdot != '\n');
			if (*(tdot) != '\n')
				while (tdot < z && *tdot != '\n') tdot++;
		}
	}
	dot = tdot;
	killing=FALSE;
}

downline(i)
long i;
{
int y, wsize=(winbot-wintop);
register int x;
	findxy();
	y = cury;
	if (i==1
	 && y >= wintop + wsize * gvars.bottom_display_margin / 100 && dot < z)
		vtscroll((int)(wsize*(gvars.bottom_display_margin - 38)/100)+1, wintop);
	passline(i);
	findxy();
	if (i==1 && cury==y && dot==z) insertc('\n',1L);
	else {
		x = curx-1;
		while (++x < oldhpos && dot < z && *dot != '\n') {
			if (*dot == '\t')
				x = min(x + TABWID - (x&7), screenwidth - 1)-1;
			else if (*dot < ' ' || *dot == DEL) x++;
			dot++;
		}
 	}
	killing=FALSE;
}

upline(i)
long i;
{
int y=cury;
char *d=dot;
register int x;
	passline(-i);
	if (d != dot) begofline();
	findxy();
	x = curx-1;
	while (++x < oldhpos && dot < z && *dot != '\n') {
		if (*dot == '\t')
			x = min(x + TABWID - (x&7), screenwidth-1) - 1;
		else if (*dot < ' ' || *dot == DEL) x++;
		dot++;
	}
	if (i==1 && home>buf
	    && y <= wintop + (winbot-wintop)*gvars.top_display_margin/100)
 		vtscroll((int)((wintop-winbot)*(40-gvars.top_display_margin)/100-1), wintop);
	killing=FALSE;
}

endofline()
{
	closegap();
	while (dot < z && *dot != '\n') dot++;
	killing=FALSE;
}


begofline()
{
	while (dot > buf && *(dot-1) != '\n') dot--;
	killing=FALSE;
}

backtoindent()
{
	closegap();
	begofline();
	while (dot < z && *dot != '\n' && isblank(*dot)) dot++;
}

delhorizspace()
{
char *tdot;
	closegap();
	while (dot>buf && isblank(*(dot-1))) dot--;
	tdot = dot;
	while (tdot < z && isblank(*tdot)) tdot++;
	delchar((long)(tdot-dot));
}

delindent()
{
	if (xarg) passline(1L);
	begofline();
	delchar(backchar(1L));
	delhorizspace();
	insertc(' ',1L);
}

killchar(i)
long i;
{
Bool killflag=killing;
	if (dot + i >= gaps) closegap();
	if (i < 0L) i = -backchar(-i);
	else i = min(i, (long) (z - dot));
	killing=killflag;
	if (killing) {
		if (i > 0) filncat(killbuffile, dot, i);
		else filnprep(killbuffile,dot, -i);
    	}
    	else {
		filncpy(killbuffile,dot, abs(i), 0600);
		killing=TRUE;
    	}
    	delchar(abs(i));
}


delchar(i)
long i;
{
int lin1,lin2,lines;
long j;
	if (i > 0L){
		if (dot + i >= gaps) gapto(dot + i + GAPDIST, GAPSIZE);
		if (dot+i <= z) {
			upd2();
			if ((CS != NIL || DL != NIL) && dot+i < away) {
				findxy();
				lin1 = cury;
				for(j=0; j<i && *(dot+j)!='\n'; j++);
				if (j==i && curx+i >= screenwidth) lin1++;
				if (i==1 && *dot != '\n'
				    && curx == screenwidth-1) lin1++;
				dot += i;
				findxy();
				lin2 = cury;
				dot -= i;
				if (lin2 > lin1) {
					lines = lin2-lin1;
					if (*(dot-1)!='\n' || dot==buf
					|| *(dot+i-1)!='\n' ) lin1++;
					vtscroll(lines,lin1);
				}
			}
			blockmove(dot+i, dot, (long)(gaps-dot-i));
			if (gape == z) gape -= i;
			z -= i;
			gaps -= i;
			updateflag = TRUE;
			cb.modified = TRUE;
		}
		else errmes(NIB);
	}
}


nextscreen()
{
	if (away < z) {
		dot=away+1;
		passline(-2L);
		begofline();
		home=dot;
		upd2();
	}
}

prevscreen()
{
	if (home > buf) {
		passline((long)(-(winbot-wintop+1)+2-cury));
		begofline();
		home=dot;
		upd2();
	}
}


readfile()
{
int f;
register char *tdot;
off_t fz;
	closegap();
	if (*cb.filename == '\0') {
		errmes(NSF);
		return;
	}
	else if ((f = open(cb.filename,0)) > -1) {
		fz = filesize(cb.filename);
		if ((long)fz > bufsize && !incbuf((long)fz - bufsize))
			errmes(FTB);
		else {
			z = buf + read(f, buf, (int) bufsize);
			for (tdot = buf; tdot<z; tdot++) *tdot &= 127;
			gotobegin();
			cb.modified=FALSE;
			cb.mtime = filemodified(cb.filename);
 		}
		close(f);
 	}
	else {
		echo("(New File)");
		z=buf; 
		gotobegin(); 
		cb.modified=FALSE;
		cb.mtime = (time_t) 0;
	}
	updateflag = TRUE;
	upd2();
	modeline();
	killing=FALSE;
	gape = gaps = z;
}

insertfile(infilename)
char *infilename;
{
int f;
register char *tdot;
off_t fz;
	closegap();
		if ((f = open(infilename,0)) > -1) {
			fz = filesize(infilename);
			if (fz == 0) echo("File is empty");
			else if ((long)(z - buf + fz) <= bufsize
			  || incbuf((long)(z - buf + fz - bufsize))) {
			  	pushpopmark(1L);
				insertc('\0', (long) fz);
				read(f,dot - fz, (int) fz);
				for (tdot = dot - fz ; tdot < dot;
					tdot++) *tdot &= 127;
				cb.modified=TRUE;
			}
			else errmes(FTB);
			close(f);
		}
		else if (*infilename != '\0') errmes(NSF);
		killing=FALSE;
}


writefile()
{
register int f;
char bakfil[FILENAMESIZE], *cp, *tdot;
struct stat fst;
unsigned mode;
	closegap();
	if (cb.majormode == PASCAL && *(z-1) == '.') {
		tdot = dot;
		gotoend();
		insertc('\n',1L);
		dot = tdot;
	}
	if (*cb.filename != '\0') {
		echo("Written: "); 
/*
		strncpy(bakfil,(cp= rindex(cb.filename,'/')) == 0?
			cb.filename: &cp[1],10);
		bakfil[10]='\0';
		strcat(bakfil,".BAK");
*/
		cp= rindex(cb.filename,'/');
		if (cp == NIL)
			cp = cb.filename;
		else
			cp++;
		sprintf(bakfil, BACKUPNAME, cp);

/* Now what if the file is a link??? (FIX THIS) */

		unlink(bakfil);
		if (rename(cb.filename,bakfil) == 0) {
			stat(bakfil,&fst);
			mode = fst.st_mode;
		}
		else mode = 0666;
		if ((f=creat(cb.filename,(int) mode))>=0) {
			if (write(f,buf,z-buf) == z-buf) {
				strout(cb.filename);
				cb.modified = FALSE;
				modeline();
				close(f);
				cb.mtime = filemodified(cb.filename);
			}
			else {
				pchar(BELL);
				echo("can't write file");
				close(f);
			}
		}
		else {
			pchar(BELL);
			echo("can't create file!");
		}
	}
	else {
		pchar(BELL);
		echo("No saved filename");
	}
	killing=FALSE;
}


killline(i)
long i;
{
Bool killflag=killing;
Bool blankline=TRUE;

	pushpopmark(1L);
	if (xarg || *dot == '\n') passline(i); 
	else {
		while (dot < z && *dot != '\n') {
			if (!isblank(*dot)) blankline = FALSE;
			dot++;
		}
		if (blankline) passline(1L);
	}
	exchgdotmark();
	killing=killflag;
	killregion();
}

killregion()
{
long int n;
	adjustmark();
	n = *mark-dot;
	if (dot != *mark) killchar(n);
	*mark=dot;
}

copyregion()
{
long howmany;
char *from= dot;
	adjustmark();
	howmany= *mark - dot;
	if (howmany < 0) { howmany = -howmany; from = *mark; }
	if (dot != *mark) {
		closegap();
		if (killing) filncat(killbuffile, from, howmany);
		else {
			filncpy(killbuffile, from, howmany, 0600);
			killing=TRUE;
		}
	}
}


gotobegin()
{
	dot=home=buf;
	cury=curx=0;
	killing=FALSE;
}

gotoend()
{
	closegap();
	dot=z;
	killing=FALSE;
}


tmodlin(text)
char *text;
{
	if (text == NIL)
		*tmpmode_string = '\0';
	else
		strcpy(tmpmode_string, text);
	modeline();
	refresh(TRUE);
}

modeline()
{
register char *pt;
char tstr[10];
register int i;
		pt = screen[winbot+1];
		*pt = '\0';
		for (i = recursive_level - 1; i > 0; i--)
			strcat(pt, "[");
		if (*tmpmode_string)
			strcat(pt, tmpmode_string);
		else {
			strcat(pt,"SCAME (");
			strcat(pt,modes[(int)cb.majormode]);
			if (cb.minormodes.autofill) strcat(pt, " Fill");
			if (cb.minormodes.overwrite) strcat(pt, " Ovwrt");
			if (defining_kbd_mac) strcat(pt, " DEF");
			strcat(pt,")  ");
			strcat(pt,cb.name);	/* FLAG CONTROL! */
			strcat(pt,":  ");
			if (strsub(cb.filename,currentdir,FALSE,FALSE) == 0) {
				int i = 0;
				while (*(cb.filename+i) == *(currentdir+i))
					i++;
				strcat(pt,cb.filename+i+(i>1));
			}
			else strcat(pt,cb.filename);	/* FLAG CONTROL! */
		}
		strcat(pt,"          ");
		/* truncate line if too long */
		pt[screenwidth-9-recursive_level] = '\0';
		if (home==buf) { if (away<z) strcat(pt,"--TOP--"); }
		else if (away==z) strcat(pt,"--BOT--");
		else {
			sprintf(tstr,"--%ld%%--",
				(unsigned) (home-buf)*100L/(unsigned)(z-buf));
			strcat(pt,tstr);
		}
		strcat(pt, "  ");
		if (cb.modified) pt[strlen(pt)-1] = '*';
		for (i = recursive_level - 1; i > 0; i--)
			strcat(pt, "]");
		fflush(stdout);
}

transchar()
{
char c;
	if (*(dot-1) != '\n') {
		if (*dot == '\n' || dot == z) backchar(1L);
		c = *dot;
		*dot = *(dot-1);
		backchar(1L);
		*dot = c;
		forwchar(2L);
		updateflag = TRUE;
		cb.modified = TRUE;
	}
	else pchar(BELL);
	killing=FALSE;
}


yank()
{
int f;
register char *tdot;
off_t fz;
	fz = filesize(killbuffile);
	if (fz && (f = open(killbuffile,0)) > -1) {
		if ((long)(z - buf + fz) <= bufsize
		    || incbuf((long)(z - buf + fz - bufsize))) {
			pushpopmark(1L);
			insertc('\0', (long) fz);
			read(f,dot - fz, (int) fz);
			for (tdot = dot - fz ; tdot < dot;
				tdot++) *tdot &= 127;
			cb.modified=TRUE;
		}
		else errmes(BF);
		close(f);
	}
	else pchar(BELL);
	killing=FALSE;
}

Bool getfilename(prompt, name)
char *prompt, *name;
{
char tfilename[FILENAMESIZE];
int c;
char tprompt[80];
	basename(name,tfilename,FALSE);
	strcpy(tprompt,prompt);
	if (*tfilename != '\0') 
		sprintf(&tprompt[strlen(tprompt)]," (%s)",tfilename);
	strcat(tprompt,": ");
	*tfilename= '\0';
	do {
		c=instring(tprompt,tfilename,strlen(tfilename),
				" \023\t", "\r\007\033");
		if (c == ESC) {
			if (*tfilename == '\0') {
				basename(name,tfilename,FALSE);
			}
			else pchar(BELL);
		}
	} while (c == ESC);
	if (c==13) {
 		if (*tfilename == '\0') basename(name, tfilename, FALSE);
		if (strcmp(name,tfilename)!=0) buildfilename(name,tfilename);
		return TRUE;
	}
	else {
		echo(NIL);
		return FALSE;
	}
}

adjustmark()
{
*mark = min(*mark,z);
}

exchgdotmark()
{
char *tdot=dot;
	adjustmark();
	if (dot < *mark)
		forwchar((long)(*mark - dot));
	else
		backchar((long)(dot - *mark));
	*mark = tdot;
}


markquant(q, i, movepoint)
enum quantity_t q;
long i;
Bool movepoint;
{
char *tdot;
	tdot = dot;
	forwquant(q, i);
	pushpopmark(1L);
	dot = tdot;
	if (movepoint && !endofquantp(q, dot))
		backquant(q, 1L);
}

killquant(q,i)
enum quantity_t q;
long i;
{
Bool killflag=killing;
	pushpopmark(1L);
	if (i>0)
		forwquant(q,i);
	else
		backquant(q,-i);
	exchgdotmark();
	killing=killflag;
	killregion();
}

forwquant(q,i)
enum quantity_t q;
register long i;
{
	closegap();
	while (i-- > 0) {
		switch (q) {
		case WORD:
 			while (dot < z && endofquantp(q,dot)) dot++;
			while (dot < z && !endofquantp(q,dot)) dot++;
			break;
		case LINE:
			passline(i);
			break;
		case SENTENCE:
			if (dot < z-1) dot++;
			while (dot < z && *dot == '\n') dot++;
			while ((dot < z-1)
				&& (index(".?!", *(dot-1)) == NIL
				   || (*dot != ' ' || *(dot+1) != ' '))
				&& (index(".?!", *(dot-1)) == NIL
				   || (*dot != '\n'))
				&& (*dot != '\n' || *(dot+1) != '\n'))
				dot++;
			break;
		case REGION:
		case PARAGRAPH:
			errmes(ILQ);
			break;
		case BUFFER:
			gotoend();
			break;
		}
	}
	killing=FALSE;
}

backquant(q,i)
enum quantity_t q;
register long i;
{
	while (i-- > 0 && dot > buf) {
		switch (q) {
		case WORD:
 			backchar(1L);
			while (endofquantp(q,dot) && backchar(1L)!=0L);
			while (!endofquantp(q,dot) && backchar(1L)!=0L);
			if (dot>buf && dot < z) dot++;
			break;
		case LINE:
			passline(-i);
			break;
		case SENTENCE:
			if (dot > buf) backchar(1L);
			while (dot > buf && (*dot == '\n' || isblank(*dot)))
 			       	backchar(1L);
			while ((dot > buf)
				&& (index(".?!", *(dot-1)) == NIL
				   || (*dot != ' ' || *(dot+1) != ' '))
				&& (index(".?!", *(dot-1)) == NIL
				   || (*dot != '\n'))
				&& (*dot != '\n' || *(dot+1) != '\n'))
				backchar(1L);
			while(dot < z && (*dot == '\n' || isblank(*dot)))
				dot++;
			break;
		case REGION:
		case PARAGRAPH:
			errmes(ILQ);
			break;
		case BUFFER:
			gotobegin();
			break;
		}
	}
	killing=FALSE;
}


upcaseword(i,flg)
register long i;
short flg;
{
register Bool swedish = (cb.majormode==SWEDISH);
	closegap();
	while (i-- > 0) {
		while (dot < z && endofquantp(WORD,dot)) dot++;
		if (dot<z && flg == 2) {
			*dot=upcase(*dot,swedish);
			dot++;
			flg=FALSE;
		}
		if (flg) {
			while (dot < z && !endofquantp(WORD,dot)) {
				*dot=upcase(*dot,swedish);
				dot++;
			}
		}
		else {
			while (dot < z && !endofquantp(WORD,dot)) {
				*dot=lowcase(*dot,swedish);
				dot++;
			}
		}
	}
	updateflag = TRUE;
	cb.modified = TRUE;
	killing=FALSE;
}

upcaseregion(f)
Bool f;
{
register char *d1,*d2;
register Bool swedish=(cb.majormode==SWEDISH);
	adjustmark();
	d1 = min(dot,*mark)-1;
	d2 = max(dot,*mark);
	if (f) {
		while (++d1 < d2) *d1=upcase(*d1,swedish);
	}
	else {
		while (++d1 < d2) *d1=lowcase(*d1,swedish);
	}
	updateflag = TRUE;
	cb.modified = TRUE;
	killing=FALSE;
}

rot13_region()
{
register char *d1,*d2;
	adjustmark();
	d1 = min(dot,*mark)-1;
	d2 = max(dot,*mark);
	if (d1 < d2-1) {
		while (++d1 < d2) {
			if (isalpha(*d1)) {
				if ((*d1 & 31) <= 13) *d1 = *d1+13;
				else *d1 = *d1-13;
			}
		}
		updateflag = TRUE;
		cb.modified = TRUE;
	}
	killing=FALSE;
}


stop()
{
#ifdef SIGTSTP
register int i;
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	setupterm(FALSE);
	echo(NIL);
	kill(getpid(), SIGTSTP);	/* stop here until continued */
	setupterm(TRUE);
	cls();
	vpos=0; hpos=0;
	upd2();
	modeline();
	for (i=0; i<=screenlen-2; i++) *oldscreen[i]='\0';
	signal(SIGTSTP, stop);
#else
	errmes(NYI);
#endif
	killing=FALSE;
}

findmode()
{
char *str;
	str=rindex(cb.filename,'.');
	if (str != NIL) {
		if (!strcmp(str,".A")) setmode(ADA);
		else if (!strcmp(str,".c")) setmode(C);
		else if (!strcmp(str,".l") || !strcmp(str,".lsp")
			|| !strcmp(str,".lisp")) setmode(LISP);
		else if (!strcmp(str,".p") || !strcmp(str,".pas"))
			setmode(PASCAL);
		else if (!strcmp(str,".swe")) setmode(SWEDISH);
		else setmode(FUNDAMENTAL);
	}
	else setmode(FUNDAMENTAL);
}

setmode(mode)
enum majormode_t mode;
{
static int haspiped=FALSE;
register int i;
register swedish = (mode == SWEDISH);
char initfile[FILENAMESIZE];
	if (mode != cb.majormode) {
		switch (mode) {
		    case LISP:
			cb.lvars.disp_matching_paren = 1L; break;
		    default:
			cb.lvars.disp_matching_paren = -1L;
		}
		if (mode == SWEDISH)
		for (i=0; i < 512; i++) {
			nupcasarr[i] = ((i & 0177) >= 'a'
					&& (i & 0177) <= '}') ? (i & 0337) : i;
#ifdef COMMENT
			nlocasarr[i] = ((i & 0177) >= 'A'
					&& (i & 0177) <= ']') ? (i | 0040) : i;
#endif
		}
		strcpy(initfile,".scamerc_");
		strcat(initfile,modes[(int)mode]);
		for (i=0; i < strlen(initfile); i++)
			initfile[i] = lowcase(initfile[i],FALSE);
		execfile(initfile);
	}
	cb.majormode = mode;
	modeline();
}

setminormode(mode,arg)
long *mode;
long arg;
{
	if (xarg) *mode = arg;
	else *mode = (long) !(*mode);
	modeline();
}

whatcursorposition()
{
char s[SCRDIMX+1];
long l;
	l = (z != buf) ? (unsigned)(dot-buf)*100L/(unsigned)(z-buf) : 0L;
	sprintf(s, "X=%d Y=%d ",curx,cury);
	if (dot < z)
		sprintf(&s[strlen(s)], "CH=%o ",*dot);
	sprintf(&s[strlen(s)], ".=%u(%ld%% of %u) Allocated=%ld",
			       dot-buf, l,   z-buf,       bufsize);
	echo(s);
}


comment()
{
	closegap();
	switch (cb.majormode) {
	    case FUNDAMENTAL:
	    case LISP:
	    case SWEDISH:
		begofline();
		while (dot < z && *dot != ';' && *dot != '\n') dot++;
		findxy();
		while (curx > gvars.commcol+4 && isblank(*(dot-1))) {
			delchar(backchar(1L));
			findxy();
		}
		while (curx < gvars.commcol-1 && insertc('\t',1L)) findxy();
		if (*(dot-1) != '\t') insertc('\t',1L);
		if (dot==z || *dot != ';') insertc(';',1L);
		else if (dot<z) dot++;
		break;
	    case PASCAL:
		begofline();
		while (((dot < z && *dot != '(') 
			|| (dot< z-1 && *(dot+1) != '*'))
			&& *dot != '\n') dot++;
		findxy();
		while (curx > gvars.commcol+4 && isblank(*(dot-1))) {
			delchar(backchar(1L));
			findxy();
		}
		while (curx < gvars.commcol-1 && insertc('\t',1L)) findxy();
		if (*(dot-1) != '\t') insertc('\t',1L);
		if (dot==z || *dot != '(') {
			insertstr("(*  *)",6L);
			backchar(3L);
		}
		else forwchar(3L);
		break;
	    case ADA:
		begofline();
		while (((dot < z && *dot != '-') 
			|| (dot< z-1 && *(dot+1) != '-'))
			&& *dot != '\n') dot++;
		findxy();
		while (curx > gvars.commcol+4 && isblank(*(dot-1))) {
			delchar(backchar(1L));
			findxy();
		}
		while (curx < gvars.commcol-1 && insertc('\t',1L)) findxy();
		if (*(dot-1) != '\t') insertc('\t',1L);
		if (dot==z || *dot != '-') insertstr("-- ",3L);
		else forwchar(3L);
		break;
	    case C:
		begofline();
		while (((dot < z && *dot != '/') 
			|| (dot< z-1 && *(dot+1) != '*'))
			&& *dot != '\n') dot++;
		findxy();
		while (curx > gvars.commcol+4 && isblank(*(dot-1))) {
			delchar(backchar(1L));
			findxy();
		}
		while (curx < gvars.commcol-1 && insertc('\t',1L)) findxy();
		if (*(dot-1) != '\t') insertc('\t',1L);
		if (dot==z || *dot != '/') {
			insertstr("/*  */",6L);
			backchar(3L);
		}
		else forwchar(3L);
		break;
	} /* switch */
}


pushpopmark(a)
long a;
{
register int i;
char *tdot;
	if (a == 1) {
		for (i=15; i > 0; i--) mark[i] = mark[i-1];
		*mark=dot;
	}
	else {
		closegap();
		tdot= min(*mark,z);
		for (i=0; i < 15 && mark[i+1] != NIL; i++) mark[i]= mark[i+1];
		mark[i] = dot;
		dot = tdot;
	}
}



int tablk(table,str,index)
struct tabstruct table[];
char *str;
int index;

/* Look through table for str.
 * If one match is found, return index of that.
 * If more than one, return -(no of matching entries).
 * If none, return -1
 *
 * Also, fill in as much as is unique in str.
 *
 */

{
register int i, count = 0;
char mstr[80];
	*mstr = '\0';
	for (i = index; table[i].name != NIL; i++)
		if (strsub(table[i].name,str,TRUE,FALSE) == 0) {
			count++;
			index = i;
			if (*mstr == '\0') strcpy(mstr,table[i].name);
			else while(strsub(table[i].name,mstr,TRUE,FALSE) != 0)
				mstr[strlen(mstr)-1] = '\0';
			if (count==1 && strneq(table[i].name,str,strlen(mstr),TRUE))
				break;
		}
	if (count == 0) return(-1);
	else {
		if (*mstr != '\0') strcpy(str,mstr);
		if (count == 1) return(index);
		else return(-count);
	}
}

listmatching(table,str)
struct tabstruct table[];
char *str;
{
register int i = -1;
	typing = FALSE;
	typeout("Here are the possible completions of what you have typed:");
	while (table[++i].name != NIL)
		if (strsub(table[i].name,str,TRUE,FALSE) == 0
			&& !typeout(table[i].name)) break;
}

Bool incbuf(i)
long i;
{
long j;
char *p;
	if (i > 0) {
		p = sbrk((int) i);
		if ((int) p != -1) {
			j = sbrk(0) - p;
			bufsize += j;
			if (j >= i) return(TRUE);
		}
	}
	errmes(BF);
	return (FALSE);
}

setfillprefix()
{
char *tdot= dot;
	begofline();
	strncpy(fillprefix,dot,tdot-dot);
	fillprefix[tdot-dot] = '\0';
	dot = tdot;
	if (!quiet) {
		echo("Fill Prefix now \"");
		write(1, fillprefix, strlen(fillprefix));
		pchar('"');
	}
}





/*
 * Gap routines. gaps points to first position in gap.
 * gape points to first character after gap. If gaps=gape
 * then gap is empty
 */

Bool gapto(pos,size)
char *pos;
long size;
{
long stepsize;
#ifdef PDP11
	stepsize = 64;
#else
	stepsize = 1024;
#endif
	closegap();
	pos = min(z, pos);
	if ((z + size <= buf + bufsize)
	    || incbuf(size + stepsize - ((long)sbrk(0) + size) % stepsize)) {
		gaps = pos;
		gape = pos+size;
		blockmove(gaps, gape, (long)(z-gaps));
		return (TRUE);
	}
	else return(FALSE);
}

closegap()
{
	if (gaps < z)
		blockmove(gape, gaps,(long)(z-gaps));
	gape = gaps = z;
}

char *func_name(fn, fname)	/* Get the printname for a function,	*/
int (*fn)();			/* given a pointer to it.		*/
char *fname;
{
register int i = 0;
	while(x_comtab[i].name != NIL && x_comtab[i].funcp != fn) i++;
	if (x_comtab[i].funcp == fn) strcpy(fname, x_comtab[i].name);
	else *fname = '\0';
	return(fname);
}

revertfile()
{
char *tdot,*thome;
	tdot=dot;
	thome = home;
	readfile();
	dot = min(tdot,z);
	if (thome <= z) home = thome;
}
