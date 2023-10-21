#ifndef lint
static char rcsid[] = "CUCL:	$Header: idledaemon.c,v 1.5 85/09/23 01:40:32 scc Exp $";
#endif lint

/*********************************************************************\
* 								      *
*  								      *
* 			    Copyright Notice: 			      *
* 								      *
* 	    (C) Cambridge University Computer Laboratory 1985	      *
* 								      *
* 	This program is Public Domain.  Permission is hereby given    *
* 	to anyone to redistributed it to anyone they want to, with    *
* 	the proviso that this notice is included unaltered.	      *
* 								      *
*	Neither the author or C.U.C.L accepts legal responsibility    *
*	for bugs in this program, or for any trouble it may cause.    *
*								      *
* 	Please send any comments, suggestions for improvements and    *
* 	bug reports/fixes to the author :-			      *
* 								      *
* 			    Stephen Crawley			      *
* 								      *
* 		    USENET: scc@cl-jenny.UUCP			      *
* 		    ARPA:   scc%cl.cam.ac.uk@ucl-cs.ARPA	      *
* 		    JANET:  scc@uk.ac.cam.cl			      *
* 								      *
\*********************************************************************/


/*
 * idledaemon [-a] [-d] [-f<number>] [-{s,i,g}<minutes>] tty ...
 *
 * Kick idle users off the system if there is a shortage of terminals.
 * 	-a	kick them off anyway (even if there are free lines)
 *	-f	try to keep this many lines free
 *	-s 	gives the time idledaemon sleeps between runs
 *	-i	gives the idle threshold
 *	-g	gives the grace period
 *	-d	debug mode ... send messages to users, but don't
 *		actually kill anything.  I used this to get people
 *		used to the idea before installing the daemon for real.
 *
 * Idledaemon first checks /etc/utmp to see how many of the named terminals
 * are free.  If enough terminals are free nothing more is done.  Otherwise,
 * idle terminals for which a warning has been sent and for which the grace
 * period has expired are SIGHUPed then SIGKILLed.  If the number of free
 * terminals is still less than the threshold, warnings are sent to any 
 * other idle terminals.
 *
 * Idledaemon's idea of an idle terminal is one for which there has been no
 * terminal input processed in the time period, and for which there are no
 * background jobs running or "sleeping".  Detached processes such as 
 * sysline are excluded from the latter category.
 *
 * If you send a SIGINT to the daemon, it will print a summary of the
 * the state of the world to standard output.
 */


/* #define DEBUG	/* print debugging info on stderr */


#define SECSPERMIN 60

/*
 * Tuning parameters and argument defaults.
 */
#define PROC_SLEEPTIME 		10
#define WARNING_TIMEOUT		(20 * SECSPERMIN)
#define DFLT_IDLETIME 		(30 * SECSPERMIN)
#define DFLT_GRACETIME 		(5 * SECSPERMIN)
#define DFLT_LOOPTIME 		(5 * SECSPERMIN)
#define DFLT_MINTTYSFREE 	1



#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <nlist.h>
#include <setjmp.h>
#include <utmp.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/stat.h>

struct nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_nproc" },
#define	X_NPROC		1
	{ "" },
};

extern errno;
long lseek();
char *malloc();
char *strncpy();

int gracetime = DFLT_GRACETIME;	/* Minimum time between message and zapping */
int idletime = DFLT_IDLETIME;	/* Time allowed before a terminal is idle */
int looptime = DFLT_LOOPTIME;	/* Sleep this long between looking */

int minttysfree = DFLT_MINTTYSFREE; 
				/* If there are less than this many terminals
				   free, start invoking sanctions ... */
				    
int killanyway = 0;		/* If non-zero, kill 'em irrespective of the
				   number of free terminals. */

int debug = 0;			/* Disable killing, and modify messages
				   sent to user's terminals */

