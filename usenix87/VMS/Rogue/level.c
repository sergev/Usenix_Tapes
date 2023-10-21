#include curses.h
#include "move.h"
#include "object.h"
#include "room.h"
#include "macros.h"

short current_level = 0, max_level = 1;
extern short current_room;
char *hunger_str = "";
short party_room;
extern short being_held;
extern short detect_monster, has_amulet;

int level_points[] = {
	      10,
	      20,
	      40,
	      80,
	     160,
	     320,
	     640,
	    1300,
	    2600,
	    5200,
	   10000,
	   20000,
	   40000,
	   80000,
	  160000,
	  320000,
	 1000000,
	10000000
};

make_level()
{
	short i;
	short must_exist1, must_exist2;

	party_room = -1;

	if (current_level < 126) {
		current_level++;
	}
	if (current_level > max_level) max_level = current_level;

	if (rand_percent(50)) {
		must_exist1 = 1;
		must_exist2 = 7;
	} else {
		must_exist1 = 3;
		must_exist2 = 5;
	}

	for (i = 0; i < MAXROOMS; i++) {
		make_room(i, must_exist1, must_exist2, 4);
	}

	try_rooms(0, 1, 2);
	try_rooms(0, 3, 6);
	try_rooms(2, 5, 8);
	try_rooms(6, 7, 8);

	for (i = 0; i < (MAXROOMS); i++) {
		connect_rooms(i, i+1, must_exist1, must_exist2, 4);
		if (i < (MAXROOMS-3)) {
			connect_rooms(i, i+3, must_exist1, must_exist2, 4);
		}
	}
	add_dead_ends();

	if (!has_amulet && (current_level >= AMULET_LEVEL)) {
		put_amulet();
	}
}

make_room(n, r1, r2, r3)
{
	short left_col, right_col, top_row, bottom_row;
	short width, height;
	short row_offset, col_offset;
	short i, j;
	short ch;

	switch(n) {
	case 0:
		left_col = 0;
		right_col = COL1-1;
		top_row = MIN_ROW;
		bottom_row = ROW1-1;
		break;
	case 1:
		left_col = COL1+1;
		right_col = COL2-1;
		top_row = MIN_ROW;
		bottom_row = ROW1-1;
		break;
	case 2:
		left_col = COL2+1;
		right_col = COLS-1;
		top_row = MIN_ROW;
		bottom_row = ROW1-1;
		break;
	case 3:
		left_col = 0;
		right_col = COL1-1;
		top_row = ROW1+1;
		bottom_row = ROW2-1;
		break;
	case 4:
		left_col = COL1+1;
		right_col = COL2-1;
		top_row = ROW1+1;
		bottom_row = ROW2-1;
		break;
	case 5:
		left_col = COL2+1;
		right_col = COLS-1;
		top_row = ROW1+1;
		bottom_row = ROW2-1;
		break;
	case 6:
		left_col = 0;
		right_col = COL1-1;
		top_row = ROW2+1;
		bottom_row = LINES - 2;
		break;
	case 7:
		left_col = COL1+1;
		right_col = COL2-1;
		top_row = ROW2+1;
		bottom_row = LINES - 2;
		break;
	case 8:
		left_col = COL2+1;
		right_col = COLS-1;
		top_row = ROW2+1;
		bottom_row = LINES - 2;
		break;
	}

	if ((n != r1) && (n != r2) && (n != r3) && rand_percent(45)) {
		goto END;
	}
	height = get_rand(4, (bottom_row-top_row+1));
	width = get_rand(7, (right_col-left_col-2));

	row_offset = get_rand(0, ((bottom_row-top_row)-height+1));
	col_offset = get_rand(0, ((right_col-left_col)-width+1));

	top_row += row_offset;
	bottom_row = top_row + height - 1;

	left_col += col_offset;
	right_col = left_col + width - 1;

	rooms[n].is_room = 1;

	for (i = top_row; i <= bottom_row; i++) {
		for (j = left_col; j <= right_col; j++) {
			if ((i == top_row) || (i == bottom_row)) {
				ch = HORWALL;
			} else if (((i != top_row) && (i != bottom_row)) &&
			((j == left_col) || (j == right_col))) {
				ch = VERTWALL;
			} else {
				ch = FLOOR;
			}
			add_mask(i, j, ch);
		}
	}
END:	rooms[n].top_row = top_row;
	rooms[n].bottom_row = bottom_row;
	rooms[n].left_col = left_col;
	rooms[n].right_col = right_col;
	rooms[n].height = height;
	rooms[n].width = width;
}

connect_rooms(room1, room2, m1, m2, m3)
{
	if ((room1 != m1) && (room1 != m2) && (room1 != m3) && (room2 != m1) &&
	    (room2 != m2) && (room2 != m3)) {
		if (rand_percent(80)) {
			return;
		}
	}

	if (adjascent(room1, room2)) {
		do_connect(room1, room2);
	}
}

