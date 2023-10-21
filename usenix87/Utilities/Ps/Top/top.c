char *copyright =
    "Top, version 2.1, copyright (c) 1984, 1986, William LeFebvre";

/*
 *  Top users display for Berkeley Unix
 *  Version 2.1
 *
 *  This program may be freely redistributed to other Unix sites, but this
 *  entire comment MUST remain intact.
 *
 *  Copyright (c) 1984, 1986, William LeFebvre, Rice University
 *
 *  This program is designed to run on either Berkeley 4.1 or 4.2 Unix.
 *  Compile with the preprocessor constant "FOUR_ONE" set to get an
 *  executable that will run on Berkeley 4.1 Unix.
 *
 *  The Sun kernel uses scaled integers instead of floating point.
 *  Compilation with the preprocessor variable "sun" gets an executable
 *  that will run on Sun Unix version 1.1 or later ("sun" is automatically
 *  set by the Sun C compiler).
 *
 *  The Pyramid splits the stack size (p_ssize) into control stack and user
 *  stack sizes.  Compilation with the preprocessor variable "pyr" gets an
 *  executable that will run on Pyramids ("pyr" is automatically set by the
 *  Pyramid C compiler).
 *
 *  See the file "Changes" for more information on version-to-version changes.
 */

#include <stdio.h>
#include <pwd.h>
#include <nlist.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/dk.h>
#include <sys/vm.h>

/* includes specific to top */
#include "layout.h"
#include "screen.h"		/* interface to screen package */
#include "top.h"
#include "top.local.h"
#include "boolean.h"

/* Special atoi routine returns either a non-negative number or one of: */
#define Infinity	-1
#define Invalid		-2

/* Size of the stdio buffer given to stdout */
#define Buffersize	2048

/* The buffer the stdio will use */
char stdoutbuf[Buffersize];

/* wish list for kernel symbols */

struct nlist nlst[] = {
    { "_avenrun" },
#define X_AVENRUN	0
    { "_ccpu" },
#define X_CCPU		1
    { "_cp_time" },
#define X_CP_TIME	2
    { "_hz" },
#define X_HZ		3
    { "_mpid" },
#define X_MPID		4
    { "_nproc" },
#define X_NPROC		5
    { "_proc" },
#define X_PROC		6
    { "_total" },
#define X_TOTAL		7
    { 0 },
};

/* build Signal masks */
#define Smask(s)	(1 << ((s) - 1))

/* for system errors */
extern int errno;

/* for getopt: */
extern int  optind;
extern char *optarg;

/* signal handling routines */
int leave();
int onalrm();
int tstop();

int nproc;
int mpid;

/* kernel "hz" variable -- clock rate */
long hz;

/* All this is to calculate the cpu state percentages */

long cp_time[CPUSTATES];
long cp_old[CPUSTATES];
long cp_change[CPUSTATES];
long total_change;
long mpid_offset;
long avenrun_offset;
long cp_time_offset;
long total_offset;

#ifdef sun
long ccpu;
long avenrun[3];
#else
double ccpu;
double avenrun[3];
#endif
double logcpu;

struct vmtotal total;

struct proc *proc;
struct proc *pbase;
int bytes;
int initialized = No;
char *myname = "top";
jmp_buf jmp_int;
char input_waiting = No;

/* routines that don't return int */

struct passwd *getpwent();
char *username();
char *itoa();
char *ctime();
char *rindex();
char *kill_procs();
char *renice_procs();

int proc_compar();
long time();

/* different routines for displaying the user's identification */
/* (values assigned to get_userid) */
char *username();
char *itoa7();

/* display routines that need to be predeclared */
int i_loadave();
int u_loadave();
int i_procstates();
int u_procstates();
int i_cpustates();
int u_cpustates();
int i_memory();
int u_memory();
int i_header();
int u_header();
int i_process();
int u_process();

/* pointers to display routines */
int (*d_loadave)() = i_loadave;
int (*d_procstates)() = i_procstates;
int (*d_cpustates)() = i_cpustates;
int (*d_memory)() = i_memory;
int (*d_header)() = i_header;
int (*d_process)() = i_process;

/* buffer of proc information lines for display updating */
/* unfortunate that this must be declared globally */
char (* screenbuf)[Display_width];

main(argc, argv)

int  argc;
char *argv[];

