/*
 * Termcap/terminfo display driver
 *
 * Termcap is a terminal information database and routines to describe 
 * terminals on most UNIX systems.  Many other systems have adopted
 * this as a reasonable way to allow for widly varying and ever changing
 * varieties of terminal types.  This should be used where practical.
 */
/* Known problems:
 *	tputs is always called with the number of lines affected set to
 *	one.  Therefore, padding may be insufficient on some sequences
 *	dispite termcap being set up correctly.
 *
 *	If you have a terminal with no clear to end of screen and
 *	memory of lines below the ones visible on the screen, display
 *	will be wrong in some cases.  I doubt that any such terminal
 *	was ever made, but I thought everyone with delete line would
 *	have clear to end of screen too...
 *
 *	Code for terminals without clear to end of screen and/or clear
 *	to end of line has not been extensivly tested.
 *
 *	Cost calculations are very rough.  Costs of insert/delete line
 *	may be far from the truth.  This is accentuated by display.c
 *	not knowing about multi-line insert/delete.
 *
 *	Using scrolling region vs insert/delete line should probably
 *	be based on cost rather than the assuption that scrolling
 *	region operations look better.
 */
#include	"def.h"

#define	BEL	0x07			/* BEL character.		*/
#define	LF	0x0A			/* Line feed.			*/

extern	int	ttrow;
extern	int	ttcol;
extern	int	tttop;
extern	int	ttbot;
extern	int	tthue;

int	tceeol;			/* Costs are set later */
int	tcinsl;
int	tcdell;

static	int	insdel;		/* Do we have both insert & delete line? */

char	*tgetstr();
char	*tgoto();
int	ttputc();

#define TCAPSLEN 1024

char tcapbuf[TCAPSLEN];

/* PC, UP, and BC are used by termlib, so must be extern and have these
 * names unless you have a non-standard termlib.
 */

int	LI;			/* standard # lines */
char    PC,
        *CM,
        *CE,
        *UP,
	*BC,
	*IM,			/* insert mode */
	*IC,			/* insert a single space */
	*EI,			/* end insert mode */
	*DC,
	*AL,			/* add line */
	*DL,			/* del line */
	*pAL,			/* parameterized add line */
	*pDL,			/* parameterized delete line */
	*TI,			/* term init -- start using cursor motion */
	*TE,			/* term end --- end using cursor motion */
	*SO,
	*SE,
        *CD,
	*CS,			/* set scroll region			*/
	*SR;			/* back index (used with scroll region	*/
#ifdef	XKEYS
char	*K[NFKEYS],		/* other function key codes		*/
	*L[NFKEYS],		/* labels for other functions keys	*/
	*KS, *KE,		/* enter keypad mode, exit keypad mode	*/
	*KH, *KU, *KD, *KL, *KR; /* home, arrow keys			*/
#endif
int	SG;	/* number of glitches, 0 for invisable, -1 for none	*/
	/* (yes virginia, there are terminals with invisible glitches)	*/

/*
 * Initialize the terminal when the editor
 * gets started up.
 */
static char tcbuf[1024];

