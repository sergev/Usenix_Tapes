

/*
	Title	: SimpleTools2.c
	Author	: Erik Kilk  Copyright 1985, 1987
	Dates	: June 7, 1985, June 3, 1986, November 8, 1986
		  November 28, 1986, April 17, 1987, April 21, 1987.

	SimpleTools is a collection of routines to aid programming
	simple "Macintosh looking" programs.  SimpleTools initializes
	the toolbox, monitors & acts upon events, and provides generic
	i/o routines for your application.  You initialize your program
	by letting SimpleTools know what windows and menus you want along
	with what functions SimpleTools should call when they are
	selected.

	The purpose of SimpleTools is to encourage you to program those
	simple programs or to pilot larger programs which you may not
	do due to the enormous effort required to use the Macintosh
	toolbox.  My goal was to study Inside Macintosh once to
	Create SimpleTools and then be able to forget most of the usages
	of the Toolbox routines.  Instead of thumbing through hundreds of
	pages of Inside Macintosh just to get something up and running,
	one need only remember a dozen generic calls.

	SimpleTools is very powerful, yet also very simple to use.  One
	can get a program up and running with desk accessories, windows,
	and menus in only a few minutes.  Advance features of SimpleTools
	allow you to retrieve enough information from SimpleTools to call
	any of the toolbox routines manually if need be.


=========================================================================

	You may use, study, copy, and freely distribute SimpleTools if:
		1)  You mention "Programmed with the aid of SimpleTools
		    (c) Erik Kilk 1986" in your About... window of all
		    programs distributed free, share, or marketted.
		2)  You register by sending $20 or more to:
			Erik Kilk
			4949 Snyder Lane, #247
			Rohnert Park, CA  94928
			(707) 794-2424 weekday afternoons
		    to encourage me to maintain and improve SimpleTools.

==>	For a diskette including the most recent version of SimpleTools,
	several detailed examples of using SimpleTools, and a MacWrite file
	describing SimpleTools and its use in more detail, send me a
	diskette with enough stamps to mail it back to you.  Make sure you
	have registered as stated above.

	This is 128k, MFS, HFS, old ROM, new ROM, Mac+, & TMON compatible.
	This file compiles and executes with Megamax 3.0 beta and
	LightSpeed 2.1.  Adjust the definition in simple.h for your
	compiler.  When porting to other compilers, pay particular
	attention to where the Lightspeed and Megamax code is specified
	since these places are the likely problem areas.

	LIGHTSPEED NOTE:

	Drag SimpleTools out of the main segment in your project window.
	You do this by dragging it below the dotted horizontal Line in the
	project window.

	SimpleTools requires the MacTraps library and stringsasm.c or
	strings.c.  If you load the unix library, your project will be
	larger than needed (unless you need unix for your own program.)

	MEGAMAX NOTE:

	Use Megamax's convert utility to convert all Mac names to all
	lower case.  If you send me suggestions and/or new code for
	SimpleTools, please convert back to mixed case first.

=========================================================================

	SimpleTools provides the following functions for your application
	to call.  Note that your application need not call any Toolbox
	routines directly.  The entire C program (including the standard
	desk accessory support):

			main ()
			{
				simpletools ("Skel");
				simplequits ();
				runsimpletools ();
			}

	will execute as a Macintosh program, terminating upon the user
	selecting Quit.  SimpleTools includes:

		simpletools ()		- init Toolbox and SimpleTools
		simplequits ()		- add Transfer & Quit menus
		simpleevents ()		- process next Mac event
		runsimpletools ()	- continuously process events

		menu ("File","New",new) - install a menu
		window ("My Window",..)	- install a window

		run (function)		- install a periodic function
		stop (function)		- remove a periodic function

		havenewrom ()		- test for new 128k ROM
		withwindow("My Window")	- set output to a window

		stgotoxy (x, y)		- set pen to text position x, y
		home ()			- clear window, set pen to home
		getline (deflt, dest)	- with TE editing, get a Line
		prompt (question, dest)	- with dialog box & TE, get a Line
		message (string)	- with dialog box, print a string
		getfile ("TEXT", name)	- with list, select a filename
		putfile (orig, name)	- with list, select a filename


	A complete Macintosh Style application (including windowing and
	menus) is given in the following trivial example...

		#include <simple.h>

		char name[50];

		got_beep()
		{
			SysBeep (10);
		}

		got_getname()
		{
			char newname[50];
			*newname = 0;
			if (prompt ("What is your name?", newname)) {
				strcpy (name, newname);
				withwindow ("My Window");
				home();
				update();
			}
		}

		in_content(x, y)
		int x, y;
		{
			MoveTo (x, y); LineTo (x, y);
		}

		update()
		{
			char outstring[100];
			stgotoxy (1, 5);
			sprintf (outstring,"Hello, %s", name);
			#ifndef MEGAMAX
			  CtoPstr (outstring);
			#endif
			DrawString (outstring);
		}

		main ()
		{
			simpletools ("Sample Program");
			simplequits ();
			menu ("Commands", "Beep", got_beep);
			menu ("Commands", "Get Name", got_getname);
			strcpy (name, "World");
			window ("My Window",0,0,0,0,0L,0L,update,
				in_content);
			runsimpletools ();
		}

=========================================================================

	ROUTINES YOUR APPLICATION MAY CALL:


	simpleevents ()

	To be called repeatedly by your program's main routine.  In
	most SimpleTools programs, your main routine will initialize
	and install SimpleTools followed by an loop such as:

		for (;;) simpleevents();

	This routine handles all window changes, menu requests, and
	other Macintosh events.  There is also a routine called with
	runsimpletools() which does not return.  It simply performs the
	above loop.  Program exit is accomplished by assigning an
	exiting routine to a menu, usually this is File/Quit.


	simpletools (about_string)
	char *about_string;

	To be called once at the very beginning of your main routine.
	This routine initializes all the Macintosh software managers
	and installs the basic Apple, File, and Edit menus.  The
	about_string is the name of the menu Item to appear first under
	the Apple menu.


	simplequits ()

	Installs a simple File/Quit and File/Transfer menu.  You only
	want to installs these if no application dependent processing
	must be done when the user selects Quit or Transfer.


	menu (name, Item, routine)
	char 	*name;
	char 	*Item;
	ProcPtr	routine;

	To be called when a new menu is to be installed on the menu bar
	or when the characteristics of that menu are to be modified.
	Name is the name of the menu to appear on the menu bar.  Item
	is the name of the Item to appear under the menu name.  Routine
	is the name of the routine to be executed when the stated menu/Item
	has been selected.  The characteristics of the menu may be changed
	by passing one of the constants itemdisable, itemenable, itemcheck,
	or itemuncheck in place of the routine.


	window (name, xtop, ytop, xbot, ybot, act, deact, update, content)
	char 	*name;
	int  	xtop, ytop;
	int  	xbot, ybot;
	ProcPtr	act, deact, update, content;

	To be called when a new window is to be installed on the screen
	or when the characteristics of that window are to be modified.
	Name is the name of the window.  Xtop, ytop, xbot, and ybot are
	the initial coordinates of the new window.  Act is the name of
	the procedure to execute when the window becomes the top or
	active window.  Deact is the name of the procedure to execute when
	the window ceases being the top window and deactivates.  Update
	is the name of the procedure called when the Macintosh needs to
	redraw the window's contents.  Content is the name of the procedure
	called when the mouse is pressed within the window.  The procedure
	specified as content will be passed an x and y integer value
	representing the local mouse coordinates.


	withwindow (name)
	char *name;

	To be called when you want to select which window will receive
	output and drawings.  In most cases, SimpleTools will select the
	appropriate window before calling your specified act, deact,
	update, or content procedures.  Use this at other times.


	run (routine)
	ProcPtr	routine;

	To be called when you want a routine to be continuously executed
	once each time simpleevents() is called.  Small, quickly running
	routines should be used so as not to Delay the next event
	processing.  Pseudo multiprocessing with each routine running in
	its own window can be accomplished by making sure a run routine
	uses withwindow() to direct its output to the proper window.


	stop (routine)
	ProcPtr routine;

	To be called when you want to remove a previously run() routine
	from the list of routines to run.  50 routines can fit into the
	run list.


	home ()

	Clears the current window and positions the pen such that any
	following text will appear in the upper left corner of the
	window.


	stgotoxy (column, row)
	int column, row;

	Positions the pen in the current window so that the next text
	output will appear in text row and column.  This is compatible
	with some old text only terminals.  stgotoxy (1, 1) positions the
	pen in the upper left corner.  Any negative coordinate leaves
	that axis of the pen where it currently is.


	getline (default, destination)
	char *default;
	char *destination;

	Calling this routine begins a "modal" mode where the user is
	required to enter a Line of text.  This would be similar to
	using scanf() on "non-Mac", text-only terminals.  This routine
	uses the Macintoshes built in Text-Edit routines allowing the
	user to edit his Line until <RETURN> is pressed.  The flashing
	bar Cursor is positioned at the current pen location.  The
	resulting string is placed into destination.  Default contains
	the initial value to be displayed on the screen.  You may use
	the null string "" for default.


	prompt (question, answer)
	char *question;
	char *answer;

	This routine places a small 3-lined Macintosh Style dialog window
	prompting the user with question and getting the answer in a
	boxed Text-Edit area.  Two buttons are displayed to terminate the
	user entry.  If Cancel is clicked upon, FALSE is returned.  If
	okay is clicked upon, TRUE is returned.  Answer must be set to
	a default value, "" is okay.


	message (message)
	char *message;

	This routine is similar to prompt except no textual response is
	asked from the user.  This is like an Alert dialog.  Just like
	prompt, TRUE or FALSE is returned depending upon which Button the
	user presses.


	getfile (file_type, reply)
	char *file_type;
	char *replay;

	This routine places the standard Macintosh SFGetFile() window
	up with a list of files of file_type.  Once the user selects a
	file, the answer is returned in the string reply.  Also, and
	very important for HFS, the working volume/folder is set so that
	any subsequent open() with reply as the file name will open the
	correct selected file.  The open() should be done before someone
	has a chance to change the working volume.  This routine will
	return FALSE if the user selects the CANCEL Button.


	putfile (origname, reply)
	char *origname;
	char *reply;

	This routine is like getfile, except the standard putfile window
	is displayed with origname as the default name to save a file as.
	The actual name selected by the user is returned in reply.  As
	getfile, the working volume/folder is set properly for the next
	open() call.

=========================================================================

	THE FOLLOWING IS THE FILE simple.h.  YOU SHOULD COPY THIS PORTION
	INTO A NEW FILE NAMED simple.h SO YOU CAN #include IT INTO YOUR
	SOURCE FILES.

=========================================================================

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
=========================================================================

		Here begins SimpleTools.c
		_________________________
*/

