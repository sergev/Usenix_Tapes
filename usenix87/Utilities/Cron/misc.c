#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: misc.c,v 1.5 87/05/02 17:34:04 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/misc.c,v $
 * $Revision: 1.5 $
 * $Log:	misc.c,v $
 * Revision 1.5  87/05/02  17:34:04  paul
 * baseline for mod.sources release
 * 
 * Revision 1.4  87/04/02  16:54:45  paul
 * added BSD/ATT differences in be_different()
 *   (another idea from rs@mirror)
 * 
 * Revision 1.3  87/02/11  00:55:10  paul
 * added strcmp_until; moved get_shell into user.c
 * 
 * Revision 1.2  87/02/02  19:25:01  paul
 * various
 * 
 * Revision 1.1  87/01/26  23:47:57  paul
 * Initial revision
 * 
 * vix 15jan87 [added TIOCNOTTY, thanks csg@pyramid]
 * vix 30dec86 [written]
 */

/* Copyright 1987 by Vixie Enterprises
 * All rights reserved
 *
 * Distribute freely, except: don't sell it, don't remove my name from the
 * source or documentation (don't take credit for my work), mark your changes
 * (don't get me blamed for your possible bugs), don't alter or remove this
 * notice.  Commercial redistribution is negotiable; contact me for details.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Paul Vixie, Vixie Enterprises, 329 Noe Street, San Francisco, CA, 94114,
 * (415) 864-7013, {ucbvax!dual,ames,ucsfmis,lll-crg,sun}!ptsfa!vixie!paul.
 */


#include "cron.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>


char *
savestr(str)
	char	*str;
{
	char	*malloc(), *strcpy();
	char	*temp;

	temp = malloc((unsigned) (strlen(str) + 1));
	(void) strcpy(temp, str);
	return temp;
}


int
nocase_strcmp(left, right)
	char	*left;
	char	*right;
{
	while (*left && (MkLower(*left) == MkLower(*right)))
	{
		left++;
		right++;
	}
	return MkLower(*left) - MkLower(*right);
}


int
strcmp_until(left, right, until)
	char	*left;
	char	*right;
	char	until;
{
	register int	diff;

	Debug(DPARS|DEXT, ("strcmp_until(%s,%s,%c) ... ", left, right, until))

	while (*left && *left != until && *left == *right)
	{
		left++;
		right++;
	}

	if (	(*left=='\0' || *left == until) 
	    &&	(*right=='\0' || *right == until)
	   )
		diff = 0;
	else
		diff = *left - *right;

	Debug(DPARS|DEXT, ("%d\n", diff))

	return diff;
}


/* strdtb(s) - delete trailing blanks in string 's' and return new length
 */
int
strdtb(s)
	register char	*s;
{
	register char	*x = s;

	/* scan forward to the null
	 */
	while (*x)
		x++;

	/* scan backward to either the first character before the string,
	 * or the last non-blank in the string, whichever comes first.
	 */
	do	{x--;}
	while (x >= s && isspace(*x));

	/* one character beyond where we stopped above is where the null
	 * goes.
	 */
	*++x = '\0';

	/* the difference between the position of the null character and
	 * the position of the first character of the string is the length.
	 */
	return x - s;
}


int
set_debug_flags(flags)
	char	*flags;
{
	/* debug flags are of the form    flag[,flag ...]
	 *
	 * if an error occurs, print a message to stdout and return FALSE.
	 * otherwise return TRUE after setting ERROR_FLAGS.
	 */

#if !DEBUGGING

	printf("this program was compiled without debugging enabled\n");
	return FALSE;

#else /* DEBUGGING */

	char	*pc = flags;

	DEBUG_FLAGS = 0;

	while (*pc)
	{
		char	**test;
		int	mask;

		/* try to find debug flag name in our list.
		 */
		for (	test = DEBUG_FLAG_NAMES, mask = 1;
			*test && strcmp_until(*test, pc, ',');
			test++, mask <<= 1
		    )
			;

		if (!*test)
		{
			fprintf(stderr,
				"unrecognized debug flag <%s> <%s>\n",
				flags, pc);
			return FALSE;
		}

		DEBUG_FLAGS |= mask;

		/* skip to the next flag
		 */
		while (*pc && *pc != ',')
			pc++;
		if (*pc == ',')
			pc++;
	}

	if (DEBUG_FLAGS)
	{
		int	flag;

		fprintf(stderr, "debug flags enabled:");

		for (flag = 0;  flag < DCOUNT;  flag++)
			if (DEBUG_FLAGS & (1 << flag))
				fprintf(stderr, " %s", DEBUG_FLAG_NAMES[flag]);
		fprintf(stderr, "\n");
	}

	return TRUE;

#endif /* DEBUGGING */
}


