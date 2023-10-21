#include curses.h
#include "object.h"
#include "room.h"

object level_objects;
unsigned short screen[SROWS][SCOLS];

extern short current_level, max_level;
extern short party_room;

short has_amulet = 0;
short foods = 0;

fighter rogue = {
	0, 0,		/* armor, weapon */
	12, 12,		/* Hp */
	16, 16,		/* Str */
	{0},		/* pack */
	0,		/* gold */
	1, 0,		/* exp, exp_points */
	0, 0,		/* row, col */
	'@',		/* char */
	1200		/* moves */
};

struct identify id_potions[POTIONS] = {
{100, "blue \0                           ", "of increase strength ",0},
{250, "red \0                            ", "of restore strength ",0},
{100, "green \0                          ", "of healing ",0},
{200, "grey \0                           ", "of extra healing ",0},
 {10, "brown \0                          ", "of poison ",0},
{300, "clear \0                          ", "of raise level ",0},
 {10, "pink \0                           ", "of blindness ",0},
 {25, "white \0                          ", "of hallucination ",0},
{100, "purple \0                         ", "of detect monster ",0},
{100, "black \0                          ", "of detect things ",0},
 {10, "yellow \0                         ", "of confusion ",0}
};

struct identify id_scrolls[SCROLLS] = {
{505, "                                   ", "of protect armor ", 0},
{200, "                                   ", "of hold monster ", 0},
{235, "                                   ", "of enchant weapon ", 0},
{235, "                                   ", "of enchant armor ", 0},
{175, "                                   ", "of identify ", 0},
{190, "                                   ", "of teleportation ", 0},
 {25, "                                   ", "of sleep ", 0},
{610, "                                   ", "of scare monster ", 0},
{210, "                                   ", "of remove curse ", 0},
{100, "                                   ", "of create monster ",0},
 {25, "                                   ", "of aggravate monster ",0}
};

struct identify id_weapons[WEAPONS] = {
	{150, "short bow ", "", 0},
	 {15, "arrows ", "", 0},
	 {35, "shurikens ", "", 0},
	{370, "mace ", "", 0},
	{480, "long sword ", "", 0},
	{590, "two-handed sword ", "", 0}
};

struct identify id_armors[ARMORS] = {
	{300, "leather armor ", "", (UNIDENTIFIED)},
	{300, "ring mail ", "", (UNIDENTIFIED)},
	{400, "scale mail ", "", (UNIDENTIFIED)},
	{500, "chain mail ", "", (UNIDENTIFIED)},
	{600, "banded mail ", "", (UNIDENTIFIED)},
	{600, "splint mail ", "", (UNIDENTIFIED)},
	{700, "plate mail ", "", (UNIDENTIFIED)}
};

struct identify id_wands[WANDS] = {
	 {25, "                                 ", "of teleport away ",0},
	 {50, "                                 ", "of slow monster ", 0},
	 {45, "                                 ", "of kill monster ",0},
	  {8, "                                 ", "of invisibility ",0},
	 {55, "                                 ", "of polymorph ",0},
	  {2, "                                 ", "of haste monster ",0},
	 {25, "                                 ", "of put to sleep ",0},
	  {0, "                                 ", "of do nothing ",0}
};

put_objects()
{
	short row, col, i, n;
	char *malloc();
	object *obj, *get_rand_object();

	if (current_level < max_level) return;

	n = get_rand(2, 4);
	if (rand_percent(35)) n++;

	if (rand_percent(50)) {
		strcpy(id_weapons[SHURIKEN].title, "daggers ");
	}
	if (rand_percent(5)) {
		make_party();
	}
	for (i = 0; i < n; i++) {
		obj = get_rand_object();
		put_object_rand_location(obj);
		add_to_pack(obj, &level_objects, 0);
	}
	put_gold();
}

put_gold()
{
	short i, j;
	short row,col;
	object *obj;

	for (i = 0; i < MAXROOMS; i++) {
		if (rooms[i].is_room && rand_percent(GOLD_PERCENT)) {
			for (j = 0; j < 25; j++) {
				row = get_rand(rooms[i].top_row+1,
				rooms[i].bottom_row-1);
				col = get_rand(rooms[i].left_col+1,
				rooms[i].right_col-1);
				if ((screen[row][col] == FLOOR) ||
				    (screen[row][col] == PASSAGE)) {
					put_gold_at(row, col);
					break;
				}
			}
		continue;
		}
	}
}

put_gold_at(row, col)
{
	object *obj;
        object *get_an_object();

	obj = get_an_object();
	obj->row = row; obj->col = col;
	obj->what_is = GOLD;
	obj->quantity = get_rand((2*current_level), (16*current_level));
	add_mask(row, col, GOLD);
	add_to_pack(obj, &level_objects, 0);
}

