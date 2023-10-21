/*
 * Name:	MicroEmacs
 *		Amiga terminal-dependent I/O (Intuition)
 * Created:	21-Apr-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 */
 
/*
 * Lots of includes.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/tasks.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <devices/console.h>
#include <libraries/dos.h>
#include <graphics/clip.h>
#include <graphics/view.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#ifdef	CHANGE_FONT
#include <libraries/diskfont.h>
#endif

#undef	TRUE			/* avoid redefinition messages 		*/
#undef	FALSE
#include "def.h"		/* includes sysdef.h and ttydef.h	*/

/*
 * External Amiga functions.  Declared explicitly
 * to avoid problems with different compilers.
 */
extern	LONG			 AbortIO();
extern	LONG			 CloseDevice();
extern	LONG			 CloseLibrary();
extern	LONG			 CloseWindow();
extern	struct	MsgPort		*CreatePort();
extern	struct	IOStdReq	*CreateStdIO();
extern	LONG			 DeletePort();
extern	LONG			 DeleteStdIO();
extern	struct	IntuiMessage	*GetMsg();
extern	int			 OpenConsole();
extern	char			*OpenLibrary();
extern	struct	Window		*OpenWindow();
#ifdef	CHANGE_FONT
extern	struct TextFont		*OpenDiskFont();
#endif
extern	LONG			 RectFill();
extern	LONG			 ReplyMsg();
extern	LONG			 SetAPen();
extern	LONG			 SetDrMd();
extern	LONG			 Wait();

#ifdef	DO_MENU
extern	LONG			 ClearMenuStrip();	/* menu functions */
extern	struct	Menu		*InitEmacsMenu();
extern	struct	MenuItem	*ItemAddress();
extern	LONG			 SetMenuStrip();
#endif

extern	int	Enable_Abort;		/* Do NOT allow abort!		*/

/*
 * External MicroEmacs functions and variables
 */
extern	int	quit();			/* Defined by "main.c"	*/
extern	char	*version;		/* Version information		*/

/*
 * Library bases (used by glue libraries)
 */
struct	IntuitionBase	*IntuitionBase;
struct	GfxBase		*GfxBase;
#ifdef	CHANGE_FONT
ULONG			DiskfontBase;
#endif

/*
 * Intuition window and menu variables
 */
#define WINDOWGADGETS (WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE)

static short	borderless = TRUE;	/* Flag for borderless window	*/
static short	leftedge = 0,		/* Last top left position	*/
		topedge = 0,
		width = 640,
		height = 200;

struct NewWindow MicroEMACS = {
	0,	0,			/* start position       	*/
	0,	0,			/* width, height (set by ttopen)*/
	0,	1,	     		/* detail pen, block pen	*/
#ifdef	DO_MENU
	MENUPICK |			/* If menu is used		*/
#endif
#ifdef	MOUSE
	MOUSEBUTTONS | 			/* If mouse is used		*/
#endif
	CLOSEWINDOW | NEWSIZE,		/* IDCMP flags			*/
	0,				/* window flags	(set by ttopen)	*/
	NULL,				/* pointer to first user gadget */
	NULL,				/* pointer to user checkmark	*/ 
	NULL,				/* title (filled in later)	*/
	NULL,				/* pointer to screen (none)	*/
	NULL,				/* pointer to superbitmap	*/
	359,101,			/* minimum size (with TOPAZ_80)	*/
	0, 0,				/* maximum size (set by ttopen)	*/
	WBENCHSCREEN			/* screen in which to open	*/ 
};

struct Window	*EmW;				/* Our window		*/
static short		toggling = FALSE;	/* Prevent menu wiping	*/

#ifdef	DO_MENU
static struct Menu	*EmacsMenu = NULL;	/* Our menu		*/
#endif
#ifdef	CHANGE_FONT
static	struct TextFont *EmFont = NULL;
#endif

