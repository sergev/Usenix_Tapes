/* X Communication module for terminals which understand the X protocol.
   Copyright (C) 1985, 1986, 1987 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */

/* Written by Yakim Martillo, mods and things by Robert Krawitz  */

/*
 *	$Source: /u2/third_party/gnuemacs.chow/src/RCS/xterm.c,v $
 *	$Author: rlk $
 *	$Locker:  $
 *	$Header: xterm.c,v 1.28 86/08/27 13:30:57 rlk Exp $
 */

#ifndef lint
static char *rcsid_TrmXTERM_c = "$Header: xterm.c,v 1.28 86/08/27 13:30:57 rlk Exp $";
#endif	lint

#include "config.h"

#ifdef HAVE_X_WINDOWS

#include "lisp.h"
#undef NULL

/* On 4.3 this loses if it comes after xterm.h.  */
#include <signal.h>

/* This may include sys/types.h, and that somehow loses
   if this is not done before the other system files.  */
#include "xterm.h"

/* Load sys/types.h if not already loaded.
   In some systems loading it twice is suicidal.  */
#ifndef makedev
#include <sys/types.h>
#endif

#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifdef BSD
#include <strings.h>
#endif
#include <sys/stat.h>

#include "dispextern.h"
#include "termhooks.h"
#include "termopts.h"
#include "termchar.h"
#include "sink.h"
#include "sinkmask.h"
#include <X/Xkeyboard.h>
/*#include <X/Xproto.h>	*/

#define min(a,b) ((a)<(b) ? (a) : (b))
#define max(a,b) ((a)>(b) ? (a) : (b))
#define sigunblockx(sig) sigblock (0)
#define sigblockx(sig) sigblock (1 << ((sig) - 1))
XREPBUFFER Xxrepbuffer;
int pixelwidth;
int pixelheight;
int PendingExposure;
int PendingIconExposure;
#define MAXICID 80
char iconidentity[MAXICID];
#define ICONTAG "emacs@"
#define METABIT 0x80
Window XXIconWindow;
Bitmap XXIconMask;

char *XXcurrentfont;
char *default_window;
int informflag;
extern struct display_line *DesiredScreen[], *PhysScreen[];
extern int initialized;

extern char *alternate_display;

int XXdebug;
int XXpid;
extern int screen_garbaged;
int XXxoffset, XXyoffset;
int IconWindow;

int WindowMapped;
int CurHL;

static int flexlines;		/* last line affect by dellines or */
				/* inslines functions */
extern int errno;
int VisibleX, VisibleY;	/* genuine location of cursor on screen */
				/* if it is there */
static int SavedX, SavedY;	/* Where the cursor was before update */
				/* started */

int bitblt;		/* Used to track bit blt events */
int CursorExists;	/* during updates cursor is turned off */
static int InUpdate;		/* many of functions here may be invoked */
				/* even if no update in progress, when */
				/* no update is in progress the action */
				/* can be slightly different */

short MouseCursor[] = {
  0x0000, 0x0008, 0x0018, 0x0038,
  0x0078, 0x00f8, 0x01f8, 0x03f8,
  0x07f8, 0x00f8, 0x00d8, 0x0188,
  0x0180, 0x0300, 0x0300, 0x0000};

short MouseMask[] = {
  0x000c, 0x001c, 0x003c, 0x007c,
  0x00fc, 0x01fc, 0x03fc, 0x07fc,
  0x0ffc, 0x0ffc, 0x01fc, 0x03dc,
  0x03cc, 0x0780, 0x0780, 0x0300};

Display *XXdisplay;
FontInfo *fontinfo;
Window XXwindow;
Cursor EmacsCursor;

char *fore_color;	/* Variables to store colors */
char *back_color;
char *brdr_color;
char *curs_color;
char *mous_color;

int fore;
int back;
int brdr;
int curs;
int mous;

int (*handler)();

static WindowInfo windowinfo;
WindowInfo rootwindowinfo;



static XKeyPressedEvent XXEvent; /* as X messages are read in they are */
                                /* stored here */
static XREPBUFFER XXqueue;/* Used for storing up ExposeRegion */
				/* replies, so that the SIGIO inter- */
				/* rupt serving routines do almost */
				/* no writes to the X socket */
/*int CurHL;			/* Current Highlighting actually being */
				/* being used for bold font right now*/

int XXborder;
int XXInternalBorder;


extern Display *XOpenDisplay ();
extern Window XCreateWindow ();
extern Cursor XDefineCursor ();
extern Cursor XCreateCursor ();
extern FontInfo *XOpenFont ();

static int flashback ();


/* HLmode -- Changes the GX function for output strings.  Could be used to
 * change font.  Check an XText library function call. 
 */

static
HLmode (new)
     int new;
{
  CurHL =  new;
}


/* External interface to control of standout mode.
   Call this when about to modify line at position VPOS
   and not change whether it is highlighted.  */

XTreassert_line_highlight (highlight, vpos)
     int highlight, vpos;
{
  HLmode (highlight);
}

/* Call this when about to modify line at position VPOS
   and change whether it is highlighted.  */

static
XTchange_line_highlight (new_highlight, vpos, first_unused_hpos)
     int new_highlight, vpos, first_unused_hpos;
{
  HLmode (new_highlight);
  XTtopos (vpos, 0);
  XTclear_end_of_line (0);
}


/* Used for starting or restarting (after suspension) the X window.  Puts the
 * cursor in a known place, update does not begin with this routine but only
 * with a call to DoDsp.  The mouse cursor is warped into the window and then
 * the cursor is turned on.
 */



static
XTset_terminal_modes ()
{
  int stuffpending;
#ifdef XDEBUG
  fprintf (stderr, "XTset_terminal_modes\n");
#endif
  InUpdate = 0;
  stuffpending = 0;
  if (!initialized)
    {
      CursorExists = 0;
      VisibleX = 0;
      VisibleY = 0;
    }
  XTclear_screen ();
#ifdef FIONREAD
  ioctl (0, FIONREAD, &stuffpending);
  if (stuffpending)
    kill (XXpid, SIGIO);
#endif
}

/* XTtopos moves the cursor to the correct location and checks whether an update
 * is in progress in order to toggle it on.
 */

