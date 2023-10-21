/* terminal control for h19 */
/* after Trm.c */
/* K. Mitchum 6/84 */

#include "tm.h"
#include "jove.h"



/* define BIGH19 if using 25 lines */
#define BIGH19



#define NCOLS 80

/* local equates for h19 escape sequences */


#ifdef BIGH19

/* enable 25th line, restart cursor, no autolf, no autocr */

#define H19INIT "\033Y  \033E"
#define H19EXIT "\033Y  \033E"
#define LINES 25
#else

/* same as above, disable 25th line */

#define H19INIT "\033y1\033y5\033y8\033y9\033E"
#define H19EXIT "\033E"
#define LINES 24
#endif


#define REVOFF "\033q"		/* reverse video off */
#define REVON "\033p"		/* reverse video on */
#define INSLN "\033L"		/* insert line */
#define DELLN "\033M"		/* delete line */
#define INSCHON "\033@"		/* enter insert character mode */
#define INSCHOFF "\033O"	/* exit insert character mode */
#define CURFWD "\033C"		/* cursor forward */
#define CURDWN "\033B"		/* cursor down */
#define CURUP "\033A"		/* cursor up */
#define CURPOS "\033Y%c%c"	/* cursor positioning sequence */
#define CURHOME "\033H"		/* cursor home, non destructive */
#define KILLN "\033K"		/* kill line */
#define DELCHR "\033N"		/* delete char */
#define CLRSCN "\033E"		/* erase screen and home */
#define BS 010			/* backspace char */


/* padding constants */

#define KILLNPAD 0.05
#define INSLNPAD 0.25
#define DELLNPAD 0.25
#define INSCHPAD 0.05
#define CURPAD 0.01
#define INITPAD 0.05
#define DELPAD 0.05
#define CLRSCNPAD 1.00
#define REVPAD 0.10


static int curX, curY, Baud, DesHL, CurHL;

static float BaudFactor;


static
enum IDmode { m_insert = 1, m_overwrite = 0} DesiredMode;

static
enum Vmode { v_regular = 1, v_reverse = 0} Vimage;

static
INSmode (new)
enum IDmode new;
{
	DesiredMode = new;
}

static HLmode (new)
{
	DesHL = new;
}


static SetHL (OverRide)
{


	register Des = OverRide ? 0 : DesHL;

	if (Des == CurHL)
		return;
	if (Vimage == v_reverse)
		printf (Des ? REVOFF : REVON);
	else
		printf (Des ? REVON : REVOFF);
	CurHL = Des;
	pad(1,REVPAD);


}


static inslines(n)
{
    SetHL(1);


    while(n--) {
	printf(INSLN);
	pad(1,INSLNPAD);
    }
}

static dellines(n)
{
    SetHL(1);

    while(n--) {
	printf(DELLN);
	pad(1,DELLNPAD);
    }
}

static writechars(start, end)
register char *start, *end;
{
    int insmode =0;
	    
	
    SetHL(0);
    if (DesiredMode == m_insert) {
	printf(INSCHON);
	insmode++;
    }
    while ((start <= end) && ((curY != (LINES -1)) || (curX < (NCOLS)))) {
	pch(*start++);
	curX++;
    }
}

static blanks(n)
register n;
{
    int insmode;
    int len = n;

    insmode = 0;
    SetHL(0);
    curX += n;
    if (DesiredMode == m_insert) {
	printf(INSCHON);
	insmode++;
    }
    while(n--) pch(' ');
}

static
pad (n, f)
float f; {
return;
}

static topos(row,column)
register row, column;
{
    SetHL(1);
	row++;
	column++;
    if(curX != column || curY != row) {
	if(curX == column || curY == row) {
	    if(curX == column +1) pch(BS);
	    else if(curX == column -1) printf(CURFWD);
	    else if(curY == row +1) printf(CURUP);
	    else if(curY == row -1) printf(CURDWN);
	    else printf(CURPOS,row+31,column+31);
	}
	else {
	    if(row == 1 && column == 1) {
		printf(CURHOME);
	    }
	    else printf(CURPOS,(row+31)&0x7f,(column+31)&0x7f);
	}
	curX = column;
	curY = row;
/*	pad(1,CURPAD);*/
    }
}


static init (BaudRate)
{
	BaudFactor = BaudRate / 100.;
	Baud = BaudRate;
/*	MetaFlag++; */
}

static reset()
{
    printf(H19INIT);
    curX = curY = 1;
    CurHL = 0;
    DesiredMode = m_overwrite;
    pad(1,INITPAD);
    wipescreen();
}

static cleanup()
{
    SetHL(1);
    printf(H19EXIT);
    topos(24,1);
    sleep(1);
}

static wipeline()
{
    SetHL(1);
    printf(KILLN);
    pad(1,KILLNPAD);
}

static wipescreen()
{
    SetHL(1);

/*
#ifdef BIGH19
    topos(25,1);
    printf(KILLN);
    topos(1,1);
#endif
*/
    printf(CLRSCN);
    pad(1,CLRSCNPAD);
}
    
static delchars(n)
{
    int len = n;
	
    SetHL(1);
    while(n--) printf(DELCHR);
    pad(len,DELPAD);
}


Trm()
{
    Vimage = v_regular;

    tt.t_length = LINES;
    tt.t_INSmode = INSmode;
	tt.t_HLmode = HLmode;
	tt.t_inslines = inslines;
	tt.t_dellines = dellines;
	tt.t_blanks = blanks;
	tt.t_init = init;
	tt.t_cleanup = cleanup;
	tt.t_wipeline = wipeline;
	tt.t_wipescreen = wipescreen;
	tt.t_topos = topos;
	tt.t_reset = reset;
	tt.t_delchars = delchars;
	tt.t_writechars = writechars;
	tt.t_window = NULL;
	reset();
	return(tt.t_length);
}



	
    
pch(c)
{
    Putc(c,&termout);
}



putp(p)
char p;
{
	writechars(&p,&p);
	return;
}
