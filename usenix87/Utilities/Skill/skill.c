#ifndef lint
static char *sccsid = "@(#)skill.c	1.9 (Jeff Forys @ SUNY/Buffalo) 4/16/86";
#endif

/*
 *  skill - send signals to processes by pid, command, username, or tty.
 *  Version 1.9
 *
 *  This program may be freely redistributed to other Unix sites, but
 *  this entire comment MUST remain intact.
 *
 *  Copyright (c) 1986, Jeff Forys, State University of New York at Buffalo
 *
 *  This program has been designed to run under BSD 4.2/3 and Sun 2/3.  The
 *  defines needed for each operating system are BSD4_2, BSD4_3,  SUN2, and
 *  SUN3, respectively.  One (and only one) of these *must* be defined.
 *
 */

#include <sys/param.h>

#ifdef BSD4_3
#include <sys/types.h>
#endif

#ifdef SUN3
#include <sys/types.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include <strings.h>
#include <nlist.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/vm.h>
#include <machine/pte.h>

#define	TTYLIM	256	/* max # ttys */
#define USRLIM	1024	/* max # user names */
#define PIDLIM	256	/* max # process ids */
#define CMDLIM	1024	/* max # commands */

dev_t	ttys[TTYLIM];	/* list of devices */
int	usrs[USRLIM];	/* list of users */
long	pids[PIDLIM];	/* list of process ids */
char	*cmds[CMDLIM];	/* list of commands */

int	itty = 0, iusr = 0, ipid = 0, icmd = 0;	/* index thru lists */
int	interact = 0, verbose = 0, warning = 0;	/* assorted flags */
int	signo = SIGTERM;			/* default signal, SIGTERM */
char	*pname;					/* program name */

char *signm[] = { 0,
"HUP", "INT", "QUIT", "ILL", "TRAP", "IOT", "EMT", "FPE",	/* 1-8 */
"KILL", "BUS", "SEGV", "SYS", "PIPE", "ALRM", "TERM", "URG",	/* 9-16 */
"STOP", "TSTP", "CONT", "CHLD", "TTIN", "TTOU", "IO", "XCPU",	/* 17-24 */
"XFSZ", "VTALRM", "PROF", "WINCH", 0, "USR1", "USR2", 0,	/* 25-31 */
};

extern char *sys_errlist[];
extern errno;

main(argc, argv)
int argc;
char *argv[];
{
    int skill();

    nice(-40);		/* Only effective when run by super user */

    if ((pname=rindex(argv[0],'/')) == NULL)	/* strip path to program */
	pname = argv[0];
    else
	pname++;

    if (chdir("/dev")) {			/* make things easier */
	fprintf(stderr, "%s: /dev is inaccessible\n");
	exit(2);
    }

    if (getargs(argc, argv))
	exit(skill());
    else
	usage();
}

getargs(argc, argv)	/* parse and categorize arguments */
int argc;
char *argv[];
{
    int help, foundsig;
    struct stat st;
    struct passwd *pp;

    foundsig = 0;
    while (--argc > 0) {	/* for each argument */
	help = 0;
	if (**(++argv) == '-') {	/* found a flag */
	    if (*++(*argv) == 'l')		/* list signals and exit */
		lstsig();
	    else if (**argv == 'i')		/* interactive mode */
		interact++;
	    else if (**argv == 'v')		/* verbose mode */
		verbose++;
	    else if (**argv == 'w')		/* display inaccessable pages */
		warning++;
	    else if ((**argv == 't') || (**argv == 'u') ||
		     (**argv == 'c') || (**argv == 'p'))
		help = **argv;
	    else if (foundsig)			/* already have a signal... */
		usage();
	    else if (isdigit(**argv)) {		/* numeric signal given */
		signo = atoi(*argv);			/* change to integer */
		if (signo < 0 || signo > NSIG)		/* check validity */
		    usage();
		foundsig++;
	    } else {				/* signal name given */
		for (signo=0; signo<=NSIG; signo++)	/* convert to number */
		    if (signm[signo] && !strcmp(signm[signo], *argv)) {
			foundsig++;
			break;
		    }
		if (! foundsig)			/* didnt match any signal */
		    usage();
	    }
	    if (! help)		/* just an option flag, go to next argument */
		continue;
	}

	if (help && !(*++(*argv) || (--argc && *++argv)))
	    usage();	/* no arg following user, tty, cmd, or pid flag */

	if (!help || help=='t') {	/* tty? */
	    if ((stat(*argv, &st)>=0) && ((st.st_mode & S_IFMT)==S_IFCHR)) {
		if (itty == TTYLIM)
		    exceed("tty");
		ttys[itty++] = st.st_rdev;
		help = -1;
	    } else if (help)
		not(*argv, "tty");
	}

	if (!help || help=='u') {	/* user name? */
 	    if ((pp=getpwnam(*argv)) != NULL) {
		if (iusr == USRLIM)
		    exceed("user");
		usrs[iusr++] = pp->pw_uid;
		help = -1;
	    } else if (help)
		not(*argv, "user name");
	}

	if (!help || help=='p') {	/* process id? */
	    if (isdigit(**argv)) {
		if (ipid == PIDLIM)
		    exceed("pid");
		pids[ipid++] = atoi(*argv);
		help = -1;
	    } else if (help)
		not(*argv, "process id");
	}

	if (help > -1) {		/* default; it's a command */
	    if (icmd == CMDLIM)
		exceed("command");
	    cmds[icmd++] = *argv;
	}
    }

    return(itty|iusr|ipid|icmd);	/* must specify something */
}