{
    register struct proc *pp;
    register struct proc **prefp;
    register int i;
    register int active_procs;
    register int change;

    static struct proc **pref;
    static char tempbuf1[50];
    static char tempbuf2[50];
    int total_procs;
    int old_sigmask;
    int proc_brkdn[7];
    int topn = Default_TOPN;
    int delay = Default_DELAY;
    int displays = 0;		/* indicates unspecified */
    long curr_time;
    char *(*get_userid)() = username;
    char *uname_field = "USERNAME";
#ifndef FOUR_ONE
    char ch;
    char msg_showing = 0;
    int readfds;
    struct timeval timeout;
#endif    
    char dostates = No;
    char do_unames = Yes;
    char do_init = No;
    char interactive = Maybe;
    char show_sysprocs = No;
    char topn_specified = No;
    char warnings = 0;

    /* set the buffer for stdout */
    setbuffer(stdout, stdoutbuf, Buffersize);

    /* get our name */
    if (argc > 0)
    {
	if ((myname = rindex(argv[0], '/')) == 0)
	{
	    myname = argv[0];
	}
	else
	{
	    myname++;
	}
    }

    /* process options */
    while ((i = getopt(argc, argv, "Sbinus:d:")) != EOF)
    {
	switch(i)
	{
	    case 'u':			/* display uid instead of name */
		do_unames = No;
		uname_field = "   UID  ";
		get_userid = itoa7;
		break;

	    case 'S':			/* show system processes */
		show_sysprocs = Yes;
		break;

	    case 'i':			/* go interactive regardless */
		interactive = Yes;
		break;

	    case 'n':			/* batch, or non-interactive */
	    case 'b':
		interactive = No;
		break;

	    case 'd':			/* number of displays to show */
		if ((i = atoiwi(optarg)) == Invalid || i == 0)
		{
		    fprintf(stderr,
			"%s: warning: display count should be positive -- option ignored\n",
			myname);
		    warnings++;
		}
		else
		{
		    displays = i;
		}
		break;

	    case 's':
		if ((delay = atoi(optarg)) < 0)
		{
		    fprintf(stderr,
			"%s: warning: seconds delay should be non-negative -- using default\n",
			myname);
		    delay = Default_DELAY;
		    warnings++;
		}
		break;

	    default:
		fprintf(stderr,
		    "Usage: %s [-Sbinu] [-d x] [-s x] [number]\n",
		    myname);
		exit(1);
	}
    }

    /* get count of top processes to display (if any) */
    if (optind < argc)
    {
	if ((topn = atoiwi(argv[optind])) == Invalid)
	{
	    fprintf(stderr,
		"%s: warning: process display count should be non-negative -- using default\n",
		myname);
	    topn = Default_TOPN;
	    warnings++;
	}
	else
	{
	    topn_specified = Yes;
	}
    }

    /* initialize the kernel memory interface */
    init_kernel();

    if (initialized != 1)
    {
	/* get the list of symbols we want to access in the kernel */
	/* errno = 0; ??? */
	nlist(VMUNIX, nlst);
	if (nlst[0].n_type == 0)
	{
	    fprintf(stderr, "%s: can't nlist image\n", VMUNIX);
	    exit(2);
	}
    
	/* get the symbol values out of kmem */
	getkval(nlst[X_PROC].n_value,  &proc,  sizeof(int),
		nlst[X_PROC].n_name);
	getkval(nlst[X_NPROC].n_value, &nproc, sizeof(int),
		nlst[X_NPROC].n_name);
	getkval(nlst[X_HZ].n_value,    &hz,    sizeof(int),
		nlst[X_HZ].n_name);
	getkval(nlst[X_CCPU].n_value,  &ccpu,  sizeof(int),
		nlst[X_CCPU].n_name);
    
	/* some calculations we use later */
    
	mpid_offset = nlst[X_MPID].n_value;
	avenrun_offset = nlst[X_AVENRUN].n_value;
	cp_time_offset = nlst[X_CP_TIME].n_value;
	total_offset = nlst[X_TOTAL].n_value;
    
	/* this is used in calculating WCPU -- calculate it ahead of time */
#ifdef sun
	logcpu = log((double)ccpu / FSCALE);
#else
	logcpu = log(ccpu);
#endif
    
	/* allocate space for proc structure array and array of pointers */
	bytes  = nproc * sizeof(struct proc);
	pbase  = (struct proc *)sbrk(bytes);
	pref   = (struct proc **)sbrk(nproc * sizeof(struct proc *));

	/* Just in case ... */
	if (pbase == (struct proc *)NULL || pref == (struct proc **)NULL)
	{
	    fprintf(stderr, "%s: can't allocate sufficient memory\n", myname);
	    exit(3);
	}
    
	/* initialize the hashing stuff */
	if (do_unames)
	{
	    init_hash();
	}
	
	if (do_init)
	{
	    initialized = 1;
	    kill(0, SIGQUIT);
	    exit(99);
	}
    }

    /* initialize termcap */
    init_termcap();

    /*
     *  Smart terminals can only display so many processes, precisely
     *	"screen_length - Header_lines".  When run on dumb terminals, nothing
     *	fancy is done anyway, so we can display as many processes as the
     *	system can make.  But since we never need to remember what is on the
     *	screen, we only allocate a buffer for one screen line.
     */
    if (smart_terminal)
    {
	/* can only display (screen_length - Header_lines) processes */
	i = screen_length - Header_lines;
	if (topn > i)		/* false even when topn == Infinity */
	{
	    fprintf(stderr,
		"%s: warning: this terminal can only display %d processes.\n",
		myname, screen_length - Header_lines);
	    topn = i;
	    warnings++;
	}
    }
    else
    {
	i = 1;
	screen_length = nproc + Header_lines;
    }

    /* allocate space for the screen buffer */
    screenbuf = (char (*)[])sbrk(i * Display_width);
    if (screenbuf == (char (*)[])NULL)
    {
	fprintf(stderr, "%s: can't allocate sufficient memory\n", myname);
	exit(4);
    }

    /* adjust for topn == Infinity */
    if (topn == Infinity)
    {
	/*
	 *  For smart terminals, infinity really means everything that can
	 *  be displayed (which just happens to be "i" at this point).
	 *  On dumb terminals, infinity means every process in the system!
	 *  We only really want to do that if it was explicitly specified.
	 *  This is always the case when "Default_TOPN != Infinity".  But if
	 *  topn wasn't explicitly specified and we are on a dumb terminal
	 *  and the default is Infinity, then (and only then) we use
	 *  "Nominal_TOPN" instead.
	 */
#if Default_TOPN == Infinity
	topn = smart_terminal ? i :
		    (topn_specified ? nproc : Nominal_TOPN);
#else
	topn = smart_terminal ? i : nproc;
#endif
    }

    /* determine interactive state */
    if (interactive == Maybe)
    {
	interactive = smart_terminal;
    }

    /* if # of displays not specified, fill it in */
    if (displays == 0)
    {
	displays = smart_terminal ? Infinity : 1;
    }

    /* hold interrupt signals while setting up the screen and the handlers */
#ifndef FOUR_ONE
    old_sigmask = sigblock(Smask(SIGINT) | Smask(SIGQUIT) | Smask(SIGTSTP));
#endif
    init_screen();
    signal(SIGINT, leave);
    signal(SIGQUIT, leave);
    signal(SIGTSTP, tstop);
#ifndef FOUR_ONE
    sigsetmask(old_sigmask);
#endif
    if (warnings)
    {
	fprintf(stderr, "....");
	fflush(stderr);			/* why must I do this? */
	sleep(3 * warnings);
    }
    clear();

    /* setup the jump buffer for stops */
    if (setjmp(jmp_int) != 0)
    {
	/* control ends up here after an interrupt */
	clear();
	reset_display();
    }

    /*
     *  main loop -- repeat while display count is positive or while it
     *		indicates infinity (by being -1)
     */

    while ((displays == -1) || (displays-- > 0))
    {
	/* read all the proc structures in one fell swoop */
	getkval(proc, pbase, bytes, "proc array");

	/* get the cp_time array */
	getkval(cp_time_offset, cp_time, sizeof(cp_time), "_cp_time");

	/* get load average array */
	getkval(avenrun_offset, avenrun, sizeof(avenrun), "_avenrun");

	/* get mpid -- process id of last process */
	getkval(mpid_offset, &mpid, sizeof(mpid), "_mpid");

	/* get total -- systemwide main memory usage structure */
	getkval(total_offset, &total, sizeof(total), "_total");

	/* count up process states and get pointers to interesting procs */
	total_procs = 0;
	active_procs = 0;
	bzero(proc_brkdn, sizeof(proc_brkdn));
	prefp = pref;
	for (pp = pbase, i = 0; i < nproc; pp++, i++)
	{
	    /*
	     *  Place pointers to each valid proc structure in pref[].
	     *  Process slots that are actually in use have a non-zero
	     *  status field.  Processes with SSYS set are system
	     *  processes---these get ignored unless show_sysprocs is set.
	     */
	    if (pp->p_stat != 0 &&
		(show_sysprocs || ((pp->p_flag & SSYS) == 0)))
	    {
		total_procs++;
		proc_brkdn[pp->p_stat]++;
		if (pp->p_stat != SZOMB)
		{
		    *prefp++ = pp;
		    active_procs++;
		}
	    }
	}

	/* display the load averages */
	(*d_loadave)(mpid, avenrun);

	/*
	 *  Display the current time.
	 *  "ctime" always returns a string that looks like this:
	 *  
	 *	Sun Sep 16 01:03:52 1973
	 *      012345678901234567890123
	 *	          1         2
	 *
	 *  We want indices 11 thru 18 (length 8).
	 */

	curr_time = time(0);
	if (smart_terminal)
	{
	    Move_to(screen_width - 8, 0);
	}
	else
	{
	    fputs("    ", stdout);
	}
	printf("%-8.8s\n", &(ctime(&curr_time)[11]));

	/* display process state breakdown */
	(*d_procstates)(total_procs, proc_brkdn);

	/* calculate percentage time in each cpu state */
	if (dostates)	/* but not the first time */
	{
	    total_change = 0;
	    for (i = 0; i < CPUSTATES; i++)
	    {
		/* calculate changes for each state and overall change */
		if (cp_time[i] < cp_old[i])
		{
		    /* this only happens when the counter wraps */
		    change = (int)
			((unsigned long)cp_time[i]-(unsigned long)cp_old[i]);
		}
		else
		{
		    change = cp_time[i] - cp_old[i];
		}
		total_change += (cp_change[i] = change);
		cp_old[i] = cp_time[i];
	    }
	    (*d_cpustates)(cp_change, total_change);
	}
	else
	{
	    /* we'll do it next time */
	    if (smart_terminal)
	    {
		z_cpustates();
	    }
	    else
	    {
		putchar('\n');
	    }
	    dostates = Yes;

	    /* remember the current values as "old" values */
	    bcopy(cp_time, cp_old, sizeof(cp_time));
	}

	/* display main memory statistics */
	(*d_memory)(
		pagetok(total.t_rm), pagetok(total.t_arm),
		pagetok(total.t_vm), pagetok(total.t_avm),
		pagetok(total.t_free));

	i = 0;
	if (topn > 0)
	{
	    /* update the header area */
	    (*d_header)(uname_field);
    
	    /* sort by cpu percentage (pctcpu) */
	    qsort(pref, active_procs, sizeof(struct proc *), proc_compar);
    
	    /* adjust for a lack of processes */
	    if (active_procs > topn)
	    {
		active_procs = topn;
	    }

	    /*
	     *  Now, show the top "n" processes.  The method is slightly
	     *	different for dumb terminals, so we will just use two very
	     *	similar loops; this increases speed but also code size.
	     */
	    if (smart_terminal)
	    {
		for (prefp = pref, i = 0; i < active_procs; prefp++, i++)
		{
		    pp = *prefp;
		    (*d_process)(i, pp, get_userid);
		}
	    }
	    else for (prefp = pref, i = 0; i < active_procs; prefp++, i++)
	    {
		pp = *prefp;
		/* (only one buffer lien with dumb terminals) */
		(*d_process)(0, pp, get_userid);
	    }
	}

	/* do end-screen processing */
	u_endscreen(i);

	/* now, flush the output buffer */
	fflush(stdout);

	/* only do the rest if we have more displays to show */
	if (displays)
	{
	    /* switch out for new display on smart terminals */
	    if (smart_terminal)
	    {
		d_loadave = u_loadave;
		d_procstates = u_procstates;
		d_cpustates = u_cpustates;
		d_memory = u_memory;
		d_header = u_header;
		d_process = u_process;
	    }
    
#ifndef FOUR_ONE
	    if (!interactive)
#endif
	    {
		/* set up alarm */
		signal(SIGALRM, onalrm);
		alarm(delay);
    
		/* wait for the rest of it .... */
		pause();
	    }
#ifndef FOUR_ONE
	    else
	    {
		/* wait for either input or the end of the delay period */
		readfds = 1;			/* for standard input */
		timeout.tv_sec  = delay;
		timeout.tv_usec = 0;
		if (select(32, &readfds, 0, 0, &timeout) > 0)
		{
		    int newval;
		    char *errmsg;
    
		    /* something to read -- clear the message area first */
		    if (msg_showing)
		    {
			if (smart_terminal)
			{
			    putcap(clear_line);
			}
			msg_showing = No;
		    }

		    /* now read it and act on it */
		    read(0, &ch, 1);
		    switch(ch)
		    {
			case '\f':		/* redraw screen */
			    reset_display();
			    clear();
			    break;

			case ' ':		/* merely update display */
			    break;
	
			case 'q':		/* quit */
			    quit(0);
			    break;
	
			case 'h':		/* help */
			case '?':
			    reset_display();
			    clear();
			    show_help();
			    standout("Hit any key to continue: ");
			    fflush(stdout);
    			    read(0, &ch, 1);
			    clear();
			    break;
    
			case 'e':		/* show errors */
			    if (error_count() == 0)
			    {
				standout(" Currently no errors to report.");
				msg_showing = Yes;
			    }
			    else
			    {
				reset_display();
				clear();
				show_errors();
				standout("Hit any key to continue: ");
				fflush(stdout);
				read(0, &ch, 1);
				clear();
			    }
			    break;
    
			case 'n':		/* new number */
			case '#':
			    standout("Number of processes to show: ");
			    newval = readline(tempbuf1, 8, Yes);
			    putchar('\r');
			    if (newval > -1)
			    {
				if (newval > (i = screen_length - Header_lines))
				{
				    standout(
				      " This terminal can only display %d processes.",
				      i);
				    newval = i;
				    msg_showing = Yes;
				    break;
				}
	
				if (newval > topn)
				{
				    /* zero fill appropriate part of screenbuf */
				    bzero(screenbuf[topn],
					(newval - topn) * Display_width);
	
				    /* redraw header if need be */
				    if (topn == 0)
				    {
					d_header = i_header;
				    }
				}
				topn = newval;
			    }
			    putcap(clear_line);
			    break;
	
			case 's':		/* new seconds delay */
			    standout("Seconds to delay: ");
			    if ((i = readline(tempbuf1, 8, Yes)) > -1)
			    {
				delay = i;
			    }
			    putchar('\r');
			    putcap(clear_line);
			    break;
    
			case 'd':		/* change display count */
			    standout("Displays to show (currently %s): ",
				    displays == -1 ? "infinite" :
						     itoa(displays));
			    if ((i = readline(tempbuf1, 10, Yes)) > 0)
			    {
				displays = i;
			    }
			    else if (i == 0)
			    {
				quit(0);
			    }
			    putchar('\r');
			    putcap(clear_line);
			    break;

			case 'k':		/* kill program */
			    fputs("kill ", stdout);
			    if (readline(tempbuf2, sizeof(tempbuf2), No) > 0)
			    {
				if ((errmsg = kill_procs(tempbuf2)) != NULL)
				{
				    putchar('\r');
				    standout(errmsg);
				}
				msg_showing = Yes;
			    }
			    else
			    {
				putchar('\r');
			    }
			    putcap(clear_line);
			    break;
	
			case 'r':		/* renice program */
			    fputs("renice ", stdout);
			    if (readline(tempbuf2, sizeof(tempbuf2), No) > 0)
			    {
				if ((errmsg = renice_procs(tempbuf2)) != NULL)
				{
				    putchar('\r');
				    standout(errmsg);
				    msg_showing = Yes;
				}
			    }
			    else
			    {
				putchar('\r');
			    }
			    putcap(clear_line);
			    break;
	
			default:
			    standout(" Command not understood");
			    msg_showing = Yes;
		    }
		}
		else if (msg_showing)
		{
		    if (smart_terminal)
		    {
			putcap(clear_line);
		    }
		    msg_showing = No;
		}
	    }
#endif
	}
    }

    quit(0);
}

