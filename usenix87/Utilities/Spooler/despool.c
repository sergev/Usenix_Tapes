#ifndef lint
static char rcsid[] = "$Header: /usr/enee/chris/sys/spool/RCS/despool.c,v 1.5 84/04/22 03:32:55 chris Exp $";
#endif

/* Copyright (c) 1984 Chris Torek / University of Maryland */

/*
 * $Header: /usr/enee/chris/sys/spool/RCS/despool.c,v 1.5 84/04/22 03:32:55 chris Exp $
 *
 * Despool the spooled files in a spool directory
 *
 * Usage: despool [ device-name ]
 *
 * $Log:	despool.c,v $
 * Revision 1.5  84/04/22  03:32:55  chris
 * Security hacking; new 'R' control card for removing D. files
 * 
 * Revision 1.4  84/01/24  21:17:33  chris
 * Rewrote large portions of DeSpool; opening the device is now delayed
 * until after a control file is found.  Locking might be fixed (I hope).
 * 
 * Revision 1.3  83/12/10  02:45:31  chris
 * Versatec hacking:  checks for EIO after failed open; if so,
 * prints "offline: device <dev>" and sleeps for 60 seconds, then
 * tries again.
 * 
 * Revision 1.2  83/12/10  01:12:25  chris
 * Fixed geteuid call
 * 
 * Revision 1.1  83/12/10  01:03:01  chris
 * Initial revision
 * 
 */

#include <stdio.h>
#include <errno.h>
#include <sys/param.h>		/* for NOFILE */
#include <4.2/dir.h>		/* 4.2 dir library */
#include "spool.h"

int	IsLocked;		/* True => we have locked spool dir */
char	*ProgName;		/* Program name, for errors */
char	*ErrDev;		/* Device name, for errors */

long	lseek ();
char	*strcpy (),
	*sprintf ();

#define	warn(msg)	err (0, msg)

/* Need an external name called "error" for readdev.c */
error (msg)
char *msg;
{
    err (1, msg);
}

err (doexit, msg)
int doexit;
char *msg;
{
    register FILE *console;

    fputs (msg, stderr);
    if (ErrDev)
	fprintf (stderr, "\tdevice %s\n", ErrDev);
    if (console = fopen ("/dev/console", "w")) {
	fputs (msg, console);
	if (ErrDev)
	    fprintf (console, "\tdevice %s\n", ErrDev);
	(void) fclose (console);
    }
    if (doexit) {
	if (IsLocked)
	    UnLock ();
	exit (1);
	/* NOTREACHED */
    }
}

/* If we are invoked with arguments, despool each argument directory (fork
   off all but the last one).  Otherwise despool all devices (again fork off
   all but the last). */

main (argc, argv)
int argc;
char **argv;
{
    register struct SpoolDevice *sd;

    argc--;
    ProgName = *argv++;
#ifndef	DEBUG
    if (geteuid ())
	error ("panic: not setuid\n");
#endif	DEBUG

    umask (022);
    GetDevices ();

    if (argc) {
	while (--argc >= 0) {
	    if (argc > 0 && fork () > 0) {
		if ((sd = FindDevice (*argv++)) == 0)
		    error ("invalid device name\n");
		DeSpool (sd);
		exit (0);
	    }
	    else {
		if ((sd = FindDevice (*argv++)) == 0)
		    error ("invalid device name\n");
		DeSpool (sd);
		ErrDev = 0;
	    }
	}
	exit (0);
    }

    for (sd = DevList; sd; sd = sd -> sd_next) {
    	if (sd -> sd_next && fork () > 0)
	    DeSpool (sd), exit (0);
	else
	    DeSpool (sd);
    }
    exit (0);
}

/* 9 is strlen("C.aa12345") */
#define	IsCFile(p) (p->d_namlen==9 && p->d_name[0]=='C' && p->d_name[1]=='.')

/* Scan the directory looking for C. files.  If we find one, try to lock this
   device.  If someone else has it locked, then we're done; just return.
   Otherwise, try to open the device.  If this fails, unlock and return.
   At this point we have the device on stdout and a control file.  Run the
   control file and continue.  After the directory is finished, unlock the
   device, and if we ran any control files, start the whole thing all over
   again in case more stuff got spooled. */

