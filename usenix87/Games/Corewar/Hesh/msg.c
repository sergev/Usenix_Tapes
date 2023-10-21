/*
** msg.c -	code dealing with the printing of messages
**
**	Routines:
**		msginit()	initialize the window
**		msgfini()	de-initialize the window
**		msg(fmt, args)	write message to window
**		msgclear()	clear the message window
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
**
static	char	rcsid[] = "$Header: msg.c,v 7.0 85/12/28 14:36:01 ccohesh Dist $";
**
*/

#include <curses.h>
#include <ctype.h>
#include "cw.h"


static	WINDOW	*msgwin;	/* status window for msg's */

extern	void	doadd();

/*
** msginit()	- initialize the message window
**		- return non-zero on error
*/
int	msginit ()
{
	if (!(msgwin = subwin(stdscr, MSGLINES, MSGCOLS, MSGYBEGIN, MSGXBEGIN)))
	{
		fprintf(stderr, "%s: message screen initialization error\n",
			argv0);
		return(TRUE);
	}
	wclear(msgwin);
	scrollok(msgwin, TRUE);
	return(FALSE);
}

/*
** msgfini()	- re-initialize the message window
*/
void	msgfini ()
{
	wclear(msgwin);
	delwin(msgwin);
}

static char msgbuf[BUFSIZ];
/*
** msg()	- display message in message window using printf() like
**		  function
*/
/*VARARGS1*/
void	msg (fmt, args)
char	*fmt;
int	args;
{
	/*
	** if the string is "", just clear the line
	*/
	if (*fmt == '\0')
	{
		wclear(msgwin);
		wrefresh(msgwin);
		return;
	}
	/*
	** otherwise print the message and flush it out
	*/
	doadd(fmt, &args);
	waddch(msgwin, '\n');
	waddstr(msgwin, msgbuf);
	wrefresh(msgwin);
}

static	void	doadd (fmt, args)
char	**fmt;
int	***args;
{
	static	FILE	junk;

	/*
	** Do the printf into buf
	*/
#ifdef	_IOSTRG
	junk._flag = _IOWRT + _IOSTRG;
	junk._ptr = &msgbuf[0];
#else
 	junk._flag = _IOWRT;
 	junk._file = _NFILE;
 	junk._ptr = (unsigned char *) &msgbuf[0];
#endif
	junk._cnt = 32767;
	_doprnt(fmt, args, &junk);
	putc('\0', &junk);
}

/*
** punctrl()	- print a readable version of a certain character
**
**	Note:	Due to the inconsistent availability of a function to perform
**		this, my own version has been built in and used in place of
**		any pre-existing function.  I believe that this particular
**		version suts down on data space considerably from the versions
**		I have found on the Berkley systems.
*/
char	*punctrl (ch)
char	ch;
{
	static	char	*str = "^ ";

	ch &= 0177;
	if (ch >= ' ' && ch <= '~')
	{
		static	char	*str1 = " ";

		*str1 = ch;
		return(str1);
	}
	if (ch == CINTR)
		return("^?");
	*(str+1) = ch + '@';
	return(str);
}
