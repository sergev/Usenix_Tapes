#include curses.h
#include "object.h"
#include "macros.h"
#include "move.h"

char *metals[MAXMETALS] = {
	"steel ",
	"bronze ",
	"gold ",
	"silver ",
	"copper ",
	"nickel ",
	"cobalt ",
	"tin ",
	"iron ",
	"magnesium ",
	"chrome ",
	"carbon ",
	"platinum ",
	"silicon ",
	"titanium "
};

char *syllables[MAXSYLLABLES] = {
	"blech ",
	"foo ",
	"barf ",
	"rech ",
	"bar ",
	"blech ",
	"quo ",
	"bloto ",
	"woh ",
	"caca ",
	"blorp ",
	"erp ",
	"festr ",
	"rot ",
	"slie ",
	"snorf ",
	"iky ",
	"yuky ",
	"ooze ",
	"ah ",
	"bahl ",
	"zep ",
	"druhl ",
	"flem ",
	"behil ",
	"arek ",
	"mep ",
	"zihr ",
	"grit ",
	"kona ",
	"kini ",
	"ichi ",
	"niah ",
	"ogr ",
	"ooh ",
	"ighr ",
	"coph ",
	"swerr ",
	"mihln ",
	"poxi "
};

inventory(pack, mask)
object *pack;
unsigned short mask;
{
	object *obj;
	short i = 0, j, maxlen = 0, n;
	char descriptions[MAX_PACK_COUNT+1][SCOLS];
	short row, col;

	obj = pack->next_object;
	while (obj) {
		if (obj->what_is & mask) {
			descriptions[i][0] = ' ';
			descriptions[i][1] = obj->ichar;
			descriptions[i][2] = ')';
			descriptions[i][3] = ' ';
			get_description(obj, descriptions[i]+4);
			if ((n = strlen(descriptions[i])) > maxlen) {
				maxlen = n;
			}
		i++;
		}
		obj = obj->next_object;
	}
	strcpy(descriptions[i++], " --press space to continue--");
	if (maxlen < 27) maxlen = 27;
	col = COLS - (maxlen + 2);

	for (row = 0; ((row < i) && (row < SROWS)); row++) {
		if (row > 0) {
			for (j = col; j < COLS; j++) {
				descriptions[row-1][j-col] = mvinch(row, j);
			}
			descriptions[row-1][j-col] = 0;
		}
		mvaddstr(row, col, descriptions[row]);
		clrtoeol();
	}
	refresh_vms();
	wait_for_ack(0);

	move(0, 0);
	clrtoeol();

	for (j = 1; j < i; j++) {
		mvaddstr(j, col, descriptions[j-1]);
	}
}

shuffle_colors()
{
	short i, j, k;

	for (i = 0; i <= POTIONS; i++) {
		j = get_rand(0, (POTIONS - 1));
		k = get_rand(0, (POTIONS - 1));
		swap_string(id_potions[j].title, id_potions[k].title);
	}
}

make_scroll_titles()
{
	short i, j, n;
	short sylls, s;

	for (i = 0; i < SCROLLS; i++) {
		sylls = get_rand(2, 5);
		strcpy(id_scrolls[i].title, "'");

		for (j = 0; j < sylls; j++) {
			s = get_rand(1, (MAXSYLLABLES-1));
			strcat(id_scrolls[i].title, syllables[s]);
		}
		n = strlen(id_scrolls[i].title);
		strcpy(id_scrolls[i].title+(n-1), "' ");
	}
}

