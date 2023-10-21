#ifndef lint
static char rcsid[] = "$Header: /usr/enee/chris/sys/spool/RCS/spool.c,v 1.5 84/04/22 04:38:45 chris Exp $";
#endif

/* Copyright (c) 1984 Chris Torek / University of Maryland */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include "spool.h"

/*
 * $Header: /usr/enee/chris/sys/spool/RCS/spool.c,v 1.5 84/04/22 04:38:45 chris Exp $
 *
 * $Log:	spool.c,v $
 * Revision 1.5  84/04/22  04:38:45  chris
 * Security hacking; new 'R' control card for removing files; use jobs
 * library on 4.1
 * 
 * Revision 1.4  83/12/22  21:19:41  chris
 * Fixed file-left-open bug
 * 
 * Revision 1.3  83/12/14  23:16:32  chris
 * Fixed security bug in spooler ... was creating the control file
 * according to the mode in the user's umask; if this was 0, anyone
 * could write the file.  Now does a chmod 0644.
 * 
 * Revision 1.2  83/12/10  01:10:26  chris
 * Fixed geteuid call
 * 
 * Revision 1.1  83/12/10  01:04:04  chris
 * Initial revision
 * 
 *
 * Spooler
 *
 * The general workings are as follows:
 *	interpret options and generate control & data files for the despooler
 * The despooler reads the control files to decide what to print, and how.
 */

char	*CFName,		/* Control file */
	*DFName,		/* Data file (if any) */
	*TFName,		/* Temporary control file */
	*PrintProgram,		/* Program (-p, -<foo>, or default) */
	*UserName,		/* User's login name */
	*Title,			/* Title (-t) */
	*ProgName;		/* argv[0], for errors */
int	CopyFlag,		/* Use a copy instead of the original (-c) */
	MailFlag,		/* Send mail back to user when done (-m) */
	UserID,			/* User's UID */
	GroupID;		/* User's GID */
struct SpoolDevice
	*Device;		/* Device */

char *malloc (),
     *sprintf (),
     *LookUpType ();
struct passwd *getpwuid ();

#ifdef BSD41
int (*sigset ())();
#define set(sig)	if (sigset (sig, SIG_IGN) == SIG_DFL) \
				(void) sigset (sig, cleanup)
#else BSD41
#define	set(sig)	if (signal (sig, SIG_IGN) == SIG_DFL) \
				(void) signal (sig, cleanup)
#endif BSD41

main (argc, argv)
int argc;
register char **argv;
{
    register char  *cp;
    char   *generic_type,
	   *fname;
    int     file,
	    got_prog,
	    do_stdin = 1;
    int     cleanup ();

    set (SIGINT);
    set (SIGQUIT);
    set (SIGHUP);
    set (SIGTERM);

    argc--;
    ProgName = *argv++;
#ifndef	DEBUG
    if (geteuid ())
	error ("panic: not setuid\n");
#endif	DEBUG
    GetDevices ();
    UserID = getuid ();
    GroupID = getgid ();
    {
	register struct passwd *p;

	if ((p = getpwuid (UserID)) == NULL)
	    error ("intruder alert\n");
	UserName = p -> pw_name;
	endpwent ();
    }
    generic_type = 0;
    got_prog = 0;
    while (--argc >= 0 || do_stdin) {
	if (argc < 0) {		/* got here for do_stdin then */
	    file = 0;		/* fileno (stdin) */
	    fname = (char *) 0;
	    goto dofile;
	}
	if (**argv == '-') {
	    cp = *argv++ + 1;
	    if (*cp == 0) {	/* "-" => stdin */
		file = 0;
		fname = (char *) 0;
		goto dofile;
	    }
	    if (cp[1] == 0) {	/* -<x>\0 */
		switch (*cp) {
		    case 'd': 
			if (--argc < 0)
			    error ("-d requires an argument\n");
			Device = FindDevice (*argv++);
			if (Device == (struct SpoolDevice *) 0) {
			    fprintf (stderr, "%s: %s: no such spool device\n",
						ProgName, argv[-1]);
			    cleanup ();
			    /* NOTREACHED */
			}
			break;
		    case 'p': 
			if (got_prog)
			    error ("use only one -p per file\n");
			if (--argc < 0)
			    error ("-p requires an argument\n");
			PrintProgram = *argv++;
			got_prog++;
			do_stdin++;
			break;
		    case 't':
			if (--argc < 0)
			    error ("-t requires an argument\n");
			Title = *argv++;
			break;
		    case 'c': 
			CopyFlag++;
			break;
		    case 'm': 
			MailFlag++;
			break;
		    default:
			fprintf (stderr, "%s: unknown option -%c\n",
						ProgName, *cp);
			cleanup ();
			/* NOTREACHED */
		}
	    }
	    else {		/* -<xyz> */
		if (generic_type)
		    error ("use only one generic type per file\n");
		generic_type = cp;
		do_stdin++;
	    }
	}
	else {			/* argument not starting with "-" */
	    /* Must be a file to print.  Ensure that the guy has read access,
	       and get a file descriptor */
	    if (access (*argv, 4) || ((file = open (*argv++, 0)) < 0)) {
		fprintf (stderr, "%s: ", ProgName);
		perror (argv[-1]);
		cleanup ();
		/* NOTREACHED */
	    }
dofile: 
	    if (Device == (struct SpoolDevice *) 0)
		error ("must specify a device name\n");
	    if (got_prog) {
		if (Device -> sd_restricted)
		    error ("restricted device (-p invalid)\n");
		if (generic_type)
		    error ("use only one of -p and generic type\n");
		got_prog = 0;
	    }
	    else {
		if (generic_type || PrintProgram == (char *) 0) {
		    PrintProgram = LookUpType (generic_type);
		    generic_type = 0;
		}
	    }
	    CopyFile (fname, file);
	    if (file != 0)
		(void) close (file);
	    do_stdin = 0;
	    Title = (char *) 0;
	}
    }
    cleanup ();
    /* NOTREACHED */
}