/*
 *  reset_display() - reset all the display routine pointers so that entire
 *	screen will get redrawn.
 */

reset_display()

{
    d_loadave    = i_loadave;
    d_procstates = i_procstates;
    d_cpustates  = i_cpustates;
    d_memory     = i_memory;
    d_header	 = i_header;
    d_process	 = i_process;
}

readline(buffer, size, numeric)

char *buffer;
int  size;
int  numeric;

{
    register char *ptr = buffer;
    register char ch;
    register char cnt = 0;

    size -= 1;
    while ((fflush(stdout), read(0, ptr, 1) > 0))
    {
	if ((ch = *ptr) == '\n')
	{
	    break;
	}

	if (ch == ch_kill)
	{
	    *buffer = '\0';
	    return(-1);
	}
	else if (ch == ch_erase)
	{
	    if (cnt <= 0)
	    {
		putchar('\7');
	    }
	    else
	    {
		fputs("\b \b", stdout);
		ptr--;
		cnt--;
	    }
	}
	else if (cnt == size || (numeric && (ch < '0' || ch > '9')))
	{
	    putchar('\7');
	}
	else
	{
	    putchar(ch);
	    ptr++;
	    cnt++;
	}
    }
    *ptr = '\0';
    return(cnt == 0 ? -1 : numeric ? atoi(buffer) : cnt);
}

