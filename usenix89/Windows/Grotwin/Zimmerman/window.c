#ifndef lint
static char sccsid[] = "@(#)window.c  2.2  [ (C) Nigel Holder 1986 ]";
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



#include <sys/ioctl.h>
#include "grotwin.h"


/*  externals defined in grotwin.c  */

/*  list of priorities - end referenced by tail variable  */
extern struct windowdetails	win[MAX_WINDOWS_PLUS];

extern int	window_priorities[];	/*  window details  */
extern int	**screen_priority;	/* which window visible at each point */
extern int	active_windows;		/*  number of user windows active  */
extern int	screenlines;		/*  lines on users screen  */
extern int	screencolumns;		/*  columns on users screen  */
extern int	time_start;		/*  where time string is placed  */
extern int	time_end;		/*  where time string ends  */
extern int	win_border;		/*  window border characteristic  */
extern int	timeon;			/*  display clock on top edge  */

extern char	**masterscreen;		/*  hook to stdscr from curses  */
extern char	*progname;		/*  program name stripped of any /'s  */
extern char	*home_shell;		/*  shell from users $SHELL variable  */
extern char	nottimestring[];	/*  used when time is not displayed  */


int	tail = 0;			/*  tail of window priority list  */



/*  create instance of window  */
createwin(winno, lines, columns, ystart, xstart)

int	winno, lines, columns, ystart, xstart;

{
	register struct windowdetails	*ptr;
	char	**virtwin(), *rindex(), *tty;

	ptr = &(win[winno]);

	/*  remember that a border surrounds window  */
	ptr->lines = lines - 2;
	ptr->columns = columns - 2;
	ptr->y_start = ystart + 1;
	ptr->x_start = xstart + 1;
	ptr->y_end = ptr->y_start + ptr->lines;
	ptr->x_end = ptr->x_start + ptr->columns; 
	ptr->y_current = 0;
	ptr->x_current = 0;
	ptr->screenptr = virtwin('\0');
	win_initialise(winno, ' ');
	/*  skip pass "/dev/"  */
	tty = 1 + rindex(ptr->pseudo_ttyname, '/');
	utmp_insert(tty, progname);

	ptr->page_buf_length = 0;
	strcpy(ptr->page_buf, "");
	ptr->output_blocked = FALSE;
	ptr->output_paged = FALSE;
	ptr->paged_page_full = FALSE;
	ptr->line_count = 0;
	ptr->overwrite = FALSE;
	ptr->cursor_addressing = FALSE;
	ptr->cursor_addr_passes = 0;
	ptr->cursor_addr_row = 0;
	ptr->standout_mode = WIN_STANDEND;
	ptr->readgrain = (ptr->lines / READGRAIN) * ptr->columns;
	if (ptr->readgrain > BUFSIZ)   {
		ptr->readgrain = BUFSIZ;
	}
	ptr->active = TRUE;
	++active_windows;

	top_dog(winno);

	/*  place cursor in new window  */
	wmove(stdscr, ptr->y_current + ptr->y_start,
						ptr->x_current + ptr->x_start);
	touchwin(stdscr);
	wrefresh(stdscr);
}



/*  provide user with half screen height window at near full screen width  */

new_window()

{
	static int	i = OVERLAY - 1;
	int		columns, rows, xstart, ystart, new_winno;

	i = (i + 1) % OVERLAY;
	columns = screencolumns - (OVERLAY * 2);
	rows = screenlines / 2;
	/*  offset slightly to enable multpile instances to be seen  */
	xstart = (i + 1) * 2;
	ystart = (rows / 2) + i; 
	check_size(&columns, &rows, &xstart, &ystart);
	new_winno = win_instance(rows, columns, ystart, xstart, home_shell);
	if (new_winno != -1)   {
		next_input_window(new_winno);
	}
	else   {
		write(1, "\007", 1);
	}
}



/*  allocate a window screen to hold characters and initialise it  */

char **
virtwin(filler)

char	filler;

{
	char	*calloc();
	register int	line, x;
	register char	**virt_ptr, *lineptr;

	virt_ptr = (char **) calloc(screenlines, sizeof (char *));
	for (line = 0 ; line < screenlines ; ++line)   {
		virt_ptr[line] = calloc(screencolumns, sizeof(char));
		lineptr = virt_ptr[line];
		for (x = 0 ; x < screencolumns ; ++x)   {
			lineptr[x] = filler;
		}
	}
	return(virt_ptr);
}



