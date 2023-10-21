
/*
	Title	: SimpleDemo.c
	Author	: Erik Kilk
	Date	: June 9, 1985
	Revised	: July 17, 1985, November 1986
	
	Synopsis: A demonstration of the use of SimpleTools with 
		other toolbox calls.  This demonstrates a very 
		advanced use of SimpleTools.  I've always disliked
		being given simple examples.  So, this example 
		uses many, many optional features of SimpleTools and
		shows how you can use the standard toolbox calls.
		
	The SimpleTools routines are being freely distributed.
	You may use them, distribute your applications which use them,
	and copy them if you send $20 or more to:
	
			Erik Kilk
			4949 Snyder Lane, #247
			Rohnert Park,  CA  94928
			
	Link this demonstration program with SimpleTools.
		
*/

/*********************************/
/*  DEFINITIONS FOR SIMPLETOOLS  */
/*********************************/

#include "simple.h"


/*******************************/
/*  INCLUDE FILES FOR TOOLBOX  */
/*******************************/

#ifdef MEGAMAX
  #include <qd.h>
  #include <event.h>
  #include <mem.h>
  #include <font.h>
  #include <qdvars.h>
  #include <misc.h>
  #include <win.h>
  #include <string.h>
  #include <stdio.h>
  #define TRUE (-1)			/* Common boolean constants */
  #define FALSE 0
#else
  #include <Quickdraw.h>
  #include <EventMgr.h>
  #include <MemoryMgr.h>
  #include <fontMgr.h>
  #include <WindowMgr.h>
  #include <strings.h>
  #include <stdio.h>
#endif

/**********************/
/*  GLOBAL VARIABLES  */
/**********************/

int circspeed = 50;		/* rate of drawing new circles */
int ourpensize = 1;		/* global pen size instead of windows */
Rect drag;			/* for moving our free mem manually */
char name[100];
char outstring[100];


/*********************/
/*   Odds and Ends   */
/*********************/

nop() 				/* Attached as a null pointer */
{
}			

/***********************/
/*  Sketch Window      */
/***********************/

sketactivate()			/* Attached to sketch's activate */
{				/* Disable the whole sketch menu */
  menu ("Sketch", "", itemenable);
  menu ("Edit", "Clear", home);  /* put our clear routine into place */
}
	
sketdeactivate()		/* Attached to sketch's deactivate */
{				/* Enable the whole sketch menu */
  menu ("Sketch", "", itemdisable);
  menu ("Edit", "Clear", nop);	/* take off our clear routine */
}

sketch(x,y)			/* follow mouse while it's down */
int x,y;			/* we will get passed the mouse Point */
{
  Point xy, lastxy;		/* points of interest */
  WindowPtr ourport;
  
  GetPort (&ourport);
  PenSize (ourpensize, ourpensize);
  MoveTo (x, y);		/* position pen at mouse down Point */
  while ( StillDown() ) {	/* while the Button is pressed */
    GetMouse (&xy);		/* read the mouse position */
    #ifdef MEGAMAX
    if ( (xy.a.v != lastxy.a.v) || (xy.a.h != lastxy.a.h) ) {
      LineTo (xy.a.h, xy.a.v);	/* if mouse moved, draw a Line to it */
      lastxy.a.v = xy.a.v;	/* record position so we can detect */
      lastxy.a.h = xy.a.h;	/*   a Move. */
    }				/* could runruns here to have circles */
    #else
    if ( (xy.v != lastxy.v) || (xy.h != lastxy.h) ) {
      LineTo (xy.h, xy.v);	/* if mouse moved, draw a Line to it */
      lastxy.v = xy.v;		/* record position so we can detect */
      lastxy.h = xy.h;		/*   a Move. */
    }				/* could runruns here to have circles */
    #endif
	runruns(0L);
	SetPort (ourport);
  }
}
    
setpensize(size)		/* Attached to pen size menus */
char *size;			/* menu Item name given to us */
{
  static char lastsize[5]={'1','\0'};	/* keep last setting */
  menu ("Sketch", lastsize, itemuncheck);	/* remove old check mark */
  menu ("Sketch", size, itemcheck);	/* install a new check mark */
  strcpy (lastsize, size);	/* save size for the next setpensize */
  ourpensize = *size - '0';		/* convert to a number */
}

/**************************/
/*   The Circles Window   */
/**************************/

int newabs(x)			/* an absolute value funtion */
int x;
{
  if (x < 0)
    x = -x;
  return (x);
}