static
XTtopos (row, col)
     register int row, col;
{
  int mask = sigblock (sigmask (SIGIO));
#ifdef XDEBUG
  fprintf (stderr, "XTtopos\n");
#endif
  cursX = col;
  cursY = row;
  if (InUpdate)
    {
      if (CursorExists)
	{
	  CursorToggle ();
	}
      sigsetmask (mask);
      return;		/* Generally, XTtopos will be invoked */
      /* when InUpdate with !CursorExists */
      /* so that wasteful XFlush is not called */
    }
  if ((row == VisibleY) && (col == VisibleX))
    {
      if (!CursorExists)
	{
	  CursorToggle ();
	}
      XFlush ();
      sigsetmask (mask);
      return;
    }
  if (CursorExists) CursorToggle ();
  VisibleX = col;
  VisibleY = row;
  if (!CursorExists) CursorToggle ();
  XFlush ();
  sigsetmask (mask);
}

/* Used to get the terminal back to a known state after resets.  Usually
 * used when restarting suspended or waiting emacs
 */

static
cleanup ()
{
  inverse_video = 0;
  HLmode (0);
}

/* wipes out numcols columns starting a current column on the current line */
  
static
XTclear_end_of_line (first_blank)
     register int first_blank;
{
  register int numcols;

#ifdef XDEBUG
  fprintf (stderr, "XTclear_end_of_line\n");

#endif
  if (cursY < 0 || cursY >= screen_height)
    {
      return;
    }
  if (first_blank >= screen_width)
    {
      return;
    }
  if (first_blank < 0)
    first_blank = 0;
  numcols = screen_width - first_blank;
  {
    int mask = sigblock (sigmask (SIGIO));
    if (cursY == VisibleY && VisibleX >= first_blank)
      {
	if (CursorExists) CursorToggle ();
      }
    XPixSet (XXwindow, 
	     first_blank * fontinfo->width + XXInternalBorder, 
	     cursY * fontinfo->height+XXInternalBorder, 
	     fontinfo->width * numcols,
	     fontinfo->height,
	     back);	
    XTtopos (cursY, first_blank);
    sigsetmask (mask);
  }
}

static
XTreset_terminal_modes ()
{
#ifdef XDEBUG
  fprintf (stderr, "XTreset_terminal_modes\n");
#endif
  XTclear_screen ();
}

static
XTclear_screen ()
{
#ifdef XDEBUG
  fprintf (stderr, "XTclear_screen\n");
#endif
  HLmode (0);
  CursorExists = 0;
  
  cursX = 0;
  cursY = 0;
  SavedX = 0;
  SavedY = 0;
  VisibleX = 0;
  VisibleY = 0;
  {
    int mask = sigblock (sigmask (SIGIO));
    XClear (XXwindow);
    CursorToggle ();
    if (!InUpdate)
      XFlush ();
    sigsetmask (mask);
  }
}

/* used by dumprectangle which is usually invoked upon ExposeRegion
 * events which come from bit blt's or moving an obscuring opaque window
 */

static
dumpchars (ActiveScreen, numcols, tempX, tempY, tempHL)
     register struct display_line **ActiveScreen;
     register int numcols;
     register int tempX, tempY, tempHL;
{
  if (numcols <= 0) return;
  if (((numcols - 1) + tempX) > screen_width)
    {
      numcols = (screen_width - tempX) + 1;
    }
  if ((tempX < 0) || (tempX >= screen_width) ||
      (tempY < 0) || (tempY >= screen_height))
    {
      return;
    }
  XText (XXwindow,
	 (tempX * fontinfo->width+XXInternalBorder),
	 (tempY * fontinfo->height+XXInternalBorder),
	 &ActiveScreen[tempY + 1]->body[tempX],
	 numcols,
	 fontinfo->id,
	 (tempHL ? back : fore),
	 (tempHL ? fore : back));
}

/* When a line has been changed this function is called.  X is so fast
 * that the actual sequence is ignore.  Rather, the new version of the
 * line is simply output if this function is invoked while in UpDate.
 * Sometimes writechars can be invoked when not in update if text is to
 * be output at the end of the line.  In this case the whole line is not
 * output.  Simply the new text at the current cursor position given
 * by VisibleX,Y.  The cursor is moved to the end of the new text.
 */
static
writechars (start, end)
     register char *start, *end;
{
  register int temp_length;
  int mask = sigblock (sigmask (SIGIO));

  if ((cursY < 0) || (cursY >= screen_height))
    {
      sigsetmask (mask);
      return;
    }
  if (CursorExists)
    {
      CursorToggle ();
    }
  if (InUpdate)
    {
      if (DesiredScreen && DesiredScreen[cursY + 1])
	{
	  temp_length = DesiredScreen[cursY + 1]->length;
	}
      else
	temp_length = 0;
      if (temp_length > 0)
	{
	  XText (XXwindow,
		 XXInternalBorder,
		 (cursY * fontinfo->height+XXInternalBorder),
		 &DesiredScreen[cursY + 1]->body[0],
		 temp_length,
		 fontinfo->id,
		 (CurHL ? back : fore),
		 (CurHL ? fore : back));
	  if (temp_length < screen_width)
	    {
	      XTclear_end_of_line (temp_length);
	    }
	  XTtopos (cursY, temp_length);
	}
      else
	{
	  XTclear_end_of_line (0);
	  XTtopos (cursY, 0);
	}
    }
  else
    {
      if ((VisibleX < 0) || (VisibleX >= screen_width))
	{
	  sigsetmask (mask);
	  return;
	}
      if ((VisibleY < 0) || (VisibleY >= screen_height))
	{
	  sigsetmask (mask);
	  return;
	}
      if (((end - start) + VisibleX) >= screen_width)
	{
	  end = start + (screen_width - (VisibleX + 1));
	}
      if (end >= start)
	{
	   XText (XXwindow,
		 (VisibleX * fontinfo->width+XXInternalBorder),
		 (VisibleY * fontinfo->height+XXInternalBorder),
		 start,
		 ((end - start) + 1),
		 fontinfo->id,
		 (CurHL ? back : fore),
		 (CurHL ? fore : back));
	  VisibleX = VisibleX + (end - start) + 1;
	}
      if (!CursorExists) CursorToggle ();
    }
  sigsetmask (mask);
}


static 
XTwrite_chars (start, len)
     register char *start;
     register int len;
{
#ifdef XDEBUG
  fprintf (stderr, "XTwrite_chars\n");
#endif
  writechars (start, start + len - 1);
}

/* The following routine is for the deaf or for the pervert who prefers 
 * that his terminal flashes at him rather than beep at him.
 */

static int flashedback;

static
XTflash ()
{
  struct itimerval itimer;

#ifdef XDEBUG
  fprintf (stderr, "XTflash\n");
#endif

  signal (SIGALRM, flashback);
  getitimer (ITIMER_REAL, &itimer);
  itimer.it_value.tv_usec += 250000;
  itimer.it_interval.tv_sec = 0;
  itimer.it_interval.tv_usec = 0;
  flashedback = 0;
  setitimer (ITIMER_REAL, &itimer, 0);
  {
    int mask = sigblock (sigmask (SIGIO));
    XPixFill (XXwindow, 0, 0, screen_width*fontinfo->width+2*XXInternalBorder,
	      screen_height * fontinfo->height+2*XXInternalBorder, WhitePixel,
	      ClipModeClipped, GXinvert, AllPlanes);
    XFlush ();
    sigsetmask (mask);
  }
  while (!flashedback) pause ();
}

