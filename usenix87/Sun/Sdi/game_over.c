/**********************************  game_over.c  **********************/
#include <pixrect/pixrect_hs.h>
#include "sdi.h"

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * This file is responsible for flashing my face on the screen if the game ends
 *  and 'non-stop' is not set.
 */

/* ORIGSIZE comes in from a -D in the makefile */
#define NUMBER 10
#define BORDERSIZE 64
#define SIZE (ORIGSIZE+BORDERSIZE)


static short message_data[] = {
#include "bigmessage.h"
};
mpr_static(origmessagepr, ORIGSIZE, ORIGSIZE, 1, message_data);

/*
 * Routine to flash my face on the screen.
 * The pr_destroy order below is tricky, but there is no storage leak. 
 */

do_game_over()
{
	int center_x = max_x/2, center_y = max_y/2, circlesize;
	struct pixrect *circlepr;
	struct pixrect *messagepr;
	struct rect r;
	int i;
	struct pixwin *pw_a = citypw,
		*pw_b = launchpw;

	messagepr = make_circle(SIZE);
	pr_rop(messagepr, BORDERSIZE/2, BORDERSIZE/2, ORIGSIZE, ORIGSIZE,
		PIX_SRC, &origmessagepr, 0, 0);
	
	pw_batch_on(launchpw);
	pw_batch_on(citypw);
	for(i=0; i<NUMBER; i++) {
		if (i > 0)
			pr_destroy(circlepr);
		circlesize = SIZE*(i+1)/(NUMBER);
		circlepr = make_circle(circlesize);
		do_picture(circlepr, circlesize, center_x, center_y, messagepr, pw_a, pw_b);
		pw_show(launchpw);
		pw_show(citypw);
	}
#ifdef ORIG
	for(i=NUMBER-1; i > 0; i--) {
		/* clear the old circle (uses old pr and circlesize) */
		do_picture(circlepr, circlesize, center_x, center_y, NULL, pw_a, pw_b);
		pr_destroy(circlepr);
		/* make the new circle */
		circlesize = SIZE*(i)/(NUMBER);
		circlepr = make_circle(circlesize);
		do_picture(circlepr, circlesize, center_x, center_y, messagepr, pw_a, pw_b);
		pw_show(launchpw);
		pw_show(citypw);
	}
	do_picture(circlepr, circlesize, center_x, center_y, NULL, pw_a, pw_b);
#else
	for (i=0; i < (max_y+SIZE/2); i += 10) {
		do_picture(circlepr, circlesize, center_x, center_y+i, messagepr, pw_a, pw_b);
		pw_show(launchpw);
		pw_show(citypw);
	}
#endif
	pr_destroy(circlepr);
	pw_batch_off(citypw);
	pw_batch_off(launchpw);
	}

/*
 * Put up one instance of the final picture.  My, stenciling is slow.
 */
do_picture(pr, size, x, y, messagepr, pw_a, pw_b)
struct pixrect *pr, *messagepr;
int size;
struct pixwin *pw_a, *pw_b;
{
		pw_stencil(pw_a,
			x - size/2, y - size/2,
			size, size, PIX_SRC,
			pr, 0, 0,
			messagepr, SIZE/2 - size/2, SIZE/2 - size/2);
		pw_stencil(pw_b,
			x - size/2, y - size/2,
			size, size, PIX_SRC,
			pr, 0, 0,
			messagepr, SIZE/2 - size/2, SIZE/2 - size/2);
}
