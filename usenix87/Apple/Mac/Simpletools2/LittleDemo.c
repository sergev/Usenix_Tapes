
/*  This is a very simple demonstration of a complete application
    using SimpleTools.
    
    Link this file with SimpleTools.o
    (SimpleTools.o is created by compiling SimpleTools.c)
 */
 
#include "simple.h"		/* SimpleTools header file	*/

#ifdef MEGAMAX
  #include <qd.h>		/* Quickdraw header file	*/
#endif

#ifdef LIGHTSPEED
  #include <Quickdraw.h>
#endif

gotbeep ()			/* to be executed when beep is picked */
{
  SysBeep (10);
}

inwindow (x, y)			/* executed when click in our window */
int x, y;
{
  Point m, lm;
  MoveTo (x, y);		/* draw a Point where the mouse is */
  LineTo (x, y);
  #ifdef MEGAMAX
    lm.a.h = x; lm.a.v = y;
    while (StillDown()) {	/* and keep drawing like a pencil  */
      GetMouse (&m);		/* but only when the mouse moves   */
      if ((m.a.h != lm.a.h) || (m.a.v != lm.a.v)) {
        LineTo (m.a.h, m.a.v);
        lm = m;
      }
    }
  #else
    lm.h = x; lm.v = y;
    while (StillDown()) {	/* and keep drawing like a pencil  */
      GetMouse (&m);		/* but only when the mouse moves   */
      if ((m.h != lm.h) || (m.v != lm.v)) {
        LineTo (m.h, m.v);
        lm = m;
      }
    }
  #endif
}

redraw ()			/* to redraw our window */
{
  MoveTo (50, 50);		/* draw a box the hard way */
  LineTo (50, 100);
  LineTo (100, 100);
  LineTo (100, 50);
  LineTo (50, 50);
}

no_edit ()			/* turn off edit menu (on activation) */
{
  menu ("Edit", "", itemdisable);
}

yes_edit ()			/* turn on edit menu (on deactivation) */
{
  menu ("Edit", "", itemenable);
}

aboutme()			/* About message */
{
  message ("LittleDemo\rSimpleTools demonstration\rBy Erik Kilk");
}

setup ()			/* Setup the menus and windows */
{
  menu (applestring, "About LittleDemo...", aboutme);
  				/* Default About is disabled */
  menu (applestring, "About LittleDemo...", itemenable);
  menu ("File", "Beep", gotbeep);
  simplequits ();
  window ("LittleDemo", 20, 50, 490, 325, 
  	no_edit, yes_edit, redraw, inwindow);
}

main ()
{
  simpletools ("About LittleDemo...");	/* Initialize SimpleTools	*/
  setup ();				/* Install our menus and window */
  for (;;) simpleevents ();		/* Handle all events		*/
}
