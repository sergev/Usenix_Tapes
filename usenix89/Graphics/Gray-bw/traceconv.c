
/* traceconv.c - a program to convert 256 level gray files to MacPaint
 * pictures.
 * Written by: Steve Hawley   Aztec C V1.06H
 */
#include <quickdraw.h>
#include <window.h>
#include <event.h>
#include <stdio.h>
#include <desk.h>
#define dithsize 7
#define ylimit 451

WindowRecord	wRecord;
WindowPtr	myWindow;
EventRecord myEvent;



main ()
{
	Rect screen;
	int x, y;
	int level;
	FILE *fopen(), *fp;
	
	InitGraf(&thePort);	/* do inits */
	InitWindows();
	FlushEvents(everyEvent,0);
	SetCursor(&arrow);
	HideCursor();
	SetRect(&screen, 4, 40, 508, 340); /*set up window */
	myWindow = NewWindow(&wRecord, &screen, ctop("Raytrace"),
			1,0,(char*)-1,1,(long)0);
	SetPort(myWindow);
	ShowWindow(myWindow);
	/* put up a new window */
	SetOrigin(0,0);
	if ( (fp = fopen("data.dis", "r")) == NULL) printf("Can't open file!\n");
	else {
		x = 0;
		y = 0;
		while( (level = getc( fp )) != EOF){
			if (dither(x, y, level)) PenPat(white);
			else PenPat(black);
			MoveTo(x,ylimiy - y);
			LineTo(x,ylimit - y);
			y++;
			if (y >= ylimit) {
				y = 0;
				x++;
			}
			if (Button()) exit(0); /* abort if button pressed */
		}
	}
	
	
	while(!Button()){
		GetNextEvent(everyEvent, &myEvent); /* wait for key/btn press */
		SystemTask();
	}
}
dither (x,y,level) /* dithering function */
register int x, y, level;
{
	 /* 8 x 8 dithering matrix */
	static int matrix[dithsize+1][dithsize+1] = {
	{  0,128, 32,160,  8,136, 40,168},
	{192, 64,224, 96,200, 72,232,104},
	{ 48,176, 16,144, 56,184, 24,152},
	{240,112,208, 80,248,120,216, 88},
	{ 12,140, 44,174,  4,132, 36,164},
	{204, 76,236,108,196, 68,228,100},
	{ 60,188, 28,156, 52,180, 20,148},
	{252,124,210, 92,244,116,212, 84}
	};



/* dithering by area access to matrix by using
	the MOD function on the x and y coordinates */
	return(level > matrix[x % dithsize][y % dithsize]);
}
