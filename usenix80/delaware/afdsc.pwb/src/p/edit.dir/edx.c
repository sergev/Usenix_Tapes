#
/*
Name:
	EDX System Program

Function:
	Act as screen manager for line editor.

Algorithm:
	Take control code commands from user, turn them into editor commands.
	Pass them on to editor and field her response.  Apply proper cursor
	movement commands to user's terminal.  Keep open the channel between
	edx and the editor, even after quitting.

	To support a new terminal type, the following actions must be taken:
	1.  Create a global integer variable named for the terminal type.
	2.  Create a series of defines for the terminal's special characters
		needed for the routines detailed below.
	3.  Modify the CLEAR routine - insert an "if" statement and code to
		output the characters necessary to clear the terminal's screen.
		End the code with a "return" statement.
	4.  Modify the SETUP routine - if necessary, insert an "if" statement
		and code to output the characters to initialize the terminal for
		use by the editor.
	5.  Modify the POSN routine - insert an "if" statement and code to 
		output the characters needed to position the cursor to an absolute
		place on the screen.  Care must be taken to translate the line
		and column numbers into the zero- or one-based indices appropriate
		to the terminal.  End the code with a "return" statement.
	6.  Modify the WBYT routine - insert an "else if" statement and code to
		output the characters needed to erase the current character.
	7.  Modify the TYPEIT routine - add a terminal name with a unique first
		letter to the list of terminal names.  Insert an "else if" statement
		and code to set the terminal's variable if its first letter is
		typed in.  If the terminal does not automatically wraparound long
		lines, add the terminal's variable name to the "funny" "if" statement.

Parameters:
	Ascii output file descriptor number
	Ascii input file descriptor number

Returns:
	EINVAL	Bad terminal type
	EIO	Bad pipe I/O

Files and Programs:
	None.

Installation Instructions:
	Put in /bin/edit.dir/edx

History:
	Received from Mike Pearson, Yourdon, Jan 80.  Integrated into editor
		as 'b' subcommand for Release 3 Version 6, Feb 80.
	Apr 80, Release 3 Version 7, W. Lazear, removed brkcnt() "feature" to
		look ahead at user input.  (Brkcnt() never existed at AFDSC and
		was dummied out initially).

*/
/*
 *	Edx  -  sub-process to ED
 *	Currently supports Omron, Hazeltine Modular One, and
 *	LSI-ADM, HP-2640A, P-E Bantum 550
 *	Must reside in /bin/edit.dir/edx
 */
#include	"edx.h"
#include	<error.h>
#define BYTSIZ		100			/* tty high water mark */
#define TABSIZ		8
#define SCRNLEN		23			/* Physical screen - 1 */
#define SCRNWID		79			/* Physical screen - 1 */
#define LRSHIFT		(TABSIZ * 2)
#define LINEXTRA	12
#define NOLINE		-1
#define QMAX		100
#define TAB	011
#define SPACE	040
#define ERASE	020
#define ESCAPE	033
#define LOW7	0177
#define QUOTE	0200
#define WALWD	(curcol == cthis && tcok && curcol < SCRNWID)
/* Hazeltine 1200 Dependent Codes */
#define H1200HME	013
#define H1200CLR	014
#define H1200ERASE	020
#define H1200SPACE	040
/* Modular One Dependent Codes */
#define M1DWN		013
#define M1UP		014
#define M1SPACE		020
#define M1POSN		021
#define M1HME		022
#define M1CLR		034
#define M1ERASE		040
#define M1TYPE1		042
#define M1LEADIN	0176
/* LSI-ADM Dependent Codes */
#define LSISPACE	014
#define LSICLR		032
#define LSIHOME		036
#define LSIERASE	040
#define LSIPOSN		075
/* HP2640 codes */
#define HPERASE		040
#define HPHOME		'H'
#define HPCLEAR		'J'
#define HPUP		'A'
#define HPADDRS		'&'
#define HPLINE		'a'
#define HPCOL		'r'
#define	HPADDRE		'C'
/* OMRON codes */
#define OMERASE		040
#define OMADDR		0175
#define OMBASE		0100
#define OMSPACE		0103
#define OMHOME		0110
#define	OMCLEAR		0112
/* Bantum Codes */
#define BTUP		0101
#define BTSPACE		0103
#define BTERASE		0040
#define BTHOME		0110
#define BTCLEAR		0113
#define BTLPOSN		0130
#define BTCPOSN		0131
#define BTPOSPLUS	0040
/* Delta Data 4000 */
#define DD4ERASE	0040
#define DD4EFF		0106
#define DD4CLEAR	0105
#define DD4ZERO		0060
/* Datagraphix 132A */
#define DGAERASE	0040
#define DGACLEAR	0014
#define DGAZERO		0060
#define DGAEIGHT	0070
/* VT100 codes */
#define V1ERASE		0040
#define V1BRACK		0133
#define V1ACH		0110
#define V1ONE		0061
#define V1TWO		0062
#define V1SEVEN		0067
#define V1JAY		0112
#define V1LESS		0074
#define V1QUES		0077
#define V1SEMI		0073
#define V1ELL		0154
/* VT52 codes */
#define V5ERASE		0040
#define V5HOME		0110
#define V5CLEAR		0112
#define V5WYE		0131
#define V5BRACK		0133
#define V5QUES		0077
#define V5TWO		0062
#define V5ELL		0154
/* Cursor Movement Commands */
#define TOP		007
#define BACKSP		010
#define LF		012
#define CUP		013
#define RIGHT		014
#define RETURN		015
#define EOL		016
#define LWORD		017
#define RWORD		020
#define RUBOUT		0177
/* All Other Commands  */
#define UNDO		000
#define RSHIFT		001
#define DTEXT		002
#define ARG		003
#define RMWORD		004
#define DLINE		005
#define RMCHAR		006
#define QUIT		021
#define REPAINT		022
#define LSHIFT		023
#define MKLINE		024
#define RMLINE		025
#define UTEXT		026
#define ULINE		027
#define RESTORE		030
#define INSERT		031
#define SAVE		032
#define JOIN		034

int	h1200, modone, lsiadm, hewlet, omron, bantum, vt52, vt100, dg132a, dd4000;

char	**savetabl[10];
char	*alloc();
char	*copy();
char	*txalloc();
char	**clostab;
char	*sbuf;
char	argline[SCRNWID];
char	*argbuf	&argline[5];
char	bytbuf[BYTSIZ];
char	chtext[512];
char	readq[QMAX];
char	screen[SCRNLEN+1][SCRNWID+1];
char	linemod[SCRNLEN+1];
char	srchbuf[SCRNWID];
char	**text;
int	arg;
int	bytcnt;
int	tcok;
int	textmod;
int	chaddr;
int	chend;
int	chmod;
int	chnum	NOLINE;
int	coffset;	/* shift offset */
int	column;		/* actual column  on screen */
int	curcol;		/* will be column on screen */
int	dol;		/* last line + 1 */
int	dot;		/* current line */
int	funny;
int	insertg		1;
int	lastsave	-1;
int	line;
int	linesp;
int	oldbasel;
int	peekc;
int	pipin;
int	pipout;
int	qcnt;
int	qnxt;
int	cthis;
int	textcol;	/* column in text */

