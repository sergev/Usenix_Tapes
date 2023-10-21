#include "hd.h"
#include "mydir.h"

char *BC, *UP, *CM, *CL;
/* dyt */
char *CE;
char *SO, *SE;
short PC;
int CO, LI;
char *CS, *CB;
char *BE, *BO;
char *SR, *CD;
char *TI, *TE;
int AM, XN;

char tspace [512], *atspace;
char *tgetstr ();
int putch ();

int ewindow = 0;		/* Separate exec window */
int xoff = 0;			/* Column offset */

curs_init () 
{
    char bp [1024];
    char * tgoto ();
    static char winbuf[10];

    tgetent (bp, TERM);
    atspace = tspace;
    PC = *tgetstr ("pc", &atspace);
    CM = tgetstr ("cm", &atspace);
    CL = tgetstr ("cl", &atspace);
    UP = tgetstr ("up", &atspace);
    BC = tgetstr ("bc", &atspace);
    CE = tgetstr ("ce", &atspace);
    CO = tgetnum ("co");
    LI = tgetnum ("li");
    CS = tgetstr ("cs", &atspace);
    /* A new one, clear to beginning of display */
    CB = tgetstr ("cb", &atspace);
    /* Blink and blink end */
    BO = tgetstr ("bo", &atspace);
    BE = tgetstr ("be", &atspace);
    if (BC == CNULL || tgetflag("bs"))
	BC = "\b";
/*
 * Enables use of reverse video/standout
 */
    SO = tgetstr ("so", &atspace);
    SE = tgetstr ("se", &atspace);
/*
 * Scroll reverse
 */
    SR = tgetstr ("sr", &atspace);
    CD = tgetstr ("cd", &atspace);
    AM = tgetflag ("am");
    XN = tgetflag ("xn");
    TI = tgetstr ("ti", &atspace);
    TE = tgetstr ("te", &atspace);

    if (SO == CNULL)
	SO = "";
    if (SE == CNULL)
	SE = "";
    if (BO == CNULL)
	BO = "";
    if (BE == CNULL)
	BE = "";
    if (CM == CNULL) 
	return 1;
    if (CO < 80) {
	fprintf(stderr, "\07Too narrow screen\r\n");
	CO = 80;
    }
    else if (CO > 80)
	xoff = CO - 80;
    setcolumn();
    if (LI < 8)
	return 1;
    sprintf(winbuf, "%8d", LI);
    WINDOW = winbuf;
    setwindow();
    return 0;
}

setwindow()
{
	register int row;
	register int owin1, owin2;

	owin1 = window;
	row = atoi(WINDOW);
	window = (row > (maxnfpp+VSHTOP+VSHBOT))
		? (maxnfpp+VSHTOP+VSHBOT) : row;
	if (window < 8) {
		putmsg("\07Screen too small");
		window = 8;
	}
	nfpp = window - (VSHTOP+VSHBOT);
	ewindow = (LI-window > PAGEMIN);
	if (owin1 != window) {
		owin2 = window;
		window = owin1;
		ewin();
		window = owin2;
		vwin();
		erase();
	}
	else
		vwin();
}

/* Multi-column listing */
setcolumn()
{
	register int i;

	i = (CO-VSHLEFT)/19;			/* Max number of columns */
	if ((column = atoi(COLUMN)) < 1)
		column = 1;
	if (column > i)
		column = i;
	colfield = (CO-VSHLEFT)/column;
	pageoff = 0;
}

static int ewinf = 0;
ewin()
{
	if (!ewindow)
		return 0;
	bufout();
	if (CS) {
		tputs(tgoto(CS, LI-1, window-VSHBOT), 0, putch);
		ewinf = 1;
	}
	atxy(1, LI);
	if (TE)
		tputs(TE, 0, putch);
	unbufout();
	return ewinf;
}

vwin()
{
	if (!ewinf)
		return;
	bufout();
	if (TI)
		tputs(TI, 0, putch);
	if (CS)
		tputs(tgoto(CS, window-1, 0), 0, putch);
	atxy(1, window-1);
	clearline();
	atxy(1, 1);
	unbufout();
	ewinf = 0;
}

hilite(a0, a1, a2)
char *a0, *a1, *a2;
{
	if ((int)a0 != 0) {
		tputs(SO, 0, putch);
		if ((int)a0 == 1)
			return;
		printf(a0, a1, a2);
	}
	tputs(SE, 0, putch);
}
