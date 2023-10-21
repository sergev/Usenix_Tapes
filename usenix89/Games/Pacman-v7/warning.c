/*
** warning.c -	code dealing with the energizers and them being eaten and
**		wearing out and informing the player his time is ending
**
**	[pm by Peter Costantinidis, Jr. @ University of California at Davis]
*/

#include "pm.h"

/*
** warning()	- warn pm that energizers are about to wear off
*/
void	warning ()
{
	reg	int	i;

	for (i = 0; i < MAX_MONS; i++)
	{
		if (!ghosts[i].mo_run)
			continue;
		if (!ghosts[i].mo_tunn)
			mvaddch(ghosts[i].mo_pos.y, ghosts[i].mo_pos.x,
				toupper(ghosts[i].mo_name));
	}
	draw();
	/*
	msleep(10l);
	*/
	for (i = 0; i < MAX_MONS; i++)
	{
		if (!ghosts[i].mo_run)
			continue;
		if (!ghosts[i].mo_tunn)
			mvaddch(ghosts[i].mo_pos.y, ghosts[i].mo_pos.x,
				ghosts[i].mo_name);
	}
	draw();
}

/*
** aggressive()	- perform all the house keeping when the enegizers
**		  wear off the pm
*/
void	aggressive ()
{
	reg	int	i;

	mons_eaten = -1;
	timer = 0;			/* reset the timer	*/
	for (i = 0; i < MAX_MONS; i++)
	{
		ghosts[i].mo_run = FALSE;
		if (islower(ghosts[i].mo_name))
			ghosts[i].mo_name = toupper(ghosts[i].mo_name);
		else
			continue;
		if (!ghosts[i].mo_tunn)
			mvaddch(ghosts[i].mo_pos.y, ghosts[i].mo_pos.x, ghosts[i].mo_name);
	}
}

/*
** submissive()	- make the ghosts eatable
*/
void	submissive ()
{
	reg	int	i;

	if (level >= MAX_LEVEL)
		timer = eat_times[MAX_LEVEL - 1];
	else
		timer = eat_times[level];
	pm_run = FALSE;
	mons_eaten = -1;
	for (i = 0; i < MAX_MONS; i++)
	{
		ghosts[i].mo_run = TRUE;
		if (isupper(ghosts[i].mo_name))
			ghosts[i].mo_name = tolower(ghosts[i].mo_name);
		else
			continue;
		if (!ghosts[i].mo_tunn)
			mvaddch(ghosts[i].mo_pos.y, ghosts[i].mo_pos.x, ghosts[i].mo_name);
	}
}