struct {
	char ispeed, ospeed;
	char erase, kill;
	int mode;
} xedtty;
main(argc,argv) char **argv;
{
	register int c, f, t;

	setup(argc,argv);
	cctotc();
	for(;;) {
	    c = readc();
	    arg = 0;
	    *argbuf = 0;
	    if((c >= 040 && c < 0177) || c == TAB)
		achar(c);
	    else if((c >= TOP && c <= RWORD) || c == RUBOUT)
		cmove(c);
	    else {
	      mainswitch:
		switch(c) {	/* Commands */

		case QUIT:
			ed();
			continue;

		case ARG:
			if((c = getarg()) == ARG)
				continue;
			arg = 1;
			goto mainswitch;

		case UTEXT:
			if(arg)
				edxio.x_base =+ atoi(argbuf) * (SCRNLEN + 1);
			else
				edxio.x_base =+ SCRNLEN + 1;
			askedit(0);
			cctotc();
			curcol = cthis;
			continue;

		case DTEXT:
			if(arg)
				edxio.x_base =- atoi(argbuf) * (SCRNLEN + 1);
			else
				edxio.x_base =- SCRNLEN + 1;
			askedit(0);
			cctotc();
			curcol = cthis;
			continue;

		case LSHIFT:
			t = coffset;
			if(arg) {
				f = atoi(argbuf);
				f = f - (f%TABSIZ);
				coffset =+ f;
				f = curcol - f;
			} else {
				coffset =+ LRSHIFT;
				f = curcol - LRSHIFT;
			}
			setmod(0);
			if(f < 1) f = 1;
			if(t != coffset) curcol = f;
			cctotc();
			curcol = cthis;
			continue;

		case RSHIFT:
			t = coffset;
			if(arg) {
				f = atoi(argbuf);
				f = f - (f%TABSIZ);
				coffset =- f;
				f = curcol + f;
			} else {
				coffset =- LRSHIFT;
				f = curcol + LRSHIFT;
			}
			if(coffset < 0) coffset = 0;
			if(f >= SCRNWID) f = SCRNWID - 1;
			setmod(0);
			if(t != coffset)
				curcol = f;
			cctotc();
			curcol = cthis;
			continue;

		case ULINE:
			if(arg) {
				edxio.x_base =+ atoi(argbuf);
				f = dot - atoi(argbuf);
			} else {
				edxio.x_base =+ dot;
				f = 0;
			}
			askedit(0);
			if(f < 0) f = 0;
			dot = f;
			cctotc();
			curcol = cthis;
			continue;

		case DLINE:
			t = edxio.x_base;
			if(arg) {
				edxio.x_base =- atoi(argbuf);
				f = dot + atoi(argbuf);
			} else {
				edxio.x_base =- SCRNLEN - dot;
				f = SCRNLEN;
			}
			askedit(0);
			if(f >= SCRNLEN) f = SCRNLEN;
			if(t != edxio.x_base)
				dot = f;
			cctotc();
			curcol = cthis;
			continue;

		case RMWORD:
			if(arg) {
				if(t = atoi(argbuf)) rmword(t);
				else rmchar(-1);
			} else rmword(1);
			continue;

		case MKLINE:
			if(arg) mkline(dot, atoi(argbuf));
			else mkline(dot, 1);
			curcol = 1;
			cctotc();
			curcol = cthis;
			continue;

		case RMLINE:
			if(arg) rmline(dot, atoi(argbuf));
			else rmline(dot, 1);
			cctotc();
			continue;

		case INSERT:
			insertg = !insertg;
			continue;

		case RMCHAR:
			if(arg) {
				if(t = atoi(argbuf)) rmchar(t);
				else rmchar(-1);
			} else rmchar(1);
			continue;

		case SAVE:
			save(dot, 0);
			continue;

		case RESTORE:
			restore(dot);
			continue;

		case REPAINT:
			clear();
			continue;

		case UNDO:
			if(chnum != dot) {
				bell();
				continue;
			}
			text[dot] = chaddr;
			linemod[dot] = 1;
			chnum = NOLINE;
			chmod = 0;
			cctotc();
			curcol = cthis;
			continue;

		case LF:
			if(!pickbuf())
				bell();
			else
				fwdsrch(0);
			continue;

		case RETURN:
			if(!pickbuf())
				bell();
			else
				fwdsrch(1);
			continue;

		case CUP:
			if(!pickbuf())
				bell();
			else
				bkwdsrch();
			continue;

		case JOIN:
			join();
			continue;

		default:
			if(c & QUOTE) {
				if((c =& LOW7) == LF)
					split();
				else
					achar(c);
			} else
				bell();
			continue;

		}
	    }
	}
}
/*

Name:
	achar

Function:
	Process ascii text character

Algorithm:
	Check for out of bounds line number and tell user.  Set new dot.
	Check for line overflow and tell user.  Either insert or overwrite
	character in line.  Mark line as modified.

Parameters:
	Character

Returns:
	None.

Files and Programs:
	None.


*/

achar(c) register int c;
{
	if(dot >= dol) {
		if(!WALWD) {
		   bl:	bell();
			return;
		}
		newdol(dot);
		cctotc();
	}
	swap(dot);
	if(!WALWD)
		goto bl;
	if(insertg) {
		if(chend >= 511)
			goto bl;
	} else {
		if(textcol >= 511)
			goto bl;
	}
	chmod = 1;
	if(insertg) insert(c);
	else overwrit(c);
	textcol++;
	tctocc();
	curcol = cthis;
	linemod[dot] = 1;
}

/*

Name:
	insert

Function:
	Insert character in line.

Algorithm:
	Scan to end of line.  Shift from insert point to end of line one character.
	Drop character into place and adjust counter.

Parameters:
	Character

Returns:
	None.

Files and Programs:
	None.


*/
insert(c)
{
	register char *lp, *bp, *np;

	bp = lp = &chtext[textcol];
	while(*lp++) ;
	*lp = 0;
	np = lp - 1;
	while(np > bp) *--lp = *--np;
	*bp = c;
	chend++;
}

/*

Name:
	overwrit

Function:
	Overlay existing character in line.

Algorithm:
	If at end of line, move end marker.  Lay character into line.

Parameters:
	character

Returns:
	None.

Files and Programs:
	None.


*/
overwrit(c) register int c;
{
	if(chtext[textcol] == 0) {
		chend++;
		chtext[textcol+1] = 0;
	}
	chtext[textcol] = c;
}

/*

Name:
	cmove

Function:
	Move cursor

Algorithm:
	React to motion commands:
		left
		delete character
		top of page
		beginning of next line
		up
		down
		right
		right one word
		left one word
		end of line.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
cmove(c) register char c;
{
	register char *p;

	switch(c) {
		case BACKSP:
		    back:
			if(cthis < curcol)
				curcol = cthis;
			else if(textcol > 0) {
				--textcol;
				tctocc();
				curcol = cthis;
			}
			break;

		case RUBOUT:
			if(!insertg)
				goto back;
			if(textcol <= 0 || curcol != cthis || curcol <= 1) {
				bell();
				return;
			}
			--textcol;
			tctocc();
			curcol = cthis;
			rmchar(1);
			break;

		case TOP:
			dot = 0;
			curcol = 1;
			cctotc();
			curcol = cthis;
			break;

		case RETURN:
			if(++dot > SCRNLEN)
				dot = SCRNLEN;
			curcol = 1;
			cctotc();
			curcol = cthis;
			break;

		case CUP:
			if(--dot < 0)
				dot = 0;
			break;

		case LF:
			if(++dot > SCRNLEN)
				dot = SCRNLEN;
			break;

		case RIGHT:
			textcol++;
			tctocc();
			curcol = (cthis > SCRNWID) ? SCRNWID : cthis;
			break;

		case RWORD:
			if(cthis < curcol &&
				(dot >= dol ||  text[dot][textcol] == 0))
					curcol = cthis;
			else
				rword();
			break;

		case LWORD:
			if(cthis < curcol &&
				(dot >= dol ||  text[dot][textcol] == 0))
					curcol = cthis;
			else
				lword();
			break;

		case EOL:
			if(dot >= dol)
				curcol = cthis;
			else {
				for(p = text[dot]; *p++; ) ;
				textcol = (--p) - text[dot];
				tctocc();
				curcol = cthis;
			}
			break;

	}
	cctotc();
}

/*

Name:
	cctotc

Function:
	Translate the individual character table to the screen image text table.

Algorithm:
	Determine the screen offset.  For each character on the line, check for 
	tab padding, control characters, or regular characters, and react
	accordingly.  Adjust column pointers.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.



*/
cctotc()
{
	register int t, c;
	register char *wl;
	int e;
	int tabjmp, tcoffs;

	if(dot[text] == argline) tcoffs = 0;
	else tcoffs = coffset;
	if(dot < dol && (wl = text[dot]) != 0) {
		e = curcol + tcoffs - 1;
		for(t = 0; (c = *wl++) && t < e; ) {
			if(c == TAB) {
				tabjmp = t + (TABSIZ - (t % TABSIZ));
				goto ct;
			} else if(c > 0 && c < 040) {
				tabjmp = t + 2;
			   ct:	if(tabjmp > e) {
					break;
				}
				t = tabjmp;
			} else
				t++;
		}
		textcol = (--wl) - text[dot];
	} else
		t = textcol = c = 0;
	if(t < tcoffs) {
		cthis = 0;
		tcok = c == 0;
	} else {
		cthis = (++t) - tcoffs;
		tcok = 1;
	}
}

