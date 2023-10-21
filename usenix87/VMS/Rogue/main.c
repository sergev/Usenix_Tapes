#include curses.h
#include "object.h"
short chan;

main()
{
	init();

	for (;;) {
		clear_level();
		make_level();
		put_objects();
		put_stairs();
		put_monsters();
		put_player();
		light_up_room();
		print_stats();
		play_level();
		free_stuff(&level_objects);
		free_stuff(&level_monsters);
		clear();
	}
}