#include "simple.h"			/* define compiler in here	*/

#ifdef MEGAMAX
        overlay "simpletools"		/* compiler dependent		*/

	#include <mem.h>
	#include <qd.h>
	#include <qdvars.h>
	#include <misc.h>
	#include <event.h>
	#include <res.h>
	#include <win.h>
	#include <dialog.h>
	#include <menu.h>
	#include <string.h>
	#include <stdio.h>
	#include <pack.h>
	#include <te.h>
	#include <toolbox.h>

	#define  ZZ	&
#else
	#include <MemoryMgr.h>
	#include <Quickdraw.h>
	#include <EventMgr.h>
	#include <ResourceMgr.h>
	#include <WindowMgr.h>
	#include <TextEdit.h>
	#include <DialogMgr.h>
	#include <MenuMgr.h>
	#include <strings.h>
	#include <stdio.h>
	#include <PackageMgr.h>
	#include <ToolboxUtil.h>
	#include <StdFilePkg.h>
	#include <pascal.h>

	#define  ZZ
#endif

#define TRUE (-1)		/* local definitions 			*/
#define FALSE 0
#define maxsruns 50		/* procedure table size			*/
#define MESSN 30		/* array size for message dialog items	*/
#define QUESN 40		/* array size for prompt dialog items	*/
#define ROM85   0x28E		/* new rom stuff 			*/
#define NEWROM  0x7FFF
#define inzoomout 8
#define inzoomin  7
#define zoomproc  8

typedef struct {			/* structure for an Item	*/
	char		itemname[40];
	int		itemno;		/* Item number within menu 	*/
	int		menuId;		/* menu id			*/
	MenuHandle	menuhand;	/* Item's menu's Handle 				*/
	ProcPtr		menurun;	/* procedure to run 		*/
	Ptr		next;		/* pointer to the next Item 	*/
} itemdatum;

typedef struct {			/* structure for a menu 	*/
	char 		menuname[20];	/* to allow reference by name 	*/
	int 		menuId;		/* menu id 			*/
	MenuHandle	menuhand;	/* menu Handle to reference menu*/
	itemdatum	**itemlist;	/* pointer to the list of items */
	Ptr 		next;		/* pointer to the next menu 	*/
} menudatum;

typedef struct {			/* structure for a window 	*/
	char		windname[80];	/* window's name and reference 	*/
	WindowPtr	wptr;		/* window's pointer reference 	*/
	ProcPtr	wact;			/* the activate procedure 	*/
	ProcPtr	wdeact;			/* the deactivate procedure 	*/
	ProcPtr	wupdate;		/* the update procedure 	*/
	ProcPtr	wcontent;		/* the content procedure 	*/
	Ptr		next;		/* pointer to the next window 	*/
} winddatum;

#ifdef LIGHTSPEED
  pascal Boolean 	*TrackBox() = 0xA83B;
  pascal void 		*ZoomWindow() = 0xA83A;
#endif

WindowPtr windowpoint();

/* Local variables */