static
flashback ()
{
  int mask = sigblock (sigmask (SIGIO) | sigmask (SIGALRM));
  XPixFill (XXwindow, 0, 0, screen_width * fontinfo->width+2*XXInternalBorder,
	    screen_height * fontinfo->height+2*XXInternalBorder, WhitePixel,
	    ClipModeClipped, GXinvert, AllPlanes);
  XFlush ();
  flashedback = 1;
  sigsetmask (mask);
}	

/* A kludge to get a bell */

static
XTfeep ()
{
  int mask = sigblock (sigmask (SIGIO));
#ifdef XDEBUG
  fprintf (stderr, "XTfeep\n");
#endif
  XFeep (0);
  sigsetmask (mask);
}

/* Artificially creating a cursor is hard, the actual position on the
 * screen (either where it is or last was) is tracked with VisibleX,Y.
 * Gnu Emacs code tends to assume a cursor exists in hardward at cursX,Y
 * and that output text will appear there.  During updates, the cursor is
 * supposed to be blinked out and will only reappear after the update 
 * finishes.
 */

CursorToggle ()
{
  register struct display_line **ActiveScreen;
  if (!WindowMapped) 
    {
      CursorExists = 0;
      return 0;
    }
  if ((VisibleX < 0) || (VisibleX >= screen_width) ||
      (VisibleY < 0) || (VisibleY >= screen_height))
    {			/* Current Cursor position trash */
      /* Not much can be done */
      XFlush ();
      CursorExists = 0;
      return 0;
      /* Currently the return values are not */
      /* used, but I could anticipate using */
      /* them in the future. */
    }
  /*  if (InUpdate && DesiredScreen)
      ActiveScreen = DesiredScreen;
      else*/
  ActiveScreen = PhysScreen;
  if (ActiveScreen && ActiveScreen[VisibleY + 1] &&
      (VisibleX < ActiveScreen[VisibleY + 1]->length))
    {
      if (CursorExists)
	{
	  XText (XXwindow,
		 VisibleX * fontinfo->width+XXInternalBorder,
		 VisibleY * fontinfo->height+XXInternalBorder,
		 &ActiveScreen[VisibleY + 1]->body[VisibleX], 1, 
		 fontinfo->id,
		 fore, back);
	}
      else
	{
	    XText (XXwindow,
		   VisibleX * fontinfo->width+XXInternalBorder,
		   VisibleY * fontinfo->height+XXInternalBorder,
		   &ActiveScreen[VisibleY + 1]->body[VisibleX], 1, 
		   fontinfo->id,
		   back, curs);
	}
    }
  else if (CursorExists)
    {
      XPixSet (XXwindow,
	       VisibleX * fontinfo->width+XXInternalBorder,
	       VisibleY * fontinfo->height+XXInternalBorder,
	       fontinfo->width, fontinfo->height, back);
    }
  else
    {
    XPixSet (XXwindow,
	     VisibleX * fontinfo->width+XXInternalBorder,
	     VisibleY * fontinfo->height+XXInternalBorder,
	     fontinfo->width, fontinfo->height, curs);
    }
  CursorExists = !CursorExists;
  /* Cursor has either been blinked in */
  /* or out */
  if (!InUpdate)
    {
      XFlush ();
    }
  return 1;
}

/* This routine is used by routines which are called to paint regions */
/* designated by ExposeRegion events.  If the cursor may be in the exposed */
/* region, this routine makes sure it is gone so that dumprectangle can */
/* toggle it back into existance if dumprectangle is invoked when not in */
/* the midst of a screen update. */
static 
ClearCursor ()
{
  int mask = sigblock (sigmask (SIGIO));
  if (!WindowMapped)
    {
      CursorExists = 0;
      sigsetmask (mask);
      return;
    }
  if ((VisibleX < 0) || (VisibleX >= screen_width)
      || (VisibleY < 0) || (VisibleY >= screen_height))
    {			/* Current Cursor position trash */
      /* Not much can be done */
      CursorExists = 0;
      sigsetmask (mask);
      return;
    }
  XPixSet (XXwindow,
	  VisibleX * fontinfo->width+XXInternalBorder,
	  VisibleY * fontinfo->height+XXInternalBorder,
	  fontinfo->width, fontinfo->height,
	  back);
  CursorExists = 0;
  sigsetmask (mask);
}

static
XTupdate_begin ()
{	
  int mask = sigblock (sigmask (SIGIO));
#ifdef XDEBUG
  fprintf (stderr, "XTupdate_begin\n");
#endif

  InUpdate = 1;
  if (CursorExists) 
    {
      CursorToggle ();
    }
  SavedX = cursX;		/* The initial"hardware" cursor position is */
  /*  saved because that is where gnu emacs */
  /*  expects the cursor to be at the end of*/
  /* the update */
  SavedY = cursY;
  dumpqueue ();
  sigsetmask (mask);
}


static
XTupdate_end ()
{	
  int mask = sigblock (sigmask (SIGIO));
#ifdef XDEBUG
  fprintf (stderr, "XTupdate_end\n");
#endif
  if (CursorExists)
    CursorToggle ();
  InUpdate = 0;
  dumpqueue ();
  XTtopos (SavedY, SavedX);	/* XTtopos invokes cursor toggle */
  sigsetmask (mask);
}

/* Used for expose region and expose copy events.  Have to get the text
 * back into the newly blank areas.
 */