/*
 *  signal handlers
 */

leave()			/* exit under normal conditions -- INT handler */

{
    end_screen();
    exit(0);
}

tstop()

{
    /* move to the lower left */
    end_screen();
    fflush(stdout);

#ifdef FOUR_ONE		/* a 4.1 system */

    /* send a STOP (uncatchable) to everyone in the process group */
    kill(0, SIGSTOP);

    /* reset the signal handler */
    signal(SIGTSTP, tstop);

#else			/* assume it is a 4.2 system */

    /* default the signal handler action */
    signal(SIGTSTP, SIG_DFL);

    /* unblock the signal and send ourselves one */
    sigsetmask(sigblock(0) & ~(1 << (SIGTSTP - 1)));
    kill(0, SIGTSTP);

    /* reset the signal handler */
    signal(SIGTSTP, tstop);

#endif
    /* reinit screen */
    reinit_screen();

    /* jump to appropriate place */
    longjmp(jmp_int, 1);

    /*NOTREACHED*/
}

quit(status)		/* exit under duress */

int status;

{
    end_screen();
    exit(status);
}

onalrm()

{
    return(0);
}

/*
 *  proc_compar - comparison function for "qsort"
 *	Compares the resource consumption of two processes using five
 *  	distinct keys.  The keys (in descending order of importance) are:
 *  	percent cpu, cpu ticks, state, resident set size, total virtual
 *  	memory usage.  The process states are ordered as follows (from least
 *  	to most important):  WAIT, zombie, sleep, stop, start, run.  The
 *  	array declaration below maps a process state index into a number
 *  	that reflects this ordering.
 */

