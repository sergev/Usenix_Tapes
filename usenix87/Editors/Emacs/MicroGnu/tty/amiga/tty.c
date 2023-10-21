/*
 * Name:	MicroEMACS
 *		Amiga console device virtual terminal display
 * Version:	GNU v30
 * Last Edit:	23-Aug-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 * Created:	19-Apr-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 *
 * Drives the Amiga console device display.  The code
 * is a combination of the Heath and ANSI terminal drivers,
 * plus some hacks to manage the console device colors.
 */

#include	"def.h"

#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/

extern	int	ttrow;
extern	int	ttcol;
extern	int	tttop;
extern	int	ttbot;
extern	int	tthue;

int	tceeol	=	3;		/* Costs, ANSI display.		*/
int	tcinsl	= 	17;
int	tcdell	=	16;


#ifdef	CHANGE_COLOR
short	mode_rendition = MODE_RENDITION,	/* set standard colors */
	text_rendition = TEXT_RENDITION,
	text_fg = TEXT_FG + 30,
	text_bg = TEXT_BG + 40,
	mode_fg = MODE_FG + 30,
	mode_bg = MODE_BG + 40;
#else				/* colors are hard-coded		*/
#define mode_rendition MODE_RENDITION
#define	text_rendition TEXT_RENDITION
#define text_fg (TEXT_FG + 30)
#define text_bg (TEXT_BG + 40)
#define mode_fg (MODE_FG + 30)
#define mode_bg (MODE_BG + 40)
#endif

/*
 * Initialize the terminal when the editor
 * gets started up.  A no-op for the Amiga
 */
ttinit()
{
}

/*
 * Clean up the terminal, in anticipation of
 * a return to the command interpreter. This
 * is a no-op on the Amiga.
 */
tttidy()
{
}

/*
 * Move the cursor to the specified
 * origin 0 row and column position. Try to
 * optimize out extra moves; redisplay may
 * have left the cursor in the right
 * location last time!
 */
ttmove(row, col)
{
	if (ttrow!=row || ttcol!=col) {
		ttputc(ESC);
		ttputc('[');
		asciiparm(row+1);
		ttputc(';');
		asciiparm(col+1);
		ttputc('H');
		ttrow = row;
		ttcol = col;
	}
}

/*
 * Erase to end of line.
 */
tteeol()
{
	ttputc(ESC);
	ttputc('[');
	ttputc('K');
}

/*
 * Erase to end of page.
 */
tteeop()
{
	ttputc(ESC);	/* reinforce current color values */
	ttputc('[');
	asciiparm((tthue == CTEXT) ? text_rendition : mode_rendition);
	ttputc(';');
	asciiparm(text_fg);
	ttputc(';');
	asciiparm(text_bg);
	ttputc('m');

	ttputc(ESC);	/* clear to end of display */
	ttputc('[');
	ttputc('J');
}

/*
 * Make a noise.
 */
ttbeep()
{
	ttputc(BEL);
	ttflush();
}

/*
 * Convert a number to decimal
 * ascii, and write it out. Used to
 * deal with numeric arguments.
 */
asciiparm(n)
register int	n;
{
	if (n > 9)
		asciiparm(n/10);
	ttputc((n%10) + '0');
}

/*
 * Insert a block of blank lines onto the
 * screen, using a scrolling region that starts at row
 * "row" and extends down to row "bot".  Deal with the one
 * line case, which is a little bit special, with special
 * case code.
 *
 * Since we don't really have a scrolling region,
 * delete the block of lines that would have been deleted if
 * we'd had one, then insert blank lines to move the rest
 * of the screen back to where it belongs.  This idea from
 * the Heath driver.
 */
VOID ttinsl(row, bot, nchunk)
{
	if (row == bot) {			/* Funny case.		*/
		if (nchunk != 1)
			panic("ttinsl: nchunk != 1");
		ttmove(row, 0);
		tteeol();
		return;
	} 
	ttmove(1+bot-nchunk, 0);
	if (nchunk > 0) {		/* Delete a chunk of lines	*/
		ttputc(ESC);		/* nchunk in size.  Rest of	*/
		ttputc('[');		/* screen moves up.		*/
		asciiparm(nchunk);
		ttputc('M');
	}
	ttmove(row, 0);
	if (nchunk > 0) {		/* Insert a chunk nchunk in size*/
		ttputc(ESC);		/* before current line,	sliding	*/
		ttputc('[');		/* rest of screen down.		*/
		asciiparm(nchunk);
		ttputc('L');
	}
	ttrow = row;			/* End up on current line	*/
	ttcol = 0;
}

