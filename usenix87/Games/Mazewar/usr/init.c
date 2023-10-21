#ifndef lint
static char rcsid[] = "$Header: init.c,v 1.1 84/08/25 17:11:15 chris Exp $";
#endif

#ifdef PLUS5
#include <sys/types.h>
#endif

#include <stdio.h>
#include <curses.h>
#include <sys/ioctl.h>
#ifdef sun
#include <pixrect/pixrect_hs.h>
#include <sun/fbio.h>
#endif
#include <sys/file.h>

#include "../h/defs.h"
#include "../h/struct.h"
#include "../h/extern.h"
#include "../h/hallmap.h"
#ifdef sun
#include "../h/sunscreen.h"
#endif
#include "../h/arrows.h"

#define ABS(a)		((a) < 0 ? -(a) : (a))


#ifdef sun
extern struct pixrect	*px_mapwin, *px_statwin;
extern struct pixrect	*px_left, *px_right, *px_up, *px_down;
#endif

WINDOW *w_birdv, *w_debug;

/*
 *  Initialize the pixrect from the whole screen.
 */

init_screen()
{
	    (void) initscr();
	    w_birdv = newwin(16,64,0,0);
	    w_debug = newwin(8,80,16,0);
}


/*
 * draw the bird's eye view maze
 */

#include "../h/wall_image.h"

draw_maze()
{
	register int i, j;

	for (i = 0; i < MAZEWIDTH; i++)
	    for (j = 0; j < MAZELENGTH; j++) {
		if (maze[i][j]) {
#ifdef sun
		    if (onsun)
			prcopyat(px_mapwin, &wall_pr, i*10, j*10, OP_WRITE);
		    else
#endif
		    if (ascii)
			mvwaddstr(w_birdv, j, i*2, "[]");
		}
		else
#ifdef sun
		    if (onsun)
			rect(px_mapwin, i*10, j*10, 10, 10, OP_SET);
		    else
#endif
		    if (ascii)
			mvwaddstr(w_birdv, j, i*2, "  ");
	    }
}

int changewb, changewd;

#define IMAGESIZE	10

draw_player(x, y, dir)
char x;
char y;
char dir;
{
	static char oldx = 1, oldy = 1, olddir = 0;
#ifdef DEBUG_2
	char s[BUFSIZ];

	(void) sprintf(s, "Printing player at %d,%d dir = %d.\n", x, y, dir);
	debugs(s);
#endif
	/* If we haven't moved, don't change anything (or redraw) */
	if ((oldx == x) && (oldy == y) && (olddir == dir))
	    return;

	/* erase old position */
#ifdef sun 
	if (onsun)
	    rect(px_mapwin, oldx * IMAGESIZE, oldy * IMAGESIZE, 10, 10, OP_SET);
	else
#endif
	    mvwaddstr(w_birdv, oldy, oldx*2, "  ");
	oldx = x;
	oldy = y;
	olddir = dir;

	switch (dir) {
	    case 0:
#ifdef sun
		if (onsun)
		    prcopyat(px_mapwin, px_up, x * IMAGESIZE,
			y * IMAGESIZE, OP_WRITE);
		else
#endif
		if (ascii)
		    mvwaddstr(w_birdv, y, x*2, "^^");
		break;
	    case 1:
#ifdef sun
		if (onsun)
		    prcopyat(px_mapwin, px_right, x * IMAGESIZE, 
			y * IMAGESIZE, OP_WRITE);
		else
#endif
		if (ascii)
		    mvwaddstr(w_birdv, y, x*2, ">>");
		break;
	    case 2:
#ifdef sun
		if (onsun)
		    prcopyat(px_mapwin, px_down, x * IMAGESIZE, 
			y * IMAGESIZE, OP_WRITE);
		else
#endif
		if (ascii)
		    mvwaddstr(w_birdv, y, x*2, "vv");
		break;
	    case 3:
#ifdef sun
		if (onsun)
		    prcopyat(px_mapwin, px_left, x * IMAGESIZE, 
			y * IMAGESIZE, OP_WRITE);
		else
#endif
		if (ascii)
		    mvwaddstr(w_birdv, y, x*2, "<<");
		break;
	}
#ifdef sun
	if (onsun)
		copyat(px_mapwin, MAPX, MAPY, OP_WRITE);
#endif sun
	if (ascii)
		changewb = 1;
}


