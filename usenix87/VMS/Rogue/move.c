#include curses.h
#include "object.h"
#include "room.h"
#include "move.h"
#include "monster.h"

extern short current_room, halluc, detect_monster, blind, being_held;
extern short confused, interrupted, has_amulet;
extern char *hunger_str;

single_move_rogue(dirch, pickup)
{
	short row, col;
	object *obj, *object_at(), *pick_up();
	char description[SCOLS];
	short n, status;

	row = rogue.row;
	col = rogue.col;

	if (being_held) {
		get_dir_rc(dirch, &row, &col);

		if (!(screen[row][col] & MONSTER)) {
			message("you are being held", 1);
			return(MOVE_FAILED);
		}
	}
	row = rogue.row;
	col = rogue.col;

	if (confused) {
		dirch = get_rand_dir();
	}

	switch(dirch) {
	case 'h':
		col--;
		break;
	case 'j':
		row++;
		break;
	case 'k':
		row--;
		break;
	case 'l':
		col++;
		break;
	case 'b':
		col--;
		row++;
		break;
	case 'y':
		col--;
		row--;
		break;
	case 'u':
		col++;
		row--;
		break;
	case 'n':
		col++;
		row++;
		break;
	}
	if (screen[row][col] & MONSTER) {
		rogue_hit(object_at(&level_monsters, row, col));
		register_move();
		return(MOVE_FAILED);
	}
	if (!can_move(rogue.row, rogue.col, row, col)) {
		return(MOVE_FAILED);
	}
	if (screen[row][col] & DOOR) {
		if (current_room == PASSAGE) {
			current_room = get_room_number(row, col);
			light_up_room();
			wake_room(current_room, 1, row, col);
		} else {
			light_passage(row, col);
		}
	} else if ((screen[rogue.row][rogue.col] & DOOR) &&
		   (screen[row][col] & TUNNEL)) {
		light_passage(row, col);
		wake_room(current_room, 0, row, col);
		darken_room(current_room);
		current_room = PASSAGE;
	} else if (screen[row][col] & TUNNEL) {
			light_passage(row, col);
	}
	mvaddch(rogue.row, rogue.col,
	get_room_char(screen[rogue.row][rogue.col], rogue.row, rogue.col));
	mvaddch(row, col, rogue.fchar);
	rogue.row = row; rogue.col = col;
	if (screen[row][col] & CAN_PICK_UP) {
		if (pickup) {
			if (obj = pick_up(row, col, &status)) {
				get_description(obj, description);
				if (obj->what_is == GOLD) {
					free(obj);
					goto NOT_IN_PACK;
				}
			} else if (!status) {
				goto MVED;
			} else {
				goto MOVE_ON;
			}
		} else {
MOVE_ON:		obj = object_at(&level_objects, row, col);
			strcpy(description, "moved onto ");
			get_description(obj, description+11);
			goto NOT_IN_PACK;
		}
		n = strlen(description);
		description[n] = '(';
		description[n+1] = obj->ichar;
		description[n+2] = ')';
		description[n+3] = 0;
NOT_IN_PACK:	message(description, 1);
		register_move();
		return(STOPPED_ON_SOMETHING);
	}
	if ((screen[row][col] & DOOR) || (screen[row][col] & STAIRS)) {
		register_move();
		return(STOPPED_ON_SOMETHING);
	}
MVED:	if (register_move()) {			/* fainted from hunger */
		return(STOPPED_ON_SOMETHING);
	}
	return((confused ? STOPPED_ON_SOMETHING : MOVED));
}

multiple_move_rogue(dirch)
{
	short row, col;
	short m;

	switch(dirch) {
	case '':
	case '\012':
	case '':
	case '':
	case '\031':
	case '\025':
	case '\016':
	case '\002':
		do {
			row = rogue.row;
			col = rogue.col;
			if (((m = single_move_rogue((dirch + 96), 1)) ==
			MOVE_FAILED) || (m == STOPPED_ON_SOMETHING) ||
			interrupted) {
				break;
			}
		} while (!next_to_something(row, col));
		break;
	case 'H':
	case 'J':
	case 'K':
	case 'L':
	case 'B':
	case 'Y':
	case 'U':
	case 'N':
		while (!interrupted &&
			single_move_rogue((dirch + 32), 1) == MOVED) ;
		break;
	}
}

is_passable(row, col)
{
	if ((row < MIN_ROW) || (row > (LINES - 2)) || (col < 0) ||
	    (col > (COLS-1))) {
		return(0);
	}
	return(screen[row][col] & (FLOOR | TUNNEL | DOOR | STAIRS));
}