/*

Name:
	tctocc

Function:
	Translate the screen image text table to individual character table.

Algorithm:
	Get the screen offset.  For each character on the line, check for tab
	padding, control character, or regular characters, and react accord-
	ingly.  Adjust columns pointers.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.



*/
tctocc()
{
	register int t, c;
	register char *wl;
	char *e;
	int tabjmp, tcoffs;

	if(text[dot] == argline) tcoffs = 0;
	else tcoffs = coffset;
	if(dot < dol && (wl = text[dot]) != 0) {
		e = &wl[textcol];
		for(t = 0; (c = *wl) && wl < e; wl++) {
			if(c == TAB) {
				tabjmp = t + (TABSIZ - (t % TABSIZ));
				goto ct;
			} else if(c > 0 && c < 040) {
				tabjmp = t + 2;
			   ct:	if(wl > e) {
					break;
				}
				t = tabjmp;
			} else
				t++;
		}
	} else
		t = textcol = c = 0;
	if(t < tcoffs) {
		cthis = 0;
		tcok = c == 0;
	} else {
		cthis = (++t) - tcoffs;
		if(cthis > SCRNWID) {
			cthis = SCRNWID;
			tcok = 0;
		} else
			tcok = 1;
	}
}

/*

Name:
	swap

Function:
	Copy text line into change area.

Algorithm:
	If line is already in change area, return.  If there's a line to be
	saved, copy it into newly allocated area.  Copy line into change area,
	save old pointer and set up new ones.

Parameters:
	Line number (text array index)

Returns:
	None.

Files and Programs:
	None.


*/
swap(n) register int n;
{
	if(chnum == n)
		return;
	if(chnum != NOLINE) {
		if(chmod) {
			textmod = 1;
			free(chaddr);
			copy(chtext, txalloc(chnum, chend + 1));
		} else 
			text[chnum] = chaddr;
	}
	chnum = n;
	chmod = 0;
	if(n == NOLINE)
		return;
	chend = copy(text[n], chtext) - chtext;
	chaddr = text[n];
	text[n] = chtext;
}

/*

Name:
	wline

Function:
	Write text line to screen.

Algorithm:
	Put out proper left border, output each character.  Use system tabs (replaced
	by spaces).  Finish line with appropriate right border.

Parameters:
	Index to line.

Returns:
	None.

Files and Programs:
	None.


*/
wline(aline)
{
	register char *wl;
	register int t, c;
	char *sl;
	static int tabjmp, tcoffs, kwote, endsiz;

	sl = screen[aline];
	if(aline >= dol) c = '-';
	else if(coffset) c = '<';
	else c = '|';
	if(*sl++ != c) {
		posn(aline, 0);
		wbyt(c);
	}
	wl = text[aline];
	if(wl == argline) tcoffs = 0;
	else tcoffs = coffset;
	if(wl == 0)
		wl = "";
	tabjmp = kwote = 0;
	endsiz = tcoffs + (SCRNWID - 1);
	for(t = 0; t < endsiz ; t++) {
		if(--tabjmp > 0) {
			c = SPACE;
		} else if(kwote) {
			c = kwote;
			kwote = 0;
		} else if(*wl == TAB) {
			tabjmp = TABSIZ - (t%TABSIZ);
			c = SPACE;
			wl++;
		} else if((c = *wl) != 0)
				wl++;
		if((c > 0 && c < 040) || c == RUBOUT) {
			if(c == RUBOUT)
				kwote = '?';
			else
				kwote = c | 0100;
			c = '^';
		}
		if(t >= tcoffs) {
			if(c == SPACE || c == 0) c = ERASE;
		lend:	if(*sl++ != c) {
				posn(aline, t-tcoffs+1);
				wbyt(c);
			}
		}
	}
	if(t == endsiz) {
		linemod[aline] = 0;
		if(aline >= SCRNLEN && funny)
			return;
		if(*wl == 0)
			c = ERASE;
		else
			c = '>';
		goto lend;
	}
}

/*

Name:
	wbyte

Function:
	Output or react to byte.

Algorithm:
	If printing character, put it in buffer.  Otherwise, put appropriate
	character sequence in buffer or adjust counters.  Flush buffer, if
	appropriate.

Parameters:
	character

Returns:
	None.

Files and Programs:
	None.


*/
wbyt(c) register char c;
{
	if(bytcnt >= BYTSIZ) dbyt();
	if(c >= 041 && c <= 0176) {	/* normal printing char */
		bytbuf[bytcnt++] = screen[line][column++] = c;
		if(modone && c == M1LEADIN) bytbuf[bytcnt-1] = '`';

	} else  switch(c) {

		case SPACE:	/* skip over current char */
			if((c = screen[line][column]) != ERASE) {
				bytbuf[bytcnt++] = c;
				column++;
				break;
			}

		case ERASE:	/* erase current char */
			if(hewlet)
				wctl(HPERASE);
			else if(omron)
				wctl(OMERASE);
			else if(bantum)
				wctl(BTERASE);
			else if(modone)
				wctl(M1ERASE);
			else if(lsiadm)
				wctl(LSIERASE);
			else if(vt52)
				wctl(V5ERASE);
			else if(vt100)
				wctl(V1ERASE);
			else if(dd4000)
				wctl(DD4ERASE);
			else if(dg132a)
				wctl(DGAERASE);
			else
				wctl(H1200ERASE);
			screen[line][column++] = ERASE;
			break;

		case LF:	/* move down one line */
			if(line == SCRNLEN) break;
			bytbuf[bytcnt++] = c;
			line++;
			break;

		case BACKSP:	/* move left one char */
			if(column == 0) break;
			bytbuf[bytcnt++] = c;
			--column;
			break;

		case RETURN:	/* move to begin of current line */
			bytbuf[bytcnt++] = c;
			column = 0;
			break;

		default:	/* error */
			bytbuf[bytcnt++] = '#';
			column++;
			break;
	}
	if(column > SCRNWID) {
		if(funny) {
			column = 0;
			line++;
			posn(line-1, SCRNWID);
		} else
			column = SCRNWID;
	}
	if(bytcnt >= BYTSIZ) dbyt();
}

/*

Name:
	wctl

Function:
	Output control character

Algorithm:
	If necessary, flush buffer.  Put character in buffer.

Parameters:
	Control character to be written.

Returns:
	None.

Files and Programs:
	None.

*/
wctl(cc)
{
	if(bytcnt >= BYTSIZ) dbyt();
	bytbuf[bytcnt++] = cc;
}

