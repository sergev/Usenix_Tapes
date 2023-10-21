#include curses.h
#include "object.h"
#include "move.h"
#include "monster.h"
#include "room.h"

short halluc = 0;
short blind = 0;
short confused = 0;
short detect_monster = 0;

extern short being_held;

char *strange_feeling = "you have a strange feeling for a moment, then it passes";

extern char *hunger_str;
extern short current_room;
extern int level_points[];

quaff()
{
	short ch, i;
	object *obj, *get_letter_object();
	char msg[SCOLS];

	ch = get_pack_letter("quaff what? ", POTION);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("no such item.", 0);
		return;
	}
	if (obj->what_is != POTION) {
		message("you can't drink that", 0);
		return;
	}
	switch(obj->which_kind) {
		case INCREASE_STRENGTH:
			message("you feel stronger now, what bulging muscles!",
			0);
			rogue.strength_current++;
			if (rogue.strength_current > rogue.strength_max) {
				rogue.strength_max = rogue.strength_current;
			}
			break;
		case RESTORE_STRENGTH:
			rogue.strength_current = rogue.strength_max;
			message("this tastes great, you feel warm all over", 0);
			break;
		case HEALING:
			message("you begin to feel better", 0);
			potion_heal(0);
			break;
		case EXTRA_HEALING:
			message("you begin to feel much better", 0);
			potion_heal(1);
			break;
		case POISON:
			rogue.strength_current -= get_rand(1, 3);
			if (rogue.strength_current < 0) {
				rogue.strength_current = 0;
			}
			message("you feel very sick now", 0);
			if (halluc) {
				unhallucinate();
			}
			break;
		case RAISE_LEVEL:
			add_exp((level_points[rogue.exp-1]-rogue.exp_points)+1);
			break;
		case BLINDNESS:
			go_blind();
			break;
		case HALLUCINATION:
			message("oh wow, everything seems so cosmic", 0);
			halluc += get_rand(500, 800);
			break;
		case DETECT_MONSTER:
			if (level_monsters.next_object) {
				show_monsters();
			} else {
				message(strange_feeling, 0);
			}
			detect_monster = 1;
			break;
		case DETECT_OBJECTS:
			if (level_objects.next_object) {
				if (!blind) {
					show_objects();
				}
			} else {
				message(strange_feeling, 0);
			}
			break;
		case CONFUSION:
			message((halluc ? "what a trippy feeling" :
			"you feel confused"), 0);
			confuse();
			break;
	}
	print_stats();
	if (id_potions[obj->which_kind].id_status != CALLED) {
		id_potions[obj->which_kind].id_status = IDENTIFIED;
	}
	vanish(obj, 1);
}

