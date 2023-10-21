/*
 * Fast monochrome life for Atari ST.
 *
 * Written by
 *   Jan Gray
 *   300 Regina St. N Apt. 2-905
 *   Waterloo, Ontario
 *   N2J 4H2
 *   Canada
 *   (jsgray@watrose.UUCP)
 *
 * Copyright (C) 1987 Jan Gray.
 * This program may be freely redistributed if this notice is retained.
 */

#include "define.h"
#include "osbind.h"

#define	MAXX		640
#define	MAXY		400
#define	BPL		32

#define	ROW_LONGS	(MAXX / BPL)
#define SCREEN_LONGS	(ROW_LONGS * MAXY)

long	*Screen;
long	NextScreen[SCREEN_LONGS];

main()
{
	Screen = (long *)Physbase();

	while (!Cconis()) {			/* until key press... */
		clearBorders();
		gen();
		copyScreen();
	}
}


copyScreen()
{
	register long	*p;
	register long	*q;

	for (p = NextScreen, q = Screen; p < &NextScreen[SCREEN_LONGS]; ) {
		*q++ = *p++;
		*q++ = *p++;
		*q++ = *p++;
		*q++ = *p++;
	}
}

clearBorders()
{
	register long	*p;

	for (p = Screen; p < &Screen[ROW_LONGS]; p++)
		*p = 0L;

	for (p = &Screen[(MAXY - 1) * ROW_LONGS]; p < &Screen[MAXY * ROW_LONGS]; p++)
		*p = 0L;

	for (p = Screen; p < &Screen[SCREEN_LONGS]; p += ROW_LONGS)
		*p &= ~0x80000000L;

	for (p = &Screen[ROW_LONGS-1]; p < &Screen[SCREEN_LONGS]; p += ROW_LONGS)
		*p &= ~1L;
}