next_to_something(drow, dcol)
{
	short i, j, i_end, j_end, row, col;
	short pass_count = 0;

	if (confused) {
		return(1);
	}
	if (blind) {
		return(0);
	}
	i_end = (rogue.row < (LINES-2)) ? 1 : 0;
	j_end = (rogue.col < (COLS-1)) ? 1 : 0;

	for (i = ((rogue.row > MIN_ROW) ? -1 : 0); i <= i_end; i++) {
		for (j = ((rogue.col > 0) ? -1 : 0); j <= j_end; j++) {
			if ((i == 0) && (j == 0)) continue;
			if (((rogue.row+i)==drow)&&((rogue.col+j)==dcol)) {
				continue;
			}
			if(screen[rogue.row+i][rogue.col+j]&(MONSTER|IS_OBJECT))return(1);
			if ((((i - j) == 1) || ((i - j) == -1)) &&
			(screen[rogue.row+i][rogue.col+j] & TUNNEL)) {
				if (++pass_count > 1) {
					return(1);
				}
			}
			row = rogue.row+i; col = rogue.col+j;
			if ((screen[row][col] & DOOR)||(is_object(row,col))) {
				if (i == 0 || j == 0) {
					return(1);
				}
			}
		}
	}
	return(0);
}

can_move(row1, col1, row2, col2) 
{
	if (!is_passable(row2, col2)) {
		return(0);
	}
	if ((row1 != row2) && (col1 != col2)) {
		if ((screen[row1][col1]&DOOR)||(screen[row2][col2]&DOOR)) {
			return(0);
		}
		if ((!screen[row1][col2]) || (!screen[row2][col1])) {
			return(0);
		}
	}
	return(1);
}

is_object(row, col)
{
	return(screen[row][col] & IS_OBJECT);
}

move_onto()
{
	short ch;
	short first_miss = 1;
	char getchartt();

	while (!is_direction(ch = getchartt())) {
		putchar(7);
		fflush(stdout);
		if (first_miss) {
			message("direction? ", 0);
			first_miss = 0;
		}
	}
	check_message();
	if (ch != CANCEL) {
		single_move_rogue(ch, 0);
	}
}

is_direction(c)
{
	return( (c == 'h') ||
		(c == 'j') ||
		(c == 'k') ||
		(c == 'l') ||
		(c == 'b') ||
		(c == 'y') ||
		(c == 'u') ||
		(c == 'n') ||
		(c == CANCEL)
		);
}

is_pack_letter(c)
{
	return(((c >= 'a') && (c <= 'z')) || (c == CANCEL) || (c == LIST));
}

check_hunger()
{
	short i, n;
	short fainted = 0;

	if (rogue.moves_left == HUNGRY) {
		hunger_str = "hungry";
		message(hunger_str, 0);
		print_stats();
	}
	if (rogue.moves_left == WEAK) {
		hunger_str = "weak";
		message(hunger_str, 0);
		print_stats();
	}
	if ((rogue.moves_left) <= FAINT) {
		if (rogue.moves_left == FAINT) {
			hunger_str = "faint";
			message(hunger_str, 1);
			print_stats();
		}
		n = get_rand(0, (FAINT - rogue.moves_left));
		if (n > 0) {
			fainted = 1;
			if (rand_percent(40)) rogue.moves_left++;
			message("you faint", 1);
			for (i = 0; i < n; i++) {
				if (rand_percent(50)) {
					move_monsters();
				}
			}
			message("you can move again", 1);
		}
	}
	if (rogue.moves_left <= STARVE) {
		killed_by(0, STARVATION);
	}
	rogue.moves_left--;
	return(fainted);
}

register_move()
{
	static short moves = 0;
	short fainted;

	if ((rogue.moves_left <= HUNGRY) || !has_amulet) {
		fainted = check_hunger();
	} else {
		fainted = 0;
	}

	move_monsters();

	if (++moves >= 80) {
		moves = 0;
		start_wanderer();
	}
	if (halluc) {
		if (!(--halluc)) {
			unhallucinate();
		} else {
			hallucinate();
		}
	}
	if (blind) {
		if (!(--blind)) {
			unblind();
		}
	}
	if (confused) {
		if (!(--confused)) {
			unconfuse();
		}
	}
	heal();

	return(fainted);
}

rest(count)
{
	int i;

	for (i = 0; i < count; i++) {
		if (interrupted) break;
		register_move();
	}
}

get_rand_dir()
{
	short d;

	d = get_rand(1, 8);

	switch(d) {
		case 1:
			return('j');
		case 2:
			return('k');
		case 3:
			return('l');
		case 4:
			return('h');
		case 5:
			return('y');
		case 6:
			return('u');
		case 7:
			return('b');
		case 8:
			return('n');
	}
}

heal()
{
	static short exp = -1, n, c = 0;

	if (rogue.exp != exp) {
		exp = rogue.exp;

		switch(exp) {
		case 1:
			n = 20;
			break;
		case 2:
			n = 18;
			break;
		case 3:
			n = 17;
			break;
		case 4:
			n = 14;
			break;
		case 5:
			n = 13;
			break;
		case 6:
			n = 10;
			break;
		case 7:
			n = 9;
			break;
		case 8:
			n = 8;
			break;
		case 9:
			n = 7;
			break;
		case 10:
			n = 4;
			break;
		case 11:
			n = 3;
			break;
		case 12:
		default:
			n = 2;
		}
	}
	if (rogue.hp_current == rogue.hp_max) {
		c = 0;
		return;
	}

	if (++c >= n) {
		c = 0;
		rogue.hp_current++;
		if (rogue.hp_current < rogue.hp_max) {
			if (rand_percent(50)) {
				rogue.hp_current++;
			}
		}
		print_stats();
	}
}