debugs(string)
char *string;
{
    static int curry = 0;

    (void) mvwaddstr(w_debug, curry, 0, "                                                                              ");
    (void) wrefresh(w_debug);
    (void) mvwaddstr(w_debug, curry++, 0, string);
    (void) wrefresh(w_debug);
    if (curry > 4)
	curry = 0;
}

srefresh()
{
    draw_nearest();	/* draw the nearest other player (if in view) */

    /*  Down here we would deal with whether we could see anyone else */
    if (ascii) {
	draw_player(me->u_x, me->u_y, me->u_dir);
	if (changewd) {
	    (void) wmove(w_debug, 0,0);
	    (void) wrefresh(w_debug);
	    changewd = 0;
	}
	if (changewb) {
	    (void) wmove(w_birdv, 0,0);
	    (void) wrefresh(w_birdv);
	    changewb = 0;
	}
    }
#ifdef sun
	else {
		draw_player(me->u_x, me->u_y, me->u_dir);
		draw_rats();
		copyat(px_statwin, STATX, STATY, OP_WRITE);
	}
#endif
}

#ifdef DEBUG
draw_others()
{
	register int i;

	draw_maze();
	for (i = 0; i < MAXPLAYER; i++) {
	    if ((! (players[i].u_flag & U_ALIVE)) || (me->u_slot == i))
		continue;
#ifdef sun
	    if (onsun)
		pr_rop(px_mapwin, players[i].u_x*10, players[i].u_y*10, 10, 10, 
		    PIX_SRC, px_up, 0, 0);
	    else
#endif
	    {
		(void) wmove(w_birdv, players[i].u_y, players[i].u_x*2);
		(void) wprintw(w_birdv, "%2d", i);
	    }
	}
}
#endif

