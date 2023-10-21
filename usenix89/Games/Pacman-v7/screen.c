/*
** screen.c -	code dealing with display (most of what is written to the
**		screen during the course of a game is done here)
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/

#include "pm.h"

/*
** p_draw_screen()- draw the board
**		- this ordering is kind of important.  the dots must
**		  be printed first and then the barriers in standout
**		  so that if the terminal uses "inverse video" for
**		  the so entry in /etc/termcap (or wherever) the
**		  paths through the maze are not in inverse video.
**		- also, whenever the screen is cleared and dots()
**		  called, barriers() must be called again, or else
**		  dots() will overwrite the barriers.
**		- the SLOWER() is so that the monsters don't move
**		  until the screen is totally redrawn
*/
void	draw_screen ()
{
	alarm(0);	/* make sure the alarm is off */
	msg("");
	randomize(SEED);
	clear();
	p_dots();	/* the dots must be printed first */
	standout();
	p_barriers();
	standend();
	p_scores();
	p_energizers();
	p_pm();
	p_pms();
	p_monsters();
	p_fruits();
	draw();
	sleep(1);
}

/*
** new_screen()	- called when a screen has been cleaned out
**		- the SLOWER() is so that the monsters don't move
**		  until the screen is totally redrawn
*/
void	new_screen ()
{
	reg	int	i;

	alarm(0);	/* make sure the alarm is off */
	msg("");
	randomize(SEED);
	flush();
	mvaddch(pm_pos.y, pm_pos.x, EMPTY);
	aggressive();
	for (i = 0; i < 4; i++)
		m_erase(ghosts[i]);
	draw();
	p_scores();
	p_dots();
	standout();
	p_barriers();
	standend();
	p_energizers();
	p_pm();
	p_monsters();
	p_fruits();
	draw();
	sleep(2);
}

/*
** old_screen()- called when pm is eaten
*/
void	old_screen ()
{
	reg	int	i;

	alarm(0);	/* make sure the alarm is off */
	msg("");
	flush();
	mvaddch(pm_pos.y, pm_pos.x, EMPTY);
	aggressive();
	for (i = 0; i < MAX_MONS; i++)
		m_erase(ghosts[i]);
	draw();
	p_scores();
	p_pms();
	p_pm();
	p_monsters();
	draw();
	sleep(1);
}

/*
** redraw()	- redraw the screen
*/
void	redraw ()
{
	alarm(0);	/* make sure the alarm is off */
	clearok(stdscr, TRUE);
	msg("");
	draw();
	sleep(1);
}

/*
** p_scores()	- print the score and high score if changed
**		- check to see if they get another pm
**		- must remember to print both scores when the
**		  game is first started
*/
void	p_scores ()
{
	static	long	_score = -1L;

	if (thescore == _score)
		return;	/* the posted score is accurate */
	if ((thescore > BONUS) && pm_bonus)
	{
		pm_bonus = FALSE;
		pms_left++;
		p_pms();
	}
	move(0, 23);
	printw("%06ld", thescore);
	if (thescore > hi_score || _score == -1L)
	{
		move(0, 47);
		printw("%06ld",
			(thescore>hi_score) ? (hi_score=thescore) : hi_score);
	}
	_score = thescore;
}

/*
** p_fruits()	- place the fruit and sets its value
*/
void	p_fruits ()
{
	reg	int	lvl = (level >= MAX_LEVEL ? MAX_LEVEL-1 : level);

	fr_ch = fruit[lvl];
	fr_val = fruit_val[lvl];
	move(13, 26);
	addch(fr_ch);
}

/*
** add_fruit()	- add eaten fruit to fruit list
**		- have to shift things over some
*/
void	add_fruit (fr)
reg	char	fr;
{
	reg	int	i;

	for (i = 6; i > 0; i--)
		fruit_eaten[i * 2] = fruit_eaten[(i - 1) * 2];
	fruit_eaten[0] = fr;
	move(0, 55);
	printw("%s", fruit_eaten);
}