/*
 * Delete a block of lines, with the uppermost
 * line at row "row", in a screen slice that extends to
 * row "bot". The "nchunk" is the number of lines that have
 * to be deleted.  This is done by deleting nchunk lines at the
 * appropriate spot, then inserting nchunk lines to make up for
 * the empty space at the bottom of the virtual scrolling region.
 */
VOID ttdell(row, bot, nchunk)
{
	if (row == bot) {		/* One line special case	*/
		ttmove(row, 0);
		tteeol();
		return;
	}
	if (nchunk > 0) {
		ttmove(row, 0);
		ttputc(ESC);
		ttputc('[');
		asciiparm(nchunk);
		ttputc('M');
	}
	ttmove(1+bot-nchunk,0);
	if (nchunk > 0) {
		ttputc(ESC);		/* For all lines in chunk	*/
		ttputc('[');		/* INS line before bottom	*/
		asciiparm(nchunk);	/* Bottom of window (and rest 	*/
		ttputc('L');		/* of screen) moves down */
	}
	ttrow = HUGE;
	ttcol = HUGE;
	ttmove(bot-nchunk,0);
}

/*
 * No-op.
 */
ttwindow(top,bot)
{
}

/*
 * No-op.
 */
ttnowindow()
{
}

#ifdef	CHANGE_COLOR
/*
 * Set the rendition of the mode line by
 * selecting colors from the following:
 *	0 -- plain text
 *	1 -- bold-face
 *	3 -- italic
 *	4 -- underscore
 *	7 -- inverse video
 * Certain of these selections may be less than
 * appealing :-)
 */

ttmode(f, n, k)
{
	register int	s;
	char		buf[2];

	if (f == FALSE) {
		if ((s = ereply("Set mode line rendition (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	mode_rendition = n;		/* store the color	*/
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set the rendition of the text area.
 * Most of these selections will be
 * less than appealing :-]
 */

tttext(f, n, k)
{
	register int	s;
	char		buf[2];

	if (f == FALSE) {
		if ((s = ereply("Set text rendition (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	text_rendition = n;		/* store the color	*/
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set foreground color for entire window
 * to a value between 30 and 37, which
 * corresponds to the arguments 0-7.
 * This requires a total refresh, which
 * sets up the screen.
 */

textforeground(f, n, k)
{
	register int	s;
	char		buf[2];

	if (f == FALSE) {
		if ((s = ereply("Text foreground color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	text_fg = n + 30;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set background color for entire window
 * to a value between 40 and 47 inclusive.
 */

textbackground(f, n, k)
{
	register int	s;
	char		buf[2];

	if (f == FALSE) {
		if ((s = ereply("Text background color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	text_bg = n + 40;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set foreground color for entire the mode line
 */

modeforeground(f, n, k)
{
	register int	s;
	char		buf[2];

	if (f == FALSE) {
		if ((s = ereply("Mode line foreground color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	mode_fg = n + 30;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set background color for the mode line
 */

modebackground(f, n, k)
{
	register int	s;
	char		buf[2];

	if (f == FALSE) {
		if ((s = ereply("Mode line background color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	mode_bg = n + 40;
	sgarbf = TRUE;
	return (TRUE);
}
#endif

/*
 * Set the current writing color to the
 * specified color. Watch for color changes that are
 * not going to do anything (the color is already right)
 * and don't send anything to the display.
 */

ttcolor(color)
register int	color;
{
	if (color != tthue) {
		if (color == CTEXT) {		/* Normal video.	*/
			ttputc(ESC);		/* Reset to 0		*/
			ttputc('[');
			ttputc('m');
			ttputc(ESC);		/* Set text style	*/
			ttputc('[');
			asciiparm(text_rendition);
			ttputc(';');
			asciiparm(text_fg);
			ttputc(';');
			asciiparm(text_bg);
			ttputc('m');
		} else if (color == CMODE) {	/* Standout mode	*/
			ttputc(ESC);		/* Reset to 0		*/
			ttputc('[');
			ttputc('m');
			ttputc(ESC);		/* Set standout mode	*/
			ttputc('[');
			asciiparm(mode_rendition);
			ttputc(';');
			asciiparm(mode_fg);	/* Use mode line colors	*/
			ttputc(';');
			asciiparm(mode_bg);
			ttputc('m');
		}
		tthue = color;			/* Save the color.	*/
	}
}

/*
 * This routine is called by the
 * "refresh the screen" command to try and resize
 * the display. The new size, which must be deadstopped
 * to not exceed the NROW and NCOL limits, is stored
 * back into "nrow" and "ncol". Display can always deal
 * with a screen NROW by NCOL. Look in "window.c" to
 * see how the caller deals with a change.
 * On the Amiga, we make the Intuition terminal driver
 * do all the work.
 */

ttresize()
{
 	setttysize();
}