lstsig()	/* display a list of available signals and exit */
{
    int signo;
    for (signo = 0; signo <= NSIG; signo++) {
	if (signm[signo])
	    printf("%s ", signm[signo]);
	if (signo == 16)
	    printf("\n");
    }
    printf("\n");
    exit(0);
}

usage()		/* display usage information and exit */
{
    fprintf(stderr, "Usage: %s [-<signal>] [-i] [-v] [-w] {<tty> <user> <pid> <command>}\n", pname);
    exit(1);
}


not(thought, this)	/* type forced incorrectly (e.g. '-u tty00') */
char *thought, *this;
{
    fprintf(stderr, "%s: not a %s\n", thought, this);
    exit(1);
}

exceed(what)	/* type 'what' exceeded array bounds */
char *what;
{
    fprintf(stderr, "%s: too many %s's specified\n", pname, what);
    exit(1);
}

/*
 *	We have categorized the arguments as ttys, users, commands,
 *	or process ids.  Now, we go through memory and find things
 *	that match.  In order for a match, we must have 1 item from
 *	each category (unless a category has nothing in it).  Upon
 *	finding a match, the process is immediately sent the signal
 *	(unless interactive mode is set).
 */

struct nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_Usrptmap" },
#define	X_USRPTMAP	1
	{ "_usrpt" },
#define	X_USRPT		2
	{ "_text" },
#define	X_TEXT		3
	{ "_nproc" },
#define	X_NPROC		4
	{ "_ntext" },
#define	X_NTEXT		5
	{ "" },
};

#define	NPROC	16

struct	proc proc[NPROC];
struct	proc *mproc;

union {
	struct	user user;
	char	upages[UPAGES][NBPG];
} user;

struct	pte *Usrptmap, *usrpt;
int	nproc;

char	*kmemf, *memf, *swapf, *nlistf;
int	kmem, mem, swap;

int skill()	/* send signal to processes */
{
    register int i, j, k;
    int reterr = -1;
    int myid, getpid();
    off_t procp;

    myid = getpid();

    kmemf = "/dev/kmem";
    if ((kmem=open(kmemf, 0)) < 0) {		/* open kmem */
	perror(kmemf);
	return(2);
    }
    memf = "/dev/mem";
    if ((mem=open(memf, 0)) < 0) {		/* open mem */
	perror(memf);
	return(2);
    }
    swapf = "/dev/drum";
    if ((swap=open(swapf, 0)) < 0) {		/* open swap device */
	perror(swapf);
	return(2);
    }

    nlistf = "/vmunix";
    nlist(nlistf, nl);				/* get system namelist */
    if (nl[0].n_type == 0) {
	fprintf(stderr, "%s: No namelist\n", nlistf);
	return(2);
    }
    usrpt = (struct pte *)nl[X_USRPT].n_value;
    Usrptmap = (struct pte *)nl[X_USRPTMAP].n_value;

    procp = getword(nl[X_PROC].n_value);
    nproc = getword(nl[X_NPROC].n_value);
    for (i=0; i<nproc; i += NPROC) {		/* for each process */
	(void) lseek(kmem, (off_t)procp, 0);
	if ((j=nproc-i) > NPROC)
	    j = NPROC;
	j *= sizeof (struct proc);
	if (read(kmem, (char *)proc, j) != j) {
	    fprintf(stderr,"%s: error reading proc table from %s\n",pname,kmemf);
	    return(2);
	}
	procp += j;
	for (j = j / sizeof (struct proc) - 1; j >= 0; j--) {
	    mproc = &proc[j];

	    if (mproc->p_stat == 0 || getuser() == 0)	/* hmmm */
		continue;
	    if (user.user.u_ttyp == 0 && itty)	/* no controlling tty */
		continue;

	    for (k=0; k<itty && ttys[k] != user.user.u_ttyd; k++)
		;
	    if (itty && k==itty)	/* couldnt find matching tty */
		continue;

	    for (k=0; k<iusr && usrs[k]!=mproc->p_uid; k++)
		;
	    if (iusr && k==iusr)	/* couldnt find matching username */
		continue;

	    for (k=0; k<ipid && pids[k]!=mproc->p_pid; k++)
		;
	    if (ipid && k==ipid)	/* couldnt find matching pid */
		continue;

	    for (k=0; k<icmd && strcmp(cmds[k],user.user.u_comm); k++)
		;
	    if (icmd && k==icmd)	/* couldnt find matching command */
		continue;

	    if (mproc->p_pid == myid || mproc->p_pid == 0)
		continue;	/* dont kill thyself or thygroup */

	    if (interact || mproc->p_pid < 3) {	/* check to make sure */
		static char yesno[10];

		(void) fseek(stdin, 0L, 0);	/* clear pending input */
		printf("%s: signal pid %u (%s executing %s)? ", pname,
			mproc->p_pid, getpwuid(mproc->p_uid)->pw_name,
			mproc->p_pid==1? "init":
			mproc->p_pid==2? "page daemon": user.user.u_comm);
		if (fgets(yesno, 10, stdin) == NULL) {	/* EOF, exit */
		    printf("\n");
		    return(0);
		}
		if (*yesno != 'y')
		    continue;
	    }

 	    if (mproc->p_stat == SZOMB) {	/* dont bother with zombies */
		fprintf(stderr, "%u: process is defunct\n", mproc->p_pid);
		continue;
	    } else if (mproc->p_flag & SWEXIT) {	/* ignore if exiting */
		fprintf(stderr, "%u: process is exiting\n", mproc->p_pid);
		continue;
	    }

	    if (kill(mproc->p_pid, signo) < 0) {	/* send signal */
		fprintf(stderr, "%u: %s\n", mproc->p_pid, sys_errlist[errno]);
		reterr = 1;
	    } else {		/* signal was sent without incident */
		reterr = 0;
		if (verbose) {	/* display killed signal */
		    printf("> sent %s to %u", signm[signo], mproc->p_pid);
		    if (! interact)	/* be a little more verbose */
			printf(" (%s executing %s)",
			    getpwuid(mproc->p_uid)->pw_name, user.user.u_comm);
		    printf("\n");
		}
	    }
	}
    }

    if (reterr == -1) {
	fprintf(stderr, "%s: no matching processes\n", pname);
	reterr = 1;
    }

    return(reterr);
}

