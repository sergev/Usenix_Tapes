/****************************  pr_helpers.c  *************************/
#include <pixrect/pixrect_hs.h>

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/* These routines are pure pixrect operators, used mostly for city operations */

static short pattern[] = {
	0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
	0x80,   0x40,   0x20,   0x10,   0x8,   0x4,   0x2,   0x1
	};
static short *end_of_pattern_ptr = &pattern[16];


#define lefthalf(x) (((x) & 0xff00) >> 8)
#define righthalf(x) ((x) & 0xff)

/*
 * Does an incremental melt of old into target, replacing
 * old, by dropping pixels down until they hit target
 * pixels.  Melts 'speed' pixels per call.  Old and target should be
 * memory pixrects.  Probably won't work as written for color.
 *
 * Pretty dumb and slow, undoubtedly could be done better.
 */

/* alg: drop topmost pixels by speed pixels.  Any pixels which were on
   in the old which matched on's in the target, still stay on.
   Warning: the speed parameter does not work for any value but 1.
 */


melt(old, target, speed)
struct pixrect *old, *target;
int speed;
{
	short x_count, y_count, max_x, count, base = 0, bit_count;
	int w = old->pr_size.x, h = old->pr_size.y;
	struct pixrect *final;
	struct mpr_data *mpr_final, *mpr_old;
	short num_shorts, speed_inc;
	short *old_image, *final_image;
	unsigned short *remember_all_bits;
	unsigned short *remember_bits, *bit_ptr;
	register short *pattern_ptr, *old_image_ptr, *final_image_ptr;

	if (w != target->pr_size.x || h != target->pr_size.y) {
		/* never leave here */
		printf("melt requires equal size pixrects.\n");
		abort();
	}

	mpr_old = mpr_d(old);

	if (mpr_old->md_offset.x != 0 || mpr_old->md_offset.y != 0) {
		/* Never leave here */
		printf("Can't handle region pixrects in 'melt'.\n");
		abort();
	};

	final = mem_create(w, h, 1);

	/* see if there is any work to do */
	pr_rop(final, 0, 0, w, h, PIX_SRC, old, 0, 0);
	pr_rop(final, 0, 0, w, h, PIX_NOT(PIX_SRC) & PIX_DST, target, 0, 0);
	if (all_zero_bits(final)) {
		pr_destroy(final);
		return;
	}
	
	/* remember what was ok from before */
	pr_rop(final, 0, 0, w, h, PIX_SRC, old, 0, 0);
	pr_rop(final, 0, 0, w, h, PIX_SRC & PIX_DST, target, 0, 0);
	
	/* melt some bits */

	mpr_final = mpr_d(final);
	num_shorts = mpr_final->md_linebytes / 2;
	speed_inc = num_shorts*speed;
	final_image = mpr_final->md_image;
	old_image = mpr_old->md_image;
	remember_all_bits = (unsigned short *)calloc(num_shorts+1, 2);
	remember_bits = (unsigned short *)calloc(w+1, 2);

	/* Melt the top */
	for(y_count = 0; y_count < h - speed; y_count += 1) {
		if (all_ones(remember_all_bits, num_shorts))
			goto next;
		bit_ptr = remember_bits;
		old_image_ptr = &old_image[base];
		final_image_ptr = &final_image[base];
		for (x_count = 0; x_count < num_shorts; x_count += 1) {
			remember_all_bits[x_count] |= *old_image_ptr;
			for(pattern_ptr = pattern; pattern_ptr < end_of_pattern_ptr; pattern_ptr++) {
				/* for each bit... */
				if (*old_image_ptr & *pattern_ptr) {
					/* if you saw one in this 'x' position before... */
					if (*bit_ptr) {
						/* just copy this one in place. */
						*final_image_ptr |= *pattern_ptr;
					} else {
						/* if this one is the first one here, melt it. */
						*bit_ptr = 1;
						*(final_image_ptr+speed_inc) |= *pattern_ptr;
					}
				}
				bit_ptr++;
			} 
			old_image_ptr++;
			final_image_ptr++;
		}
	base += num_shorts;
	}

	/* Move the rest of the image as is */
next:
	for(; y_count < h - speed; y_count += 1) {
		old_image_ptr = &old_image[base];
		final_image_ptr = &final_image[base];
		for (x_count = 0; x_count < num_shorts; x_count += 1) {
			*final_image_ptr |= *old_image_ptr;
			old_image_ptr++;
			final_image_ptr++;
		}
	base += num_shorts;
	}


	/* return value in old */
	pr_rop(old, 0, 0, w, h, PIX_SRC, final, 0, 0);
	pr_destroy(final);
	free(remember_bits);
	free(remember_all_bits);
}


static lookup[] = {
#include "lookup.h"
};	

/*
 * Count the number of bits in a pixrect.
   Not tested on, and may not work for, color.
 */
count_bits(pr)
struct pixrect *pr;		/* should be a memory pixrect for dvc ind. */
{
	register short x_count, y_count, max_x, count, base = 0, num_shorts;
	struct mpr_data *mpr = mpr_d(pr);
	if (mpr->md_offset.x != 0 || mpr->md_offset.y != 0) {
		/* Never leave here */
		printf("Can't handle region pixrects in 'count_bits'.\n");
		abort();
	};
	count = 0;
	num_shorts = mpr->md_linebytes / 2;
	for(y_count = 0; y_count < pr->pr_size.y; y_count++) {
		for (x_count = 0; x_count < num_shorts; x_count += 1) {
			count += lookup[lefthalf(mpr->md_image[base+x_count])]
				+lookup[righthalf(mpr->md_image[base+x_count])];
		}
		base += num_shorts;
	}
	return count;
}

/*
 * See if a memory pixrect is all zero.
 */
all_zero_bits(pr)
struct pixrect *pr;		/* should be a memory pixrect */
{
	register short x_count, y_count, max_x, count, base = 0, num_shorts;
	struct mpr_data *mpr = mpr_d(pr);
	if (mpr->md_offset.x != 0 || mpr->md_offset.y != 0) {
		/* Never leave here */
		printf("Can't handle region pixrects in 'count_bits'.\n");
		abort();
	};
	count = 0;
	num_shorts = mpr->md_linebytes / 2;
	for(y_count = 0; y_count < pr->pr_size.y; y_count++) {
		for (x_count = 0; x_count < num_shorts; x_count += 1) {
			if (mpr->md_image[base+x_count])
				return 0;
		}
		base += num_shorts;
	}
	return 1;
}

/*
 * See if an array of shorts contains all ones.
 */
all_ones(x, len)
register len;
register unsigned short *x;
{
	for(; len; len--) {
		if (*x++ != 0xffff) {
			return 0;
		}
	}
	return 1;
}


/*
 * Grow could, and possibly should, be made to work like melt, and
 * grow only at the edge of black areas, but growing linearly up
 * from the bottom looks ok too. (Which is not true in revese for melting!).
 * Unfortunately they are now asymmetrical, because grow needs a 'position'
 * parameter which says how far from the bottom we are.
 *
 * Grow only works for 64x64 bit pixrects (because of hardwired constants),
 * unlike melt which can melt anything.
 */

grow(old, target, position)
struct pixrect *old, *target;
{
	if (position < 1 || position > 64)
		return;
	pr_rop(old, 0, 64 - position, 64, position, PIX_SRC,
		target, 0, 64 - position); 
}