do_connect(room1, room2)
{
	short row1, col1, row2, col2, dir;

	if ((rooms[room1].left_col > rooms[room2].right_col) &&
	(on_same_row(room1, room2))) {
		put_door(room1, LEFT, &row1, &col1);
		put_door(room2, RIGHT, &row2, &col2);
		dir = LEFT;
	} else if ((rooms[room2].left_col > rooms[room1].right_col) &&
	(on_same_row(room1, room2))) {
		put_door(room1, RIGHT, &row1, &col1);
		put_door(room2, LEFT, &row2, &col2);
		dir = RIGHT;
	} else if ((rooms[room1].top_row > rooms[room2].bottom_row) &&
	(on_same_col(room1, room2))) {
		put_door(room1, UP, &row1, &col1);
		put_door(room2, DOWN, &row2, &col2);
		dir = UP;
	} else if ((rooms[room2].top_row > rooms[room1].bottom_row) &&
	(on_same_col(room1, room2))) {
		put_door(room1, DOWN, &row1, &col1);
		put_door(room2, UP, &row2, &col2);
		dir = DOWN;
	} else {
		return;
	}
	draw_simple_passage(row1, col1, row2, col2, dir);
	if (rand_percent(10)) {
		draw_simple_passage(row1, col1, row2, col2, dir);
	}
	rooms[room1].doors[dir/2].other_room = room2;
	rooms[room1].doors[dir/2].other_row = row2;
	rooms[room1].doors[dir/2].other_col = col2;

	rooms[room2].doors[(((dir+4)%8)/2)].other_room = room1;
	rooms[room2].doors[(((dir+4)%8)/2)].other_row = row1;
	rooms[room2].doors[(((dir+4)%8)/2)].other_col = col1;
}

clear_level()
{
	int i, j;

	for (i = 0; i < MAXROOMS; i++) {
		rooms[i].is_room = 0;
		for (j = 0; j < 4; j++) {
			rooms[i].doors[j].other_room = NO_ROOM;
		}
	}
	for (i = 0; i < SROWS; i++) {
		for (j = 0; j < SCOLS; j++) {
			screen[i][j] = BLANK;
		}
	}
	detect_monster = 0;
	being_held = 0;
}

print_stats()
{
	char mbuf[100];

	sprintf(mbuf, 
	"Level: %d  Gold: %3d  Hp: %2d(%d)  Str: %2d(%d)  Arm: %2d  Exp: %d/%d %s", 
	current_level, rogue.gold, rogue.hp_current, rogue.hp_max, rogue.strength_current, 
	rogue.strength_max, get_armor_class(rogue.armor), rogue.exp, rogue.exp_points,
    hunger_str);
	mvaddstr(LINES-1, 0, mbuf);
	clrtoeol();
	refresh_vms();
}

add_mask(row, col, mask)
int row, col;
unsigned short mask;
{
	if (mask == DOOR) {
		remove_mask(row, col, HORWALL);
		remove_mask(row, col, VERTWALL);
	}
	screen[row][col] |= mask;
/*switch(mask) {
	case FLOOR:
		mvaddch(row, col, '.');
		break;
	case STAIRS:
		mvaddch(row, col, '%');
		break;
	case DOOR:
		mvaddch(row, col, '+');
		break;
	case VERTWALL:
		mvaddch(row, col, '|');
		break;
	case HORWALL:
		mvaddch(row, col, '-');
		break;
	case TUNNEL:
		mvaddch(row, col, '#');
		break;
}*/
}

remove_mask(row, col, mask)
{
	screen[row][col] &= (~mask);
}

adjascent(room1, room2)
{
	if ((!rooms[room1].is_room) || (!rooms[room2].is_room)) {
		return(0);
	}
	two_sort(room1, room2);
	return((on_same_col(room1,room2) || on_same_row(room1, room2))
	&& (((room2 - room1) == 1) || (((room2 - room1) == 3))));
}

put_door(rn, dir, row, col)
short *row, *col;
{
	switch(dir) {
	case UP:
	case DOWN:
		*row = ((dir == UP) ? rooms[rn].top_row :
				     rooms[rn].bottom_row);
		*col = get_rand(rooms[rn].left_col+1,
		rooms[rn].right_col-1);
		break;
	case RIGHT:
	case LEFT:
		*row = get_rand(rooms[rn].top_row+1,
		rooms[rn].bottom_row-1);
		*col = (dir == LEFT) ? rooms[rn].left_col :
				     rooms[rn].right_col;
		break;
	}
	add_mask(*row, *col, DOOR);
}

