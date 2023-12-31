/*
 *  Virtual terminal handler
 *  Written by Kenneth Almquist, AGS Computers  (HO 4C601, X7105).
 *  Modified by Stephen Hemminger, to use TERMCAP (without curses)
 */

#include <stdio.h>
#include <ctype.h>

#define ANSI	/* support multiple line insert/delete */
#define MAXPLEN 24
#define MAXLLEN 80
#define BOTLINE (ROWS - 1)
#define DIRTY 01

/* terminal escape sequences from termcap */
#define HO _tstr[0]		/* home */
#define CL _tstr[1]		/* clear screen */
#define CD _tstr[2]		/* clear to end of screen */
#define CE _tstr[3]		/* clear to end of line */
#define xUP _tstr[4]		/* up one line */
#define DO _tstr[5]		/* down one line */
#define US _tstr[6]		/* underline */
#define UE _tstr[7]		/* underline end */
#define BT _tstr[8]		/* backtab */
#define xBC _tstr[9]		/* backspace */
#define AL _tstr[10]		/* insert line */
#define DL _tstr[11]		/* delete line */
#define CM _tstr[12]		/* cursor move */
#define CH _tstr[13]		/* cursor horizontal move */
#define CV _tstr[14]		/* cursor vertical move */
#define CS _tstr[15]		/* scrolling region */
#define SF _tstr[16]		/* scroll forwards */
#define SR _tstr[17]		/* scroll backwards */
#define TI _tstr[18]		/* start cursor mode */
#define TE _tstr[19]		/* end cursor mode */
#define TA _tstr[20]		/* tab char (if not \t) */
#define CR _tstr[21]		/* carriage return (if not \r) */
#define xPC _tstr[22]		/* for reading pad character */
#ifdef ANSI
#define ALC _tstr[23]		/* insert multiple lines */
#define DLC _tstr[24]		/* delete multiple lines */
#endif
char PC;			/* pad character */
char *BC, *UP;			/* external variables for tgoto */

#ifdef ANSI
static char sname[] = "hoclcdceupdousuebtbcaldlcmchcvcssfsrtitetacrpcALDL";
char *_tstr[25];
#else
static char sname[] = "hoclcdceupdousuebtbcaldlcmchcvcssfsrtitetacrpc";
char *_tstr[23];
#endif
int     HOlen;			/* length of HO string */


/* terminal flags */
#define BS _tflg[0]		/* can backspace */
#define AM _tflg[1]		/* has auto margins */
#define XN _tflg[2]		/* no newline after wrap */
#define RET !_tflg[3]		/* has carriage return */
#define NS _tflg[4]		/* has SF (scroll forward) */
#define PT _tflg[5]		/* has tabs */
#define XT _tflg[6]		/* tabs are destructive */
int	GT = 1;			/* tab stops on terminal are set */

static char bname[] = "bsamxnncnsptxt";
char _tflg[7];


extern char *tgoto(), *tgetstr();
extern char *getenv(), *strcpy();

#define ULINE 0200
#define CURSEEN 1

/* Constants accessable by user */
int     hasscroll;		/* scrolling type, 0 == no scrolling */
int     ROWS;			/* number of lines on screen */
int     COLS;			/* width of screen */
int	needbeep;		/* user sets this to generate beep */

struct line {
	char    len;
	char    flags;
	char    l[MAXLLEN];
};

int     _row, _col;
int     _srow, _scol;
struct line _virt[MAXPLEN], _actual[MAXPLEN];
int     _uline = 0;
int     _junked = 1;
int     _curjunked;
int	_partupd;
int     _dir = 1;
int	_shifttop, _shiftbot;
int	_shift;
int	_scratched;
int     vputc();



/*
 * Tell refresh to shift lines in region upwards count lines.  Count
 * may be negative.  The virtual image is not shifted; this may change
 * later.  The variable _scratched is set to supress all attempts to
 * shift.
 */

ushift(top, bot, count) {
	if (_scratched)
		return;
	if (_shift != 0 && (_shifttop != top || _shiftbot != bot)) {
		_scratched++;
		return;
	}
	_shifttop = top;
	_shiftbot = bot;
	_shift += count;
}



