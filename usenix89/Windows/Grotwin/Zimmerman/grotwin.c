#ifndef lint
static char sccsid[] = "@(#)grotwin.c  2.2  [ (C) Nigel Holder 1986 ]";
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
*	  Permission to use this program is granted, provided it is not
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
#include <sys/file.h>
#include <signal.h>
#include "grotwin.h"



extern int	active_windows;		/*  number of user windows active  */



/*  system dependent stuff  */

static char	manfile[]       = "/usr/mrc/grotwin.man";
static char	helpfile[]      = "/usr/mrc/grotwin.help";
static char	system_more[]   = "more";
static char	default_shell[] = "/bin/sh";


static int	ignore_homefile = FALSE;	/*  look for default file  */

static char	initfile[BUFSIZ + 1];		/*  temp string buffer  */


/*  default startup values that may be changed by user command line  */
int	timeon = TRUE;			/*  display clock on top edge  */
int	win_border = WIN_STANDEND;	/*  normal brightness window borders  */

int	startup = TRUE;			/*  program startup period   */
int	confused = FALSE;		/*  for when program hangs up  */
int	**screen_priority = NULL;	/* which window visible at each point */

char	timestring[]	= " hh:ss ";	/*  time on top edge of b/g window  */
char	nottimestring[]	= "-------";	/*  used when time is not displayed  */


struct windowdetails	win[MAX_WINDOWS_PLUS];		/*  window details  */

struct sgttyb	masterterm;		/*  used to store initial tty state  */
struct tchars	mtchars;
int		mlocalmode;

int	screenlines;			/*  lines on users screen  */
int	screencolumns;			/*  columns on users screen  */
int	time_start;			/*  where time string is placed  */
int	time_end;			/*  where time string ends  */

char	**masterscreen;			/*  hook to stdscr from curses  */
char	*progname;			/*  program name stripped  of any /'s*/
char	*home_shell;			/*  shell from users $SHELL variable  */




main(argc, argv)

int argc;
char *argv[];

{
	int	length, i;
	char	*rindex(), *getenv();

	if ((progname = rindex(argv[0], '/')) != NULL)   {
		++progname;
	}
	else   {
		progname = argv[0];
	}
	/*  scan command line  */
	while (argc > 1  &&  argv[1][0] == '-')   {
		switch (argv[1][1])   {
			case  'n'  :
				ignore_homefile = TRUE;
				break;

			case  't'  :
				timeon = FALSE;
				break;

			case  'b'  :
				win_border = WIN_STANDOUT;
				break;

			case  'h'  :
				man_info();
				exit(0);
				break;

			default  :
				fprintf(stderr,
					"usage: %s [-n][-t][-b][-h] [file]\n\n",
								progname);
				fprintf(stderr, "\t\t-n\t-\tignore ");
				fprintf(stderr, "$HOME startup file\n");
				fprintf(stderr, "\t\t-t\t-\tturn time off\n");
				fprintf(stderr, "\t\t-b\t-\tbold windows\n");
				fprintf(stderr, "\t\t-h\t-\thelp info\n");
				exit(1);
				break;
		}
		--argc;		/*  skip over program name  */
		++argv;
	}
	if (isatty(0) == 0)   {
		fprintf(stderr, "%s: standard input not a tty\n", progname);
		exit(2);
	}
	if (argc > 1)   {			/*  check for startup file  */
		strcpy(initfile, argv[1]);
	}
	else   {
		if (ignore_homefile != TRUE)   {
			/*  check for default file in users home directory  */
			strcpy(initfile, getenv("HOME"));
			if (*initfile)   {
				length = strlen(initfile);
				strcpy(initfile + length, "/.");
				strcpy(initfile + length + 2, progname);
				if (access(initfile, F_OK) == -1)   {
					*initfile = '\0';
				}
			}
		}
		else   {
			*initfile = '\0';
		}
	}
	for (i = 3 ; i < NOFILE ; ++i)   {		/*  just to be sure  */
		close(i);
	}

	/*  get user tty characteristics  */
	ioctl(0, TIOCGETP, &masterterm);
	ioctl(0, TIOCLGET, &mlocalmode);
	ioctl(0, TIOCGETC, &mtchars);

	screen_init();		/*  set up curses and screen dimensions  */
	if (screencolumns < WORLD_MIN_COLUMNS  ||
					screenlines < WORLD_MIN_ROWS)   {
		fprintf(stderr, "%s: screen size too small !\r\n", progname);
		fprintf(stderr, "screen size must be at least");
		fprintf(stderr, " %d lines by %d columns\r\n",
					WORLD_MIN_ROWS, WORLD_MIN_COLUMNS);
		die();
	}

	initialise();			/*  initialise globals  */
	setup();			/*  initial windows  */
	startup = FALSE;
	manager();			/*  the window manager  */

	/*  should never get here  */
	fprintf(stderr, "\r\n%s: internal error (fallen off end), sorry !\r\n", 
								progname);
	die();
}