/*

Name:
	clear

Function:
	Clear screen.

Algorithm:
	For each line, mark it as modified (for display() during next readc()).
	Clear screen matrix.  Then perform specific commands to clearn terminal
	screen.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.

*/
clear()
{
	register int t, i;

	column = line = 0;
	for(t = 0; t <= SCRNLEN; t++) {
		linemod[t] = 1;
		for(i = 0; i <= SCRNWID; i++)
			screen[t][i] = ERASE;
	}
	if(hewlet) {
		wctl(ESCAPE);
		wctl(HPHOME);
		wctl(ESCAPE);
		wctl(HPCLEAR);
		return;
	}
	if(bantum) {
		wctl(ESCAPE);
		wctl(BTCLEAR);
		for(i = 0; i < 50; i++)
			wctl(0177);
		return;
	}
	if(omron) {
		wctl(ESCAPE);
		wctl(OMHOME);
		wctl(ESCAPE);
		wctl(OMCLEAR);
		dbyt();
		sleep(2);
		return;
	}
	if(lsiadm) {
		wctl(LSICLR);
		return;
	}
	if(modone) {
		wctl(M1LEADIN);
		wctl(M1CLR);
		return;
	}
	if(vt52) {
		wctl(ESCAPE);
		wctl(V5HOME);
		wctl(ESCAPE);
		wctl(V5CLEAR);
		return;
	}
	if(dg132a) {
		wctl(DGACLEAR);
		return;
	}
	if(vt100) {
		wctl(ESCAPE);
		wctl(V1BRACK);
		wctl(V1ONE);
		wctl(V1SEMI);
		wctl(V1ONE);
		wctl(V1ACH);
		wctl(ESCAPE);
		wctl(V1BRACK);
		wctl(V1TWO);
		wctl(V1JAY);
		return;
	}
	if(dd4000) {
		wctl(ESCAPE);
		wctl(DD4CLEAR);
		return;
	}
	wctl(H1200CLR);
}

/*

Name:
	posn

Function:
	Position cursor on screen.

Algorithm:
	Check for reasonable coordinates and keep in bounds.  Perform specific
	terminal commands to move cursor to absolute line/column coordinate on
	screen.

Parameters:
	Line and column

Returns:
	None.

	None.
Files and Programs:


*/
posn(l, c)
{
	register ll, cc, tlc;

	if(l < 0) l = 0;
	else if(l > SCRNLEN) l = SCRNLEN;
	if(c < 0) c = 0;
	else if(c > SCRNWID) c = SCRNWID;

	ll = line;
	cc = column;
	if(ll == l && cc == c) return;

	if(modone) {
		if(l == ll) {
			if((tlc = c - cc) < 0) tlc = -tlc;
			if(tlc <= 4) {
				while(c < column) wbyt(BACKSP);
				while(c > column) wbyt(M1SPACE);
				return;
			}
		}
		wctl(M1LEADIN);
		wctl(M1POSN);
		if(c < 32) wctl(c + 96);
		else wctl(c);
		wctl(l + 96);
		line = l;
		column = c;
		return;
	}
	if(vt52) {
		wctl(ESCAPE);
		wctl(V5WYE);
		wctl(l + 040);
		wctl(c + 040);
		line = l;
		column = c;
		return;
	}
	if(dg132a) {
		wctl(ESCAPE);
		wctl(DGAEIGHT);
		wctl(DGAZERO);
		ll = l + 1;
		wctl(ll/10 + '0');
		wctl(ll%10 + '0');
		wctl(DGAZERO);
		cc = c + 1;
		wctl(cc/10 + '0');
		wctl(cc%10 + '0');
		line = l;
		column = c;
		return;
	}
	if(vt100) {
		wctl(ESCAPE);
		wctl(V1BRACK);
		ll = l + 1;
		wctl(ll/10 + '0');
		wctl(ll%10 + '0');
		wctl(V1SEMI);
		cc = c + 1;
		wctl(cc/10 + '0');
		wctl(cc%10 + '0');
		wctl(V1ACH);
		line = l;
		column = c;
		return;
	}
	if(dd4000) {
		wctl(ESCAPE);
		wctl(DD4EFF);
		wctl(DD4ZERO);
		wctl(c/10 + '0');
		wctl(c%10 + '0');
		wctl(DD4ZERO);
		wctl(l/10 + '0');
		wctl(l%10 + '0');
		line = l;
		column = c;
		return;
	}
 	if(bantum) {
		if((tlc = (c - column)) <= 0) {
			if(tlc >= -3) {
				while(c < column)
					wbyt(BACKSP);
			} else {
				wctl(ESCAPE);
				wctl(BTCPOSN);
				wctl(c + BTPOSPLUS);
				column = c;
			}
		} else {
			wctl(ESCAPE);
			wctl(BTCPOSN);
			wctl(c + BTPOSPLUS);
			column = c;
		}
		if((tlc = (l - line)) < 0) {
			wctl(ESCAPE);
			wctl(BTLPOSN);
			wctl(l + BTPOSPLUS);
			line = l;
		} else {
			if(tlc <= 3)
				while(l > line) {
					wbyt(LF);
			} else {
				wctl(ESCAPE);
				wctl(BTLPOSN);
				wctl(l + BTPOSPLUS);
				line = l;
			}
		}

		return;
	}

	if(lsiadm) {
		if(l == ll) {
			if((tlc = c - cc) < 0) tlc = -tlc;
			if(tlc <= 4) {
				while(c < column) wbyt(BACKSP);
				while(c > column) wbyt(SPACE);
				return;
			}
		}
		wctl(ESCAPE);
		wctl(LSIPOSN);
		wctl(l+040);
		wctl(c+040);
		line = l;
		column = c;
		return;
	}
	if(omron) {
		if(l == ll) {
			if((tlc = c - cc) < 0) tlc = -tlc;
			if(tlc <= 4) {
				while(c < column) wbyt(BACKSP);
				while(c > column) {
					wctl(ESCAPE);
					wctl(OMSPACE);
					column++;
				}
				return;
			}
		}
		wctl(ESCAPE);
		wctl(OMADDR);
		ll = l + 1;
		wctl(OMBASE + ((ll / 16) % 16));
		wctl(OMBASE + (ll % 16));
		cc = c + 1;
		wctl(OMBASE + ((cc / 16) % 16));
		wctl(OMBASE + (cc % 16));
		line = l;
		column = c;
		return;
	}

	if(hewlet) {
		tlc = c - cc;
		if(tlc < 0) {
			if(-tlc > c+1)
				tlc = c + 1;
		}
		if(tlc > 0) {
			if(tlc >= 8)
				tlc =/ 8;
		} else
			tlc = -tlc;
		if(l - ll >= 0) tlc =+ l - ll;
		else tlc =+ -(l - ll) * 2;
		if(tlc <= 9) {
			tlc = l;
			for(         ; ll < tlc; ll++) wctl(LF);
			for(ll = line; ll > tlc; --ll) {
				wctl(ESCAPE);
				wctl(HPUP);
			}
			line = tlc;
			tlc = c;
			if((cc = column) > tlc && (cc - tlc) > tlc)
				wbyt(RETURN);
			if((cc = column) < tlc && (tlc - cc) >= 8) {
				ll = tlc - (tlc % 8);
				while(cc < ll) {
					wctl(TAB);
					cc =+ 8 - (cc % 8);
				}
				column = cc;
			}
			for(cc = column; cc > tlc; --cc) wbyt(BACKSP);
			for(cc = column ; cc < tlc; cc++) wbyt(SPACE);
		} else {
			wctl(ESCAPE);
			wctl(HPADDRS);
			wctl(HPLINE);
			if((tlc = l/10) > 0)
				wctl(tlc+'0');
			wctl(l-tlc*10 + '0');
			wctl(HPCOL);
			if((tlc = c/10) > 0)
				wctl(tlc+'0');
			wctl(c-tlc*10 + '0');
			wctl(HPADDRE);
		}
		line = l;
		column = c;
		return;
	}

	/* else Hazeltine 1200 */
	tlc = l;
	if(ll > tlc) {
		wctl(H1200HME);
		line = column = 0;
	}
	for(ll = line; ll < tlc; ll++) wbyt(LF);
	tlc = c;
	if(cc > tlc && (cc - tlc) > tlc) wbyt(RETURN);

	for(cc = column; cc > tlc; --cc) wbyt(BACKSP);
	for(cc = column; cc < tlc; cc++) wbyt(SPACE);
}

