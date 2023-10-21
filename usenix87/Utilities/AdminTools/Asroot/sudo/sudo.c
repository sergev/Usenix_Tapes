#ifndef lint
static char *sccsid = "@(#)sudo.c	1.4 (SUNYAB CS) 12/14/85";
#endif

/*
 * sudo - run a command as superuser
 *
 * Usage: sudo [command]
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <utmp.h>
#include <pwd.h>
#include <ctype.h>

#define SUDOFILE	"/usr/local/adm/sudoers"
#define	LOGFILE		"/usr/local/adm/sudo.log"
#define	BLOGFILE	"/usr/local/adm/sudo.log.failures"

#define TRUE 1
#define FALSE 0

char    buf[BUFSIZ];
char   *bufp = buf;
char   *index (), *malloc ();

struct passwd  *pw, *getpwuid ();

main (argc, argv, envp)
int     argc;
char   *argv[], *envp[];
{
    char   *name, bufcmd[BUFSIZ], *origpath;
    int found;
    pw = getpwuid (getuid ());
    name = pw -> pw_name;
    if (setuid (0) == -1) {
	perror (*argv);
	exit (1);
    }
    argc--, argv++;
    found = lookup (name);
    if (argc == 0) {
	if (*buf)
	    printf ("%s\n", buf);	/* echo permissions, if any */
	exit (0);
    }
    if (!found) {			/* not sudo permitted */
	enterlog (BLOGFILE, name, argc, argv);
	fprintf (stderr, "Nope.\n");
	exit (1);
    }
    enterlog (LOGFILE, name, argc, argv);
    bufp = index (bufp, ' ');
    (void) sscanf (bufp, "%s", bufcmd);
    if (strcmp (bufcmd, "all")) {
	if (!strncmp (bufcmd, "PATH", 4) && index (bufcmd, '=')) {
	    origpath = envp[2];
	    envp[2] = bufcmd;
	    execvp (*argv, argv);	/* return when cmd not in PATH*/
	    envp[2] = origpath;
	}
	if (!inlist (argv)) {
	    enterlog (BLOGFILE, name, argc, argv);
	    fprintf (stderr, "Nope, you're not allowed to do that\n");
	    exit (1);
	}
    }
    execvp (*argv, argv);
    enterlog (BLOGFILE, name, argc, argv);
    printf ("%s: Command not found\n", *argv);
    exit (1);
}

lookup (name)
char   *name;
{
    register int    namelen;
    register    FILE * fp;
    namelen = strlen (name);
    if ((fp = fopen (SUDOFILE, "r")) == 0) {
	perror (SUDOFILE);
	exit (1);
    }
    while (getbuf (buf, BUFSIZ, fp) > 0) {
	if (!strncmp (buf, name, namelen)) {
	    (void) fclose (fp);
	    return (TRUE);
	}
    }
    (void) fclose (fp);
    return (FALSE);
}

getbuf (buffer, buflim, fp)
char   *buffer;
int     buflim;
FILE * fp;
{
    char    ch, eof=EOF;
    int     buflen = 0;
    while (buflen < buflim && ((ch = getc (fp)) != eof))
	if ((ch == '\n') && buflen && (!isspace (ungetc (getc (fp), fp)))) {
		*buffer = '\0';
		return (buflen);
	}
	else {
	    if (isspace (ch))
		ch = ' ';
	    buflen++;
	    *buffer++ = ch;
	}
    *buffer = '\0';
    return (buflen);
}

inlist (cmd)
char  **cmd;
{
    int     cmdlen;
    char    bcmd[BUFSIZ];
    cmdlen = strlen (*cmd);
    while (bufp = index (bufp, ' ')) {
	(void) sscanf (++bufp, "%s", bcmd);
	if (!strncmp (bcmd, *cmd, cmdlen)) {
	    if (bufp = index (bufp, '='))
		(void) sscanf (++bufp, "%s", *cmd); 	/* substitute a path */
	    return (TRUE);
	}
    }
    return (FALSE);
}

enterlog (logfile, name, argc, argv)
char   *logfile, *name;
int     argc;
char  **argv;
{
    char   *s;
    time_t  now;
    FILE * fp;
    now = time ((time_t *) 0);
    fp = fopen (logfile, "a");
    s = (char *) ctime (&now);
    fprintf (fp, "%-18.20s: %10s: ", s, name);
    while (argc--)
	fprintf (fp, "%s ", *argv++);
    fprintf (fp, "\n");
    (void) fclose (fp);
}
