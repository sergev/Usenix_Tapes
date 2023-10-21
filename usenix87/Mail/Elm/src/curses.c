/** 			curses.c		**/

/**  This library gives programs the ability to easily access the
     termcap information and write screen oriented and raw input
     programs.  The routines can be called as needed, except that
     to use the cursor / screen routines there must be a call to
     InitScreen() first.  The 'Raw' input routine can be used
     independently, however.

     Modified 2/86 to work (hopefully) on Berkeley systems.  If
     there are any problems with BSD Unix, please report them to
     the author at taylor@hplabs (fixed, if possible!)

     Modified 5/86 to add memory lock support, thanks to the
     suggested code by Steve Wolf.

     Modified (as if I'm keeping track) to add 24,80 defaults

     (C) Copyright 1985 Dave Taylor, HP Colorado Networks
**/

#include "headers.h"

#ifdef RAWMODE
# ifdef BSD
#  ifndef BSD4.1
#    include <sgtty.h>
#  else
#    include <termio.h>
#  endif
# else
#  include <termio.h>
# endif
#endif

#include <ctype.h>

#ifdef BSD
#undef tolower
#endif
#include "curses.h"

#ifdef RAWMODE
# define TTYIN	0
#endif

#ifdef SHORTNAMES
# define CleartoEOS	ClrtoEOS
# define _clearinverse	_clrinv
# define _cleartoeoln	_clrtoeoln
# define _cleartoeos	_clr2eos
# define _transmit_off	xmit_off
# define _transmit_on	xmit_on
#endif

extern int debug;

#ifdef RAWMODE
#  ifndef BSD
struct termio _raw_tty, 
              _original_tty;
#  else
#    define TCGETA	TIOCGETP
#    define TCSETAW	TIOCSETP

struct sgttyb _raw_tty,
	      _original_tty;
#  endif

static int _inraw = 0;                  /* are we IN rawmode?    */

#endif

#ifdef UTS
static int _clear_screen = 0;		/* Next i/o clear screen? */
static char _null_string[SLEN];		/* a string of nulls...   */
#endif


#define DEFAULT_LINES_ON_TERMINAL	24
#define DEFAULT_COLUMNS_ON_TERMINAL	80

static int _memory_locked = 0;		/* are we IN memlock??   */
static int _line  = -1,			/* initialize to "trash" */
           _col   = -1;

static int _intransmit;			/* are we transmitting keys? */

static
char *_clearscreen, *_moveto, *_up, *_down, *_right, *_left,
     *_setbold, *_clearbold, *_setunderline, *_clearunderline, 
     *_sethalfbright, *_clearhalfbright, *_setinverse, *_clearinverse,
     *_cleartoeoln, *_cleartoeos, *_transmit_on, *_transmit_off,
     *_set_memlock, *_clear_memlock;

static
int _lines, _columns;

static char _terminal[1024];              /* Storage for terminal entry */
static char _capabilities[1024];           /* String for cursor motion */

static char *ptr = _capabilities;	/* for buffering         */

int    outchar();			/* char output for tputs */
char  *tgetstr(),     		       /* Get termcap capability */
      *tgoto();				/* and the goto stuff    */

InitScreen()
{
	/* Set up all this fun stuff: returns zero if all okay, or;
        -1 indicating no terminal name associated with this shell,
        -2..-n  No termcap for this terminal type known
   */

	int  tgetent(),      /* get termcap entry */
	     error;
	char termname[40];
	char *strcpy(), *getenv();
	
	dprint0(8,"InitScreen()\n");

	if (getenv("TERM") == NULL) return(-1);

#ifdef UTS

	/* use _line for lack of a better variable, what the heck! */

	for (_line = 0; _line < SLEN; _line++)
		_null_string[_line] = '\0';
#endif

	if (strcpy(termname, getenv("TERM")) == NULL)
		return(-1);

	if ((error = tgetent(_terminal, termname)) != 1)
		return(error-2);

	_line  =  0;		/* where are we right now?? */
	_col   =  0;		/* assume zero, zero...     */

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
	_lines	      	   = tgetnum("li");
	_columns	   = tgetnum("co");
	_transmit_on	   = tgetstr("ks", &ptr);
	_transmit_off      = tgetstr("ke", &ptr);
	_set_memlock	   = tgetstr("ml", &ptr);
	_clear_memlock	   = tgetstr("mu", &ptr);


	if (!_left) {
		_left = ptr;
		*ptr++ = '\b';
		*ptr++ = '\0';
	}

	return(0);
}

