#ifndef	lint
static char sccsid[] = "@(#)talk.c  1.2 [Nigel Holder (C) -  Baddow] 04/12/85";
#endif


/***************************************
*
*	Author   :  Nigel Holder
*
*	Date     :  12 June 1985
*		     4 December 1985		changed elapsed time stuff
*						to be synchronous to windows
*
*
*	Copyright (C) 1986  by Nigel Holder
*
*	   Permission to use this program is granted, provided it is not
*	sold, or distributed for direct commercial advantage, and includes
*	the copyright notice and this clause.
*
*
*	   Talk - an interactive communication program that allows users
*	to talk on a character basis (as opposed to a line basis, as for
*	the system write utility).
*
*	   Written for System V as it uses named pipes !
*	   (and BSD already has a talk utility).
*
*	Bugs:
*
*	1.   Not as good as BSD talk, but it suffices.
*	    (restricted to current host machine)
*
*	2.   Really need select() type statement (BSD) instead of sleeping
*	     for 1 second between no input or output activity.
*	     (version 8 should fix this).
*
*	3.   Should check for name fields in dividewin overflowing screenwidth
*
*	4.   Probably should disable CTRL-c stopping program when connected.
*
***************************************/


#include <curses.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <pwd.h>
#include <utmp.h>
#include <sys/dir.h>


#define		FIFO			( 0010660 )
#define		EXIST			( 00 )
#define         INWINLINES              ( LINES / 2 )
#define         OUTWINLINES             ( LINES - INWINLINES - 1 )
#define		CONNECT			( 0xF0 )
#define		DISCONNECT		( 0xF3 )
#define		DELETE			( 0xFC )
#define		END_OF_FILE		( 0x04 )
#define		REFRESH			( 0x0C )
#define		SPACE			( 0x20 )
#define		BELL			( 0x07 )
#define		CLOCK_TEMP		( sizeof(elapsed) )
#define		CLOCK_TICK		( 15 )
#define		RETRIES			( 3 )
#define		TRIES			( RETRIES + 1 )
#define		WAIT_TIME		( 20 )
#define		DIRPREFIX		( sizeof(tempdir) + 20 )

#define		not_printable(x)	\
				(x != '\n'  &&  x != '\t'  &&  isprint(x) == 0) 

/*  --  globals  --  */

FILE	*writing = NULL;			/*  pipe to write down  */
int	reading = -1;				/*  fd for keyboard  */
int	connected = FALSE;			/*  whether actually talking  */
int	failed = FALSE;				/*  couldn't connect  */
int	forced = FALSE;				/*  ctrl-c stopped program  */
int	time_changed = FALSE;			/*  to update elapsed time  */

WINDOW	*inwin, *outwin, *dividewin;		/*  the three windows  */
int	inwinx, inwiny;				/*  cursor pos in each window */
int	outwinx, outwiny;
int	divwiny, divwinx;

char	elapsed[] = "[%02d:%02d:%02d]  ";	/*  format of elapsed time  */
char	tempdir[] = "/tmp/";			/*  temp place for pipes  */
char	wprefix[] = "tw_";			/*  prefix for pipes  */
char	rprefix[] = "tr_";
char	writefile[DIRSIZ + DIRPREFIX];		/*  temp pipe filenames  */
char	readfile[DIRSIZ + DIRPREFIX];
char	tmp[BUFSIZ];				/*  general purpose !  */
char	*myname, *theirname, *theirtty;
char	*progname;

int	master;					/*  whether master or slave   */
int	delchar;				/*  favourite delete char  */




main(argc, argv)

int	argc;
char	*argv[];

{
	int	forced_die();
	char	*strrchr(), *getlogin();

	if ((progname = strrchr(argv[0], '/')) != NULL)   {
		++progname;
	}
	else   {
		progname = argv[0];
	}
	if (argc < 2)   {
		fprintf(stderr, "usage: %s user [tty]\n", progname);
		exit(1);
	}
	theirname = argv[1];
	if (argc > 2)   {
		theirtty = argv[2];
	}
	else   {
		theirtty = "";
	}
	if ((myname = getlogin()) == NULL)   {
		fprintf(stderr, "You don't exist. Go away.\n");
		exit(2);
	}
	signal(SIGINT, SIG_IGN);		/*  play safe  */
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	screen_init();				/*  set up windows  */
	signal(SIGINT, forced_die);		/*  gracefully trap signals  */
	signal(SIGHUP, forced_die);
	set_up_channel();			/*  establish named pipes  */
	talk();			/*  let your fingers do the walking !  */
	die();					/*  should never get here !  */
	exit(3);
}



