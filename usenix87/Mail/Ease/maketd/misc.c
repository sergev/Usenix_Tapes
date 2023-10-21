/* Written & hacked by Stephen Uitti, PUCC staff, ach@pucc-j, 1985
 * maketd is copyright (C) Purdue University, 1985
 *
 * Permission is hereby given for its free reproduction and
 * modification for non-commercial purposes, provided that this
 * notice and all embedded copyright notices be retained.
 * Commercial organisations may give away copies as part of their
 * systems provided that they do so without charge, and that they
 * acknowledge the source of the software.
 */

#ifdef BSD2_9
#include <sys/types.h>
#include <signal.h>
#endif
#ifdef BSD4_2
#include <sys/signal.h>
#endif
#include <stdio.h>

#include "maketd.h"

/* VARARGS */
/* print and error and exit */
void
err(a, b, c, d)
char *a;				/* the format string, as printf */
char *b, *c, *d;			/* arguments, as printf */
{
    fprintf(stderr, "%s: ", prgnm);
    fprintf(stderr, a, b, c, d);
    fprintf(stderr, "\n");
    errrec();				/* clean up backup file, if any */
    exit(1);
}

/* errrec - error recovery: recover temp file */
static void
errrec()
{
    if (backedup) {
	backedup = FALSE;		/* prevent infinite error recursion */
	unlink(makename);		/* may or may not exist */
	if (link(backupfn, makename))
	    err("can't link %s to %s during recovery", makename, backupfn);
	if (unlink(backupfn))
	    err("can't delete %s during recovery", backupfn);
	err("edit of '%s' aborted - file unchanged.", makename);
    }
    exit(1);
}

/* signal trap routines, one for each */
static void
errhup()
{
    fprintf(stderr, "%s: SIGHUP recieved\n", prgnm);
    errrec();
}

static void
errint()
{
    fprintf(stderr, "%s: SIGINT recieved\n", prgnm);
    errrec();
}

static void
errill()
{
    fprintf(stderr, "%s: SIGILL recieved\n", prgnm);
    errrec();
}

static void
errtrap()
{
    fprintf(stderr, "%s: SIGTRAP recieved\n", prgnm);
    errrec();
}

static void
erriot()
{
    fprintf(stderr, "%s: SIGIOT recieved\n", prgnm);
    errrec();
}

static void
erremt()
{
    fprintf(stderr, "%s: SIGEMT recieved\n", prgnm);
    errrec();
}

static void
errfpe()
{
    fprintf(stderr, "%s: SIGFPE recieved\n", prgnm);
    errrec();
}

static void
errbus()
{
    fprintf(stderr, "%s: SIGBUS recieved\n", prgnm);
    errrec();
}

static void
errsegv()
{
    fprintf(stderr, "%s: SIGSEGV recieved\n", prgnm);
    errrec();
}

static void
errsys()
{
    fprintf(stderr, "%s: SIGSYS recieved\n", prgnm);
    errrec();
}

static void
errpipe()
{
    fprintf(stderr, "%s: SIGPIPE recieved\n", prgnm);
    errrec();
}

static void
erralrm()
{
    fprintf(stderr, "%s: SIGALRM recieved\n", prgnm);
    errrec();
}

static void
errterm()
{
    fprintf(stderr, "%s: SIGTERM recieved\n", prgnm);
    errrec();
}

/* catchsig - init signal traps */
catchsig()
{
    signal(SIGHUP, errhup);
    signal(SIGINT, errint);
    signal(SIGILL, errill);
    signal(SIGTRAP, errtrap);
    signal(SIGIOT, erriot);
    signal(SIGEMT, erremt);
    signal(SIGFPE, errfpe);
    signal(SIGBUS, errbus);
    signal(SIGSEGV, errsegv);
    signal(SIGSYS, errsys);
    signal(SIGPIPE, errpipe);
    signal(SIGALRM, erralrm);
    signal(SIGTERM, errterm);
/* Stock 2.9BSD has all the above, but not as many as 4.2BSD:
 * Want SIGQUIT to drop core.
 * Not worrying about:	SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF
 * cannot be caught: SIGKILL, SIGSTOP
 * Leaving alone: SIGURG, SIGTSTP, SIGCONT, SIGCHLD, SIGTTIN, SIGTTOU, SIGIO
 */
}

/* lastlnch - return last char before newline or NULL in string */
char
lastlnch(p)
register char *p;
{
    register char *q;

    q = p;
    while (*p != '\0' && *p != '\n')
	q = p++;
    return *q;
}
