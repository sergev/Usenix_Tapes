#include curses.h
#include "room.h"
#include "object.h"
#include "move.h"

short current_room;
room rooms[MAXROOMS];
extern short detect_monster;
extern short blind;

light_up_room()
{
	short i, j;

	if (blind) return;
	for (i = rooms[current_room].top_row;
	i <= rooms[current_room].bottom_row; i++) {
		for (j = rooms[current_room].left_col;
		j <= rooms[current_room].right_col; j++) {
			mvaddch(i, j, get_room_char(screen[i][j], i, j));
		}
	}
	mvaddch(rogue.row, rogue.col, rogue.fchar);
}

light_passage(row, col)
{
	short i, j, i_end, j_end;

	if (blind) return;

	i_end = (row < (LINES-2)) ? 1 : 0;
	j_end = (col < (COLS-1)) ? 1 : 0;

	for (i = ((row > MIN_ROW) ? -1 : 0); i <= i_end; i++) {
		for (j = ((col > 0) ? -1 : 0); j <= j_end; j++) {
			if (is_passable(row+i, col+j)) {
				mvaddch(row+i, col+j,
				get_room_char(screen[row+i][col+j],
				row+i, col+j));
			}
		}
	}
}

darken_room(rn)
short rn;
{
	short i, j;

	if (blind) return;

	for (i = rooms[rn].top_row + 1;
	i < rooms[rn].bottom_row; i++) {
		for (j = rooms[rn].left_col + 1;
		j < rooms[rn].right_col; j++) {
			if (!is_object(i, j) &&
			   !(detect_monster && (screen[i][j] & MONSTER))) {
				if (!hiding_xeroc(i, j)) {
					mvaddch(i, j, ' ');
				}
			}
		}
	}
}

get_room_char(mask, row, col)
register short mask;
register short row, col;
{
	if (mask & MONSTER) {
		return(get_monster_char_row_col(row, col));
	}
	if (mask & SCROLL) {
		return('?');
	}
	if (mask & POTION) {
		return('!');
	}
	if (mask & FOOD) {
		return(':');
	}
	if (mask & WAND) {
		return('/');
	}
	if (mask & ARMOR) {
		return(']');
	}
	if (mask & WEAPON) {
		return(')');
	}
	if (mask & GOLD) {
		return('*');
	}
	if (mask & TUNNEL) {
		return('#');
	}
	if (mask & HORWALL) {
		return('-');
	}
	if (mask & VERTWALL) {
		return('|');
	}
	if (mask & AMULET) {
		return(',');
	}
	if (mask & FLOOR) {
		return('.');
	}
	if (mask & DOOR) {
		return('+');
	}
	if (mask & STAIRS) {
		return('%');
	}
	return(' ');
}

get_rand_row_col(row, col, mask)
short *row, *col;
unsigned short mask;
{
	short rn;
AGAIN:
	do {
		*row = get_rand(MIN_ROW, SROWS-2);
		*col = get_rand(0, SCOLS-1);
		rn = get_room_number(*row, *col);
	} while (!(screen[*row][*col] & mask) || (rn == NO_ROOM));
	if (screen[*row][*col] & (~mask)) goto AGAIN;
}

get_rand_room()
{
	short i;

	do {
		i = get_rand(0, MAXROOMS-1);
	} while (!rooms[i].is_room);

	return(i);
}

fill_room_with_objects(rn)
{
	short i;
	object *obj, *get_rand_object();
	short n, N, row, col;

	N = ((rooms[rn].bottom_row - rooms[rn].top_row) - 1) *
	    ((rooms[rn].right_col - rooms[rn].left_col) - 1);
	n =  get_rand(5, 10);
	if (n > N) n = N - 2;

	for (i = 0; i < n; i++) {
		do {
			row = get_rand(rooms[rn].top_row+1,
				       rooms[rn].bottom_row-1);
			col = get_rand(rooms[rn].left_col+1,
				       rooms[rn].right_col-1);
		} while (screen[row][col] != FLOOR);

		obj = get_rand_object();
		put_object_at(obj, row, col);
	}
	return(n);
}

get_room_number(row, col)
{
	short i;

	for (i = 0; i < MAXROOMS; i++) {
		if ((row >= rooms[i].top_row)&&(row <= rooms[i].bottom_row) &&
		    (col >= rooms[i].left_col)&&(col <= rooms[i].right_col)) {
			return(i);
		}
	}
	return(NO_ROOM);
}

shell()
{
/*
	char *getenv(), *rindex();
	char *sh;
	int status;

	move(LINES-1, 0);
	refresh_vms();
	stop_window();
	putchar('\n');

	if (!(sh = getenv("SHELL"))) {
		sh = "/bin/sh";
	}

	if (!fork()) {
		if (setreuid(-1, getuid()) < 0) exit(1);
		execl(sh, rindex(sh, '/') + 1, 0);
		exit(0);
	}
	wait(&status);
	start_window();
	wrefresh(curscr);
*/
}