DeSpool (dev)
register struct SpoolDevice *dev;
{
    register DIR   *dirf;
    register struct direct *dp;
    char     dirname[200],	/* hardcoded pathname lengths again *sigh* */
	     msg[100];
    int      IsOpen;
    extern   errno;

    IsOpen = 0;			/* dev not connected to stdout yet */
    (void) sprintf (dirname, SpoolDirectory, dev -> sd_namelist.nl_name);
    if (chdir (dirname)) {
	(void) sprintf (msg, "panic: can't chdir to %s\n", dirname);
	error (msg);
    }
    if ((dirf = opendir (".")) == NULL)
	error ("panic: can't open \".\"\n");
rescan:
    while (dp = readdir (dirf)) {
	if (!IsCFile (dp))
	    continue;
	if (!IsLocked) {
	    if (Lock ())
		break;		/* someone else is doing this dir */
	}
	if (!IsOpen) {
	    int fd;
	    ErrDev = dev -> sd_namelist.nl_name;
	    fd = DevOpen (dev -> sd_dev);
	    if (fd != 1) {
		(void) sprintf (msg, "panic: got fd %d (should be 1)\n", fd);
		error (msg);
	    }
	    IsOpen++;
	}
	DoFile (dp -> d_name, dev);
#ifdef	DEBUG
	strcpy (msg, dp -> d_name);
	*msg = 'B';
	(void) link (dp -> d_name, msg);
#endif	DEBUG
	(void) unlink (dp -> d_name);
    }
    closedir (dirf);
    if (IsLocked) {		/* Then we did at least one C. file */
	UnLock ();
	if (dirf = opendir ("."))
	    goto rescan;
    }
}

/* Open the named device; try hard to make it stdout (fd 1) */
DevOpen (name)
char *name;
{
    int fd;
    static unsigned int sleepy;

    /* In order to get stdout we must have stdin tied somewhere, so we
       open /dev/null and close it only if it's not stdout. */
    fd = open ("/dev/null", 0);	/* ensure fd 0 used */
    (void) close (1);		/* get rid of stdout */
    if (fd > 1)			/* don't need it & didn't just close it */
	(void) close (fd);

    /* The EIO (''Old MacDonald Had A Farm, E I E I O...'') stuff is a
       Versatec hack.  The versatec gives back EIO if offline, ENXIO for
       already open. */
    sleepy = 60;
reopen:
    fd = open (name, 1);	/* BOGUS!  Should be per-dev! */
    if (fd < 0 && (errno == EIO || errno == ENXIO)) {
	warn (errno == EIO ? "offline:" : "busy:");
	(void) sleep (sleepy);	/* Wait for someone to fix. */
	sleepy += 30;		/* Gradually slow down too, */
	if (sleepy >= 600)	/* but not more than 10 minutes apart */
	    sleepy = 600;	/* so that we don't doze off. */
	goto reopen;
    }
    if (fd < 0) {
	char msg[100];
	(void) sprintf (msg, "can't open %s\n", name);
	error (msg);
    }
    return fd;
}

/* For now, we lock using links.  When 4.2 rolls around we'll use the
   fancy file locking stuff. */
Lock () {
    register int n;
    char lname[10];

    (void) sprintf (lname, "l.%d", getpid ());
    (void) unlink (lname);
    if ((n = creat (lname, 0)) < 0) {
	warn ("can't create lock temp\n");
	return 1;
    }
    (void) close (n);
    if ((n = link (lname, "lock")) == 0)
	IsLocked++;
    (void) unlink (lname);
    return n;
}

UnLock () {
    if (unlink ("lock"))
	warn ("can't unlink lock\n");
    IsLocked = 0;
}

/* Perform all work indicated by the C. file.  NOTE:  the size of 'buf'
   matches the size restrictions enforced by the spooler, to ensure that
   long names don't overrun important variables and wreak havoc (after
   all, we *are* setuid...). */