struct ttyinfo
{
    char	tty_line[8];	/* Terminal name */
    int		tty_state;
    char	tty_name[8];	/* Logged in user's name */
    time_t	tty_logintime;
    time_t	tty_accesstime;
    time_t	tty_warningtime;
    struct proc *tty_shellproc;	/* Process table entry for login shell */
    short	tty_shellpid;	/* Process id of login shell */
    short	tty_notifier;	/* Process which is writing to terminal */
};

/*
 * State of terminal.
 */
#define TTY_UNKNOWN	0	/* tty not in utmp (yet) */
#define TTY_FREE	1	/* not logged in */
#define TTY_ACTIVE	2	/* logged in */
#define TTY_NOINPUT	3	/* idle session (last time we checked) */
#define TTY_WARNED	4	/* a warning has been sent */
#define TTY_KILLED	5	/* session has been killed (transient) */
#define TTY_STUCK	6	/* session is unkillable ! */
#define NOS_STATES	7

char *statestrings[NOS_STATES] = {
	"UNKNOWN", "FREE", "ACTIVE", "NOINPUT", 
	"WARNED", "KILLED", "STUCK"};

int nttys, nttysfree;	/* number of terminals and terminals currently free */
struct ttyinfo *ttytable;

int utmp = -1;		/* fd for /etc/utmp */
int nutmp = 0;		/* number of utmp entries */
struct utmp *utmptable = 0;

/*
 * Process table globals.
 */
int kmem;		/* fd for /dev/kmem */

unsigned long procp;	/* base of process table */
unsigned long nproc;	/* entries in process table */
struct proc *proctable;

unsigned long kreadvar();

time_t timenow;
int notifiers;		/* number of outstanding notification processes */
int notifierskilled;	/* the notifiers have been SIGKILLED */
jmp_buf timeout;	/* ... for abandoning the wait when a notifier 
			   gets stuck ... */

#ifdef DEBUG
#define Bomb abort()
#define dprintf0(msg) fprintf(stderr,msg)
#define dprintf1(msg,a1) fprintf(stderr,msg,a1)
#define dprintf2(msg,a1,a2) fprintf(stderr,msg,a1,a2)
#else DEBUG
#define Bomb exit(1)
#define dprintf0(msg) 
#define dprintf1(msg,a1) 
#define dprintf2(msg,a1,a2) 
#endif DEBUG


main (argc, argv)
int argc;
char **argv;
{
    int     argerror = 0;
    setlinebuf (stderr);

    for (argc--, argv++; *argv && **argv == '-'; argv++, argc--)
    {
	switch ((*argv)[1])
	{
	    case 'a': 
		killanyway++;
		break;
	    case 'd':
		debug++;
		break;
	    case 'f': 
		minttysfree = atoi (&(*argv)[2]);
		break;
	    case 'g': 
		gracetime = atoi (&(*argv)[2]) * SECSPERMIN;
		break;
	    case 'i': 
		idletime = atoi (&(*argv)[2]) * SECSPERMIN;
		break;
	    case 'l': 
		looptime = atoi (&(*argv)[2]) * SECSPERMIN;
		break;
	    default: 
		argerror = 1;
	}
    }

    if (argerror || argc <= 0)
    {
	fprintf (stderr,
		"usage: idledaemon [-a] [-d] [-f<nos>] [-{s,i,g}<mins>] tty ..\n");
	Bomb;
    }

    if (idletime <= looptime || gracetime <= 0 || looptime <= 0)
    {
	fprintf (stderr,
		"idledaemon: inconsistent idle, grace and loop times\n");
	Bomb;
    }

    if (chdir ("/dev") < 0)
    {
	perror ("idledaemon: can't cd to \"/dev\": ");
	Bomb;
    }

    init_signals ();
    init_ttytable (argc, argv);
    init_kmem ();

    /*
     * All OK ... orphan ourselves.
     */
    if (fork() != 0)
	exit (0);

    for (;;)
    {
	timenow = time ((time_t *) 0);
	dprintf1 ("\ntime is %s", ctime(&timenow));
	/*
         * Count the terminals in use, dealing with changes in user.
	 */
	scanutmp ();
        dprintf1 ("there are %d terminals free\n", nttysfree);

	if (nttysfree < minttysfree || killanyway)
	{
	    /*
	     * For each tty in use, see if there has been any input, and
	     * clear the WARNED state if there has been.  The value
	     * returned is the number of possibly idle terminals.
	     */
	    if (scanttys () > 0)
	    {
		struct ttyinfo *tp;

		/*
		 * Knock sessions off one at a time in order of
		 * decreasing idleness.
		 */
		while (nttysfree < minttysfree || killanyway)
		{
		    if (!killmostidle())
			break;
		}
		/*
		 * Satisfied?
		 */
		if (nttysfree < minttysfree || killanyway)
		{
		    /*
		     * Nope!  Send out warnings to the next batch.
		     */
		    read_proctable ();
		    for (tp = ttytable; tp < &ttytable[nttys]; tp++)
		    {
			if (tp -> tty_state == TTY_NOINPUT && ttyidle (tp))
			    ttywarning (tp);
		    }
		}
		collectnotifiers ();
	    }    
	}
	sleep ((unsigned) looptime);
    }
}