circles()			/* draw a Random sized circle */
{
  static WindowPtr wind = (WindowPtr) 0; /* keep "circle" WindowPtr */
  static int speed = 1;		/* current count between new circles */
  register int cx, cy;		/* center of the circle */
  int r, b, rd;			/* right and bottom edges */
  register Pattern *color;	/* color for the circle */
  Rect therect;			/* rectangle to draw circle in */
  if ( --speed == 0) {		/* count, and if it reaches zero... */
    speed = circspeed;		/* reset the count */
    if ((long)wind == 0L) 	/* get window pointer if we need */
      wind = windowpoint ("Circles");  /* save window for next time */
    SetPort (wind);		/* set output to the circle window */
    #ifdef MEGAMAX
      r = wind->portRect.a.right;	/* get current size of window */
      b = wind->portRect.a.bottom;
    #else
      r = wind->portRect.right;	/* get current size of window */
      b = wind->portRect.bottom;
    #endif
    cx = newabs(Random()) % (r-30) + 1;	/* pick a Random location */
    cy = newabs(Random()) % (b-30) + 1;
    rd = newabs(Random()) % 25 + 5;		/* and size */
    SetRect ( &therect, cx, cy, cx+rd, cy+rd);	/* make a rectangle */
    switch ( newabs(Random()) % 4) {		/* pick a color */
      case 0: color = (Pattern *)(&dkGray); break;
      case 1: color = (Pattern *)(&ltGray); break;
      case 2: color = (Pattern *)(&gray); break;
      case 3: color = (Pattern *)(&black); break;
    }
    FillOval ( &therect, color);	/* make the circle */
  }
}

setspeed(name)				/* attached to circle's menu */
char *name;
{
  menu ("Circles","Slow/S", itemuncheck);	/* another way to uncheck last */
  menu ("Circles","Medium/M", itemuncheck); /* just uncheck everything */
  menu ("Circles","Fast/F", itemuncheck);
  menu ("Circles", name, itemcheck);	/* then check the current  */
  if (strcmp("Slow/S", name) ==0) circspeed = 100;  /* set the reset count */
  else if (strcmp("Medium/M", name) ==0) circspeed = 50;
  else if (strcmp("Fast/F", name) ==0) circspeed = 1;
}

/***********************************************/
/*   Generate a new sketch window by command   */
/***********************************************/

nwindow()			/* command to attatch to new window menu */
{
  static topplace = 100;	/* remember where to put next window */
  char newname[255];		/* string storage for window's name */
  strcpy (newname, "New Window");	/* default window name */
retry:				/* prompt for the name from user */
  if (prompt ("Give me a unique name for the new window:", newname)) {
    if ( (long)windowpoint(newname) != 0L ) {
      if (message ("Sorry, a window by that name already exists.")) 
        goto retry;		/* if exists, ask for another name */
    } else {
      if ( strlen(newname) > 0)  {  /* if ok, make the new window */
        /* then it is new */
        window (newname, 20,topplace, 200,topplace+60, 
	  sketactivate, sketdeactivate, nop, sketch);
        topplace += 5;		/* adjust top for next new window */
        if (topplace > 300) topplace = 100;  /* reset top if too low */
      }
    }
  }
}

/**********************/
/*  About SimpleDemo  */
/**********************/

usageinfo()		/* messages for Usage Info menu choice */
{
  char mess[255];	/* string to form a message */
  strcpy (mess,"You may use, distribute your programs which use, ");
  strcat (mess,"and copy SimpleTools if you send ");
  strcat (mess,"$20 or more to ...");
  if (message (mess)) {  /* if OK pressed for first message */
    strcpy (mess, "Erik Kilk\r4949 Snyder Lane, #247\r");
    strcat (mess, "Rohnert Park, CA  94928");
    if (message (mess))  /* if OK pressed for 2nd message */
      message ("Thank you for your support.");  /* show final message */
  }
}

tellabout()		/* for the About SimpleDemo menu choice */
{
  char mess[255];	/* string to form a message */
  strcpy (mess,"SimpleDemo -- Copyright 1986 Erik Kilk\r");
  strcat (mess,"A demonstration of the use of SimpleTools");
  strcat (mess," as a Macintosh toolbox aid.");
  if (message (mess))	/* display the string in a dialog */
    usageinfo();
}
  
/**************************/
/* The free memory window */
/**************************/

writemem(forsure)			/* write the FreeMem value */
int forsure;				/* true if force write */
{					/* false if only on change */
  static long lastmem;
  long mem;
  int foo;
  char thestring[10], outstr[10];
  Rect erasearea;
  mem = FreeMem();			/* get new free memory */
  foo = mem != lastmem;			/* compare it to last reading */
  if ( forsure | foo ) {		/* if forsure or different */
    withwindow ("FreeMem");		/* set window for output */
    home();
    MoveTo (0, 12);			/* write new value */
    NumToString (mem, thestring);
    #ifndef MEGAMAX
      PtoCstr (thestring);
    #endif
    strcpy (outstr, " Free ");
    strcat (outstr, thestring);
    #ifndef MEGAMAX
      CtoPstr (outstr);
    #endif
    DrawString (outstr);
    lastmem = mem;			/* record this for next time */
  }
}

