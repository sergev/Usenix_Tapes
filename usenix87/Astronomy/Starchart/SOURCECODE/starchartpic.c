/*
 * Unix Pic file format driver for startchart.c mainline
 */

#include <stdio.h>
#include "starchart.h"

#define PICFRAG 8	/* split long "move,draw,...,draw" strings for pic */

/*
 * Chart parameters (limiting magnitude and window x,y,w,h)
 */

mapblock thumbnail =	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			3.2, 480, 0, 480, 240, 0.0 };

mapblock master =	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			8.0, 0, 370, 960, 960, 0.0 };

/*
 * Generic Star Drawing Stuff
 */

static int oldps, dcount;

#define PICSCALE (1.0/160.0) /* roughly 6.5/1024.0 */

float conv(i)
    {
    return(i*PICSCALE);
    }

vecsize(newps)
    int newps;
    {
    if (newps != oldps) printf("\n.ps %d", newps);
    oldps = newps;
    }

 vecopen()
   {
   printf(".nf\n.ll 6.75i\n.po 0.75i\n.RT\n.PS");
   vecsize(10);
   printf("\n.tl'Database: \\fIYale Star Catalog\\fP'\\s18\\fBStarChart\\fP\\s0'Software: \\fIAWPaeth@watCGL\\fP'");
   printf("\n.ll 7.25i");
   }

vecclose()
    {
    printf("\n.PE\n");
    fflush(stdout);
    }

vecmove(x, y)
    {
    dcount = 0;
    printf("\nline from %.3fi,%.3fi", conv(x), conv(y));
    }

vecdraw(x, y)
    {
    dcount++;
    printf(" to %.3fi,%.3fi", conv(x), conv(y));
    if (dcount > PICFRAG) vecmove(x,y);	/* must fragment long pic commands */
    }

vecsyms(x, y, s)
    char *s;
    {
    printf("\n\"\\ %s\" ljust at %.3fi,%.3fi", s, conv(x), conv(y));
    }

vecmovedraw(x1, y1, x2, y2)
    {
    vecmove(x1, y1);
    vecdraw(x2, y2);
    }

drawP(x, y)
    {
    vecsize(10);
    vecsyms(x, y, "P");
    }

drawstar(x, y, mag)
    {
    switch (mag)
	{
	case -1: draw0(x, y); break;
	case  0: draw0(x, y); break;
	case  1: draw1(x, y); break;
	case  2: draw2(x, y); break;
	case  3: draw3(x, y); break;
	case  4: draw4(x, y); break;
	default: draw5(x, y); break;
	}
    }

vecsymcen(x, y, s)
    char *s;
    {
    printf("\n\"%s\" at %.3fi,%.3fi", s, conv(x), conv(y));
    }

draw0(x, y)
    {
    vecsize(18);
    vecsymcen(x, y, "\\o'\\(bu+'");
    }

draw1(x, y)
    {
    vecsize(16);
    vecsymcen(x, y, "\\(bu");
    }

draw2(x, y)
    {
    vecsize(12);
    vecsymcen(x, y, "\\o'\\(bu+'");
    }

draw3(x, y)
    {
    vecsize(10);
    vecsymcen(x, y, "\\(bu");
    }

draw4(x, y)
    {
    vecsize(8);
    vecsymcen(x, y, "\\o'\\(bu+'");
    }

draw5(x, y)
    {
    vecsize(6);
    vecsymcen(x, y, "\\(bu");
    }