static
initialise()			/*  initialise globals  */

{
	int	**priority_win();
	int	fatal(), forced_die(), child_exit(), update_time();
	int	i, string_length;
	char	**virtwin();

	/*  set all windows to inactive status  */
	for (i = 0 ; i < MAX_WINDOWS_PLUS ; ++i)   {
		win[i].active = FALSE;
		strcpy(win[i].pseudo_ttyname, "unknown");
	}

	/*  work out where time will go  */
	string_length = sizeof(timestring) - 1;	/*  -1 for null at end  */
	time_start = (screencolumns - string_length) / 2;
	time_end = time_start + string_length - 1;

	screen_priority = priority_win();	/*  to decide display areas  */
	/*  slight liberty  */
	masterscreen = stdscr->_y;		/*  from curses package  */

	/*******************
	*   This window is always the lowest priority one.
	*   It cannot be accessed by the user and therefore provides
	*   a background for the portions of the screen that are not used.
	*******************/

	win[MAX_WINDOWS].y_start = 1;
	win[MAX_WINDOWS].x_start = 1;
	win[MAX_WINDOWS].y_end = screenlines - 1;
	win[MAX_WINDOWS].x_end = screencolumns - 1;
	win[MAX_WINDOWS].screenptr = virtwin('\0');
	win_initialise(MAX_WINDOWS, BACKGROUND);
	for (i = 0 ; i < sizeof(nottimestring) - 1 ; ++i)   {
		nottimestring[i] = BACKGROUND;
	}

	signal(SIGALRM, update_time);
	update_time();
	if (timeon == TRUE)   {		/*  display time first time round  */
		display_time(DISPLAY);
	}
	top_dog(MAX_WINDOWS);		/*  install background window  */

	/*  stop a core dump - playing safe as leaves tty in previous state  */

	signal(SIGILL, fatal);
	signal(SIGTRAP, fatal);
	signal(SIGIOT, fatal);
	signal(SIGEMT, fatal);
	signal(SIGSEGV, fatal);
	signal(SIGBUS, fatal);
	signal(SIGSYS, fatal);

	/*  signals to be used to stop program externally  */

	signal(SIGHUP, forced_die);
	signal(SIGINT, forced_die);
	signal(SIGQUIT, forced_die);
	signal(SIGTERM, forced_die);

	/*  in order to pick up dead children (when a window is removed)  */
	signal(SIGCHLD, child_exit);
}



static
setup()					/*  initial windows  */