menudatum 	**simplemenus;		/* Handle to menu data 		*/
char 		accname[80];		/* desk accessory name to open 	*/
Rect 		dragrect, sizerect;	/* limits for moving windows 	*/
Rect 		swholescreen;
winddatum	**simplewinds;		/* Handle to window data 	*/
int		firstwind;		/* if no windows have been made	*/
ProcPtr 	simpleruns[maxsruns];	/* list of procedures to run 	*/
WindowPtr	debugw;			/* window pointer for debugging */
int		snewrom;
int		getlinecaps = FALSE;

/************************************************************************/
/* GLOBAL USER MODIFIABLE VARIABLES 					*/
/* These are variables that you can declare extern so that you can use	*/
/* them to change the SimpleTools defaults				*/
/************************************************************************/

/* wprocid = type of window to Create on next window() call		*/
/* For LightSpeed C, use a lower case d and upper case P for 		*/
/* 	DocumentProc.  Megamax conversion program does this wrong.	*/

int 		wprocid = documentProc;

/* dogoaway = is the next created window to have a go-away box		*/

int 		dogoaway = TRUE;

/* keydownproc = the procedure to be called when keyDown is detected	*/
/* autokeyproc = the procedure to be called when autoKey is detected	*/
/* BOTH OF THESE ARE PASSED THE EVENTRECORD SO THE KEY CAN BE FOUND	*/

ProcPtr 	keydownproc,
  		autokeyproc; 

/* applestring = a string containing the Apple for menu reference	*/

char 		applestring[2]
  		= {'\024', '\0'}; 

/* windmenu = does the next window created get a choice under the
              window menu (so a closed window can be brought back	*/
          
int 		windmenu = TRUE;

/* sizeredraw = is the window's content area erased and made
                updateable upon being resized				*/
            
int		sizeredraw = FALSE;

/* show_new_window = does the created window get displayed right away
		on the screen, if not, then it is hidden		*/

int		show_new_window = TRUE;


/******************************************************************/
/* Dialog lists.  These were calculated by using the new resource */
/* editor to make a template for a dialog and then using fedit to */
/* list the hex listing of the Item list for the dialog.	  */
/******************************************************************/

int messd[MESSN] = {2,0,0,0x38,0xf1,0x4c,0x12d,0x402,0x4f4b,0,0,5,5,
		0x36,0x12d,0x800,0,0,0x38,0xac,0x4c,0xe8,0x406,
		0x4361,0x6e63,0x656c};
int quesd[QUESN] = {3,0,0,0x21,0xf0,0x35,0x12c,0x402,0x4f4b,0,0,8,8,
		0x28,0xe8,0x800,0,0,0x2b,8,0x4b,0xe8,0x1000,0,0,
		8,0xf0,0x1c,0x12c,0x406,0x4361,0x6e63,0x656c};


/* Local procedure */

void stnop()				/* a no op procedure for defaults */
{
}

char *ptoc(s)
char *s;
{
	#ifndef MEGAMAX
		return (PtoCstr(s));
	#else
		return (s);
	#endif
}

char *ctop(s)
char *s;
{
	#ifndef MEGAMAX
		return (CtoPstr(s));
	#else
		return (s);
	#endif
}

/* Given a menu name, find our data structure for it.  Return a Handle
   to this structure.  This is a local procedure for SimpleTools use.
*/

/* local procedure */

menudatum **getourmenuhandle (name)
char *name;				/* name of menu bar menu */
{
	menudatum **here, **temp;	/* hand to menu structure*/
	here = simplemenus;

	/* find the menu name or the end of out menu list */
	HLock (here);
	while (strcmp(name,(**here).menuname) && (**here).next )  {
		temp = here;
		here = (menudatum **)(**here).next;
		HUnlock (temp);
		HLock (here);
	}

	/* see if we found it or just the end of the list */
	if (!strcmp(name,(**here).menuname)) {
		HUnlock (here);
		return (here);
	} else {
		HUnlock (here);
		return ((menudatum **)0L);
	}
}

/* This takes a Handle to our personal Item record and either a
   procedure name or a modifier code.  If it got a procedure name,
   it sets it to the Item's procedure to run when the Item is chosen.
   If it got a modifier code, it changes the state of the menu's Item
   to checked, unchecked, enabled, or disabled.  It especially keeps
   track of the standard Edit menu items so we can restore them after
   a desk accessory is finished.
*/

/* Local procedure */

setitems ( items, routine)	/* set a menu Item's routine or display */
itemdatum	**items;	/* if items is neg, then whole menu 	*/
ProcPtr	routine;
{
	int			inumber;
	MenuHandle		mhand;

	/* check to see if a procedure pointer was given to us */
	if ( (((long)items)>0L) && (routine > (ProcPtr)0x1000L)) {
						/* good procedure value */
		(**items).menurun = routine;
		return;
	}

	/* Calculate which Item number we are going to modify */
	if ( (long)items < 0L) {		/* the whole menu 	*/
		mhand = (MenuHandle) (0L - (long)items);
		inumber = 0;
	} else {				/* just one Item 	*/
		mhand = (**items).menuhand;
		inumber = (**items).itemno;
	}

	/* If a NULL procedure pointer, then set to a no_op routine */
	if ( (inumber > 0) && ((**items).menurun == (ProcPtr)0L) )
		(**items).menurun = (ProcPtr) stnop;

	/* Now change the state of a menu Item */
	switch ((int)routine) {
		case itemdisable:
			DisableItem(mhand,inumber); break;
		case itemenable:
			EnableItem(mhand, inumber); break;
		case itemcheck:
			CheckItem(mhand, inumber, TRUE); break;
		case itemuncheck:
			CheckItem(mhand, inumber, FALSE); break;
	}
	if (inumber == 0) DrawMenuBar();  /* if main menu was changed 	*/

}

/* This routine is called by the simpletools() initial routine.  It gets
   the pointer list of menus started, loads the desk accessories into
   the Apple menu, and loads up some standard menu entries.  The reason
   menu File has a New entry, and none others, is because as this code
   currently stands, a menu must have at least one Item.  And since we
   want File before Edit, I had to make an entry.  The most commonly used
   Item under File is Quit.  But we like quit to be at the end of the list.
   So, since New is usually always first when it is used, that the one
   chosen to start File.
*/

/* Local procedure */