dumprectangle (top, left, rows, cols)
     register int top, left, rows, cols;
{
  register struct display_line **ActiveScreen;
  register int index;
  int localX, localY, localHL;
  int mask;
  rows += top;
  cols += left;
  top /= fontinfo->height;
  /* Get row and col containing up and */
  /* left borders of exposed region -- */
  /* round down here*/
  left /= fontinfo->width;
  rows += (fontinfo->height - 1);
  cols += (fontinfo->width - 1);
  rows /= fontinfo->height;
  /* Get row and col containing bottom and */
  /* right borders -- round up here */
  rows -= top;
  cols /= fontinfo->width;
  cols -= left;
  if (rows < 0) return;
  if (cols < 0) return;
  if (top > (screen_height - 1)) return;
  if (left > (screen_width - 1)) return;
  if ((VisibleX >= left) && (VisibleX < (left + cols)) &&
      (VisibleY >= top) && (VisibleY < (top + rows)))
    {
      ClearCursor ();
    }
  if (InUpdate && DesiredScreen)
    ActiveScreen = PhysScreen;
  else if (PhysScreen)
    ActiveScreen = PhysScreen;/* When queue is dumped in update this */
  else
    return;
  /* should perhaps be DesiredScreen */
  /* but PhysScreen is guaranteed to contain*/
  /* date which was good for every line on */
  /* screen. For desired screen only for */
  /* lines which are changing.  Emacs does */
  /* not consider a line within a newly */
  /* exposed region necessarily to have */
  /* been changed.  Emacs knows nothing */
  /* about ExposeRegion events.*/
  for (localY = top, index = 0;
       (index < rows) && (localY < screen_height);
       ++index, ++localY)
    {
      if ((localY < 0) || (localY >= screen_height)) continue;
      if (!ActiveScreen[localY + 1]) continue;
      if ((left + 1) > ActiveScreen[localY + 1]->length) continue;
      localX = left;
      localHL = ActiveScreen[localY + 1]->highlighted;
      dumpchars (ActiveScreen,
		 min (cols,
		      ActiveScreen[localY + 1]->length
			- localX),
		 localX, localY, localHL);
    }
  if (!InUpdate && !CursorExists) CursorToggle ();
  /* Routine usually called */
  /* when not in update */
}

/* What sections of the window will be modified from the UpdateDisplay
 * routine is totally under software control.  Any line with Y coordinate
 * greater than flexlines will not change during an update.  This is really
 * used only during dellines and inslines routines (scraplines and stufflines)
 */
static
XTset_terminal_window (n)
     register int n;
{
#ifdef XDEBUG
  fprintf (stderr, "XTset_terminal_window\n");
#endif
  if ((n <= 0) || (n > screen_height))
    flexlines = screen_height;
  else
    flexlines = n;
}

XTins_del_lines (vpos, n)
     int vpos, n;
{
#ifdef XDEBUG
  fprintf (stderr, "XTins_del_lines\n");
#endif
  XTtopos (vpos, 0);
  if (n >= 0) stufflines (n);
  else scraplines (-n);
		
}

static 
XTinsert_chars (start, len)
     register char *start;
     register int len;
{
#ifdef XDEBUG
  fprintf (stderr, "XTinsert_chars\n");
#endif
  writechars (start, start + len - 1);
}

static 
XTdelete_chars (n)
     register int n;
{
  char *msg = "***Delete Chars Called Outside of Update!!!***";
#ifdef XDEBUG
  fprintf (stderr, "XTdelete_chars\n");
#endif
  writechars (msg, msg + strlen (msg) - 1);
}

static
stufflines (n)
     register int n;
{
  register int topregion, bottomregion;
  register int length, newtop, mask;

  if (cursY >= flexlines)
    return;
  
  if (!WindowMapped)
    {
      bitblt = 0;
      return;
    }
  mask = sigblock (sigmask (SIGIO));
  if (CursorExists) CursorToggle ();
  dumpqueue ();
  sigsetmask (mask);
  topregion = cursY;
  bottomregion = flexlines - (n + 1);
  newtop = cursY + n;
  length = (bottomregion - topregion) + 1;
  if ((length > 0) && (newtop <= flexlines))
    {
      mask = sigblock (sigmask (SIGIO));
      /* Should already have cleared */
      /* queue of events associated */
      /* with old bitblts */
      XMoveArea (XXwindow, XXInternalBorder,
		 topregion * fontinfo->height+XXInternalBorder,
		 XXInternalBorder, newtop * fontinfo->height+XXInternalBorder,
		 screen_width * fontinfo->width,
		 length * fontinfo->height);
      if (WindowMapped)
	bitblt = 1;
      XFlush ();
      sigsetmask (mask);
      while (bitblt) 
	{
	  kill (XXpid, SIGIO);
	}
      XFlush ();
    }
  newtop = min (newtop, (flexlines - 1));
  length = newtop - topregion;
  if (length > 0)
    {
      XPixSet (XXwindow,
	       XXInternalBorder,
	       topregion * fontinfo->height+XXInternalBorder,
	       screen_width * fontinfo->width,
	       n * fontinfo->height,
	       back);
    }
  /*  if (!InUpdate) CursorToggle (); */
}

static
scraplines (n)
     register int n;
{
  int mask;
  if (!WindowMapped)
    {
      bitblt = 0;
      return;
    }
  
  if (cursY >= flexlines)
    return;
  mask = sigblock (sigmask (SIGIO));
  if (CursorExists) CursorToggle ();
  dumpqueue ();
  if ((cursY + n) >= flexlines)
    {
      if (flexlines >= (cursY + 1))
	{
	  XPixSet (XXwindow,
		   XXInternalBorder, cursY * fontinfo->height+XXInternalBorder,
		   screen_width * fontinfo->width,
		   (flexlines - cursY) * fontinfo->height,
		   back);
	}
      sigsetmask (mask);
    }
  else
    {
      XMoveArea (XXwindow,
		 XXInternalBorder,
		 (cursY + n) * fontinfo->height+XXInternalBorder,
		 XXInternalBorder, cursY * fontinfo->height+XXInternalBorder,
		 screen_width * fontinfo->width,
		 (flexlines - (cursY + n)) * fontinfo->height);
      if (WindowMapped)
	bitblt = 1;
      XFlush ();
      sigsetmask (mask);
      while (bitblt)
	{
	  kill (XXpid, SIGIO);
	}
      mask = sigblock (sigmask (SIGIO));
      XFlush ();
      XPixSet (XXwindow, XXInternalBorder,
	       (flexlines - n) * fontinfo->height+XXInternalBorder,
	       screen_width * fontinfo->width,
	       n * fontinfo->height, back);
      sigsetmask (mask);
    }
  /* if (!InUpdate) CursorToggle (); */
}
	
/* Substitutes for standard read routine.  Under X not interested in individual
 * bytes but rather individual packets.
 */

