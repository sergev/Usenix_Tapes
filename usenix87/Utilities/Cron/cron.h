/* cron.h - header for vixie's cron
 *
 * $Header: cron.h,v 1.4 87/05/02 17:33:08 paul Exp $
 * $Source: /usr/src/local/vix/cron/cron.h,v $
 * $Revision: 1.4 $
 * $Log:	cron.h,v $
 * Revision 1.4  87/05/02  17:33:08  paul
 * baseline for mod.sources release
 * 
 * Revision 1.3  87/03/31  12:07:13  paul
 * more and more and more suggestions from rs@mirror
 * 
 * Revision 1.2  87/02/13  00:29:14  paul
 * getting ready for beta test
 * 
 * Revision 1.1  87/02/13  00:26:01  paul
 * Initial revision
 *
 * vix 14jan87 [0 or 7 can be sunday; thanks, mwm@berkeley]
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

#ifndef	_CRON_FLAG
#define	_CRON_FLAG

#include <stdio.h>
#include <ctype.h>
#include <bitstring.h>

/*
 * these are site-dependent
 */
			/*
			 * choose one of these MAILCMD commands.  I use
			 * /bin/mail for speed; it makes 'biff' go but doesn't
			 * do aliasing.  /usr/lib/sendmail does aliasing but is
			 * a hog for short messages.
			 */

/*# define MAILCMD "/usr/lib/sendmail -F\"Vcron Daemon\" -odi -oem  %s"  /*-*/
			/* -Fx	= set full-name of sender
			 * -odi	= Option Deliverymode Interactive (synchronous)
			 * -oem	= Option Errors Mailedtosender
			 */

# define MAILCMD "/bin/mail -d  %s"		/*-*/
			/* -d = undocumented but common flag: deliver locally?
			 */

			/* LIBDIR is unrelated to /usr/lib; it's actually
			 * the base of the cron spool tree.  any control
			 * files for cron will be there; also a directory
			 * called 'crontabs' will be there to hold the
			 * individual cron tabs.  this is less clean than
			 * a /usr/spool/cron, /usr/lib/cron; you can change
			 * it to be that way at your own taste.  remember
			 * that SysV does it /usr/spool/cron, .../crontabs.
			 */
#define	LIBDIR		"/usr/spool/cron/%s"

			/* SPOOLDIR is where the crontabs live
			 */
#define	SPOOLDIR	"/usr/spool/cron/crontabs/%s"

			/* this file is created by 'crontab' when it changes
			 * a cron tab; 'cron' sees it and reloads its tables.
			 */
#define	POKECRON	"/usr/spool/cron/POKECRON"

			/* this is the name of the environment variable
			 * that contains the user name.  it isn't read by
			 * cron, but it is SET by crond in the environments
			 * it creates for subprocesses.  on BSD, it will
			 * always be USER; on SysV it could be LOGNAME or
			 * something else.
			 */
#define	USERENV		"USER"

/*
 * you shouldn't have to change anything after this
 */


	/* these are really immutable, and are
	 *   defined for symbolic convenience only
	 * TRUE, FALSE, and ERR must be distinct
	 */
#define TRUE		1
#define FALSE		0
	/* system calls return this on success */
#define OK		0
	/*   or this on error */
#define ERR		(-1)

	/* turn this on to get '-x' code */
#define DEBUGGING	FALSE

#define READ_PIPE	0	/* which end of a pipe pair do you read? */
#define WRITE_PIPE	1	/*   or write to? */
#define STDIN		0	/* what is stdin's file descriptor? */
#define STDOUT		1	/*   stdout's? */
#define STDERR		2	/*   stderr's? */
#define VFORK		vfork	/* sometimes vfork might be too virtual */
#define ERROR_EXIT	1	/* exit() with this will scare the shell */
#define	OK_EXIT		0	/* exit() with this is considered 'normal' */
#define	MAX_FNAME	100	/* max length of internally generated fn */
#define	MAX_COMMAND	1000	/* max length of internally generated cmd */
#define	MAX_TEMPSTR	100	/* obvious */
#define	MAX_UNAME	20	/* max length of username, should be overkill */
#define	ROOT_UID	0	/* don't change this, it really must be root */
#define	DEXT		0x0001	/* extend flag for other debug masks */
#define	DSCH		0x0002	/* scheduling debug mask */
#define	DPROC		0x0004	/* process control debug mask */
#define	DPARS		0x0008	/* parsing debug mask */
#define	DMISC		0x0010	/* misc debug flag */
#define	DCOUNT		5	/* how many debug bits are in use? */