showmem(windp)				/* write FreeMem on an update */
WindowPtr windp;
{
  EndUpdate (windp);			/* allow use of entire window */
  writemem(TRUE);			/* clear and write new value */
  BeginUpdate (windp);
}

newmemvalue()				/* write FreeMem occasionally */
{
  writemem(FALSE);			/* write only if value changed */
}

/* This is a special, non-standard, method of moving a window.  Our
   free memory window does not have a title bar.  So, I have decided
   to allow it to be moved if the mouse is clicked in its content
   Region.  This routine is attached to the free memory in content
   procedure with SimpleTools.  SimpleTools convenienly gives us
   various parameters, here we need the window pointer and the event
   record.  The event record has the coordinates of the mouse in
   globals */
   
movemem(x,y,windp,event)	/* free mem's inContent procedure */
int x,y;			/* SimpleTools gives us these for */
WindowPtr windp;		/* an in-content procedure */
EventRecord *event;
{
  #ifdef MEGAMAX
  	DragWindow (windp, &(event->where), &drag);
  #else
  	DragWindow (windp, event->where, &drag);
  #endif
}

/************************/
/*   Question window    */
/************************/

quest_update ()				/* repaint question window	*/
{
    stgotoxy (0, 0);			/* top left corner textwise	*/
    strcpy (outstring, "Hello, ");
    strcat (outstring, name);
    #ifndef MEGAMAX
    	CtoPstr(outstring);
    #endif
    DrawString(outstring);
}

quest_getname ()			/* get a name in quest window	*/
{
    getlinecaps = TRUE;			/* force each word to be cap'd	*/
    showawindow ("Questions");		/* make window active		*/
    quest_update ();
    stgotoxy (0, 2);			/* 3rd row, far left textwise	*/
    strcpy (outstring,"What is your name? ");  
    #ifndef MEGAMAX
    	CtoPstr (outstring);
    #endif
    DrawString (outstring);
    getline ("", name);			/* ask for name of user		*/
    home ();				/* erase the window		*/
    quest_update ();			/* repaint the window		*/
}

/**********************/
/*   Initialization   */
/**********************/

setup()		/* to be called with Start Demo menu choice */
{
  int temp;
  strcpy (name, "World");
  SetRect (&drag, 4, 24, 508, 338);  /* drag area for free mem window */
  windmenu = FALSE;	/* modify default values, optional */
  dogoaway = FALSE;	/* optional change */
  window("Circles", 20, 55, 245, 315,nop, nop, nop, nop); /* circles */
  temp = wprocid;	/* save value for an optional change */
  wprocid = 3; 		/* optional change to set to altDBoxProc */
  window ("FreeMem", 407, 322, 502, 337, nop, nop, showmem, movemem);
  wprocid = temp;	/* return to defaults */
  windmenu = TRUE;	
  dogoaway = TRUE;
  window ("Sketch", 265, 55, 490, 315,sketactivate,sketdeactivate, 
  	nop, sketch);	/* display the sketch pad window */
  window ("Questions", 146, 276, 368, 337, nop, nop, quest_update, nop);
  menu ( applestring, "About SimpleDemo", tellabout);
  menu ( applestring, "About SimpleDemo", itemenable);
  menu ( "File", "New", nwindow);		/* setup File menu */
  menu ( "File", "New", itemenable);
  menu ( "File", "Set Name", quest_getname);
  simplequits ();				/* Transfer and Quit	*/
  menu ("Circles", "Speed", itemdisable);	/* install circles menu */
  menu ("Circles", "Slow/S", setspeed);		/* optional key equivs */
  menu ("Circles", "Medium/M", setspeed);
  menu ("Circles", "Fast/F", setspeed);
  menu ("Circles", "Medium/M", itemcheck);
  menu ("Sketch", "Pen Size", itemdisable);  /* install sketches menu */
  menu ("Sketch", "", itemdisable);
  menu ("Sketch", "1", setpensize);
  menu ("Sketch", "1", itemcheck);
  menu ("Sketch", "2", setpensize);
  menu ("Sketch", "3", setpensize);
  menu ("Sketch", "4", setpensize);
  menu ("Sketch", "5", setpensize);
  menu ("Sketch", "6", setpensize);
  run (circles);		/* keep drawing circles */
  run (newmemvalue);		/* check if mem changed occasionally */
}

main()
{
  simpletools("About SimpleDemo");	/* setup Apple and Edit menus */
  setup();				/* setup our program */
  for (;;) simpleevents();		/* process events */
}