{
	FILE	*fp, *fopen();
	int	columns, rows, xstart, ystart;
	int	i, limit, items_read, winno;
	char	*getenv(), *rindex(), *fgets();
	char	*program, buffer[BUFSIZ + 1], buffer2[BUFSIZ + 1];

	/*  find users favourite shell  */
	if ((home_shell = getenv("SHELL")) == NULL)   {
		home_shell =  default_shell;
	}
	/*  check for file to define startup windows  */
	if (*initfile)   {
		if ((fp = fopen(initfile, "r")) == NULL)   {
			fprintf(stderr, "%s: can't open %s\r\n",
							progname, initfile);
			die();
		}
		limit = MAX_WINDOWS;
	}
	else   {		/*  default windows  */
		fp = NULL;
		limit = 2;
	}

	winno = -2;		/*  invalid value to enable test at end  */
	for (i = 0 ; i < limit ; ++i)   {	/*  create each window  */
		program = buffer2;
		if (fp != NULL)   {
			if (fgets(buffer, BUFSIZ, fp) == NULL)   {
				break;
			}
			items_read = sscanf(buffer, "%d%d%d%d%[^\n\0]",
				&columns, &rows, &xstart, &ystart, program);
			if (items_read < 4)   {
				winno = -2;
				break;
			}
			check_size(&columns, &rows, &xstart, &ystart);
			if (items_read < 5)   {
				strcpy(program, home_shell);
			}
			else   {
				while (*program == ' ' || *program == '\t')   {
					++program;
				}
				/*  if just white space default to shell  */
				if (! *program)   {
					strcpy(program, home_shell);
				}
			}
		}
		else   {		/*  default window sizes  */
			columns = -1;
			rows = -1;
			xstart = -1;
			ystart = -1;
			check_size(&columns, &rows, &xstart, &ystart);
			strcpy(program, home_shell);
		}

		winno = win_instance(rows, columns, ystart, xstart, program);
		if (winno != -1)   {
			next_input_window(winno);
		}
	}
	if (fp != NULL)   {
		fclose(fp);
	}
	if (active_windows <= 0)   {
		if (winno == -2)   {
			fprintf(stderr, "%s: format of %s is invalid\r\n",
							progname, initfile);
		}
		else   {
			fprintf(stderr, "%s: can't open any windows\r\n",
								progname);
		}
		die();
	}
}



/*******************
*   Check (and change if necessary), that window dimensions selected can
*   be displayed on output device screen - if not then firstly move start
*   positions and then change dimensions if this fails.
*
*   Special negative value meanings:
*
*	In size field:
*		-1   -   full screen width or height depending on position
*		-2   -   half full screen width or height depending on position
*
*	In position field:
*		-1   -   screen width or height minus specified width or height
*		-2   -   half full width or height start positiono
*
*******************/

check_size(columns, rows, xstart, ystart)

int	*columns, *rows, *xstart, *ystart;

{
	if (*columns == -1  ||  *columns > screencolumns)   {
		*columns = screencolumns;
	}
	else   {
		if (*columns == -2)   {		/*  half width  */
			*columns = screencolumns / 2;
		}
		else   {
			if (*columns < MIN_COLUMNS)   {	/*  too small  */
				*columns = MIN_COLUMNS;
			}
		}
	}
	if (*rows == -1  ||  *rows > screenlines)   {
		*rows = screenlines;
	}
	else   {
		if (*rows == -2)   {		/*  half heigth  */
			*rows = screenlines / 2;
		}
		else   {
			if (*rows < MIN_ROWS)   {	/*  too small  */
				*rows = MIN_ROWS;
			}
		}
	}
	if (*xstart == -1)   {			/*  pad to fit  */
		*xstart = screencolumns - *columns;
	}
	else   {
		if (*xstart == -2)   {		/*  half width  */
			*xstart = screencolumns /2 ;
		}
		if (*xstart + *columns > screencolumns)   {
			*xstart = screencolumns - *columns;
		}
	}
	if (*ystart == -1)   {		/*  pad to fit  */
		*ystart = screenlines - *rows;
	}
	else   {
		if (*ystart == -2)   {		/*  half height  */
			*ystart = screenlines /2 ;
		}
		if (*ystart + *rows > screenlines)   {
			*ystart = screenlines - *rows;
		}
	}
}


static
screen_init()		/*  initialise curses package and screen size  */

{
	initscr();
	screencolumns = COLS;
	screenlines = LINES;
	raw();
	nonl();
	noecho();
}



/*******************
*   Save current screen in temporary window to enable non-window
*   user interaction, such as a short window command summary
*   and window status information to occur.  Window is replaced
*   at end of interaction.
*******************/

inform(function, winno)

int	(*function)();
int	winno;

