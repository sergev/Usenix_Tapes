#include "termio.h"

char	*getenv(),		/* These are here so we can use termlib */
	*tgetstr(),
	PC;			/* global used for padding character */
#undef	putchar			/* need to undefine it so we can redefine it */
int	putchar();
short	ospeed;			/* global used by tputs */
char	*term_name,
	str_buf[60],
	*ptr2ptr,
	*cl_str,
	*cm_str,
	*ce_str,
	terminfo_buff[1024];
	

int	timer();		/* pre-declare for signal setup in unblocked_io	*/
jmp_buf	env;			/* environment storage for setjmp/longjmp */

struct	sgttyb	org_stty,	/* Global so that we can restore Exact state */
		cur_stty;

/*
	Assign_tty: Attempts to open a tty device by pathname (ie.
	/dev/ttyi2). If successful, then return file descriptor, else
	return -1. Will also set terminal for exclusive use mode if
	open was successful.

	On Entry: string pointer to tty pathname
	On Exit:  file descriptor or -1 for failure
*/

assign_tty(path)
char	*path;
{
int	tty_fd;

	if ( (tty_fd = open(path, O_RDWR)) >= 0 )
	{
		if ( ioctl(tty_fd, TIOCEXCL) == -1 )
		{
			tty_fd = -1;
		}
	}
	return((tty_fd < 0) ? -1 : tty_fd);
}


/*
	Deassign_tty: Attempts to close a tty device by file descriptor.
	First, will reset tty to non-exclusive use mode.

	On Entry: int file descriptor
	On Exit:  0 for success or -1 for failure
*/

deassign_tty(tty_fd)
int	tty_fd;
{

	ioctl(tty_fd, TIOCNXCL);
	return(close(tty_fd));
}


/* 
	Get_tty: Will return the current sgttyb structure information
		 for a previously assigned tty.

	On Entry: tty file descriptor, and a pointer to a structure to
		  store the information in.
	On Exit:  return -1 if error else tty info in structure if all ok
*/


get_tty(tty_fd, tty_struct)
int	tty_fd;
struct	sgttyb	*tty_struct;
{
	return(ioctl(tty_fd, TIOCGETP, tty_struct));
}



/* 
	Set_tty: Will set a previously assigned tty to the parameters
		 passed in the sgttyb structure.

	On Entry: tty file descriptor, and a pointer to a structure to
		  read the information from.
	On Exit:  return -1 if error.
*/


set_tty(tty_fd, tty_struct)
int	tty_fd;
struct	sgttyb	*tty_struct;
{
	return(ioctl(tty_fd, TIOCSETP, tty_struct));
}


/*
	Raw_tty: Attempts to open a tty device by pathname (ie.
	/dev/ttyi2), and then will set terminal up for raw i/o
	mode.

	On Entry: string pointer to tty pathname
	On Exit:  file descriptor or -1 for failure
*/

raw_tty(terminal)
char	*terminal;
{
int	tty_fd;

	if ( -1 == (tty_fd = assign_tty(terminal)) )
	{
		return(-1);		/* return with error code	*/
	}

	if ( -1 == get_tty(tty_fd, &org_stty) )
	{
		deassign_tty(tty_fd);
		return(-1);		/* return with error code	*/
	}

	cur_stty = org_stty;		/* copy information	*/
	cur_stty.sg_flags |= RAW;	/* raw mode (all 8 bits pass)	*/
	cur_stty.sg_flags &= ~(TANDEM|ECHO|CRMOD);	/* remove all char processing */

	if ( -1 == set_tty(tty_fd, &cur_stty) )
	{
		deassign_tty(tty_fd);
		return(-1);		/* return with error code	*/
	}
	return(tty_fd);		/* all went well, return tty file desc	*/
}


/*
	cook_tty: Will set a terminal back to its original state by
	file descriptor and will leave that descriptor open.

	On Entry: tty file descriptor
	On Exit:  returns 0 on success or -1 on failure
*/

cook_tty(tty_fd)
int	tty_fd;
{
	if ( -1 == set_tty(tty_fd, &org_stty) )
	{
		return(-1);		/* return with error code	*/
	}

	return(0);	/* return with no errors	*/
}


/*
	uncook_tty: Will set a terminal back to its raw state by
	file descriptor and will leave that descriptor open.

	On Entry: tty file descriptor
	On Exit:  returns 0 on success or -1 on failure
*/

uncook_tty(tty_fd)
int	tty_fd;
{
	if ( -1 == set_tty(tty_fd, &cur_stty) )
	{
		return(-1);		/* return with error code	*/
	}

	return(0);	/* return with no errors	*/
}


/*
	reset_tty: Attempts to set a terminal back to its original state by
	file descriptor and then will close tty file descriptor.

	On Entry: tty file descriptor
	On Exit:  returns 0 on success or -1 on failure
*/

reset_tty(tty_fd)
int	tty_fd;
{
	if ( -1 == set_tty(tty_fd, &org_stty) )
	{
		deassign_tty(tty_fd);
		return(-1);		/* return with error code	*/
	}

	deassign_tty(tty_fd);
	return(0);	/* return with no errors	*/
}


