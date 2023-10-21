/*
 * browser - Rummage around on disks.
 *
 *	copyright (c) 1986, Mike Meyer
 *
 * Permission is hereby granted to distribute this program, so long as this
 * source file is distributed with it, and this copyright notice is not
 * removed from the file.
 *
 * Locks, general -	do I need a stack of locks, one per directory level?
 *			do I have to free the lock in done?
 */

#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <intuition/intuition.h>
#include <stdio.h>

#define INTUITION_REV		1
#define GRAPHICS_REV		1

#define LONGEST_NAME		80	/* Longest file name we can deal with */
#define LONGEST_LINE		256	/* Longest line we will deal with */
#define AVG_LINE_LENGTH		66	/* A guess, tune it if you need to */

#define UP_GADGET		((unsigned short) 0)
#define DOWN_GADGET		((unsigned short) 1)
#define GWIDTH			16	/* Width of my two gadgets */
#define GHEIGHT			9	/* and their heights */

/*
 * Pictures for the up and down arrows
 */
static USHORT arrows[2][GHEIGHT] = {
	{0xFE7F, 0xFC3F, 0xFA5F, 0xF66F,	/* Up */
	 0xFE7F, 0xFE7F, 0xFE7F, 0xFE7F, 0xFE7F},
	{0xFE7F, 0xFE7F, 0xFE7F, 0xFE7F,	/* Down */
	 0xFE7F, 0xF66F, 0xFA5F, 0xFC3F, 0xFE7F}
	} ;
/*
 * Now, the Image structures that use the arrows
 */
static struct Image Arrow_Image[2] = {
	{0, 0, GWIDTH, GHEIGHT, 1, arrows[UP_GADGET], 1, 0,  /* Up */
		(struct Image *) NULL},
	{0, 0, GWIDTH, GHEIGHT, 1, arrows[DOWN_GADGET], 1, 0,  /* Down */
		(struct Image *) NULL}
	} ;
/*
 * Now, my two Gadget structures
 */
static struct Gadget Up_Gadget = {
	(struct Gadget *) NULL,
	0, 1 - GHEIGHT,			/* Left, Top */
	GWIDTH, GHEIGHT,
	GRELBOTTOM | GADGIMAGE		/* Standard bottom border gadget */
	| GADGHCOMP,
	RELVERIFY | BOTTOMBORDER,	/* Messages when released */
	BOOLGADGET,			/* These be boolean gadgets */
	(APTR) &(Arrow_Image[UP_GADGET]),
	(APTR) NULL,			/* No rendering image, using HCOMP */
	(struct IntuiText *) NULL,
	0,				/* No mutex */
	(APTR) NULL,			/* Nothing special */
	UP_GADGET,			/* Yes, this is the up gadget */
	(APTR) NULL			/* And nothing of mine */
	} ;

static struct Gadget Down_Gadget = {
	&Up_Gadget,			/* Next gadget is Up_Gadget */
	GWIDTH + 2, 1 - GHEIGHT,	/* Left, Top */
	GWIDTH, GHEIGHT,
	GRELBOTTOM | GADGIMAGE		/* Standard bottom border gadget */
	| GADGHCOMP,
	RELVERIFY | BOTTOMBORDER,	/* Messages when released */
	BOOLGADGET,			/* These be boolean gadgets */
	(APTR) &(Arrow_Image[DOWN_GADGET]),
	(APTR) NULL,			/* No rendering image, using HCOMP */
	(struct IntuiText *) NULL,
	0,				/* No mutex */
	(APTR) NULL,			/* Nothing special */
	DOWN_GADGET,			/* Yes, this is the up gadget */
	(APTR) NULL			/* And nothing of mine */
	} ;
/*
 * Now, the window for it all
 */
static struct NewWindow New_Window = {
	0, 0, 640, 200,			/* Full screen */
	-1, -1,				/* Default pens */
	MENUPICK | CLOSEWINDOW		/* I want to know about menupicks */
	| GADGETUP,			/* Window closes and gadgets */
	ACTIVATE			/* Standard window */
	| SMART_REFRESH | NOCAREREFRESH | SIZEBBOTTOM
	| WINDOWSIZING | WINDOWDEPTH | WINDOWCLOSE | WINDOWDRAG,
	&Down_Gadget,			/* Add my gadgets */
	(struct Image *) NULL,
	"Browser 0.0",			/* Title */
	(struct Screen *) NULL,
	(struct BitMap *) NULL,
	100, 40,			/* Minimum sizes */
	640, 200,			/* Maximum sizes */
	WBENCHSCREEN			/* and put it on the workbench */
	} ;
/*
 * My very own variables (mostly for done)
 */
static struct Window	*My_Window = NULL ;
static FILE		*infile = NULL ;	/* Current input file */
static void		Page_File(unsigned short) ;

