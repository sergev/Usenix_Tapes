/*	@(#)ttyvt100.h 1.8 86/04/13 MITRE	*/

/*
 * Copyright (c) 1985 by Mitre Corp.
 */

/* cursor states */
#define NOCURSOR	0
#define UNDERCURSOR	1
#define BLOCKCURSOR	2

/* terminal states */
#define ALPHA		0	/* normal state reading characters */
#define ESCAPE 		1	/* plain escape so far */
#define ESCBRKT		2	/* CSI; i.e. esc-[ */
/* deleted */
#define VT52            4	/* emulating vt52 terminal */
/* deleted */
#define ESCBRKTQM	6	/* now have esc-[? sequence */
#define ESCAPESHARP     7	/* now have esc-# sequence */
#define ESCAPELPRN      8	/* now have esc-( sequence */
#define ESCAPERPRN      9	/* now have esc-) sequence */
#define ESC52Y         10	/* in vt52 abs cursor mode sequence */
#define EATCHARS       11	/* when at end of line and nowrap */

#define G0 0			/* grafix states set with esc-( and esc-) */
#define G1 1

unsigned short  marks[24];		/* font type for each line */
		
unsigned short  reflections[24][132];	/* one for each char in image */

#define  NUMER5 1		/* vt52 numeric mode for keys */
#define  NUMERA 2		/* ansi numeric mode  "   "   */
#define  APPL5  3		/* vt52 application mode  */
#define  APPLA  4		/* ansi application mode  */

		/*font handling states for each line  */
		/*used for setting marks in line array  */
#define  NORMAL_F  0x00		/* standard 80 col mode  */
#define  WIDE_F    0x40		/* one of the double wide types */
#define  TOP_F     0x01		/* top half of a double height char */
#define  BOTTOM_F  0x02		/* bottom half of same */
#define  NARROW_F  0x20		/* 132 col mode (66 if wide) */
#define  GRAFIX_F  0x04		/* using a graphics font */
#define  BUSY      0x80		/* set if any characters have been written */
#define  BOLD_F    0x08		/* bold attribute on */
#define  SIZE_MASK 0xF0		/* just the width part */
#define  TYPE_MASK 0x0F		/* just the font part */
#define  HALF_MASK 0x03		/* just top and bottom */

#define  UK      0		/* special character set pound sign */
#define  ASC     1		/* regular ascii character set */
#define  GRAPH   2		/* graphics character set */

#define  BOLD    0x01		/* attributes for fillfunc memory */
#define  UNDER   0x02
#define  REVERSE 0x04
#define  BLINK   0x08



#define grafon()  graf_norm = (activeCharset == G1)?((g1 == GRAPH)?GRAPH:ASC)\
	:((g0 == GRAPH)?GRAPH:ASC)    /* 1=ASCII, 2=GRAPHIC */

typedef int character;
#define MAX_SCREEN_WIDTH  132
#define	CHAR_BUF_LEN	300
