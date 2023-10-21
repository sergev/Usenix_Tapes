#ifndef lint
static char rcsid[] = "$Header: initplayer.c,v 1.1 84/08/25 17:11:19 chris Exp $";
#endif

#include <stdio.h>
#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"

/*
** set up some useful information about the player to pass to the daemon
** on startup
*/
initplayer(player)
register struct user *player;
{
	char *getlogin();
	char *strncpy();
	extern int version;

	player->u_dir = 0;
	(void) strncpy(player->u_name, getlogin(), 32);
	(void) gethostname(player->u_hostname, 16);
	player->u_flag = U_ALIVE;
	player->u_hp = HITPOINTS;
	player->u_kills = 0;
	player->u_state.s_incarnation = 0;	/* first incarnation */

	/*
	** This code makes sure that the daemon will not allow fake or
	** old versions to run and screw things up
	*/
	player->u_version = version;
	player->u_magic = V_MAGIC;

	initposition(player);
}

/*
** Put the player in a good starting location.  This function assumes that
** the player will eventually find a good place to land.  This depends on
** on the maze.
*/
initposition(player)
register struct user *player;
{

	do {
	    player->u_x = random() % MAZEWIDTH;
	    player->u_y = random() % MAZELENGTH;
	} while (maze[player->u_x][player->u_y]);
}