XTread_socket (sd, bufp, numchars)
     register int sd;
     register char *bufp;
     register int numchars;
{
  
  int count;
  char *where_mapping;
  int nbytes;
  int stuffpending;
  int temp_width, temp_height;
  int mask = sigblock (sigmask (SIGIO));
  /* XKeyPressedEvent event; */
  /*  typedef struct reply {XEvent event; struct reply *next} Reply;
      Reply *replies = NULL;*/
	
  count = 0;
  if (numchars <= 0)
    {	/* To keep from overflowing read buffer */
      numchars = 1;
      --bufp;
    }
  while (bitblt || XPending () != 0)
    {
      /* while there are more events*/
      XNextEvent (&XXEvent);
      switch (XXEvent.type)
	{
/*	case X_Reply:
	{
	extern char *malloc ();
	Reply *reply = (Reply *) malloc (sizeof (Reply));
	reply->next = replies;
	reply->event = XXEvent;
	replies = reply;
	break;
	}*/
	default:
	  break;
	case ExposeWindow:
	  if (((XExposeEvent *)&XXEvent)->window == XXIconWindow)
	    {
	      PendingIconExposure = 1;
	    }
	  else
	    PendingExposure = 1;/* No reason to repeat */
	  /* this if several */
	  /* ExposeWindow events */
	  /* come in quick succes-*/
	  /* ion */
	  break;
	case ExposeRegion:
	  if (PendingExposure)
	    {	/* Don't bother with */
	      /* region events when */
	      /* full window event */
	      /* is pending */
	      break;
	    }
	  loadxrepbuffer (&XXEvent, &XXqueue);
	  if (XXqueue.rindex == XXqueue.windex)
	    {
	      PendingExposure = 1;
	    }
	  if ((XXqueue.rindex > XXqueue.mindex) ||
	      (XXqueue.windex > XXqueue.mindex) ||
		(XXqueue.rindex < 0) ||
		  (XXqueue.windex < 0))
	    {
	      PendingExposure = 1;
	    }
	  break;
	case ExposeCopy:	/* For ExposeCopy sync */
	  /* will block all outgoing */
	  /* requests until this is */
	  /* decremented */
	  if (WindowMapped) bitblt = 0;
	  break;
	case KeyPressed:
	  /* bcopy (XXEvent, event, sizeof (XKeyPressedEvent)); */
	  where_mapping = XLookupMapping (&XXEvent, &nbytes);
	  /* Nasty fix for arrow keys */
	  if (!nbytes && IsCursorKey (XXEvent.detail & 0xff))
	    {
	      switch (XXEvent.detail & 0xff)
		{
		case KC_CURSOR_LEFT:
		  where_mapping = "\002";
		  break;
		case KC_CURSOR_RIGHT:
		  where_mapping = "\006";
		  break;
		case KC_CURSOR_UP:
		  where_mapping = "\020";
		  break;
		case KC_CURSOR_DOWN:
		  where_mapping = "\016";
		  break;
		}
	      nbytes = 1;
	    }
	  if (numchars - nbytes > 0)
	    {
	      bcopy (where_mapping, bufp, nbytes);
	      bufp += nbytes;
	      count += nbytes;
	      numchars -= nbytes;
	    }
/*	  else
	    {
	      bcopy (where_mapping, bufp, numchars);
	      bufp += numchars;
	      count += numchars;
	      numchars = 0;
	      *(bufp-1) = *(where_mapping + nbytes - 1);
	    }*/
	  break;
	case ButtonPressed:
	case ButtonReleased:
	  switch (spacecheck (Xxrepbuffer.mindex, 
			      Xxrepbuffer.rindex,
			      Xxrepbuffer.windex, 0))
	    {
	    case 0:
	      loadxrepbuffer (&XXEvent,
			      &Xxrepbuffer);
	      if (informflag && (numchars > 1))
		{
		  *bufp++ = (char) 'X' & 037; /* C-x */
		  ++count;
		  --numchars;
		  *bufp++ = (char) 0;  /* C-@ */
		  ++count;
		  --numchars;
		}
	      break;
	    case -1:
	      break;
	    case -2:
	    default:
	      fixxrepbuffer ();
	      break;
	    }
	  break;
	}
    }
/*  while (replies) {
    Reply *reply = replies;
    XPutBackEvent (&reply->event);
    replies = reply->next;
    free (reply);
    }*/
  if (count < 0)
    count = 0;
  if (CursorExists)
    xfixscreen ();
  sigsetmask (mask);
  return count;
}

/* refresh bitmap kitchen sink icon */
refreshicon ()
{
  int mask = sigblock (sigmask (SIGIO));
  if (XXIconWindow) 
    XBitmapBitsPut (XXIconWindow, 0,  0, sink_width, sink_height,
		    sink_bits, BlackPixel, WhitePixel, 
		    XXIconMask, GXcopy, AllPlanes);
  XFlush ();
  sigsetmask (mask);
}

XBitmapIcon () 
{
  int mask = sigblock (sigmask (SIGIO));
  if (!IconWindow)
    {
      XSetIconWindow (XXwindow,XXIconWindow);
      XSelectInput (XXIconWindow, ExposeWindow);
      IconWindow = !IconWindow;
    }
  sigsetmask (mask);
}

XTextIcon () 
{
  int mask = sigblock (sigmask (SIGIO));
  if (IconWindow)
    {
      XClearIconWindow (XXwindow);
      XSelectInput (XXIconWindow, NoEvent);
      IconWindow = !IconWindow;
    }
  sigsetmask (mask);
}

/* Interpreting incoming keycodes. Should have table modifiable as needed
 * from elisp.
 */

/* Exit gracefully from gnuemacs, doing an autosave and giving a status.
 */

XExitGracefully ()
{
  XCleanUp ();
  exit (70);
}

xfixscreen ()
{
  register int temp_width, temp_height;
  int mask = sigblock (sigmask (SIGIO));
  /* register int temp_x, temp_y; */
  dumpqueue ();
  /* Check that the connection is in fact open.  This works by doing a nop */
  /* (well, almost) write operation.  If there is an XIOerror or a */
  /* SIGPIPE, exit gracefully.  This fixes the loop-on-logout bug.*/
  /* XIOErrorHandler (XExitGracefully); */
  XPixFill (XXwindow, 0, 0, 1, 1, back, ClipModeClipped, GXnoop, AllPlanes);
  XFlush ();
  /* XIOErrorHandler (0); */
  if (PendingIconExposure)
    {
      refreshicon ();
      PendingIconExposure = 0;
    }
  if (PendingExposure)
    {
      PendingExposure = 0;
      ClearCursor ();
      XXqueue.rindex = 0;
      XXqueue.windex = 0;
      XQueryWindow (XXwindow, &windowinfo); /* Dangerous to do */
      /* writes here but */
      /* otherwise would */
      /* have to alter */
      /* gnu emacs display */
      /* routines to query */
      /* when screen garbaged */
      temp_width = (windowinfo.width - 2 * XXInternalBorder) / fontinfo->width;
      temp_height = (windowinfo.height- 2*XXInternalBorder) / fontinfo->height;
      if (temp_width != screen_width || temp_height != screen_height)
	change_screen_size (max (5, temp_height), max (10, temp_width));
      XXxoffset= windowinfo.x;
      XXyoffset = windowinfo.y;
      /*if (temp_x != XXxoffset || temp_y != XXyoffset)
	XSetOffset (temp_x, temp_y);*/
      dumprectangle (0, 0,
		     screen_height * fontinfo->height + 2 * XXInternalBorder,
		     screen_width * fontinfo->width + 2 * XXInternalBorder);
    }
  if (!InUpdate)
    if (!CursorExists)
      CursorToggle ();
  sigsetmask (mask);
  kill (XXpid, SIGIO);
}

