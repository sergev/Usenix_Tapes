#include curses.h
#include "object.h"
#include "move.h"
#include "monster.h"

extern short current_room, being_held, current_level;

zapp()
{
	short wch;
	short first_miss = 1;
	object *wand, *get_letter_object();
	short dir, row, col;
	object *monster, *get_zapped_monster();
	char getchartt();

	while (!is_direction(dir = getchartt())) {
		putchar(7);
		fflush(stdout);
		if (first_miss) {
			message("direction? ", 0);
			first_miss = 0;
		}
	}
	if (dir == CANCEL) {
		check_message();
		return;
	}
	if ((wch = get_pack_letter("zap with what?", WAND)) == CANCEL) {
		check_message();
		return;
	}
	check_message();

	if (!(wand = get_letter_object(wch))) {
		message("no such item.", 0);
		return;
	}
	if (wand->what_is != WAND) {
		message("you can't zap with that", 0);
		return;
	}
	if (wand->class <= 0) {
		message("nothing happens", 0);
		goto RM;
	} else {
		wand->class--;
	}
	row = rogue.row; col = rogue.col;
	monster = get_zapped_monster(dir, &row, &col);
	if (monster != 0) {
		wake_up(monster);
		zap_monster(monster, wand->which_kind);
	}
RM:	register_move();
}

object *get_zapped_monster(dir, row, col)
short dir;
short *row, *col;
{
	object *object_at();
	short orow, ocol;
	short i;

	for (;;) {
		orow = *row; ocol = *col;
		get_dir_rc(dir, row, col);
		if (((*row == orow) && (*col == ocol)) ||
		   (screen[*row][*col] & (HORWALL | VERTWALL)) ||
		   (screen[*row][*col] == BLANK)) {
			return(0);
		}
		if (screen[*row][*col] & MONSTER) {
			if (!hiding_xeroc(*row, *col)) {
				return(object_at(&level_monsters, *row, *col));
			}
		}
	}
}

zap_monster(monster, kind)
object *monster;
short kind;
{
	short row, col;
	struct object *nm;

	row = monster->row;
	col = monster->col;
	nm = monster->next_object;

	switch(kind) {
	case SLOW_MONSTER:
		if (monster->m_flags & HASTED) {
			monster->m_flags &= (~HASTED);
		} else {
			monster->quiver = 0;
			monster->m_flags |= SLOWED;
		}
		break;
	case HASTE_MONSTER:
		if (monster->m_flags & SLOWED) {
			monster->m_flags &= (~SLOWED);
		} else {
			monster->m_flags |= HASTED;
		}
		break;
	case TELEPORT_AWAY:
		teleport_away(monster);
		break;
	case KILL_MONSTER:
		rogue.exp_points -= monster->kill_exp;
		monster_damage(monster, monster->quantity);
		break;
	case INVISIBILITY:
		monster->m_flags |= IS_INVIS;
		mvaddch(row, col, get_monster_char(monster));
		break;
	case POLYMORPH:
		if (monster->ichar == 'F') {
			being_held = 0;
		}
MA:		*monster = monster_tab[get_rand(0, MONSTERS-1)];
		if ((monster->ichar == 'X') && ((current_level < XEROC1) ||
		    (current_level > XEROC2))) {
			goto MA;
		}
		monster->what_is = MONSTER;
		monster->row = row; monster->col = col;
		monster->next_object = nm;
		wake_up(monster);
		if (can_see(row, col)) {
			mvaddch(row, col, get_monster_char(monster));
		}
		break;
	case PUT_TO_SLEEP:
		monster->m_flags |= IS_ASLEEP;
		monster->m_flags &= (~WAKENS);
		break;
	case DO_NOTHING:
		message("nothing happens", 0);
		break;
	}
}

teleport_away(monster)
object *monster;
{
	short row, col;

	if (monster->ichar == 'F') {
		being_held = 0;
	}
	get_rand_row_col(&row, &col, (FLOOR | TUNNEL | IS_OBJECT));
	remove_mask(monster->row, monster->col, MONSTER);
	mvaddch(monster->row, monster->col,
	get_room_char(screen[monster->row][monster->col], monster->row,
	monster->col));

	monster->row = row; monster->col = col;
	add_mask(row, col, MONSTER);

	if (can_see(row, col)) {
		mvaddch(row, col, get_monster_char(monster));
	}
}
