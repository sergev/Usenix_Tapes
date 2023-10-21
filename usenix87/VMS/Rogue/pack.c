#include curses.h
#include "move.h"
#include "object.h"

char *CURSE_MESSAGE = "you can't, it appears to be cursed";
char ichars[26];

extern short has_amulet;

object *add_to_pack(obj, pack, condense)
object *obj, *pack;
{
	object *op, *check_duplicate();

	if (condense) {
		if (op = check_duplicate(obj, pack)) {
			free(obj);
			return(op);
		} else {
			obj->ichar = next_avail_ichar();
		}
	}

	if (pack->next_object == 0) {
		pack->next_object = obj;
	} else {
		op = pack->next_object;

		while (op->next_object) {
			op = op->next_object;
		}
		op->next_object = obj;
	}
	obj->next_object = 0;
	return(obj);
}

remove_from_pack(obj, pack)
object *obj, *pack;
{
	while (pack->next_object != obj) {
		pack = pack->next_object;
	}
	pack->next_object = pack->next_object->next_object;
}

object *pick_up(row, col, status)
short *status;
{
	object *obj, *object_at(), *add_to_pack();

	obj = object_at(&level_objects, row, col);
	*status = 1;

	if ((obj->what_is == SCROLL) && (obj->which_kind == SCARE_MONSTER) &&
	    (obj->picked_up > 0)) {
		message("the scroll turns to dust as you pick it up", 1);
		remove_from_pack(obj, &level_objects);
		remove_mask(row, col, SCROLL);
		free(obj);
		*status = 0;
		if (id_scrolls[SCARE_MONSTER].id_status == UNIDENTIFIED) {
			id_scrolls[SCARE_MONSTER].id_status = IDENTIFIED;
		}
		return(0);
	}
	if (obj->what_is == GOLD) {
		rogue.gold += obj->quantity;
		remove_mask(row, col, GOLD);
		remove_from_pack(obj, &level_objects);
		print_stats();
		return(obj);	/* obj will be free()ed in single_move() */
	}
	if (get_pack_count(obj) >= MAX_PACK_COUNT) {
		message("Pack too full", 1);
		return(0);
	}
	if (obj->what_is == AMULET) {
		has_amulet = 1;
	}

	remove_mask(row, col, obj->what_is);
	remove_from_pack(obj, &level_objects);
	obj = add_to_pack(obj, &rogue.pack, 1);
	obj->picked_up++;
	return(obj);
}

drop()
{
	object *obj, *new;
	object *get_letter_object();
	short ch;
	object *get_an_object();
	char description[SCOLS];

	if (screen[rogue.row][rogue.col] & IS_OBJECT) {
		message("There's already something there", 0);
		return;
	}
	if (!rogue.pack.next_object) {
		message("You have nothing to drop", 0);
		return;
	}
	ch = get_pack_letter("drop what? ", IS_OBJECT);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("No such item.", 0);
		return;
	}
	if (obj == rogue.weapon) {
		if (obj->is_cursed) {
			message(CURSE_MESSAGE, 0);
			return;
		}
		rogue.weapon = 0;
	} else if (obj == rogue.armor) {
		if (obj->is_cursed) {
			message(CURSE_MESSAGE, 0);
			return;
		}
		rogue.armor = 0;
		print_stats();
	}
	obj->row = rogue.row;
	obj->col = rogue.col;

	if ((obj->quantity > 1) && (obj->what_is != WEAPON)) {
		obj->quantity--;
		new = get_an_object();
		*new = *obj;
		new->quantity = 1;
		obj = new;
		goto ADD;
	}
	if (obj->what_is == AMULET) {
		has_amulet = 0;
	}
	make_avail_ichar(obj->ichar);
	remove_from_pack(obj, &rogue.pack);
ADD:	add_to_pack(obj, &level_objects, 0);
	add_mask(rogue.row, rogue.col, obj->what_is);
	strcpy(description, "dropped ");
	get_description(obj, description+8);
	message(description, 0);
	register_move();
}

object *check_duplicate(obj, pack)
object *obj, *pack;
{
	object *op;

	if (!(obj->what_is & (WEAPON | FOOD | SCROLL | POTION))) {
		return(0);
	}
	op = pack->next_object;