put_object_at(obj, row, col)
object *obj;
{
	obj->row = row;
	obj->col = col;
	add_mask(row, col, obj->what_is);
	add_to_pack(obj, &level_objects, 0);
}

object *object_at(pack, row, col)
object *pack;
{
	object *obj;

	obj = pack->next_object;

	while (obj && (obj->row != row) || (obj->col != col)) {
		obj = obj->next_object;
	}
	return(obj);
}

object *get_letter_object(ch)
{
	object *obj;

	obj = rogue.pack.next_object;

	while (obj && (obj->ichar != ch)) {
		obj = obj->next_object;
	}
	return(obj);
}

free_stuff(objlist)
object *objlist;
{
	object *obj;

	while (objlist->next_object) {
		obj = objlist->next_object;
		objlist->next_object =
			objlist->next_object->next_object;
		free(obj);
	}
}

char *name_of(obj)
object *obj;
{
	char *retstring;

	switch(obj->what_is) {
	case SCROLL:
		retstring = obj->quantity > 1 ? "scrolls " : "scroll ";
		break;
	case POTION:
		retstring = obj->quantity > 1 ? "potions " : "potion ";
		break;
	case FOOD:
		retstring = obj->quantity > 1 ? "rations " : "ration ";
		break;
	case WAND:
		retstring = "wand ";
		break;
	case WEAPON:
		switch(obj->which_kind) {
		case ARROW:
			retstring=obj->quantity > 1 ? "arrows " : "arrow ";
			break;
		case SHURIKEN:
			if (id_weapons[SHURIKEN].title[0] == 'd') {
			retstring=obj->quantity > 1 ? "daggers " : "dagger ";
			} else {
			retstring=obj->quantity > 1?"shurikens ":"shuriken ";
			}
			break;
		default:
			retstring = id_weapons[obj->which_kind].title;
		}
		break;
	default:
		retstring = "unknown ";
		break;
	}
	return(retstring);
}

object *get_rand_object()
{
	object *obj, *get_an_object();

	obj = get_an_object();

	if (foods < (current_level/2)) {
		obj->what_is = FOOD;
	} else {
		obj->what_is = get_rand_what_is();
	}
	obj->identified = 0;

	switch(obj->what_is) {
	case SCROLL:
		get_rand_scroll(obj);
		break;
	case POTION:
		get_rand_potion(obj);
		break;
	case WEAPON:
		get_rand_weapon(obj);
		break;
	case ARMOR:
		get_rand_armor(obj);
		break;
	case WAND:
		get_rand_wand(obj);
		break;
	case FOOD:
		foods++;
		get_food(obj);
		break;
	}
	return(obj);
}

get_rand_what_is()
{
	short percent;
	short what_is;

	percent = get_rand(1, 92);

	if (percent <= 30) {
		what_is = SCROLL;
	} else if (percent <= 60) {
		what_is = POTION;
	} else if (percent <= 65) {
		what_is = WAND;
	} else if (percent <= 75) {
		what_is = WEAPON;
	} else if (percent <= 85) {
		what_is = ARMOR;
	} else {
		what_is = FOOD;
	}
	return(what_is);
}

get_rand_scroll(obj)
object *obj;
{
	short percent;

	percent = get_rand(0, 82);

	if (percent <= 5) {
		obj->which_kind = PROTECT_ARMOR;
	} else if (percent <= 11) {
		obj->which_kind = HOLD_MONSTER;
	} else if (percent <= 20) {
		obj->which_kind = CREATE_MONSTER;
	} else if (percent <= 35) {
		obj->which_kind = IDENTIFY;
	} else if (percent <= 43) {
		obj->which_kind = TELEPORT;
	} else if (percent <= 52) {
		obj->which_kind = SLEEP;
	} else if (percent <= 57) {
		obj->which_kind = SCARE_MONSTER;
	} else if (percent <= 66) {
		obj->which_kind = REMOVE_CURSE;
	} else if (percent <= 71) {
		obj->which_kind = ENCHANT_ARMOR;
	} else if (percent <= 76) {
		obj->which_kind = ENCHANT_WEAPON;
	} else {
		obj->which_kind = AGGRAVATE_MONSTER;
	}
}

get_rand_potion(obj)
object *obj;
{
	short percent;

	percent = get_rand(1, 105);

	if (percent <= 5) {
		obj->which_kind = RAISE_LEVEL;
	} else if (percent <= 15) {
		obj->which_kind = DETECT_OBJECTS;
	} else if (percent <= 25) {
		obj->which_kind = DETECT_MONSTER;
	} else if (percent <= 35) {
		obj->which_kind = INCREASE_STRENGTH;
	} else if (percent <= 45) {
		obj->which_kind = RESTORE_STRENGTH;
	} else if (percent <= 55) {
		obj->which_kind = HEALING;
	} else if (percent <= 65) {
		obj->which_kind = EXTRA_HEALING;
	} else if (percent <= 75) {
		obj->which_kind = BLINDNESS;
	} else if (percent <= 85) {
		obj->which_kind = HALLUCINATION;
	} else if (percent <= 95) {
		obj->which_kind = CONFUSION;
	} else {
		obj->which_kind = POISON;
	}
}