set_up_channel()		/*  establish connection between users  */

{
	char	*nameof();
	char	clock_temp[CLOCK_TEMP];

	sprintf(readfile, "%s%s%s%s", tempdir, rprefix, myname, theirname);
	sprintf(writefile, "%s%s%s%s", tempdir, wprefix, myname, theirname);

	/*  check to see if any old connections lying around !  */
	/*  they could be caused (left around) by system crashes etc.  */
	if (( ! access(readfile, EXIST))  &&  out_of_date(readfile))   {
		unlink(readfile);
	}
	if (( ! access(writefile, EXIST))  &&  out_of_date(writefile))   {
		unlink(writefile);
	}
	wheader(inwin, "[No connection yet]\n");
	sprintf(readfile, "%s%s%s%s", tempdir, wprefix, theirname, myname);
	if (access(readfile, EXIST))   {
		master = TRUE;
		master_channel();
		unlink(readfile);	/*  remove pipes (clever eh ?)  */
		unlink(writefile);
	}
	else   {
		master = FALSE;
		slave_channel();
	}
	connected = TRUE;
	wheader(inwin, "[Connected]\n");
	beep();
	sprintf(clock_temp, elapsed, 0, 0, 0);
	sprintf(tmp, "  %s is talking to %s (%s) ",
					myname, theirname, nameof(theirname));
        wmove(dividewin, 0, (COLS - strlen(tmp) - strlen(clock_temp)) / 2);
	wstandout(dividewin);
	waddstr(dividewin, tmp);
	getyx(dividewin, divwiny, divwinx);
	waddstr(dividewin, clock_temp);
	wstandend(dividewin);
        wrefresh(dividewin);
}



out_of_date(file_ptr)	/*  determine whether pipe file is not in use  */

char	*file_ptr;

{
	long	time();
	struct stat	stat_buf;

	if (stat(file_ptr, &stat_buf) == -1)   {   /*  assume doesn't exist  */
		return(FALSE);
	}
	if ((time((long *) 0) - stat_buf.st_mtime) > (TRIES * WAIT_TIME))   {
		return(TRUE);
	}
	return(FALSE);
}



/*  create named pipes for connection to slave and try to connect  */

master_channel()

{
	static char	*retry[] =   {
				"1st retry",
				"2nd retry",
				"3rd retry",
				"4th retry"
	};

	char	*dialup();
	int	retries, waiting;
	char	*retry_message, *dialtty, c;

	sprintf(readfile, "%s%s%s%s", tempdir, rprefix, myname, theirname);
	umask(0);
	if (mknod(readfile, FIFO) == -1)   {
		error("can't create %s", readfile);
	}
	if (mknod(writefile, FIFO) == -1)   {
		error("can't create %s", writefile);
	}
	open_channel();
	dialtty = dialup(CONNECT);
	sprintf(tmp, "[Waiting for your party to respond on %s]\n", dialtty);
	wheader(inwin, tmp);
	c = DISCONNECT;
	for (retries = 0 ; c != CONNECT  &&  retries <= RETRIES ; ++retries)   {
		for (waiting = 0 ; c != CONNECT  &&
				waiting < WAIT_TIME ; ++waiting)   {
			sleep(1);
			read(reading, &c, 1);
		}
		if (c != CONNECT  &&  retries < RETRIES)   {
			if (retries < (sizeof(retry) / sizeof(char *)))   {
				retry_message = retry[retries];
			}
			else   {
				retry_message = "Lost count !";
			}
			dialtty = dialup(CONNECT);
			sprintf(tmp, "[Ringing your party again on %s (%s)]\n",
							dialtty, retry_message);
			wheader(inwin, tmp);
		}
	}
	if (c != CONNECT)   {
		failed = TRUE;
		wheader(inwin, "[Your party would not respond]\n");
		die();
	}
}



slave_channel()		/*  form file names for slave connection  */

{
	sprintf(readfile, "%s%s%s%s", tempdir, wprefix, theirname, myname);
	sprintf(writefile, "%s%s%s%s", tempdir, rprefix, theirname, myname);
	open_channel();
	putc(CONNECT, writing);
}



open_channel()			/*  establish communications via pipes  */

