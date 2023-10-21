#include curses.h
#include "object.h"
#include "move.h"
#include "monster.h"

short being_held;

extern short current_level, max_level, has_amulet, detect_monster, halluc;
extern int level_points[];

special_hit(monster)
object *monster;
{
   if ((current_level >= monster->is_protected) &&
       (current_level <= monster->is_cursed))
   {
	switch(monster->ichar) {
	case 'A':
		if ((current_level >= AQUATAR1) &&
		    (current_level <= AQUATAR2)) rust(monster);
		break;
	case 'F':
		if ((current_level >= FLYTRAP1) &&
		    (current_level <= FLYTRAP2)) being_held = 1;
		break;
	case 'I':
		if ((current_level >= ICEMSTR1) &&
		    (current_level <= ICEMSTR2)) freeze(monster);
		break;
	case 'L':
		if ((current_level >= LEPCHAN1) &&
		    (current_level <= LEPCHAN2)) steal_gold(monster);
		break;
	case 'N':
		if ((current_level >= NYMPH1) &&
		    (current_level <= NYMPH2)) steal_item(monster);
		break;
	case 'R':
		if ((current_level >= RSNAKE1) &&
		    (current_level <= RSNAKE2)) sting(monster);
		break;
	case 'V':
		if ((current_level >= VAMPIRE1) &&
		    (current_level <= VAMPIRE2)) drain_life();
		break;
	case 'W':
		if ((current_level >= WRAITH1) &&
		    (current_level <= WRAITH2)) drain_level();
		break;
	case 's':
		if ((current_level >= SPIDER1) &&
		    (current_level <= SPIDER2)) acid_trip(monster);
		break;
	}
   }
}

rust(monster)
object *monster;
{
	if (!rogue.armor || (get_armor_class(rogue.armor) <= 1) ||
	    (rogue.armor->which_kind == LEATHER)) {
		return;
	}
	if (rogue.armor->is_protected) {
		if (!monster->identified) {
			message("the rust vanishes instantly", 0);
			monster->identified = 1;
		}
	} else {
		rogue.armor->damage_enchantment--;
		message("your armor weakens", 0);
		print_stats();
	}
}
acid(monster)
object *monster;
{
	message("the dragon spits acid upon you",0);
	if (rogue.armor->is_protected && (!(rand()%12)) ) {
		if (!monster->identified) {
			message("the acid has no effect",0);
			monster->identified = 1;
		}
	} else {
		rogue.armor->damage_enchantment--;
		message("your armor weakens", 0);
		print_stats();
	}
}

freeze(monster)
object *monster;
{
	short freeze_percent = 99;
	short i, n;

	if (rand_percent(12)) return;

	freeze_percent -= (rogue.strength_current+(rogue.strength_current/2));
	freeze_percent -= rogue.exp * 4;
	freeze_percent -= (rogue.armor->damage_enchantment * 5);
	freeze_percent -= (rogue.hp_max / 3);

	if (freeze_percent > 10) {
		monster->identified = 1;
		message("you are frozen", 1);

		n = get_rand(5, 9);
		for (i = 0; i < n; i++) {
			move_monsters();
		}
		if (rand_percent(freeze_percent)) {
			for (i = 0; i < 50; i++) {
				move_monsters();
			}
			killed_by((object *)0, HYPOTHERMIA);
		}
		message("you can move again", 1);
		monster->identified = 0;
	}
}

steal_gold(monster)
object *monster;
{
	int amount;

	if (rand_percent(15)) return;

	if (rogue.gold > 50) {
		amount = rogue.gold > 1000 ? get_rand(8, 15) : get_rand(2, 5);
		amount = rogue.gold / amount;
	} else {
		amount = rogue.gold/2;
	}
	amount += (get_rand(0, 2) - 1) * (rogue.exp + current_level);
	if ((amount <= 0) && (rogue.gold > 0)) {
		amount = rogue.gold;
	}

	if (amount > 0) {
		rogue.gold -= amount;
		message("your purse feels lighter", 0);
		print_stats();
	}
	disappear(monster);
}

steal_item(monster)
object *monster;
{
	object *obj;
	short i, n, has_something = 0;
	char description[80];

	if (rand_percent(15)) return;

	obj = rogue.pack.next_object;

	if (!obj) {
		goto DSPR;
	}
	while (obj) {
		if ((obj != rogue.armor) && (obj != rogue.weapon)) {
			has_something = 1;
			break;
		}
		obj = obj->next_object;
	}
	if (!has_something) goto DSPR;

	n = get_rand(0, MAX_PACK_COUNT);
	obj = rogue.pack.next_object;

	for (i = 0; i <= n; i++) {
		obj = obj->next_object;
		while (!obj || (obj == rogue.weapon) || (obj == rogue.armor)) {
			if (!obj) {
				obj = rogue.pack.next_object;
			} else {
				obj = obj->next_object;
			}
		}
	}
	strcpy(description, "she stole ");
	get_description(obj, description+10);
	message(description, 0);

