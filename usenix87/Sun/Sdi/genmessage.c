#include <pixrect/pixrect_hs.h>
#define NULL 0

/*
 * Copyright 1987 by Mark Weiser.
 * Permission to reproduce and use in any manner whatsoever on Suns is granted
 * so long as this copyright and other identifying marks of authorship
 * in the code and the game remain intact and visible.  Use of this code
 * in other products is reserved to me--I'm working on Mac and IBM versions.
 */

/*
 * Real quick and dirty hack to enlarge an image.  Reads and outputs
 * in C-style pixrect form (human readable, #includable).  Takes
 * two arguments: the file to be enlarged, and the integer enlargement.
 *
 * This is not used during the game itself (too slow), but run at
 * 'make' time to constuct the popup face for the end of game.
 */
short *in;
/* size of input image */
#define SIZE 64
/* how much larger to make each dimension */
#define MAXMULT 16
int MULT;

/*
 * The theory is, by changing only 'pattern' and 'shift' to reflect the
 * bit numbering of the target machine, this code will work for big
 * and little endians.
 */

static short pattern[] = {
	0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100,
	0x80,   0x40,   0x20,   0x10,   0x8,   0x4,   0x2,   0x1
	};

static short shift[] = {
	15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
	};

static short out[SIZE*SIZE*MAXMULT*MAXMULT/16 + 1];


main(argc, argv)
char **argv;
{
	int row, col, val, i, j;
	struct pixrect *pr, *icon_load_mpr();

	char error_msg[256];
	if (argc != 3) {
		printf("Usage: genmessage file factor\n");
		exit(1);
	}
	argc--; argv++;
	if ((pr = icon_load_mpr(*argv, error_msg)) == NULL) {
		printf("Could not get file '%s'.\n", *argv);
		printf("%s",error_msg);
	}
	in = mpr_d(pr)->md_image;

	argc--; argv++;
	MULT = atol(*argv);
	if (MULT > MAXMULT) {
		printf("Factor too big.\n");
		exit(1);
	}

	printf("/* Format_version=1, Width=%d, Height=%d, Depth=1, Valid_bits_per_item=16\n */\n", SIZE*MULT, SIZE*MULT);
	for (row = 0; row < SIZE; row++) {
		for (col = 0; col < SIZE; col++) {
			val = bitval(in, row*SIZE + col);
			for (i=0; i<MULT; i++)
				for (j=0; j<MULT; j++)
					bitset(out, row*MULT*MULT*SIZE+(MULT*SIZE*i) + col*MULT+j, val);
			}
	}
	for (i = 0; i < SIZE*SIZE*MULT*MULT/128; i++) {
		for ( j = 0;  j < 8; j++) {
			printf(" 0x%04x, ", out[i*8 + j] & 0xffff);
		}
		printf("\n");
	}
	exit(0);
}


/* Machine-dependent: depends on what kind of endian. */

bitset(b, l, v)
short *b;
{
	b[l/16] = b[l/16] & ~pattern[l%16] | (((v & 0x1) << shift[l%16]) & pattern[l%16]);
}

bitval(b, l)
short *b;
{
	return ((b[l/16] & pattern[l%16]) >> shift[l%16]) & 0x1;
}