{
	FILE	*fopen();

	if ((writing = fopen(writefile, "w+")) == NULL)   {
		error("can't open FIFO for writing");
	}
	setbuf(writing, (char *) 0);	/*  unbuffer stream  */
	if ((reading = open(readfile, O_NDELAY | O_RDONLY)) == -1)   {
		error("can't open FIFO for reading");
	}
}



wheader(win, message)		/*  print message at head of window  */

WINDOW	*win;
char	*message;

{
	wmove(win, 0, 0);
	wclrtoeol(win);
	waddstr(win, message);
	wrefresh(win);
}



screen_init()				/*  initialise talk windows  */

{
        int	i;

        initscr();
        inwin = newwin(INWINLINES, COLS, 0, 0);
        outwin = newwin(OUTWINLINES, COLS, INWINLINES + 1, 0);
        dividewin = newwin(1, COLS, INWINLINES, 0);
	noecho();
	nodelay(inwin, TRUE);
	cbreak();
        scrollok(inwin, TRUE);
        scrollok(outwin, TRUE);
	idlok(inwin, TRUE);
	idlok(outwin, TRUE);
        wmove(dividewin, 0, 0);
        for (i = 0 ; i < COLS ; ++i)   {
                waddch(dividewin, '-');
        }
        wnoutrefresh(inwin);	/* faster screen update (a la curses manual) */
        wnoutrefresh(outwin);
        wnoutrefresh(dividewin);
	doupdate();
}



/*  attempt to notify user that you wish to talk to him, or have given up  */

char *
dialup(status)

int	status;

{
	struct utmp	*ptr, *user, *getutent();
	void	setutent();
	long	tloc, time();
	char	*nameof(), ctime();
	FILE	*fp;
	int	found, logins;
	char	current_time[26 + 1];

	found = FALSE;
	logins = 0;
	setutent();
	while ((ptr = getutent()) != NULL)   {		/*  search for user  */
		if (strcmp(ptr->ut_user, theirname) == 0)   {
			if ( ! *theirtty)   {
				user = ptr;
				found = TRUE;
				break;
			}
			else   {
				if (strcmp(theirtty, ptr->ut_line) == 0)   {
					user = ptr;
					found = TRUE;
					break;
				}
				else  {
					/*  not used at present  */
					++logins;
				}
			}
		}
	}
	if ( ! found)   {
		sprintf(tmp, "[Your party is not logged on %s]\n",
			( ! *theirtty )  ?  "\b" : theirtty);
		wheader(inwin, tmp);
		failed = TRUE;
		die();
	}
	sprintf(tmp, "/dev/%s", user->ut_line);
	if ((fp = fopen(tmp, "w")) == NULL)   {
		wheader(inwin, "[Your party has disabled messages]\n");
		failed = TRUE;
		die();
	}
	if (status == CONNECT)   {
		fprintf(fp,
			"\r\7%s (%s) is phoning - respond with 'talk %s'    \n",
						myname, nameof(myname), myname);
	}
	else   {
		if (time(&tloc) != -1)   {
			strcpy(current_time, ctime(&tloc));
			current_time[strlen(current_time) - 1] = NULL;
		}
		else   {
			strcpy(current_time, "now");
		}
		fprintf(fp, "\r\7%s (%s) has stopped phoning [%s]\n",
					myname, nameof(myname), current_time);
	}
	fclose(fp);
	return(user->ut_line);
}



talk()				/*  talk to other user  */

{
	int	clock();

	/*  initialise window cursor positions  */
	inwinx = outwinx = outwiny = 0;
	inwiny = 1;

	delchar = erasechar();
	time_changed = FALSE;

	/*  place cursor in input window  */
	wmove(inwin, inwiny, inwinx);
	wrefresh(inwin);

	signal(SIGALRM, clock);
	alarm(CLOCK_TICK);
	while (1)   {
		/*  place cursor in input window  */
		wmove(inwin, inwiny, inwinx);
		wrefresh(inwin);
		/*  check for any activity  */
		if (input() == FALSE  &&  output() == FALSE)   {
			sleep(1);
		}

		/*******************
		*      SIGALRM was asynchronous to the rest of the program
		*   ie. it occurred whenever it liked and updated elapsed
		*   time.  This worked ok in general, although once in a while
		*   things would get confused (nothing CTRL-l couldn't
		*   overcome).  Fixed by using a global flag.
		*******************/

		if (time_changed)   {			/*  elapsed time  */
			time_changed = FALSE;
			update_time();
		}
	}
}



clock()		/*  set global flag to indicate time should be updated  */