	if (obj->what_is == AMULET) {
		has_amulet = 0;
	}
	vanish(obj, 0);
DSPR:	disappear(monster);
}

disappear(monster)
object *monster;
{
	short row, col;

	row = monster->row;
	col = monster->col;

	remove_mask(row, col, MONSTER);
	if (can_see(row, col)) {
		mvaddch(row, col, get_room_char(screen[row][col], row, col));
	}
	remove_from_pack(monster, &level_monsters);
	free(monster);
}

cough_up(monster)
object *monster;
{
	object *obj, *get_an_object(),*get_rand_object();
	short row, col, i, j, n;

	if (current_level < max_level) {
		return;
	}

	switch(monster->ichar) {
	case 'L':
		obj = get_an_object();
		obj->what_is = GOLD;
		obj->quantity = get_rand(9, 599);
		break;
	default:
		if (rand_percent(monster->which_kind)) {
			obj = get_rand_object();
			break;
		}
		return;
	}
	row = monster->row;
	col = monster->col;

	for (n = 0; n <= 5; n++) {
		for (i = -n; i <= n; i++) {
			if (try_to_cough(row+n, col+i, obj)) {
				return;
			}
			if (try_to_cough(row-n, col+i, obj)) {
				return;
			}
		}
		for (i = -n; i <= n; i++) {
			if (try_to_cough(row+i, col-n, obj)) {
				return;
			}
			if (try_to_cough(row+i, col+n, obj)) {
				return;
			}
		}
	}
	free(obj);
}

try_to_cough(row, col, obj)
short row, col;
object *obj;
{
	if ((row<MIN_ROW)||(row>(LINES-2))||(col<0)||(col>(COLS-1))) {
		return(0);
	}
	if ((!(screen[row][col] & IS_OBJECT)) &&
	    (screen[row][col] & (TUNNEL|FLOOR|DOOR)) &&
	    !(screen[row][col] & MONSTER)) {
		put_object_at(obj, row, col);
		mvaddch(row, col, get_room_char(screen[row][col], row, col));
		refresh_vms();
		return(1);
	}
	return(0);
}

orc_gold(monster)
object *monster;
{
	short i, j, row, col, rn, s;

	if (monster->identified) {
		return(0);
	}
	if ((rn = get_room_number(monster->row, monster->col)) < 0) {
		return(0);
	}
	for (i = rooms[rn].top_row+1;
	i < rooms[rn].bottom_row; i++) {
		for (j = rooms[rn].left_col+1;
		j < rooms[rn].right_col; j++) {
			if ((screen[i][j] & GOLD) &&
			   !(screen[i][j] & MONSTER)) {
				monster->m_flags |= CAN_GO;
				s = monster_can_go(monster, i, j);
				monster->m_flags &= (~CAN_GO);
				if (s) {
					move_monster_to(monster, i, j);
					monster->m_flags |= IS_ASLEEP;
					monster->m_flags &= (~WAKENS);
					monster->identified = 1;
					return(1);
				}
				monster->identified = 1;
				monster->m_flags |= CAN_GO;
				mv_monster(monster, i, j);
				monster->m_flags &= (~CAN_GO);
				monster->identified = 0;
				return(1);
			}
		}
	}
	return(0);
}

check_orc(monster)
object *monster;
{
	if (monster->ichar == 'O') {
		monster->identified = 1;
	}
}

check_xeroc(monster)
object *monster;
{
	char *monster_name();
	char msg[80];

	if ((monster->ichar == 'X') && monster->identified) {
		wake_up(monster);
		monster->identified = 0;
		mvaddch(monster->row, monster->col,
		get_room_char(screen[monster->row][monster->col],
		monster->row, monster->col));
		check_message();
		sprintf(msg, "wait, that's a %s!", monster_name(monster));
		message(msg, 1);
		return(1);
	}
	return(0);
}

hiding_xeroc(row, col)
register short row, col;
{
	object *object_at(), *monster;

	if ((current_level < XEROC1) || (current_level > XEROC2) ||
	    !(screen[row][col] & MONSTER)) {
		return(0);
	}
	monster = object_at(&level_monsters, row, col);

	if ((monster->ichar == 'X') && monster->identified) {
		return(1);
	}
	return(0);
}

sting(monster)
object *monster;
{
	short ac, sting_chance = 35;
	char msg[80], *monster_name();

	if (rogue.strength_current < 5) return;

	ac = get_armor_class(rogue.armor);

	sting_chance += (6 * (6 - ac));

	if (rogue.exp > 8) {
		sting_chance -= (6 * (rogue.exp - 8));
	}

	if (sting_chance > 100) {
		sting_chance = 100;
	} else if (sting_chance <= 0) {
		sting_chance = 1;
	}
	if (rand_percent(sting_chance)) {
		sprintf(msg, "the %s's bite has weakened you",
		monster_name(monster));
		message(msg, 0);
		rogue.strength_current--;
		print_stats();
	}
}