	while (op) {
		if ((op->what_is == obj->what_is) && 
		    (op->which_kind == obj->which_kind)) {

			if ((obj->what_is != WEAPON) ||
			((obj->what_is == WEAPON) &&
			((obj->which_kind == ARROW) ||
			(obj->which_kind == SHURIKEN)) &&
			(obj->quiver == op->quiver))) {
				op->quantity += obj->quantity;
				return(op);
			}
		}
		op = op->next_object;
	}
	return(0);
}

next_avail_ichar()
{
	short i;

	for (i = 'a'; i < 'z'; i++) {
		if (!ichars[i-'a']) {
			ichars[i-'a'] = 1;
			return(i);
		}
	}
	return(0);
}

make_avail_ichar(ch)
{
	ichars[ch - 'a'] = 0;
}

wait_for_ack(prompt)
{
	char getchartt();
	if (prompt) {
		printf("%s ", MORE);
		fflush(stdout);
	}
	while (getchartt() != ' ') ;
}

get_pack_letter(prompt, mask)
char *prompt;
unsigned short mask;
{
	char getchartt();
	short first_miss = 1;
	short ch;

	message(prompt, 0);
	while (!is_pack_letter(ch = getchartt())) {
		putchar(7);
		fflush(stdout);
		if (first_miss) {
WHICH:			message(prompt, 0);
			first_miss = 0;
		}
	}
	if (ch == LIST) {
		check_message();
		inventory(&rogue.pack, mask);
		goto WHICH;
	}
	check_message();
	return(ch);
}

take_off()
{
	char description[SCOLS];
	object *obj;

	if (rogue.armor) {
		if (rogue.armor->is_cursed) {
			message(CURSE_MESSAGE, 0);
		} else {
			mv_aquatars();
			obj = rogue.armor;
			rogue.armor = 0;
			strcpy(description, "was wearing ");
			get_description(obj, description+12);
			message(description, 0);
			print_stats();
			register_move();
		}
	} else {
		message("not wearing any", 0);
	}
}

wear()
{
	short ch;
	register object *obj;
	char description[SCOLS];
	object *get_letter_object();

	if (rogue.armor) {
		message("your already wearing some", 0);
		return;
	}
	ch = get_pack_letter("wear what? ", ARMOR);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("No such item.", 0);
		return;
	}
	if (obj->what_is != ARMOR) {
		message("You can't wear that", 0);
		return;
	}
	rogue.armor = obj;
	obj->identified = 1;
	get_description(obj, description);
	message(description, 0);
	print_stats();
	register_move();
}

wield()
{
	short ch;
	register object *obj;
	char description[SCOLS];
	object *get_letter_object();

	if (rogue.weapon && rogue.weapon->is_cursed) {
		message(CURSE_MESSAGE, 0);
		return;
	}
	ch = get_pack_letter("wield what? ", WEAPON);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("No such item.", 0);
		return;
	}
	if (obj->what_is != WEAPON) {
		message("You can't wield that", 0);
		return;
	}
	if (obj == rogue.weapon) {
		message("in use", 0);
	} else {
		rogue.weapon = obj;
		get_description(obj, description);
		message(description, 0);
		register_move();
	}
}

call_it()
{
	short ch;
	register object *obj;
	struct identify *id_table, *get_id_table();
	char buf[MAX_TITLE_LENGTH+2];
	object *get_letter_object();

	ch = get_pack_letter("call what? ", (SCROLL | POTION | WAND));

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("No such item.", 0);
		return;
	}
	if (!(obj->what_is & (SCROLL | POTION | WAND))) {
		message("surely you already know what that's called", 0);
		return;
	}
	id_table = get_id_table(obj);

	if (get_input_line(buf, id_table[obj->which_kind].title)) {
		id_table[obj->which_kind].id_status = CALLED;
		strcpy(id_table[obj->which_kind].title, buf);
	}
}

get_pack_count(new_obj)
object *new_obj;
{
	object *obj;
	short count = 0;

	if (!(obj = rogue.pack.next_object)) {
		return(0);
	}
	while (obj) {
		if (obj->what_is != WEAPON) {
			count += obj->quantity;
		} else {
			if ((new_obj->what_is != WEAPON) ||
			    ((obj->which_kind != ARROW) &&
			    (obj->which_kind != SHURIKEN)) ||
			    (new_obj->which_kind != obj->which_kind) ||
			    (obj->quiver != new_obj->quiver)) {
				count++;
			}
		}
		obj = obj->next_object;
	}
	return(count);
}