initsmenus(about)			/* init simpletools' menus */
char *about;
{
	itemdatum **items;

	simplemenus = (menudatum **) NewHandle ( (long)sizeof(menudatum));
	HLock (simplemenus);

	strcpy ( (**simplemenus).menuname, applestring);
	(**simplemenus).menuId = 1;
	(**simplemenus).next = (Ptr) 0L;
	ctop ((**simplemenus).menuname);
	(**simplemenus).menuhand = NewMenu (1, (**simplemenus).menuname);
	ptoc ((**simplemenus).menuname);
	HUnlock ((**simplemenus).menuhand);

	(**simplemenus).itemlist = (itemdatum **)NewHandle (
			(long)sizeof(itemdatum));
	items = (itemdatum **) (**simplemenus).itemlist;
	HLock (items);

	strcpy ((**items).itemname, about);
	(**items).itemno = 1;
	(**items).menuId = 1;
	(**items).menuhand = (**simplemenus).menuhand;
	(**items).menurun = (ProcPtr) stnop;
	(**items).next = 0L;
	HUnlock (items);

	ctop (about);
	AppendMenu ((**simplemenus).menuhand, about);
	ptoc (about);
	DisableItem ((**simplemenus).menuhand, 1);
	menu (applestring, "-", (ProcPtr) itemdisable);
	#ifdef MEGAMAX
	  AddResMenu ((**simplemenus).menuhand, "DRVR");
	#else
	  AddResMenu ((**simplemenus).menuhand, 'DRVR');
	#endif
	InsertMenu ((**simplemenus).menuhand, 0);
	HUnlock (simplemenus);

	menu ("File", "New", (ProcPtr)itemdisable);
	menu ("Edit", "Undo", stnop);
	menu ("Edit", "-", (ProcPtr)itemdisable);
	menu ("Edit", "Cut/X", stnop);
	menu ("Edit", "Copy/C", stnop);
	menu ("Edit", "Paste/V", stnop);
	menu ("Edit", "Clear", stnop);
}

/* Local procedure */

#ifndef LIGHTSPEED
gottrans ()
{
  char prog[80];
  char *argv[3];
  if ( getfile("APPL", prog) ) {
    argv[1] = NULL;
    execv (prog, argv);
  }
}
#endif

/* Local procedure */

gotquit ()
{
  ExitToShell();
}

/* This routine is for the Windows menu Item.  The Windows menu is
   set up when new windows are added.  It is used to bring forward and
   bring into view windows that may be under other windows or have been
   sent hiding by a click on their close box.
*/

/* Local procedure */

showawindow(name)			/* show the named window	*/
char *name;
{
	WindowPtr foo;
	foo = windowpoint(name);	/* get its window pointer	*/
	if ( foo ) {
		ShowWindow(foo);	/* show it on the screen	*/
		SetPort (foo);		/* set further output to it */
		if ( foo != FrontWindow())	/* if it isn't active,	*/
			SelectWindow (foo);	/* activate it 		*/
	}
}

/* Local procedure */

winddatum **wdatum(windpt)		/* return Handle to window data */
WindowPtr windpt;
{
	winddatum **wind, **temp;

	if (firstwind) return ((winddatum **) 0L);
	wind = simplewinds;
	HLock (wind);

	while ( ((**wind).wptr != windpt) && (**wind).next) {
		temp = wind;
		wind = (winddatum **) (**wind).next;
		HUnlock (temp);
		HLock (wind);
	}

	if ( (**wind).wptr == windpt) {
		HUnlock (wind);
		return (wind);
	} else {
		HUnlock (wind);
		return ((winddatum **) 0L);	/* zero if not found */
	}
}

/* Local procedure */

runruns(event)			/* run all the installed run procedures	*/
EventRecord *event;		/* returns number of routines run 	*/
{
	int i=0;
	WindowPtr saveport;
	GetPort (&saveport);
	while ( simpleruns[i] )
		(*(simpleruns[i++])) (event);
	SetPort (saveport);
	return(i);
}

/* Local procedure */

stdialog( question, answer, type)  /* a general dialog displayer 	*/
char *question;
char *answer;
int  type;			/* type:  1=prompt, 2=message 		*/
{
	DialogPtr dialog;	/* dialog reference 			*/
	Handle Item, items;	/* handles for the dialog items 	*/
	Rect screen, box;	/* rectangles for dialog/items 		*/
	int dtype, hit, canc;	/* Item type and which was hit 		*/
	char tempanswer[255];	/* address where answer is 		*/

	items = NewHandle (512L);/* get memory for items list 		*/
	HLock (items);		/* lock it down 			*/
	if (type == 1)
		BlockMove (quesd, *items, (long) QUESN * 2L);
	else
		BlockMove (messd, *items, (long) MESSN * 2L);
	SetRect (&screen, 103, 50, 409, 137);

	/* For LIGHTSPEED, use a lower case d and upper case B and P	*/
	/* for DBoxProc.  Megamax conversion utility does this wrong.	*/

	dialog = NewDialog (0L, &screen, "", 0, dBoxProc, -1L, 0, 0L, items);
	GetDItem (dialog, 2, &dtype, &Item, &box);
	ctop (question);
	SetIText (Item, question);		/* set Item#2 text 	*/
	ptoc (question);
	if (type == 1) {			/* set default answer	*/
		GetDItem (dialog, 3, &dtype, &Item, &box);
		ctop (answer);
		SetIText (Item, answer);
		ptoc (answer);
		canc = 4;
	} else
		canc = 3;
	ShowWindow (dialog);			/* display the dialog	*/
	do {
		ModalDialog (0L, &hit);		/* process the dialog	*/
	} while ( (hit != 1) & (hit != canc) );
	if (type == 1) {
		GetDItem (dialog, 3, &dtype, &Item, &box);
		HLock (Item);
		GetIText (Item, tempanswer);	/* get Item#3 text 	*/
		ptoc (tempanswer);
		strcpy (answer, tempanswer);	/* make a copy of it 	*/
		HUnlock (Item);
	}
	HUnlock(items);				/* unlock items memory	*/
	HPurge(items);				/* purge it 		*/
	DisposDialog (dialog);			/* get rid of dialog	*/
	return ( hit==1 );			/* return true if ok	*/
}

/* Local procedures */

docommand (which, thisevent)
long which;
EventRecord *thisevent;
{
	int themenu, theitem;
	long size;
	char *cpoint;
	GrafPtr tempport;
	menudatum **here, **temp;
	itemdatum **items, **tempitems;
	char **myreshandle;
	Handle myhandle;

	themenu = HiWord (which);
	theitem = LoWord (which);
	if ((themenu == 1) && (theitem != 1)) {

		/* start up a desk accessory */
		HLock (simplemenus);
		GetItem ((**simplemenus).menuhand, theitem, accname);
		SetResLoad (FALSE);
		#ifdef MEGAMAX
		  myreshandle = GetNamedResource ("DRVR", accname);
		#else
		  myreshandle = GetNamedResource ('DRVR', accname);
		#endif
		SetResLoad (TRUE);
		size = SizeResource (myreshandle);
		myhandle = NewHandle ( size + 3072L);
		if (myhandle == 0L)
			message ("Not enough memory to do that.");
		else {
			DisposHandle (myhandle);
			GetPort (&tempport);
			OpenDeskAcc(accname);
			SetPort (tempport);
		}
		HUnlock (simplemenus);
		return;
	}
	if (themenu ==3) {
		/* do any system edits */
		if (SystemEdit(theitem -1))  return;
	}

	/* now we run an installed menu procedure */
	here = simplemenus;
	HLock (here);

	/* find out menu structure given the menu id */
	while ( ((**here).menuId != themenu) && (**here).next) {
		temp = here;
		here = (menudatum **)(**here).next;
		HUnlock (temp);
		HLock (here);
	}

	if ((**here).menuId == themenu) {

		/* now find the Item structure */
		items = (**here).itemlist;
		HUnlock (here);
		HLock (items);

		while ( ((**items).itemno != theitem) && (**items).next) {
			tempitems = items;
			items = (itemdatum **)(**items).next;
			HUnlock (tempitems);
			HLock (items);
		}

		/* prepare to give the Item name to the procedure */
		cpoint = (**items).itemname;
		if ((**items).itemno == theitem)
		/* if we found the Item, call its procedure */
			(*((**items).menurun))(cpoint,thisevent) ;
		HUnlock (items);
	}
}