drain_level()
{
	short hp;
	if (!rand_percent(20) || (rogue.exp <= 7)) {
		return;
	}
	rogue.exp_points = level_points[rogue.exp-2] - get_rand(10, 50);
	rogue.exp -= 2;
	add_exp(1);
	hp = get_rand(3,10);
	rogue.hp_max += hp;
	if (rogue.hp_current > hp + 1) rogue.hp_current -= hp;
}

drain_life()
{
	if (!rand_percent(25) || (rogue.hp_max<=30) || (rogue.hp_current<10)) {
		return;
	}
	message("you feel weaker", 0);
	rogue.hp_max--;
	rogue.hp_current--;
	if (rand_percent(50)) {
		if (rogue.strength_current >= 5) {
			rogue.strength_current--;
			if (rand_percent(50)) {
				rogue.strength_max--;
			}
		}
	}
	print_stats();
}

m_confuse(monster)
object *monster;
{
	char msg[80];
	char *monster_name();

	if (monster->identified) {
		return(0);
	}
	if (!can_see(monster->row, monster->col)) {
		return(0);
	}
	if (rand_percent(45)) {
		monster->identified = 1;
		return(0);
	}
	if (rand_percent(65)) {
		monster->identified = 1;
		sprintf(msg, "the gaze of the %s has confused you",
		monster_name(monster));
		message(msg, 1);
		confuse();
		return(1);
	}
	return(0);
}

paralyze(monster)
object *monster;
{
   short i;
   char msg[80];
   char *monster_name();

   if (monster->identified) return(0);
   if (!can_see(monster->row,monster->col)) return(0);
   if (rand_percent(50))
   {
	monster->identified = 1;
	return(0);
   }
   if (rand_percent(50))
   {
	monster->identified = 1;
	sprintf(msg,"the gaze of the %s has you paralyzed with fear",
		monster_name(monster));
	message(msg,1);
	i = get_rand(2,6);
	while (i--)
	{
	   move_monsters();
	}
	sleep(1);
	message("you feel sanity return",0);
	return (1);
   }
}


acid_trip(monster)
object *monster;
{
   short ac, sting_chance = 40;
   char msg[80], *monster_name();

   if (monster->identified) return (0);

   ac = get_armor_class(rogue.armor);

   sting_chance += (6 * (6 - ac));

   if (rogue.exp > 8) {
	sting_chance -= (6 * (rogue.exp - 8));
   }

   if (sting_chance > 100) {
	sting_chance = 100;
   } else if (sting_chance <= 0) {
	sting_chance = 1;
   }
   if (rand_percent(sting_chance))
   {
	monster->identified = 1;
	sprintf(msg,"the %s's bite leaves you feeling SO funky",
					  monster_name(monster)); 
	message(msg, 1);
	halluc += get_rand(100,300);
	return (1);
   }
}


breath(monster,oder)
object *monster;
int oder;
{
	short row, col;
	char breath_char;

	if (!can_see(monster->row, monster->col)) {
		return(0);
	}
	if (rand_percent(50)) {
		return(0);
	}
	if (!rogue_is_around(monster->row, monster->col)) {

		row = monster->row; col = monster->col;
		get_closer(&row, &col, rogue.row, rogue.col);
		standout();
	
		switch(oder){

		case B_FLAME : breath_char='*';
				break;
		case B_ACID  : breath_char='>';
				break;
		case B_SLEEP : breath_char='<';
				break;
		case B_CONFUSION : breath_char='"';
				break;
		case B_COLD  : breath_char='^';
				break;
		case B_LIGHTNING : breath_char='~';
				break;
		}

		do {
			mvaddch(row, col, breath_char);
			refresh_vms();
			get_closer(&row, &col, rogue.row, rogue.col);
		} while ((row != rogue.row) || (col != rogue.col));
		standend();
		row = monster->row; col = monster->col;
		get_closer(&row, &col, rogue.row, rogue.col);
		do {
			mvaddch(row, col, get_room_char(screen[row][col],
				row, col));
			refresh_vms();
			get_closer(&row, &col, rogue.row, rogue.col);
		} while ((row != rogue.row) || (col != rogue.col));
	}
		switch(oder){

		case B_FLAME : 
			monster_hit(monster, "flame");
		break;
		case B_ACID  : 
			monster_hit(monster, "acid");
		break;
		case B_SLEEP : 
			monster_hit(monster, "sleeping gas");
			if((rand()%5)<2) sleep_scroll();
		break;
		case B_CONFUSION :
			monster_hit(monster, "nerve gas");
			if((rand()%5)<3) confuse();
		break;
		case B_COLD  : 
			monster_hit(monster, "ice");
			if((rand()%5)>3) freeze(monster);
		break;
		case B_LIGHTNING :
			monster_hit(monster, "lightning");
		break;
		}
	return(1);
}

get_closer(row, col, trow, tcol)
short *row, *col;
short trow, tcol;
{
	if (*row < trow) {
		(*row)++;
	} else if (*row > trow) {
		(*row)--;
	}
	if (*col < tcol) {
		(*col)++;
	} else if (*col > tcol) {
		(*col)--;
	}
}