char *return_value_of(termcap_label)
char *termcap_label;
{
	/** This will return the string kept by termcap for the 
	    specified capability. Modified to ensure that if 
	    tgetstr returns a pointer to a transient address	
	    that we won't bomb out with a later segmentation
	    fault (thanks to Dave@Infopro for this one!)

	    Tweaked to remove padding sequences.
	 **/

	static char escape_sequence[20];
	register int i=0,j=0;
	char buffer[20];
	char *myptr, *tgetstr();     		/* Get termcap capability */

	dprint1(9,"return_value_of(%s)\n", termcap_label);

	if (strlen(termcap_label) < 2)
	  return(NULL);

	if (termcap_label[0] == 's' && termcap_label[1] == 'o')
	  strcpy(escape_sequence, _setinverse);
	else if (termcap_label[0] == 's' && termcap_label[1] == 'e')
	  strcpy(escape_sequence, _clearinverse);
	else if ((myptr = tgetstr(termcap_label, &ptr)) == NULL)
	  return( (char *) NULL );
	else
	  strcpy(escape_sequence, myptr);

	if (chloc(escape_sequence, '$') != -1) {
	  while (escape_sequence[i] != '\0') {
	    while (escape_sequence[i] != '$' && escape_sequence[i] != '\0')
	      buffer[j++] = escape_sequence[i++];
	    if (escape_sequence[i] == '$') {
	      while (escape_sequence[i] != '>') i++;
	      i++;
	    }
	  }
	  buffer[j] = '\0';
	  strcpy(escape_sequence, buffer);
	}

	return( (char *) escape_sequence);
}

transmit_functions(newstate)
int newstate;
{
	/** turn function key transmission to ON | OFF **/

	dprint1(9,"transmit_functions(%s)\n", onoff(newstate));

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

	if (_lines == 0) _lines = DEFAULT_LINES_ON_TERMINAL;
	if (_columns == 0) _columns = DEFAULT_COLUMNS_ON_TERMINAL;

	dprint2(9,"ScreenSize(_,_) returning %d, %d\n", _lines-1, _columns);

	*lines = _lines - 1;		/* assume index from zero */
	*columns = _columns;
}

GetXYLocation(x,y)
int *x,*y;
{
	/* return the current cursor location on the screen */

	*x = _line;
	*y = _col;
}

ClearScreen()
{
	/* clear the screen: returns -1 if not capable */

	_line = 0;	/* clear leaves us at top... */
	_col  = 0;

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

	char *stuff, *tgoto();

	/* we don't want to change "rows" or we'll mangle scrolling... */

	if (col > COLUMNS) col = COLUMNS;
	if (col < 0)       col = 0;

#ifdef UTS
	if (isatube) {
		at row+1, col+1;
		_line = row;
		_col  = col;
		return(0);
	}
#endif
	if (!_moveto)
		return(-1);

	if (row == _line) {
	  if (col == _col)
	    return(0);				/* already there! */

	  else if (abs(col - _col) < 5) {	/* within 5 spaces... */
	    if (col > _col)
	      CursorRight(col - _col);
	    else 
	      CursorLeft(_col - col);
          }
	  else {		/* move along to the new x,y loc */
	    stuff = tgoto(_moveto, col, row);
	    tputs(stuff, 1, outchar);
	    fflush(stdout);
	  }
	}
	else if (col == _col && abs(row - _line) < 5) {
	  if (row < _line)
	    CursorUp(_line - row);
	  else
	    CursorDown(row - _line);
	}
	else if (_line == row-1 && col == 0) {
	  putchar('\n');	/* that's */
	  putchar('\r');	/*  easy! */
	  fflush(stdout);
	}
	else {
	  stuff = tgoto(_moveto, col, row);
	  tputs(stuff, 1, outchar);
	  fflush(stdout);
	}

	_line = row;	/* to ensure we're really there... */
	_col  = col;

	return(0);
}