/*
 * And someone else's variables
 */
struct IntuitionBase	*IntuitionBase ;
struct GfxBase		*GfxBase ;
extern struct Menu	*AutoMenu ;

/*
 * Finally, declare the string twiddling functions as voids
 */
void	strcat(char *, char *), strcpy(char *, char *) ,
	strncat(char *, char *, int) ;

#ifdef	DEBUG
main() {
#else
_main() {
#endif
	register struct IntuiMessage	*message, *GetMsg(struct MsgPort *) ;
	register unsigned short		class, code ;

	/* Set up the world this lives in */

	if ((IntuitionBase = (struct IntuitionBase *)
	    OpenLibrary("intuition.library", INTUITION_REV)) == NULL)
		done(20, "can't open intuition library") ;

	if ((GfxBase = (struct GfxBase *)
	    OpenLibrary("graphics.library", GRAPHICS_REV)) == NULL)
		done(20, "can't open graphics library") ;

	if ((My_Window = (struct Window *) OpenWindow(&New_Window)) == NULL)
		done(20, "can't open the window") ;

	SetAPen(My_Window -> RPort, 1) ;	/* Should be default! */

	/* Set up the first menu level */
	Menu_Init() ;
	Menu_Add("disks ", TRUE) ;
	Menu_Item_Add("df0:", ITEMENABLED, 0, 0) ;
	Menu_Item_Add("df1:", ITEMENABLED, 0, 0) ;
	Menu_Item_Add("ram:", ITEMENABLED, 0, 0) ;
	SetMenuStrip(My_Window, AutoMenu) ;

	/* Now spin on messages, handling them as they arrive */
	for (;;) {
		Wait(1 << My_Window -> UserPort -> mp_SigBit) ;
		while (message = GetMsg(My_Window -> UserPort)) {
			class = message -> Class ;
			code = message -> Code ;
			ReplyMsg(message) ;

			switch (class) {
				case CLOSEWINDOW:
					done(0, "exit") ;

				case MENUPICK:
					if (code != MENUNULL)
						Examine_Menu_Pick(code) ;
					break ;

				case GADGETUP:
					Page_File(((struct Gadget *) (message -> IAddress)) -> GadgetID) ;
					break ;

				default:
#ifdef	DEBUG
					printf("browser: intuition event 0x%x\n", class) ;
#endif
					done(20, "unexpected intution event") ;
					break ;
				}
			}
		}
	done(20, "broke out of never-breaking loop!") ;
	}
/*
 * Examine_Menu_Pick - takes a menu pick, and twiddles the state variables
 *	to match that pick.
 */
static
Examine_Menu_Pick(Menu_Number) unsigned short Menu_Number; {
	register unsigned short	level, i, dirp ;
	register char		*cp ;
	char			*name, *strchr(char *, int) ;
	struct MenuItem		*ItemAddress(struct Menu *, unsigned short) ;
	/* State variables that describe the current directory */
	static char		Dir_Name[LONGEST_NAME] ;
	static unsigned short	Menu_Level = 0 ;

	name = ((struct IntuiText *)
		(ItemAddress(AutoMenu, Menu_Number) -> ItemFill)) -> IText ;
	level = MENUNUM(Menu_Number) ;

	/* Got what we want, so clear the menu to avoid confusing the user */
	ClearMenuStrip(My_Window) ;

	/* set dirp to FALSE if the name is not a directory or disk */
	dirp = (strchr(name, '/') != NULL || strchr(name, ':') != NULL) ;

	/* First, set the directory name right */
	if (level > Menu_Level)			/* Not possible, die */
		done(20, "impossible menu value returned") ;
	else if (level == 0)			/* picked a new disk */
		Dir_Name[0] = '\0' ;
	else if (level < Menu_Level) {		/* Throw away some levels */
		for (i = 1, cp = strchr(Dir_Name, ':'); i < level; i++) {
			if (cp == NULL) done(20, "broken file name") ;
			cp = strchr(cp, '/') ;
			}
		if (cp == NULL) done(20, "broken file name") ;
		*++cp = '\0' ;
		}
	/* else Menu_Level == level, chose a file at current level */

	/* Now, fix up the menu and it's state variable */
	while (Menu_Level > level) {
		Menu_Level-- ;
		Menu_Pop() ;
		}

	/* If we added a directory, tack it onto the name */
	if (dirp) {
		Menu_Level++ ;
		strncat(Dir_Name, name,	LONGEST_NAME - strlen(Dir_Name) - 1) ;
		}

	/* Now, tell the user all about it */
	if (dirp) Add_Dir(Dir_Name, name) ;
	else Display_File(Dir_Name, name) ;
	SetMenuStrip(My_Window, AutoMenu) ;
	}
/*
 * Add_Dir - given a dir and a name, add the menu name with the files in
 *	dir as entries.
 */
static
Add_Dir(dir, name) char *dir, *name; {
	register char			*last_char ;
	register struct FileLock	*my_lock, *Lock(char *, int) ;
	unsigned short			count ;
	struct FileInfoBlock		File_Info ;
	static char			Name_Buf[LONGEST_NAME] ;

	/* Fix up the trailing / if it needs it */
	last_char = &dir[strlen(dir) - 1] ;
	if (*last_char == '/') *last_char = '\0' ;

	/* Now, start on the directory */
	if ((my_lock = Lock(dir, ACCESS_READ)) == NULL) {
#ifdef	DEBUG
		printf("browser: error trying to lock %s, IoErr says %d\n",
				dir, IoErr()) ;
#endif
		done(20, "can't get lock on file") ;
		}

	if (!Examine(my_lock, &File_Info)) done(20, "can't examine file") ;
	if (File_Info . fib_DirEntryType < 0)
		done(20, "Add_Dir called with a non-directory") ;

	Menu_Add(name, TRUE) ;
	for (ExNext(my_lock, &File_Info), count = 0;
	     IoErr() != ERROR_NO_MORE_ENTRIES;
	     ExNext(my_lock, &File_Info), count++)
		if (File_Info . fib_DirEntryType < 0)
			Menu_Item_Add(File_Info . fib_FileName, ITEMENABLED,
				0, 0) ;
		else {
			strcpy(Name_Buf, File_Info . fib_FileName) ;
			strcat(Name_Buf, "/") ;
			Menu_Item_Add(Name_Buf, ITEMENABLED, 0, 0) ;
			}
	if (count == 0) Menu_Item_Add("EMPTY", 0, 0, 0) ;

	/* Put everything back */
	if (*last_char == '\0') *last_char = '/' ;
	UnLock(my_lock) ;
	}
/*
 * Display_File - given a directory path and file name, put the first page of
 *	the file in the window.
 */
static
Display_File(dir, name) char *dir, *name; {
	static char	File_Name[LONGEST_NAME] ;
	FILE		*fopen(char *, char *) ;

	/* Get the file name */
	strcpy(File_Name, dir) ;
	strcat(File_Name, name) ;

	if (infile != NULL) fclose(infile) ;
	if ((infile = fopen(File_Name, "r")) == NULL) {
#ifdef	DEBUG
		printf("File: %s\n", File_Name) ;
#endif
		done(20, "can't open file") ;
		}
	Page_File(DOWN_GADGET) ;		/* Down from page 0 */
	}
/*
 * Page_File - move the file up or down one "page"
 */
static void
Page_File(direction) int direction; {
	/* These two should be registers, but it the it breaks the compiler */
	short	Page_Length, where ;
	static char	buffer[LONGEST_LINE], *blanks ;

	if (infile == NULL) return ;

	blanks =
"                                                                                 " ;

	Page_Length = (My_Window -> Height - 20) / 8 ;

	if (direction == UP_GADGET) {		/* Seek back one page */
		if (ftell(infile) < AVG_LINE_LENGTH * (Page_Length + 2))
			fseek(infile, 0L, 0) ;
		else {
			fseek(infile, (long) -Page_Length * AVG_LINE_LENGTH, 1) ;
			fgets(buffer, LONGEST_LINE, infile) ;
			}
		}
	else if (direction != DOWN_GADGET)
		done(20, "Illegal argument to Page_File") ;

	for (where = 17; Page_Length--; where += 8) {
		Move(My_Window -> RPort, 3, where) ;
		Text(My_Window -> RPort, blanks, 79) ;
		Move(My_Window -> RPort, 3, where) ;
		if (fgets(buffer, LONGEST_LINE, infile) == NULL)
			return ;
		if (strlen(buffer) < LONGEST_LINE - 1)
			Text(My_Window -> RPort, buffer, strlen(buffer) - 1) ;
		else {
			Text(My_Window -> RPort, "Long line read!", 15) ;
			fclose(infile) ;
			infile = NULL ;
			return ;
			}
		}
	}
/*
 * done - just close everything that's open, and exit.
 */
static
done(how, why) int how; char *why; {

	if (My_Window) {
		ClearMenuStrip(My_Window) ;
		Menu_Clear() ;
		CloseWindow(My_Window) ;
		}
	if (GfxBase) CloseLibrary(GfxBase) ;
	if (IntuitionBase) CloseLibrary(IntuitionBase) ;
	if (infile) fclose(infile) ;

#ifdef	DEBUG
	printf("browser: %s\n", why) ;
#endif
	(void) OpenWorkBench() ;
	exit(how) ;
	}

