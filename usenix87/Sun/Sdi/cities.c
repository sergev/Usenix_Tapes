/**********************************  cities.c  ************************/
#include <pixrect/pixrect_hs.h>
#include <sunwindow/notify.h>
#include <stdio.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Code to do things to cities.  Much of the code here consists of
 * passing a routine into doto_cities, which then calls that routine
 * on each city.  See the comment for doto_cities (near the end of the file)
 * for more information.
 */

static short default_city_data[] = {
#include "default_city.h"
};
mpr_static(default_city_pr, 64, 64, 1, default_city_data);

static struct pixrect *city_pr_ptr = &default_city_pr;
static colormap_t city_colormap, *city_colormap_ptr = NULL;

static short melted_city_data[] = {
#include "melt.h"
};
mpr_static(melted_city_pr, 64, 64, 1, melted_city_data);


#define MIN_SPACE 4
#define MAX_CITIES 40

static int city_space, excess, global_count;
static long bits_in_city;

static int do_update(), do_count(), do_placement(), do_kill(), 
	   do_update_all(), do_grow(), do_update_cities(), do_melt_all(); 

static int cities_inited = 0;
static short cities[MAX_CITIES];


static int growcount;	/* ugh, another weird global.
						 * set by do_update_cities, used only by 'do_grow'.
						 */

/*
 * Read a new city pixrect from a file.
 */
init_city_bits(filename)
char *filename;
{
	/* this routine is only called if there is non-default city. */
	if (filename != NULL) {
			char error_msg[256];
			struct pixrect *tmp,*icon_load_mpr();
			if ((tmp = icon_load_mpr(filename, error_msg)) == NULL) {
				printf("Could not get pr '%s'.\n", filename);
				printf("%s",error_msg);
				printf("Using default cities.\n");
			} else {
				city_pr_ptr = tmp;
			}
	}
}

/*
 * Compute the positions of all the cities based on current screen size,
 * and declare all the cities to be unmelted.
 */
init_cities()
{
	int i, old_num_cities = num_cities;
	bits_in_city = count_bits(city_pr_ptr);
	num_cities = ((max_x - MARGIN)/64) + 1;	/* bigger than possible */
	city_space = 0;

	while (city_space < MIN_SPACE) {
		num_cities -= 1;
		city_space = ((max_x - MARGIN) - (num_cities * 64))/(num_cities-1);
	};
	
	excess = max_x - (MARGIN + 64*num_cities + city_space*(num_cities-1));
	for (i=old_num_cities; i<num_cities; i++) {
		cities[i] = 1;
	}	
	cities_inited = 1;
}

/*
 * display the cities, melted or otherwise.
 */
place_cities(pw)
Pixwin *pw;
{
	pw_writebackground(pw, 0, max_y-8, max_x, max_y, PIX_NOT(PIX_SRC));
	doto_cities(pw, do_placement, city_pr_ptr, 0);
	doto_cities(pw, do_placement, &melted_city_pr, 1);
}

/* helper for 'place_cities', passed into 'doto_cities' */
static int
do_placement(pr, city_pr)
struct pixrect *pr, *city_pr;
{
	pr_rop(pr, 0, 0, 64, 64, PIX_SRC, city_pr, 0, 0);
	return 1;	/* this forces the new city back into the pixwin */
}

/*
 * Pop a new city onto the display at a random empty position.
 */
new_city(pw)
Pixwin *pw;
{
	int pick, num_gone = 0, i;
	for (i=0; i < num_cities; i += 1) {
		if (!cities[i])
			num_gone += 1;
	}
	if (num_gone == 0)
		return;
	pick = (random() % num_gone) + 1;
	for (i=0; i < num_cities; i += 1) {
		if (!cities[i] && !--pick)
			break;
	}
	/* restore a random melted city */
	cities[i] = 1;
	do_update_cities(pw, 1, do_grow);
}

/* helper for do_update_cites, grow a city. */
static int
do_grow(pr, speed)
struct pixrect *pr;
{
	grow(pr, city_pr_ptr, growcount+16);
	return 1;
}