x_term_init ()
{
  char *vardisplay;
  char *temp_font;
  register char *option;
  extern XTinterrupt_signal ();
  int reversevideo;
  Color cdef;
  char *progname;

  vardisplay = (alternate_display ? alternate_display
		: (char *) egetenv ("DISPLAY"));
  if (!vardisplay || *vardisplay == '\0')
    {
      fprintf (stderr, "DISPLAY environment variable must be set\n");
      exit (-200);
    }

  XXdisplay = XOpenDisplay (vardisplay);
  if (XXdisplay == (Display *) 0)
    {
      fprintf (stderr, "X server not responding.  Check your DISPLAY environment variable.\n");
      exit (-200);
    }
  x_init_1 (1);
  Xxrepbuffer.mindex = XREPBUFSIZE - 1;
  Xxrepbuffer.windex = 0;
  Xxrepbuffer.rindex = 0;
  XXqueue.mindex = XREPBUFSIZE - 1;
  XXqueue.windex = 0;
  XXqueue.rindex = 0;
  WindowMapped = 0;
  baud_rate = 9600;
  min_padding_speed = 10000;
  must_write_spaces = 1;
  informflag = 1;
  MetaFlag = 1;
  visible_bell = 1;
  interrupt_input = 1;
  inverse_video = 1;
  bitblt = 0;
  PendingExposure = 0;

  fix_screen_hook = xfixscreen;
  clear_screen_hook = XTclear_screen;
  clear_end_of_line_hook = XTclear_end_of_line;
  ins_del_lines_hook = XTins_del_lines;
  change_line_highlight_hook = XTchange_line_highlight;
  insert_chars_hook = XTinsert_chars;
  write_chars_hook = XTwrite_chars;
  delete_chars_hook = XTdelete_chars;
  ring_bell_hook = XTfeep;
  reset_terminal_modes_hook = XTreset_terminal_modes;
  set_terminal_modes_hook = XTset_terminal_modes;
  update_begin_hook = XTupdate_begin;
  update_end_hook = XTupdate_end;
  set_terminal_window_hook = XTset_terminal_window;
  read_socket_hook = XTread_socket;
  topos_hook = XTtopos;
  /* raw_topos_hook = XTraw_topos;  */
  reassert_line_highlight_hook = XTreassert_line_highlight;
  scroll_region_ok = 1;        /* we'll scroll partial screens */
  char_ins_del_ok = 0;         /* just as fast to write the line */
  line_ins_del_ok = 1;         /* we'll just blt 'em */
  fast_clear_end_of_line = 1;  /* X does this well */
  memory_below_screen = 0;	/* we don't remember what scrolls 
				   off the bottom */
  dont_calculate_costs = 1;

  /* New options section */
  IconWindow = 0;
  XXborder = 1;
  XXInternalBorder = 1;
  screen_width = 80;
  screen_height = 66;
  reversevideo = 0;
  XXxoffset = 0;
  XXyoffset = 0;
  XXdebug = 0;

  handler = XExitGracefully;
  XErrorHandler (handler);
  XIOErrorHandler (handler);

  progname = "emacs";
  if (option = XGetDefault (progname,"ReverseVideo"))
    if (strcmp (option,"on") == 0) reversevideo = 1;
  if (option = XGetDefault (progname, "BitmapIcon"))
    if (strcmp (option, "on") == 0) IconWindow = 1;

  if (option = XGetDefault (progname,"BorderWidth"))
    XXborder = atoi (option);
  if (option = XGetDefault (progname,"InternalBorder"))
    XXInternalBorder = atoi (option);

  brdr_color = XGetDefault (progname,"Border");
  if (!brdr_color) brdr_color = XGetDefault (progname, "BorderColor");
  back_color = XGetDefault (progname,"Background");
  fore_color = XGetDefault (progname,"Foreground");
  mous_color = XGetDefault (progname,"Mouse");
  curs_color = XGetDefault (progname,"Cursor");

  temp_font  = XGetDefault (progname,"BodyFont");
  if (temp_font == 0) temp_font = "vtsingle";
  XXcurrentfont = (char *) xmalloc (strlen (temp_font) + 1);
  strcpy (XXcurrentfont, temp_font);

  if (DisplayCells () > 2)
    {

      if (fore_color && XParseColor (fore_color, &cdef) &&
	  XGetHardwareColor (&cdef))
	fore = cdef.pixel;
      else
	{
	  fore_color = "black";
	  fore = BlackPixel;
	}

      if (back_color && XParseColor (back_color, &cdef) &&
	  XGetHardwareColor (&cdef))
	back = cdef.pixel;
      else
	{
	  back_color = "white";
	  back = WhitePixel;
	}

      if (curs_color && XParseColor (curs_color, &cdef) &&
	  XGetHardwareColor (&cdef))
	curs = cdef.pixel;
      else
	{
	  curs_color = "black";
	  curs = BlackPixel;
	}

      if (mous_color && XParseColor (mous_color, &cdef) &&
	  XGetHardwareColor (&cdef))
	mous = cdef.pixel;
      else
	{
	  mous_color = "black";
	  mous = BlackPixel;
	}

      if (brdr_color && XParseColor (brdr_color, &cdef) &&
	  XGetHardwareColor (&cdef))
	brdr = cdef.pixel;
      else
	{
	  brdr_color = "black";
	  brdr = BlackPixel;
	}
    }
  else
    {
      fore_color  = curs_color = mous_color = brdr_color = "black";
      fore = curs = mous = brdr = BlackPixel;
      back_color = "white";
      back = WhitePixel;
    }

  /*
  if (fore_color && DisplayCells () > 2 &&
      XParseColor (fore_color, &cdef) && XGetHardwareColor (&cdef))
    fore = cdef.pixel;
  else if (fore_color && strcmp (fore_color, "black") == 0)
    fore = BlackPixel;
  else if (fore_color && strcmp (fore_color, "white") == 0)
    fore = WhitePixel;
  else
    {
      fore_color = "black";
      fore = BlackPixel;
    }

  if (back_color && DisplayCells () > 2 &&
      XParseColor (back_color, &cdef) && XGetHardwareColor (&cdef))
    back = cdef.pixel;
  else if (back_color && strcmp (back_color, "white") == 0)
    back = WhitePixel;
  else if (back_color && strcmp (back_color, "black") == 0)
    back = BlackPixel;
  else
    {
      back_color = "white";
      back = WhitePixel;
    }

  if (brdr_color && DisplayCells () > 2 &&
      XParseColor (brdr_color, &cdef) && XGetHardwareColor (&cdef))
    brdr = cdef.pixel;
  else if (brdr_color && (!strcmp (brdr_color, "gray") ||
			  !strcmp (brdr_color, "grey") ||
			  !strcmp (brdr_color, "Gray") ||
			  !strcmp (brdr_color, "Grey")))
    brdr = BlackPixel;
  else if (brdr_color && strcmp (brdr_color, "white") == 0)
    brdr = WhitePixel;
  else
    {
      brdr_color = "black";
      brdr = BlackPixel;
    }

  if (curs_color && DisplayCells () > 2 &&
      XParseColor (curs_color, &cdef) && XGetHardwareColor (&cdef))
    curs = cdef.pixel;
  else if (curs_color && strcmp (curs_color, "black") == 0)
    curs = BlackPixel;
  else if (curs_color && strcmp (curs_color, "white") == 0)
    curs = WhitePixel;
  else
    {
      curs_color = "black";
      curs = BlackPixel;
    }

  if (mous_color && DisplayCells () > 2 &&
      XParseColor (mous_color, &cdef) && XGetHardwareColor (&cdef))
    mous = cdef.pixel;
  else if (mous_color && strcmp (mous_color, "black") == 0)
    mous = BlackPixel;
  else if (mous_color && strcmp (mous_color, "white") == 0)
    mous = WhitePixel;
  else
    {
      mous_color = "black";
      mous = BlackPixel;
    }
    */
  
  XXpid = getpid ();
  if (XXcurrentfont == (char *) 0)
    {
      fprintf (stderr, "Memory allocation failure.\n");
      exit (-150);
    }
  default_window = "=80x24+0+0";
/* RMS: XTread_socket does not have an interface suitable
   for being a signal handler.  In any case, the SIGIO handler is
   set up in init_keyboard and X uses the same one as usual.  */
/*  signal (SIGIO, XTread_socket); */
  signal (SIGPIPE, XExitGracefully);
  XQueryWindow (RootWindow, &rootwindowinfo);
  strncpy (iconidentity, ICONTAG, MAXICID);
  fontinfo = XOpenFont (XXcurrentfont);
  if (fontinfo == (FontInfo *) 0)
    {
      fprintf (stderr, "No font\n");
      exit (-98);
    }
  pixelwidth = screen_width * fontinfo->width + 2 * XXInternalBorder;
  pixelheight = screen_height * fontinfo->height + 2 * XXInternalBorder;
  XXwindow = XCreateWindow (RootWindow,
			    XXxoffset /* Absolute horizontal offset */,
			    XXyoffset /* Absolute Vertical offset */,
			    pixelwidth, pixelheight,
			    XXborder, BlackPixmap, WhitePixmap);
  if (!XXwindow)
    {
      fprintf (stderr, "Unable to create window.\n");
      exit (-97);
    }

  XXIconWindow = XCreateWindow (RootWindow, 0, 0, sink_width, sink_height,
				2, WhitePixmap, (Pixmap) NULL);
  
  if (!XXIconWindow)
    {
      fprintf (stderr, "Unable to create icon window.\n");
      fflush (stderr);
      exit (-97);
    }
  XSelectInput (XXIconWindow, NoEvent);
  XXIconMask = XStoreBitmap (sink_mask_width, sink_mask_height, sink_mask_bits);

  XSelectInput (XXwindow, NoEvent);
  XSetResizeHint (XXwindow, 2 * XXInternalBorder, 2 * XXInternalBorder,
		  /* fontinfo->width * 1, fontinfo->height * 1, */
		  fontinfo->width, fontinfo->height);

  if (gethostname (&iconidentity[sizeof (ICONTAG) - 1],
		   (MAXICID - 1) - sizeof (ICONTAG)))
    {
      iconidentity[sizeof (ICONTAG) - 2] = '\0';
    }
  XStoreName (XXwindow, &iconidentity[0]);

  EmacsCursor = XCreateCursor (16, 16, MouseCursor, MouseMask,
			       0, 0, mous, back, GXcopy);
  XDefineCursor (XXwindow, EmacsCursor);
  flexlines = screen_height;
#if 0
/* Do not call XPopUpWindow here!  This is too early.
   It is supposed ot be called via the term-setup-hook
   and not until after lisp/term/x-win.el has had a chance
   to process the user's switches.
   I am not sure that there are any circumstances under which
   this should be done here  -- RMS.  */
  XPopUpWindow ();		/* This looks at Vxterm */
#endif /* 0 */
  if (reversevideo) XFlipColor ();
}