draw_simple_passage(row1, col1, row2, col2, dir)
{
	short i, middle;

	if ((dir == LEFT) || (dir == RIGHT)) {
		if (col2 < col1) {
			swap(row1, row2);
			swap(col1, col2);
		}
		middle = get_rand(col1+1, col2-1);
		for (i = col1+1; i != middle; i++) {
			add_mask(row1, i, TUNNEL);
		}
		for (i = row1; i != row2; i += (row1 > row2) ? -1 : 1) {
			add_mask(i, middle, TUNNEL);
		}
		for (i = middle; i != col2; i++) {
			add_mask(row2, i, TUNNEL);
		}
	} else {
		if (row2 < row1) {
			swap(row1, row2);
			swap(col1, col2);
		}
		middle = get_rand(row1+1, row2-1);
		for (i = row1+1; i != middle; i++) {
			add_mask(i, col1, TUNNEL);
		}
		for (i = col1; i != col2; i += (col1 > col2) ? -1 : 1) {
			add_mask(middle, i, TUNNEL);
		}
		for (i = middle; i != row2; i++) {
			add_mask(i, col2, TUNNEL);
		}
	}
}

on_same_row(room1, room2)
{
	return((room1 / 3) == (room2 / 3));
}

on_same_col(room1, room2)
{
	return((room1 % 3) == (room2 % 3));
}

add_dead_ends()
{
	short i, j;
	short start;
	short row, col, distance, dir;
	short found;
	short dead_end_percent;

	if (current_level <= 2) return;

	start = get_rand(0, (MAXROOMS-1));
	dead_end_percent = 12 + current_level + current_level;

	for (i = 0; i < MAXROOMS; i++) {
		j = ((start + i) % MAXROOMS);
		if (rooms[j].is_room) continue;
		if (!rand_percent(dead_end_percent)) continue;

		row = rooms[j].top_row + get_rand(0, 6);
		col = rooms[j].left_col + get_rand(0, 19);

		found = 0;
		while (!found) {
			distance = get_rand(8, 20);
			dir = get_rand(0, 3) * 2;

			for (j = 0; (j < distance) && !found; j++) {
				switch(dir) {
				case UP:
					if ((row - 1) >= MIN_ROW) {
						--row;
					}
					break;
				case RIGHT:
					if ((col + 1) < (COLS-1)) {
						++col;
					}
					break;
				case DOWN:
					if ((row + 1) < (LINES-2)) {
						++row;
					}
					break;
				case LEFT:
					if ((col - 1) > 0) {
						--col;
					}
					break;
				}
				if ((screen[row][col] & VERTWALL) ||
				(screen[row][col] & HORWALL) ||
				(screen[row][col] & DOOR)) {
					break_in(row, col, screen[row][col],
					dir);
					found = 1;
				} else {
					add_mask(row, col, TUNNEL);
				}
			}
		}
	}
}

break_in(row, col, ch, dir)
{
	short rn;
	short i, drow, dcol;

	if (ch == DOOR) {
		return;
	}
	rn = get_room_number(row, col);

	if (ch == VERTWALL) {
		if (col == rooms[rn].left_col) {
			if (rooms[rn].doors[LEFT/2].other_room != NO_ROOM) {
				drow = door_row(rn, LEFT);
				for (i = row; i != drow;
				i += (drow < row) ? -1 : 1) {
					add_mask(i, col-1, TUNNEL);
				}
			} else {
				rooms[rn].doors[LEFT/2].other_room = DEAD_END;
				add_mask(row, col, DOOR);
			}
		} else {
			if (rooms[rn].doors[RIGHT/2].other_room != NO_ROOM) {
				drow = door_row(rn, RIGHT);
				for (i = row; i != drow;
				i += (drow < row) ? -1 : 1) {
					add_mask(i, col+1, TUNNEL);
				}
			} else {
				rooms[rn].doors[RIGHT/2].other_room = DEAD_END;
				add_mask(row, col, DOOR);
			}
		}
	} else {		/* break in througt top or bottom --- */
		if (col == rooms[rn].left_col) {
			if (row == MIN_ROW) {
				add_mask(row+1, col-1, TUNNEL);
				break_in(row+1, col, VERTWALL, RIGHT);
			} else if (row == (LINES-2)) {
				add_mask(row-1, col-1, TUNNEL);
				break_in(row-1, col, VERTWALL, RIGHT);
			} else {
				if (row == rooms[rn].top_row) {
					if (dir == RIGHT) {
						add_mask(row-1, col-1, TUNNEL);
						add_mask(row-1, col, TUNNEL);
					}
					add_mask(row-1, col+1, TUNNEL);
					break_in(row, col+1, HORWALL, DOWN);
				} else {
					if (dir == RIGHT) {
						add_mask(row+1, col-1, TUNNEL);
						add_mask(row+1, col, TUNNEL);
					}
					add_mask(row+1, col+1, TUNNEL);
					break_in(row, col+1, HORWALL, UP);
				}
			}
			return;
		} else if (col == rooms[rn].right_col) {
			if (row == MIN_ROW) {
				add_mask(row+1, col+1, TUNNEL);
				break_in(row+1, col, VERTWALL, LEFT);
			} else if (row == (LINES-2)) {
				add_mask(row-1, col+1, TUNNEL);
				break_in(row-1, col, VERTWALL, LEFT);
			} else {
				if (row == rooms[rn].top_row) {
					if (dir == DOWN) {
						add_mask(row-1, col+1, TUNNEL);
						add_mask(row, col+1, TUNNEL);
					}
					add_mask(row+1, col+1, TUNNEL);
					break_in(row+1, col, VERTWALL, LEFT);
				} else {
					if (dir == UP) {
						add_mask(row+1, col+1, TUNNEL);
						add_mask(row, col+1, TUNNEL);
					}
					add_mask(row-1, col+1, TUNNEL);
					break_in(row-1, col, VERTWALL, LEFT);
				}
			}
			return;
		}
		if (row == rooms[rn].top_row) {
			if (rooms[rn].doors[UP/2].other_room != NO_ROOM) {
				dcol = door_col(rn, UP);
				for (i = col; i != dcol;
				i += (dcol < col) ? -1 : 1) {
					add_mask(row-1, i, TUNNEL);
				}
			} else {
				rooms[rn].doors[UP/2].other_room = DEAD_END;
				add_mask(row, col, DOOR);
			}
		} else {
			if (rooms[rn].doors[DOWN/2].other_room != NO_ROOM) {
				dcol = door_col(rn, DOWN);
				for (i = col; i != dcol;
				i += (dcol < col) ? -1 : 1) {
					add_mask(row+1, i, TUNNEL);
				}
			} else {
				rooms[rn].doors[DOWN/2].other_room = DEAD_END;
				add_mask(row, col, DOOR);
			}
		}
	}
	return(0);
}