/*
 * The bridge between Intuition events and Emacs' single
 * input stream...
 */
static USHORT		class,			/* Intuition event	*/
			code,			/*   information	*/
			qualifier;
static APTR		address;
static SHORT		x, y;
static LONG		intuitionMsgBit;	/* Signal bit		*/
#define INTUITION_MESSAGE ((LONG) (1L << intuitionMsgBit))

/*
 * To more thoroughly handle Intuition events, we buffer
 * them into a circular queue until other routines ask
 * for the information carried in them.
 */
#define	NIBUF	256			/* Rather roomy...		*/
#define	EVT_KBD		0
#define	EVT_MOUSE	1
#define EVT_MENU	2
#define NULLEVT	((struct event *) 0)

struct	event {
	USHORT type;			/* What is it?			*/
	union	{
		UBYTE	ch;		/* Keyboard event		*/
		struct {		/* Mouse click			*/
			SHORT row, col;	/* location in character raster	*/
			USHORT qualifier;
		} mouse;
		USHORT	code;		/* Menu event			*/
	} data;
}		ibuf[NIBUF];		/* Input buffer			*/
int		ibufo, nibuf;		/* head, # of bytes in ibuf	*/

/*
 * Console output
 */
#define	CSI	0x9b			/* Command Sequence Introducer	*/
#define	ESC	0x1b			/* Escape key			*/
#define	NOBUF	512			/* About 1/4 screen		*/

static struct MsgPort	*consoleWritePort;	/* I/O ports 		*/
static struct MsgPort	*consoleReadPort;	
static struct IOStdReq	*consoleWriteMsg;	/* I/O messages		*/
static struct IOStdReq	*consoleReadMsg;
static LONG		consoleMsgBit;		/* signal bit		*/
#define CONSOLE_MESSAGE ((LONG) (1L << consoleMsgBit))
static unsigned char	letter;			/* Console input buffer	*/

static unsigned char	obuf[NOBUF];	/* Terminal output buffer	*/
int			nobuf;		/* # of bytes in above		*/
int			nrow;		/* Terminal size, rows.		*/
int			ncol;		/* Terminal size, columns.	*/
extern int		ttrow;		/* Current cursor row		*/

/*
 * Open up the virtual terminal MicroEMACS communicates with.
 * Set up the window, console, and menu strip.
 */

ttopen()
{
	register struct Screen *s;

	Enable_Abort = 0;				/* Disable ^C	*/

	GfxBase = (struct GfxBase *)
		OpenLibrary("graphics.library", (LONG) 0);
	if (GfxBase  == NULL)				/* Graphics lib	*/
		cleanup(1);

	IntuitionBase = (struct IntuitionBase *)	/* Intuition	*/
		OpenLibrary("intuition.library", (LONG) 0);
	if (IntuitionBase == NULL)
		cleanup(2);

#ifdef	CHANGE_FONT
	DiskfontBase = (ULONG) OpenLibrary("diskfont.library", (LONG)0);
	if (DiskfontBase == NULL)
		cleanup(21);
#endif
	/*
	 * Create our window. Set window flags based on the current
	 * value of the borderless flag, and the maximum size of the
	 * window based on the size of the first screen in the screen
	 * list with screen type WBENCHSCREEN (of which there had better
	 * be *EXACTLY* one, right???...).  To avoid possible crashes
	 * if user is moving screens around, turn off multitasking
	 * during the loop.
	 */
	Forbid();	/* user might be moving screen */
	for (s = IntuitionBase->FirstScreen; s ; s = s->NextScreen)
		if ((s->Flags & SCREENTYPE) == WBENCHSCREEN)
			break;
	MicroEMACS.MaxWidth = s->Width;
	MicroEMACS.MaxHeight = s->Height;
	Permit();

	/* Set the window size based on the last one that was open,
	 * if it was borderless. Otherwise make it fill the screen.
	 * Set max/min widths based on current screen size.
	 *
	 * Set flags and window title, then open window
	 */
	if (borderless) {
		MicroEMACS.Flags = WINDOWGADGETS | ACTIVATE | BORDERLESS;
#ifdef	TOGGLE_ZOOMS
		MicroEMACS.LeftEdge = 0;
		MicroEMACS.TopEdge = 0;
		MicroEMACS.Width = MicroEMACS.MaxWidth;
		MicroEMACS.Height = MicroEMACS.MaxHeight;
#endif
	} else {
		MicroEMACS.Flags = WINDOWGADGETS | ACTIVATE | WINDOWSIZING;
#ifndef	TOGGLE_ZOOMS
	}
#endif
		MicroEMACS.LeftEdge = leftedge;
		MicroEMACS.TopEdge = topedge;
		MicroEMACS.Width = width;
		MicroEMACS.Height = height;
#ifdef	TOGGLE_ZOOMS
	}