/* Make a unique name by filling in the last 7 characters of the given
   string. */
UniqueName (name)
char *name;
{
    register int    i,
                    pid;
    register char  *s = name;

    pid = getpid ();
    while (*s)
	s++;
    s -= 5;
    for (i = 5; --i >= 0; ) {
	s[i] = pid % 10 + '0';
	pid /= 10;
    }
    *--s = 'a';
    s[-1] = 'a';
    while (access (name, 0) == 0) {
	if (++*s > '~') {
	    *s = 'a';
	    if (++s[-1] > '~')
		error ("panic: can't get unique name\n");
	}
    }
}

StartDeSpool ()
{
    switch (vfork ()) {
	case 0:			/* child */
	    execl (DeSpoolDaemon, DeSpoolDaemon, Device -> sd_dev, 0);
	    fprintf (stderr, "%s: ", ProgName);
	    perror ("can't exec despooler");
	    _exit (0);
	case -1:	    /* Leave file to be despooled later */
	    fprintf (stderr, "%s: ", ProgName);
	    perror ("can't fork");
    }
}

cleanup () {
    if (CFName)
	(void) unlink (CFName);
    if (DFName)
	(void) unlink (DFName);
    if (TFName)
	(void) unlink (TFName);
    exit (0);
}

/* Copy the user's data file whose name is 'name' and is already opened as fd
   'f'.  Basic task is to generate the control cards and make a link to the
   data file (if possible) or copy the data file (if CopyFlag is set).  Care
   is taken to make sure that the control file is secure.  NOTE:  the 200
   character buffer size is the same as in the despooler.  If you change one,
   change the other too!  Here we mercilessly truncate excessively long names
   (more security hacking). */
