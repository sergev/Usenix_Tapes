# include	<curses.h>
# include	<signal.h>

# define RN	(((Seed = Seed*11109+13849) >> 16) & 0xffff)

typedef struct {
	int	x, y;
} YX;

typedef struct spot {
	int	nexits;
	YX	exits[20];
	int	used;
} SPOT;

char	Solchar;

int	Maxy, Maxx, Seed, inter(), Endy;

SPOT	Maze[24][80];

main(ac, av)
int	ac;
char	**av; {

	register int	starty, startx;

	for (starty = 0; starty < 24; starty++)
		for (startx = 0; startx < 24; startx++)
			Maze[starty][startx].used = Maze[starty][startx].nexits = 0;
	Seed = getpid();
	initscr();
	leaveok(stdscr, TRUE);
	signal(SIGINT, inter);
	nonl();
	noecho();
	if (ac == 1) {
		Maxy = 22;
		Maxx = 78;
	}
	else {
		Maxy = atoi(av[1]);
		Maxx = atoi(av[2]);
	}
	starty = (rnd(Maxy) / 2) * 2;
	startx = (rnd(Maxx) / 2) * 2;
	mvaddch(starty, startx, '#');
	leaveok(stdscr, TRUE);
	Endy = (rnd(Maxy) / 2) * 2;
	dig(starty, startx);
	standout();
	Solchar = (SO ? '#' : ':');
	trace((rnd(Maxy) / 2) * 2, 0);
	standend();
	move(LINES - 1, 0);
	clrtoeol();
	refresh();
	endwin();
}

dig(y, x)
register int	y, x; {

	register int	i, count, newy, newx, nexty, nextx;
	register SPOT	*sp;
	static int	dely[] = { 2,-2, 0, 0 };
	static int	delx[] = { 0, 0, 2,-2 };

# ifdef DEBUG
	mvprintw(LINES - 1, 0, "dig(%d, %d)");
	clrtoeol();
	refresh();
	getchar();
# endif
	for (;;) {
		count = 0;
		for (i = 0; i < 4; i++) {
			newy = y + dely[i];
			newx = x + delx[i];
			if (newy < 0 || newy > Maxy || newx < 0 || newx > Maxx)
				continue;
			if (mvinch(newy, newx) != ' ')
				continue;
			if (rnd(++count) == 0) {
				nexty = newy;
				nextx = newx;
			}
		}
		if (count == 0) {
# ifdef DEBUG
			mvprintw(LINES - 1, 0, "count = 0");
			clrtoeol();
			refresh();
			getchar();
# endif
			return;
		}
# ifdef DEBUG
		mvprintw(LINES - 1, 0, "y = %d, nexty = %d, x = %d, nextx = %d", y, nexty, x, nextx);
		clrtoeol();
# endif
		sp = &Maze[y][x];
		sp->exits[sp->nexits].y = nexty;
		sp->exits[sp->nexits++].x = nextx;
		sp = &Maze[nexty][nextx];
		sp->exits[sp->nexits].y = y;
		sp->exits[sp->nexits++].x = x;
		if (nexty == y)
			if (nextx - x < 0)
				mvaddch(nexty, nextx + 1, '#');
			else
				mvaddch(nexty, nextx - 1, '#');
		else
			if (nexty - y < 0)
				mvaddch(nexty + 1, nextx, '#');
			else
				mvaddch(nexty - 1, nextx, '#');
		refresh();
		mvaddch(nexty, nextx, '#');
		refresh();
# ifdef DEBUG
		getchar();
# endif
		dig(nexty, nextx);
	}
}

trace(y, x)
register int	y, x;
{
	register int	i;
	register SPOT	*sp;
	register bool	goodone = FALSE;

	sp = &Maze[y][x];
	sp->used = TRUE;
# ifdef DEBUG
	mvprintw(LINES - 1, 0, "y = %d, x = %d, nexits = %d", y, x, sp->nexits);
	clrtoeol();
	refresh();
	getchar();
# endif
	for (i = 0; i < sp->nexits; i++)
		if (!Maze[sp->exits[i].y][sp->exits[i].x].used) {
# ifdef DEBUG
			mvprintw(LINES - 1, 0, "%d: calling trace(%d, %d)", i, sp->exits[i].y, sp->exits[i].x);
			clrtoeol();
			refresh();
			getchar();
# endif
			if (trace(sp->exits[i].y, sp->exits[i].x)) {
				if (sp->exits[i].y > y)
					mvaddch(y + 1, x, Solchar);
				else if (sp->exits[i].y < y)
					mvaddch(y - 1, x, Solchar);
				if (sp->exits[i].x > x)
					mvaddch(y, x + 1, Solchar);
				else if (sp->exits[i].x < x)
					mvaddch(y, x - 1, Solchar);
				refresh();
				mvaddch(y, x, Solchar);
				refresh();
				goodone = TRUE;
			}
		}
	if (y == Endy && x == Maxx) {
		mvaddch(y, x, Solchar);
		refresh();
		return TRUE;
	}
	else
		return goodone;
}

/*
 * rnd:
 *	Pick a very random number.
 */
rnd(range)
register int range;
{
    return range == 0 ? 0 : abs((int) RN) % range;
}

inter()
{
	endwin();
	exit(1);
}