int ddir[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
draw_nearest()
{
	register struct user *i;
	struct user *nearest();
	int x, y;
	static oldx = -1, oldy = -1, oldi = -1;
#ifdef DEBUG
	char s[BUFSIZ];
#endif

	if (peek) {
	    x = me->u_x + ddir[me->u_dir][0];
	    y = me->u_y + ddir[me->u_dir][1];
	}
	else {
	    x = me->u_x;
	    y = me->u_y;
	}
	if ((i = nearest((me->u_dir + peek) % 4, x, y)) == 0) {
	    if (oldx >= 0) {		/* clear last picture */
#ifdef sun 
		    if (onsun) {
#ifdef sun_showplayer
			rect(px_mapwin, oldx*10, oldy*10, 10, 10, OP_SET);
			copyat(px_mapwin, MAPX, MAPY, OP_WRITE);
#endif
		    } else
#endif
			(void) mvwaddstr(w_birdv, oldy, oldx*2, "  ");
		changewb = 1;
	    }
	    oldx = -1;			/* No one in view */
	}
	else {
	    if ((oldx == i->u_x) && (oldy == i->u_y) && (i->u_slot == oldi))
		return;

	    if (oldx >= 0) {		/* clear last picture */
#ifdef sun
		    if (onsun)
#ifdef sun_showplayer
			rect(px_mapwin, oldx*10, oldy*10, 10, 10, OP_SET);
#else
				;
#endif
		    else
#endif
			(void) mvwaddstr(w_birdv, oldy, oldx*2, "  ");
	    }
	    oldx = i->u_x;
	    oldy = i->u_y;
	    oldi = i->u_slot;
#ifdef sun
		if (onsun) {
#ifdef sun_showplayer
#define TEXT_FOOTING	9
			mvprintf(px_mapwin, oldx * 10, oldy * 10 + TEXT_FOOTING,
			    "%d", i->u_slot);
			copyat(px_mapwin, MAPX, MAPY, OP_WRITE);
#endif
		} else
#endif
		{
		    (void) wmove(w_birdv, oldy, oldx*2);
		    (void) wprintw(w_birdv, "%2d", i->u_slot);
		}
	    changewb = 1;
	}

#ifdef DEBUG_2
	if (debug) {
	    (void) sprintf(s, "Nearest player is %d\n", i->u_slot);
	    debugs(s);
	}
#endif
	return;
}

extern int bitmaze[];

struct user *
nearest(medir, mex, mey)
int medir, mex, mey;
{
    register int i, x, y;
/*
    register struct user *up;
*/
    register int dx, dy;
    register int temp;

    dx = ddir[medir][0];
    dy = ddir[medir][1];

    for (x = mex + dx, y = mey + dy; ! maze[x][y]; x += dx, y+= dy) {
	if (temp = bitmaze[inmaze(x,y)]) {
	    for (i = 0; i < MAXPLAYER; temp >>= 1, i++)
		if ((temp & 0x1) && (players[i].u_flag & U_ALIVE)) {
		    if (inmaze(x,y) == inmaze(players[i].u_x, players[i].u_y))
			return(&players[i]);
		    delplayer(i, x, y);
		    continue;
		}
	}
/*
	    for (i = 0, up = &players[0]; i < MAXPLAYER; up++, i++) {
		if ((! (up->u_flag & U_ALIVE)) || (i == me->u_slot))
		    continue;
		if ((up->u_x == x) && (up->u_y == y))
		    return(up);
	    }
*/
    }
    return(0);
}

draw_status(i)
register int i;
{

#ifdef sun
#define TOP		15
#define SPACING		40
#define RIGHTEDGE	1
#define SECONDLINE	15
#define INDENT		18
	if (onsun) {
		/* clear out the previous status area */
		rect(px_statwin, 0, i * SPACING, STATW, SPACING, OP_SET);

		if (players[i].u_flag & U_ALIVE) {
			mvprintf(px_statwin, 
			    RIGHTEDGE, i * SPACING + TOP,
			    "%c%2d) %.10s@%s",
			    (me->u_slot == players[i].u_slot) ? '*' : ' ',
			    i, players[i].u_name, players[i].u_hostname);
			mvprintf(px_statwin, RIGHTEDGE + INDENT, 
			    i * SPACING + SECONDLINE + TOP, 
			    "points: %d", players[i].u_kills);
		}
	} else
#endif sun
	{
		(void) wmove(w_debug, i/2, (i%2) * 40); 
		if (players[i].u_flag & U_ALIVE) {
			(void) wprintw(w_debug, "%2d %-8s %-16s %3d %3d", i,
			players[i].u_name, players[i].u_hostname,
			players[i].u_hp, players[i].u_kills);
		}
		else {
		(void) wprintw(w_debug, "                                    ");
		}
		changewd = 1;
	}
}

#ifdef sun
draw_rats()
{
	register struct user *i;
	int x, y;

	if (peek) {
	    x = me->u_x + ddir[me->u_dir][0];
	    y = me->u_y + ddir[me->u_dir][1];
	} else {
	    x = me->u_x;
	    y = me->u_y;
	}
	draw_hall((me->u_dir + peek) % 4, x, y);
}

draw_hall(dir, mex, mey)
int dir, mex, mey;
{
	register int dx, dy;
	register int *dl, *dr;
	register int x, y;
	struct user	*near;
	int i = 0;
	int hall[MAXDEPTH];

	dx = ddir[dir][0];
	dy = ddir[dir][1];
	dl = ddir[(dir + 3) % 4];
	dr = ddir[(dir + 1) % 4];

	for (x = mex, y = mey; i < MAXDEPTH; x += dx, y+= dy) {
		hall[i] = 0;
		/* Left square */
		if (!maze[x + dl[0]][y + dl[1]])
			hall[i] |= H_LEFTP;
		/* Right square */
		if (!maze[x + dr[0]][y + dr[1]])
			hall[i] |= H_RIGHTP;
		if (maze[x][y]) {
			hall[i] |= H_WALL;
			break;
		}
		i++;
	}

	{
		int	dist, odir;

		near = nearest(dir, mex, mey);
		if (near == NULL)
			draw_view(hall, 0, 0);
		else {
			dist = ABS(mex - near->u_x) + ABS(mey - near->u_y);
			odir = relative_dir(dir, near->u_dir);
			draw_view(hall, dist, odir);
		}
	}
}


/*
 *  Return the other player's relative direction
 *  to you.  This routine assumes that the player
 *  will never be at a diagonal diection from you.
 */

relative_dir(me, him)
{
	if (me == him)
		return DIR_BACK;
	if (me == (him + 2) % 4)
		return DIR_FRONT;
	if (me == (him + 1) % 4)
		return DIR_LEFT;
	if (me == (him + 3) % 4)
		return DIR_RIGHT;
	
	return(0);
}
#endif sun
