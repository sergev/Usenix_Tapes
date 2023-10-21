/** 			curses.c		**/

/**  This library gives programs the ability to easily access the
     termcap information and write screen oriented and raw input
     programs.  The routines can be called as needed, except that
     to use the cursor / screen routines there must be a call to
     InitScreen() first.  The 'Raw' input routine can be used
     independently, however.

     Modified 2/86 to work (hopefully) on Berkeley systems.  If
     there are any problems with BSD Unix, please report them to
     the author at hpcnoe!dat@HPLABS (fixed, if possible!)

     (C) Copyright 1985 Dave Taylor, HP Colorado Networks
**/

#include "headers.h"

#ifdef RAWMODE
# ifdef BSD
#  include <sgtty.h>
# else
#  include <termio.h>
# endif
#endif

#include <ctype.h>

#ifdef BSD
#undef tolower
#endif
#include "curses.h"

#ifdef BSD
# include "/usr/include/curses.h"  	/* don't ask! */
#endif

#ifdef RAWMODE
# define TTYIN	0
#endif

extern int debug;

#ifdef RAWMODE
#  ifndef BSD
    struct termio _raw_tty, 
                _original_tty;
#  endif

static int _inraw = 0;                  /* are we IN rawmode?    */
static int _line  = -1,			/* initialize to "trash" */
	   _col   = -1;

#ifdef UTS
static int _clear_screen = 0;		/* Next i/o clear screen? */
static char _null_string[SLEN];		/* a string of nulls...   */
#endif

#endif

static int _intransmit;			/* are we transmitting keys? */

static
char *_clearscreen, *_moveto, *_up, *_down, *_right, *_left,
     *_setbold, *_clearbold, *_setunderline, *_clearunderline, 
     *_sethalfbright, *_clearhalfbright, *_setinverse, *_clearinverse,
     *_cleartoeoln, *_cleartoeos, *_transmit_on, *_transmit_off;
static
int
     _lines, _columns;

static char _terminal[1024];              /* Storage for terminal entry */
static char _capabilities[256];           /* String for cursor motion */

static char *ptr = _capabilities;	/* for buffering         */

int    outchar();			/* char output for tputs */

InitScreen()
{
   /* Set up all this fun stuff: returns zero if all okay, or;
        -1 indicating no terminal name associated with this shell,
        -2..-n  No termcap for this terminal type known
   */

   int  tgetent(),      /* get termcap entry */
        error;
   char *tgetstr(),     /* Get termcap capability */
        termname[40];

#ifdef SUN
   if (getenv("TERM") == NULL)
     return(-1);
#endif

#ifdef UTS

    /* use _line for lack of a better variable, what the heck! */

    for (_line = 0; _line < SLEN; _line++)
	_null_string[_line] = '\0';
#endif

   if (strcpy(termname, getenv("TERM")) == NULL)
     return(-1);

   if ((error = tgetent(_terminal, termname)) != 1)
     return(error-2);

   _line  =  1;		/* where are we right now?? */
   _col   =  1;		/* assume zero, zero...     */

   /* load in all those pesky values */
   _clearscreen       = tgetstr("cl", &ptr);
   _moveto            = tgetstr("cm", &ptr);
   _up                = tgetstr("up", &ptr);
   _down              = tgetstr("do", &ptr);
   _right             = tgetstr("nd", &ptr);
   _left              = tgetstr("bs", &ptr); 
   _setbold           = tgetstr("so", &ptr);
   _clearbold         = tgetstr("se", &ptr);
   _setunderline      = tgetstr("us", &ptr);
   _clearunderline    = tgetstr("ue", &ptr);
   _setinverse        = tgetstr("so", &ptr);
   _clearinverse      = tgetstr("se", &ptr);
   _sethalfbright     = tgetstr("hs", &ptr);
   _clearhalfbright   = tgetstr("he", &ptr);
   _cleartoeoln       = tgetstr("ce", &ptr);
   _cleartoeos        = tgetstr("cd", &ptr);
   _lines	      = tgetnum("li");
   _columns	      = tgetnum("co");
   _transmit_on	      = tgetstr("ks", &ptr);
   _transmit_off      = tgetstr("ke", &ptr);


   if (!_left) {
      _left = ptr;
      *ptr++ = '\b';
      *ptr++ = '\0';
   }

#ifdef BSD
	initscr();	/* initalize curses too! */
#endif

   return(0);
}

