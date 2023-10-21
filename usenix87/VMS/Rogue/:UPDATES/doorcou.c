#include "object.h"
#include "monster.h"
#include "room.h"
#include "move.h"

door_course(monster, entering, row, col)
object *monster;
short entering, row, col;
{
	short trow, tcol, i, j, rn;
	short rrow, ccol;

	monster->row = row;
	monster->col = col;

	if (monster_can_see(monster, rogue.row, rogue.col)) {
		monster->trow = -1;
		return;
	}
	rn = get_room_number(row, col);

	if (entering) {		/* entering room */
		for (i = 0; i < MAXROOMS; i++) {
			if (!rooms[i].is_room || (i == rn)) continue;
			for (j = 0; j < 4; j++) {
				if (rooms[i].doors[j].other_room == rn) {
				monster->trow = rooms[i].doors[j].other_row;
				monster->tcol = rooms[i].doors[j].other_col;
				if ((monster->trow == row) &&
				    (monster->tcol == col)) {
					continue;
				}
				return;
				}
			}
		}
	} else {		/* exiting room */
		rrow = row;
		ccol = col;
		if (get_other_room(rn, &rrow, &ccol)) {
			monster->trow = rrow;
			monster->tcol = ccol;
		} else {
			monster->trow = -1;
		}
	}
}

get_other_room(rn, row, col)
short rn, *row, *col;
{
	short d = -1;

	if ((screen[*row][(*col)-1]&HORWALL)&&(screen[*row][(*col)+1]&HORWALL)){
		if (screen[(*row)+1][*col] & FLOOR) {
			d = UP/2;
		} else {
			d = DOWN/2;
		}
	} else {
		if (screen[*row][(*col)+1] & FLOOR) {
			d = LEFT/2;
		} else {
			d = RIGHT/2;
		}
	}
	if ((d != -1) && (rooms[rn].doors[d].other_room > 0)) {
		*row = rooms[rn].doors[d].other_row;
		*col = rooms[rn].doors[d].other_col;
		return(1);
	}
	return(0);
}