x_init_1 (unrequest)
{
#ifdef F_SETOWN
  extern int old_fcntl_owner;
#endif
  extern void init_sigio (), request_sigio (), unrequest_sigio ();

  dup2 (dpyno (), 0);
  close (dpyno ());
  dpyno () = 0;			/* Looks a little strange?
				   check the def of the macro;
				   it is a genuine lvalue */
  init_sigio ();
  request_sigio ();
#ifdef F_SETOWN
  old_fcntl_owner = fcntl (0, F_GETOWN, 0);
  fcntl (0, F_SETOWN, getpid ());
#endif
  if (unrequest) unrequest_sigio ();
}

/* Process all queued ExposeRegion events. */
static
dumpqueue ()
{
  register int i;
  XExposeRegionEvent r;
  if ((XXqueue.rindex > XXqueue.mindex) ||
      (XXqueue.windex > XXqueue.mindex) ||
      (XXqueue.rindex < 0) ||
      (XXqueue.windex < 0))
    {
      PendingExposure = 1;
    }
  else
    while (XXqueue.rindex != XXqueue.windex)
      {
	if (CursorExists)
	  CursorToggle ();
	unloadxrepbuffer (&r, &XXqueue);
	dumprectangle (r.y - XXInternalBorder, r.x - XXInternalBorder,
		       r.height, r.width);
      }
}

XSetFlash ()
{
  ring_bell_hook = XTflash;
}

XSetFeep ()
{
  ring_bell_hook = XTfeep;
}

