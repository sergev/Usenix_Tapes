#ifndef lint
static char sccsid[] = "@(#)manager.c  2.2  [ (C) Nigel Holder 1986 ]";
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
#include <sys/file.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include "grotwin.h"



extern int	errno;			/*  system error  */

/*  externals defined in grotwin.c  */

extern struct windowdetails	win[MAX_WINDOWS_PLUS];	/*  window details  */

extern struct sgttyb	masterterm;	/*  used to store initial tty state  */
extern struct tchars	mtchars;
extern int		mlocalmode;

extern int	startup;		/*  program startup period  */
extern int	confused;		/*  for when program hangs up  */
extern int	timeon;			/*  display clock on top edge  */
extern int	time_start;		/*  where time string is placed  */

extern char	*progname;		/*  program name stripped  of any /'s*/
extern char	timestring[];		/*  time on top edge of b/g window  */
extern char	nottimestring[];	/*  used when time is not displayed  */


unsigned int	child_died = 0;		/*  flag to catch SIGCHLD  */

/*  slavemask used to form mask of active file descriptors for select()  */
int	slavemask = 1 << 0;		/*  standard input (fd = 0)  */
int	active_windows = 0;		/*  number of user windows active  */
int	input_winno = 0;		/*  current window  */
int	input_changed = FALSE;		/*  to force cursor to be redrawn  */
int	alldead = FALSE;		/*  no user window active  */
int	exit_forced = FALSE;		/*  forced exit of program  */

long	clock;				/*  time in seconds of last minute  */

/*  list of priorities - end referenced by tail variable  */
int	window_priorities[MAX_WINDOWS_PLUS];




manager()	/*  manage input from active window and output from all  */

{
	/*******************
	*   Nice largeish timeout to make select() return only when something
	*   is ready (stops eating up cpu time).
	*   Must be less than 30 seconds for detection of hangup.
	*******************/

	static struct timeval	timeout = { 10L, 0L };
	static unsigned int	children_dead = 0;

	struct windowdetails	*ptr;
	int	 i, readfds, waiting;

	/*  make corners indicate indicate this input window  */
	top_corners(input_winno);

	while (1)   {
		/*  used to locate internal hangup  */
		if (confused == TRUE)   {
			confused = FALSE;
			if (timeon == TRUE)   {		/*  time changed  */
				display_time(DISPLAY);
			}
		}

		/*******************
		*   Check for dead children.  Even though child_died
		*   may be incremented asynchronously, having another
		*   count that is incremented until it is equal to
		*   child_died will overcome any problems of reading
		*   an out of date value of child_died being used.
		*******************/
   
		while (children_dead != child_died)   {
			++children_dead;
			check_exit();		/*  remove window  */
		}
		/*  place cursor in active window if moved  */
		if (input_changed == TRUE)   {
			ptr = &(win[input_winno]);
			wmove(stdscr, ptr->y_current + ptr->y_start,
				ptr->x_current + ptr->x_start);
			touchwin(stdscr);
			wrefresh(stdscr);
			input_changed = FALSE;
		}

		/*******************
		*  Check if their is any keyboard input and obey - then
		*  check each window for any output and display it .
		*  Order is important since user expects response to input
		*  immediately (this does mean that window tty could be closed
		*  after select has said there is input waiting though !).
		*******************/

		readfds = slavemask;
		if (select(NOFILE, &readfds, 0, 0, &timeout) <= 0)   {
			continue;
		}

		/*  keyboard input waiting  */
		if (readfds & (1 << 0))   {
			get_input();
		}
		touchwin(stdscr);
		wrefresh(stdscr);
		ptr = win;

		/*******************
		*   need to check if window is active since it may
		*   have been removed by action of keyboard input !
		*******************/

		for (i = 0 ; i < MAX_WINDOWS ; ++i, ++ptr)   {
			if (! ptr->active)   {
				continue;
			}
			/*  check for previous output still waiting  */
			if (ptr->page_buf_length > 0)   {
				update_screen(i);
				input_changed = TRUE;
				continue;
			}
			else   {	/*  read current output waiting  */
				if (readfds & ptr->slavemaskfd)   {
					waiting = read(ptr->masterfd,
							ptr->page_buf,
							ptr->readgrain);
					if (waiting != -1)   {
						ptr->page_buf[waiting] = '\0';
						ptr->page_buf_length = waiting;
						update_screen(i);
						input_changed = TRUE;
					}
				}
			}
		}
	}
}