DoFile (file, dev)
char *file;
struct SpoolDevice *dev;
{
    register FILE  *f;
    register char  *cp;
    register int    c;
    int     uid,
            gid,
            mailflag,
            in_fd,
	    DidHeader;
    char    user[50],
            title[200],
            buf[200],
            msg[300];

    if ((f = fopen (file, "r")) == NULL) {
	(void) sprintf (msg, "can't open control file %s\n", file);
	warn (msg);
	return;
    }
    uid = -1;			/* none yet */
    gid = -1;
    mailflag = 0;
    user[0] = 0;
    title[0] = 0;
    in_fd = -1;
    DidHeader = 0;
    while (fgets (buf, sizeof buf, f)) {
	cp = buf + strlen (buf);
	if (cp > buf && *--cp == '\n')
	    *cp = 0;
	cp = buf;
	c = *cp++;
	if (*cp == ' ')
	    cp++;
	switch (c) {
	case '#':
	case 'D':
	case 0:
	    break;
	case 'u':
	    if (sscanf (cp, "%d %d", &uid, &gid) != 2) {
		(void) sprintf (msg, "bad format control file [u card] %s\n",
			file);
		warn (msg);
		uid = gid = -1;
	    }
	    break;
	case 'M':
	    mailflag++;
	    break;
	case 'U':
	    strcpy (user, cp);
	    break;
	case 'T':
	    strcpy (title, cp);
	    break;
	case '<':		/* input file */
	    if (in_fd >= 0)
		(void) close (in_fd);
	    if ((in_fd = open (cp, 0)) < 0) {
		(void) sprintf (msg, "(%s) can't open %s\n", file, cp);
		warn (msg);
	    }
	    break;
	case 'X':		/* execute program */
	    if (uid < 0 || gid < 0 || *user == 0) {
		(void) sprintf (msg, "bad format control file %s\n", file);
		warn (msg);
		break;
	    }
	    if (in_fd < 0)
		break;
	    if (DidHeader == 0 && dev -> sd_header) {
		DidHeader++;
		ForkExec (uid, gid, fileno (f), dev -> sd_header);
	    }
	    ForkExec (uid, gid, in_fd, cp);
	    break;
	case 'R':		/* remove file */
	    if (unlink (cp)) {
		(void) sprintf (msg, "can't remove file \"%s\"\n", cp);
		warn (msg);
	    }
	    break;
	default: 
	    cp = buf;
	    if (c & 0200) {
		c &= 0177;
		*cp++ = 'M', *cp++ = '-';
	    }
	    if (c == 0177)
		*cp++ = '^', *cp++ = '?';
	    else if (c < ' ')
		*cp++ = '^', *cp++ = c + '@';
	    else
		*cp++ = c;
	    *cp = 0;
	    (void) sprintf (msg, "odd character (%s) in %s\n", buf, file);
	    warn (msg);
	    break;
	}
    }
    if (in_fd > 0)
	(void) close (in_fd);
#ifdef lint
    in_fd = mailflag;		/* ...for now */
#endif
}

/* Execute "prog" as user (uid, gid).  If fd is nonzero move it to fd 0. */
ForkExec (uid, gid, fd, prog)
int uid, gid, fd;
char *prog;
{
    register int    pid,
                    w;
    int     status;

#ifdef	DEBUG
    fprintf (stderr, "about to exec \"%s\"\n", prog);
    fprintf (stderr, "uid = %d gid = %d fd = %d\n", uid, gid, fd);
#endif	DEBUG
    for (w = 0; w < 10 && (pid = fork ()) < 0; )
	sleep ((unsigned) ++w);
    if (pid < 0) {
	warn ("can't fork\n");
	return;
    }
    if (pid) {			/* parent */
	while ((w = wait (&status)) != pid && w != -1)
	    ;
	return;
    }
    /* child */
    (void) setgid (gid);
    (void) setuid (uid);
    if (fd) {
	dup2 (fd, 0);
	(void) close (fd);
    }
    (void) lseek (0, 0L, 0);	/* rewind stdin */
    for (w = 3; w < NOFILE; w++)
	(void) close (w);
    execl ("/bin/sh", "sh", "-c", prog, 0);
    error ("can't execl /bin/sh ?!\n");
    /* NOTREACHED */
}
