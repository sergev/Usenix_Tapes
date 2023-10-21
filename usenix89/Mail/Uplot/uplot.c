/*
 * uplot.c
 *
 *	Crude hack for drawing a map of the UUCP net - reads on standard
 *	input a highly massaged version of the '#L' lines from the UUCP map,
 *	processed by uplot.sed and sorted, and writes plot(3) instructions
 *	on standard output.
 *
 *	Released to the public domain August, 1986.
 *	Larry Campbell	(maynard!campbell)
 *	The Boston Software Works, Inc.
 *	120 Fulton Street, Boston, MA 02109
 */

#include <stdio.h>

/* These defines set the bounds of the rectangle being drawn */

#define MINLAT 25
#define MAXLAT 50

#define MINLON (-125)
#define MAXLON (-60)

struct
    {
    int deg;
    int min;
    } lat, lon;

int n, x, y, lastx, lasty, weight, col, maxweight,
    goodpoints, badpoints, outpoints, uniques, debug;

char buf[256];

main(argc, argv)
char *argv[];
{
if (argc > 1) debug++;
openpl();
erase();
space(MINLON*60, MINLAT*60, (MAXLON-30)*60, MAXLAT*60);

city("Boston",		42,	- 71);
city("Chicago",		42,	- 88);
city("Dallas",		33,	- 97);
city("DC",		39,	- 77);
city("Denver",		40,	-105);
city("Houston",		30,	- 95);
city("LA",		34,	-118);
city("Miami",		26,	- 80);
city("NYC",		41,	- 74);
city("SF",		38,	-122);
city("Seattle",		47,	-123);
city("Toronto",		43,	- 79);
city("Vancouver",	49,	-124);

lastx = lasty = 0;

while (gets(buf) != NULL)
    {
    n = sscanf(buf, "%d %d N / %d %d W", &lat.deg, &lat.min, &lon.deg, &lon.min);
    if (n != 4)
	n = sscanf(buf, "%d %d %*d N / %d %d %*d W", &lat.deg, &lat.min, &lon.deg, &lon.min);
    if (n != 4)
	n = sscanf(buf, "%d %d N %d %d W", &lat.deg, &lat.min, &lon.deg, &lon.min);
    if (n != 4)
	n = sscanf(buf, "%d %d %*d N %d %d %*d W", &lat.deg, &lat.min, &lon.deg, &lon.min);
    if (n != 4)
	n = sscanf(buf, "%d %d %*d %*d N / %d %d %*d %*d W", &lat.deg, &lat.min, &lon.deg, &lon.min);
    if (n != 4)
	n = sscanf(buf, "%d %d W / %d %d N", &lon.deg, &lon.min, &lat.deg, &lat.min);
    if (n != 4)
	n = sscanf(buf, "%d %d %*d W / %d %d %*d N", &lon.deg, &lon.min, &lat.deg, &lat.min);
    if (n != 4)
	n = sscanf(buf, "%d %d W %d %d N", &lon.deg, &lon.min, &lat.deg, &lat.min);
    if (n != 4)
	n = sscanf(buf, "%d %d %*d W %d %d %*d N", &lon.deg, &lon.min, &lat.deg, &lat.min);
    if (n != 4)
	n = sscanf(buf, "%d %d %*d %*d W / %d %d %*d %*d N", &lon.deg, &lon.min, &lat.deg, &lat.min);
    if (n != 4)
	{
	n = sscanf(buf, "%d N / %d W", &lat.deg, &lon.deg) + 2;
	lat.min = lon.min = 0;
	}
    if (n != 4)
	{
	n = sscanf(buf, "%d W / %d N", &lon.deg, &lat.deg) + 2;
	lat.min = lon.min = 0;
	}
    if (n != 4)
	{
	badpoints++;
	if (debug)
	    printf("bad: %s\n", buf);
	}
    else
	{
	goodpoints++;
	lon.deg = -lon.deg;
	lon.min = -lon.min;
	if (lat.deg >= MAXLAT || lat.deg < MINLAT ||
	     lon.deg >= MAXLON || lon.deg < MINLON)
	    {
	    outpoints++;
	    continue;
	    }
	x = lon.deg * 60 + lon.min;
	y = lat.deg * 60 + lat.min;
	if (x == lastx && y == lasty)
	    weight++;
	else
	    {
	    uniques++;
	    if (weight > maxweight) maxweight = weight;
	    switch (weight)
	    	{
	    	case 0: col = 1; break;
	    	case 1: col = 2; break;
	    	default: col = 3; break;
	    	}
	    if (debug)
		printf("lastx = %d, lasty = %d, weight = %d, color = %d\n", lastx, lasty, weight, col);
	    if (lastx && lasty)
	    	{
		color(col);
		point(lastx, lasty);
	    	}
	    weight = 0;
	    lastx = x;
	    lasty = y;
	    }
	}
    }
printf("Good: %d, bad: %d, out of range: %d, max: %d, uniques: %d\n", goodpoints, badpoints, outpoints, maxweight, uniques);
}

city(s, lat, lon)
char *s;
int lat, lon;
{
char buf[80];

if (lat < MINLAT || lat >= MAXLAT) return;
if (lon < MINLON || lon >= MAXLON) return;
color(2);
move(lon*60, lat*60);
sprintf(buf, ".%s", s);
label(buf);
}