{
	time_changed = TRUE;
	signal(SIGALRM, clock);
	alarm(CLOCK_TICK);
}



update_time()			/*  display current time on screen  */

{
	static int	elapsed_time = 0;

	elapsed_time += CLOCK_TICK;
	wmove(dividewin, divwiny, divwinx);
	wstandout(dividewin);
	wprintw(dividewin, elapsed, elapsed_time / 3600,
					elapsed_time / 60, elapsed_time % 60);
	wstandend(dividewin);
	wrefresh(dividewin);
}



input()			/*  check for and act on user input  */

{
	int	x;

	x = wgetch(inwin);
	getyx(inwin, inwiny, inwinx);
	if (x != -1)   {
		switch(x)   {
			case  END_OF_FILE  :
				waddstr(inwin, "\n[You have disconnected]");
				wrefresh(inwin);
				die();
				break;

			case  REFRESH  :
				clearok(curscr, TRUE);
				wrefresh(curscr);
				break;

			case  BELL  :
				putc(x, writing);
				break;

			default  :
				if (x != delchar)   {
					if (not_printable(x))   {
						beep();
						break;
					}
					wmove(inwin, inwiny, inwinx);
					waddch(inwin, x);
				}
				else   {
					wprevch(inwin);
					wdelch(inwin);
					x = DELETE;
				}
				getyx(inwin, inwiny, inwinx);
				wrefresh(inwin);
				putc(x, writing);
				break;
		}
		return(TRUE);
	}
	return(FALSE);
}



output()		/*  check for and act on any output to be displayed  */

{
	char	c;

	if (read(reading, &c, 1) != 0)   {
		switch (c)   {
			case  DISCONNECT  :
				waddstr(inwin,
					"\n[Your party has disconnected]");
				wrefresh(inwin);
				beep();
				die();
				break;

			case  DELETE  :
				wprevch(outwin);
				wdelch(outwin);
				break;

			case  BELL  :
				beep();
				break;

			default  :
				wmove(outwin, outwiny, outwinx);
				waddch(outwin, c);
				break;
		}
		getyx(outwin, outwiny, outwinx);
		wrefresh(outwin);
		return(TRUE);
	}
	return(FALSE);
}



/*  move cursor back to previous non-space char within window  */

wprevch(win)

WINDOW	*win;

{
	int	x, y;

	getyx(win, y, x);
	if (--x < 0)   {
		if (--y < 0)   {
			wmove(win, 0, 0);
			return;
		}
		for (x = win->_maxx - 1 ; x >= 0 ; --x)   {
			wmove(win, y, x);
			if (winch(win) != SPACE)   {
				return;
			}
		}
		return;
	}
	wmove(win, y, x);
}



char *
nameof(user)			/*  find name of user (from passwd file)  */

char	*user;

{
	struct passwd	*entry, *getpwnam();

	entry = getpwnam(user);
	if (*(entry->pw_gecos))   {
		return(entry->pw_gecos);
	}
	return("no name");
}



forced_die()			/*  program interrupted by signal  */

{
	alarm(0);		/*  disable clock to be safe  */
	forced = TRUE;
	die();
}



die()			/*  gracefully exit program  */

{
	char	*dialup();

	alarm(0);
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);

	if (writing != NULL)   {
		putc(DISCONNECT, writing);
		fclose(writing);
	}
	if (reading != -1)   {
		close(reading);
	}
	if (master  &&  (! connected))   {
		unlink(readfile);
		unlink(writefile);
	}

	wmove(outwin, OUTWINLINES - 1, 0);
	if (forced)   {
		sprintf(tmp, "[Forced exit%s] ",
			(connected)  ?  " (disconnected)" : "");
		waddstr(outwin, tmp);
	}
        wrefresh(outwin);
	flushinp();
	nodelay(inwin, FALSE);
	delwin(inwin);
	delwin(outwin);
	delwin(dividewin);
	endwin();
	if (failed)   {
		exit(4);
	}
	if (! connected)   {
		dialup(DISCONNECT);
	}
	putchar('\n');
	exit(0);
}



/*  print error message and terminate program (via die())  */
error(message, arg1, arg2)

char	*message, *arg1, *arg2;

{
	wmove(outwin, OUTWINLINES - 2, 0);
	wprintw(outwin, "%s: ", progname);
	wprintw(outwin, message, arg1, arg2);
	wrefresh(outwin);
	failed = TRUE;
	die();
}