/*  fire off window of specified dimensions running named program within it  */

win_instance(lines, columns, ystart, xstart, program)

int	lines, columns, ystart, xstart;
char	*program;

{
	struct windowdetails	*ptr;
	int	winno;
	char	*calloc();

	if ((winno = freewin()) == -1)   {
		return(-1);
	}
	if (newtty(winno) == -1)   {
		return(-1);
	}
	ptr = &(win[winno]);
	ptr->progy = calloc(strlen(program) + 1, sizeof(char));
	strcpy(ptr->progy, program);

	switch (ptr->slavepid = fork())   {
		case  -1  :		/*  too many system processes !  */
			/*  can't use removewin since haven't used createwin  */
			close(ptr->masterfd);
			close(ptr->slavefd);
			ptr->active = FALSE;
			return(-1);
			break;

		case  0  :		/*  child  */
			action_slave(ptr->slavefd, ptr->progy,
							lines - 2, columns - 2);
			/*  _exit to stop flushing buffers twice (apparently) */
			_exit(10);
			break;

		default  :		/*  parent  */
			/*****************
			*   In case any old (still running) processes started
			*   on pty have not been disassociated from it.
			*****************/

			ioctl(ptr->masterfd, TIOCSPGRP, &(ptr->slavepid));
			ioctl(ptr->slavefd, TIOCSPGRP, &(ptr->slavepid));
			/*  rendezvous with child (see comment in child)  */
			write(ptr->masterfd, "\n", 1);
			break;
	}
	/*  must be after fork (faster startup with createwin here)  */
	slavemask |= ptr->slavemaskfd;		/*  tty now accessible  */
	createwin(winno, lines, columns, ystart, xstart);
	return(winno);
}



static
freewin()			/*  return win[] location not in use  */

{
	register int	i;
	register struct windowdetails	*ptr;

	for (i = 0, ptr = win ; i < MAX_WINDOWS ; ++i, ++ptr)  {
		if (! ptr->active)   {
			return(i);
		}
	}
	return(-1);
}



static
newtty(winno)		/*  return number of a free pseudo tty (cyclic ish)  */

int	winno;

{
	/*  not very efficient for pty[q-r] at present, to be changed  */

	static char	*pseudo_termlist[] = {
				"0", "1", "2", "3", "4", "5", "6", "7",
				"8", "9", "a", "b", "c", "d", "e", "f"
	};
	static char	*masterside[] = {
				"/dev/ptyp", "/dev/ptyq", "/dev/ptyr"
	};
	static char	*slaveside[] = {
				"/dev/ttyp", "/dev/ttyq", "/dev/ttyr"
	};
	static int	elements = sizeof(pseudo_termlist) / sizeof(char *);
	static int	i = (sizeof(pseudo_termlist) / sizeof(char *)) - 1;
	static int	pty_groups = sizeof(masterside) / sizeof(char *);
	static int	j = (sizeof(masterside) / sizeof(char *)) - 1;

	register struct windowdetails	*ptr;
	char	temp[20];
	int	pty, tty_element, fd;

	ptr = &(win[winno]);
	/*  if pseudo tty in use, assume can't open either master or slave  */
	for (pty = 0 ; pty < pty_groups ; ++pty)   {
		for (tty_element = 0 ; tty_element < elements ; ++tty_element) {
			i = (i + 1) % elements;
			sprintf(temp, "%s%s", masterside[j],pseudo_termlist[i]);
			if ((fd = open(temp, O_RDWR)) != -1)   {
				ptr->masterfd = fd;
				sprintf(temp, "%s%s", slaveside[j],
							pseudo_termlist[i]);
				if ((fd = open(temp, O_RDWR)) != -1)   {
					ptr->slavemaskfd = 1 << ptr->masterfd;
					ptr->slavefd = fd;
					strcpy(ptr->pseudo_ttyname, temp);
					return(0);
				}
				close(ptr->masterfd);
			}
		}
		/*  start at the beginning for next pty group  */
		if (i != 0)   {
			i = elements - 1;
		}
		j = (j + 1) % pty_groups;
	}
	return(-1);
}