/*
 * Initialise tty table from (the rest of) the argument vector.
 */
init_ttytable(argc, argv)
int argc;
char **argv;
{
    struct ttyinfo *tp;

    ttytable = (struct ttyinfo *) malloc((unsigned) argc * sizeof(*ttytable));
    if (ttytable == 0)
    {
	fprintf (stderr, "idledaemon: can't calloc ttytable!");
	Bomb;
    }
    nttys = argc;

    for (tp = ttytable; argc > 0; argc--, argv++, tp++)
    {
	struct stat stbuf;

	/*
	 * Check we can stat each terminal.
	 */
	if (stat(*argv, &stbuf) < 0)
	{
	    fprintf (stderr, "idledaemon: can't stat %s: ", *argv);
	    perror ("");
	    Bomb;
	}
	(void) strncpy (tp -> tty_line, *argv, sizeof (tp -> tty_line));
	tp -> tty_name[0] = '\000';
	tp -> tty_logintime = 0;
	tp -> tty_warningtime = 0;
	tp -> tty_accesstime = 0;
	tp -> tty_shellpid = 0;
	tp -> tty_notifier = 0;
	tp -> tty_state = TTY_UNKNOWN;
    }
}


/*
 * Initialise bits and pieces needed to read the proc table
 */
init_kmem ()
{
    if ((kmem = open ("kmem", O_RDONLY, 0)) < 0)
    {
	perror ("idledaemon: can't open /dev/kmem");
	Bomb;
    }
    nlist ("/vmunix", nl);
    if (nl[0].n_type == 0)
    {
	fprintf (stderr, "idledaemon: can't get namelist for /vmunix\n");
	Bomb;
    }
    procp = kreadvar (X_PROC);
    nproc = kreadvar (X_NPROC);
    proctable = (struct proc *) malloc((unsigned) nproc * sizeof(*proctable));
    if (proctable == 0)
    {
	fprintf (stderr, "idledaemon: can't calloc proc table buffer!\n");
	Bomb;
    }
}


/*
 * Read complete proc table into the proctable buffer.
 */
read_proctable ()
{
    if (lseek (kmem, (long) procp, L_SET) != procp)
    {
	perror ("idledaemon: lseek to proc table failed: ");
	Bomb;
    }
#define proctabsize (nproc * sizeof (struct proc))
    if (read (kmem, (char *) proctable, (int) proctabsize) != proctabsize)
    {
	perror ("idledaemon: proc table read failed: ");
	Bomb;
    }
}


/*
 * Read a long from the kernel.
 */