CursorUp(n)
int n;
{
	/** move the cursor up 'n' lines **/

	_line = (_line-n > 0? _line - n: 0);	/* up 'n' lines... */

#ifdef UTS
	if (isatube) {
		at _line+1, _col+1;
		return(0);
	}
#endif
	if (!_up)
		return(-1);

	while (n-- > 0)
		tputs(_up, 1, outchar);

	fflush(stdout);
	return(0);
}


CursorDown(n)
int n;
{
	/** move the cursor down 'n' lines **/

	_line = (_line+n < LINES? _line + n: LINES);	/* down 'n' lines... */

#ifdef UTS
	if (isatube) {
		at _line+1, _col+1 ;
		return(0);
	}
#endif

	if (!_down)
		return(-1);

	while (n-- > 0)
		tputs(_down, 1, outchar);

	fflush(stdout);
	return(0);
}


CursorLeft(n)
int n;
{
	/** move the cursor 'n' characters to the left **/

	_col = (_col - n> 0? _col - n: 0);	/* left 'n' chars... */

#ifdef UTS
	if (isatube) {
		at _line+1, _col+1;
		return(0);
	}
#endif

	if (!_left)
		return(-1);

	while (n-- > 0)
		tputs(_left, 1, outchar);

	fflush(stdout);
	return(0);
}