/*
 * generate a beep on the terminal
 */

beep() {
	vputc('\7');
}

/*
 * Move to one line below the bottom of the screen.
 */

botscreen() {
	_amove(BOTLINE, 0);
	vputc('\n');
	vflush();
}



move(row, col) {
	if (row < 0 || row >= ROWS || col < 0 || col >= COLS)
		return;
	_row = row;
	_col = col;
}



/*
 * Output string at specified location.
 */

mvaddstr(row, col, str)
	char *str;
	{
	move(row, col);
	addstr(str);
}


addstr(s)
char   *s;
{
	register char  *p;
	register struct line   *lp;
	register int    col = _col;

	lp = &_virt[_row];
	if (lp->len < col) {
		p = &lp->l[lp->len];
		while (lp->len < col) {
			*p++ = ' ';
			lp->len++;
		}
	}
	for (p = s; *p != '\0'; p++) {
		if (*p == '\n') {
			lp->len = col;
			lp->flags |= DIRTY;
			col = 0;
			if (++_row >= ROWS)
				_row = 0;
			lp = &_virt[_row];
		}
		else {
			lp->l[col] = *p;
			lp->flags |= DIRTY;
			if (++col >= COLS) {
				lp->len = COLS;
				col = 0;
				if (++_row >= ROWS)
					_row = 0;
				lp = &_virt[_row];
			}
		}
	}
	if (lp->len <= col)
		lp->len = col;
	_col = col;
}



addch (c) {
	register struct line   *lp;
	register char  *p;

	lp = &_virt[_row];
	if (lp->len < _col) {
		p = &lp->l[lp->len];
		while (lp->len < _col) {
			*p++ = ' ';
			lp->len++;
		}
	}
	lp->l[_col] = c;
	if (lp->len == _col)
		lp->len++;
	if (++_col >= COLS) {
		_col = 0;
		if (++_row >= ROWS)
			_row = 0;
	}
	lp->flags |= DIRTY;
}



clrtoeol() {
	register struct line   *lp;

	lp = &_virt[_row];
	if (lp->len > _col) {
		lp->len = _col;
		lp->flags |= DIRTY;
	}
}


/*
 * Clear an entire line.
 */

clrline(row) {
	register struct line   *lp;

	lp = &_virt[row];
	if (lp->len > 0) {
		lp->len = 0;
		lp->flags |= DIRTY;
	}
}



clear() {
	erase();
	_junked++;
}



erase() {
	register    i;

	for (i = 0; i < ROWS; i++) {
		_virt[i].len = 0;
		_virt[i].flags |= DIRTY;
	}
}



/*
 * Leave lines as they are on the screen.
 */

nochange(line1, line2) {
	while (line1 <= line2) {
		_virt[line1] = _actual[line1] ;
		if (_junked)
			clrline(line1) ;
		line1++ ;
	}
}



refresh() {
	register int i;
	register int j;
	int len;
	int needchk;
	register char *p, *q;

/*
	if (checkin())
		return;
*/
	needchk = 0;
	i = 1;
	if (_junked) {
		_sclear();
		_junked = 0;
	} else if (! _scratched) {
		if (_shift) {
			needchk++;
			if (_shift > 0)
			_ushift(_shifttop, _shiftbot, _shift);
			else
			i = _dshift(_shifttop, _shiftbot, -_shift);
		} else {
			i = _dir;
		}
	}
	_dir = i;
	_dir = 1;		/* forget about _dir stuff */
	_shift = 0;
	_partupd = 1;
	if (needchk && checkin())
		return;
	_scratched = 0;
	_fixlines();
	for (i = _dir > 0 ? 0 : BOTLINE; i >= 0 && i < ROWS; i += _dir) {
		if ((_virt[i].flags & DIRTY) == 0)
			continue;
		_ckclrlin(i);		/* decide whether to do a clear line */
					/* probably should consider cd as well */
		len = _virt[i].len;
		if (_actual[i].len < len)
			len = _actual[i].len;
		p = _virt[i].l;
		q = _actual[i].l;
		for (j = 0; j < len; j++) {
			if (*p != *q) {
				_amove(i, j);
				_aputc(*p);
				*q = *p;
			}
			p++, q++;
		}
		len = _virt[i].len;
		if (_actual[i].len > len) {
			_clrtoeol(i, len);
		}
		else {
			for (; j < len; j++) {
				if (*p != ' ') {
					_amove(i, j);
					_aputc(*p);
				}
				*q++ = *p++;
			}
			_actual[i].len = len;
		}
		if (checkin())
			return;
	}
	_dir = 1;
	if (needbeep) {
		beep();
		needbeep = 0;
	}
	if (CURSEEN)
		_amove(_row, _col);
	vflush();			/* flush output buffer */
	_partupd = 0;
}


