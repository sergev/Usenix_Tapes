/*********************
**      Chat        **   The Chat System by SLB
**  Miscellaneous   ** 
**   Subroutines    **        Revision 6.0
**		    **
** prtime()	    **            September 18, 1985
** who()	    **
** ridname()	    **
** cmds()	    **  (misc.c - Written by Sanford L. Barr)
** putch()	    **
** shell()	    **
** error()	    **
** sysexec()	    **
**********************/

#include "chat.h"

prtime(ti, strng)	/* prtime returns the time in 12 hour format to	strng */
char	*strng;
long	ti;
{
	struct	tm	*tp;
	struct	tm	*localtime();
	int	tim,
		min,
		pm;

	tp = localtime(&ti);
	tim = tp->tm_hour;		/* Set hours */
	min = tp->tm_min;		/* Set mins  */
	pm = (tim > 11);
	if (tim	> 11)
		tim -= 12;
	if (tim	== 0)
		tim = 12;
	sprintf(strng, "%2d:%02d%s", tim, min, pm ? "pm" : "am");
	return;
}

cmds()	/* Print command list */
{
	char	input;

	clear();			/* Clear Screen */
	puts("\r\n\r\n\r\n\
------------------------------------\r\n\
------Chat System Command List------\r\n\
------------------------------------\r\n\
These are the available	commands:  -\r\n\
                                   -\r\n\
(L)	Display	users logged in.   -\r\n\
(P)	Page a user via mail(1).   -\r\n\
(Q)	Exit chat session.         -\r\n\
(S)	Send message.              -\r\n\
(V)	Display	version	number.    -\r\n\
(W)	See who's logged into Chat.-\r\n\
(!)	Execute	shell command.     -\r\n\
(!sh)	Escape to shell.	   -\r\n\
(?)	Print this menu.           -\r\n\
------------------------------------\r\n\
");
	hilight("Press [Return] to Continue:");
	input = getchar();
	clear();
	return;
}

putch(ch)			/* output a character */
int ch;
{
	putchar(ch);
}

shell()		/* Read	and execute a shell command */
{
	char	input[80],
		chr;

	ttyset(OFF,ON);				/* back	to normal */
	fputs("\r\nSuspending Chat: Enter command and hit return\r\n!",stdout);
	fgets(input, 80, stdin);

	if (sysexec(input) == 127)
		puts("EXECUTION ERROR: Can't generate a shell\r");

	ttyset(ON,OFF);				/* Cbreak again */
	hilight("Press [Return] to Resume Chat:");
	chr = getchar();
	clear();				/* Clear Screen */
}

who(f, cl)		/* Show	who's logged into chat */
int f, cl;
{

	users =	0;

	lseek(lfile, 0L, 0);		/* Rewind LOGFILE */
	if (!f)	{
		if (cl)
			clear(); 	/* Clear Screen */
		puts("\r\n\r");
		puts("Users In Chat         Tty          When\r");
	}

	while (read(lfile, (char *) &lbuf, sizeof(lbuf))==sizeof(lbuf))	{
		if (lbuf.l_line[0]=='\0' || lbuf.l_name[0]=='\0')
			continue;	/* Skip	blankies */
		if (users > MAXUSERS)
			break;

		sprintf(names[users].l_line, "%5.5s", lbuf.l_line);
		sprintf(names[users].l_name, "%-20.20s", lbuf.l_name);
		sprintf(names[users].l_time, "%7.7s", lbuf.l_time);

		if (!f)
			printf("%-20.20s %-5.5s	 %7.7s\r\n",lbuf.l_name,
						    lbuf.l_line,lbuf.l_time);

	users++;
	}
	if (!f)
		puts("\r");
	return;
}

error(estring,file)
char	*estring;
{

	fputs("SYSTEM FILE ERROR",stderr);
	if (file)
		fprintf(stderr,"\r\n%s\r\n",estring);
	else
		fprintf(stderr," %s\r\n");
}

ridname()	/* Get's rid of	user, on mytty,	in LOGFILE */
{

	lseek(lfile, 0L, 0);			/* Rewind file */

	buffer[0] = '\0';
	while(read(lfile, (char	*) &lbuf, sizeof(lbuf))	== sizeof(lbuf)) {
		if (!strncmp(lbuf.l_line, mytty, length))
			continue;

		strncat(buffer,	lbuf.l_line, sizeof(lbuf.l_line));
		strncat(buffer,	lbuf.l_name, sizeof(lbuf.l_name));
		strncat(buffer,	lbuf.l_time, sizeof(lbuf.l_time));

	}
	strcat(buffer, "\0");

	if ((wfd=fopen(LOGFILE,	"w")) == FERROR) {
		perror(LOGFILE);
		return(ERR);
	}
	fputs(buffer, wfd);

	if (fclose(wfd)	== ERR)	{
		perror(LOGFILE);
		return(ERR);
	}
	return(OK);
}

sysexec(s)
char *s;
{
	int status, pid, w;
	register int (*istat)(), (*qstat)();

	if ((pid = fork()) == 0) {
		setuid(getuid());
		execl("/bin/sh", "sh", "-c", s,	0);
		_exit(127);
	}
	istat =	signal(SIGINT, SIG_IGN);
	qstat =	signal(SIGQUIT,	SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT,	qstat);
	return(status);
}