CursorRight(n)
int n;
{
	/** move the cursor 'n' characters to the right (nondestructive) **/

	_col = (_col+n < COLUMNS? _col + n: COLUMNS);	/* right 'n' chars... */

#ifdef UTS
	if (isatube) {
		at _line+1, _col+1;
		return(0);
	}
#endif

	if (!_right)
		return(-1);

	while (n-- > 0)
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

int
HasMemlock()
{
	/** returns TRUE iff memory locking is available (a terminal
	    feature that allows a specified portion of the screen to
	    be "locked" & not cleared/scrolled... **/

	return ( _set_memlock && _clear_memlock );
}

static int _old_LINES;

int
StartMemlock()
{
	/** mark the current line as the "last" line of the portion to 
	    be memory locked (always relative to the top line of the
	    screen) Note that this will alter LINES so that it knows
	    the top is locked.  This means that (plus) the program 
	    will scroll nicely but (minus) End memlock MUST be called
	    whenever we leave the locked-memory part of the program! **/

	if (! _set_memlock)
	  return(-1);

	if (! _memory_locked) {

	  _old_LINES = LINES;
	  LINES -= _line;		/* we can't use this for scrolling */

	  tputs(_set_memlock, 1, outchar);
	  fflush(stdout);
	  _memory_locked = TRUE;
	}

	return(0);
}

int
EndMemlock()
{
	/** Clear the locked memory condition...  **/

	if (! _set_memlock)
	  return(-1);

	if (_memory_locked) {
	  LINES = _old_LINES;		/* back to old setting */
  
	  tputs(_clear_memlock, 1, outchar);
	  fflush(stdout);
	  _memory_locked = FALSE;
	}
	return(0);
}


Writechar(ch)
char ch;
{
	/** write a character to the current screen location. **/

#ifdef UTS
	char buffer[2];	/* can't output characters! */

	if (isatube) {
	  buffer[0] = ch;
	  buffer[1] = '\0';

	  at _line+1, _col+1;
	  panel (noerase, noinit, noread) {
	    #ON, buffer, 1#
	  }
 	}
	else
#endif
	  putchar(ch);

	if (ch == BACKSPACE)	/* moved BACK one! */
		_col--;
	else if (ch >= ' ')	/* moved FORWARD one! */
		_col++;
}

/*VARARGS2*/

Write_to_screen(line, argcount, arg1, arg2, arg3)
char *line;
int   argcount, arg1, arg2, arg3;
{
	/** This routine writes to the screen at the current location.
  	    when done, it increments lines & columns accordingly by
	    looking for "\n" sequences... **/

	switch (argcount) {
	case 0 :
		PutLine0(_line, _col, line);
		break;
	case 1 :
		PutLine1(_line, _col, line, arg1);
		break;
	case 2 :
		PutLine2(_line, _col, line, arg1, arg2);
		break;
	case 3 :
		PutLine3(_line, _col, line, arg1, arg2, arg3);
		break;
	}
}

PutLine0(x, y, line)
int x,y;
char *line;
{
	/** Write a zero argument line at location x,y **/

	register int i;

#ifdef UTS
	if (isatube) {
		at x+1, y+1;
		panel (init=_clear_screen, noread, erase=_clear_screen) {
		  #ON, line, strlen(line)-1#
		}
		_clear_screen = 0;
		_col += printable_chars(line);

		/* line wrapped around?? */
		while (_col > COLUMNS) {
		  _col  -= COLUMNS;
		  _line += 1;
		}

		/* now let's figure out if we're supposed to do a "<return>" */

		for (i=0; i < strlen(line); i++)
			if (line[i] == '\n') {
				_line++;
				_col = 0;		/* on new line! */
			}
		return(0);
	}
#endif
	MoveCursor(x,y);
	printf("%s", line);	/* to avoid '%' problems */
	fflush(stdout);
	_col += printable_chars(line);

	while (_col > COLUMNS) { 	/* line wrapped around?? */
		_col -= COLUMNS;
		_line += 1;
	}

	/** now let's figure out if we're supposed to do a "<return>" **/

	for (i=0; i < strlen(line); i++)
		if (line[i] == '\n') {
			_line++;
			_col = 0;		/* on new line! */
		}
}

/*VARARGS2*/
PutLine1(x,y, line, arg1)
int x,y;
char *line;
char *arg1;
{
	/** write line at location x,y - one argument... **/

	char buffer[VERY_LONG_STRING];

	sprintf(buffer, line, arg1);

	PutLine0(x, y, buffer);
}

/*VARARGS2*/
PutLine2(x,y, line, arg1, arg2)
int x,y;
char *line;
char *arg1, *arg2;
{
	/** write line at location x,y - one argument... **/

	char buffer[VERY_LONG_STRING];

	sprintf(buffer, line, arg1, arg2);

	PutLine0(x, y, buffer);
}

/*VARARGS2*/
PutLine3(x,y, line, arg1, arg2, arg3)
int x,y;
char *line;
char *arg1, *arg2, *arg3;
{
	/** write line at location x,y - one argument... **/

	char buffer[VERY_LONG_STRING];

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

		at _line+1, _col+1;
		panel (noerase, noinit, noread) {
		  #ON, buffer, strlen(buffer)-1#
		}

		return(0);
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

	if (isatube) {
	  for (line_at = _line; line_at < LINES-1; line_at++) {
		panel (noread, noinit, noread) {
	            #@ line_at, 1# #ON, _null_string, COLUMNS# 
		}
	  }
	
  	  return(0);
	}

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
	  (void) ioctl(TTYIN, TCSETAW, &_original_tty);
	  _inraw = 0;
	}
	else if (state == ON && ! _inraw) {

	  (void) ioctl(TTYIN, TCGETA, &_original_tty);	/** current setting **/

	  (void) ioctl(TTYIN, TCGETA, &_raw_tty);    /** again! **/
#ifdef BSD
	  _raw_tty.sg_flags &= ~(ECHO | CRMOD);	/* echo off */
	  _raw_tty.sg_flags |= CBREAK;	/* raw on    */
#else
	  _raw_tty.c_lflag &= ~(ICANON | ECHO);	/* noecho raw mode        */

	  _raw_tty.c_cc[VMIN] = '\01';	/* minimum # of chars to queue    */
	  _raw_tty.c_cc[VTIME] = '\0';	/* minimum time to wait for input */

#endif
	  (void) ioctl(TTYIN, TCSETAW, &_raw_tty);

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