#define badshift(i, count) (_actual[i].len != _virt[i+count].len || strncmp(_actual[i].l, _virt[i+count].l, _actual[i].len) != 0)

_dshift(top, bot, count) {
	register    i;

	if (count >= bot - top || hasscroll < 4) {  /* must have CS or AL/DL */
		_scratched++;
		return 1;
	}
	for (i = bot - count; _actual[i].len == 0 || _partupd && badshift(i, count); i--)
		if (i == top)
			return 1;
	for (i = top; i <= bot; i++)
		_virt[i].flags |= DIRTY;
	for (i = bot; i >= top + count; i--)
		_actual[i] = _actual[i - count];
	for (; i >= top; i--)
		_actual[i].len = 0;

	if (hasscroll != 5) {		/* can we define scrolling region, and scroll back */
		tputs(tgoto(CS, bot, top), 1, vputc);/* define scroll region */
		_curjunked = 1;
		_amove(top, 0);
		for (i = count; --i >= 0;)
			tputs(SR, 1, vputc);/* scroll back */
		tputs(tgoto(CS, BOTLINE, 0), 1, vputc);
		_curjunked = 1;
	} else {
		_amove(bot - count + 1, 0);
		if (CD && bot == BOTLINE)
			tputs(CD, 1, vputc);
		else {
#ifdef ANSI
			if (DLC && count > 1)
				tputs(tgoto(DLC, count, count), 0, vputc);
			else
#endif
			for (i = count; --i >= 0;)
				tputs(DL, ROWS - _srow, vputc);
		}
		_amove(top, 0);
#ifdef ANSI
		if (count > 1 && ALC)
			tputs(tgoto(ALC, count, count), 0, vputc), count=0;
#endif
		for (i = count; --i >= 0;)
			tputs(AL, ROWS - _srow, vputc);
	}
	return -1;
}


_ushift(top, bot, count) {
	register    i;

	if (count >= bot - top || hasscroll == 0) {
		return;
	}
	for (i = top + count; _actual[i].len == 0 || _partupd && badshift(i, -count); i++)
		if (i == bot)
			return;
	if (hasscroll == 1 || hasscroll == 3) {
		/* we cheat and shift the entire screen */
		/* be sure we are shifting more lines into than out of position */
		if ((bot - top + 1) - count <= ROWS - (bot - top + 1))
			return;
		top = 0, bot = BOTLINE;
	}
	for (i = top; i <= bot; i++)
		_virt[i].flags |= DIRTY;
	for (i = top; i <= bot - count; i++)
		_actual[i] = _actual[i + count];
	for (; i <= bot; i++)
		_actual[i].len = 0;

	if (hasscroll != 5) {
		if (top != 0 || bot != BOTLINE) {
			tputs(tgoto(CS, bot, top), 0, vputc);
			_curjunked = 1;
		}
		_amove(bot, 0);	/* move to bottom */
		for (i = 0; i < count; i++) {
			if (SF)		/* scroll forward */
				tputs(SF, 1, vputc);
			else
				vputc('\n');
		}
		if (top != 0 || bot != BOTLINE) {
			tputs(tgoto(CS, BOTLINE, 0), 0, vputc);
			_curjunked = 1;
		}
	} else {
		_amove(top, 0);
#ifdef ANSI
		if (DLC && count > 1)
			tputs(tgoto(DLC, count, count), 0, vputc);
		else
#endif
		for (i = count; --i >= 0;)
			tputs(DL, ROWS - _srow, vputc);
		if (bot < BOTLINE) {
			_amove(bot - count + 1, 0);
#ifdef ANSI
			if (ALC && count > 1)
				tputs(tgoto(ALC, count, count), 0, vputc), count=0;
#endif
			for (i = count; --i >= 0;)
				tputs(AL, ROWS - _srow, vputc);
		}
	}
}