/* return how many died */
static int global_citycount;
compute_cities(pw)
Pixwin *pw;
{
	global_citycount = 0;
	doto_cities(pw, do_count, 0, 0);
	return global_citycount;
}

/* helper for 'compute_cities', passed into 'doto_cities' */
static int
do_count(pr, client)
struct pixrect *pr;
int *client;
{
	if (city_dead(pr)) {
		global_citycount += 1;
	}
	return 0;	/* avoids pw_write in doto_cities */
}

/*
 * Melt or grow each city, as appropriate.
 */
update_cities(pw, speed)
{
	do_update_cities(pw, speed, do_update);
	doto_cities(pw, do_kill, 0, 0);
}

/* helper for 'update_cities', passed to 'do_update_cities', grows or melts. */
static int
do_update(pr, speed)
struct pixrect *pr;
int speed;
{
	if (city_dead(pr)) {
		melt(pr, &melted_city_pr, speed);
		return 1;	
	} else {
		grow(pr, city_pr_ptr, growcount+16);
		return 1;   
	}
}

/* Helper for 'update_cities', passed into doto_cities, computes dead cities. */
static int
do_kill(pr, tmp)
struct pixrect *pr;
{
	if (city_dead(pr))
		cities[global_count] = 0;
	return 0;
}

/* Do what it says. */
melt_all_cities(pw, speed)
{
	do_update_cities(pw, speed, do_melt_all);
}


/* helper for 'melt_all_cities', passed into 'do_update_cities' */
static int
do_melt_all(pr, speed)
struct pixrect *pr;
int speed;
{
	melt(pr, &melted_city_pr, speed);
	return 1;	/* forces writing out the melted city in doto_cities */
}

/*
 * Call function 'helper' numerous times on each city, thus causing
 * growing or melting (or both), depending upon 'helper'.
 */
static int
do_update_cities(pw, speed, helper)
int (*helper)();
{
	int i;
	int count = (64 / speed) + 1;
	for (growcount = 1; growcount < count; growcount += 1) {
		/* we might plausibly batch here, but the melt looks better without */
		doto_cities(pw, helper, speed, 0);
	}
}

/*
   Cities are a kind of abstract datatype.  To do something to all
   cities, pass a function and a piece of client data to 'doto_cities',
   and the function will be called once for each city still in
   existence (or if 'non-cities' is non-zero, once for each city NOT
   still in existence.)  The calling sequence for the passed in
   function will be:
		func(pr, client_data);
   where 'pr' is a 64x64 pixrect of a city.  If 'func' returns 0 then
   'pr' is assumed to be unchanged, otherwise 'pr' is written back into
   the pixwin so the change is visible. Global_count is set before the
   call to the ordinal number of the city currently being operated on.

   Init_cities must be called before any city operations.
 */

static char tmp_image_place[512];
mpr_static(tmpcity_pr, 64, 64, 1, tmp_image_place);

doto_cities(pw, func, client_data, non_cities)
Pixwin *pw;
int (*func)();
int *client_data;
{
	int x = MARGIN/2 + excess/2, stop_x = max_x - MARGIN/2 - 64;
	int y = max_y - 64;
	if (! cities_inited) {
		/* no return from here */
		printf("'doto_cities called before 'init_cities'.\n");
		abort();
	}
	global_count = 0;
	while (x <= stop_x) {
		if ((!non_cities && cities[global_count]) ||
				(non_cities && !cities[global_count])) {
			pw_read(&tmpcity_pr, 0, 0, 64, 64, PIX_SRC, pw, x, y);
			if (func(&tmpcity_pr, client_data))
				pw_write(pw, x, y, 64, 64, PIX_SRC, &tmpcity_pr, 0, 0);
		}
		global_count += 1;
		x += 64 + city_space;
	}
}

/*
 * See if a city is dead.  It is if 2/3's of its pixels are clobbered.
 */
city_dead(pr)
struct pixrect *pr;
{
	long bitcount = count_bits(pr);
	if (bitcount < 2*bits_in_city/3)
		return 1;
	else return 0;
}

