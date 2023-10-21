#include curses.h
#include "object.h"
#include "move.h"
#include "monster.h"

extern short current_room;
extern char *CURSE_MESSAGE;
extern char hit_message[];

throw()
{
	char getchartt();
	short wch;
	short first_miss = 1, f = 1;
	object *weapon, *get_letter_object();
	short dir, row, col;
	object *monster, *get_thrown_at_monster();

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
	if ((wch = get_pack_letter("throw what?", WEAPON)) == CANCEL) {
		check_message();
		return;
	}
	check_message();

	if (!(weapon = get_letter_object(wch))) {
		message("no such item.", 0);
		return;
	}
	if (weapon->what_is != WEAPON) {
		wch = get_rand(0, 2);

		switch(wch) {
		case 0:
			message("if you don't want it, drop it!", 0);
			break;
		case 1:
			message("throwing that would do noone any good", 0);
			break;
		case 2:
			message("why would you want to throw that?", 0);
			break;
		}
		return;
	}
	if ((weapon == rogue.weapon) && (weapon->is_cursed)) {
		message(CURSE_MESSAGE, 0);
		return;
	}
	row = rogue.row; col = rogue.col;
	monster = get_thrown_at_monster(dir, &row, &col);
	mvaddch(rogue.row, rogue.col, rogue.fchar);
	refresh_vms();

	if (can_see(row, col) && ((row != rogue.row) || (col != rogue.col))) {
		mvaddch(row, col, get_room_char(screen[row][col], row, col));
	}
	if (monster) {
		wake_up(monster);
		check_orc(monster);

		if (!throw_at_monster(monster, weapon)) {
			f = flop_weapon(weapon, row, col);
		}
	} else {
		f = flop_weapon(weapon, row, col);
	}
	vanish(weapon, 1);
}

throw_at_monster(monster, weapon)
object *monster, *weapon;
{
	short damage, hit_chance;
	char *name_of();
	short t;

	hit_chance = get_hit_chance(weapon);
	t = weapon->quantity;
	weapon->quantity = 1;
	sprintf(hit_message, "the %s", name_of(weapon));
	weapon->quantity = t;

	if (!rand_percent(hit_chance)) {
		strcat(hit_message, "misses  ");
		return(0);
	}
	strcat(hit_message, "hit  ");
	damage = get_weapon_damage(weapon);
	if (((weapon->which_kind == ARROW) && (rogue.weapon &&
	   ((rogue.weapon->which_kind == SHORT_BOW)||
	    (rogue.weapon->which_kind==LONG_BOW)))) ||
	   ((weapon->which_kind == SPEAR) && (rogue.weapon == weapon))||
	   ((weapon->which_kind == JAVELIN) && (rogue.weapon == weapon))||
	   ((weapon->which_kind == HAMMER) && (rogue.weapon == weapon)) ||
	   ((weapon->which_kind == SHURIKEN) && (rogue.weapon == weapon))) {
		damage += get_weapon_damage(rogue.weapon);
		damage = ((damage * 2) / 3);
	}
	monster_damage(monster, damage);
	return(1);
}

object *get_thrown_at_monster(dir, row, col)
short dir;
short *row, *col;
{
	object *object_at();
	short orow, ocol;
	short i;

	orow = *row; ocol = *col;

	for (i = 0; i < 24; i++) {
		get_dir_rc(dir, row, col);
		if ((screen[*row][*col] == BLANK) ||
		     (screen[*row][*col] & (HORWALL | VERTWALL))) {
			*row = orow;
			*col = ocol;
			return(0);
		}
		if ((i != 0) && can_see(orow, ocol)) {
			mvaddch(orow, ocol, get_room_char(screen[orow][ocol],
				orow, ocol));
		}
		if (can_see(*row, *col)) {
			if (!(screen[*row][*col] & MONSTER)) {
				mvaddch(*row, *col, ')');
			}
			refresh_vms();
		}
		orow = *row; ocol = *col;
		if (screen[*row][*col] & MONSTER) {
			if (!hiding_xeroc(*row, *col)) {
				return(object_at(&level_monsters, *row, *col));
			}
		}
		if (screen[*row][*col] & TUNNEL) {
			i += 2;
		}
	}
	return(0);
}

flop_weapon(weapon, row, col)
object *weapon;
short row, col;
{
	object *new_weapon, *get_an_object();
	short i, j, r, c, found = 0, inc1, inc2;
	char msg[80];
	char *name_of();
	inc1 = get_rand(0, 1) ? 1 : -1;
	inc2 = get_rand(0, 1) ? 1 : -1;

	r = row;
	c = col;

	if ((screen[r][c] & ~(FLOOR | TUNNEL | DOOR)) ||
	    ((row == rogue.row) && (col == rogue.col))) {
		for (i = inc1; i != (2 * -inc1); i -= inc1) {
			for (j = inc2; j != (2 * -inc2); j -= inc2) {
				r = row + i;
				c = col + j;

				if ((r > (LINES-2)) || (r < MIN_ROW) ||
				    (c > (COLS-1)) || (c < 0)) {
					continue;
				}
				if (!screen[r][c]) continue;
				if ((screen[r][c] & ~(FLOOR|TUNNEL|DOOR))
				|| ((r == rogue.row) && (c == rogue.col))) {
					continue;
				}
				found = 1;
				goto FOUND;
			}
		}
	} else {
		found = 1;
	}
FOUND:  if (found) {
		new_weapon = get_an_object();
		*new_weapon = *weapon;
		new_weapon->quantity = 1;
		new_weapon->row = r;
		new_weapon->col = c;
		add_mask(r, c, WEAPON);
		add_to_pack(new_weapon, &level_objects, 0);
		if (can_see(r, c)) {
			mvaddch(r, c, get_room_char(screen[r][c], r, c));
		}
	} else {
		short t;

		t = weapon->quantity;
		weapon->quantity = 1;
		sprintf(msg, "the %svanishes as it hits the ground",
		name_of(weapon));
		weapon->quantity = t;
		message(msg, 0);
	}
	return(found);
}
