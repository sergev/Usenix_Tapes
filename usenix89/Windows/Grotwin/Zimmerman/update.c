#ifndef lint
static char sccsid[] = "@(#)update.c  2.2  [ (C) Nigel Holder 1986 ]";
#endif

/***************************************
*
*	Author  :  Nigel Holder
*
*	Date    :  10 July 1986
*
*
*	Copyright (C) 1986  by Nigel Holder 
* 
*	   Permission to use this program is granted, provided it is not
*	sold, or distributed for direct commercial advantage, and includes
*	the copyright notice and this clause.
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



#include "grotwin.h"


/*  defines specific to grotty terminal type (see term and termcap below)  */

#define		INSERT_LINE		( 0x01 )
#define		DELETE_LINE		( 0x02 )
#define		STANDOUT		( 0x05 )
#define		STANDEND		( 0x06 )
#define		BELL			( 0x07 )
#define		BACKSPACE		( 0x08 )
#define		TAB			( 0x09 )
#define		CURSOR_DOWN		( 0x0A )
#define		CURSOR_UP		( 0x0B )
#define		CLEAR_SCREEN		( 0x0C )
#define		NONDESTRUCT_SPACE	( 0x18 )
#define		DELETE_TO_EOL		( 0x19 )
#define		CURSOR_ADDR		( 0x1B )
#define		TABS			( 0x08 )

#define		SPECIAL			'\0'



extern struct windowdetails	win[MAX_WINDOWS_PLUS];

extern int	**screen_priority;

extern char	**masterscreen;



/*******************
*
*   My very own terminal type.  Loosely based on a adm3a and Newbury 8000
*   Used in grotwin.c but defined here since this module handles terminal
*   escape sequences.  Although a vt100 definition would probably be better,
*   this simple terminal type generally uses less I/O to achieve the same
*   goals (ie. cursor movement), at the cost of programs that use
*   vt100 escape sequences without looking at TERMCAP.
*
*   Termcap entry should also be placed in in /etc/termcap as well.
*
*******************/

char	term[]    = "TERM=grotty";
char	termcap[] = "TERMCAP=gr|grotty|grotwin pseudo tty:\
co#%d:li#%d:cl=\^L:cm=\\E=%%+ %%+ :am:bs:nd=\^X:ce=\^Y:\
up=\^K:do=\^J:kb=\^H:so=\^E:se=\^F:al=\^A:dl=\^B:";



/*  updates specified window with string  */

update_screen(winno)

int	winno;

{
	register struct windowdetails	*ptr;
	register int	x, y;
	int	length, temp;
	char	*strcpy(), *string;

	ptr = &(win[winno]);
	if (ptr->paged_page_full  ||  ptr->output_blocked)   {	/* can't o/p */
		return;
	}
	string = ptr->page_buf;
	length = ptr->page_buf_length;
	while (length-- > 0)   {
		*string &= 0x7F;	/*  strip off top most bit  */
		if (ptr->cursor_addressing)   {
			temp = *string++;
			/*  check for cursor addressing  */
			if (++(ptr->cursor_addr_passes) == 1)   {
				if (temp != '=')   {
					ptr->cursor_addressing = FALSE;
					ptr->cursor_addr_passes = 0;
					mywaddch(winno, CURSOR_ADDR);
					mywaddch(winno, temp);
					continue;
				}
			}
			else   {
				if (ptr->cursor_addr_passes == 2)   {
					ptr->cursor_addr_row = temp - ' ';
					if (ptr->cursor_addr_row >= ptr->lines){
						ptr->cursor_addr_row =
								ptr->lines - 1;
					}
					else   {
						if (ptr->cursor_addr_row < 0)  {
							ptr->cursor_addr_row =0;
						}
					}
				}
				else   {
					/*  move it  */
					ptr->y_current = ptr->cursor_addr_row;
					ptr->x_current = temp - ' ';
					if (ptr->x_current >= ptr->columns)   {
						ptr->x_current = ptr->columns-1;
					}
					else   {
						if (ptr->x_current < 0)   {
							ptr->x_current = 0;
						}
					}
					ptr->cursor_addressing = FALSE;
					ptr->cursor_addr_passes = 0;
				}
			}
			continue;
		}
		y = ptr->y_current;
		x = ptr->x_current;
		switch (*string)   {
			case  '\r'  :
				ptr->x_current = 0;
				break;

			case  '\n'  :
				if (ptr->output_paged  &&
					    ++ptr->line_count >= ptr->lines)   {
					ptr->paged_page_full = TRUE;
					ptr->line_count = 0;
					/*  remember rest of output  */
					strcpy(ptr->page_buf, string);
					ptr->page_buf_length = length + 1;
					return;
				}
				if (y >= (ptr->y_end - ptr->y_start - 1))   {
					ptr->y_current = 0;
					if (! ptr->overwrite)   {
						mywlineop(winno, DELETE_LINE);
						ptr->y_current = y;
					}
				}
				else   {
					++ptr->y_current;
				}
				if (ptr->overwrite)   {
					mywline_clear(winno,
						ptr->y_current + ptr->y_start);
				}
				break;

			case  CURSOR_UP  :
				if (y > 0)   {
					--y;
				}
				ptr->y_current = y;
				break;

			case  CLEAR_SCREEN  :
				mywclear(winno);
				break;

			case  CURSOR_ADDR  :
				ptr->cursor_addressing = TRUE;
				break;

			case  TAB  :
				x =  x + TABS - (x  % TABS);
				if (x > ptr->columns - 1)   {
					x = ptr->columns - 1;
				}
				ptr->x_current = x;
				break;

			case  BACKSPACE  :
				if (x > 0)   {
					--ptr->x_current;
				}
				break;

			case  STANDOUT  :
				ptr->standout_mode = WIN_STANDOUT;
				break;

			case  STANDEND  :
				ptr->standout_mode = WIN_STANDEND;
				break;

			case  BELL  :
				write(1, "\007", 1);
				break;

			case  NONDESTRUCT_SPACE  :
				if (x < ptr->columns - 1)   {
					++x;
				}
				else   {
					ptr->y_current = ++y;
					x = 0;
				}
				ptr->x_current = x;
				break;

			case  INSERT_LINE  :
				mywlineop(winno, INSERT_LINE);
				break;

			case  DELETE_LINE  :
				mywlineop(winno, DELETE_LINE);
				break;

			case  DELETE_TO_EOL  :
				temp = x;
				while (temp++ < ptr->columns)   {
					mywaddch(winno, ' ');
				}
				ptr->x_current = x;
				break;

			case  SPECIAL  :
				break;

			default  :
				mywaddch(winno, *string);
				if (x >= ptr->columns - 1)   {
					/*  insert \r\n into string  */
					length = string_insert(string + 1,
									"\r\n");
				}
				break;
		}
		++string;
	}
	ptr->page_buf_length = 0;
}