XNewFont (newname)
     register char *newname;
{
  FontInfo *temp;
  int mask = sigblock (sigmask (SIGIO));
  XFlush ();
  if (XXdebug)
    fprintf (stderr, "Request id is %d\n", XXdisplay->request);
  temp = XOpenFont (newname);
  if (temp == (FontInfo *) 0)
    {
      sigsetmask (mask);
      if (QLength () > 0)
	{
	    kill (XXpid, SIGIO);
	}
      return -1;
    }
  XCloseFont (fontinfo);
  fontinfo = temp;
  XSetResizeHint (XXwindow, 2*XXInternalBorder, 2*XXInternalBorder,
		  /* fontinfo->width * 1, fontinfo->height * 1, */
		  fontinfo->width, fontinfo->height);
  XSetWindowSize (screen_height, screen_width);
  sigsetmask (mask);
  if (QLength () > 0)
    {
	kill (XXpid, SIGIO);
    }
  return 0;
}

XFlipColor ()
{
  Pixmap temp;
  int tempcolor;
  char *tempname;
  Cursor temp_curs;
  int mask = sigblock (sigmask (SIGIO));
  CursorToggle ();
  temp = XMakeTile (fore);
  XChangeBackground (XXwindow, temp);
  XFreePixmap (temp);
  temp = XMakeTile (back);
  if (XXborder)
    XChangeBorder (XXwindow, temp);
  XFreePixmap (temp);
  brdr = back;
  brdr_color = back_color;
  tempcolor = fore;
  fore = back;
  back = tempcolor;
  tempname = fore_color ;
  fore_color = back_color;
  back_color = tempname;
/*  XPixFill (XXwindow, 0, 0, screen_width * fontinfo->width,
	    screen_height * fontinfo->height, back, ClipModeClipped,
	    GXcopy, AllPlanes);
  dumprectangle (0, 0, screen_height * fontinfo->height + 2 * XXInternalBorder,
  screen_width * fontinfo -> width + 2 * XXInternalBorder);*/
  XRedrawDisplay ();
  if (curs == WhitePixel)
    {
	curs = BlackPixel;
	curs_color = "black";
    }
  else if (curs == BlackPixel)
    {
	curs = WhitePixel;
	curs_color = "white";
    }
  if (mous == WhitePixel)
    {
	mous = BlackPixel;
	mous_color = "black";
    }
  else if (mous == BlackPixel)
    {
	mous = WhitePixel;
	mous_color = "white";
    }
  temp_curs = XCreateCursor (16, 16, MouseCursor, MouseMask, 0, 0,
			     mous, back, GXcopy);
  XUndefineCursor (XXwindow);
  XDefineCursor (XXwindow, temp_curs);
  XFreeCursor (EmacsCursor);
  bcopy (&temp_curs, &EmacsCursor, sizeof (Cursor));
  CursorToggle ();
  XFlush ();
  sigsetmask (mask);
}

XSetOffset (xoff, yoff)
     register int xoff, yoff;
{
  int mask = sigblock (sigmask (SIGIO));
  if (xoff < 0)
    {
      XXxoffset = rootwindowinfo.width + (++xoff) - pixelwidth - 4;
    }
  else
    {
      XXxoffset = xoff;
    }
  if (yoff < 0)
    {
      XXyoffset
	= rootwindowinfo.height + (++yoff) - pixelheight - 4;
    }
  else
    {
      XXyoffset = yoff;
    }
  XMoveWindow (XXwindow, XXxoffset, XXyoffset);
  sigsetmask (mask);
  /* XWarpMouse (XXwindow, pixelwidth >> 1, pixelheight >> 1); */
}

XSetWindowSize (rows, cols)
     register int rows, cols;
{
  /* if (rows < 3) rows = 24;
     if (cols < 1) cols = 80; */
  pixelwidth = cols * fontinfo->width + 2 * XXInternalBorder;
  pixelheight = rows * fontinfo->height + 2 * XXInternalBorder;
  XChangeWindow (XXwindow, pixelwidth, pixelheight);
  XFlush ();
  change_screen_size (rows, cols);
  PendingExposure = 0;
}

XPopUpWindow ()
{
  int mask;
  if (WindowMapped)
    return;
  mask = sigblock (sigmask (SIGIO));
  if (!x_edges_specified)
    Fx_rubber_band ();
  bitblt = 0;
  CursorExists = 0;
  VisibleX = 0;
  VisibleY = 0;
  WindowMapped = 1;
  XMapWindow (XXwindow);
  dumprectangle (0, 0,
		 screen_height * fontinfo->height + 2 * XXInternalBorder,
		 screen_width * fontinfo->width + 2 * XXInternalBorder);
  XSelectInput (XXwindow, KeyPressed | ExposeWindow
		| ButtonPressed | ButtonReleased | ExposeRegion | ExposeCopy);
  /*	XWarpMouse (XXwindow, pixelwidth >> 1, pixelheight >> 1);*/
  XTtopos (0, 0);
  if (IconWindow)
    {
      XSetIconWindow (XXwindow,XXIconWindow);
      XSelectInput (XXIconWindow, ExposeWindow);
    }
  else
    {
      XClearIconWindow (XXwindow);
      XSelectInput (XXIconWindow, NoEvent);
    }
  /*  XRedrawDisplay ();*/
  XFlush ();
  sigsetmask (mask);
}

spacecheck (mindex, rindex, windex, minfreespace)
     register int mindex, rindex, windex, minfreespace;
{
  if ((rindex > mindex) || (windex > mindex))
    {
      /* fprintf (stderr, "Fatal Mouse Buffer Error.\n");
	 fprintf (stderr, "%d = mindex, %d = rindex, %d = windex\n",
	 mindex, rindex, windex); */
      return -2;
    }
  if (windex >= rindex)
    {
      if ((mindex - (windex - rindex)) > minfreespace)
	return 0;
    }
  else
    {
      if (((rindex - windex) - 1) > minfreespace)
	return 0;
    }
  return -1;
}

loadxrepbuffer (p_xrep, p_buffer)
     register XEvent *p_xrep;
     register XREPBUFFER *p_buffer;
{
  p_buffer->xrep[p_buffer->windex] = *p_xrep;
  if (p_buffer->windex == p_buffer->mindex)
    p_buffer->windex = 0;
  else
    p_buffer->windex++;
}

unloadxrepbuffer (p_xrep, p_buffer)
     register XEvent *p_xrep;
     register XREPBUFFER *p_buffer;
{
  if (p_buffer->windex == p_buffer->rindex)
    return -1;
  *p_xrep = p_buffer->xrep[p_buffer->rindex];
  if (p_buffer->rindex == p_buffer->mindex)
    p_buffer->rindex = 0;
  else
    p_buffer->rindex++;
  return 0;
}

fixxrepbuffer ()
{
  Xxrepbuffer.mindex = XREPBUFSIZE - 1;
  Xxrepbuffer.windex = 0;
  Xxrepbuffer.rindex = 0;
}

#endif /* HAVE_X_WINDOWS */