read_scroll()
{
	short ch;
	object *obj, *get_letter_object();
	char msg[SCOLS];
	char *get_ench_color();

	ch = get_pack_letter("read what? ", SCROLL);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("no such item.", 0);
		return;
	}
	if (obj->what_is != SCROLL) {
		message("you can't read that", 0);
		return;
	}
	switch(obj->which_kind) {
		case SCARE_MONSTER:
			message("you hear a maniacal laughter in the distance",
			0);
			break;
		case HOLD_MONSTER:
			hold_monster();
			break;
		case ENCHANT_WEAPON:
			if (rogue.weapon) {
				sprintf(msg, "your %sglows %sfor a moment",
				id_weapons[rogue.weapon->which_kind].title,
				get_ench_color());
				message(msg, 0);
				if (get_rand(0, 1) == 1) {
					rogue.weapon->to_hit_enchantment++;
				} else {
					rogue.weapon->damage_enchantment++;
				}
				rogue.weapon->is_cursed = 0;
			} else {
				message("your hands tingle", 0);
			}
			break;
		case ENCHANT_ARMOR:
			if (rogue.armor) {
				sprintf(msg, "your armor glows %sfor a moment",
				get_ench_color());
				message(msg, 0);
				rogue.armor->damage_enchantment++;
				rogue.armor->is_cursed = 0;
				print_stats();
			} else {
				message("your skin crawls", 0);
			}
			break;
		case IDENTIFY:
			message("this is a scroll of identify", 0);
			message("what would you like to identify?", 0);
			obj->identified = 1;
			id_scrolls[obj->which_kind].id_status = IDENTIFIED;
			identify();
			break;
		case TELEPORT:
			teleport();
			break;
		case SLEEP:
			sleep_scroll();
			break;
		case PROTECT_ARMOR:
			if (rogue.armor) {
				message( "your armor is covered by a shimmering gold shield", 0);
				rogue.armor->is_protected = 1;
			} else {
				message("your acne seems to have disappeared",
				0);
			}
			break;
		case REMOVE_CURSE:
			message(
			"you feel as though someone is watching over you", 0);
			if (rogue.armor) {
				rogue.armor->is_cursed = 0;
			}
			if (rogue.weapon) {
				rogue.weapon->is_cursed = 0;
			}
			break;
		case CREATE_MONSTER:
			create_monster();
			break;
		case AGGRAVATE_MONSTER:
			aggravate();
			break;
	}
	if (id_scrolls[obj->which_kind].id_status != CALLED) {
		id_scrolls[obj->which_kind].id_status = IDENTIFIED;
	}
	vanish(obj, 1);
}

vanish(obj, rm)
object *obj;
short rm;
{
	if (obj->quantity > 1) {
		obj->quantity--;
	} else {
		remove_from_pack(obj, &rogue.pack);
		make_avail_ichar(obj->ichar);
		free(obj);
	}
	if (rm) {
		register_move();
	}
}

potion_heal(extra)
{
	float ratio;
	short add;

	ratio = ((float)rogue.hp_current) / rogue.hp_max;

	if (ratio >= 0.90) {
		rogue.hp_max += (extra + 1);
		rogue.hp_current = rogue.hp_max;
	} else {
		if (ratio < 30.00) {
			ratio = 30.00;
		}
		if (extra) {
			ratio += ratio;
		}
		add = (short)(ratio*((float)rogue.hp_max-rogue.hp_current));
		rogue.hp_current += add;
		if (rogue.hp_current > rogue.hp_max) {
			rogue.hp_current = rogue.hp_max;
		}
	}
	if (blind) {
		unblind();
	}
	if (confused && extra) {
		unconfuse();
	}
	if (confused) {
		confused = ((confused - 9) / 2);
		if (confused <= 0) {
			unconfuse();
		}
	}
	if (halluc && extra) {
		unhallucinate();
	} else if (halluc) {
		halluc = (halluc/2) + 1;
	}
}

identify()
{
	short ch;
	object *obj, *get_letter_object();
	struct identify *id_table, *get_id_table();
	char description[SCOLS];
AGAIN:
	ch = get_pack_letter("identify what? ", IS_OBJECT);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("no such item, try again", 0);
		message("", 0);
		check_message();
		goto AGAIN;
	}
	obj->identified = 1;
	if (obj->what_is & (SCROLL | POTION | WEAPON | ARMOR | WAND)) {
		id_table = get_id_table(obj);
		id_table[obj->which_kind].id_status = IDENTIFIED;
	}
	get_description(obj, description);
	message(description, 0);
}

eat()
{
	short ch;
	short moves;
	object *obj, *get_letter_object();
	char msg[SCOLS];

	ch = get_pack_letter("eat what? ", FOOD);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("no such item.", 0);
		return;
	}
	if (obj->what_is != FOOD) {
		message("you can't eat that", 0);
		return;
	}
	moves = get_rand(800, 1000);

	if (moves >= 900) {
		message("yum, that tasted good", 0);
	} else {
		message("yuk, that food tasted awful", 0);
		add_exp(3);
	}
	rogue.moves_left /= 2;
	rogue.moves_left += moves;
	hunger_str = "";
	print_stats();

	vanish(obj, 1);
}