child_exit()		/*  increment global flag of dead children  */

{
	++child_died;
}



check_exit()				/*  check for terminated shells  */

{
	register int	i;
	register struct windowdetails	*ptr;
	int pid;

	for (i = 0, ptr = win ; i < MAX_WINDOWS ; ++i, ++ptr)   {
		if (ptr->active)   {
			if (kill(ptr->slavepid, 0) == -1 && errno == ESRCH)   {
				/*******************
				*   Should really check that pid is equal
				*   to ptr->slaveppid, but what could
				*   be done if it wasn't.
				*******************/
				pid = shell_cleanup(i);
			}
		}
	}

	/*  if no shells left - exit program  */
	if (alldead  &&  ! startup)   {
		alarm(0);
		signal(SIGALRM, SIG_IGN);
		signal(SIGCHLD, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		die();
	}
}



update_time()		/*  called via SIGALRM - update global time  */

{
	static long	last_clock = 0L;
	char	*ctime();

	time(&clock);

	/*******************
	*   Following is a simple little check to ensure that if program
	*   will terminate within a minute if it hangs up badly.
	*   Relies on manager taking less than 30 seconds on select timeout !
	*******************/

	if (clock - last_clock > 30)   {	/*  internal hangup  */
		if (confused == TRUE  &&  startup == FALSE)   {
			confused = REALLY_CONFUSED;
			forced_die();
		}
		last_clock = clock;
	}
	alarm(60 - (int) (clock % 60));
	confused = TRUE;			/*  until proven otherwise  */
}



display_time(command)		/*  display or remove time on top border  */

int	command;

{
	wmove(stdscr, 0, time_start);
	if (command == DISPLAY)   {
		/*  isolate hours and mins  */
		strncpy(timestring + 1, ctime(&clock) + 11, 5);
		waddstr(stdscr, timestring);
	}
	else   {	/*  REMOVE  */
		waddstr(stdscr, nottimestring);
	}
	input_changed = TRUE;	/*  place cursor back in input window  */
}



static
shell_cleanup(winno)		/* remove all references to a now dead window */

int	winno;

{
	union wait	status;			/*  different from sysV  */
	int pid;

	/*******************
	*   Stop child waiting for parent to perform a wait.
	*   Otherwise child becomes zombie and hogs a place in process
	*   list table.
	*******************/
	pid = wait(&status);
	removewin(winno);
	/*  place cursor in current window  */
	if (! alldead)  {
		/*  was current window one terminated  */
		if (winno == input_winno)   {
			next_input_window(window_priorities[0]);
			top_dog(input_winno);
		}
	}
	return(pid);
}


kill_all_shells()

{
	int	i;

	exit_forced = TRUE;
	for (i = 0 ; i < MAX_WINDOWS ; ++i)   {
		if (win[i].active)   {
			if (kill_shell(i) == -1)   {
				printf(" window %d process wouldn't die ", i);
				sleep(2);
			}
			removewin(i);
		}
	}
	die();
}



/*  kill process within window - tries gently and then with brute force  */

static
kill_shell(winno)

int	winno;

{
	static int	kill_list[] = { SIGHUP, SIGINT, SIGTERM, SIGKILL };
	int		i;

	kill(win[winno].slavepid, SIGCONT);	/*  wake up just in case  */
	for (i = 0 ; i < (sizeof(kill_list) / sizeof(int)) ; ++i)   {
		if (kill(win[winno].slavepid, kill_list[i]) == 0)   {
			return(kill_list[i]);
		}
	}
	return(-1);
}



static
removewin(winno)			/*  remove window from existance  */

int	winno;

{
	struct windowdetails	*ptr;
	char	*rindex(), *tty;

	ptr = &(win[winno]);
	ptr->active = FALSE;
	slavemask &= ~ ptr->slavemaskfd;
	if (--active_windows <= 0)   {
		alldead = TRUE;
	}
	close(ptr->masterfd);
	close(ptr->slavefd);
	tty = 1 + rindex(ptr->pseudo_ttyname, '/');
	utmp_delete(tty);
	free_virt_win(ptr->screenptr);
	free(ptr->progy);
	/*  test to stop needless screen redrawing if quiting program  */
	if (! exit_forced)   {		/*  clear screen where window was  */
		bottom_dog(winno, REMOVE);
		touchwin(stdscr);
		wrefresh(stdscr);
	}
}



next_input_window(value)

int	value;

{
	static int	prev_winno = MAX_WINDOWS - 1;
	register struct windowdetails	*ptr;
	int		i, oldinput;

	oldinput = input_winno;
	if (value == -1)   {
		for (i = 0 ; i < MAX_WINDOWS ; ++i)  {
			prev_winno = (prev_winno + 1) % MAX_WINDOWS;
			if (win[prev_winno].active)   {
				input_winno = prev_winno;
				break;
			}
		}
	}
	else   {
		/*  check for illegal window number or inactive window  */
		if (value >= 0  &&  value < MAX_WINDOWS)   {
			if (win[value].active)   {
				input_winno = value;
				prev_winno = value;
			}
			else   {
				return(-1);
			}
		}
		else   {
			return(-1);
		}
	}

	/*  remove active indicators in top corners of old acitive window  */
	if (oldinput >= 0)   {
		ptr = &(win[oldinput]);
		if (ptr->active)   {
			ptr->screenptr[ptr->y_start - 1][ptr->x_start - 1] =' ';
			ptr->screenptr[ptr->y_start - 1][ptr->x_end] = ' ';
			top_corners(oldinput);
		}
	}
	/*  add active indicators to top corners of active window  */
	ptr = &(win[input_winno]);
	ptr->screenptr[ptr->y_start - 1][ptr->x_start - 1] = '+';
	ptr->screenptr[ptr->y_start - 1][ptr->x_end] = '+';
	top_corners(input_winno);

	input_changed = TRUE;

	/*  bodge for now to force corners to be indicated !  */
	/*  should go before ptr = ...  */
 	if (oldinput == input_winno)   {
		return(-1);
	}
	return(oldinput);
}



static
get_input()

{
	int	length, i;
	char	input[BUFSIZ + 1];

	length = read(0, input, BUFSIZ);
	for (i = 0 ; i < length ; ++i)   {
		put_char((unsigned char) input[i]);
	}
}



static
put_char(c)

unsigned char	c;

{
	extern int	window_info(), help_info();

	static unsigned int	prev_c = '\0';
	static int	new_winno = 0;	/*  in case c == WIN_SWITCH_NUMBER  */

	struct windowdetails	*ptr;
	int	upper_case, temp;
	unsigned char	c_orig;

	c &=  0x7F;		/*  strip off top bit for WIN_SELECT_NUMBER  */
	ptr = &(win[input_winno]);
	if (prev_c == SELECT)   {
		c_orig = c;
		/*******************
		*   to help distinguish between upper case only commands,
		*   make upper_case variable FALSE in order to execlude
		*  lower case and control instances
		*******************/
		upper_case = FALSE;		/*  until proved otherwise  */
		if (isdigit(c))   {
			new_winno = c - '0';
			if (new_winno < MAX_WINDOWS)   {
				c = WIN_SWITCH_NUMBER;
			}
		}
		else   {
			if (isalpha(c))   {
				if (isupper(c))   {
					upper_case = TRUE;
				}
				/*  to allow 'n' and CTRL-N to be the same  */
				c = c & CTRL_MASK;
			}
		}
		switch(c)   {
			case  WIN_SWITCH  :
				new_winno = -1;
			case  WIN_SWITCH_NUMBER  :
				temp = input_winno;
				if (next_input_window(new_winno) == -1  ||
						temp == input_winno)   {
					write(1, "\007", 1);
					break;		/*  nasty practice  */
				}
				top_dog(input_winno);
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

			case  NEW_WINDOW  :
				new_window();
				break;

			case  SCREEN_REDRAW  :
				touchwin(curscr);
				wrefresh(curscr);
				break;

			case  EXPAND  :
				widen_window(input_winno, VERTICAL_EXPAND);
				widen_window(input_winno, HORIZONTAL_EXPAND);
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

			case  VERTICAL_EXPAND  :
				widen_window(input_winno, VERTICAL_EXPAND);
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

			case  HORIZONTAL_EXPAND  :
				widen_window(input_winno, HORIZONTAL_EXPAND);
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

			case  PAGE_MODE  :
				if (upper_case)   {	/*  page off  */
					ptr->output_paged = FALSE;
					if (ptr->paged_page_full)   {
						ptr->paged_page_full = FALSE;
					}
				}
				else   {		/*  page on  */
					if (! ptr->output_paged)   {
						ptr->output_paged = TRUE;
						ptr->paged_page_full = FALSE;
						ptr->line_count = 0;
					}
				}
				break;

			case  REMOVE_WIN  :
				/*  only look for upper case   */
				if (upper_case)   {
					if (kill_shell(input_winno) == -1)   {
						/*  failed (why ?)  */
						write(1, "\007", 1);
					}
					break;
				}
				/*  fall through to default case (nasty)  */
				goto fallthru;

			case  FORCED_EXIT  :
				if (upper_case)   {
					forced_die();
					break;
				}
				/*  only look for upper case   */
				/*  fall through to default case (nasty)  */
				goto fallthru;

			case  XON  :
				ptr->output_blocked = FALSE;
				break;

			case  XOFF  :
				ptr->output_blocked = TRUE;
				break;

			case  OVERWRITE_MODE  :
				if (upper_case)   {	/*  overwrite off  */
					ptr->overwrite = FALSE;
				}
				else   {		/*  overwrite on  */
					ptr->overwrite = TRUE;
				}
				break;

			case  TIME_TOGGLE  :
				/*  toggle time between on and off  */
				if (timeon == TRUE)   {
					timeon = FALSE;
					display_time(REMOVE);
				}
				else   {
					timeon = TRUE;
					update_time();
				}
				break;

			case  INFORM  :
				inform(window_info, input_winno);
				break;

			case  HELP  :
				inform(help_info, input_winno);
				break;

			case  CLEAR  :
				mywclear(input_winno);
				input_changed = TRUE;
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

			case  EXPOSE_WINDOW  :
				top_dog(input_winno);
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

			case  HIDE_WINDOW  :
				bottom_dog(input_winno, HIDE);
				touchwin(stdscr);
				wrefresh(stdscr);
				break;

	fallthru :
			default  :
				if (ptr->paged_page_full)   {
					ptr->paged_page_full = FALSE;
				}
				else   {
					c = c_orig;
					write(ptr->masterfd, &c, 1);
					if (c == SELECT)   {
						c = ~c;
					}
				}
				break;
		}
	}
	else   {
		if (c != SELECT)   {
			if (ptr->paged_page_full)   {
				ptr->paged_page_full = FALSE;
			}
			else   {
				write(ptr->masterfd, &c, 1);
			}
		}
	}
	prev_c = c;
}



/*  fire off slave process within window  */

static
action_slave(slavefd, progy, lines, columns)

int	slavefd;
char	*progy;
int	lines, columns;

{
	extern char	**environ;
	extern char	termcap[];
	extern char	term[];		/*  my very own terminal type !  */

	int	fd, ldisc, pgrp, i, j, k;
	char	**argv, **envp, *calloc();
	char	c, buffer[BUFSIZ + 1], *ptr1, *ptr2;

	signal(SIGALRM, SIG_IGN);
	alarm(0);		/*  turn off clock signal  */

	/*  reset all caught signals  */
	for (i = 1 ; i <= NSIG ; ++i)   {
		signal(i, SIG_DFL);
	}

	/*  make pseudo tty connection  */
	close(0);
	dup(slavefd);
	close(1);
	dup(slavefd);
	close(2);
	dup(slavefd);
	for (i = 3 ; i < NOFILE ; ++i)   {	/*  close all other files  */
		close(i);
	}
	/*  disassociate /dev/tty from parent control group  */
	if ((fd = open("/dev/tty", O_RDWR, 0)) != -1)   {
		ioctl(fd, TIOCNOTTY, 0);
		close(fd);
	}

	/*******************
	*      From experience, it would appear that the master side of a
	*   pseudo tty must set the process group for the master side (pty)
	*   before the slave side sets its process group equal to its
	*   own pid (master is also set to this value, obtained via fork
	*   of child).
	*      This is achieved by the parent writing '\n' (specifically '\n'
	*   to overcome buffering), on its tty when it has set its
	*   own tty and its childs tty (action duplicated by child) to the
	*   process group equal to the childs process number, thereby
	*   achieving a simple rendezvous.
	*******************/

	/*  rendezvous with parent  */
	read(0, &c, 1);

	/*  set up tty in a usable state as parent tty may not be  */
	ldisc = NTTYDISC;
	masterterm.sg_flags |= ECHO;
	masterterm.sg_flags |= CRMOD;
	masterterm.sg_flags &= ~CBREAK;
	masterterm.sg_flags &= ~RAW;
	ioctl(0, TIOCSETP, &masterterm);
	ioctl(0, TIOCSETD, &ldisc);
	ioctl(0, TIOCLBIS, &mlocalmode);
	ioctl(0, TIOCSETC, &mtchars);

	/*  set up unique process group and attach to process and tty  */
	pgrp = getpid();
	setpgrp(pgrp, pgrp);
	ioctl(0, TIOCSPGRP, &pgrp);	/*  parent also performs this  */
	set_tty_size(0, lines, columns, FALSE);		/*  set tty size  */

	for (i = 0 ; environ[i] ; ++i)   {	/*  find last entry  */
		;
	}
	/*  form new environment for progy  */
	envp = (char **) calloc(i + 2, sizeof(char *));
	/*  comb through and remove TERM and TERMCAP entries  */
	for (k = 0, j = 0 ; j < i ; ++j)   {
		if (strncmp(environ[j], "TERMCAP", 7) != 0  &&
				strncmp(environ[j], "TERM", 4) != 0)   {
			envp[k++] = environ[j];
		}
	}

	/*******************
	*   Now add grotwin terminal type.
	*   Could be clever and place these at start of environment
	*   for quick access !
	*******************/

	envp[k] = calloc(strlen(term) + 1, sizeof(char));
	strcpy(envp[k++], term);
	sprintf(buffer, termcap, columns, lines);
	envp[k] = calloc(strlen(buffer) + 1, sizeof(char));
	strcpy(envp[k], buffer);
	envp[++k] = (char *) NULL;

	environ = envp;
	argv = (char **) calloc(MAXARGS + 1, sizeof(char *));

	strcpy(buffer, progy);
	i = 0;
	ptr1 = buffer;
	while (i < MAXARGS)   {
		for(ptr2 = ptr1 ; *ptr1 != ' '  &&  *ptr1 ; ++ptr1)   {
			;
		}
		argv[i++] = ptr2;
		if (! *ptr1)   {
			break;
		}
		*ptr1++ = '\0';
	}
	argv[i] = (char *) NULL;
	execvp(argv[0], argv);

	/*  exec failed  */
	fprintf(stderr, "\r\n%s: can't find %s  ", progname, argv[0]);
	fflush(stderr);
	free(argv);
	free(envp[k - 1]);
	free(envp[k - 2]);
	free(envp);
	close(0);	/*  close files  */
	close(1);
	close(2);
	sleep(5);
}



/*******************
*   Set tty size and inform associated control group
*   via SIGWINCH that the tty size has changed.
*******************/

set_tty_size(fd, lines, columns, inform)

int fd, lines, columns, inform;

{
	struct ttysize	tty_size;
	int	pgrp;

	tty_size.ts_lines = lines;		/*  set up new tty size  */
	tty_size.ts_cols = columns;
	if (ioctl(fd, TIOCSSIZE, &tty_size) == -1)   {
		return;			/*  can't change tty size  */
	}
	if (inform == TRUE)   {		/*  inform process  */
		if (ioctl(fd, TIOCGPGRP, &pgrp) == 0)   {
			killpg(pgrp, SIGWINCH);
		}
	}
}