free_virt_win(virt_ptr)			/*  return structure to free list  */

char	**virt_ptr;

{
	int	line;

	for (line = 0 ; line < screenlines ; ++line)   {
		free(virt_ptr[line]);
	}
	free(virt_ptr);
}



/*******************
*   Screen priority positions to decide display areas.
*   Originally declared via virtwin(), but has been changed to use
*   ints (instead of chars) to hopefully speed it up (by requiring less
*   conversions to ints for comparisons etc.).
*******************/


int **
priority_win()

{
	char		*calloc();
	register int	x, *lineptr, **priority_ptr, line;

	priority_ptr = (int **) calloc(screenlines, sizeof (int *));
	for (line = 0 ; line < screenlines ; ++line)   {
		priority_ptr[line] = (int *) calloc(screencolumns, sizeof(int));
		lineptr = priority_ptr[line];
		for (x = 0 ; x < screencolumns ; ++x)   {
			lineptr[x] = -1;
		}
	}
	return(priority_ptr);
}



free_priority_win(priority_ptr)		/*  return structure to free store  */

int	**priority_ptr;

{
	register int	line;

	for (line = 0 ; line < screenlines ; ++line)   {
		free(priority_ptr[line]);
	}
	free(priority_ptr);
}



/*  fill window with specified fill pattern (usually a ' ')  */

win_initialise(winno, filler)

int	winno;
char	filler;

{
	register	char *lineptr;
	register	int x, y;
	register	int y_end, x_end;
	register	struct windowdetails *ptr;
	int		y_start, x_start;

	ptr = &(win[winno]);
	y_start = ptr->y_start;
	x_start = ptr->x_start;
	y_end = ptr->y_end;
	x_end = ptr->x_end;

	/*  fill window with filler  */
	for (y = y_start ; y < y_end ; ++y)   {
		lineptr = ptr->screenptr[y];
		for (x = x_start ; x < x_end ; ++x)   {
			lineptr[x] = filler;
		}
	}
	border(winno, BOX);
}



border(winno, command)				/*  place box around window  */

int	 winno, command;

{
	register struct windowdetails	 *ptr;
	register int	 x, y, y_end, x_end;
	int		 top, side, y_start, x_start;
	char		 winid;

	if (command == BOX)   {
		top = '-' | win_border;
		side = '|' | win_border;
	}
	else   {	/*  NOT_BOX  */
		top = ' ';
		side = ' ';
	}
	ptr = &(win[winno]);
	y_start = ptr->y_start - 1;
	x_start = ptr->x_start - 1;
	y_end = ptr->y_end;
	x_end = ptr->x_end;
	/*  side of box  */
	for (y = y_start + 1; y < y_end ; ++y)   {
		ptr->screenptr[y][x_start] = side;
		ptr->screenptr[y][x_end] = side;
	}
	/*  top and bottom  */
	for (x = x_start + 1 ;  x < x_end ; ++x)   {
		ptr->screenptr[y_start][x] = top;
		ptr->screenptr[y_end][x] = top;
	}
	ptr->screenptr[y_start][x_start] = ' ';
	ptr->screenptr[y_start][x_end] = ' ';
	ptr->screenptr[y_end][x_start] = ' ';
	ptr->screenptr[y_end][x_end] = ' ';
	/*  place window number in top corners (ish)  */
	if (winno < MAX_WINDOWS  &&  top != ' ')   {
		winid = '0' + winno;
		ptr->screenptr[y_start][x_start + 3] = winid;
		ptr->screenptr[y_start][x_end - 3] = winid;
	}
}



/*  make window topmost one and adjust visibility priority list accordingly  */

top_dog(winno)

int	winno;