{
	WINDOW		*helpwin, *newwin();
	int		old_timeon, waiting;
	char		c;

	/*******************
	*   Need to stop time updating screen since we are
	*   temporarily not using stdscr.
	*******************/

	alarm(0);			/*  turn off alarm  */
	old_timeon = timeon;
	if (timeon == TRUE)   {		/*  remove time  */
		timeon = FALSE;
		display_time(REMOVE);
	}

	helpwin = newwin(screenlines, screencolumns, 0, 0);
	overwrite(stdscr, helpwin);
	wclear(stdscr);
	wrefresh(stdscr);
	/*  call supplied function  */
	if ((waiting = ((*function)(winno))) == TRUE)   {
		printf("[ Hit return to continue ] ");
		fflush(stdout);
		read(0, &c, 1);
	}
	overwrite(helpwin, stdscr);
	overwrite(helpwin, curscr);
	/*  don't know where cursor is, so force whole screen redraw  */
	wrefresh(curscr);
	delwin(helpwin);
	if (old_timeon == TRUE)   {
		timeon = TRUE;
		update_time();		/*  display time this time round  */
	}
}



man_info()

{
	if (access(manfile, R_OK) != -1)   {
		sprintf(initfile, "%s %s", system_more, manfile);
		system(initfile);
	}
	else   {
		fprintf(stderr, "%s: sorry, can't find manual page %s\n",
							progname, manfile);
	}
}



help_info(winno)		/*  print help info by pages  */

int	winno;

{
	FILE	*fopen(), *fp, *out;

	if ((fp = fopen(helpfile, "r")) != NULL)   {
		page(fp);
		fclose(fp);
	}
	else   {
		if (startup == FALSE)   {
			out = stdout;
		}
		else   {		/*  grotwin -h invocation  */
			out = stderr;
			fprintf(out, "%s: ", progname);
		}
		fprintf(out, "sorry, helpfile %s not available\r\n", helpfile);
	}
	return(TRUE);
}



page(fp)

FILE	*fp;

{
	static char	page_string[] = "[ Hit any key for more ] ";

	int	lines;
	char	*fgets(), buffer[80];

	lines = 1;
	while (fgets(buffer, sizeof(buffer), fp) != NULL)   {
		fputs(buffer, stdout);
		putc('\r', stdout);
		if (++lines >= screenlines)   {
			lines = 1;
			fputs(page_string, stdout);
			fflush(stdout);
			read(0, buffer, 1);	/*  wait for user  */
			clear_line(sizeof(page_string));
		}
	}
}



clear_line(length)		/*  clear a line on screen  */

int length;

{
	int	outchar();

	putc('\r', stdout);

	/*******************
	*   Clear line.  Try to use termcap delete to end of line,
	*   delete line or just output plain spaces if all else fails.
	*   Delete to end of line may be faster than delete line
	*   since delete line may require the terminal to
	*   scroll the bottom line.
	*******************/

	if (CE != (char *) NULL)   {
		tputs(CE, 1, outchar);
	}
	else   {
		if (DL != (char *) NULL)   {
			tputs(CE, 1, outchar);
		}
		else   {				/*  bodge it  */
			while (length--)   {
				putc(' ', stdout);
			}
		}
	}
	putc('\r', stdout);
	fflush(stdout);
}



static
outchar(c)		/*  output routine for use with tputs in clear_line  */

char c;

{
	putc(c, stdout);
}



forced_die()

{
	alarm(0);
	signal(SIGINT, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	kill_all_shells();
}



die()

{
	static char	confused_mess[] =
			"\7\r\n--  Internal error - please report  --\r\n";

	alarm(0);
	if (startup == FALSE)   {
		wclear(stdscr);
		wmove(stdscr, LINES - 1, 0);
		wrefresh(stdscr);
	}
	endwin();
	if (screen_priority != NULL)   {
		free_priority_win(screen_priority);
	}
	if (confused == REALLY_CONFUSED)   {
		fprintf(stderr, confused_mess);
		exit(3);
	}
	exit(0);
}



static
fatal()

{
	static char	fatalmess[] =
				"\n\n\7  caught nasty signal - exiting  \n";
	static int	first_time = TRUE;

	alarm(0);
	signal(SIGINT, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	/*  endwin will do this anyway, good for following printfs though  */
	ioctl(0, TIOCSETP, &masterterm);
	ioctl(0, TIOCLBIS, &mlocalmode);
	ioctl(0, TIOCSETC, &mtchars);
	fprintf(stderr, fatalmess);
	/*  safety check to stop endless recursion if hopelessly lost  */
	if (first_time)   {
		first_time = FALSE;
		forced_die();
		die();
	}
	exit(4);
}