/* Local procedure */

domousedown(thisevent)		/* respond to mouse down events */
EventRecord *thisevent;		/* passed the event record */
{
	WindowPtr whichwindow;
	int code, x, y;
	char *cpoint;
	menudatum **omhand;
	winddatum **thewdatum;
	long newplace;
	Point temp;
	GrafPtr saveport;

	code = FindWindow(ZZ(thisevent->where), &whichwindow);
	switch (code) {
	    case inMenuBar:
		docommand(MenuSelect(ZZ(thisevent->where)),thisevent);
		break;
	    case inSysWindow:
		SystemClick(thisevent, whichwindow); break;
	    case inDrag:
		DragWindow(whichwindow, ZZ(thisevent->where),
			&dragrect); break;
	    case inGrow:
		newplace= GrowWindow(whichwindow, ZZ(thisevent->where),
			&sizerect);
		SizeWindow(whichwindow, LoWord(newplace),
		HiWord(newplace), TRUE);
		if (sizeredraw) {
			GetPort (&saveport);
			SetPort (whichwindow);
			EraseRect (&swholescreen);
			InvalRect (&swholescreen);
			SetPort (saveport);
		}
		break;
	    case inGoAway:
		if ( TrackGoAway(whichwindow, ZZ(thisevent->where))) {
			HideWindow (whichwindow);
		}
		break;
	    case inzoomout:
	    case inzoomin:
	        #ifdef MEGAMAX
		  if ( trackbox(whichwindow, ZZ(thisevent->where), code)) {
			zoomwindow (whichwindow, code, 0);
		#else
		  if ( TrackBox(whichwindow, ZZ(thisevent->where), code)) {
			ZoomWindow (whichwindow, code, 0);
		#endif
			GetPort (&saveport);
			SetPort (whichwindow);
			EraseRect (&swholescreen);
			InvalRect (&swholescreen);
			SetPort (saveport);
		}
		break;
	    case inContent:

		/* make the window active if it isn't yet */
		if (whichwindow != FrontWindow()) {
			SelectWindow(whichwindow);
		}

		/* find our window data */
		thewdatum = wdatum (whichwindow);
		if (thewdatum) {

			/* convert the Point of click to the window's
			   own coordinates since this will be always
			   more useful than the global coordintates */
			temp = thisevent->where;
			SetPort (whichwindow);
			GlobalToLocal (&temp);
			#ifdef MEGAMAX
			  x = temp.a.h;
			  y = temp.a.v;
			#else
			  x = temp.h;
			  y = temp.v;
			#endif

			/* call the window's in content routine */
			HLock (thewdatum);
			(*((**thewdatum).wcontent))(x, y, whichwindow,
				thisevent);
			HUnlock (thewdatum);
		}
		break;
	}
}


/************************************************************************/
/* 	GLOBAL ROUTINES INTENDED TO BE USER CALLABLE PROCEDURES		*/
/* THE FOLLOWING PROCEDURES HAVE BEEN WRITTEN FOR THE USER'S 		*/
/* APPLICATION TO CALL.    						*/
/* 									*/
/************************************************************************/

havenewrom ()			/* returns true if new roms installed	*/
{
	return ((*((int *)ROM85)) == NEWROM);
}

/* Menu is usually called like:

		menu ("File", "Print...", got_print)

   where the first argument is the name appearing on the menubar.  The
   2nd argument is the name appearing when the menu is pulled down.  The
   3rd argument is USUALLY the routine to be called when the user
   selects this particular menu.  Non-existant menus are created following
   the last.  The menu ordering may never be changed once created.
   Existint menus have their "routine-to-be-executed" assignment changed
   to the new routine.  If the long values 0L, 1L, 2L, or 3L are passed
   instead of a procedure, the menu characteristic is set as specified
   by the constants itemdisable, itemenable, itemcheck, itemuncheck.  For
   example:
   		menu ("File", "Print...", itemdisable)

   PROCEDURES ASSIGNED TO MENUS ARE CALLED WITH TWO ARGUMENTS.  YOU
   DO NOT NEED TO DECLARE THESE IN YOUR PROCEDURE IF YOU DO NOT USE
   THEM.  FOR EXAMPLE, GOT_PRINT MAY BE DECLARED AS:

   		got_print()
		char *itemname;
		EventRecord *current_event;
		{
			...
		}

   Itemname is a char* pointing to the Item name.  This allows the same
   menu procedure to be used for multiple menu/Item pairs.  Maybe your
   Size menu just as items 9 Point, 10 Point, 12 Point, etc.  This way
   you can specify the same procedure for each and determine what to do
   by looking at itemname.

   Current_event is a pointer to the current EventRecord that detected
   the menu selection.  You may look at this as needed.
*/

menu (name, Item, routine)		/* install or change a menu	*/
char *name;				/* the menu name 		*/
char *Item;				/* the Item name 		*/
ProcPtr routine;			/* a procedure or modifier 	*/
{
	menudatum **here,**temp;	/* a roving Handle to our data 	*/
	menudatum **ourmhandle;		/* another Handle to our data 	*/
	itemdatum **items,**tempitems;	/* a Handle to the Item 	*/
	int lastid, lastitem;

	/* get the Handle to menu named 'name' */
	if ((ourmhandle = getourmenuhandle (name)) == 0L) {

		/* make a new menu entry by finding the end of the list */
		here = simplemenus;
		HLock (here);
		while ((**here).next) {
			temp = here;
			here = (menudatum **)(**here).next;
			HUnlock (temp);
			HLock (here);
		}

		/* make a structure for our new entry */
		lastid = (**here).menuId;
		(**here).next = (Ptr)NewHandle ( (long)sizeof(menudatum));
		temp = here;
		here = (menudatum **)(**here).next;
		HUnlock (temp);
		HLock (here);

		strcpy ( (**here).menuname, name);
		(**here).menuId = ++lastid;
		(**here).next = (Ptr) 0L;

		/* make a new Item structure */
		(**here).itemlist = (itemdatum **)NewHandle (
			(long)sizeof(itemdatum));

		/* make a new menu entry for the Macintosh */
		ctop (name);
		(**here).menuhand = NewMenu (lastid, name);
		ptoc (name);
		items = (**here).itemlist;

		HLock (items);
		strcpy ((**items).itemname, Item);
		(**items).itemno = 1;
		(**items).menuId = lastid;
		(**items).menuhand = (**here).menuhand;
		(**items).menurun = (ProcPtr) 0L;
		(**items).next = 0L;
		HUnlock (items);

		/* install and display the menu */
		ctop (Item);
		AppendMenu ((**here).menuhand, Item);
		ptoc (Item);
		InsertMenu ((**here).menuhand,0);
		HUnlock (here);

		setitems (items, routine);
		DrawMenuBar();
		return(TRUE);
	}
	else {
		HLock (ourmhandle);

		if (strlen(Item) == 0) {
		  /* then adjust main menu */
		  setitems( 0L - (long) ((**ourmhandle).menuhand), routine);
		  return(FALSE);
		}

		/* see if Item is in list */
		items = (**ourmhandle).itemlist;
		HLock (items);

		while ( strcmp(Item,(**items).itemname) && (**items).next) {
			tempitems = items;
			items = (itemdatum **)(**items).next;
			HUnlock (tempitems);
			HLock (items);
		}
		if (strcmp(Item,(**items).itemname) ==0) {
			setitems( items, routine);
			return(FALSE);
		}
		else {
			/* make new Item entry */
			lastitem = (**items).itemno;
			(**items).next =(Ptr)NewHandle((long)sizeof(itemdatum));
			tempitems = items;
			items = (itemdatum **)(**items).next;
			HUnlock (tempitems);
			HLock (items);

			strcpy ((**items).itemname, Item);
			(**items).itemno = ++lastitem;
			(**items).menuId = (**ourmhandle).menuId;
			(**items).menuhand = (**ourmhandle).menuhand;
			(**items).menurun = (ProcPtr) 0L;
			(**items).next = 0L;
			HUnlock (items);

			/* and install the Item in the menu bar */
			ctop (Item);
			AppendMenu ((**ourmhandle).menuhand,Item);
			ptoc (Item);
			HUnlock (ourmhandle);
			setitems (items, routine);
			return(TRUE);
		}
	}
}

/* Given a menu name, return the real menu Handle as used by most
   of the Macintosh toolbox menu manager routines.
*/

MenuHandle mhand (name)			/* find MenuHandle		*/
char *name;				/* given name of menu 		*/
{
	menudatum	**menu;
	MenuHandle	temp;		/* a Handle to our data 	*/

	menu = getourmenuhandle(name);
	if ( menu ) {
		HLock (menu);
		temp = (**menu).menuhand;
		HUnlock (menu);
		return ( temp );	/* return menu Handle 		*/
	} else
		return ( (MenuHandle) 0 );
}


/*  Call this routine if you want these SimpleTools defined quiting	*/
/*  procedures.  You may just install your own instead.	 The time to	*/
/*  call this is after you have installed all your other "File" items.	*/
/*  By calling this last, you will place Transfer and Quit on the end	*/
/*  of the menu list.							*/

simplequits ()
{
	menu ("File", "-", itemdisable);

	#ifndef LIGHTSPEED
	menu ("File", "Transfer.../T", gottrans);
	#endif

	menu ("File", "Quit/Q", gotquit);
}

/* Given a window's name, return its window pointer so that other
   Macintosh Window Manager routines can be called for that window. */

WindowPtr windowpoint(name)		/* get window pointer 		*/
char *name;				/* given window's name 		*/
{
	winddatum **wind, **tempwind;	/* Handle to our window data 	*/
	WindowPtr	temp;

	if (firstwind) return ((WindowPtr)0);
	wind = simplewinds;		/* look for the named window 	*/
	HLock (wind);

	while ( strcmp ((**wind).windname, name) && (**wind).next) {
		tempwind = wind;
		wind = (winddatum **) (**wind).next;
		HUnlock (tempwind);
		HLock (wind);
	}
	if ( strcmp ((**wind).windname, name) ==0) {
		temp = (**wind).wptr;
		HUnlock (wind);
		return ( temp );	/* return pointer		*/
	} else {
		HUnlock (wind);
		return ( (WindowPtr) 0);/* or zero if it wasn't found	*/
	}
}

/* This routine installs a new window onto the screen.  It also gives
   that window an Item in the Window menu.  This routine is also used
   to modify a window's associated routines.  The x,y positions are the
   top left and bottom right corners of where the window should originally
   be placed.  The coordinates are never used when this routine is called
   to update an already existing window.  But the spaces must be filled,
   so you can use zeros if you want.  Once the window has been displayed in
   its original position, the user has complete control of its size and
   placement with the mouse.

   YOU MUST ASSIGN PROCEDURES TO BE CALLED WHEN SIMPLETOOLS DETECTS THAT
   THIS WINDOW IS BECOMMING ACTIVE, DEACTIVATING, NEEDS UPDATING, OR
   THE MOUSE HAS BEEN PRESSED IN ITS CONTENT.  JUST LIKE THE MENU PROCEDURE,
   THESE PROCEDURES ARE PASSED SOME ARGUMENTS.  YOU DO NOT HAVE TO
   DECLARE THESE IF YOU DON'T WHAT TO USE THEM.  IF YOU USE THE ARGUMENTS,
   YOU WOULD DECLAR THESE PROCEDURES AS FOLLOWS:

   	my_activate (windp, event)	same as my_update
	my_deactivate (windp, event)	same as my_update

	my_update (windp, event)
	WindowPtr 	windp;		the window being acted upon
	EventRecord 	*event;		the current event record
	{
		...
	}

	my_inContent (x, y, windp, event)
	int 		x, y;		mouse position in local coords
	WindowPtr 	windp;		like above
	EventRecord 	*event;		like above
	{
		...
	}
*/

window(name, xtop, ytop, xbot, ybot, a, d, u, c)
char *name;			/* window's name 			*/
int xtop, ytop, xbot, ybot;	/* position if this is a new window 	*/
ProcPtr a, d, u, c;		/* activate, deactivate, update, and 	*/
{				/*  content procedures  		*/
	winddatum **wind, **temp;/* Handle to our window data 		*/
	winddatum **newentry;	/* another Handle 			*/
	Rect newplace;		/* rectable for the window's placement	*/

	if (a == (ProcPtr) 0)
		a = (ProcPtr) stnop;
	if (d == (ProcPtr) 0)
		d = (ProcPtr) stnop;
	if (u == (ProcPtr) 0)
		u = (ProcPtr) stnop;
	if (c == (ProcPtr) 0)
		c = (ProcPtr) stnop;
	if ( !firstwind ) {

		/* see if window is in the list */
		wind = simplewinds;
		HLock (wind);

		while ( strcmp ((**wind).windname, name) && (**wind).next) {
			temp = wind;
			wind = (winddatum **) (**wind).next;
			HUnlock (temp);
			HLock (wind);
		}
		if ( strcmp ((**wind).windname, name) ==0) {

			/* reset the found window's parameters */
			(**wind).wact = (ProcPtr) a;
			(**wind).wdeact = (ProcPtr) d;
			(**wind).wupdate = (ProcPtr) u;
			(**wind).wcontent = (ProcPtr) c;
			SetPort ( (**wind).wptr);
			HUnlock (wind);

			return(FALSE);
		}
		HUnlock (wind);
	}

	/* make a new window entry */
	newentry = (winddatum **)NewHandle ( (long) sizeof (winddatum));
	if (firstwind)
		simplewinds = newentry;
	else
		(**wind).next = (Ptr) newentry;
	firstwind = 0;
	HLock (newentry);

	strcpy ((**newentry).windname, name);
	SetRect (&newplace, xtop, ytop, xbot, ybot);
	if (EmptyRect (&newplace))
		SetRect (&newplace, 10, 42, 500, 330);
	ctop (name);
	(**newentry).wptr = NewWindow (0L, &newplace, name, show_new_window,
		wprocid, -1L, dogoaway, newentry);
	ptoc (name);
	(**newentry).wact = (ProcPtr) a;
	(**newentry).wdeact = (ProcPtr) d;
	(**newentry).wupdate = (ProcPtr) u;
	(**newentry).wcontent = (ProcPtr) c;
	(**newentry).next = (Ptr) 0;
	if (windmenu)
		menu ("Windows", name, showawindow);
	SetPort ( (**newentry).wptr);
	HUnlock (newentry);

	return(TRUE);
}

withwindow(name)			/* set output to window by name	*/
char *name;				/* give it the window's name 	*/
{					/* returns if window exists	*/
	winddatum **wind, **temp;
	wind = simplewinds;
	if (firstwind) return(FALSE);	/* search for the window's name */

	HLock (wind);
	while ( strcmp ((**wind).windname, name) && (**wind).next) {
		temp = wind;
		wind = (winddatum **) (**wind).next;
		HUnlock (temp);
		HLock (wind);
	}
	if ( strcmp ((**wind).windname, name) ==0) {
		SetPort ( (**wind).wptr);	/* set output to it 	*/
		HUnlock (wind);
		return(TRUE);
	} else {
		HUnlock (wind);
		return(FALSE);
	}
}

/* This run procedure is used to install routines to be occasionally
   run.  The routine will be run once for each call to simpleevents()
   which is done repeatedly by runsimpletools().

   EACH ROUTINE INSERTED INTO THE RUN LIST IS RUN MULTIPLE TIMES UNTIL
   IT IS REMOVED BY CALLING STOP.  THE ROUTINE IS CALLED WITH A SINGLE
   ARGUMENT, A POINTER TO THE EVENT JUST RETURNED BY GETNEXTEVENT() AND
   BEFORE SIMPLETOOLS PROCESSES IT.
*/

run(routine)			/* install a run procedure 	*/
ProcPtr routine;		/* give it the procedure 	*/
{				/* return TRUE if successful 	*/
	int i;
	i = 0;			/* add it to the end of the list */
	while ( simpleruns[i] != (ProcPtr) 0L) i++;
	if (i < maxsruns) {
		simpleruns[i] = routine;
		simpleruns[i+1] = (ProcPtr) 0L;
		return(TRUE);
	} else
		return(FALSE);
}

/* This routine removes a procedure from the list of run procedures */

stop(routine)			/* stop a procedure from running*/
ProcPtr routine;		/* give the procedure 		*/
{				/* return TRUE if successful 	*/
	int i = 0;
	while ( (simpleruns[i] != routine) && simpleruns[i])  i++;
	if (simpleruns[i]) {
		while ( simpleruns[i] != (ProcPtr)0 ) {
			simpleruns[i] = simpleruns[i+1];
			i++;
		}
		return(TRUE);
	} else {
		return(FALSE);
	}
}

void home ()			/* text-based home of the pen with
				   the window being erased.		*/
{
	GrafPtr	port;
	GetPort (&port);
	EraseRect (&(port->portRect));
	stgotoxy (1, 1);
}

stgotoxy (x, y)				/* goto text position x, y	*/
int x, y;
{
	Point		pt;
	int		newx, newy;
	FontInfo	font;

	GetFontInfo (&font);
	GetPen (&pt);
	#ifdef MEGAMAX
	if (x < 0)
		newx = pt.a.h;
	else
		newx = font.widMax * (x);
	if (y < 0)
		newy = pt.a.v;
	#else
	if (x < 0)
		newx = pt.h;
	else
		newx = font.widMax * (x);
	if (y < 0)
		newy = pt.v;
	#endif
	else
		newy = (font.ascent + font.descent + font.leading) * (y+1);
	MoveTo (newx, newy);
}

/* The getline procedure is to be called when you want to simply get a Line
   of text from the user at the current pen position on the screen.  You
   will probably preceed this with a call to stgotoxy(x,y).  You would call
   it like:
   		getline ("Erik", name);

   where name is a character array.  This works MUCH better than scanf()
   or gets() since it uses the Macintosh TextEdit routines to allow the
   user to edit the Line being input.

   Getline is very "modal" and no other events are handled while the
   user is expected to enter the Line.  Getline returns ONLY when the
   user presses <RETURN>.

   Routines scheduled to run by the run() routine are called.  Make sure
   your run routines don't strip all <RETURNS> from the event record
   they get or getline will never stop.
*/

getline (dfault, destination)		/* using TE, get a Line		*/
char *dfault, *destination;		/* default string, dest 	*/
{
	TEHandle 		hte;
	Rect			destRect;
	Point			pen,pt;
	FontInfo		FInfo;
	int			done, mask, code, in_already, nextcap;
	GrafPtr			port, window;
	EventRecord		event;
	char			key;
	CursHandle		c;

	GetPort (&port);		/* Calculate Rect for TE	*/
	if (port != FrontWindow())  SelectWindow (port);
	GetPen (&pen);
	GetFontInfo (&FInfo);
	#ifdef MEGAMAX
		SetRect (&destRect, pen.a.h, pen.a.v - FInfo.ascent,
			1000, pen.a.v + FInfo.descent);
	#else
		SetRect (&destRect, pen.h - 1, pen.v - FInfo.ascent,
			1000, pen.v + FInfo.descent);
	#endif
	EraseRect (&destRect);
	hte = TENew (&destRect, &destRect);
	TESetText (dfault, (long)strlen(dfault), hte);
	TEActivate (hte);
	TEUpdate (&destRect, hte);
	mask = mDownMask + keyDownMask + autoKeyMask + mUpMask;
	done = FALSE;
	#ifdef MEGAMAX
	  c = GetCursor (ibeamcursor);
	#else
	  c = GetCursor (iBeamCursor);
	#endif
	in_already = FALSE;
	nextcap = getlinecaps;
	do {				/* "modal" loop until <cr>	*/
		SystemTask ();
		TEIdle (hte);

		GetNextEvent (mask, &event);
		runruns (&event);
		GetMouse (&pt);		/* use I beam in TE		*/
		if (PtInRect (ZZ(pt), &destRect)) {
		  if ( ! in_already )  {
		     SetCursor (*c);
		     in_already = TRUE;
		  }
		} else {
		  if ( in_already ) {
		      InitCursor ();
		      in_already = FALSE;
		  }
		}
		switch (event.what) {
		    case mouseDown:
			code = FindWindow (ZZ(event.where),&window);
			if ((code == inContent) && (window == port)) {
			   GlobalToLocal (&event.where);
				if (PtInRect(ZZ(event.where),&destRect))
					TEClick (ZZ(event.where), 0, hte);
				else SysBeep (1);
			} else SysBeep (20);
			break;
		    case keyDown:
		    case autoKey:
		        key = (char) (event.message & 0xFFL);
			if (nextcap && (key >= 'a') && (key <= 'z'))
				key -= ' ';
			nextcap = FALSE;
			if (key == ' ') nextcap = getlinecaps;
			if (key != '\r')  TEKey (key, hte);
			else  done = TRUE;
			break;
		}
	} while (!done);
	TEDeactivate (hte);

	/* For LIGHTSPEED, use a lowercase te and upper case L in	*/
	/* TElength.  Megamax conversion utility does this wrong too.	*/

	strncpy (destination, *TEGetText(hte), (*hte)->teLength);
	destination[(*hte)->teLength] = 0;

	TEDispose (hte);
    	InitCursor ();
}

/*  Use prompt when you want a tiny window to pop up to ask the user
    a question.  The question is drawn and a TextEdit box is provided
    to get the answer.  Whatever the user leaves in the answer box
    is returned in answer.  Two buttons are also displayed:  OK and
    CANCEL.  Prompt returns TRUE or FALSE depending on which Button was
    pressed.
*/

prompt ( question, answer)		/* dialog box question/answer */
char *question;
char *answer;
{
	return (stdialog (question, answer, 1));
}

/* Message is just like prompt except no answer box is displayed.  An
   OK and CANCEL Button works just like prompt.
*/

message ( message )			/* dialog box message 		*/
char *message;
{
	return (stdialog (message, message, 2));
}

/*  This routine is a simpler whay to call the toolbox SFGetFile()
    routine.  Simple call this like:

    		getfile ("TEXT", filename)

    where filename is a character array.  Replace TEXT with whatever
    file type you desire.  The file manager's working directory is set
    correctly so that a subsequent open() call with filename will work.
*/

getfile (ftype, reply)
char ftype[];
char reply[];
{
	Point where;
	SFReply frommac;

	#ifdef MEGAMAX
	  where.a.h = 75; where.a.v = 50;
	#else
	  where.h = 75; where.v = 50;
	#endif
	if (strlen(ftype) != 4)
	  SFGetFile (ZZ(where), NULL, NULL, -1, NULL, NULL, &frommac);
	else
	  SFGetFile (ZZ(where), NULL, NULL, 1, ftype, 0L, &frommac);
	if (frommac.good) {
	  SetVol ("", frommac.vRefNum);
	  strcpy (reply, frommac.fName);
	  return (TRUE);
	}
	else return (FALSE);
}

/* This is like getfile, but may get a new file name from the user.
   Origname is the default you want to present to the user.
*/

putfile (origname,reply)
char *origname;
SFReply *reply;
{
	Point where;
	SFReply frommac;

	#ifdef MEGAMAX
	  where.a.h = 75; where.a.v = 50;
	#else
	  where.h = 75; where.v = 50;
	#endif
	SFPutFile (ZZ(where), "", origname, 0L, &frommac);
	if (frommac.good) {
	  SetVol ("", frommac.vRefNum);
	  strcpy (reply, frommac.fName);
	  return (TRUE);
	}
	return (FALSE);
}

/* This routine initializes SimpleTools and MUST be called before	*/
/* most of the other SimpleTools routines are called.			*/
/* The passed about string is the menu Item name to appear just under	*/
/* the Apple menu.  This will be disabled and can be enabled using	*/
/* a menu() call.  This routine also initializes the Macintosh 		*/
/* for application execution and desk accessory processing.		*/

simpletools(about)	/* to be called at the beginning of program 	*/
char *about;
{
	#ifdef MEGAMAX
	  maxapplzone();	/* allow maximum heap expansion 	*/
	#else
	  MaxApplZone();
	#endif
	FlushEvents (everyEvent,0);  /* ignore left over events 	*/
	InitGraf (&thePort);	/* initialize the screen 		*/
	InitFonts();
	InitWindows();
	InitMenus();
	InitCursor();		/* make the arrow Cursor 		*/
	TEInit();
	InitDialogs(gotquit);
	snewrom = havenewrom();

	/* Ugh.  For LightSpeed use a lower case d in DocumentProc.	*/
	/* Megamax conversion utility is at fault here.			*/

	wprocid = documentProc;
	if (snewrom) wprocid = zoomproc;
	SetRect ( &sizerect, 20, 50, 250, 330);
	simpleruns[0] = (ProcPtr) 0;  /* empty the run list 		*/

	/* These are the bounds we are allowed to size a window or
	   Move a window to.
	*/

	swholescreen = dragrect = thePort -> portRect;
	InsetRect (&dragrect, 4, 4);
	SetRect ( &sizerect, 20, 20, 2048, 700);
	firstwind = 1;			/* empty window table		*/
	keydownproc = (ProcPtr) stnop;	/* default key hit procedures 	*/
	autokeyproc = (ProcPtr) stnop;
	initsmenus(about);		/* install the menus 		*/
}

simpleevents()				/* to be called in the main loop */
{
	EventRecord newevent;
	winddatum **thewdatum;
	SystemTask();			/* Do the system D.A. etc. stuff */
	HiliteMenu(0);
	GetNextEvent(everyEvent, &newevent);
	runruns(&newevent);		/* Do our run procedures 	*/
	switch (newevent.what) {
	    case mouseDown:
		domousedown(&newevent); break;
	    case keyDown:
		if (newevent.modifiers & cmdKey)
			docommand(MenuKey((char)(newevent.message & 0xffL)),
				&newevent);
		(*(keydownproc))(&newevent);
		 break;
	    case autoKey:
		if (newevent.modifiers & cmdKey)
			docommand(MenuKey((char)(newevent.message & 0xffL)),
				&newevent);
		(*(autokeyproc))(&newevent);
		break;
	    case activateEvt:
		thewdatum = wdatum(newevent.message);
		if (thewdatum) {
			SetPort(newevent.message);
			HLock (thewdatum);
			if (newevent.modifiers & 1) {

				(*((**thewdatum).wact))(newevent.message,
					&newevent);
			} else {
				(*((**thewdatum).wdeact))(newevent.message,
					&newevent);
			}
			HUnlock (thewdatum);
		}
		break;
	    case updateEvt:
		thewdatum = wdatum(newevent.message);
		if (thewdatum) {
			SetPort (newevent.message);
			BeginUpdate (newevent.message);
			HLock (thewdatum);
			(*((**thewdatum).wupdate))(newevent.message,
				&newevent);
			HUnlock (thewdatum);
			EndUpdate (newevent.message);
		}
		break;
	}
}

runsimpletools ()
{
	for (;;) simpleevents();
}

