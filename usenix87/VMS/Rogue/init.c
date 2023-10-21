#include stdio.h
#include curses.h
#include "object.h"
#include "room.h"

char player[32];
char *player_name;
short cant_int = 0, did_int = 0;

extern char ichars[];
extern short party_room;
extern short chan;

init()
{
	char *cuserid(); 
	short i;
	int byebye();

	if (!(player_name = cuserid(player))) {
		fprintf(stderr, "Hey!  Who are you?");
		exit(2);
	}
/*
 * cheat for now
 */
	printf("Hello %s, just a moment while I dig the dungeon...",
	player_name);
	fflush(stdout);
	ttopen();
	sleep(3);

	initscr();
	for (i = 0; i < 26; i++) {
		ichars[i] = 0;
	}
	start_window();
	if ((LINES < 24) || (COLS < 80)) {
		clean_up("must be played on 24 x 80 screen");
	}
	LINES = SROWS;

	srand(time());
	shuffle_colors();
	mix_metals();
	make_scroll_titles();

	level_objects.next_object = 0;
	level_monsters.next_object = 0;
	player_init();
}

player_init()
{
	object *get_an_object(), *obj;

	rogue.pack.next_object = 0;

	obj = get_an_object();
	get_food(obj);
	add_to_pack(obj, &rogue.pack, 1);

	obj = get_an_object();		/* initial armor */
	obj->what_is = ARMOR;
	obj->which_kind = RING;
	obj->class = RING+2;
	obj->is_cursed = obj->is_protected = 0;
	obj->damage_enchantment = 1;
	obj->identified = 1;
	add_to_pack(obj, &rogue.pack, 1);
	rogue.armor = obj;

	obj = get_an_object();		/* initial weapons */
	obj->what_is = WEAPON;
	obj->which_kind = MACE;
	get_weapon_thd(obj);
	obj->is_cursed = 0;
	obj->damage = "2d3";
	obj->to_hit_enchantment = obj->damage_enchantment = 1;
	obj->identified = 1;
	add_to_pack(obj, &rogue.pack, 1);
	rogue.weapon = obj;

	obj = get_an_object();
	obj->what_is = WEAPON;
	obj->which_kind = BOW;
	get_weapon_thd(obj);
	obj->is_cursed = 0;
	obj->damage = "1d2";
	obj->to_hit_enchantment = 1;
	obj->damage_enchantment = 0;
	obj->identified = 1;
	add_to_pack(obj, &rogue.pack, 1);

	obj = get_an_object();
	obj->what_is = WEAPON;
	obj->which_kind = ARROW;
	obj->quantity = get_rand(25, 35);
	get_weapon_thd(obj);
	obj->is_cursed = 0;
	obj->damage = "1d2";
	obj->to_hit_enchantment = 0;
	obj->damage_enchantment = 0;
	obj->identified = 1;
	add_to_pack(obj, &rogue.pack, 1);
}

clean_up(estr)
char *estr;
{
	move(LINES-1, 0);
	refresh_vms();
	stop_window();
	printf("\n%s\n", estr);
	exit(1);
}

start_window()
{
	crmode();
	noecho();
	nonl();
}

stop_window()
{
	endwin();
}

byebye()
{
	clean_up("Okay, bye bye!");
}