unsigned long kreadvar (name)
int name;
{
    unsigned long res;
    unsigned long addr = nl[name].n_value;

    if (lseek (kmem, (long) addr, L_SET) != addr)
    {
	perror ("idledaemon: lseek on kmem failed: ");
	Bomb;
    }
    if (read (kmem, (char *) &res, sizeof(res)) != sizeof (res))
    {
	perror ("idledaemon: read on kmem failed: ");
	Bomb;
    }
    dprintf2 ("kreadvar: &%x -> %x\n", addr, res);
    return (res);
}


/*
 * Relocate a proc table pointer, relative to proctable.
 */
struct proc *reloc (pp)
struct proc *pp;
{
    return (struct proc *) (
	    (unsigned) pp - (unsigned) procp + (unsigned) proctable);
}


/*
 * Read utmp file and update the tty table for sessions that have
 * logged in or out.  Then count free terminals..
 */
scanutmp ()
{
    struct stat statbuf;
    int size;
    struct utmp *up;
    struct ttyinfo *tp;

    /*  Open utmp file if we haven't already */
    if (utmp == -1)
    {
	utmp = open ("/etc/utmp", O_RDONLY, 0);
	if (utmp < 0)
	{
	    perror ("idledaemon: can't open /etc/utmp: ");
	    Bomb;
	}
    }
    else
    {
	if (lseek (utmp, (long) 0, L_SET) != 0)
	{
	    perror ("idledaemon: lseek on utmp failed: ");
	    Bomb;
	}
    }

    /*
     * Find out how big the file is.  If it is bigger than it was before,
     * allocate a bigger buffer.
     */
    if (fstat (utmp, &statbuf) < 0)
    {
	perror ("idledaemon: utmp stat failed: ");
	Bomb;
    }
    size = statbuf.st_size;
    if ((size / sizeof (struct utmp)) != nutmp)
    {
	nutmp = size / sizeof (struct utmp);
	if (utmptable != 0)
	    free ((char *) utmptable);
	utmptable = (struct utmp *) malloc ((unsigned) size);
    }

    /*
     * Read utmp and update the tty table from it.
     */
    if (read (utmp, (char *) utmptable, size) != size)
    {
	fprintf (stderr, "idledaemon: utmp read failed\n");
	Bomb;
    }
    for (up = utmptable; up < &utmptable[nutmp]; up++)
    {
	for (tp = ttytable; tp < &ttytable[nttys]; tp++)
	{
	    if (strcmp (tp -> tty_line, up -> ut_line) == 0)
	    {
	        if (up -> ut_name[0] == '\000')
	        {
		    tp -> tty_name[0] = '\000';
		    if (tp -> tty_state != TTY_FREE)
		        ttystate (tp, TTY_FREE);
		}
		else
		{
		    if (up -> ut_time != tp -> tty_logintime ||
			strcmp (tp -> tty_name, up -> ut_name) != 0)
		    {
			(void) strncpy (tp -> tty_name, up -> ut_name, 
				 sizeof (tp -> tty_name));
			tp -> tty_logintime = up -> ut_time;
			tp -> tty_accesstime = 0;
			tp -> tty_shellpid = 0;
			ttystate (tp, TTY_ACTIVE);
		    }
		}
	    }
	}
    }
    /*
     * Count the free terminals.
     */
    nttysfree = 0;
    for (tp = ttytable; tp < &ttytable[nttys]; tp++)
	if (tp -> tty_state == TTY_FREE || tp -> tty_state == TTY_UNKNOWN)
	    nttysfree++;
}


/*
 * Look at the atimes for all ttys in the table, and adjust state.
 * The number of terminals that have had no input is returned.
 */
