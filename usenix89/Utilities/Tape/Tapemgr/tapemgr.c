#ifndef lint
static char rcsid[] =
            "$Header: tapemgr.c,v 2.3 86/08/15 13:19:39 thompson Exp $";
#endif
/*
 *
 *			T A P E M G R
 *
 * AUTHOR:  Written by Michael A . Thompson at Dalhousie University
 *	Department of Mathematics, Statistics and Computing Science.
 *
 * PURPOSE: To restrict access to tapedrives to one user at a time.
 *	See tapemgr.1 for details of usage.
 *
 * NOTICE: This is free software, do with it what you will.
 *	
 */
 
/*
 *	$Locker:  $
 *	$Source: /usr/src/local/tapemgr/RCS/tapemgr.c,v $
 *
 * $Log:	tapemgr.c,v $
 * Revision 2.3  86/08/15  13:19:39  thompson
 * shorter logmessage
 * 
 * Revision 2.2  86/04/08  04:27:06  thompson
 * fix comments.
 * 
 * Revision 2.1  86/03/25  11:50:59  thompson
 * author info.
 * 
 * Revision 2.0  86/03/24  23:50:30  thompson
 * First, release.
 * 
 * Revision 1.1  86/03/22  23:23:43  thompson
 * Initial revision
 * 
 */
#include	<ctype.h>
#include	<pwd.h>
#include	<stdio.h>
#include	<strings.h>
#include	<sys/ioctl.h>
#include	<sys/param.h>
#include	<sys/mtio.h>
#include	<sys/stat.h>
#include	<sys/time.h>
#include	<sys/wait.h>

#define RESTORE_TERMINAL	/* Define this if you want the terminal
				   state restored upon termination */
#define KERNAL_PAGER		/* Define this if you have the kernal
				   pager installed on you system
				   <2037@utcsstat.UUCP> */

#define MAX_DRIVES	4	/* The maximum number of drives that your
				   system is designed to support. */
#define NUM_DRIVES	1	/* The number of drives that you system
				   actually has. */