#endif
	MicroEMACS.Title = (UBYTE *) version;	/* name for window */
	if ((EmW = OpenWindow(&MicroEMACS)) == NULL)
		cleanup(3);

#ifdef	CHANGE_FONT
	/* If the user requested a different font for the text, EmFont
	 * will be non-null, so set the font for the RastPort.  The
	 * conole device will pick this up when we open it later on.
	 */
	if (EmFont)
		SetFont(EmW->RPort, EmFont);
#endif

	/* Once the window is created, get the Intuition signal bit,
	 * set up the menu, and tell the virtual terminal how big
	 * it is.
 	 */
	intuitionMsgBit = EmW->UserPort->mp_SigBit;
#ifdef	DO_MENU
	if (toggling == FALSE)
		EmacsMenu = InitEmacsMenu(EmW);
	SetMenuStrip(EmW, EmacsMenu);
#endif
	setttysize();

	/* Set up the console device.  Create the necessary read/write
	 * ports and messages, attach the console device thus created
	 * to our window, initialize the console input buffer, and
	 * queue the first read to the console.
	 */

	consoleWritePort = CreatePort("Emacs.con.write",(LONG) 0);
	if (consoleWritePort == NULL)
		cleanup(4);
	consoleWriteMsg = CreateStdIO(consoleWritePort);
	if (consoleWriteMsg == NULL)
		cleanup(5);

	consoleReadPort = CreatePort("Emacs.con.read",(LONG) 0);
	if (consoleReadPort == NULL)
		cleanup(6);
	consoleReadMsg = CreateStdIO(consoleReadPort);
	if (consoleReadMsg == NULL)
		cleanup(7);

	if (OpenConsole(consoleWriteMsg,consoleReadMsg,EmW) != 0)
		cleanup(8);
	consoleMsgBit = consoleReadPort->mp_SigBit;

	QueueRead(consoleReadMsg,&letter);
	nibuf = ibufo = 0;

	return (0);
}

/*
 * Close the virtual terminal, aborting any
 * I/O to the console device and de-allocating
 * everything we have allocated.
 */
ttclose()
{
	ttflush();
	AbortIO(consoleReadMsg);
	CloseDevice(consoleWriteMsg);
	cleanup(0);
	Enable_Abort = 1;
}

/*
 * Toggle between a borderless window
 * and a sizeable window. This lets you
 * use the whole screen if you want.
 * Bound to "toggle-window-hack" by
 * ttykbd.c
 */

togglewindow(f, n, k)
{
	toggling = TRUE;			/* Notify the system	*/
#ifdef	TOGGLE_ZOOMS
	if (!borderless) {
#endif
		leftedge = EmW->LeftEdge;	/* save window state	*/
		topedge = EmW->TopEdge;
		width = EmW->Width;
		height = EmW->Height;
#ifdef	TOGGLE_ZOOMS
	}
#endif
	ttclose();				/* reset to zero	*/

	borderless = !borderless;		/* toggle window flag	*/
	ttopen();				/* re-open tty window	*/
	sgarbf = TRUE;				/* screen was trashed	*/
	nrow = ncol = -1;			/* trash screen size	*/
	refresh();				/* and redraw it	*/
	toggling = FALSE;			/* Ok, done		*/
	return (TRUE);
}