scanttys ()
{
    struct stat statbuf;
    struct ttyinfo *tp;
    int candidates = 0;

    for (tp = ttytable; tp < &ttytable[nttys]; tp++)
    {
	if (tp -> tty_state != TTY_FREE)
	{
	    if (stat (tp -> tty_line, &statbuf) < 0)
	    {
		fprintf (stderr, "stat(%s) failed: ", tp -> tty_line);
		perror ("");
		continue;
	    }

	    if (statbuf.st_atime != tp -> tty_accesstime)
	    {
	        tp -> tty_accesstime = statbuf.st_atime;
	        if (tp -> tty_state != TTY_ACTIVE)
		{
		    /*
		     * At some point since the last scan, the terminal
		     * has reactivated.  It may not still be active,
		     * but that is dealt with below.
		     */
		    ttystate (tp, TTY_ACTIVE);
		}
	    }
	    
	    /*
	     * Spot idle terminals, and timeout old warnings.
	     */
	    if (timenow > tp -> tty_accesstime + idletime)
	    {
		if (tp -> tty_state == TTY_ACTIVE ||
		    (tp -> tty_state == TTY_WARNED &&
		     timenow > tp -> tty_warningtime + WARNING_TIMEOUT))
		{
		    ttystate (tp, TTY_NOINPUT);
		}
		candidates++;
	    }
	}
    }
    dprintf1 ("scantty: %d candidates\n", candidates);
    return (candidates);
}


/*
 * Decide if a terminal has no active processes.
 */
ttyidle (tp)
struct ttyinfo *tp;
{
    if (tp -> tty_shellpid == 0)
    {	
	/*
	 * Cache information about process shell.
	 */
	if (!findshell (tp))
	    return (0);
    }
    else
    {
	if (tp -> tty_shellproc -> p_pid != tp -> tty_shellpid)
	{
	    dprintf0 ("err ... this process table is like wierd man!\n");
	    if (!findshell (tp))
		return (0);
	}
    }
    return (idleprocs (tp -> tty_shellproc, 0));
}


/*
 * Change terminal state and dprint ... it happens a lot
 */
ttystate (tp, state)
struct ttyinfo *tp;
int state;
{
    tp -> tty_state = state;
    dprintf2 ("%s state -> %s\n", tp -> tty_line, statestrings[state]);
}


/*
 * Starting at the shell process, traverse the tree of processes looking for
 * an active one.  An active process is defined to be any process in states
 * SWAIT, SRUN or SIDL, or in state SSLEEP with slptime < MAXSLP.
 */
idleprocs (pp, depth)
struct proc *pp;
int depth;
{
    register struct proc *cpp;
    register short width = 0;

    if (depth > nproc)
    {
	dprintf0 ("eek!! idleprocs looping vertically!!\n");
	return (0);
    }

    switch (pp -> p_stat)
    {
        case SSLEEP:
	    if (pp -> p_slptime >= PROC_SLEEPTIME)
		break;
	case SWAIT:
	case SRUN:
	case SIDL:
	    dprintf1 ("process %d is active\n", pp -> p_pid);
	    return (0);		/* not idle */
    }
    dprintf1 ("process %d is idle\n", pp -> p_pid);

    /*
     * Recursively check all children starting at youngest.
     */
    cpp = pp -> p_cptr;
    while (cpp != NULL)
    {
	if (width++ > nproc)
	{
	    dprintf0 ("eek!! idleproc looping horizontally!!\n");
	    return (0);
	}
	cpp = reloc (cpp);
	if (!idleprocs(cpp, depth + 1))
	    return (0);
	cpp = cpp -> p_osptr;
    }
    dprintf2 ("checked %d children for %d ...\n", width, pp -> p_pid);
    return (1);
}


/*
 * Find the "shell" process associated with the terminal.  
 * The algorithm is ...
 *	Look up the controlling process group for the tty. If the
 *	    associated process DNE, search for one with the same pgrp.
 *	Starting with the process, chain back through the parents until a
 *          process with ppid == 1 is found.  That's the shell.
 */
findshell (tp)
struct ttyinfo *tp;
{
    int tty = open (tp -> tty_line, O_WRONLY, 0);
    short pgrp;
    register paranoia = 0;
    register struct proc *pp, *altpp;

