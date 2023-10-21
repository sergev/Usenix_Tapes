#include <curses.h>
#include "object.h"
#include "move.h"
#include "monster.h"

object *fight_monster = 0;
short detect_monster;
char hit_message[80] = "";

extern short halluc, blind, being_held, interrupted;

monster_hit(monster, other)
register object *monster;
char *other;
{
	short damage, hit_chance;
	char *mn, *monster_name();
	float minus;

	if (fight_monster && (monster != fight_monster)) {
		fight_monster = 0;
	}
	monster->trow = -1;
	hit_chance = monster->class;
	hit_chance -= (rogue.exp + rogue.exp);
	if (rogue.armor) hit_chance -= 3*(rogue.armor->damage_enchantment);
	if (hit_chance < 0) hit_chance = 0;

	if (!fight_monster) {
		interrupted = 1;
	}

	mn = monster_name(monster);

	if (!rand_percent(hit_chance)) {
		if (!fight_monster) {
			sprintf(hit_message + strlen(hit_message),
			"the %s misses", (other ? other : mn));
			message(hit_message, 0);
			hit_message[0] = 0;
		}
		return;
	}
	if (!fight_monster) {
		sprintf(hit_message + strlen(hit_message), "the %s hit", (other ? other : mn));
		message(hit_message, 0);
		hit_message[0] = 0;
	}

	if (monster->ichar != 'F') {
	   damage = get_damage(monster->damage, 1);
	   if (rogue.armor) minus = (float) (rogue.armor->class * 3.00 
					     + rogue.armor->damage_enchantment);
	   minus = minus/100.00 * (float) damage;
	   damage -= (short) minus;
	} else {
		damage = monster->identified++;
	}
	if (damage > 0) {
		rogue_damage(damage, monster);
	}
	special_hit(monster);
}

rogue_hit(monster)
register object *monster;
{
	short damage, hit_chance;
	char mbuf[80], *monster_name();
	float minus;
	short cx;

	if (check_xeroc(monster)) {
		return;
	}
	hit_chance = get_hit_chance(rogue.weapon);

	if (!rand_percent(hit_chance)) {
		if (!fight_monster) {
			strcpy(hit_message, "you miss  ");
		}
		goto RET;
	}
	damage = get_weapon_damage(rogue.weapon);

	if (monster_damage(monster, damage)) {	/* still alive? */
		if (!fight_monster) {
			strcpy(hit_message, "you hit  ");
		}
	}
RET:	check_orc(monster);
	wake_up(monster);
}

rogue_damage(d, monster)
short d;
object *monster;
{
extern char *player_name;
static char byebye[80];

	if (d >= rogue.hp_current) {
		rogue.hp_current = 0;
		print_stats();
		killed_by(monster, 0);
	}

	rogue.hp_current -= d;
	print_stats();
	if (rogue.hp_current <  ((short)(rogue.hp_max*0.1+1))) {
		sprintf(byebye,"%s is about to die... ", player_name);
		message(byebye, 1);
	 }
}

get_damage(ds, r)
char *ds;
{
	register short i = 0, j, n, d, total = 0;

	while (ds[i]) {
		n = get_number(ds+i);
		while (ds[i++] != 'd') ;
		d = get_number(ds+i);
		while ((ds[i] != '/') && ds[i]) i++;

		for (j = 0; j < n; j++) {
			if (r) {
				total += get_rand(1, d);
			} else {
				total += d;
			}
		}
		if (ds[i] == '/') {
			i++;
		}
	}
	return(total);
}

get_w_damage(obj)
object *obj;
{
	char new_damage[12];
	register short to_hit, damage;
	register short i = 0;

	if (!obj) {
		return(-1);
	}
	to_hit = get_number(obj->damage) + obj->to_hit_enchantment;
	while (obj->damage[i++] != 'd') ;
	damage = get_number(obj->damage + i) + obj->damage_enchantment;

	sprintf(new_damage, "%dd%d", to_hit, damage);

	return(get_damage(new_damage, 1));
}

get_number(s)
register char *s;
{
	register short i = 0;
	register short total = 0;

	while ((s[i] >= '0') && (s[i] <= '9')) {
		total = (10 * total) + (s[i] - '0');
		i++;
	}
	return(total);
}