/*
	blocked_io: Attempts to set a terminal to blocked i/o mode by
	file descriptor.

	On Entry: tty file descriptor
	On Exit:  returns fcntl value on success or -1 on failure
*/

blocked_io(tty_fd)
int	tty_fd;
{
int	f_flags;

	f_flags = fcntl(tty_fd, F_GETFL, 0);	/* get current file flags */
	if (f_flags == -1)
	{
		return(-1);
	}
	f_flags += FNDELAY;	/* no delay on read/write (no blocking i/o) */
	return(fcntl(tty_fd, F_SETFL, f_flags));
}


/*
	unblocked_io: Attempts to set a terminal to unblocked i/o mode by
	file descriptor.

	On Entry: tty file descriptor
	On Exit:  returns fcntl value on success or -1 on failure
*/

unblocked_io(tty_fd)
int	tty_fd;
{
int	f_flags;

	f_flags = fcntl(tty_fd, F_GETFL, 0);	/* get current file flags */
	if (f_flags == -1)
	{
		return(-1);
	}
	f_flags -= FNDELAY;	/* delay on read/write (blocking i/o) */
	return(fcntl(tty_fd, F_SETFL, f_flags));
}


/*	timer: Is an interrupt routine that will perform a longjmp 	*/
/*		when it is called by the alarm interrupt.		*/

timer(signal)
int	signal;	/* variable to catch signal sent to routine.	*/
		/* we will just ignore it.			*/
{
	longjmp(env, 1);
}


/*	timed_read: Will attempt to read from the terminal specified	*/
/*		    by the tty file descriptor for a set period of time.*/
/*									*/
/*	On Entry: tty file descriptor to read from, pointer to a byte	*/
/*		  to return received char in number of bytes to read	*/
/*		  and time in seconds.					*/
/*	On Exit:  0 for ok read or -1 for no char received.		*/
/*									*/

timed_read(tty_fd, ch, count, seconds)
int	tty_fd,
	count;
byte	*ch;
unsigned	seconds;
{
int	ret_code,
	tim_code;


	signal(SIGALRM, timer);
	alarm(seconds);

	ret_code = -1;	/* assume bad read */

	tim_code = setjmp(env);
	while ( (tim_code == 0) && (ret_code != 1) )
	{
		ret_code = read(tty_fd, ch, count);
	}

	alarm(0);	/* turn alarms off */
	signal(SIGALRM, SIG_DFL);

	return( ((ret_code != -1) && (ret_code == count)) ? 0 : -1 );
}


/*	stimed_read: Will attempt to read from the terminal specified	*/
/*		    by the tty file descriptor for a set period of time.*/
/*									*/
/*	On Entry: tty file descriptor to read from, pointer to a byte	*/
/*		  to return received char in.				*/
/*		  and time in seconds.					*/
/*	On Exit:  0 for ok read or -1 for no char received.		*/
/*									*/

stimed_read(tty_fd, ch, seconds)
int	tty_fd;
byte	*ch;
unsigned	seconds;
{
int	ret_code,
	tim_code;


	signal(SIGALRM, timer);
	alarm(seconds);

	ret_code = -1;	/* assume bad read */

	tim_code = setjmp(env);
	while ( (tim_code == 0) && (ret_code != 1) )
	{
		ret_code = read(tty_fd, ch, 1);
	}

	alarm(0);	/* turn alarms off */
	signal(SIGALRM, SIG_DFL);

	return( (ret_code != -1) ? 0 : ret_code );
}


term_setup()
{
	term_name = getenv("TERM");	/* get terminal type name */
	if (term_name == (char *) 0)	/* see if TERM is defined */
	{
		return(-1);		/* no, so return error */
	}

	if (tgetent(terminfo_buff, term_name) != 1)	/* read termcap info */
	{
		return(-1);		/* if not in termcap then return error */
	}

	gtty(1, &org_stty);		/* get stdout info */
	ospeed = org_stty.sg_ospeed;	/* set global for tputs to use */

	ptr2ptr = str_buf;		/* setup pointer to a pointer */
	cm_str = tgetstr("pc", &ptr2ptr); /* get pad character */
	if (cm_str)			/* if any pad character */
	{
		PC = *cm_str;		/* set global to it */
	}
	else
	{
		PC = 0;			/* else say no padding needed */
	}

	cl_str = tgetstr("cl", &ptr2ptr);	/* get clear screen string */
	cm_str = tgetstr("cm", &ptr2ptr);	/* get cursor motion string */
	ce_str = tgetstr("ce", &ptr2ptr);	/* get clear to end of line */

	return(0);		/* return all ok */
}


cls()
{
	tputs(cl_str, tgetnum("li"), putchar);
}

clr_eol()
{
	tputs(ce_str, tgetnum("li"), putchar);
}


mov_cur(col, line)
int	col,
	line;
{
	col--;		/* termlib starts screens at 0,0 so we */
	line--;		/* need to normalize OUR coordinates */

	tputs(tgoto(cm_str, col, line), 1, putchar);
}


tolower(ch)
char	ch;
{
	return( ( (ch >= 'A') && (ch <= 'Z') ) ? ch + ' ' : ch );
}