    if (tty < 0)
    {
	fprintf (stderr, "idledaemon: open /dev/%s failed in findshell: ",
		tp -> tty_line);
	perror ("");
	return (0);
    }
    /* 4.2 lint library (?) bug ... */
    if (ioctl (tty, TIOCGPGRP, (char *) &pgrp) < 0)
    {
	fprintf (stderr, "idledaemon: ioctl on /dev/%s to get pgrp failed: ",
		tp -> tty_line);
	perror ("");
	close (tty);
	return (0);
    }
    close (tty);
    for (pp = proctable; pp < &proctable[nproc] && pp -> p_pid != pgrp; pp++)
    {
	if (pp -> p_pgrp == pgrp)
	    altpp = pp;
    }
    if (pp -> p_pid != pgrp)
	if (altpp -> p_pgrp == pgrp)
	{
	    pp = altpp;
	    dprintf2 ("findshell starting with proc %d in pgrp %d\n", 
		      altpp -> p_pid, pgrp);
	}
	else
	{
	    fprintf (stderr, 
		    "idledaemon: no processes for pgrp %d (/dev/%s)\n",
		    pgrp, tp -> tty_line);
	    return (0);
	}
    else
	dprintf1 ("findshell starting with pgrp leader %d\n", pgrp);
    while (pp -> p_ppid > 1)
    {
	pp = reloc (pp -> p_pptr);
	dprintf1 ("parent is %d\n", pp -> p_pid);
	if (paranoia++ > nproc)
	{
	    dprintf0 ("eek!! findshell looping!!\n");
	    return (0);
	}
    }
    dprintf1 ("findshell found process %d\n", pp -> p_pid);
    tp -> tty_shellpid = pp -> p_pid;
    tp -> tty_shellproc = pp;
    return (1);
}


/*
 * Send a HUP to all the decendents of the process ... depth first
 */
hupprocs (pp, depth)
struct proc *pp;
int depth;
{
    register struct proc *cpp;
    register width = 0;
    register i;

    if (depth > nproc)
    {
	dprintf0 ("eek!! killprocs looping vertically!!\n");
	return;
    }

    /*
     * See if the process has gone already ...
     */
    if (kill (pp -> p_pid, 0) < 0)
    {
	dprintf2 ("hupprocs: process %d has already gone away: errno %d\n", 
		pp -> p_pid, errno);
	return;
    }

    dprintf1 ("hupprocs: doing children of process %d\n", pp -> p_pid);

    /*
     * Recursively HUP all children starting at youngest.
     */
    cpp = pp -> p_cptr;
    while (cpp != NULL)
    {
	if (width++ > nproc)
	{
	    dprintf0 ("eek!! killproc looping horizontally!!\n");
	    return;
	}
	cpp = reloc (cpp);
	hupprocs(cpp, depth + 1);
	cpp = cpp -> p_osptr;
    }
    dprintf2 ("HUPed %d children for %d ...\n", width, pp -> p_pid);
    
    if (kill (pp -> p_pid, SIGHUP) < 0)
    {
	dprintf2 ("HUP to %d failed ... errno %d\n", pp -> p_pid, errno);
	return;
    }
    else
	dprintf1 ("HUP sent to %d\n", pp -> p_pid);
    if (pp -> p_stat == SSTOP)
    {
	(void) kill (pp -> p_pid, SIGCONT);
	dprintf1 ("CONT sent to %d\n", pp -> p_pid);
    }

    for (i = 0; i < 5; i++)
    {
        if (kill (pp -> p_pid, 0) < 0)
	{
	    dprintf1 ("HUPed process %d has gone\n", pp -> p_pid);
	    return;
	}
	dprintf0 (".");	    
	sleep (1);
    }
    dprintf1 ("HUPed process %d didn't go away\n", pp -> p_pid);
}


/*
 * Send an idle warning message.
 */
ttywarning (tp)
struct ttyinfo *tp;
{
    ttymsg (tp, "WARNING: you may be auto-logged out in %d mins.\r\n", 
	    gracetime / SECSPERMIN);
    tp -> tty_warningtime = timenow;	/* (or there abouts) */
    ttystate (tp, TTY_WARNED);
}