{
	register struct windowdetails	*ptr;
	register int	x, *line_priority;
	register char	*lineptr, *masterline;
	int		y, y_start, x_start, y_end, x_end, start, i;

	ptr = &(win[winno]);
	y_start = ptr->y_start - 1;
	x_start = ptr->x_start - 1;
	y_end = ptr->y_end;
	x_end = ptr->x_end;

	/*  ensure that time is not overwritten on top border  */
	if (y_start == 0)   {
		lineptr = ptr->screenptr[0];
		line_priority = screen_priority[0];
		masterline = masterscreen[0];
		for (x = x_start ; x <= x_end ; ++x)   {
			if (x < time_start  ||  x > time_end)   {
				line_priority[x] = winno;
				masterline[x] = lineptr[x];
			}
			else   {
				nottimestring[x - time_start] = lineptr[x];
				line_priority[x] = winno;
				if (timeon == FALSE)   {
					masterline[x] = lineptr[x];
				}
			}
		}
		++y_start;
	}
	for (y = y_start ; y <= y_end ; ++y)   {	
		lineptr = ptr->screenptr[y];
		line_priority = screen_priority[y];
		masterline = masterscreen[y];
		for (x = x_start; x <= x_end ; ++x)   {
			line_priority[x] = winno;
			masterline[x] = lineptr[x];
		}
	}

	/*  find window in list  */
	for (i = 0 ; i < tail ; ++i)   {
		if (window_priorities[i] == winno)   {
			break;
		}
	}
	if (i == tail)   {		/*  new window  */
		++tail;
	}
	/*  push down priorities and place latest on top  */
	while (i > 0)   {
		window_priorities[i] = window_priorities[i - 1];
		--i;
	}
	window_priorities[0] = winno;
}



/*******************
*   Either make window backmost one or remove completely by
*   making window lie behind fixed screen background window.
*   Adjust visibility priority list accordingly.
*******************/

bottom_dog(winno, status)

int	winno, status;

{
	register struct windowdetails	*ptr;
	register int	x, *line_priority;
	register char	*lineptr, *masterline;
	int		y, i, j, start, end;

	/*  find position in list  */
	for (start = 0 ; start < tail ; ++start)   {
		if (window_priorities[start] == winno)   {
			break;
		}
	}
	if (status == HIDE)   {
		end = tail - 2;
	}
	else   {	/*  REMOVE  */
		end = tail - 1;
		--tail;		/*  since window no longer exists  */
	}
	/*  move up and place winno in list  */
	for (i = start ; i < end ; ++i)   {
		window_priorities[i] = window_priorities[i + 1];
	}
	window_priorities[end] = winno;

	/*  dumb removal of window whilst maintaining window priorities  */
	ptr = &(win[winno]);
	for (i = tail - 1 ; i >= 0 ; --i)   {
		j = window_priorities[i];
		for (y = ptr->y_start - 1 ; y <= ptr->y_end ; ++y)   {
			lineptr = win[j].screenptr[y];
			line_priority = screen_priority[y];
			masterline = masterscreen[y];
			for (x = ptr->x_start - 1 ; x <= ptr->x_end ; ++x)   {
				/*  ensure that time is not overwritten  */
				if (y == 0 && x >= time_start && x <= time_end){
					if (lineptr[x] != '\0')   {
						nottimestring[x - time_start] =
								lineptr[x];
					}
					if (timeon == TRUE)   {
						continue;
					}
				}
				if (lineptr[x] != '\0')   {
					line_priority[x] = j;
					masterline[x] = lineptr[x];
				}
			}
		}
	}
}



top_corners(winno)		/*  expose top corners of a window  */

int	winno;

{
	register struct windowdetails	*ptr;
	register	int y, x;
	int	diff;

	ptr = &(win[winno]);
	y = ptr->y_start - 1;
	x = ptr->x_start - 1;
	diff = ptr->x_end - x;
	/*  for both top corners  */
	for ( ; x <= ptr->x_end ; x += diff)   {
		if (screen_priority[y][x] == winno)   {
			if (y == 0)   {	/*  check for overwriting time  */
				if (x >= time_start  &&  x <= time_end)   {
					nottimestring[x - time_start] =
							ptr->screenptr[y][x];
					if (timeon == TRUE)   {
						continue;
					}
				}
			}
			masterscreen[y][x] = ptr->screenptr[y][x];
		}
	}
}



/*******************
*   Doesn't check if widen action will have any effect.  This allows
*   user to send SIGWINCH to process if he really wants to.
*******************/

widen_window(winno, direction)

int	winno, direction;