unsigned char sorted_state[] =
{
    0,	/* not used		*/
    3,	/* sleep		*/
    1,	/* ABANDONED (WAIT)	*/
    6,	/* run			*/
    5,	/* start		*/
    2,	/* zombie		*/
    4	/* stop			*/
};
 
proc_compar(pp1, pp2)

struct proc **pp1;
struct proc **pp2;

{
    register struct proc *p1;
    register struct proc *p2;
    register int result;
#ifndef sun
    register double dresult;
#endif

    /* remove one level of indirection */
    p1 = *pp1;
    p2 = *pp2;

    /* compare percent cpu (pctcpu) */
#ifdef sun
    if ((result = p2->p_pctcpu - p1->p_pctcpu) == 0)
#else
    if ((dresult = p2->p_pctcpu - p1->p_pctcpu) == 0)
#endif
    {
	/* use cpticks to break the tie */
	if ((result = p2->p_cpticks - p1->p_cpticks) == 0)
	{
	    /* use process state to break the tie */
	    if ((result = sorted_state[p2->p_stat] -
			  sorted_state[p1->p_stat])  == 0)
	    {
		/* use priority to break the tie */
		if ((result = p2->p_pri - p1->p_pri) == 0)
		{
		    /* use resident set size (rssize) to break the tie */
		    if ((result = p2->p_rssize - p1->p_rssize) == 0)
		    {
			/* use total memory to break the tie */
#ifdef pyr
			result = (p2->p_tsize + p2->p_dsize + p2->p_ussize) -
				 (p1->p_tsize + p1->p_dsize + p1->p_ussize);
#else
			result = (p2->p_tsize + p2->p_dsize + p2->p_ssize) -
				 (p1->p_tsize + p1->p_dsize + p1->p_ssize);
#endif
		    }
		}
	    }
	}
    }
#ifndef sun
    else
    {
	result = dresult < 0.0 ? -1 : 1;
    }
#endif