_sclear() {
	register struct line   *lp;

	tputs(CL, 0, vputc);
	_srow = _scol = 0;
	for (lp = _actual; lp < &_actual[ROWS]; lp++) {
		lp->len = 0;
	}
	for (lp = _virt; lp < &_virt[ROWS]; lp++) {
		if (lp->len != 0)
			lp->flags |= DIRTY;
	}
}


_clrtoeol(row, col) {
	register struct line *lp = &_actual[row];
	register i;

	if (CE && lp->len > col + 1) {
		_amove(row, col);
		tputs(CE, 1, vputc);
	} else {
		for (i = col ; i < lp->len ; i++) {
			if (lp->l[i] != ' ') {
				_amove(row, i);
				_aputc(' ');
			}
		}
	}
	lp->len = col;
}


_fixlines() {
	register struct line   *lp;
	register char  *p;
	register int    i;

	for (i = 0; i < ROWS; i++) {
		lp = &_virt[i];
		if (lp->flags & DIRTY) {
			lp = &_virt[i];
			for (p = &lp->l[lp->len]; --p >= lp->l && *p == ' ';);
			lp->len = (int)(p - lp->l) + 1;
			if (lp->len == _actual[i].len && strncmp(lp->l, _actual[i].l, lp->len) == 0)
				lp->flags &= ~DIRTY;
		}
	}
}


/*
 * Consider clearing the line before overwriting it.
 * We always clear a line if it has underlined characters in it
 * because these can cause problems.  Otherwise decide whether
 * that will decrease the number of characters to change.  This
 * routine could probably be simplified with no great loss.
 */

_ckclrlin(i) {
	int     eval;
	int     len;
	int     first;
	register struct line   *vp, *ap;
	register int    j;

	if (!CE)
		return;
	ap = &_actual[i];
	vp = &_virt[i];
	len = ap->len;
	eval = -strlen(CE);
	if (len > vp->len) {
		len = vp->len;
		eval = 0;
	}
	for (j = 0; j < len && vp->l[j] == ap->l[j]; j++);
	if (j == len)
		return;
	first = j;
	while (j < len) {
		if (vp->l[j] == ' ') {
			if (ap->l[j] != ' ') {
				while (++j < len && vp->l[j] == ' ' && ap->l[j] != ' ') {
					eval++;
				}
				if (j == len)
					eval++;
				continue;
			}
		}
		else {
			if (vp->l[j] == ap->l[j]) {
				while (++j < len && vp->l[j] == ap->l[j]) {
					eval--;
				}
				continue;
			}
		}
		j++;
	}
	if (US) {
		for (j = 0 ; j < ap->len ; j++) {
			if (ap->l[j] & ULINE) {
				eval = 999;
				if (first > j)
					first = j;
				break;
			}
		}
	}
	for (j = first; --j >= 0;)
		if (vp->l[j] != ' ')
			break;
	if (j < 0)
		first = 0;
	if (eval > 0) {
		_amove(i, first);
		tputs(CE, 0, vputc);
		_actual[i].len = first;
	}
}



/*
 * Move routine
 * 	first compute direct cursor address string and cost
 *	then relative motion string and cost,
 *	then home then relative and cost
 *	choose smallest and do it.
 *
 *	The plod stuff is to build the strings (with padding) then decide
 */
static char *plodstr;		/* current location in relmove string */

plodput(c) { *plodstr++ = c; }