/*
 * Search and destroy the terminal with the most idle time.
 */
killmostidle()
{
    register struct ttyinfo *tp,
                           *itp;

    read_proctable ();
 /* Find worst offender.  */
    for (tp = ttytable, itp = (struct ttyinfo  *) 0;
	    tp < &ttytable[nttys]; tp++)
    {
	if (tp -> tty_state == TTY_WARNED &&
		tp -> tty_accesstime + idletime + gracetime < timenow &&
		(!itp || (tp -> tty_accesstime < itp -> tty_accesstime)))
	{
	    if (!ttyidle (tp))
	    {
	    /* 
	     * Zombified session has revived itself without
	     * any input from terminal.  Probably some clock
	     * driven process.
	     */
		dprintf1 ("Urghhh! terminal %s has revived itself\n",
			tp -> tty_line);
		ttystate (tp, TTY_NOINPUT);
	    }
	    else
		itp = tp;
	}
    }
    if (!itp)
	return (0);		/* Ah well ... nothing left to kill */

/*
 * Zap all processes attached to a terminal.  If the shell process goes
 * away, nttysfree is incremented to stop the mainline killing more
 * sessions or sending out warnings unnecessarily (unneces-celery).
 */
    ttymsg (itp, "Your session is being auto-logged out\r\n", 0);
    sleep (1);

    read_proctable ();		/* Hmm ... a little bit of paranoia here */
    if (debug == 0 && (itp -> tty_shellproc -> p_pid == itp -> tty_shellpid))
    {
    /* 
     * Send HUPs to all processes attached to terminal.
     */
	hupprocs (itp -> tty_shellproc, 0);

    /* 
     * If the shell is still hanging around, stick the knife in.
     */
	if (kill (itp -> tty_shellpid, 0) == 0)
	{
	    dprintf1 ("KILLing shell process %d\n", itp -> tty_shellpid);
	    (void) kill (itp -> tty_shellpid, SIGKILL);
	    sleep (5);
	    if (kill (itp -> tty_shellpid, 0) < 0)
		nttysfree++;
	    else
	    {
		dprintf1 ("Shell process %d cannot be killed!!\n", 
			  itp -> tty_shellpid);
		ttystate (itp, TTY_STUCK);
		return (1);
	    }
	}
	else
	    nttysfree++;
    }
    else
    {
	dprintf1 ("skipped kills for process %d\n", itp -> tty_shellpid);
	nttysfree++;
    }
    ttystate (itp, TTY_KILLED);
    return (1);
}


/*
 * Send a message to a user's terminal from a sub-process
 */
ttymsg (tp, message, arg)
struct ttyinfo *tp;
char *message;
int arg;
{
    tp -> tty_notifier = fork();
    if (tp -> tty_notifier < 0)
    {
	perror ("idledaemon: fork failed: ");
	return;
    }
    /*
     * Child process sends message.  It will be collected later.
     */
    if (tp -> tty_notifier == 0)
	notifier (tp -> tty_line, message, arg);	/* does not return */
    notifiers++;
    (void) alarm (2);
}


/*
 * ALARM handler for collectnotifiers() ... give the notifiers a helping hand
 */
killnotifiers ()
{
    struct ttyinfo *tp;

    /*
     * If we have already sent the kills, something screwy is going on!
     */
    if (notifierskilled)
    {
	dprintf0 ("Timeout for collectnotifiers() after sending kills!\n");
	/* 4.2 lint library bug ... */
	longjmp(timeout, 1);
    }

    for (tp = ttytable; tp < &ttytable[nttys]; tp++)
    {
        if (tp -> tty_notifier > 0)
	{
	    (void) kill (tp -> tty_notifier, SIGKILL);
	    dprintf1 ("killing notifier %d\n", tp -> tty_notifier);
	}
    }
    notifierskilled++;
    (void) alarm (5);    
}