char *return_value_of(termcap_label)
char *termcap_label;
{
	/** This will return the string kept by termcap for the 
	    specified capability. Modified to ensure that if 
	    tgetstr returns a pointer to a transient address	
	    that we won't bomb out with a later segmentation
	    fault (thanks to Dave@Infopro for this one!) **/

	static char escape_sequence[20];	

   	char *tgetstr();     		/* Get termcap capability */

	strcpy(escape_sequence, tgetstr(termcap_label, &ptr));
	return( (char *) escape_sequence);
}

transmit_functions(newstate)
int newstate;
{
	/** turn function key transmission to ON | OFF **/
	
	if (newstate != _intransmit) {
	  _intransmit = ! _intransmit;
	  if (newstate == ON)
   	    tputs(_transmit_on, 1, outchar);
	  else 
   	    tputs(_transmit_off, 1, outchar);

   	  fflush(stdout);      /* clear the output buffer */
	}
}

/****** now into the 'meat' of the routines...the cursor stuff ******/

ScreenSize(lines, columns)
int *lines, *columns;
{
	/** returns the number of lines and columns on the display. **/

	*lines = _lines - 1;		/* assume index from zero */
	*columns = _columns;
}

ClearScreen()
{
        /* clear the screen: returns -1 if not capable */

#ifdef UTS
   if (isatube) {
     _clear_screen++;	/* queue up for clearing... */
     return(0);
   }
#endif

   if (!_clearscreen) 
     return(-1);

   tputs(_clearscreen, 1, outchar);
   fflush(stdout);      /* clear the output buffer */
   return(0);
}

MoveCursor(row, col)
int row, col;
{
        /** move cursor to the specified row column on the screen.
            0,0 is the top left! **/

           char *tgoto();
	   char *stuff;

	_line = row;		/* update current location... */
	_col  = col;

#ifdef UTS
	if (isatube) {
	  at row, col;
	  return(0);
	}
#endif
        if (!_moveto) 
          return(-1);

        stuff = (char *) tgoto(_moveto, col, row);
	tputs(stuff, 1, outchar);
        fflush(stdout);
        return(0);
}


CursorUp()
{
        /** move the cursor up one line **/

	_line = (_line> 0? _line - 1: _line);	/* up one line... */

#ifdef UTS
	if (isatube) {
	  at _line, _col;
	  return(0);
	}
#endif
        if (!_up)
           return(-1);

   	tputs(_up, 1, outchar);
	fflush(stdout);
        return(0);
}


CursorDown()
{
        /** move the cursor down one line **/

	_line = (_line< LINES? _line + 1: _line);	/* up one line... */

#ifdef UTS
	if (isatube) {
	  at _line, _col ;
	  return(0);
	}
#endif

       if (!_down) 
          return(-1);

       tputs(_down, 1, outchar);
       fflush(stdout);
       return(0);
}


CursorLeft()
{
        /** move the cursor one character to the left **/

	_col = (_col > 0? _col - 1: _col);	/* up one line... */

#ifdef UTS
	if (isatube) {
	  at _line, _col;
	  return(0);
	}
#endif

       if (!_left) 
          return(-1);

       tputs(_left, 1, outchar);
       fflush(stdout);
       return(0);
}


CursorRight()
{
        /** move the cursor one character to the right (nondestructive) **/

	_col = (_col < COLUMNS? _col + 1: _col);	/* up one line... */

#ifdef UTS
	if (isatube) {
	  at _line, _col;
	  return(0);
	}
#endif

       if (!_right) 
          return(-1);

       tputs(_right, 1, outchar);
       fflush(stdout);
       return(0);
}


StartBold()
{
        /** start boldface/standout mode **/

       if (!_setbold) 
         return(-1);

       tputs(_setbold, 1, outchar);
       fflush(stdout);
       return(0);
}


EndBold()
{
        /** compliment of startbold **/

        if (!_clearbold) 
           return(-1);

       tputs(_clearbold, 1, outchar);
       fflush(stdout);
       return(0);
}


StartUnderline()
{
        /** start underline mode **/

       if (!_setunderline) 
          return(-1);

       tputs(_setunderline, 1, outchar);
       fflush(stdout);
       return(0);
}


EndUnderline()
{
        /** the compliment of start underline mode **/

       if (!_clearunderline) 
          return(-1);

       tputs(_clearunderline, 1, outchar);
       fflush(stdout);
       return(0);
}


StartHalfbright()
{
        /** start half intensity mode **/

       if (!_sethalfbright) 
         return(-1);

       tputs(_sethalfbright, 1, outchar);
       fflush(stdout);
       return(0);
}

EndHalfbright()
{
        /** compliment of starthalfbright **/

       if (!_clearhalfbright) 
          return(-1);

       tputs(_clearhalfbright, 1, outchar);
       fflush(stdout);
       return(0);
}