/*

Name:
	dbyt

Function:
	Display bytes.

Algorithm:
	Write out any waiting bytes.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
dbyt()
{
	register int t;

	write(1, bytbuf, bytcnt);
	bytcnt = 0;
}

/*

Name:
	getarg

Function:
	Get command argument from user.

Algorithm:
	Save dot and column pointers.  Set "Arg:" as beginning of top line
	and mark it as updated.  Read user input (forcing prompt to be flushed
	out) until command charcter is read.  Then reset dot and column pointers.

Parameters:
	None.

Returns:
	command character to be executed

Files and Programs:
	None.


*/
getarg()
{
	register int c;
	int acurcol, adot, atext;

	acurcol = curcol;
	adot = dot;
	dot = 0;
	curcol = 6;
	linemod[0] = 1;
	atext = text[0];
	text[0] = argline;
	textcol = 5;
	for(;;) {
		tctocc();
		curcol = cthis;
		argline[textcol] = 0;
		if(curcol >= SCRNWID) {
			bell();
			continue;
		}
		linemod[0] = 1;
		c = readc();
		if(c == BACKSP || c == RUBOUT) {
			if(textcol <= 5) continue;
			argline[--textcol] = 0;
			continue;
		}
		if(c == TAB || c & QUOTE) {
			argline[textcol++] = c & LOW7;
			continue;
		}
		if((c >= 0 && c < 040) || c == 0177) {
			argline[textcol] = 0;
			arg = 1;
			dot = adot;
			curcol = acurcol;
			text[0] = atext;
			linemod[0] = 1;
			cctotc();
			return(c);
		}
		argline[textcol++] = c;
	}
}

/*

Name:
	rword

Function:
	Move right to beginning of next word.

Algorithm:
	Find out whether you're between words now.  If not, skip over non-delimiters
	to end of current word.  Then skip over any multiple delimiters to first
	letter of word.  Adjust pointers.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
rword()
{
	register char *l, *b;
	register int typ;

	l = b = text[dot];
	l = &b[textcol];
	if(dot >= dol || *b == 0)
		return;
	typ = type(*l);
	if(typ) {
		while(*++l && type(*l) == typ) ;
		typ = type(*l);
	}
	if(typ == 0)
		while(*++l && type(*l) == 0) ;

	textcol = l - b;
	tctocc();
	curcol = cthis;
}

/*

Name:
	type

Function:
	Determine character class of argument.

Algorithm:
	Classes are alphanumeric (a-z, A-Z, 0-9), word delimiter (space,
	erase, tab), or other.

Parameters:
	character to be tested

Returns:
	1 = alphanumeric
	0 = delimiter
	-1 = other

Files and Programs:
	None.


*/
type(cc) char cc;
{
	register char c;

	c = cc;
	if((c >= 'a' && c <= 'z')
		|| (c >= '0' && c <= '9')
			|| (c >= 'A' && c <= 'Z')) return(1);
	if(c == SPACE || c == ERASE || c == TAB) return(0);
	return(-1);
}

/*

Name:
	lword

Function:
	Move left to beginning of previous word.

Algorithm:
	Find out whether you're between or at start of word now.
	If so, back up to previous word end.  Scan backwards to beginning
	of word.  Adjust pointers.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
lword()
{
	register char *l, *b;
	register int typ;

	if(dot >= dol || textcol <= 0) return;
	l = b = text[dot];
	l =+ textcol;
	typ = type(*l);
	if(typ == 0 || type(l[-1]) == 0)
		while(--l >= b && (typ = type(*l)) == 0) ;
	else
		typ = type(*--l);
	while(--l >= b && type(*l) == typ) ;
	if(l < b) textcol = 0;
	else textcol = (++l) - b;
	tctocc();
	if((curcol = cthis) <= 0 && !tcok)
		curcol = 1;
}

/*

Name:
	rmword

Function:
	Remove text word.

Algorithm:
	Check for illogical addresses and tell user.  Get copy of line to be
	modified.  For the specified number of words, get the length of all
	the words and remove that many characters.

Parameters:
	Number of words to be removed.

Returns:
	None.

Files and Programs:
	None.


*/
rmword(cnt)
{
	register int typ, count;
	register char *l;
	int t;

	if(!WALWD || dot >= dol) {
	   bl:	bell();
		return;
	}
	swap(dot);
	if(*(l = &chtext[textcol]) == 0)
		goto bl;
	count = 0;
	while(--cnt >= 0) {
		typ = type(*l);
		for(t = 0; *l != 0 && type(*l) == typ; t++) l++;
		count =+ t;
	}
	rmchar(count);
}

/*

Name:
	fwdsrch

Function:
	Search in forward direction.

Algorithm:
	Until end of screen, search for match on specified string in buffer.
	If not found, check for and perform wraparound on same line.
	If unsuccessful, ring bell.

Parameters:
	Wraparound indicator (1 = yes, 0 = no)\

Returns:
	None.

Files and Programs:
	None.


*/
fwdsrch(wrap)
{
	register col;
	register char *wl, *c;
	int lnum;
	int strtcol, end;

	strtcol = col = textcol;
	col++;
	lnum = dot;
	end = SCRNLEN + 1;
	for(;;) {
		while(lnum < end) {
			wl = text[lnum];
			for(c = &wl[col]; *c; c++)
				if(parteq(c, sbuf)) {
					found(lnum, c);
					return;
			}
			col = 0;
			lnum++;
		}
		if(wrap <= 0) break;
		lnum = 0;
		end = dot;
		wrap = -1;
	}
	if(wrap) {
		wl = text[(lnum = dot)];
		col = 0;
		for(c = &wl[col]; col++ < strtcol; c++)
			if(parteq(c, sbuf)) {
				found(lnum, c);
				return;
			}
	}
	bell();
}

/*

Name:
	bkwdsrch

Function:
	Search in backward direction.

Algorithm:
	Until beginning of screen, search for match on specified string in
	buffer.  Tell user if unsuccessful.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
bkwdsrch()
{
	register col;
	register char *wl, *c;
	int lnum;

	col = textcol;
	lnum = dot;
	wl = text[lnum];
	c = &wl[--col];
	for(;;) {
		for( ; col-- >= 0; --c)
			if(parteq(c, sbuf)) {
				found(lnum, c);
				return;
			}
		if(--lnum < 0) {
			bell();
			return;
		}
		wl = text[lnum];
		for(col = 1; *wl++; col++);
		c = wl;
	}
}

/*

Name:
	parteq

Function:
	Match 2 strings

Algorithm:
	Compare two strings for length of second one.

Parameters:
	Pointers to two strings.

Returns:
	1 = match
	0 = no match

Files and Programs:
	None.


*/
parteq(lsec,skey)
{
	register char *l, *s;

	l = lsec;
	s = skey;

	while(*l++ == *s++ && *s);

	return(*--l == *--s);
}

/*

Name:
	found

Function:
	Post process successful search.

Algorithm:
	If the argument buffer was used in search, save value in search buffer.
	If found columns are off screen, shift and repaint screen.

Parameters:
	Line number
	Column number

Returns:
	None.

Files and Programs:
	None.


*/
found(l, c) char *c;
{
	register char *wl, *cc;
	register t;

	if(sbuf == argbuf) {
		wl = argbuf;
		cc = srchbuf;
		while(*cc++ = *wl++);
	}
	cc = c;
	wl = text[l];

	for(t = 0; wl < cc; )
		if(*wl++ == TAB) t =+ (TABSIZ - (t % TABSIZ));
		else t++;

	t++;
	if(t < coffset || t > coffset + SCRNWID) {
		coffset = (t - (t % TABSIZ)) - TABSIZ * 5;
		if(coffset < 0) coffset = 0;
		setmod(0);
	}
	dot = l;
	curcol = t - coffset;
	cctotc();
	curcol = cthis;
}

/*

Name:
	pickbuf

Function:
	Get address of proper argument buffer.

Algorithm:
	check argument buffer, then remembered value in search buffer.

Parameters:
	None.

Returns:
	1 = something in buffer
	0 = buffer empty

Files and Programs:
	None.


*/
pickbuf()
{
	if(*(sbuf = argbuf) != 0)
		return(1);
	else
		return(*(sbuf = srchbuf) != 0);
}