get_rand_weapon(obj)
object *obj;
{
	short percent;
	short i;
	short blessing, cursed, increment;

	obj->which_kind = get_rand(0, (WEAPONS-1));

	if ((obj->which_kind == ARROW) || (obj->which_kind == SHURIKEN)) {
		obj->quantity = get_rand(3, 15);
		obj->quiver = get_rand(0, 126);
	} else {
		obj->quantity = 1;
	}
	obj->identified = 0;
	obj->to_hit_enchantment = obj->damage_enchantment = 0;
	get_weapon_thd(obj);

	/* notice, long swords are ALWAYS cursed or blessed */

	percent = get_rand(1, ((obj->which_kind == LONG_SWORD) ? 32 : 96));
	blessing = get_rand(1, 3);
	obj->is_cursed = 0;

	if (percent <= 16) {
		increment = 1;
	} else if (percent <= 32) {
		increment = -1;
		obj->is_cursed = 1;
	}
	if (percent <= 32) {
		for (i = 0; i < blessing; i++) {
			if (rand_percent(50)) {
				obj->to_hit_enchantment += increment;
			} else {
				obj->damage_enchantment += increment;
			}
		}
	}
	switch(obj->which_kind) {
	case BOW:
		obj->damage = "1d2";
		break;
	case ARROW:
		obj->damage = "1d2";
		break;
	case SHURIKEN:
		obj->damage = "1d4";
		break;
	case MACE:
		obj->damage = "2d3";
		break;
	case LONG_SWORD:
		obj->damage = "3d4";
		break;
	case TWO_HANDED_SWORD:
		obj->damage = "4d5";
		break;
	}
}

get_rand_armor(obj)
object *obj;
{
	short percent;
	short blessing;

	obj->which_kind = get_rand(0, (ARMORS-1));
	obj->class = obj->which_kind + 2;
	if ((obj->which_kind == PLATE) || (obj->which_kind == SPLINT)) {
		obj->class--;
	}
	obj->is_cursed = 0;
	obj->is_protected = 0;
	obj->damage_enchantment = 0;

	percent = get_rand(1, 100);
	blessing = get_rand(1, 3);

	if (percent <= 16) {
		obj->is_cursed = 1;
		obj->damage_enchantment -= blessing;
	} else if (percent <= 33) {
		obj->damage_enchantment += blessing;
	}
}

get_rand_wand(obj)
object *obj;
{
	obj->which_kind = get_rand(0, (WANDS-1));
	obj->class = get_rand(3, 7);
}

get_food(obj)
object *obj;
{
	obj->which_kind = obj->what_is = FOOD;
}

put_stairs()
{
	short row, col;

	get_rand_row_col(&row, &col, (FLOOR | TUNNEL));
	screen[row][col] = STAIRS;
}

get_weapon_thd(obj)
object *obj;
{
	switch(obj->which_kind) {
	case BOW:
		break;
	case ARROW:
		break;
	case SHURIKEN:
		break;
	case MACE:
		break;
	case LONG_SWORD:
		break;
	case TWO_HANDED_SWORD:
		break;
	}
}

get_armor_class(obj)
object *obj;
{
	if (obj) {
		return(obj->class + obj->damage_enchantment);
	}
	return(0);
}

object *get_an_object()
{
	object *obj;
	char *malloc();

	if (!(obj = (object *) malloc(sizeof(object)))) {
		clean_up("Cannot allocate item");
	}
	obj->quantity = 1;
	obj->ichar = 'L';
	obj->picked_up = 0;		/* not picked up yet */
	return(obj);
}

make_party()
{
	object *obj;
	short n;

	party_room = get_rand_room();
	n = fill_room_with_objects(party_room);
	fill_room_with_monsters(party_room, n);
}

show_objects()
{
	object *obj;

	obj = level_objects.next_object;

	while (obj) {
		mvaddch(obj->row, obj->col, get_room_char(obj->what_is));
		obj = obj->next_object;
	}
}

put_amulet()
{
	short row, col;
	object *obj, *get_an_object();

	obj = get_an_object();
	obj->what_is = AMULET;
	put_object_rand_location(obj);
	add_to_pack(obj, &level_objects, 0);
}

put_object_rand_location(obj)
object *obj;
{
	short row, col;

	get_rand_row_col(&row, &col, (FLOOR | TUNNEL));
	add_mask(row, col, obj->what_is);
	obj->row = row;
	obj->col = col;

}
