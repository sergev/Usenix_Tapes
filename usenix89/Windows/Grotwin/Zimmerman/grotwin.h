/***************************************
*
*	Author  :  Nigel Holder
*
*	Date    :  10 July 1986
*
*	   Grotwin - provides a somewhat primitive windowing capability
*	for people unfortunate enough to use the standard 24 x 80 type
*	of terminal when the console is in use.  Definitely written for
*	4.[2,3] at the present time, as Sys V.2 does not cater for pseudo
*	terminals or have the select() facility (amongst other things !)
*	(version 8 should fix this).
*
*	Files used :-
*
*	   grotwin.c	 -	window system initialisation
*				deals with startup files
*
*	   manager.c	 -	window manager
*				deals with input and output
*
*	   window.c	 -	window manipulator
*				deals with aspects concerning windows
*				during normal usage
*
*	   update.c	 -	simulates dumb terminal for use
*				with window manager.
*
*	   grotwin.h	 -	header file for above
*
*	   utmp.c	 -	utmp manipulator
*				updates utmp entries for each window
*
*	   grotwin.make  -	makefile for above
*	   
*	Bugs :-
*
*	   Can't have the situation where no windows are present !
*	   Needs a vt100 like terminal type
*
***************************************/



#include <stdio.h>
#include <sys/param.h>
#include <curses.h>



/*******************
*      The following defines are very ascii dependent (which is good since its
*   anti-IBM EBSDIC)
*******************/


/*******************
*   maximum number of windows (maximum of 10 imposed), depends on
*   stdin, stdout, stderr, utmp update and two per window
*   (NOFILE is 30 on a Sun 2 running op sys version 2.2, normally 20
*   though on unix systems)
*******************/

#define		SYSTEM_WINDOWS		( (NOFILE - 4) / 2 )

#define		MAX_WINDOWS		( 10 )		/*  windows 0 to 9  */

#if		( MAX_WINDOWS > SYSTEM_WINDOWS )
#undef		MAX_WINDOWS
#define		MAX_WINDOWS		( SYSTEM_WINDOWS )
#endif		MAX_WINDOWS

#define		MAX_WINDOWS_PLUS	( MAX_WINDOWS + 1 )

#define		REALLY_CONFUSED		( 2 )
#define		READGRAIN		( 2 )
#define		CTRL_MASK		( 0x1F )
#define		WIN_SWITCH_NUMBER	( (unsigned char) 0x80 )

/*  user window control commands  */
#define		SELECT			( 'a' & CTRL_MASK )
#define		WIN_SWITCH		( 'w' & CTRL_MASK )
#define		NEW_WINDOW		( 'n' & CTRL_MASK )
#define		FORCED_EXIT		( 'f' & CTRL_MASK )
#define		REMOVE_WIN		( 'r' & CTRL_MASK )
#define		CLEAR			( 'z' & CTRL_MASK )
#define		MOVE			( 'm' & CTRL_MASK )
#define		EXPAND			( 'x' & CTRL_MASK )
#define		VERTICAL_EXPAND		( 'v' & CTRL_MASK )
#define		HORIZONTAL_EXPAND	( 'c' & CTRL_MASK )
#define		EXPOSE_WINDOW		( 'e' & CTRL_MASK )
#define		HIDE_WINDOW		( 'h' & CTRL_MASK )
#define		SCREEN_REDRAW		( 'l' & CTRL_MASK )
#define		XON			( 'q' & CTRL_MASK )
#define		XOFF			( 's' & CTRL_MASK )
#define		PAGE_MODE		( 'p' & CTRL_MASK )
#define		OVERWRITE_MODE		( 'o' & CTRL_MASK )
#define		TIME_TOGGLE		( 't' & CTRL_MASK )
#define		INFORM			( 'i' & CTRL_MASK )
#define		HELP			( '?' )

#define		BUFLEN			( 256 )
#define		TIME_RES		( 30 )
#define		HIDE			( 0 )
#define		DISPLAY			( 2 )
#define		REMOVE			( 1 )
#define		MAXARGS			( 20 )
#define		WORLD_MIN_COLUMNS	( 20 )
#define		WORLD_MIN_ROWS		( 6 )
#define		MIN_COLUMNS		( 3 )
#define		MIN_ROWS		( 4 )		/*  3 doesn't work  */
#define		OVERLAY			( 4 )
#define		BOX			( 1 )
#define		NOT_BOX			( 0 )
#define		BAD_INPUT		( 0 )
#define		MISTAKE			( 1 )
#define		NORMAL			( 2 )

#define		BACKGROUND		'.'

#define         WIN_STANDOUT            ( (char) 0x80 ) 
#define         WIN_STANDEND            ( (char) 0x00 ) 



struct windowdetails   {
		int	active;
		int	masterfd;
		int	slavefd;
		int	slavepid;
		int	slavemaskfd;
		int	output_blocked;
		int	output_paged;
		int	line_count;
		int	paged_page_full;
		int	overwrite;
		int	cursor_addressing;
		int	cursor_addr_passes;
		int	cursor_addr_row;
		int	lines;
		int	columns;
		int	x_current;
		int	y_current;
		int	x_start;
		int	y_start;
		int	x_end;
		int	y_end;
		int	readgrain;
		int	page_buf_length;
		char	**screenptr;
		char	*page_ptr;
		char	page_buf[BUFSIZ + 3];
		char	pseudo_ttyname[20];
		char	*progy;
		char	standout_mode;
};

/* dpz additions to grotwin.h for non-Sun machines 08/23/86 */

#ifndef SUN

struct ttysize {
	int	ts_lines;	/* number of lines on terminal */
	int	ts_cols;	/* number of columns on terminal */
};

#define	TIOCSSIZE	_IOW(t,103,struct ttysize)/* set tty size */
#define	SIGWINCH 28	/* window changed */
#undef  NOFILE          /* not on a sun, so make it 20 */
#define NOFILE 20

#endif