#define MAX_RETRIES	5	/* How many times the program will attempt
				   to allocate the requested number of
				   drives after finding that no more are
				   currently available (NOTE: it sleeps
				   for SLEEP3 seconds between attempts */

#ifndef DEBUG
#define SLEEP1		1	/* Short sleep */
#define SLEEP2		5	/* Med. sleep */
#define SLEEP3		120	/* Long sleep */
#define TIME_OUT	(60 * 60 * 6)
				/* 6 hours -- how long before timeout
				   occurs */
#define WARNING		(60 * 5)/* 5 minutes -- how long after timeout
				   occures before everything is actually
				   shutdown */
#else DEBUG
#define SLEEP1		1
#define SLEEP2		5
#define SLEEP3		5
#define TIME_OUT	(45)
#define WARNING		(15)
#endif DEBUG


/* the following defines calculate the various device numbers for a
   given drive x: AR indicates auto-rewind on close, nnnBPI indicates the
   number of bit per inch. */
#define AR_800BPI(x)	(0 + (x - 1))
#define NAR_800BPI(x)	(AR_800BPI(x) + MAX_DRIVES)
#define AR_1600BPI(x)	(NAR_800BPI(x) + MAX_DRIVES)
#define NAR_1600BPI(x)	(AR_1600BPI(x) + MAX_DRIVES)
#define AR_6250BPI(x)	(NAR_1600BPI(x) + MAX_DRIVES)
#define NAR_6250BPI(x)	(AR_6250BPI(x) + MAX_DRIVES)

#define MT_NAME		"/dev/mt"/* prefix for the block devices */
#define RMT_NAME	"/dev/rmt"/* prefix for the raw devices */

#define ROOT		0	/* uid of root */
#define TAPEMGR_UID	30	/* uid of the user tapemgr */
#define TAPEMGR_GID	300	/* gid of the user tapemgr */

#define DRIVE_PROT	0600	/* the protection modes for the tape
				   devices */

#define SHELL		"/bin/sh"/* the default shell */

#ifndef DEBUG
#define LOGFILE		"/usr/adm/tapelog"
				/* where the log is kept, if this file
				   does not exist no log is kept */
#else DEBUG
#define LOGFILE		"tapelog"
#endif

#define ENV_VAR		"TAPE_DRIVES"
				/* Environment variable to inform children
				   about which drives are available */
#define ENV_VAR_EQ	"TAPE_DRIVES="
				/* This should be the same as ENV_VAR with
				   an = tagged on */

#define NO_SIGNAL	EOF	/* these are just used as fill for when
				   clean_up is called directly rather than
				   by an interupt */
#define NO_CODE		EOF
#define NO_CONTEXT	((struct sigcontext *) EOF)

#define TRUE		(logical) '\1'
#define FALSE		(logical) '\0'
typedef char    logical;

struct mtop mtoffl = {		/* tape operation offline */
    MTOFFL, 1
};

int     otpgrp;			/* for storing and restoring the state of
				   the terminal */
#ifdef RESTORE_TERMINAL
int     oldisc;
struct sgttyb   otty;
struct tchars   otchars;
#ifdef KERNAL_PAGER
struct tpage    otpage;
#endif KERNAL_PAGER
int     olocalm;
struct ltchars  oltchars;
#endif RESTORE_TERMINAL

int     fd_tty = EOF;		/* equal to the fd of the terminal */

struct sigvec   ovec;
struct sigvec   vec_dfl = {	/* to restore the original signal handler 
				*/
    SIG_DFL, 0, 0
};
int     kill_and_clean (), clean_up (), timeout ();
struct sigvec   vec_kill_and_clean = {
				/* to cause kill_and_clean to be called
				   when a signal is received */
    kill_and_clean, 0, 0
};
struct sigvec   vec_clean_up = {/* to cause clean_up to be called when a
				   signal is received */
    clean_up, 0, 0
};
struct sigvec   vec_timeout = {	/* to cause timeout to be called when a
				   signal is received */
    timeout, 0, 0
};

struct itimerval    time_out,	/* the values for the interval timer */
                    ovalue;

int     pid;			/* the pid of the child */

char  **tapemgr_env = 0;	/* the environment to be passed to the
				   child */

logical allocated[MAX_DRIVES + 1];/* which drives have been allocated
				   allocated[0] == TRUE means that reset
				   is being performed and no allocations
				   are to be performed */

logical verbose = TRUE;		/* TRUE for a mouthy program */

char   *getenv (), *getlogin (), *malloc (), *realloc (), *calloc ();

/*---------------------------------------------------------------------------
 * 
 *			P R I V I L E G E D _ U S E R
 *
 *	This program returns TRUE if the user who is running it is root or
 * is in the group tapemgr.
 *
 *      Entry Conditions:        none.
 *
 *      Exit Conditions:         once the status of the person running the
 *				 program has been determined.
 *
 *      Returns:                 TRUE iff the user is privileged
 *				 FALSE otherwise.
 *
 *      Side Effects:            none.
 *
 *      Static Variables:        first_time, gidset, ngrps, cur_grp,
 *				 privileged
 *
 *      External Variables:      none.
 *
 *      Non-library Functions:   none.
 *
 *---------------------------------------------------------------------------
 */
logical privileged_user () {
    static  logical first_time = TRUE;
    static int  gidset[NGROUPS],
                ngrps,
                cur_grp;
    static  logical privileged = FALSE;

    if (first_time && !(privileged = getuid () == ROOT)) {
	ngrps = getgroups (NGROUPS, gidset);
	first_time = FALSE;
	for (cur_grp = 0; cur_grp < ngrps && !privileged; cur_grp++)
	    privileged = gidset[cur_grp] == TAPEMGR_GID;
    }
    return privileged;
}

/*---------------------------------------------------------------------------
 * 
 *			L O G M E S G
 *
 *	Log an action in the tapelog file if it exists.
 *
 *      Entry Conditions:        none.
 *
 *      Exit Conditions:         none.
 *
 *      Returns:                 nothing.
 *
 *      Side Effects:            none.
 *
 *      Static Variables:        none.
 *
 *      External Variables:      none.
 *
 *      Non-library Functions:   none.
 *
 *---------------------------------------------------------------------------
 */
void logmesg (action, drive)
char   *action;
int     drive;
{
    FILE * logfile;
    struct stat logfile_stat;
    struct timeval  tp;
    struct timezone tzp;
    struct passwd  *pwdent;

    if (stat (LOGFILE, &logfile_stat))
#ifndef DEBUG
	    return;
#else DEBUG
	    perror (LOGFILE);
#endif DEBUG
    (void) gettimeofday (&tp, &tzp);
    pwdent = getpwuid (getuid ());
    if (!(logfile = fopen (LOGFILE, "a")))
	return;
    fprintf (logfile,
#ifndef DEBUG
	    "%d,%s,%s/%s(%d),%s",
#else DEBUG
	    "drive=%d,action=%s,login/user(uid)=%s/%s(%d),date and time=%s",
#endif DEBUG
	    drive - 1, action, getlogin (), pwdent -> pw_name,
	    getuid (), ctime ((time_t *) & tp.tv_sec));
    (void) fclose (logfile);
}

/*---------------------------------------------------------------------------
 * 
 *			K I L L _ A N D _ C L E A N
 *
 *	Kill the children.
 *
 *      Entry Conditions:        SIGALRM
 *
 *      Exit Conditions:         all children killed.
 *
 *      Returns:                 nothing.
 *
 *      Side Effects:            resets the handler for SIGALRM and turns off
 *				 the interval timer.
 *
 *      Static Variables:        none.
 *
 *      External Variables:      pid, vec_dfl, verbose, otpgrp.
 *
 *      Non-library Functions:   logmesg.
 *
 *---------------------------------------------------------------------------
 */
int     kill_and_clean (sig, code, scp)
int     sig,
        code;
struct sigcontext  *scp;
{
    int     cur_tpgrp,
            prev_tpgrp;

    (void) setitimer (ITIMER_REAL, &ovalue, &time_out);
    (void) sigvec (SIGALRM, &vec_dfl, &ovec);
    if (pid) {
	if (sig == SIGALRM) {
	    logmesg ("Timeout", 0);
	    fprintf (stderr, "Timeout has arrived!!!!\007\n");
	}
	else {
	    logmesg ("Killed", 0);
	    fprintf (stderr, "I been shot jim!!!\007\n");
	}
	if (fd_tty != EOF) {
	    if (verbose) {
		printf ("Take That(pid=%d)", pid);
		(void) fflush (stdout);
	    }
	    (void) kill (pid, SIGHUP);
	    sleep (SLEEP2);
	    (void) kill (pid, SIGKILL);
	    sleep (SLEEP2);
	    (void) ioctl (fd_tty, (int) TIOCGPGRP, (char *) & cur_tpgrp);
	    for (prev_tpgrp = EOF;
		    cur_tpgrp != prev_tpgrp && cur_tpgrp != otpgrp;) {
		if (verbose) {
		    printf ("...and That(pgrp=%d)", cur_tpgrp);
		    (void) fflush (stdout);
		}
		(void) killpg (cur_tpgrp, SIGHUP);
		sleep (SLEEP2);
		(void) killpg (cur_tpgrp, SIGKILL);
		prev_tpgrp = cur_tpgrp;
		sleep (SLEEP2);
		(void) ioctl (fd_tty, (int) TIOCGPGRP, (char *) & cur_tpgrp);
	    }
	    if (verbose)
		printf ("...I think it's dead now.\n");
	}
    }
}

/*---------------------------------------------------------------------------
 * 
 *			T I M E O U T
 *
 *	nothing much really, just sets things up for the kill.
 *
 *      Entry Conditions:        SIGALRM
 *
 *      Exit Conditions:         none.
 *
 *      Returns:                 nothing.
 *
 *      Side Effects:            sets the handler for SIGALRM to
 *				 kill_and_clean.
 *
 *      Static Variables:        none.
 *
 *      External Variables:      vec_kill_and_clean.
 *
 *      Non-library Functions:   none.
 *
 *---------------------------------------------------------------------------
 */
int     timeout (sig, code, scp)
int     sig,
        code;
struct sigcontext  *scp;
{
    fprintf (stderr,
	    "You have %d minutes %d seconds in which to clean up, timeout has arrived.\007\n",
	    (int) WARNING / 60, WARNING % 60);
    (void) sigvec (SIGALRM, &vec_kill_and_clean, &ovec);
}

/*---------------------------------------------------------------------------
 * 
 *			S E T _ D E V I C E
 *	Sets the ownership and protection of the specified device, if chutzpah
 * is set then it just pretends to set the ownership and protection.
 *
 *      Entry Conditions:        none.
 *
 *      Exit Conditions:         none.
 *
 *      Returns:                 nothing.
 *
 *      Side Effects:            none.
 *
 *      Static Variables:        none.
 *
 *      External Variables:      verbose.
 *
 *      Non-library Functions:   none.
 *
 *---------------------------------------------------------------------------
 */
void set_device (drive, uid, silently, chutzpah)
char   *drive;
int     uid;
logical silently, chutzpah;
{
    struct stat drive_stat;

    if (stat (drive, &drive_stat)) {
	if (!silently || chutzpah)
	    if (verbose)
		printf ("%15s", "");
	return;
    }
    if (!chutzpah) {
	(void) chown (drive, uid, TAPEMGR_GID);
	(void) chmod (drive, DRIVE_PROT);
    }
    if (!silently || chutzpah)
	if (verbose)
	    printf ("%15s", drive);
}

/*---------------------------------------------------------------------------
 * 
 *			S E T _ D R I V E
 *
 *	Runs set_device for every device for a given drive, except for
 * the raw no auto rewind 800bpi device which is run with chutzpah TRUE
 * it is left up to the calling routine to run set_device for that device.
 *
 *      Entry Conditions:        none.
 *
 *      Exit Conditions:         none.
 *
 *      Returns:                 nothing.
 *
 *      Side Effects:            none.
 *
 *      Static Variables:        none.
 *
 *      External Variables:      verbose.
 *
 *      Non-library Functions:   set_device.
 *
 *---------------------------------------------------------------------------
 */
void set_drive (drive, uid)
int     drive,
        uid;
{
    char    cur_drive[BUFSIZ];

    if (verbose) {
	printf ("BLOCK Devices available with drive %d.\n", drive - 1);
	printf ("%15s%15s%15s%15s\n", "", "800bpi", "1600bpi", "6250bpi");
	printf ("%15s", "Auto Rewind:");
    }
    set_device (sprintf (cur_drive, "%s%d", MT_NAME, AR_800BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", MT_NAME, AR_1600BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", MT_NAME, AR_6250BPI (drive)),
	    uid, FALSE, FALSE);
    if (verbose)
	printf ("\n%15s", "No Auto Rewind:");
    set_device (sprintf (cur_drive, "%s%d", MT_NAME, NAR_800BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", MT_NAME, NAR_1600BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", MT_NAME, NAR_6250BPI (drive)),
	    uid, FALSE, FALSE);
    if (verbose) {
	printf ("\nRAW Devices available with drive %d.\n", drive - 1);
	printf ("%15s%15s%15s%15s\n", "", "800bpi", "1600bpi", "6250bpi");
	printf ("%15s", "Auto Rewind:");
    }
    set_device (sprintf (cur_drive, "%s%d", RMT_NAME, AR_800BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", RMT_NAME, AR_1600BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", RMT_NAME, AR_6250BPI (drive)),
	    uid, FALSE, FALSE);
    if (verbose)
	printf ("\n%15s", "No Auto Rewind:");
    set_device (sprintf (cur_drive, "%s%d", RMT_NAME, NAR_800BPI (drive)),
	    uid, FALSE, TRUE);
    set_device (sprintf (cur_drive, "%s%d", RMT_NAME, NAR_1600BPI (drive)),
	    uid, FALSE, FALSE);
    set_device (sprintf (cur_drive, "%s%d", RMT_NAME, NAR_6250BPI (drive)),
	    uid, FALSE, FALSE);
    if (verbose)
	printf ("\n");
}

/*---------------------------------------------------------------------------
 * 
 *			C L E A N _ U P
 *
 *	restore the terminal, and reset all the allocated drives.
 *
 *      Entry Conditions:        SIGINT or called from the main program.
 *
 *      Exit Conditions:         all allocated drives released.
 *
 *      Returns:                 never.
 *
 *      Side Effects:            blocks all signals
 *
 *      Static Variables:        none.
 *
 *      External Variables:      verbose, allocated, mtoffl, oldisc, otty,
 *				 otpgrp, otchars, otpage, olocalm, oltchars.
 *
 *      Non-library Functions:   priviliged_user, logmesg.
 *
 *---------------------------------------------------------------------------
 */
int     clean_up (sig, code, scp)
int     sig,
        code;
struct sigcontext  *scp;
{
    FILE * fdrive;
    struct stat drive_stat;
    int     drive;
    char    cur_drive[BUFSIZ];

    (void) sigsetmask (~0);
    if (verbose)
	printf ("Cleaning up...\n");
    for (drive = 1; drive <= NUM_DRIVES; drive++)
	if (allocated[drive]) {
	    (void) sprintf (cur_drive, "%s%d", RMT_NAME, NAR_800BPI (drive));
	    if (stat (cur_drive, &drive_stat)) {
		perror (cur_drive);
		continue;
	    }
	    if (privileged_user () || getuid () == drive_stat.st_uid) {
		if (verbose)
		    printf ("Reseting drive %d.\n", drive - 1);
		if ((fdrive = fopen (cur_drive, "r"))) {
		    (void) ioctl (fileno (fdrive), (int) MTIOCTOP,
			    (char *) & mtoffl);
		    (void) fclose (fdrive);
		}
		set_drive (drive, TAPEMGR_UID);
		set_device (cur_drive, TAPEMGR_UID, TRUE, FALSE);
		logmesg ("Reset", drive);
	    }
	}
#ifdef RESTORE_TERMINAL
    if (fd_tty != EOF) {
	(void) ioctl (fd_tty, (int) TIOCSETD, (char *) & oldisc);
	(void) ioctl (fd_tty, (int) TIOCSETP, (char *) & otty);
	(void) ioctl (fd_tty, (int) TIOCSPGRP, (char *) & otpgrp);
	(void) ioctl (fd_tty, (int) TIOCSETC, (char *) & otchars);
#ifdef KERNAL_PAGER
	(void) ioctl (fd_tty, (int) TIOCSPAG, (char *) & otpage);
#endif KERNAL_PAGER
	(void) ioctl (fd_tty, (int) TIOCLSET, (char *) & olocalm);
	(void) ioctl (fd_tty, (int) TIOCSLTC, (char *) & oltchars);
    }
#endif RESTORE_TERMINAL
    exit (0);
}

/*---------------------------------------------------------------------------
 * 
 *			S E T _ E N V
 *
 *	Adds the specified drive to the environment variable ENV_VAR
 *
 *      Entry Conditions:        none.
 *
 *      Exit Conditions:         none.
 *
 *      Returns:                 nothing.
 *
 *      Side Effects:            modifies tapemgr_env.
 *
 *      Static Variables:        first_time, space.
 *
 *      External Variables:      none.
 *
 *      Non-library Functions:   none.
 *
 *---------------------------------------------------------------------------
 */
void set_env (drive, envp)
int     drive;
char  **envp;
{
    static  logical first_time = TRUE;
    static  logical space = TRUE;
    unsigned int    count;
    char  **tenvp,
          **nenvp,
            env_buf[BUFSIZ];

    if (!tapemgr_env)
	if (getenv (ENV_VAR))
	    tapemgr_env = envp;
	else {
	    for (count = 2, tenvp = envp; *tenvp; tenvp++, count++);
	    tapemgr_env = (char **) calloc (count, sizeof (char *));
	    for (tenvp = envp, nenvp = tapemgr_env; *tenvp;
		    *(nenvp++) = *(tenvp++));
	    *nenvp = ENV_VAR_EQ;
	    *++nenvp = NULL;
	    space = FALSE;
	}
    for (nenvp = tapemgr_env;
	    strncmp (*nenvp, ENV_VAR_EQ, strlen (ENV_VAR_EQ)); nenvp++);
    if (space)
	(void) sprintf (env_buf, "%s %d", *nenvp, drive);
    else {
	(void) sprintf (env_buf, "%s%d", *nenvp, drive);
	space = TRUE;
    }
    if (first_time) {
	*nenvp = strcpy (malloc ((unsigned int) strlen (env_buf) + 1),
		env_buf);
	first_time = FALSE;
    }
    else
	*nenvp = strcpy (realloc (*nenvp,
		    (unsigned int) strlen (env_buf) + 1),
		env_buf);
}

main (argc, argv, envp)
int     argc;
char  **argv,
      **envp;
{
    struct stat drive_stat;
    union wait status;
    int     drive,
            num_allocated,
            num_optarg,
            option,
            org_sigmask;
    int     num_drives = 1;
    int     retries = 0;
    char   *called = *argv;
    char    cur_drive[BUFSIZ];
    char   *tape_command = getenv ("SHELL") ? getenv ("SHELL") : SHELL;
    extern char *optarg;
    logical one_allocated;
    logical t_out = TRUE;

    if (geteuid () != ROOT) {
	fprintf (stderr,
		"This program must be suid to root for it to work.\n");
	exit (13);
    }
    while ((option = getopt (argc, argv, "r:n:c:vt")) != EOF)
	switch (option) {
	    case 'r': 
		if (!strcmp (optarg, "all")) {
		    fprintf (stderr, "Reseting all drives\n");
		    for (drive = 0; drive <= NUM_DRIVES; drive++)
			allocated[drive] = TRUE;
		}
		else {
		    if (!isdigit (optarg[0])) {
			fprintf (stderr,
				"The -r option expects a numeric argument in the range (0,%d)%s",
				NUM_DRIVES - 1, "\n\tor the string all.\n");
			exit (13);
		    }
		    num_optarg = atoi (optarg);
		    if (num_optarg < 0 || num_optarg > NUM_DRIVES - 1) {
			fprintf (stderr,
				"%d is not a valid drive number, valid drive numbers are in the range (0,%d).\n",
				num_optarg, NUM_DRIVES - 1);
			exit (13);
		    }
		    allocated[0] = TRUE;
		    allocated[num_optarg + 1] = TRUE;
		}
		break;
	    case 'n': 
		if (!strcmp (optarg, "all")) {
		    fprintf (stderr, "Allocating all drives\n");
		    num_drives = NUM_DRIVES;
		}
		else {
		    if (!isdigit (optarg[0])) {
			fprintf (stderr,
				"The -n option expects a numeric argument in the range (1,%d)%s",
				NUM_DRIVES, "\n\tor the string all.\n");
			exit (13);
		    }
		    num_optarg = atoi (optarg);
		    if (num_optarg < 1 || num_optarg > NUM_DRIVES) {
			fprintf (stderr,
				"%d is not a valid number of drives,\n\tvalid numbers of drives are in the range (1,%d).\n",
				num_optarg, NUM_DRIVES);
			exit (13);
		    }
		    num_drives = num_optarg;
		}
		break;
	    case 'c': 
		tape_command = optarg;
		break;
	    case 'v': 
		verbose = !verbose;
		break;
	    case 't': 
		if (!privileged_user ())
		    fprintf (stderr,
			    "You have to be root or belong the the group tapemgr to use the -t option.\n");
		else
		    t_out = !t_out;
		break;
	    case '?': 
	    default: 
		fprintf (stderr,
			"Usage: %s [-r DRIVE_NUMBER|all] | [-n NUMBER_OF_DRIVES|all]%s",
			called, "\n\t[-c TAPE_COMMAND] [-v] [-t]\n");
		exit (13);
		break;
	}
    if (!allocated[0]) {	/* oh boy, we get some drives */
	(void) sigvec (SIGINT, &vec_clean_up, &ovec);
				/* clean up after ^C */
	org_sigmask = sigsetmask (~(1 << (SIGINT - 1)));
	if (verbose)
	    printf ("Attempting to allocate %d drives.\n", num_drives);
	for (num_allocated = 0; num_allocated < num_drives;) {
	    for (drive = 1, one_allocated = FALSE;
		    drive <= NUM_DRIVES && !one_allocated; drive++) {
		if (allocated[drive])
		    continue;
		(void) sprintf (cur_drive,
			"%s%d", RMT_NAME, NAR_800BPI (drive));
		if (stat (cur_drive, &drive_stat)) {
		    perror (cur_drive);
		    continue;
		}
		if (drive_stat.st_uid != TAPEMGR_UID)
		    continue;
		allocated[drive] = TRUE;
		fprintf (stderr, "Allocating drive %d.\n", drive - 1);
		set_device (cur_drive, getuid (), TRUE, FALSE);
		sleep (SLEEP1);
		(void) stat (cur_drive, &drive_stat);
		if (drive_stat.st_uid != getuid ()) {
		    allocated[drive] = FALSE;
		    fprintf (stderr,
			    "Thought we had drive %d but I guess I was wrong.\n",
			    drive - 1);
		    continue;
		}
		set_drive (drive, getuid ());
		set_env (drive - 1, envp);
		one_allocated = TRUE;
		num_allocated++;
		logmesg ("Allocation", drive);
	    }
	    if (!one_allocated) {
		if (retries > MAX_RETRIES) {
		    fprintf (stderr,
			    "Could not allocate the requested number of drives.\n");
		    clean_up (NO_SIGNAL, NO_CODE, NO_CONTEXT);
		}
		else {
		    if (verbose)
			printf ("Retrying...snore for %d secs", SLEEP3);
		    sleep (SLEEP3);
		    if (verbose)
			printf ("...yawn let's try again.\n");
		    retries++;
		}
	    }
	}
	(void) sigsetmask (~0);
	if (isatty (fileno (stderr)))/* are we attached to a terminal? */
	    fd_tty = fileno (stderr);
	else
	    if (isatty (fileno (stdout)))
		fd_tty = fileno (stdout);
	    else
		if (isatty (fileno (stdin)))
		    fd_tty = fileno (stdin);
	if (fd_tty != EOF) {
	    (void) ioctl (fd_tty, (int) TIOCGPGRP, (char *) & otpgrp);
#ifdef RESTORE_TERMINAL
	    (void) ioctl (fd_tty, (int) TIOCGETD, (char *) & oldisc);
	    (void) ioctl (fd_tty, (int) TIOCGETP, (char *) & otty);
	    (void) ioctl (fd_tty, (int) TIOCGETC, (char *) & otchars);
#ifdef KERNAL_PAGER
	    (void) ioctl (fd_tty, (int) TIOCGPAG, (char *) & otpage);
#endif KERNAL_PAGER
	    (void) ioctl (fd_tty, (int) TIOCLGET, (char *) & olocalm);
	    (void) ioctl (fd_tty, (int) TIOCGLTC, (char *) & oltchars);
#endif RESTORE_TERMINAL
	}
	if ((pid = fork ()) == EOF)
	    perror (called);
	else
	    if (pid) {
		if (t_out) {
		    time_out.it_value.tv_sec = TIME_OUT;
		    time_out.it_interval.tv_sec = WARNING;
		    (void) sigvec (SIGALRM, &vec_timeout, &ovec);
		    sleep (SLEEP1);
		    if (verbose)
			printf ("Timeout in %d hours %d minutes %d seconds.\n",
				(int) TIME_OUT / 3600,
				(int) (TIME_OUT % 3600) / 60, TIME_OUT % 60);
		    (void) setitimer (ITIMER_REAL, &time_out, &ovalue);
		}
		(void) sigvec (SIGHUP, &vec_kill_and_clean, &ovec);
		(void) sigsetmask (~((1 << (SIGHUP - 1)) |
			    ((t_out ? 1 : 0) << (SIGALRM - 1))));
		(void) wait (&status);
		pid = 0;
		if (t_out) {
		    (void) setitimer (ITIMER_REAL, &ovalue, &time_out);
		    (void) sigvec (SIGALRM, &vec_dfl, &ovec);
		}
	    }
	    else {
		if (verbose)
		    printf ("Starting up: `%s'\n", tape_command);
		(void) setuid (getuid ());
		(void) sigsetmask (org_sigmask);
		(void) sigblock (1 << (SIGINT - 1));
		(void) sigblock (1 << (SIGQUIT - 1));
		(void) sigblock (1 << (SIGTSTP - 1));
		(void) execle (SHELL, SHELL, "-c", tape_command, 0,
			tapemgr_env);
		perror (tape_command);
		exit (13);
	    }
    }
    clean_up (NO_SIGNAL, NO_CODE, NO_CONTEXT);
}
