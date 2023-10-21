#define LIGHTSPEED	{define either LIGHTSPEED or MEGAMAX or your own}

#include <stdio.h>

#ifdef MEGAMAX
  #include <menu.h>
  #include <win.h>
#endif

#ifdef LIGHTSPEED
  #include <MenuMgr.h>
  #include <WindowMgr.h>
#endif

#define itemdisable	0L
#define itemenable	1L
#define itemcheck	2L
#define itemuncheck	3L

extern char 		applestring[];
extern WindowPtr 	windowpoint();
extern MenuHandle 	mhand();
extern int 		windmenu;
extern int 		dogoaway;
extern int 		wprocid;
extern int		show_new_window;
extern int 		sizeredraw;
extern int		getlinecaps;
extern ProcPtr 		keydownproc;
extern ProcPtr 		autokeyproc;
extern void		home();
extern void		stnop();