/*

Name:
	rmline

Function:
	Remove line from file.

Algorithm:
	Check for illogical address and tell user.  Save lines for possible
	undoing.  For specified number of lines, free line text and adjust
	file end pointers.
	If more lines are needed from EDIT, get them.

Parameters:
	Starting line number
	Number of lines to remove

Returns:
	None.

Files and Programs:
	None.


*/
rmline(from, cnt) register int from, cnt;
{
	register int f;

	if(cnt <= 0 || from >= dol || from + cnt > dol) {
		bell();
		return;
	}
	if(chnum >= from)
		if(chnum < from+cnt)
			swap(NOLINE);
		else
			chnum =- cnt;
	setmod(from);
	save(from, cnt);
	for(f = cnt; from < dol; from++) {
		if(--f >= 0)
			txfree(from);
		text[from] = text[from + cnt];
	}

	dol =- cnt;
	textmod = 1;
	if(dol <= SCRNLEN && edxio.x_more)
		askedit(1);
}

/*

Name:
	rmchar

Function:
	Remove character from line.

Algorithm:
	Check for illogical address and tell user.  Get copy of line to be
	modified and mark it as changed.  Slide line to left over removed 
	characters and adjust end of line pointers.

Parameters:
	count of characters to be removed (-1 = entire line)

Returns:
	None.

Files and Programs:
	None.


*/
rmchar(cnt) register int cnt;
{
	register char *rp, *dp;

	if(!WALWD || dot >= dol) {
	   bl:	bell();
		return;
	}
	swap(dot);
	if(textcol >= chend)
		goto bl;
	chmod = linemod[dot] = 1;
	if(cnt == -1) {
		chtext[textcol] = 0;
		chend = textcol;
		return;
	}
	if(textcol + cnt > chend)
		cnt = chend - textcol;
	rp = &text[dot][textcol];
	dp = rp + cnt;
	while(*rp++ = *dp++) ;
	chend =- cnt;
	cctotc();
	curcol = cthis;
}

/*

Name:
	txalloc

Function:
	Allocate space in text array.

Algorithm:
	Call alloc() for space.  Save pointer in text array and put null
	at end of space.

Parameters:
	Line number (text array index)
	Length of line

Returns:
	Pointer to null at end of space.

Files and Programs:
	None.


*/
char *
txalloc(f, len) register int len;
{
	register char *p;

	text[f] = p = alloc(len);
	*p = 0;
	return(p);
}

/*

Name:
	txfree

Function:
	Free space in text array.

Algorithm:
	If not the change buffer, deallocate space.  Reset array pointer.

Parameters:
	Line number (text array index)

Returns:
	None.

Files and Programs:
	None.


*/
txfree(f)
{
	register char *p;

	if((p = text[f]) && p != chtext)
		free(p);
	text[f] = 0;
}

/*

Name:
	mkline

Function:
	Open up empty line space in file.

Algorithm:
	Check for error conditions (>100 lines or negative count) and inform
	user.  Expand line pointer space if needed to contain new lines.
	Slide existing lines down from point of expansion.  Repaint screen with
	open space included.  Allocate space for border on empty lines.

Parameters:
	Line number
	Number of lines

Returns:
	None.

Files and Programs:
	None.


*/
mkline(at, cnt) register int at, cnt;
{
	register int t;

	if(cnt > 100 || cnt <= 0) {
		bell();
		return;
	}
	if(at >= dol)
		return;
 	if(cnt + dol >= linesp)
		expand(cnt + dol);
	if(chnum >= at)
		chnum =+ cnt;
	for(t = dol - 1; t >= at; --t)
		text[t+cnt] = text[t];
	setmod(at);
	dol =+ cnt;
	for(t = at + cnt; at < t; at++)
		txalloc(at, 2);
	textmod = 1;
}

/*

Name:
	split

Function:
	Split a line into two lines.

Algorithm:
	Check for illogical conditions and tell user.  Get space for second
	half of pair by inserting empty line or expanding file by one line.
	Now free up space and copy second half into reallocated space.  Mark
	lines as changed and move dot if on-screen.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
split()
{
	register char *p;

	if(!WALWD || curcol <= 0) {
	   bl:	bell();
		return;
	}
	if(chmod == dot)
		swap(NOLINE);
	if(dot+1 < dol)
		mkline(dot+1, 1);
	else if(dot+1 == dol)
		newdol(dol);
	else
		goto bl;
	p = &text[dot][textcol];
	txfree(dot+1);
	copy(p, txalloc(dot+1, len(p)));
	text[dot][textcol] = 0;
	linemod[dot] = 1;
	if(dot < SCRNLEN) dot++;
	curcol = 1;
	cctotc();
	curcol = cthis;
}

/*

Name:
	join

Function:
	Join together two lines.

Algorithm:
	Get any argument value.  Tell user if illegal number.  Find and validate
	total length of potential join lines.  Join all lines and remove all
	but first (the new longer one).

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
join()
{
	register int cnt, totl, t;

	if(arg) {
		if((cnt = atoi(argbuf)) <= 0) {
		    bl: bell();
			return;
		}
	} else
		cnt = 1;
	if(dot + cnt >= dol)
		goto bl;
	swap(dot);
	for(totl = t = 0; t < cnt; )
		totl =+ len(text[dot + (++t)]) - 1;
	if(chend + totl > 511)
		goto bl;
	for(t = 0; t < cnt; )
		chend = copy(text[dot + (++t)], &chtext[chend]) - chtext;
	linemod[dot] = 1;
	chmod = 1;
	swap(NOLINE);
	rmline(dot+1, cnt);
}

/*

Name:
	save

Function:
	Save (cut) copy of text lines.

Algorithm:
	If called from command line, scan buffer for legal buffer name.  Save 
	name as last one accessed.  Notify user of illegal name.  Get number of
	lines to save.  If not entered, use default of 1.  Update the save table.

	If internal call, update the save table.

	In either case, copy and record pointer to text of saved lines into newly
	allocated space.

Parameters:
	Line number
	Number of lines to be saved (0 = invoked from cmd line, >0 = internal call)

Returns:
	None.

Files and Programs:
	None.


*/
save(from, cnt) register int from, cnt;
{
	register char **tbl;
	char *argp;

	if(!cnt) {
		if(*(argp = argbuf) >= 'a') lastsave = *argp++ - 'a';
		else lastsave = 0;
		if(lastsave < 0 || lastsave > 9) {
		   bl:	lastsave = -1;
			bell();
			return;
		}
		if((cnt = atoi(argp)) < 0)
			goto bl;
		if(cnt == 0)
			cnt = 1;
		tbl = savetabl[lastsave] = newtbl(savetabl[lastsave], cnt);
	} else {
		tbl = clostab = newtbl(clostab, cnt);
	}
	if(from + cnt > dol)
		goto bl;
	while(--cnt >= 0)
		copy(text[from+cnt], tbl[cnt] = alloc(len(text[from+cnt])));
}

/*

Name:
	newtbl

Function:
	Create new save table entry.

Algorithm:
	Deallocate old pointers for buffer entry.  Allocate new pointers and zero
	them out.

Parameters:
	Pointer to table entry
	Number of lines.

Returns:
	Pointer to table entry

Files and Programs:
	None.


*/
newtbl(tab, cnt) register char **tab; register int cnt; 
{
	register char **t;
	char **tp;

	if(t = tab) {
		while(*t)
			free(*t++);
		free(tab);
	}
	tp = t = alloc((++cnt) * 2);
	while(--cnt >= 0)
		*(t++) = 0;
	return(tp);
}