CopyFile (name, f)
char *name;
int f;
{
    char cnbuf[200], dnbuf[200], tnbuf[200], dirname[200];
    int  islink = 0;
    register FILE *cf;

    /* Generate file names.  There are 7 Xs in each string below. */
    (void) sprintf (dirname, SpoolDirectory, Device -> sd_namelist.nl_name);
    (void) sprintf (cnbuf, "%s/C.XXXXXXX", dirname);
    (void) sprintf (dnbuf, "%s/D.XXXXXXX", dirname);
    (void) sprintf (tnbuf, "%s/T.XXXXXXX", dirname);
    UniqueName (cnbuf);
    UniqueName (dnbuf);
    UniqueName (tnbuf);
    CFName = cnbuf;
    TFName = tnbuf;
    DFName = dnbuf;
    /* If possible, link to the named file, or as a second alternative put
       the file name right into the control card */
    if (!CopyFlag && name != (char *) 0)
	if (link (name, DFName) == 0)
	    islink++;
    /* Build temporary command file */
    if ((cf = fopen (TFName, "w")) == NULL) {
	fprintf (stderr, "%s: can't create ", ProgName);
	perror (TFName);
	cleanup ();
	/* NOTREACHED */
    }
    /* Make it rw-r--r--... very important!! */
    if (chmod (TFName, 0644)) {
	fprintf (stderr, "%s: can't chmod 0644 ", ProgName);
	perror (TFName);
	cleanup ();
	/* NOTREACHED */
    }
    fprintf (cf, "u %d %d\n", UserID, GroupID);
    fprintf (cf, "D %s\n", Device -> sd_namelist.nl_name);
#ifdef	DEBUG
    if (CopyFlag)
	fprintf (cf, "# -c\n");
    if (islink)
	fprintf (cf, "# link\n");
#endif	DEBUG
    if (MailFlag)
	fprintf (cf, "M\n");
    fprintf (cf, "U %s\n", UserName);
    if (Title)
	fprintf (cf, "T %.*s\n", sizeof dnbuf, Title);
    fprintf (cf, "< %s\n", DFName);
    fprintf (cf, "X %.*s\n", sizeof dnbuf, PrintProgram);
    fprintf (cf, "R %s\n", DFName);
    if (ferror (cf)) {
	fprintf (stderr, "%s: error making control file: ", ProgName);
	perror ("");
	cleanup ();
	/* NOTREACHED */
    }
    (void) fclose (cf);
    /* Copy data */
    if (!islink) {
#ifdef	VMUNIX
	char buf[BUFSIZ * 10];
#else	VMUNIX
	char buf[BUFSIZ];
#endif	VMUNIX
	register int n, of;

	of = creat (DFName, 0666);
	if (of < 0) {
	    fprintf (stderr, "%s: can't create ", ProgName);
	    perror (DFName);
	    cleanup ();
	    /* NOTREACHED */
	}
	(void) chown (DFName, UserID, GroupID);
	while ((n = read (f, buf, sizeof buf)) > 0) {
	    if (write (of, buf, n) != n) {
		fprintf (stderr, "%s: write error on ", ProgName);
		perror (DFName);
		cleanup ();
		/* NOTREACHED */
	    }
	}
	(void) close (of);
	if (n < 0) {
	    fprintf (stderr, "%s: read error on ", ProgName);
	    perror (name ? name : "[stdin]");
	    cleanup ();
	    /* NOTREACHED */
	}
    }
    /* Finally, move the temporary control file to a real control file */
    if (link (TFName, CFName)) {
	fprintf (stderr, "%s: can't link %s to ", ProgName, TFName);
	perror (CFName);
	cleanup ();
	/* NOTREACHED */
    }
    (void) unlink (TFName);
    StartDeSpool ();
    CFName = 0, DFName = 0, TFName = 0;
}

/* Look up a type name and convert to a program name */
char *
LookUpType (type)
register char *type;
{
    register struct proglist *p;

    if (type == (char *) 0) {
	for (p = Device -> sd_proglist; p; p = p -> pr_next)
	    if (p -> pr_shortname == (char *) 0)
		return p -> pr_longname;
	goto bad;
    }
    for (p = Device -> sd_proglist; p; p = p -> pr_next) {
	if (p -> pr_shortname == (char *) 0)
	    continue;
	if (*p -> pr_shortname == *type)
	    if (strcmp (p -> pr_shortname, type) == 0)
		return p -> pr_longname;
    }
bad:
    fprintf (stderr, "%s: generic ", ProgName);
    if (type == (char *) 0)
	fprintf (stderr, "default");
    else
	fprintf (stderr, "type %s ", type);
    fprintf (stderr, "not defined for device %s\n",
				Device -> sd_namelist.nl_name);
    cleanup ();
    /* NOTREACHED */
}

/* Must be a function -- called from readdev.c */
error (msg) char *msg; {
    fprintf (stderr, "%s: %s", ProgName, msg);
    cleanup ();
    /* NOTREACHED */
}