StartInverse()
{
        /** set inverse video mode **/

       if (!_setinverse) 
         return(-1);

       tputs(_setinverse, 1, outchar);
       fflush(stdout);
       return(0);
}


EndInverse()
{
        /** compliment of startinverse **/

       if (!_clearinverse) 
         return(-1);

       tputs(_clearinverse, 1, outchar);
       fflush(stdout);
       return(0);
}

PutLine0(x, y, line)
int x,y;
char *line;
{
	/** Write a zero argument line at location x,y **/

	_line = x;
	_col  = y;

#ifdef UTS
	if (isatube) {
	  panel (erase=_clear_screen, cursor=_line, _col) {
	    #ON, line, strlen(line)-1#
	  }
	  _clear_screen = 0;
	  _col += (printable_chars(line) - 1);
	  return(0);
	}
#endif
	MoveCursor(x,y);
	printf("%s", line);	/* to avoid '%' problems */
	fflush(stdout);
	_col += (printable_chars(line) - 1);
}

PutLine1(x,y, line, arg1)
int x,y;
char *line;
char *arg1;
{
	/** write line at location x,y - one argument... **/

	char buffer[SLEN];
	
	sprintf(buffer, line, arg1);

	PutLine0(x, y, buffer);
}

PutLine2(x,y, line, arg1, arg2)
int x,y;
char *line;
char *arg1, *arg2;
{
	/** write line at location x,y - one argument... **/

	char buffer[SLEN];
	
	sprintf(buffer, line, arg1, arg2);

	PutLine0(x, y, buffer);
}

PutLine3(x,y, line, arg1, arg2, arg3)
int x,y;
char *line;
char *arg1, *arg2, *arg3;
{
	/** write line at location x,y - one argument... **/

	char buffer[SLEN];
	
	sprintf(buffer, line, arg1, arg2, arg3);

	PutLine0(x, y, buffer);
}

CleartoEOLN()
{
        /** clear to end of line **/
#ifdef UTS
	char buffer[SLEN];
	register int cols, i = 0;

	if (isatube) {

	  for (cols = _col; cols < COLUMNS; cols++)
	    buffer[i++] = ' ';

	  buffer[i] = '\0';
	
	  panel (noerase, cursor=_line, _col) {
	    #ON, buffer, strlen(buffer)-1#
	  }
	}
#endif

       if (!_cleartoeoln) 
         return(-1);

       tputs(_cleartoeoln, 1, outchar);
       fflush(stdout);  /* clear the output buffer */
       return(0);
}

CleartoEOS()
{
        /** clear to end of screen **/

#ifdef UTS
	register int line_at;

	for (line_at = _line; line_at < LINES-1; line_at++) {
	  panel (noerase) { 
	    #ON, _null_string, COLUMNS# 
	  }
	}
	return(0);

#endif

       if (!_cleartoeos) 
         return(-1);

       tputs(_cleartoeos, 1, outchar);
       fflush(stdout);  /* clear the output buffer */
       return(0);
}

#ifdef RAWMODE

Raw(state)
int state;
{
	/** state is either ON or OFF, as indicated by call **/

        if (state == OFF && _inraw) {
#ifdef BSD
	  echo();
	  nocrmode();
#else
	  (void) ioctl(TTYIN, TCSETAW, &_original_tty);
#endif
          _inraw = 0;
	}
        else if (state == ON && ! _inraw) {
#ifdef BSD
	   noecho();
	   crmode();
#else
	  (void) ioctl(TTYIN, TCGETA, &_original_tty);	/** current setting **/
	  	
	  (void) ioctl(TTYIN, TCGETA, &_raw_tty);    /** again! **/
	  _raw_tty.c_iflag &= ~(INLCR | ICRNL |BRKINT);
	  _raw_tty.c_iflag |= IXON;
	  _raw_tty.c_oflag |= OPOST;
	  _raw_tty.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
	  _raw_tty.c_lflag &= ~(ICANON | ECHO);
	  _raw_tty.c_cc[VMIN] = '\01';
	  _raw_tty.c_cc[VTIME] = '\0';
	  (void) ioctl(TTYIN, TCSETAW, &_raw_tty);
#endif

          _inraw = 1;
        }
}

int
ReadCh()
{
        /** read a character with Raw mode set! **/

        register int result;
        char ch;

        result = read(0, &ch, 1);
	
	return(result == 0? EOF : ch);
}

#endif

outchar(c)
char c;
{
	/** output the given character.  From tputs... **/
	/** Note: this CANNOT be a macro!              **/

	putc(c, stdout);
}