_amove(row, col) {
	char direct[20];
	char rel[4 * MAXPLEN + 3 * MAXLLEN];
	char ho[3 * MAXPLEN + 3 * MAXLLEN];
	int cost, newcost;
	register char *movstr;

	if (row == _srow && col == _scol && _curjunked == 0)
		return;
	_setul(0);

	cost = 999;
	if (CM) {
		plodstr = direct;
		tputs(tgoto(CM, col, row), 0, plodput);
		*plodstr = '\0';
		cost = plodstr - direct;
		movstr = direct;
	}
	if (_curjunked == 0) {
		plodstr = rel;
		if (_vmove(_srow, row) >= 0
		 && plodstr - rel < cost
		 && _hmove(_scol, col, row) >= 0
		 && (newcost = plodstr - rel) < cost) {
			*plodstr = '\0';
			cost = newcost;
			movstr = rel;
		}
	}
	if (cost > HOlen) {	/* is it worth calculating */
		plodstr = ho;
		tputs(HO, 0, plodput);
		if (_vmove(0, row) >= 0
		 && plodstr - rel < cost
		 && _hmove(0, col, row) >= 0
		 && (newcost = plodstr - ho) < cost) {
			*plodstr = '\0';
			cost = newcost;
			movstr = ho;
		}
	}

	while (--cost >= 0) {
		vputc(*movstr++);
	}
	_srow = row, _scol = col;
	_curjunked = 0;
}


_vmove(orow, nrow) {
	char direct[64];
	char *saveplod = plodstr;

	if (CV) {
		plodstr = direct;
		tputs(tgoto(CV, nrow, nrow), 0, plodput);
		*plodstr = '\0';
		plodstr = saveplod;
	}
	if (orow > nrow) {		/* cursor up */
		if (! UP)
			return -1;
		while (orow > nrow) {
			tputs(UP, 1, plodput);
			orow--;
		}
	}
	while (orow < nrow) {		/* cursor down */
		if (DO)
			tputs(DO, 1, plodput);
		else
			*plodstr++ = '\n';
		orow++;
	}
	if (CV && plodstr - saveplod >= strlen(direct)) {
		register char *p;
		plodstr = saveplod;
		for (p = direct ; *plodstr = *p++ ; plodstr++);
	}
	return 0;
}


_hmove(ocol, ncol, row) {
	char direct[64];
	char ret[3 * MAXLLEN];
	char *saveplod = plodstr;
	char *movstr;
	int cost, newcost;

	cost = 999;
	if (CH) {
		plodstr = direct;
		tputs(tgoto(CH, ncol, ncol), 0, plodput);
		cost = plodstr - direct;
		movstr = direct;
		plodstr = saveplod;
	}
	if (RET && ocol > ncol) {	/* consider doing carriage return */
		plodstr = ret;
		if (CR)
			tputs(CR, 1, plodput);
		else
			*plodstr++ = '\r';
		if (_relhmove(0, ncol, row) >= 0
		 && (newcost = plodstr - ret) < cost) {
			cost = newcost;
			movstr = ret;
		}
		plodstr = saveplod;
	}
	if (_relhmove(ocol, ncol, row) < 0) {
		if (cost == 999)
			return -1;
		goto copy;
	}
	if (plodstr - saveplod > cost) {
copy:		plodstr = saveplod;
		while (--cost >= 0)
			*plodstr++ = *movstr++;
	}
	return 0;
}