void
set_cron_uid()
{
	int	seteuid();

	if (ERR == seteuid(ROOT_UID))
	{
		perror("seteuid");
		exit(ERROR_EXIT);
	}
}


#if defined(BSD)
void
be_different()
{
	/* release the control terminal:
	 *  get new pgrp (name after our PID)
	 *  do an IOCTL to void tty association
	 */

	extern int	getpid(), setpgrp(), open(), ioctl(), close();
	auto int	fd;

	(void) setpgrp(0, getpid());

	if ((fd = open("/dev/tty", 2)) >= 0)
	{
		(void) ioctl(fd, TIOCNOTTY, (char*)0);
		(void) close(fd);
	}
}
#endif /*BSD*/

#if defined(ATT)
void
be_different()
{
	/* not being a system V wiz, I don't know if this is what you have
	 * to do to release your control terminal.  what I want to accomplish
	 * is to keep this process from getting any signals from the tty.
	 *
	 * some system V person should let me know if this works... (vixie)
	 */
	int	setpgrp(), close(), open();

	(void) setpgrp();

	(void) close(STDIN);	(void) open("/dev/null", 0);
	(void) close(STDOUT);	(void) open("/dev/null", 1);
	(void) close(STDERR);	(void) open("/dev/null", 2);
}
#endif /*ATT*/

/* get_char(file) : like getc() but increment LINE_NUMBER on newlines
 */
int
get_char(file)
	FILE	*file;
{
	int	ch;

	ch = getc(file);
	if (ch == '\n')
		Set_LineNum(LINE_NUMBER + 1)
	return ch;
}


/* unget_char(ch, file) : like ungetc but do LINE_NUMBER processing
 */
void
unget_char(ch, file)
	int	ch;
	FILE	*file;
{
	ungetc(ch, file);
	if (ch == '\n')
		Set_LineNum(LINE_NUMBER - 1)
}


/* get_string(str, max, file, termstr) : like fgets() but
 *		(1) has terminator string which should include \n
 *		(2) will always leave room for the null
 *		(3) uses get_char() so LINE_NUMBER will be accurate
 *		(4) returns EOF or terminating character, whichever
 */
int
get_string(string, size, file, terms)
	char	*string;
	int	size;
	FILE	*file;
	char	*terms;
{
	int	ch;
	char	*index();

	while (EOF != (ch = get_char(file)) && !index(terms, ch))
		if (size > 1)
		{
			*string++ = (char) ch;
			size--;
		}

	if (size > 0)
		*string = '\0';

	return ch;
}


/* skip_comments(file) : read past comment (if any)
 */
void
skip_comments(file)
	FILE	*file;
{
	int	ch;

	while (EOF != (ch = get_char(file)))
	{
		/* ch is now the first character of a line.
		 */

		while (ch == ' ' || ch == '\t')
			ch = get_char(file);

		if (ch == EOF)
			break;

		/* ch is now the first non-blank character of a line.
		 */

		if (ch != '\n' && ch != '#')
			break;

		/* ch must be a newline or comment as first non-blank
		 * character on a line.
		 */

		while (ch != '\n' && ch != EOF)
			ch = get_char(file);

		/* ch is now the newline of a line which we're going to
		 * ignore.
		 */
	}
	unget_char(ch, file);
}
