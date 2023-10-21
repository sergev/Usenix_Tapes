/*
 * TTY Display driver for startchart.c mainline
 */

#include <stdio.h>
#include <ctype.h>
#include "starchart.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define ROWS 31
#define COLS 79

/*
 * Chart parameters (limiting magnitude and window x,y,w,h)
 */

mapblock thumbnail =	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			3.0, 420, 35, 480, 195, 0.0 };

mapblock master =	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			4.9, 20, 265, 880, 500, 0.0 };

/*
 * Generic Star Drawing Stuff
 */

static int oldx, oldy, scrx, scry;
static char **linevec;

vecopen()
   {
   int i;
   linevec = (char**)(calloc(ROWS, sizeof(char*)));
   for (i=0; i<ROWS; i++) linevec[i] = (char*)(calloc(COLS, sizeof(char)));
   }

vecclose()
    {
    int i, j, k;
    char c;
    for (i=0; i<ROWS; i++)
	{
	for (j=COLS-1; j>0; j--) if (linevec[i][j]) break;
	for (k=0; k<=j; k++) putchar((c=linevec[i][k]) ? c : ' ');
	putchar('\n');
	free(linevec[i]);
	}
    fflush(stdout);
    free(linevec);
    }

vecsize(points)
    int points;
    {
    }

vecmove(x, y)
    {
    oldx = x;
    oldy = y;
    scrx = x*COLS/1024;
    scry = (768-y)*ROWS/768;
    }

vecdraw(x, y)
    {
    int dx, dy, savex, savey, i, steps;
    char c;
    savex = oldx;
    savey = oldy;
    dx = x-oldx;
    dy = y-oldy;
    c = (abs(dx) > abs(dy)) ? '-' : '|';
    steps = MAX(MAX(abs(dx),abs(dy))/12, 1);
    for(i=0; i<=steps; i++)
	{
	vecmove(savex+i*dx/steps,savey+i*dy/steps);
	sym(c);
	}
    }

vecsyms(x, y, s)
    char *s;
    {
    char c;
    vecmove(x, y);
    while(c = *s++)
	{
	sym(c);
	scrx++;
	}
    }

vecmovedraw(x1, y1, x2, y2)
    {
    vecmove(x1, y1);
    vecdraw(x2, y2);
    }

drawP(x, y)
    {
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

draw0(x, y)
    {
    vecsyms(x, y, "O");
    }

draw1(x, y)
    {
    vecsyms(x, y, "*");
    }

draw2(x, y)
    {
    vecsyms(x, y, "o");
    }

draw3(x, y)
    {
    vecsyms(x, y, "~");
    }

draw4(x, y)
    {
    vecsyms(x, y, ",");
    }

draw5(x, y)
    {
    vecsyms(x, y, ".");
    }

char overwrite(under, over)
    char under, over;
    {
    if (over == under) return(over);
    if (!under) return(over);
    if (!over) return(under);
    if (isspace(under)) return(over);
    if (isspace(over)) return(under);
    if (isalnum(under) && !isalnum(over)) return(under);
    if (!isalnum(under) && isalnum(over)) return(over);
    if ((under == '-') && (over == '|')) return('+');
    if ((under == '|') && (over == '-')) return('+');
    if ((under == '+') && (over == '-')) return('+');
    if ((under == '+') && (over == '|')) return('+');
    if ((under == '-') && (over == '+')) return('+');
    if ((under == '|') && (over == '+')) return('+');
    if ((under == ',') && (over == '.')) return(';');
    if ((under == '.') && (over == ',')) return(';');
    if ((under == '.') && (over == '.')) return(':');
    if ((under == '|') && (over == '.')) return('!');
    if ((under == '|') && (over == ',')) return('!');
    if ((under == '.') && (over == '|')) return('!');
    if ((under == ',') && (over == '|')) return('!');
    if ((under == '-') && (over == '~')) return('=');
    if ((under == '~') && (over == '-')) return('=');
    return(over);
    }

sym(c)
    char c;
    {
    if ( (scrx >= 0) && (scrx < COLS) && (scry >= 0) && (scry < ROWS) )
	linevec[scry][scrx] = overwrite(linevec[scry][scrx], c );
    }
