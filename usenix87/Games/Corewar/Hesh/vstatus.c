/*
** vstatus.c	- manage the status window
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: vstatus.c,v 7.0 85/12/28 14:36:44 ccohesh Dist $";
**
*/

#include <curses.h>
#include "cw.h"


static	WINDOW	*statwin;

/*
** statinit()	- initialize the status window
*/
int	statinit (jobs)
reg	process	*jobs;
{
	reg	int	i;

	if (!(statwin = subwin(stdscr, SLINES, SCOLS, SYBEGIN, SXBEGIN)))
	{
		fprintf(stderr, "%s: status screen initialization error\n",
			argv0);
		return(TRUE);
	}
	wclear(statwin);
	scrollok(statwin, FALSE);
	for (i=0; i<jobcnt && i < SLINES; i++)
		vstajob(jobs++);
	if (ON(PASSNUM))
		vstatus();
	return(FALSE);
}

/*
** statfini()	- de-initialize the status window
*/
void	statfini ()
{
	wclear(statwin);
	delwin(statwin);
}

/*
** vstatus()	- print the current cycle number and jobs left 
**		  in the status window
*/
void	vstatus ()
{
	wmove(statwin, SLINES-1, 0);
	wprintw(statwin, "Cycle: %04d  Jobs: %2d", cycle, jobsleft);
	wrefresh(statwin);
}

/*
** vstajob()	- update the entry for the given job in the status window
*/
void	vstajob (j)
reg	process	*j;
{
	wmove(statwin, j->pid, 0);
	wprintw(statwin,"%c. \"%.10s\" %4d %s",
		'A' + j->pid, j->pname, j->pc, diedstr(j->pdied));
	wrefresh(statwin);
}
