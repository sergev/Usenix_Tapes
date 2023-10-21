/*
** misc.c -	miscellaneous functions
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/
#include <signal.h>
#include "pm.h"


/*
** chg_lvl()	- change the level by the desired ammount
*/
void	chg_lvl (delta)
reg	int	delta;
{
	level += delta;
	if (level < 0)
		level = 0;
	ch = ' ';
	oldch = ' ';
	newch = ' ';
	pm_run = TRUE;
	sleep(1);
	new_screen();
}

/*
** scrcpy()	- copy score structures
**		- there are more efficient ways todo this, but lint doesn't
**		  like any of the ones i came up with
*/
void	scrcpy (to, from)
reg	score	*to, *from;
{
	to->sc_uid = from->sc_uid;
	to->sc_score = from->sc_score;
	to->sc_level = from->sc_level;
	to->sc_flags = from->sc_flags;
	to->sc_mons = from->sc_mons;
	strncpy(to->sc_name, from->sc_name, sizeof(from->sc_name));
	return;
}

/*
** dir_int()	- changes a char direction indicator to a int
*/
int	dir_int (dir)
reg	char	dir;
{
	switch (dir)
	{
		case MUP:
			return(0);
		case MDOWN:
			return(1);
		case MLEFT:
			return(2);
		case MRIGHT:
			return(3);
		default:
			return(-1);
	}
	/*NOTREACHED*/
}

static	char	*dirs[] =
{
	"\n\n\t\tWelcome to the game of pm\n",
	"Just a few words of information and caution to",
	"beginning pm players.  This game is very expensive!!!",
	"It is so expensive that your usercode may not last",
	"more than a few medium length games.  The arcade",
	"equivalent of this costs a quarter per game, no",
	"matter how long it lasts.  With pm it is the other way",
	"around, the longer you play the more expensive it gets!",
	"If you are unfamilar with the commands for movement, I",
	"suggest that you try getting the hang of them by playing",
	"other games (such as rogue, snake, or tank) that use",
	"similar commands.  It could get very expensive getting",
	"the hang of moving around by learning on pm.",
	"The higher your baud rate, the better the game performs.",
	"9600 is a pretty good speed to run at.",
	"For a summary of valid commands, type in a '?'.",
	"\nHappy packing!!!",
	"\n[Hit return to continue] ",
	0
};

/*
** directions()	- print out any opening messages to beginners
*/
void	directions ()
{
	reg	char	**s = dirs;

	while (*s)
		printf("%s\n", *s++);
	trash(getchar());
}

/*
** get_pass()	- read in the password
**		- only read in 8 characters!
*/
char	*get_pass ()
{
	static	char	buf[9];
	reg	int	i;

	nocrmode();
	for (i = 0; i < 9; i++)
		buf[i] = '\0';
	for (i = 0; i < 8; i++)
	{
		reg	char	in;

		if ((in = getchar()) == '\n')
			break;
		buf[i] = in;
	}
	crmode();
	return(buf);
}

/*
** int_dir()	- changes an int to a char direction indicator
**		- the % insures that it is in range
*/
char	int_dir (dir)
reg	int	dir;
{
	static	char	_dirs[] =
	{
		MUP, MDOWN, MLEFT, MRIGHT, 0
	};

	return(_dirs[dir % MAX_DIRS]);
}

/*
** lturn()	- return the direction to the left, relative to
**		  the given direction
*/
char	lturn (dir)
reg	char	dir;
{
	switch (dir)
	{
		case MUP:
			return(MLEFT);
		case MDOWN:
			return(MRIGHT);
		case MLEFT:
			return(MDOWN);
		case MRIGHT:
			return(MUP);
		default:
			return(MSTOP);
	}
	/*NOTREACHED*/
}

/*
** mons_str()	- return the (full) name of the given monster
*/
char	*mons_str (mon)
reg	char	mon;
{
	switch (mon)
	{	case HARPO:
			return("Harpo");
		case GROUCHO:
			return("Groucho");
		case CHICO:
			return("Chico");
		case ZEPPO:
			return("Zeppo");
		default:
			return("Anonymous");
	}
	/*NOTREACHED*/
}

/*
** opposite()	- return the direction opposite to that specified
*/
char	opposite (dir)
reg	char	dir;
{
	switch (dir)
	{
		case MUP:
			return(MDOWN);
		case MDOWN:
			return(MUP);
		case MLEFT:
			return(MRIGHT);
		case MRIGHT:
			return(MLEFT);
		default:
			return(MSTOP);
	}
}