/*
** p_pm()	- place the pm in its starting position
*/
void	p_pm ()
{
	pm_tunn = FALSE;
	pm_pos.x = 26;
	pm_pos.y = 17;
	move(pm_pos.y, pm_pos.x);
	addch(PM);
}

/*
** p_pms()	- place the spare pm's
*/
void	p_pms ()
{
	reg	int	i;

	for (i = 1; i < MAX_PMS; i++)
	{
		move(0, (2 * (i - 1)));	
		if (i >= pms_left)
			addch(EMPTY);
		else
			addch(PM);
	}
}

/*
** p_energizers() - put in the energizers
*/
void	p_energizers ()
{
	move(4, 51);
	addch(ENERGY);
	move(4, 1);
	addch(ENERGY);
	move(17, 1);
	addch(ENERGY);
	move(17, 51);
	addch(ENERGY);
	e_left = MAX_ENERGY;
}

static	char	*_board[] =
{
	"#####################################################\n",
	"#                       #####                       #\n",
	"# ######### ########### ##### ########### ######### #\n",
	"# ######### ########### ##### ########### ######### #\n",
	"#                                                   #\n",
	"# ######### ##### ################# ##### ######### #\n",
	"#           #####       #####       #####           #\n",
	"########### ########### ##### ########### ###########\n",
	"#         # #####                   ##### #         #\n",
	"########### ##### ######## ######## ##### ###########\n",
	"                  #               #                  \n",
	"########### ##### ################# ##### ###########\n",
	"#         # #####                   ##### #         #\n",
	"########### ##### ################# ##### ###########\n",
	"#                       #####                       #\n",
	"# ######### ########### ##### ########### ######### #\n",
	"#     #####                               #####     #\n",
	"##### ##### ##### ################# ##### ##### #####\n",
	"#           #####       #####       #####           #\n",
	"# ##################### ##### ##################### #\n",
	"# ##################### ##### ##################### #\n",
	"#                                                   #\n",
	"#####################################################\n",
	0
};

/*
** p_barriers()	- fills in the board
*/
void	p_barriers ()
{
	static	int	once = TRUE;
	static	WINDOW	*tmp;

	if (once)
	{
		reg	char	**str = _board;

		if ((tmp = newwin(0, 0, 0, 0)) == ERR)
		{
			move(0, 0);
			printw("barriers(): newwin() error");
			draw();
			quit_it();
		}
		wmove(tmp, TOP, 0);
		while (*str)
			waddstr(tmp, *str++);
		once = FALSE;
	}
	overlay(tmp, stdscr);
}

static	char	*_dots[] =
{
	"                                                     \n",
	" . . . . . . . . . . . .     . . . . . . . . . . . . \n",
	" .         .           .     .           .         . \n",
	" .         .           .     .           .         . \n",
	" . . . . . . . . . . . . . . . . . . . . . . . . . . \n",
	" .         .     .                 .     .         . \n",
	" . . . . . .     . . . .     . . . .     . . . . . . \n",
	"           .                             .           \n",
	"           .                             .           \n",
	"           .              =              .           \n",
	"-          .                             .          -\n",
	"           .                             .           \n",
	"           .                             .           \n",
	"           .                             .           \n",
	" . . . . . . . . . . . .     . . . . . . . . . . . . \n",
	" .         .           .     .           .         . \n",
	" . . .     . . . . . . .     . . . . . . .     . . . \n",
	"     .     .     .                 .     .     .     \n",
	" . . . . . .     . . . .     . . . .     . . . . . . \n",
	" .                     .     .                     . \n",
	" .                     .     .                     . \n",
	" . . . . . . . . . . . . . . . . . . . . . . . . . . \n",
	"                                                     \n",
	0
};

/*
** p_dots()	- fills in the board
*/
void	p_dots ()
{
	reg	char	**str = _dots;

	d_left = MAX_DOTS;
	move(TOP, 0);
	while (*str)
		addstr(*str++);
}