    return(result);
}

/* routines to translate uids into a string */

char *user_name(euid, ruid)

int euid, ruid;

{
    return(username(euid));
}

char *user_uid(euid, ruid)

int euid, ruid;

{
    return(itoa7(euid));
}

/*
 *  These routines handle uid to username mapping.
 *  They use a hashing table scheme to reduce reading overhead.
 */

struct hash_el {
    int  uid;
    char name[8];
};

#define    H_empty	-1

/* simple minded hashing function */
#define    hashit(i)	((i) % Table_size)

struct hash_el hash_table[Table_size];

init_hash()

{
    register int i;
    register struct hash_el *h;

    for (h = hash_table, i = 0; i < Table_size; h++, i++)
    {
	h->uid = H_empty;
    }
}

char *username(uid)

register int uid;

{
    register int index;
    register int found;
    register char *name;

    /* This is incredibly naive, but it'll probably get changed anyway */
    index = hashit(uid);
    while ((found = hash_table[index].uid) != uid)
    {
	if (found == H_empty)
	{
	    /* not here -- get it out of passwd */
	    index = get_user(uid);
	    break;		/* out of while */
	}
	index = (index + 1) % Table_size;
    }
    return(hash_table[index].name);
}

enter_user(uid, name)

register int  uid;
register char *name;