/*
 *	the remaining 2 functions are based on similar functions from 'ps'
 */

getword(loc)
    unsigned long loc;
{
    int word;

    (void) lseek(kmem, (off_t)loc, 0);
    if (read(kmem, (char *)&word, sizeof (word)) != sizeof (word))
	fprintf(stderr, "error reading kmem at %x\n", loc);
    return (word);
}

getuser()	/* read user structure, return 1 if ok, 0 otherwise */
{
    struct pte *pteaddr, apte;
    struct pte arguutl[UPAGES+CLSIZE];
    register int i;
    int ncl, size;

#ifdef BSD4_2
    size = sizeof (struct user);
#endif

#ifdef BSD4_3
    size = sizeof (struct user);
#endif

#ifdef SUN2
    size = roundup(sizeof (struct user), DEV_BSIZE);
#endif

#ifdef SUN3
    size = roundup(sizeof (struct user), DEV_BSIZE);
#endif

    if ((mproc->p_flag & SLOAD) == 0) {
	(void) lseek(swap, (off_t)dtob(mproc->p_swaddr), 0);
	if (read(swap, (char *)&user.user, size) != size) {
	    if (warning)
		printf("*** cant read u for pid %d from %s\n",
		      mproc->p_pid, swapf);
	    return (0);
	}
	return (1);
    }

    pteaddr = &Usrptmap[btokmx(mproc->p_p0br) + mproc->p_szpt - 1];
    (void) lseek(kmem, (off_t)pteaddr, 0);
    if (read(kmem, (char *)&apte, sizeof(apte)) != sizeof(apte)) {
	if (warning)
	    printf("*** cant read indir pte to get u for pid %d from %s\n",
		   mproc->p_pid, kmemf);
	return (0);
    }

    (void) lseek(mem,(off_t)ctob(apte.pg_pfnum+1)-(UPAGES+CLSIZE)*sizeof(struct pte),0);
    if (read(mem, (char *)arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
	if (warning)
	    printf("*** cant read page table for u of pid %d from %s\n",
		   mproc->p_pid, memf);
	return (0);
    }

    ncl = (size + NBPG*CLSIZE - 1) / (NBPG*CLSIZE);
    while (--ncl >= 0) {
	i = ncl * CLSIZE;
	(void) lseek(mem, (off_t)ctob(arguutl[CLSIZE+i].pg_pfnum), 0);
	if (read(mem, user.upages[i], CLSIZE*NBPG) != CLSIZE*NBPG) {
	    if (warning)
		printf("*** cant read page %d of u of pid %d from %s\n",
		       arguutl[CLSIZE+i].pg_pfnum,mproc->p_pid,memf);
	    return(0);
	}
    }

    return (1);
}