to_hit(obj)
object *obj;
{
	short tohit = 0;
	short i = 0;

	if (!obj) {
		return(1);
	}
	return(get_number(obj->damage) + obj->to_hit_enchantment);
}

damage_for_strength(s)
{
	if (s <= 6) {
		return(s-5);
	}
	if (s <= 14) {
		return(1);
	}
	if (s <= 17) {
		return(3);
	}
	if (s <= 18) {
		return(4);
	}
	if (s <= 20) {
		return(5);
	}
	if (s <= 21) {
		return(6);
	}
	if (s <= 30) {
		return(7);
	}
	return(8);
}

monster_damage(monster, damage)
object *monster;
{
	char *mn, *monster_name();
	short row, col;

	monster->quantity -= damage;

	if (monster->quantity <= 0) {
		row = monster->row;
		col = monster->col;
		remove_mask(row, col, MONSTER);
		mvaddch(row, col, get_room_char(screen[row][col]));
		refresh_vms();

		fight_monster = 0;
		cough_up(monster);
		mn = monster_name(monster);
		sprintf(hit_message+strlen(hit_message), "defeated the %s", mn);
		message(hit_message, 1);
		hit_message[0] = 0;
		add_exp(monster->kill_exp);
		print_stats();
		remove_from_pack(monster, &level_monsters);

		if (monster->ichar == 'F') {
			being_held = 0;
		}
		free(monster);
		return(0);
	}
	return(1);
}

fight(to_the_death)
short to_the_death;
{
	short ch;
	short row, col;
	short first_miss = 1;
	short possible_damage;
	object *object_at();

	while (!is_direction(ch = getchartt())) {
		putchar(7);
		fflush(stdout);
		if (first_miss) {
			message("direction?", 0);
			first_miss = 0;
		}
	}
	check_message();
	if (ch == CANCEL) {
		return;
	}
	row = rogue.row; col = rogue.col;
	get_dir_rc(ch, &row, &col);

	if (!(screen[row][col] & MONSTER) || blind || hiding_xeroc(row, col)) {
NM:		message("I see no monster there", 0);
		return;
	}
	fight_monster = object_at(&level_monsters, row, col);
	if ((fight_monster->m_flags & IS_INVIS) && !detect_monster) {
		goto NM;
	}

	possible_damage = ((get_damage(fight_monster->damage, 0) * 2) / 3);

	while (fight_monster) {
		single_move_rogue(ch, 0);
		if (!to_the_death && rogue.hp_current <= possible_damage) {
			fight_monster = 0;
		}
		if (!(screen[row][col] & MONSTER) || interrupted) {
			fight_monster = 0;
		}
	}
}

get_dir_rc(dir, row, col)
short dir;
short *row, *col;
{
	switch(dir) {
	case 'h':
		if (*col > 0) {
			(*col)--;
		}
		break;
	case 'j':
		if (*row < (LINES-2)) {
			(*row)++;
		}
		break;
	case 'k':
		if (*row > MIN_ROW) {
			(*row)--;
		}
		break;
	case 'l':
		if (*col < (COLS-1)) {
			(*col)++;
		}
		break;
	case 'y':
		if ((*row > MIN_ROW) && (*col > 0)) {
			(*row)--;
			(*col)--;
		}
		break;
	case 'u':
		if ((*row > MIN_ROW) && (*col < (COLS-1))) {
			(*row)--;
			(*col)++;
		}
		break;
	case 'n':
		if ((*row < (LINES-2)) && (*col < (COLS-1))) {
			(*row)++;
			(*col)++;
		}
		break;
	case 'b':
		if ((*row < (LINES-2)) && (*col > 0)) {
			(*row)++;
			(*col)--;
		}
		break;
	}
}

get_hit_chance(weapon)
object *weapon;
{
	short hit_chance;

	hit_chance = 40;
	hit_chance += 3 * to_hit(weapon);
	hit_chance += (rogue.exp + rogue.exp);
	if (hit_chance > 100) hit_chance = 100;
	return(hit_chance);
}

get_weapon_damage(weapon)
object *weapon;
{
	short damage;

	damage = get_w_damage(weapon);
	damage += damage_for_strength(rogue.strength_current);
	damage += ((rogue.exp + 1) / 2);
	return(damage);
}