/*
 * Collect all terminal notification processes, sending a kill if
 * necessary in case someone has left their terminal with output blocked.
 */
collectnotifiers ()
{
    union wait status;
    int     child;
    struct ttyinfo *tp;

    if (notifiers == 0)
	return;

    dprintf0 ("Collecting notifiers\n");
    notifierskilled = 0;

    (void) signal (SIGALRM, killnotifiers);
    (void) alarm (5);

    if (setjmp (timeout) == 0)
    {
	while (notifiers > 0)
	{
	    child = wait (&status);
	    dprintf2 ("notifier %d status %x\n", child, status);
	    for (tp = ttytable; tp < &ttytable[nttys]; tp++)
	    {
		if (child == tp -> tty_notifier)
		{
		    tp -> tty_notifier = 0;
		    notifiers--;
		}
	    }
	}
    }
    else
	dprintf1 ("Abandoned wait for %d notifiers\n", notifiers);

    notifiers = 0;
    (void) alarm (0);
    (void) signal (SIGALRM, SIG_IGN);
}


/*
 * (In a child process ...) Send a message to a terminal, then exit.
 */
notifier (line, message, arg)
char *line, *message;
int arg;
{
    FILE * tty;
    char    hostname[20];

    /* 4.2 lint library bug ... */
    (void) gethostname (hostname, sizeof (hostname));
    tty = fopen (line, "w");

    if (tty != NULL)
    {
	if (debug)
	    fprintf (tty, "**** Testing idledaemon: PLEASE IGNORE!\r\n\r\n");
	fprintf (tty, "**** \007\007Message from %s.idledaemon at %8.8s\r\n\r\n**** ",
		hostname, ctime (&timenow) + 11);
	fprintf (tty, message, arg);
	fputs ("\r\n", tty);
	(void) fclose (tty);
    }
    else
	dprintf2 ("notifier failed to open %s ... errno %d\n", line, errno);
    (void) fflush (stderr);
    exit (0);
}


displaysig (sig, x, y)
{
    time_t sigtime;

    time (&sigtime);
    dprintf2 ("Signal %d caught at %s", sig, ctime(&sigtime));
    fflush (stderr);
    exit (1);
}


/*
 *  Signal routine to display tty state tables on standard output.
 */
printstate (sig, x, y)
{
    time_t sigtime;
    register i;
    register struct ttyinfo *tp;

    sleep (1);			/* time for the next shell prompt ... */
    time (&sigtime);
    printf ("\nIdledaemon terminal info: time %25.25s", ctime(&sigtime));
    printf ("%d terminals out of %d are free\n", nttysfree, nttys);
    printf (
"tty     state   user    login time        last access       warning sent\n");
    for (i = 0, tp = ttytable; i < nttys; i++, tp++)
    {
	printf ("%s\t%s", tp -> tty_line, statestrings[tp -> tty_state]);
	if (tp -> tty_state == TTY_FREE || tp -> tty_state == TTY_UNKNOWN)
	{
	    putchar ('\n');
	    continue;
	}
        printf ("\t%s", tp -> tty_name);
	printf ("\t%15.15s", ctime (&(tp -> tty_logintime)) + 4);
	if (tp -> tty_state != TTY_ACTIVE)
	{
	    printf ("   %15.15s", ctime (&(tp -> tty_accesstime)) + 4);
	    if (tp -> tty_state == TTY_WARNED)
	        printf ("   %15.15s", ctime (&(tp -> tty_warningtime)) + 4);
	}
	putchar ('\n');
    }
    fflush (stdout);
}


/*
 *  Set up signal handlers.
 */
init_signals ()
{
    register i;
    for (i = 1; i <= SIGPROF; i++)
    {
	if (i != SIGCONT && i != SIGCHLD && i != SIGHUP && i != SIGINT)
	    (void) signal (i, displaysig);
    }
    signal (SIGHUP, SIG_IGN);
    signal (SIGINT, printstate);
}