#ifdef	CHANGE_FONT
/*
 * Select a different font for the Emacs window.
 * This obviously does not work very well
 * with proportional fonts, so we ask the
 * user to confirm before he uses one.
 * It's available if you want to be able
 * to use your own disk font (or Topaz 15
 * under 1.2) to edit with.
 */

setfont(f, n, k)
{
	register int	s, size;
	register struct TextFont *newfont;
	char		fontname[80], fontpath[84], fontsize[3];
	struct TextAttr	ta;

	/* Get font size */
	if (f == TRUE)
		size = n;
	else {
		if ((s = ereply("Font size: ",
				fontsize, sizeof(fontsize))) != TRUE)
			return (s);
		size = atoi(fontsize);
	}

	if (size <= 0) {	/* reset to default font	*/
		if (EmFont)
			CloseFont(EmFont);
		EmFont = NULL;
	} else {		/* user wants to set a new font name */
		if ((s = ereply("Font name (e.g. topaz): ",
				fontname, sizeof(fontname))) != TRUE)
			return (s);
		strcpy(fontpath,fontname);
		strncat(fontpath,".font",sizeof(fontpath));/* make name */

		/* set up text attributes */
		ta.ta_Name = (UBYTE *)fontpath;
		ta.ta_YSize = size;
		ta.ta_Style = FS_NORMAL;
		ta.ta_Flags = 0; /* use either */

		/* Look for the font */
		ewprintf("Looking for %s %d...",fontname,size);
		if ((newfont = OpenDiskFont(&ta)) == NULL) {
			ewprintf("Can't find %s %d!",fontname,size);
			return (FALSE);
		} else { /* Found it! Check before using it. */
			if ((newfont->tf_YSize != size) && ((s = eyesno(
			   "Size unavailable - use closest"))!=TRUE)){
				CloseFont(newfont);
				return (FALSE);
			}
			if ((newfont->tf_Flags & FPF_PROPORTIONAL) &&
			    (((s = eyesno("Use proportional font")))!=TRUE)){
				CloseFont(newfont);
				return (FALSE);
			}
			/* Get rid of old font and cache the new one */
			if (EmFont)
				CloseFont(EmFont);
			EmFont = newfont;
		}
	}

	/* Now that the font is selected, close the window. */
	toggling = TRUE;			/* Notify the system	*/
	ttclose();				/* reset to zero	*/
	ttopen();				/* re-open w/new font	*/
	nrow = -1;				/* trash size		*/
	ncol = -1;				/* so refresh() works	*/
	refresh();				/* redo whole screen	*/
	if (size > 0)
		ewprintf("Now using font %s %d",fontname,EmFont->tf_YSize);
	else
		ewprintf("Now using default font");
	return (TRUE);
}
#endif

/*
 * Write a single character to the screen.
 * Buffered for extra speed, so ttflush()
 * does all the work.
 */
ttputc(c)
unsigned char c;
{
	obuf[nobuf++] = c;
	if (nobuf >= NOBUF)
		ttflush();
}

/*
 * Flush characters from the output buffer.
 * Just blast it out with a console write call.
 */
ttflush()
{
	if (nobuf > 0) {
		ConWrite(consoleWriteMsg, obuf, nobuf);
		nobuf = 0;
	}
}

/*
 * Get a character for Emacs, without echo or
 * translation.  Basically, handle Intuition
 * events until we get one that signifies
 * a character was typed in some way.
 */