door_row(rn, dir)
{
	short i, col, row;

	if (rooms[rn].doors[dir/2].other_room == NO_ROOM) {
		return(-1);
	}

	row = rooms[rn].top_row;

	switch(dir) {
	case LEFT:
		col = rooms[rn].left_col;
		for (i = row; !(screen[i][col] & DOOR); i++) ;
		break;
	case RIGHT:
		col = rooms[rn].right_col;
		for (i = row; !(screen[i][col] & DOOR); i++) ;
		break;
	}
	return(i);
}

door_col(rn, dir)
{
	short i, col, row;

	if (rooms[rn].doors[dir/2].other_room == NO_ROOM) {
		return(-1);
	}

	col = rooms[rn].left_col;

	switch(dir) {
	case UP:
		row = rooms[rn].top_row;
		for (i = col; !(screen[row][i] & DOOR); i++) ;
		break;
	case DOWN:
		row = rooms[rn].bottom_row;
		for (i = col; !(screen[row][i] & DOOR); i++) ;
		break;
	}
	return(i);
}

put_player()
{
	for (;;) {
		get_rand_row_col(&rogue.row, &rogue.col, (FLOOR | IS_OBJECT));

		current_room = get_room_number(rogue.row, rogue.col);

		if (current_room != party_room) {
			break;
		}
	}
}

check_down()
{
	if (screen[rogue.row][rogue.col] & STAIRS) {
		return(1);
	}
	message("I see no way down", 0);
	return(0);
}

check_up()
{
	if (!(screen[rogue.row][rogue.col] & STAIRS)) {
		message("I see no way up", 0);
		return(0);
	}
	if (!has_amulet) {
		message("your way is magically blocked", 0);
		return(0);
	}
	if (current_level == 1) {
		win();
	} else {
		current_level -= 2;
		return(1);
	}
}

add_exp(e)
{
	char mbuf[40];
	short new_exp;
	short i, hp;

	rogue.exp_points += e;

	if (rogue.exp_points >= level_points[rogue.exp-1]) {
		new_exp = get_exp_level(rogue.exp_points);
		for (i = rogue.exp+1; i <= new_exp; i++) {
			sprintf(mbuf, "welcome to level %d", i);
			message(mbuf, 0);
			hp = get_rand(3, 10);
			rogue.hp_current += hp;
			rogue.hp_max += hp;
			print_stats();
		}
		rogue.exp = new_exp;
	}
	print_stats();
}

get_exp_level(e)
{
	short i;

	for (i = 0; i < 50; i++) {
		if (level_points[i] > e) {
			break;
		}
	}
	return(i+1);
}

try_rooms(r1, r2, r3)
{
	if (rooms[r1].is_room && !rooms[r2].is_room && rooms[r3].is_room) {
		if (rand_percent(75)) {
			do_connect(r1, r3);
		}
	}
}