_relhmove(ocol, ncol, row) {
	int tab;

	if (ocol < ncol && PT && GT) {	/* tab (nondestructive) */
		while ((tab = (ocol + 8) & ~07) <= ncol) {
			if (TA)
				tputs(TA, 1, plodput);
			else
				*plodstr++ = '\t';
			ocol = tab;
		}
		if (tab < COLS && tab - ncol < ncol - ocol) {
			if (TA)
				tputs(TA, 1, plodput);
			else
				*plodstr++ = '\t';
			ocol = tab;
		}
	} else if (BT && GT && ocol > ncol) {	/* backwards tab */
		while ((tab = (ocol - 1) &~ 07) >= ncol) {
			if (BS && tab == ocol - 1) {
				if (BC)
					tputs(BC, 1, plodput);
				else
					*plodstr++ = '\b';
			} else
				tputs(BT, 1, plodput);
			ocol = tab;
		}
		if (ncol - tab + 1 < ocol - ncol) {
			tputs(BT, 1, plodput);
			ocol = tab;
		}
	}
	if (ocol > ncol) {			/* cursor left */
		if (! BS)
			return -1;
		while (ocol > ncol) {
			if (BC != NULL)
				tputs(BC, 1, plodput);
			else
				*plodstr++ = '\b';
			ocol--;
		}
	}
	if (ocol < ncol) {			/* cursor right */
		register struct line *lp = &_actual[row];
		/*
		 * This code doesn't move over underlined characters properly,
		 * but in practice this doesn't seem to matter.
		 */
		while (ocol < ncol) {
			if (ocol < lp->len)
				*plodstr++ = lp->l[ocol];
			else
				*plodstr++ = ' ';
			ocol++;
		}
	}
	return 0;
}



_aputc(c) {
	if (AM && _scol == COLS - 1 && _srow == ROWS - 1)
		return;
	_setul(c & ULINE);
	vputc(c & ~ULINE);
	if (++_scol >= COLS) {
		if (! AM) {
			_scol--;
		} else  if (XN) {
			_curjunked++;
		} else {
			_scol = 0;
			++_srow;
		}
	}
}


_setul(on) {
	if (on) {
		if (_uline == 0 && US != NULL) {
			tputs(US, 1, vputc);
			_uline = 1;
		}
	}
	else {
		if (_uline != 0 && UE != NULL) {
			tputs(UE, 1, vputc);
			_uline = 0;
		}
	}
}


/*
 * Initialize termcap strings for later use.
 */
initterm() {
	static char tcbuf[1024];	/* termcap buffer */
	register char  *cp;

	if ((cp = getenv("TERM")) == NULL)
		xerror("TERM not set in environment");

	switch (tgetent(tcbuf, cp)) {
		case 0:
			xerror("Terminal not found in TERMCAP");
		case -1:
			xerror("Can't open /etc/termcap");
		case 1:
			break;
	}

	if ((ROWS = tgetnum("li")) == -1
	 || (COLS = tgetnum("co")) == -1)
		xerror("Can't get screen size");
	_zap();

	if (CL == NULL)
		xerror ("No clear screen defined");

	if (HO == NULL && CM == NULL)
		xerror("No home or cursor addressing");
	if (HO)
		HOlen = strlen(HO);
	else
		HOlen = 999;

	PC = xPC ? xPC[0] : 0;
	BC = xBC;
	UP = xUP;

	if (tgetnum("ug") > 0)
		US = UE = NULL;

	if (XT)				/* Destructive tab code not included */
		PT = 0;			/* to keep things simple */

	if (ROWS > MAXPLEN)
		ROWS = MAXPLEN;
	if (COLS > MAXLLEN) {
		COLS = MAXLLEN;
		AM = XN = 1;
	}

	/* Select article scrolling algorithm.  We prefer scrolling region
	   over insert/delete line because it's faster on the HP */
	hasscroll = 0;
	if (!NS) {
		hasscroll = 1;
		if (SR)
			hasscroll = 3;
		if (CS)
			hasscroll++;
	}
	if (AL && DL && hasscroll != 4)
		hasscroll = 5;
}


rawterm() {
	if (TI != NULL)
		tputs(TI, 0, vputc);
}


cookedterm() {
	if (TE != NULL) {
		tputs(TE, 0, vputc);
		vflush() ;
	}
}


/* get strings from termcap */
_zap() {
	static char tstrbuf[1024];
	static char *tp;
	register char  *namp, **sp, *bp;

	tp = tstrbuf;
	sp = _tstr;
	for (namp = sname; *namp; namp += 2) {
		*sp++ = tgetstr(namp, &tp);
	}
	bp = _tflg;
	for (namp = bname; *namp; namp += 2) {
		*bp++ = tgetflag(namp, &tp);
	}
}