hold_monster()
{
	short i, j;
	short mcount = 0;
	object *monster, *object_at();
	short row, col;

	for (i = -2; i <= 2; i++) {
		for (j = -2; j <= 2; j++) {
			row = rogue.row + i;
			col = rogue.col + j;
			if ((row < MIN_ROW) || (row > (LINES-2)) || (col < 0) ||
			     (col > (COLS-1))) {
				continue;
			}
			if (screen[row][col] & MONSTER) {
				monster = object_at(&level_monsters, row, col);
				monster->m_flags |= IS_ASLEEP;
				monster->m_flags &= (~WAKENS);
				mcount++;
			}
		}
	}
	if (mcount == 0) {
		message("you feel a strange sense of loss", 0);
	} else if (mcount == 1) {
		message("the monster freezes", 0);
	} else {
		message("the monsters around you freeze", 0);
	}
}

teleport()
{
	if (current_room >= 0) {
		darken_room(current_room);
	} else {
		mvaddch(rogue.row, rogue.col,
		get_room_char(screen[rogue.row][rogue.col], rogue.row,
		rogue.col));
	}
	put_player();
	light_up_room();
	being_held = 0;
}

hallucinate()
{
	object *obj;
	short ch;

	if (blind) return;

	obj = level_objects.next_object;

	while (obj) {
		move(obj->row, obj->col);
		ch = inch();
		if (((ch < 'A') || (ch > 'Z')) &&
		    ((obj->row != rogue.row) || (obj->col != rogue.col)))
		if ((ch != ' ')&&(ch != '.')&&(ch != '#')&&(ch != '+')) {
			mvaddch(obj->row, obj->col, get_rand_obj_char());
		}
		obj = obj->next_object;
	}
	obj = level_monsters.next_object;

	while (obj) {
		move(obj->row, obj->col);
		ch = inch();
		if ((ch >= 'A') && (ch <= 'Z')) {
			mvaddch(obj->row,obj->col,get_rand('A','Z'));
		}
		obj = obj->next_object;
	}
}

unhallucinate()
{
	halluc = 0;
	if (current_room == PASSAGE) {
		light_passage(rogue.row, rogue.col);
	} else {
		light_up_room();
	}
	message("everything looks SO boring now", 0);
}

unblind()
{
	blind = 0;
	message("the veil of darkness lifts", 0);
	if (current_room == PASSAGE) {
		light_passage(rogue.row, rogue.col);
	} else {
		light_up_room();
	}
	blind = 0;
	if (detect_monster) {
		show_monsters();
	}
	if (halluc) {
		hallucinate();
	}
}

sleep_scroll()
{
	short i;

	message("you fall asleep", 0);
	sleep(1);
	i = get_rand(4, 10);

	while (i--) {
		move_monsters();
	}
	sleep(1);
	message("you can move again", 0);
}

go_blind()
{
	short i, j;

	if (!blind) {
		message("a cloak of darkness falls around you", 0);
	}
	blind += get_rand(500, 800);

	if (current_room >= 0) {
		for (i = rooms[current_room].top_row + 1;
		     i < rooms[current_room].bottom_row; i++) {
			for (j = rooms[current_room].left_col + 1;
			     j < rooms[current_room].right_col; j++) {
				mvaddch(i, j, ' ');
			}
		}
	}
	mvaddch(rogue.row, rogue.col, rogue.fchar);
	refresh_vms();
}

char *get_ench_color()
{
	if (halluc) {
		return(id_potions[get_rand(0, POTIONS-1)].title);
	}
	return("blue ");
}

confuse()
{
	confused = get_rand(12, 22);
}

unconfuse()
{
	char msg[80];

	confused = 0;
	sprintf(msg, "you feel less %s now", (halluc ? "trippy" : "confused"));
	message(msg, 0);
}