ttinit() {
        char *getenv();
        char *t, *p, *tgetstr();
        char *tv_stype;
#ifdef	XKEYS
	char kname[3], lname[3];
	int i;
#endif

#ifdef	VAXC
        if ((tv_stype = trnlnm("TERM")) == NULL)
#else
        if ((tv_stype = getenv("TERM")) == NULL)/* Don't want VAX C getenv() */
#endif
                panic("Environment variable TERM not defined!");

        if((tgetent(tcbuf, tv_stype)) != 1)
	{
		(VOID) strcpy(tcbuf, "Unknown terminal type ");
		(VOID) strcat(tcbuf, tv_stype);
                panic(tcbuf);
        }

        p = tcapbuf;
        t = tgetstr("pc", &p);
        if(t) PC = *t;

	LI = tgetnum("li");
        CD = tgetstr("cd", &p);
        CM = tgetstr("cm", &p);
        CE = tgetstr("ce", &p);
        UP = tgetstr("up", &p);
	BC = tgetstr("bc", &p);
	IM = tgetstr("im", &p);
	IC = tgetstr("ic", &p);
	EI = tgetstr("ei", &p);
	DC = tgetstr("dc", &p);
	AL = tgetstr("al", &p);
	DL = tgetstr("dl", &p);
	pAL= tgetstr("AL", &p);	/* parameterized insert and del. line */
	pDL= tgetstr("DL", &p);
	TI = tgetstr("ti", &p);
	TE = tgetstr("te", &p);
	SO = tgetstr("so", &p);
	SE = tgetstr("se", &p);
	CS = tgetstr("cs", &p); /* set scrolling region */
	SR = tgetstr("sr", &p);
	SG = tgetnum("sg");	/* standout glitch 	*/
#ifdef	XKEYS
	/* get the 10 standard termcap keys */
	strcpy(kname,"kx");
	strcpy(lname,"lx");
	for (i = 0; i < 10; i++) {
		kname[1] = i + '0';
		K[i] = tgetstr(kname, &p);
		lname[1] = i + '0';
		L[i] = tgetstr(lname, &p);
	}
	/* Hack to get another bunch */
	strcpy(kname,"Kx");
	strcpy(lname,"Lx");
	for (i = 0; i < 10; i++) {
		kname[1] = i + '0';
		K[10 + i] = tgetstr(kname, &p);
		lname[1] = i + '0';
		L[10 + i] = tgetstr(lname, &p);
	}

	/* Get the rest of the sequences */
	KS = tgetstr("ks", &p);
	KE = tgetstr("ke", &p);
	KH = tgetstr("kh", &p);
	KU = tgetstr("ku", &p);
	KD = tgetstr("kd", &p);
	KL = tgetstr("kl", &p);
	KR = tgetstr("kr", &p);
#endif

        if(CM == NULL || UP == NULL)
            panic("This terminal is to stupid to run MicroGnuEmacs\n");
	ttresize();			/* set nrow & ncol	*/

	/* watch out for empty capabilities (sure to be wrong)	*/
	if (CE && !*CE) CE = NULL;
	if (CS && !*CS) CS = NULL;
	if (SR && !*SR) SR = NULL;
	if (AL && !*AL) AL = NULL;
	if (DL && !*DL) DL = NULL;
	if (pAL && !*pAL) pAL = NULL;
	if (pDL && !*pDL) pDL = NULL;
	if (CD && !*CD) CD = NULL;

	if(!CE)	tceeol = ncol;
	else 	tceeol = charcost(CE);

	/* Estimate cost of inserting a line */
	if (CS && SR)	tcinsl = charcost(CS)*2 + charcost(SR);
	else if (pAL)   tcinsl = charcost(pAL);
	else if (AL)    tcinsl = charcost(AL);
	else		tcinsl = NROW * NCOL;	/* make this cost high enough */

	/* Estimate cost of deleting a line */
	if (CS)		tcdell = charcost(CS)*2 + 1;
	else if (pDL)   tcdell = charcost(pDL);
	else if (DL)    tcdell = charcost(DL);
	else		tcdell = NROW * NCOL;	/* make this cost high enough */

	/* Flag to indicate that we can both insert and delete lines */
	insdel = (AL || pAL) && (DL || pDL);

        if (p >= &tcapbuf[TCAPSLEN])
		panic("Terminal description too big!\n");
	if (TI && *TI) putpad (TI);	/* init the term */
}

/*
 * Clean up the terminal, in anticipation of
 * a return to the command interpreter. This is a no-op
 * on the ANSI display. On the SCALD display, it sets the
 * window back to half screen scrolling. Perhaps it should
 * query the display for the increment, and put it
 * back to what it was.
 */
tttidy() {
	if (TE && *TE) putpad (TE);	/* set the term back to normal mode */
#ifdef	XKEYS
	ttykeymaptidy();
#endif
}

/*
 * Move the cursor to the specified
 * origin 0 row and column position. Try to
 * optimize out extra moves; redisplay may
 * have left the cursor in the right
 * location last time!
 */
ttmove(row, col) {
    char        *tgoto();

    if (ttrow!=row || ttcol!=col) {
	putpad(tgoto(CM, col, row));
	ttrow = row;
	ttcol = col;
    }
}

/*
 * Erase to end of line.
 */
tteeol() {
    if(CE) putpad(CE);
    else {
	register int i=ncol-ttcol;
	while(i--) ttputc(" ");
	ttrow = ttcol = HUGE;
    }
}

/*
 * Erase to end of page.
 */
tteeop() {
    if(CD) putpad(CD);
    else {
    	putpad(CE);
	if (insdel) ttdell(ttrow + 1, LI, LI - ttrow - 1);
	else {		/* do it by hand */
	    register int line;
	    for (line = ttrow + 1; line <= LI; ++line) {
	    	ttmove(line, 0);
		tteeol();
	    }
	}
	ttrow = ttcol = HUGE;
    }
}

/*
 * Make a noise.
 */
ttbeep() {
	ttputc(BEL);
	ttflush();
}

/*
 * Insert nchunk blank line(s) onto the
 * screen, scrolling the last line on the
 * screen off the bottom.  Use the scrolling
 * region if possible for a smoother display.
 * If no scrolling region, use a set
 * of insert and delete line sequences
 */
ttinsl(row, bot, nchunk) {
    register int	i;
    
    if (row == bot) {		/* Case of one line insert is 	*/
	ttmove(row, 0);		/*	special			*/
	tteeol();
	return;
    }
    if (CS && SR) {		/* Use scroll region and back index	*/
	ttwindow(row,bot);
	ttmove(row, 0);
	while (nchunk--) putpad(SR);
	ttnowindow();
	return;
    } else if (insdel) {
	ttmove(1+bot-nchunk, 0);
	if (pDL) putpad (tgoto(pDL, 0, nchunk));
	else for (i=0; i<nchunk; i++)	/* For all lines in the chunk	*/
		putpad(DL);
	ttmove(row, 0);
	if (pAL) putpad (tgoto(pAL, 0, nchunk));
	else for (i=0; i<nchunk; i++)	/* For all lines in the chunk	*/
		putpad(AL);
	ttrow = HUGE;
	ttcol = HUGE;
    } else panic("ttinsl: Can't insert/delete line");
}

