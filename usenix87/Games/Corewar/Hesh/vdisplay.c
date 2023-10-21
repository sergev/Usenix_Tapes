/*
** vdisplay.c	- routines dealing with the visual display of the
**		  Corewar game
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: vdisplay.c,v 7.0 85/12/28 14:36:39 ccohesh Dist $";
**
*/

#include <curses.h>
#include <ctype.h>
#include "cw.h"


/*
** Strategy:
**
** 1.	Fill the screen with a rectangle of characters (MEMPTY).
**	Let each character represent 10 memory locations.
**	Since MAX_MEM is 10,000 (and I am afraid this value is
**	going to be rather hard coded into the display routines)
**	this will have to be a 50 column by 20 row box.
** 2.	Place an uppercase letter in the location corresponding to
**	the value of the job's PC.  If more than one job occupies the
**	same location, then place a digit representing the number of
**	jobs.
** 3.	Place a lowercase letter at the square corresponding to the
**	memory location that the job is modifying.  If more than one
**	job is modifying the same memory section, place MSHARE there.
** 4.	If a job is modifying a memsect that another job is modifying,
**	place a MHIT.
**
**	Therefore, we need routines to do the following:
**
**		initialize window
**			fill with MEMPTY's
**		update screen wrt PC values and delta mem locs
*/

#define	BOXS(p)		(p/NUMMEM)
#define	BOXX(p)		(BOXS(p) % MCOLS)
#define	BOXY(p)		(BOXS(p) / MCOLS)

static	WINDOW	*memwin;

extern	void	clearbox(), membox();

/*
** vupdate()	- update the memory display
*/
int	vupdate (ps)
reg	process	*ps;
{
	reg	process	*p;

	for (p = ps; (p - ps) < jobcnt; p++)
	{	/* update plstmem's	*/
		auto	int	addr = (p->plstmem - &(memory[0]));
		auto	int	y = BOXY(addr),		/* hides earlier def. */
				x = BOXX(addr);		/* hides earlier def. */
		auto	char	pch = (char) p->pid + 'a';
		reg	char	ch;

		if (!p->plstmem || p->pdied)
			continue;
		if ((addr/PAGESIZE) == (p->pc/PAGESIZE))
			continue;	/* writing to same mem page */
		if ((ch = mvwinch(memwin, y, x)) == MEMPTY)
			waddch(memwin, pch);
		else if (islower(ch))
			waddch(memwin, MSHARE);
		else
			msg("Unknown character in membox (%s) @ (%d,%d)",
				punctrl(ch), y, x);
	}
	for (p = ps; (p - ps) < jobcnt; p++)
	{	/* upate pc's	*/
		auto	int	y = BOXY(p->pc),	/* hides earlier def. */
				x = BOXX(p->pc);	/* hides earlier def. */
		auto	char	pch = (char) p->pid + 'A';
		reg	char	ch = mvwinch(memwin, y, x);

		if (p->pdied || ch == MFULL || ch == MHIT)
			continue;
		if (ch == MEMPTY)
			waddch(memwin, pch);
		else if (isupper(ch))
			waddch(memwin, '2');
		else if (isdigit(ch))
			waddch(memwin, (ch<'9') ? (ch + (char) 1) : MFULL);
		else if (ch == MSHARE)
			waddch(memwin, MHIT);
		else if (islower(ch))
		{
			if (ch == tolower(pch))
				waddch(memwin, pch);
			else
				waddch(memwin, MHIT);
		}
		else
			msg("unknown character in membox (%s) @ (%d,%d)",
				punctrl(ch), y, x);
	}
	wrefresh(memwin);
	for (p = ps; (p - ps) < jobcnt; p++)
	{	/* upate pc's	*/
		auto	int	addr = (p->plstmem - &(memory[0]));

		if (p->pdied)
			continue;
		mvwaddch(memwin, BOXY(p->pc), BOXX(p->pc), MEMPTY);
		if (!p->plstmem)
			continue;
		mvwaddch(memwin, BOXY(addr), BOXX(addr), MEMPTY);
	}
}

/*
** vinit()	- perform curses initializations
**		- initialize memory window
**		- return non-zero on error
*/
int	vinit ()
{
	if (initscr() == ERR)
	{
		fprintf(stderr, "%s: screen initialization error\n", argv0);
		perror(argv0);
		return(TRUE);
	}
	if (LINES < MIN_LINES || COLS < MIN_COLS)
	{
		fprintf(stderr, "%s: screen must be at least %d by %d\n",
			argv0, MIN_LINES, MIN_COLS);
		return(TRUE);
	}
	if (!(memwin = subwin(stdscr, MLINES, MCOLS, MYBEGIN, MXBEGIN)))
	{
		fprintf(stderr, "%s: can't create memory-window\n", argv0);
		return(TRUE);
	}
	if (msginit())
		return(TRUE);
	membox();
	clearbox();
	refresh();
	crmode();
	noecho();
	return(FALSE);
}

/*
** vfini()	- perform screen shutdown actions
*/
void	vfini ()
{
	msgfini();
	clear();
	refresh();
	delwin(memwin);
	endwin();
}

/*
** clearbox()	- fill the box with MEMPTY's
*/
static	void	clearbox ()
{
	reg	int	y, x;

	for (y=0; y<MLINES; y++)
		for (x=0; x<MCOLS; x++)
			mvwaddch(memwin, y, x, MEMPTY);
}

/*
** membox()	- draw a box around the memory window
*/
static	void	membox ()
{
	reg	int	y, x;

	for (x = MXBEGIN-1, y = MYBEGIN-1; x < (MXBEGIN+MCOLS+1); x++)
		mvaddch(y, x, HORTBORDER), mvaddch(y+MLINES+1, x, HORTBORDER);
	for (y = MYBEGIN, x = MXBEGIN-1; y < (MYBEGIN+MLINES); y++)
		mvaddch(y, x, VERTBORDER), mvaddch(y, x+MCOLS+1, VERTBORDER);
}