{
	struct windowdetails	*ptr;
	int	old_x_start, old_x_end, x_diff, x;
	int	old_y_start, old_y_end, y_diff, y;
	char	**virtwin(), **old_screenptr;

	ptr = &(win[winno]);
	border(winno, NOT_BOX);			/*  remove window border */
	bottom_dog(winno, REMOVE);		/*  remove window  */
	if (direction == HORIZONTAL_EXPAND)   {
		y_diff = 0;
		old_x_start = ptr->x_start;	/*  set up new x size  */
		old_x_end = ptr->x_end;
		ptr->x_start = 1;
		x_diff = old_x_start - ptr->x_start;
		ptr->columns = screencolumns - 2;
		ptr->x_end = ptr->x_start + ptr->columns;
	}
	else   {	/*  VERTICAL_EXPAND  */
		x_diff = 0;
		old_y_start = ptr->y_start;	/*  set up y new size  */
		old_y_end = ptr->y_end;
		ptr->y_start = 1;
		y_diff = old_y_start - ptr->y_start;
		ptr->lines = screenlines - 2;
		ptr->y_end = ptr->y_start + ptr->lines;
	}
	check_size(&ptr->columns, &ptr->lines, &ptr->x_start, &ptr->y_start);
	/*  copy old window contents to new window  */
	old_screenptr = ptr->screenptr;
	ptr->screenptr = virtwin('\0');
	win_initialise(winno, ' ');
	if (direction == HORIZONTAL_EXPAND)   {
		for (y = ptr->y_start ; y < ptr->y_end ; ++y)   {
			for (x = old_x_start ; x < old_x_end ; ++ x)   {
				ptr->screenptr[y][x - x_diff] =
							old_screenptr[y][x];
			}
		}
	}
	else   {	/*  VERTICAL_EXPAND  */
		for (y = old_y_start ; y < old_y_end ; ++y)   {
			for (x = ptr->x_start ; x < ptr->x_end ; ++ x)   {
				ptr->screenptr[y - y_diff][x] =
							old_screenptr[y][x];
			}
		}
	}
	top_dog(winno);
	next_input_window(winno);

	/*******************
	*   place cusor in window - can't rely on manager() to do this
	*   with input_changed flag since it may be servicing output before
	*   input_changed check is performed again !
	*******************/
	wmove(stdscr, ptr->y_current + ptr->y_start,
						ptr->x_current + ptr->x_start);
	free_virt_win(old_screenptr);
	/*  set new (?) tty size  */
	set_tty_size(ptr->slavefd, ptr->lines, ptr->columns, TRUE);
}



window_info(winno)		/*  display information about windows  */

int	winno;

{
	static char	command_string[] =
			"[ w or 0-9 for next window, return to exit ] ";
	int	found;
	char	c;

	while (1)   {
		selected_window_info(winno);
		fputs(command_string, stdout);
		fflush(stdout);
		do   {
			read(1, &c, 1);
			if (c == '\r'  ||  c == '\n')   {
				return(FALSE);
			}
			found = FALSE;
			if (c == 'w')   {
				do   {
					winno = (winno + 1) % MAX_WINDOWS;
				}   while (win[winno].active == FALSE);
				found = TRUE;
			}
			else   {
				winno = c - '0';
				if (winno >= 0  &&  winno <= MAX_WINDOWS)   {
					if (win[winno].active == TRUE)   {
						found = TRUE;
					}
				}
				else   {
					winno += '0';		/*  restore  */
				}
			}
		if (found == TRUE)   {		/*  remove command  */
			clear_line(sizeof(command_string));
		}
		else   {			/*  indicate error  */
			write(1, "\007", 1);
		}
		}   while (found != TRUE);
	}
}



static
selected_window_info(winno)	/*  display dumbly status of a window  */

int winno;

{
	struct windowdetails	*ptr;
	char	*status(), *truth_status();

	ptr = &(win[winno]);
	printf("\r\nstatus information for window %d\r\n\n", winno);
	printf("      tty name  -  %s\r\n", ptr->pseudo_ttyname);
	printf("         lines  -  %-6d\r\n", ptr->lines);
	printf("       columns  -  %-6d\r\n", ptr->columns);
	printf("       running  -  %s  [ pid = %d ]\r\n",
						ptr->progy, ptr->slavepid);
	printf("          XOFF  -  %s\r\n", status(ptr->output_blocked));
	printf("     page mode  -  %s\r\n", status(ptr->output_paged));
	if (ptr->output_paged)   {
		printf("page mode full  -  %s\r\n",
					truth_status(ptr->paged_page_full));
	}
	printf("page overwrite  -  %s\r\n", status(ptr->overwrite)); 
	putchar('\n');
}



char *
status(expr)

int expr;

{
	if (expr)   {
		return("ON");
	}
	else   {
		return("OFF");
	}
}



char *
truth_status(expr)

int expr;

{
	if (expr)   {
		return("TRUE");
	}
	else   {
		return("FALSE");
	}
}