/*
** rturn()	- return the direction to the right, relative to
**		  the given direction
*/
char	rturn (dir)
reg	char	dir;
{
	switch (dir)
	{
		case MUP:
			return(MRIGHT);
		case MDOWN:
			return(MLEFT);
		case MLEFT:
			return(MUP);
		case MRIGHT:
			return(MDOWN);
		default:
			return(MSTOP);
	}
}

/*
** quit_it()	- stop the game
*/
void	quit_it ()
{
	echo();
	nocrmode();
	endwin();
	exit(0);
}

/*
** shell()	- set thier uid to thier realuid and give them a shell
*/
void	shell ()
{
	reg	char	*sh;
	reg	int	pid;
	extern	char	*getenv();

	echo();
	nocrmode();
	_puts(CL);		/* clear screen		*/
	move(LINES - 1, 0);	/* and go to the bottom */
	draw();
	if ((sh = getenv("SHELL")) == NULL) /* check for a preferred shell */
		sh = DEFAULT_SH;
	if ((pid = fork()) == -1)
	{
		fprintf(stderr, "fork failed, by by!\n");
		quit_it();
	}
	if (!pid) /* if child */
	{
		if (setuid(uid) == -1) /* incase we are running setuid */
			exit((fprintf(stderr, "Can't setuid(%d)\n", uid), 1));
		if (setgid(getgid()) == -1) /* incase we are running setgid */
			exit((fprintf(stderr,"Can't setgid(%d)\n",getgid()),1));
#ifndef	LINT
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
#endif
		execle(sh, "shell", "-i", 0, environ);
		perror("pm");
		exit(1);
	}
#ifndef	LINT
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#endif
	wait(0);
	trap(0);		/* reset signals		*/
	noecho();
	crmode();
	printf("[Press return to continue]");
	trash(getchar());
	redraw();
}

/*
** toletter()	- translate the numeric move (key pad) to a letter move
**		- this is to facilitate the use of keypads if the
**		  terminal is so equipped
**		- return NULL for invalid
*/
char	toletter (in)
reg	char	in;
{
	static	char	mvs[] =
	{
		NULL, NULL, MDOWN, NULL, MLEFT, MSTOP, MRIGHT, NULL, MUP, NULL
	};

	return(mvs[(in - '0') % 9]);
}

/*
** trap()	- catches signals
**		- flag is zero for the initial call, non-zero
**		  when an interrupt is recieved
*/
void	trap (flag)
reg	int	flag;
{
	if (!flag)
	{
#ifndef	LINT
		signal(SIGINT, trap);
		signal(SIGHUP, trap);
#endif
		return;
	}
#ifndef	LINT
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
#endif
	clear();
	draw();
	quit_it();
}

/*
** tunn_look()	- return what is at the given location in the tunnel
*/
char	tunn_look (pos)
reg	coord	*pos;
{
	reg	int	i;

	for (i = 0; i < MAX_MONS; i++)
		if (AT(pos, &ghosts[i].mo_pos))
			return(ghosts[i].mo_name);
	if (AT(pos, &pm_pos))
		return(PM);
	move(pos->y, pos->x);
	return(INCH());
}

#ifdef	BAD_OVERLAY
/*
** overlay() -	a bug exist[s,ed] in the curses function overlay() on the
**		4.2 machine on which this program was updated.  this file
**		is included in case this bug is not just local to this
**		system.
**
*/

/*
** In the outer `for' loop the "<" should have been a "<=".
*/
# define	min(a,b)	(a < b ? a : b)
# define	max(a,b)	(a > b ? a : b)

/*
 *	This routine writes win1 on win2 non-destructively.
 *
 * 11/5/82 (Berkeley) @(#)overlay.c	1.4
 */
overlay(win1, win2)
reg WINDOW	*win1, *win2; {

	reg char	*sp, *end;
	reg int		x, y, endy, endx, starty, startx;

# ifdef DEBUG
	fprintf(outf, "OVERLAY(%0.2o, %0.2o);\n", win1, win2);
# endif
	starty = max(win1->_begy, win2->_begy) - win1->_begy;
	startx = max(win1->_begx, win2->_begx) - win1->_begx;
	endy = min(win1->_maxy, win2->_maxy) - win1->_begy - 1;
	endx = min(win1->_maxx, win2->_maxx) - win1->_begx - 1;
/*
** this is what was erroneously here:
**
**	for (y = starty; y < endy; y++) {
**
** below you will find the correct code (s/</<=/)
*/
	for (y = starty; y <= endy; y++) {
		end = &win1->_y[y][endx];
		x = startx + win1->_begx;
		for (sp = &win1->_y[y][startx]; sp <= end; sp++) {
			if (!isspace(*sp))
				mvwaddch(win2, y + win1->_begy, x, *sp);
			x++;
		}
	}
}
#endif