ttgetc()
{
	register struct	IntuiMessage *message;	/* IDCMP message 	*/
	register LONG		wakeupmask;	/* which signals?	*/
	register int	charfound;		/* got a character yet?	*/
	unsigned char	nextchar();		/* return next char evt	*/

	if (striptochar())			/* any chars in buffer?	*/
		return (int) (nextchar() & 0xFF);

	charfound = FALSE;			/* nope -- have to wait	*/
	do {
		wakeupmask = Wait(INTUITION_MESSAGE|CONSOLE_MESSAGE);

		if (wakeupmask & CONSOLE_MESSAGE) {	/* keyboard	 */
			GetMsg(consoleReadPort);	/* free message	 */
			qchar(letter);			/* do this FIRST */
			QueueRead(consoleReadMsg, &letter);
			charfound = TRUE;
		}

		if (wakeupmask & INTUITION_MESSAGE)	/* Intuition	*/
			while(message =	GetMsg(EmW->UserPort))
				if (dispatch(message) == TRUE)
					charfound = TRUE;
	} while (charfound == FALSE);

	return (int) (nextchar() & 0xFF);		/* found a character!	*/
}

/*
 * Handle the events we handle...  The result
 * returned indicates if we've put a character
 * in the input buffer.
 */
static dispatch(msg)
register struct IntuiMessage *msg;
{
#ifdef	DO_MENU
	register struct	MenuItem	*item;
#endif
	register int			txheight, txwidth;
	register struct RastPort	*rp;
	int				dx, dy, fgpen, drmode;
			
	class =	msg->Class;		/* grab the info before we 	*/
	code = msg->Code;		/* reply to the message		*/
	qualifier = msg->Qualifier;
	address = msg->IAddress;
	x = msg->MouseX;
	y = msg->MouseY;
	ReplyMsg(msg);			/* return it to Intuition	*/

	switch(class) {			/* see what the fuss is about	*/
#ifdef	DO_MENU
	case MENUPICK:
		if (code == MENUNULL)
			return (FALSE);
		while (code != MENUNULL) {/* handle multiple selection	*/
			qmenu(code);
			item = ItemAddress(EmacsMenu,(LONG) code);
			code = item->NextSelect;
		}
		return (TRUE);		/* puts <CSI>M~ in event queue	*/
		break;
#endif

#ifdef	MOUSE
	case MOUSEBUTTONS:			/* fake the mouse key	*/
		if (code != SELECTDOWN)		/* ignore SELECTUP	*/
			return (FALSE);
		qmouse(x, y, qualifier);
		return (TRUE);
		break;
#endif
	case NEWSIZE:
		/* Sometimes when you resize the window to make it
		 * smaller, garbage is left at the right and bottom
		 * sides of the window. This code is devoted to
		 * (somehow) getting rid of this garbage.  Any
		 * suggestions?
		 */

		rp = EmW->RPort;
		fgpen = rp->FgPen;		/* save params		*/
		drmode = rp->DrawMode;
		SetDrMd(rp, (LONG) JAM1);
		SetAPen(rp, (LONG) EmW->RPort->BgPen);

		/* Check the bottom of the window
		 */
		txheight = EmW->Height - EmW->BorderTop - EmW->BorderBottom;
		if (dy = (txheight % FontHeight(EmW)))
			RectFill(rp,
				(LONG) EmW->BorderLeft,
				(LONG) EmW->BorderTop + txheight - dy - 1,
				(LONG) (EmW->Width - 1) - EmW->BorderRight,
				(LONG) (EmW->Height - 1) - EmW->BorderBottom);

		/* Check the right side
		 */
		txwidth = EmW->Width - EmW->BorderLeft - EmW->BorderRight;
		if (dx = txwidth % FontWidth(EmW))
			RectFill(rp,
				(LONG) EmW->BorderLeft + txwidth - dx - 1,
				(LONG) EmW->BorderTop,
				(LONG) (EmW->Width - 1) - EmW->BorderRight,
				(LONG) (EmW->Height - 1) - EmW->BorderBottom);

		SetDrMd(rp, (LONG) drmode);
		SetAPen(rp, (LONG) fgpen);	/* restore colors */

		/* Tell the console device to resize itself */
		ttputc(CSI);
		ttputc('t');
		ttputc(CSI);
		ttputc('u');
		ttflush();

		/* Signal the editor that a new size has occurred */
		qchar(ESC);
		qchar('\f');

		return (TRUE);			/* we done (finally)	*/
		break;

        case CLOSEWINDOW:			/* Call quit() directly	*/
		quit(FALSE, 1, KRANDOM);
		return (FALSE);
                break;
	default:
		panic("HandleMsg: unknown event!!!");
		break;
	}
	return(FALSE);
}