/*

Name:
	restore

Function:
	Restore (paste) copy of text lines.

Algorithm:
	If reference is made to actual buffer name, use it.  Otherwise,
	use last reference.  Check for valid buffer name.  Tell user of error.
	Get number of lines count from argument buffer.  Check for reasonableness
	with respect to table entry length.  If restored lines are at end of file,
	extend end of file.  Expand core table, if necessary.  Slide existing
	text down to make room for new text.  Copy saved lines into place and
	repaint screen.

Parameters:
	Line number after which restored lines should go

Returns:
	None.

Files and Programs:
	None.


*/
restore(to) register int to;
{
	register int cnt;
	int f;
	register char **tbl;
	char *argp;

	argp = argbuf;
	if(arg && *argp >= 'a')
		f = *argp++ - 'a';
	else
		f = lastsave;
	if(f == 'u' - 'a')
		tbl = clostab;
	else if(f >= 0 && f <= 9)
		tbl = savetabl[f];
	else {
	    bl: bell();
		return;
	}
	cnt = ilen(tbl);
	if((f = atoi(argp)) != 0 && f < cnt)
		cnt = f;
	if(cnt <= 0 || tbl == 0)
		goto bl;
	if(chnum >= to)
		chnum =+ cnt;
	if(to > dol)
		newdol(to);
	if(dol + cnt > linesp)
		expand(dol + cnt);
	for(f = dol - 1; f >= to; --f)
		text[f+cnt] = text[f];
	for(f = cnt; --f >= 0; )
		copy(tbl[f], txalloc(to + f, len(tbl[f])));
	dol =+ cnt;
	setmod(to);
	textmod = 1;
	cctotc();
}

/*

Name:
	readc

Function:
	Read a character from the user.

Algorithm:
	If character was previously read and saved, return it.  If no input is
	waiting, get and display a character.  Handle escaped characters.

Parameters:
	None.

Returns:
	Next character being input.

Files and Programs:
	None.


*/
readc()
{
	register int old, rc;
	char c;

	if(rc = peekc) {
		peekc = 0;
		return(rc);
	}
	if(!input() && display()) {
		if(text[line][textcol] == TAB && WALWD &&
			column && column < SCRNWID && line < dol) {
				old = screen[line][column];
				wbyt('T');
				wbyt(BACKSP);
		} else old = 0;
		dbyt();
		if(read(0, &c, 1) <= 0)
			exit(-1);
		readq[qcnt++] = c & LOW7;
		if(old) {
			wbyt(old);
			wbyt(BACKSP);
			dbyt();
		}
	}
	rc = readq[qnxt++];
	if(rc != ESCAPE && rc != '\\')
		return(rc);
	if(rc == ESCAPE)
		rc = '^';
	else if(!h1200)
		return(rc);
	do {
		old = 0;
		if(input()) {
		    notelse:
			c = readq[qnxt++];
		} else {
			if(!display())
				goto notelse;
			if(WALWD && column && column < SCRNWID && line < dol) {
				old = screen[line][column];
				wbyt(rc);
				wbyt(BACKSP);
				dbyt();
			}
			if(read(0, &c, 1) <= 0)
				exit(-1);
			if(old) {
				wbyt(old);
				wbyt(BACKSP);
				dbyt();
			}
		}
	} while((c =& LOW7) == 0);
	if(rc == '^') {
		if((c > 0 && c < 040) || c == 0177)
			c =| QUOTE;
	} else {
		switch(c) {
			case '\\':			break;
			case '(':	c = '{';	break;
			case ')':	c = '}';	break;
			case '!':	c = '|';	break;
			case '\'':	c = '`';	break;
			case '^':	c = '~';	break;
			default:   peekc = c; c = rc;	break;
		}
	}
	return(c);
}

/*

Name:
	input

Function:
	Determine number of input characters.

Algorithm:
	Either return number of characters already in queue or get a character
	and put it in the queue.

Parameters:
	None.

Returns:
	1 = character waiting

Files and Programs:
	None.


*/
input()
{
	register int bc, tot;
	char c;

	if(peekc)
		return(1);
	if(qnxt >= qcnt)
		qnxt = qcnt = 0;
	if((tot = qcnt - qnxt) != 0)
		return(tot);
	tot = 0;
	for(bc = brkcnt(); --bc >= 0; tot++) {
		if(qcnt >= QMAX)
			break;
		if(read(0, &c, 1) <= 0)
			exit(EIO);
		readq[qcnt++] = c & LOW7;
	}
	return(tot);
}

/*

Name:
	display

Function:
	Refill screen.

Algorithm:
	For length of screen, write modified lines.  Position cursor at current
	line and column.

Parameters:
	None.

Returns:
	1 = normal, 0 = error (input chars waiting)

Files and Programs:
	None.


*/
display()
{
	register int i;

	for(i = 0; i <= SCRNLEN; i++) {
		if(input())
			return(0);
		if(linemod[i])
			wline(i);
	}
	posn(dot, curcol);
	return(1);
}

/*

Name:
	ed

Function:
	Quit the screen editor and return control to the line editor.

Algorithm:
	Set the cursor at the lower left portion of the screen, putting
	out DEL's to waste time.  Tell editor the current status and write
	the modified file to the editor.  Update the screen, reset the
	terminal settings, character queues, and text array.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
ed()
{
	register int i;

	posn(SCRNLEN,0);
	for(i = 5; --i <= 0; )
		wctl(0177);
	dbyt();
	swap(NOLINE);
	edxio.x_count = textmod;
	if(dol && dot >= dol)
		edxio.x_dot = edxio.x_base + dol;
	else
		edxio.x_dot = edxio.x_base + dot;
	edxio.x_more = 0;
	write(pipout, &edxio, sizeof edxio);
	if(textmod)
		writfil();
	rmrplac();
	stty(1, &xedtty);
	clear();
	dot = edxio.x_dot;
	qcnt = qnxt = 0;
	curcol = 1;
	cctotc();
	curcol = cthis;
}

/*

Name:
	askedit

Function:
	Get text from editor.

Algorithm:
	Proceed only if forced.  Set up edit request.  Send request to editor.
	If file was modified, send it to editor.  Get new text for display.

Parameters:
	Flag whether to force edx to ask editor.

Returns:
	None.

Files and Programs:
	None.


*/
askedit(force)
{
	if(!force && edxio.x_base == oldbasel)
		return;
	swap(NOLINE);
	edxio.x_count = textmod;
	edxio.x_dot = edxio.x_base + dot;
	edxio.x_more = 1;
	write(pipout, &edxio, sizeof edxio);
	if(textmod)
		writfil();
	rmrplac();
}

/*

Name:
	rmplac

Function:
	Remove and replace text.

Algorithm:
	Read pipe from editor to find out what follows.  Expand in-core line
	table and fill it.  Repaint screen.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
rmrplac()
{
	register int t;

	pipread(&edxio, sizeof edxio);
	if(edxio.x_base == 0)
		edxio.x_base = 1;
	t = (edxio.x_count < SCRNLEN) ? SCRNLEN : edxio.x_count;
	scratch();
	expand(t);
	if(edxio.x_count)
		readfile();
	chnum = NOLINE;
	textmod = 0;
	oldbasel = edxio.x_base;
	setmod(0);
}

/*

Name:
	bell

Function:
	Output two bell characters.

Algorithm:
	Flush waiting output, then write out two bells.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
bell()
{
	dbyt();
	write(1,"\7\7",2);
}

/*

Name:
	setup

Function:
	Initialize screen editor.

Algorithm:
	Mask signals, get pipe file descriptors, prompt and get user's terminal
	type.  Perform any necessary initialization on terminal.  Get block of
	text from editor and clear screen.

Parameters:
	Count of command line arguments
	Pointer to array of argument pointers.

Returns:
	None.

Files and Programs:
	None.


*/
setup(argc, argv) char **argv;
{
	signal(2,1);
	signal(3,1);
	if(argc != 3) exit(-1);
	pipout = argv[2][0] - '0';
	pipin = argv[2][1] - '0';
	argv[2][0] = argv[2][1] = 0;
	typeit();
	gtty(1, &xedtty);
	xedtty.mode = 041 | (xedtty.mode & ~073);
	stty(1, &xedtty);
	if(vt52) {
		wctl(ESCAPE);
		wctl(V5BRACK);
		wctl(V5QUES);
		wctl(V5TWO);
		wctl(V5ELL);
	}
	if(vt100) {
		wctl(ESCAPE);
		wctl(V1LESS);
		wctl(ESCAPE);
		wctl(V1BRACK);
		wctl(V1QUES);
		wctl(V1SEVEN);
		wctl(V1ELL);
	}
	if(modone) {
		wctl(M1LEADIN);
		wctl(M1TYPE1);
	}
	copy("Arg: ", argline);
	rmrplac();
	clear();
	dot = edxio.x_dot;
	curcol = 1;
}