#define	MAILSUBJ	"one of your cron commands generated this output"
#define	REG		register
#define	PPC_NULL	((char **)NULL)

#define	Skip_Blanks(c, f) \
			while (c == '\t' || c == ' ') \
				c = get_char(f);

#define	Skip_Nonblanks(c, f) \
			while (c!='\t' && c!=' ' && c!='\n' && c != EOF) \
				c = get_char(f);

#define	Skip_Line(c, f) \
			do {c = get_char(f);} while (c != '\n' && c != EOF);

#if DEBUGGING
# define Debug(mask, message) \
			if ( (DEBUG_FLAGS & (mask) ) == (mask) ) \
				printf message;
#else /* !DEBUGGING */
# define Debug(mask, message) \
			;
#endif /* DEBUGGING */

#define	MkLower(ch)	(isupper(ch) ? tolower(ch) : ch)
#define	MkUpper(ch)	(islower(ch) ? toupper(ch) : ch)
#define	Set_LineNum(ln)	{Debug(DPARS|DEXT,("linenum=%d\n",ln)); \
			 LINE_NUMBER = ln; \
			}

#define	FIRST_MINUTE	0
#define	LAST_MINUTE	59
#define	MINUTE_COUNT	(LAST_MINUTE - FIRST_MINUTE + 1)

#define	FIRST_HOUR	0
#define	LAST_HOUR	23
#define	HOUR_COUNT	(LAST_HOUR - FIRST_HOUR + 1)

#define	FIRST_DOM	1
#define	LAST_DOM	31
#define	DOM_COUNT	(LAST_DOM - FIRST_DOM + 1)

#define	FIRST_MONTH	1
#define	LAST_MONTH	12
#define	MONTH_COUNT	(LAST_MONTH - FIRST_MONTH + 1)

#define	FIRST_DOW	0
#define	LAST_DOW	7
#define	DOW_COUNT	(LAST_DOW - FIRST_DOW + 1)

			/* each users' crontab will be held as a list of
			 * the following structure.
			 */

typedef	struct	_entry
	{
		struct _entry	*next;
		char		*cmd;
		bit_decl(	minute,	MINUTE_COUNT	)
		bit_decl(	hour,	HOUR_COUNT	)
		bit_decl(	dom,	DOM_COUNT	)
		bit_decl(	month,	MONTH_COUNT	)
		bit_decl(	dow,	DOW_COUNT	)
		int		dom_star, dow_star;
	}
	entry;

			/* the crontab database will be a list of the
			 * following structure, one element per user.
			 */

typedef	struct	_user
	{
		struct _user	*next;		/* next item in list */
		int		uid;		/* uid from passwd file */
		int		gid;		/* gid from passwd file */
		char		**envp;		/* environ for commands */
		entry		*crontab;	/* this person's crontab */
	}
	user;

				/* in the C tradition, we only create
				 * variables for the main program, just
				 * extern them elsewhere.
				 */

#ifdef MAIN_PROGRAM

# if !defined(LINT) && !defined(lint)
		static char *copyright[] = {
			"@(#) Copyright (C) 1987 by Vixie Enterprises",
			"@(#) All rights reserved"
		};
# endif

		char *MONTH_NAMES[] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
			NULL};
		char *DOW_NAMES[] = {
			"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun",
			NULL};
		char *PROGNAME;
		int LINE_NUMBER;

# if DEBUGGING
		int DEBUG_FLAGS;
		char *DEBUG_FLAG_NAMES[] = {	/* sync with #defines */
			"ext", "sch", "proc", "pars", "misc",
			NULL};
# endif /* DEBUGGING */

#else /* !MAIN_PROGRAM */

		extern char	*MONTH_NAMES[];
		extern char	*DOW_NAMES[];
		extern char	*PROGNAME;
		extern int	LINE_NUMBER;
# if DEBUGGING
		extern int	DEBUG_FLAGS;
		extern char	*DEBUG_FLAG_NAMES[];
# endif /* DEBUGGING */
#endif /* MAIN_PROGRAM */


#endif	/* _CRON_FLAG */