#ifdef	DO_MENU
/*
 * Return the next menu selection number to
 * the caller.  Used by "ttymenu.c".
 */
ttmenu(codep)
USHORT *codep;
{
	register struct event *e;
	struct event *nextevt();

	e = nextevt();
	if (e->type != EVT_MENU)
		return (FALSE);

	*codep = e->data.code;
	remevt();			/* remove event by hand	*/
	return (TRUE);
}
#endif

#ifdef	MOUSE
/*
 * Return the next mouse click values to
 * the caller.   *Rowp and *colp will contain
 * the row and column where the mouse click occured.
 * This is so that only the terminal driver has
 * to know about the size of the window's font.
 * If the flag argument f is FALSE, the mouse event
 * is *not* removed from the queue, allowing routines
 * that need to (mainly getmouse()) to peek at it.
 */
ttmouse(f, rowp,colp,qualp)
int f;
USHORT *rowp, *colp, *qualp;
{
	register struct event *e;
	struct event *nextevt();

	e = nextevt();
	if (e->type != EVT_MOUSE)
		return (FALSE);		/* next isn't mouse evt */

	*colp = e->data.mouse.col;
	*rowp = e->data.mouse.row;
	*qualp = e->data.mouse.qualifier;
	if (f)
		remevt();			/* remove the event	*/
	return (TRUE);
}
#endif

/*
 * Return the current size of the virtual
 * terminal in nrow and ncol, making sure
 * we don't go beyond the size of the internal
 * video array.
 * Assumes the current font is monospaced
 * (not always safe bet any more :-) :-).
 */
setttysize()
{
	nrow = (EmW->Height - EmW->BorderTop
			- EmW->BorderBottom) / FontHeight(EmW);
	ncol = (EmW->Width - EmW->BorderLeft
			- EmW->BorderRight) / FontWidth(EmW);
	if (nrow < 1)
		nrow = 1;
	if (nrow > NROW)
		nrow = NROW;
	if (ncol < 1)
		ncol = 1;
	if (ncol > NCOL)
		ncol = NCOL;
}

/*
 * Exit as soon as possible, after displaying
 * the message.
 */