/*
 * Delete nchunk line(s) from "row", replacing the
 * bottom line on the screen with a blank line. 
 * Unless we're using the scrolling region, this is 
 * done with a crafty sequences of insert and delete 
 * lines.  The presence of the echo area makes a
 * boundry condition go away.
 */
ttdell(row, bot, nchunk)
{
    register int	i;
    
    if (row == bot) {		/* One line special case	*/
	ttmove(row, 0);
	tteeol();
	return;
    }
    if (CS) {			/* scrolling region	*/
	ttwindow(row, bot);
	ttmove(bot, 0);
	while (nchunk--) ttputc(LF);
	ttnowindow();
    }
    else if(insdel) {
	ttmove(row, 0);			/* Else use insert/delete line	*/
	if (pDL) putpad (tgoto(pDL, 0, nchunk));
	else for (i=0; i<nchunk; i++)	/* For all lines in the chunk	*/
		putpad(DL);
	ttmove(1+bot-nchunk,0);
	if (pAL) putpad (tgoto(pAL, 0, nchunk));
	else for (i=0; i<nchunk; i++)	/* For all lines in the chunk	*/
		putpad(AL);
	ttrow = HUGE;
	ttcol = HUGE;
    } else panic("ttdell: Can't insert/delete line");
}

/*
 * This routine sets the scrolling window
 * on the display to go from line "top" to line
 * "bot" (origin 0, inclusive). The caller checks
 * for the pathalogical 1 line scroll window that
 * doesn't work right, and avoids it. The "ttrow"
 * and "ttcol" variables are set to a crazy value
 * to ensure that the next call to "ttmove" does
 * not turn into a no-op (the window adjustment
 * moves the cursor).
 * 
 */
ttwindow(top, bot)
{
	if (CS && (tttop!=top || ttbot!=bot)) {
		putpad(tgoto(CS, bot, top));
		ttrow = HUGE;			/* Unknown.		*/
		ttcol = HUGE;
		tttop = top;			/* Remember region.	*/
		ttbot = bot;
	}
}

/*
 * Switch to full screen scroll. This is
 * used by "spawn.c" just before is suspends the
 * editor, and by "display.c" when it is getting ready
 * to exit.  This function gets to full screen scroll 
 * by telling the terminal to set a scrolling regin
 * that is LI or nrow rows high, whichever is larger.
 * This behavior seems to work right on systems
 * where you can set your terminal size.
 */
ttnowindow()
{
    if (CS) {
	putpad(tgoto(CS, (nrow > LI ? nrow : LI) - 1, 0));
	ttrow = HUGE;			/* Unknown.		*/
	ttcol = HUGE;
	tttop = HUGE;			/* No scroll region.	*/
	ttbot = HUGE;
    }
}

/*
 * Set the current writing color to the
 * specified color. Watch for color changes that are
 * not going to do anything (the color is already right)
 * and don't send anything to the display.
 * The rainbow version does this in putline.s on a
 * line by line basis, so don't bother sending
 * out the color shift.
 */
ttcolor(color) register int color; {
    if (color != tthue) {
	if (color == CTEXT) {		/* Normal video.	*/
	    putpad(SE);
	} else if (color == CMODE) {	/* Reverse video.	*/
	    putpad(SO);
	}
	tthue = color;			/* Save the color.	*/
    }
}

/*
 * This routine is called by the
 * "refresh the screen" command to try and resize
 * the display. The new size, which must be deadstopped
 * to not exceed the NROW and NCOL limits, it stored
 * back into "nrow" and "ncol". Display can always deal
 * with a screen NROW by NCOL. Look in "window.c" to
 * see how the caller deals with a change.
 */
ttresize() {
	setttysize();			/* found in "ttyio.c",	*/
					/* ask OS for tty size	*/
	if (nrow < 1)			/* Check limits.	*/
		nrow = 1;
	else if (nrow > NROW)
		nrow = NROW;
	if (ncol < 1)
		ncol = 1;
	else if (ncol > NCOL)
		ncol = NCOL;
}

#ifdef NO_RESIZE
static setttysize() {
	nrow = tgetnum("li");
	ncol = tgetnum("co");
}
#endif

static int cci;

static			/* fake char output for charcost() */
fakec(c) char c; {
#ifdef	lint
	c++;
#endif
	cci++;
}

/* calculate the cost of doing string s */
charcost (s) char *s; {
    cci = 0;

    tputs(s, nrow, fakec);
    return cci;
}

putpad(str) char *str; {
        tputs(str, 1, ttputc);
}