get_description(obj, description)
object *obj;
char *description;
{
	char *name_of(), *item_name;
	struct identify *id_table, *get_id_table();
	char more_info[32];

	if (obj->what_is == AMULET) {
		strcpy(description, "the amulet of Yendor ");
		return;
	}
	item_name = name_of(obj);

	if (obj->what_is == GOLD) {
		sprintf(description, "%d pieces of gold", obj->quantity);
		return;
	}

	if (obj->what_is != ARMOR) {
		if (obj->quantity == 1) {
			strcpy(description, "a ");
		} else {
			sprintf(description, "%d ", obj->quantity);
		}
	}
	if (obj->what_is == FOOD) {
		strcat(description, item_name);
		strcat(description, "of food ");
		return;
	}
	id_table = get_id_table(obj);

	if (obj->what_is & (WEAPON | ARMOR | WAND)) goto CHECK;

	switch(id_table[obj->which_kind].id_status) {
	case UNIDENTIFIED:
CHECK:		switch(obj->what_is) {
		case SCROLL:
			strcat(description, item_name);
			strcat(description, "entitled: ");
			strcat(description, id_table[obj->which_kind].title);
			break;
		case POTION:
			strcat(description, id_table[obj->which_kind].title);
			strcat(description, item_name);
			break;
		case WAND:
			if (obj->identified ||
			(id_table[obj->which_kind].id_status == IDENTIFIED)) {
				goto ID;
			}
			if (id_table[obj->which_kind].id_status == CALLED) {
				goto CALL;
			}
			strcat(description, id_table[obj->which_kind].title);
			strcat(description, item_name);
			break;
		case ARMOR:
			if (obj->identified) {
				goto ID;
			}
			strcpy(description, id_table[obj->which_kind].title);
			break;
		case WEAPON:
			if (obj->identified) {
				goto ID;
			}
			strcat(description, name_of(obj));
			if (obj == rogue.weapon) {
				strcat(description, "in hand");
			}
			break;
		}
		break;
	case CALLED:
CALL:		switch(obj->what_is) {
		case SCROLL:
		case POTION:
		case WAND:
			strcat(description, item_name);
			strcat(description, "called ");
			strcat(description, id_table[obj->which_kind].title);
			goto MI;
			break;
		}
		break;
	case IDENTIFIED:
ID:		switch(obj->what_is) {
		case SCROLL:
		case POTION:
			strcat(description, item_name);
			strcat(description, id_table[obj->which_kind].real);
			break;
		case WAND:
			strcat(description, item_name);
			strcat(description, id_table[obj->which_kind].real);
MI:			if (obj->identified) {
				sprintf(more_info, "[%d]", obj->class);
				strcat(description, more_info);
			}
			break;
		case ARMOR:
			sprintf(description, "%s%d ",
			((obj->damage_enchantment >= 0) ? "+" : ""),
			obj->damage_enchantment);
			strcat(description, id_table[obj->which_kind].title);
			sprintf(more_info, "[%d] ", get_armor_class(obj));
			strcat(description, more_info);
			if (obj == rogue.armor) {
				strcat(description, "being worn");
			}
			break;
		case WEAPON:
			sprintf(description+strlen(description), "%s%d,%s%d ",
			((obj->to_hit_enchantment >= 0) ? "+" : ""),
			obj->to_hit_enchantment,
			((obj->damage_enchantment >= 0) ? "+" : ""),
			obj->damage_enchantment);
			strcat(description, name_of(obj));
			if (obj == rogue.weapon) {
				strcat(description, "in hand");
			}
			break;
		}
		break;
	}
}

mix_metals()
{
	short i, j, k;

	for (i = 0; i <= MAXMETALS; i++) {
		j = get_rand(0, MAXMETALS-1);
		k = get_rand(0, MAXMETALS-1);
		swap_string(metals[j], metals[k]);
	}
	for (i = 0; i < WANDS; i++) {
		strcpy(id_wands[i].title, metals[i]);
	}
}

single_inventory()
{
	short ch;
	char description[SCOLS];
	object *obj, *get_letter_object();

	ch = get_pack_letter("inventory what? ", IS_OBJECT);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("No such item.", 0);
		return;
	}
	strcpy(description, "x) ");
	description[0] = ch;
	get_description(obj, description+3);
	message(description, 0);
}

struct identify *get_id_table(obj)
object *obj;
{
	switch(obj->what_is) {
	case SCROLL:
		return(id_scrolls);
		break;
	case POTION:
		return(id_potions);
		break;
	case WAND:
		return(id_wands);
		break;
	case WEAPON:
		return(id_weapons);
		break;
	case ARMOR:
		return(id_armors);
		break;
	}
}