/*  insert s2 at beginning of s1 after shifting s2 to make room  */
/*  returns length of new string  */

static
string_insert(s1, s2)

char	*s1, *s2;

{
	register char	*src_ptr, *dest_ptr;
	register int	i;
	int		n1, n2;

	n1 = strlen(s1);
	n2 = strlen(s2);
	src_ptr = s1 + n1;
	dest_ptr = src_ptr + n2;
	/*  include string delimiter  */
	i = n1 + 1;
	while (i--)   {
		*dest_ptr-- = *src_ptr--;
	}
	i = n2;
	while (i--)   {
		*s1++ = *s2++;
	}
	return(n1 + n2);
}



static
mywaddch(winno, c)		/*  add char to window  */

int	winno;
char	c;

{
	register struct windowdetails	*ptr;
	register int	x, y;

	ptr = &(win[winno]);
	x = ptr->x_current + ptr->x_start;
	y = ptr->y_current + ptr->y_start;
	c |= ptr->standout_mode;
	ptr->screenptr[y][x] = c;
	if (screen_priority[y][x] == winno)   {
		masterscreen[y][x] = c;
	}
	++ptr->x_current;
}



static
mywlineop(winno, command)	/*  insert - delete a line on the screen  */

int	winno, command;

{
	register struct windowdetails *ptr;
	register int x, *line_priority;
	register char *image, *screenline;
	int y, y_actual_current;
	char *temp;

	ptr = &(win[winno]);
	y_actual_current = ptr->y_current + ptr->y_start;

	/*  move lines around  */
	if (command == DELETE_LINE)   {
		temp = ptr->screenptr[y_actual_current];
		for (y = y_actual_current ; y < ptr->y_end - 1 ; ++y)   {
			ptr->screenptr[y] = ptr->screenptr[y + 1];
		}
		ptr->screenptr[ptr->y_end - 1] = temp;
		mywline_clear(winno, ptr->y_end - 1);
	}
	else   {	/*  INSERT_LINE  */
		temp = ptr->screenptr[ptr->y_end - 1];
		for (y = ptr->y_end - 1 ; y > y_actual_current ; --y)   {
			ptr->screenptr[y] = ptr->screenptr[y - 1];
		}
		ptr->screenptr[y_actual_current] = temp;
		mywline_clear(winno, y_actual_current);
	}

	/*  update real screen  */
	for (y = y_actual_current ; y < ptr->y_end ; ++y)   {
		line_priority = screen_priority[y];
		screenline = masterscreen[y];
		image = ptr->screenptr[y];
		for (x = ptr->x_start ; x < ptr->x_end ; ++x) {
			if (line_priority[x] == winno)   {
				screenline[x] = image[x];
			}
		}
	}
}



static
mywline_clear(winno, line)

int	winno, line;

{
	register struct windowdetails	*ptr;
	register int	x, *line_priority;
	register char	*lineptr, *masterline;

	ptr = &(win[winno]);
	lineptr = ptr->screenptr[line];
	line_priority = screen_priority[line];
	masterline = masterscreen[line];
	for (x = ptr->x_start ; x < ptr->x_end ; ++x)   {
		lineptr[x] = ' ';
		if (line_priority[x] == winno)   {
			masterline[x] = ' ';
		}
	}
}



mywclear(winno)					/*  clear window  */

int	winno;

{
	struct windowdetails	*ptr;
	int	y;

	ptr = &(win[winno]);
	for (y = ptr->y_start ; y < ptr->y_end ; ++y)   {
		mywline_clear(winno, y);
	}
	ptr->y_current = 0;
	ptr->x_current = 0;
	ptr->line_count = 0;
}
