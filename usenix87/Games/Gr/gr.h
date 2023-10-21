/*
 * $Header: gr.h,v 1.14 86/12/17 19:25:02 mcooper Exp $
 *------------------------------------------------------------------
 *
 * $Source: /big/src/usc/bin/gr/RCS/gr.h,v $
 * $Revision: 1.14 $
 * $Date: 86/12/17 19:25:02 $
 * $State: Exp $
 * $Author: mcooper $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 *
 * Michael A. Cooper (mcooper@oberon.USC.EDU)
 * University Computing Services,
 * University of Southern California
 *
 *------------------------------------------------------------------
 * $Log:	gr.h,v $
 * Revision 1.14  86/12/17  19:25:02  mcooper
 * Config file is now called "gr.conf".
 * 
 * Revision 1.13  86/12/02  19:50:43  mcooper
 * Removed all references to FINDTTY
 * since findtty.c is not being distributed.
 * 
 * Revision 1.12  86/11/07  19:23:21  mcooper
 * If BSD42 is defined (from Makefile), then we
 * define FINDTTY.  This file should no longer need
 * tweaking by people installing gr.
 * 
 * Revision 1.11  86/07/17  15:22:39  mcooper
 * Bumped MAXTTYS to 120 for portability to
 * other UCS machines.
 * 
 * Revision 1.10  86/07/14  15:54:47  mcooper
 * Moved most of the compile time configuration
 * options to the control file.
 * 
 * Revision 1.9  86/07/03  14:27:44  mcooper
 * - Major work on "special" ttys.  
 *   - Bug fix in deltty().
 *   - Changed are #define macros.
 *   - Added findtty() (in findtty.c) to try and
 *     determine the users real tty.  (4.2bsd)
 * - Added debug mode.
 * - Misc. - Can't remember them all.
 * 
 * Revision 1.8  86/06/18  13:47:58  mcooper
 * Implemented BAD_TTYS - Check to see if
 * the user is on a BAD_TTYS.  If so, game
 * playing is not permited at all.  Also
 * cleaned up #ifdef TOD stuff.
 * 
 * Revision 1.7  86/06/04  12:21:55  mcooper
 * Use SP_NAME for name of special ttys when
 * printing messages to the user.
 * 
 * Revision 1.6  86/06/02  13:32:20  mcooper
 * More cleanup on SP_TTYS.
 * 
 * Revision 1.5  86/05/30  21:34:05  mcooper
 * Added support for "special" ttys.  i.e. only a limited
 * number of SP_TTYS.
 * 
 * Revision 1.4  86/05/14  16:09:17  mcooper
 * Cleaned up and somewhat de-linted.
 * 
 * Revision 1.3  86/03/25  15:50:31  mcooper
 * Moved CONTROL back to normal place in /usr/games/lib.
 * 
 * Revision 1.2  86/03/25  15:47:57  mcooper
 * Lines beginning with '#' are comment lines.
 * 
 * Revision 1.1  86/02/12  17:49:54  mcooper
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 * CONTROL - Master control file.
 */
#ifndef DEBUG
# define CONTROL		"/usr/games/lib/gr.conf"
#else
# define CONTROL		"gr.conf"
#endif

/* 
 * MAXTTYS - The maximum number of tty?? in /dev.
 *	      Only used with FINDTTY.
 */
#define MAXTTYS	120


/*
 * Misc. defines - Leave this alone!
 */
#define TRUE		1
#define FALSE		0
#define COMMENT		'#'
#define SLEEPTIME	60		/* sleep interval */
#define dprintf		if (debug) printf

#define CFBUF		256

struct cfinfo {
	char 	cf_game[CFBUF];		/* name of the game */
	double 	cf_load;			/* max load average */
	int 	cf_users;			/* max users */
	int 	cf_priority;		/* game priority (setpriority(2)) */
	char 	cf_badttys[CFBUF];	/* ttys game playing not permitted on */
	char 	cf_freettys[CFBUF];	/* list of ttys to watch */
	int 	cf_nofreettys;		/* number of freettys must be open */
	char 	cf_spname[CFBUF];	/* name to use for freettys */
	int 	cf_todmorn;			/* hour of morning for games off */
	int 	cf_todeven;			/* hour of evening for games on */
	char	cf_hidedir[CFBUF];	/* directory containing the real games */
	char	cf_logfile[CFBUF];	/* file to log info to */
	char	cf_nogames[CFBUF];	/* file that says no games */
};