{
    register int length;
    register int index;
    register int try;
    static int uid_count = 0;

    /* avoid table overflow -- insure at least one empty slot */
    if (++uid_count >= Table_size)
    {
	fprintf(stderr, "table overflow: too many users\n");
	quit(10);
    }

    index = hashit(uid);
    while ((try = hash_table[index].uid) != H_empty)
    {
	if (try == uid)
	{
	    return(index);
	}
	index = (index + 1) % Table_size;
    }
    hash_table[index].uid = uid;
    strncpy(hash_table[index].name, name, 8);
    return(index);
}

get_user(uid)

register int uid;

{
    struct passwd *pwd;
    register int last_index;

    while ((pwd = getpwent()) != NULL)
    {
	last_index = enter_user(pwd->pw_uid, pwd->pw_name);
	if (pwd->pw_uid == uid)
	{
	    return(last_index);
	}
    }
    return(enter_user(uid, itoa7(uid)));
}

atoiwi(str)

char *str;

{
    register int len;

    len = strlen(str);
    if (len != 0)
    {
	if (strncmp(str, "infinity", len) == 0 ||
	    strncmp(str, "all",      len) == 0 ||
	    strncmp(str, "maximum",  len) == 0)
	{
	    return(Infinity);
	}
	else if (str[0] == '-')
	{
	    return(Invalid);
	}
	else
	{
	    return(atoi(str));
	}
    }
    return(0);
}

/*
 *  itoa - convert integer (decimal) to ascii string for positive numbers
 *  	   only (we don't bother with negative numbers since we know we
 *	   don't use them).
 */

static char buffer[16];		/* shared by the next two routines */

char *itoa(val)

register int val;

{
    register char *ptr;

    ptr = buffer + sizeof(buffer);
    *--ptr = '\0';
    if (val == 0)
    {
	*--ptr = '0';
    }
    else while (val != 0)
    {
	*--ptr = (val % 10) + '0';
	val /= 10;
    }
    return(ptr);
}

/*
 *  itoa7(val) - like itoa, except the number is right justified in a 7
 *	character field.  This code is a duplication of itoa instead of
 *	a front end to a more general routine for efficiency.
 */

char *itoa7(val)

register int val;

{
    register char *ptr;

    ptr = buffer + sizeof(buffer);
    *--ptr = '\0';
    if (val == 0)
    {
	*--ptr = '0';
    }
    else while (val != 0)
    {
	*--ptr = (val % 10) + '0';
	val /= 10;
    }
    while (ptr > buffer + sizeof(buffer) - 7)
    {
	*--ptr = ' ';
    }
    return(ptr);
}

