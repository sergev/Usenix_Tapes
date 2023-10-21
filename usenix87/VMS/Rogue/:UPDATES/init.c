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
extern short AMULET_LEVEL;

init()
{
	char *cuserid(); 
	short i;
	int byebye();


	if (!(player_name = cuserid(player))) {
		fprintf(stderr, "Hey!  Who are you?");
		ttclose();
		exit(2);
		}
/*
 * cheat for now
 */
	printf("Hello %s, just a moment while I dig the dungeon...",
	player_name);
	fflush(stdout);
/*	lib$disable_ctrl(0x1000000, &old_mask); */
	ttopen();
	sleep(1); /* emulate Unix */

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
	get_rand_armor2(obj);
	obj->what_is = ARMOR;
	obj->identified = 1;
	add_to_pack(obj, &rogue.pack, 1);
	rogue.armor = obj;
	AMULET_LEVEL = get_rand(25,39) + obj->class;

	obj = get_an_object();		/* initial weapons */
	obj->what_is = WEAPON;
	get_rand_weapon2(obj);
	obj->identified = 1;
	add_to_pack(obj, &rogue.pack, 1);
	rogue.weapon = obj;

	obj = get_an_object();
	obj->what_is = WEAPON;
	obj->which_kind = SHORT_BOW;
	get_weapon_thd(obj);
	obj->is_cursed = 0;
	obj->damage = "1d8";
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
	ttclose();
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