/*

Name:
	readfile

Function:
	Read file from editor

Algorithm:
	Read character count, then real text.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
readfile()
{
	register int cnt;

	for(dol = 0; (cnt = intread()); dol++)
		pipread(txalloc(dol, cnt), cnt);
	
}

/*

Name:
	pipread

Function:
	Read text from editor pipe.

Algorithm:
	For count, read chunks of pipe input from editor.  Exit on error.

Parameters:
	Buffer address.
	Count of characters needed.

Returns:
	EIO	I/O error

Files and Programs:
	None.


*/
pipread(l, cnt) register char *l; register int cnt;
{
	register int b;
	int t;

	b = 0;
	while(b < cnt && (t = read(pipin, &l[b], cnt - b)) > 0)
		b =+ t;
	if(t <= 0)
		exit(EIO);
}

/*

Name:
	wrtfil

Function:
	Write text to editor.

Algorithm:
	For length of file, send character count and data as a line through
	the pipe to the editor.  Finish with zero length count signal.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
writfil()
{
	register char *tx;
	register int f;
	int i;

	for(f = 0; f < dol; f++) {
		tx = text[f];
		i = len(tx);
		write(pipout, &i, 2);
		write(pipout, tx, --i);
		write(pipout, "\n", 1);
	}
	i = 0;
	write(pipout, &i, 2);
}

/*

Name:
	intread

Function:
	Read character count as integer.

Algorithm:
	Read integer from editor pipe as character count of what's to follow.

Parameters:
	None.

Returns:
	Value of integer

Files and Programs:
	None.


*/
intread()
{
	int intg; 

	pipread(&intg, 2);
	return(intg);
}

/*

Name:
	newdol

Function:
	Set a new end of file.

Algorithm:
	If you need space, get line space.  Mark the end as modified.
	Allocate empty lines after the new end.

Parameters:
	Line number.

Returns:
	None.

Files and Programs:
	None.


*/
newdol(l) register int l;
{
	if(l >= linesp)
		expand(l);
	setmod(dol);
	while(dol <= l)
		txalloc(dol++, 2);
	textmod = 1;
}

/*

Name:
	scratch

Function:
	Empty text block.

Algorithm:
	For range of lines, free up text space and zero text pointers.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
scratch()
{
	register int t;
	register char *p;

	for(t = dol; --t >= 0; ) {
		if((p = text[t]) && p != chtext)
			free(p);
		text[t] = 0;
	}
	if(text)
		free(text);
	text = 0;
	dol = linesp = 0;
}

/*

Name:
	expand

Function:
	Expand text array to new length.

Algorithm:
	Allocate up to line number, plus some extra 12 lines.

Parameters:
	Line number

Returns:
	None.

Files and Programs:
	None.


*/
expand(n) int n;
{
	register int *p1, *p2, m;
	int *t;

	if(n < linesp)
		return;
	t = p2 = alloc((n + LINEXTRA) * 2);
	for(m = n + LINEXTRA; --m >= 0; )
		*p2++ = 0;
	p2 = t;
	if(p1 = text) {
		for(m = linesp; --m >= 0; )
			*p2++ = *p1++;
		free(text);
	}
	linesp = n + LINEXTRA;
	text = t;
}

/*

Name:
	setmod

Function:
	Set modified flag.

Algorithm:
	From specified line to end of screen, set modified flag.
	Repaint of screen will occur in display().

Parameters:
	Beginning line number

Returns:
	None.

Files and Programs:
	None.


*/
setmod(l) register int l;
{
	register char *p;

	for(p = &linemod[l] ; l++ <= SCRNLEN; )
		*p++ = 1;
}

/*

Name:
	copy

Function:
	Copy first string to second area.

Algorithm:
	For length of first string, copy to second string pointer.

Parameters:
	Pointers to two strings.

Returns:
	Pointer to end of copy area.

Files and Programs:
	None.


*/
char *
copy(s1, s2) register char *s1, *s2;
{
	while(*s2++ = *s1++) ;
	return(--s2);
}

/*

Name:
	len

Function:
	Find length of character string.

Algorithm:
	Count number characters in character string to end of character string.

Parameters:
	Pointer to character string.

Returns:
	Length of character string.

Files and Programs:
	None.


*/
len(pp) register char *pp;
{
	register char *p;

	p = pp;
	while(*p++) ;
	return(p - pp);
}

/*

Name:
	ilen

Function:
	find length of integer string.

Algorithm:
	Count number of words in integer string to end of integer string.

Parameters:
	Pointer to integer string.

Returns:
	Length of string

Files and Programs:
	None.


*/
ilen(ii) register int *ii;
{
	register int *i;

	i = ii;
	while(*i++) ;
	return((--i) - ii);
}

/*

Name:
	alloc

Function:
	Bring C library alloc() routine to local environment.

Algorithm:
	Get minimum of one K from end of program.

Parameters:
	Number of bytes.

Returns:
	End of allocated area.

Files and Programs:
	None.


*/
#define	logical	char *

struct fb {
	logical	size;
	char	*next;
};

int	freelist[] {
	0,
	-1,
};
logical	slop	2;

char *
alloc(asize)
logical asize;
{
	register logical size;
	register logical np;
	register logical cp;

	if ((size = asize) == 0)
		return(0);
	size =+ 3;
	size =& ~01;
	for(;;) {
		for (cp=freelist; (np= cp->next) != -1; cp=np) {
			if (np->size>=size) {
				if (size+slop >= np->size) {
					cp->next = np->next;
					return(&np->next);
				}
				cp = cp->next = np+size;
				cp->size = np->size - size;
				cp->next = np->next;
				np->size = size;
				return(&np->next);
			}
		}
		asize = size<1024? 1024: size;
		if ((cp = sbrk(asize)) == -1) {
			return (-1);
		}
		cp->size = asize;
		free(&cp->next);
	}
}

/*

Name:
	free

Function:
	Bring C library free() routine into local environment.

Algorithm:
	Deallocate space previously allocated via alloc().

Parameters:
	Size of area to be deallocated.

Returns:
	None.

Files and Programs:
	None.


*/
free(aptr)
char *aptr;
{
	register logical ptr;
	register logical cp;
	register logical np;

	ptr = aptr-2;
	cp = freelist;
	while ((np = cp->next) < ptr)
		cp = np;
	if (ptr+ptr->size == np) {
		ptr->size =+ np->size;
		ptr->next = np->next;
		np = ptr;
	} else
		ptr->next = np;
	if (cp+cp->size == ptr) {
		cp->size =+ ptr->size;
		cp->next = ptr->next;
	} else
		cp->next = ptr;
}

/*

Name:
	typeit

Function:
	Find out terminal type.

Algorithm:
	Prompt user for terminal type and set appropriate flag.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
typeit()
{
	char b[SCRNWID];

	write(1,"Input first character of terminal type (from the following list):\n",66);
	write(1, "omron bantum hp lsi modone 100vt 54vt dg132a 4000dd: ", 52);
	if(read(0, b, SCRNWID) <= 0)
		exit(EINVAL);
	if(*b == 'm') modone++;
	else if(*b == 'l') lsiadm++;
	else if(*b == 'h') hewlet++;
	else if(*b == 'o') omron++;
	else if(*b == 'b') bantum++;
	else if(*b == '1') vt100++;
	else if(*b == '5') vt52++;
	else if(*b == 'd') dg132a++;
	else if(*b == '4') dd4000++;
	else exit(EINVAL);
	if(omron || lsiadm || hewlet || modone)
		funny++;
}
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
brkcnt() {
	return(0);
}