panic(s)
char *s;
{
	ewprintf(s);		/* put message at bottom	*/
	Delay((ULONG) 90);	/* wait 1.5 seconds		*/
	ttclose();		/* get rid of window &resources	*/
	exit(10000);		/* go 'way			*/
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *			 Event buffer management		 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * If the buffer's full, crap out, else
 * return a pointer to the (ibufo + nibuf)'th
 * event record (mod NIBUF).  Postincrement
 * nibuf so it points at the next record and
 * also keeps track of how many events
 * are in the buffer.
 */
static struct event *newevt()
{
	return ((nibuf < NIBUF) ? 
		(ibuf + ((ibufo + nibuf++) % NIBUF)) : NULLEVT);
}

/*
 * Return pointer to next item in queue,
 * *without* removing it.
 */
static struct event *nextevt()
{
	return (nibuf ? (ibuf + ibufo) : NULLEVT);
}

/*
 * Move buffer pointer to next item in queue.
 */
static remevt()
{
	if (nibuf <= 0)
		nibuf = 0;
	else {
		nibuf--;
		ibufo++;
		ibufo %= NIBUF;		/* wrap around in buffer	*/
	}
}

/*
 * Return true if there are some characters available
 * in the buffer.  Unlike striptochar, don't do anything
 * to the input buffer, just return a value.
 */
typeahead()
{
	register int bufp;

	for (bufp = 0; bufp < nibuf; bufp++)
		if (ibuf[(ibufo + bufp) % NIBUF].type == EVT_KBD)
			return (TRUE);
	return (FALSE);
}

/*
 * See if there are any characters queued,
 * stripping any other events that may
 * be in the way.  *Don't* remove the character
 * from the  queue.
 */
static striptochar()
{
	register struct event *e;

	while (e = nextevt())
		if (e->type == EVT_KBD)
			return (TRUE);
		else
			remevt();
	return (FALSE);
}

/*
 * Return next character in event buffer.
 */
static unsigned char nextchar()
{
	register struct event *e;

	if (e = nextevt()) {
		remevt();
		return (e->data.ch);
	}
	else
		return ((unsigned char) 0);	/* shouldn't happen	*/
}

/*
 * Add a keyboard event to the queue
 */
static qchar(c)
unsigned char c;
{
	register struct event *e;

	if (e = newevt()) {
		e->type = EVT_KBD;
		e->data.ch = c;
	}
}

#ifdef	MOUSE
/*
 * Add a mouse event to the queue, calculating
 * the row and column value from the current height
 * and width of the window's font.
 */
static qmouse(x, y, qual)
SHORT x, y;
USHORT qual;
{
	register struct event *e;

	qchar(CSI);
	qchar('P');
	qchar('~');
	if (e = newevt()) {
		e->type = EVT_MOUSE;
		e->data.mouse.col = (x - EmW->BorderLeft) / FontWidth(EmW);
		e->data.mouse.row = (y - EmW->BorderTop) / FontHeight(EmW);
		e->data.mouse.qualifier = qual;
	}
}
#endif

#ifdef	DO_MENU
/*
 * Add a menu key to queue
 */
static qmenu(code)
USHORT code;
{
	register struct event *e;

	qchar(CSI);		/* menu key sequence	*/
	qchar('M');
	qchar('~');
	if (e = newevt()) {
		e->type = EVT_MENU;
		e->data.code = code;
	}
}
#endif

/*
 * Clean up.
 *
 * Fall through all the possible cases (0 means
 * get rid of everything and start with the case
 * that fits the error situation).
 */

static cleanup(prob)
{
	switch (prob) {
	case 0:			/* just clean everything up

	case 8:			/* couldn't open console device		*/
		DeleteStdIO(consoleReadMsg);
	case 7:			/* couldn't get console read msg	*/
		DeletePort(consoleReadPort);
	case 6:			/* couldn't get console read port	*/
		DeleteStdIO(consoleWriteMsg);
	case 5:			/* couldn't get console write msg	*/
		DeletePort(consoleWritePort);
	case 4:			/* couldn't get console write port	*/
#ifdef	CHANGE_FONT
		if ((toggling == FALSE) && EmFont)
			CloseFont(EmFont);/* access_count-- */
#endif
#ifdef	DO_MENU
		if (toggling == FALSE) {
			ClearMenuStrip(EmW);
			DisposeMenus(EmacsMenu);
		}
#endif
		CloseWindow(EmW);
	case 3:			/* couldn't open window			*/
#ifdef	CHANGE_FONT
		CloseLibrary(DiskfontBase);
#endif
	case 21:		/* couldn't open DiskfontBase		*/
		CloseLibrary(IntuitionBase);
	case 2:			/* couldn't open IntuitionBase		*/
		CloseLibrary(GfxBase);
	case 1:			/* couldn't open GfxBase -- do nothing	*/
		break;
	}
        return(0);
}

