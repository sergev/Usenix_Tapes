/* uumail.c --- uucp remailer
 *	EHS 4/2/85
 *      Added rebuild sentinel and better error handling 10/13/85
 *
 * Compile as:
 *	cc -O -s uumail.c -ldbm -o uumail
 *
 *  Usage:
 *    usually called from sendmail in the following form:
 *      uumail -f from addr < message
 *    "from" is the address of the sender, and is passed to "uux"
 *       in case of a remote error
 *    "addr" is an address in the following form:
 *		site!site!site!user
 *     and "user" can take on any form not containing a "!"
 *
 *     If a "-D" is used instead of a "-f", no mail will be sent
 *     and various bits of diagnostic info will be printed to the
 *     standard error output.  Messages that would normally be
 *     printed at the console are also printed to stderr, in this
 *     case.  The body of the message is also dumped to stderr.
 *
 *     At least one "!" MUST be present in the address or else it will 
 *     be considered an error.
 *
 *    The address is rewritten for the first applicable site
 *    found in the database.  If the path cannot be rewritten,
 *    an error code is returned along with a message indicating
 *    the problem.
 *
 *    If the special sentinel value of @@@ is not present in the
 *    database, then it is assumed that the database is being
 *    rebuilt and the requesting process is blocked for TIMEOUT
 *    (default = 180) seconds.  If, after 5 such blocks, the
 *    sentinel is not present, an error message is logged to
 *    the console, and the error code EX_TEMPFAIL is returned.
 *    The same is true if the dbm files cannot be initialized.
 *
 *    Note:
 * 	The "uux" flags given below are for 4.3 BSD uucp and
 *	may not exist for your version of uucp.  Note especially
 * 	that the "-L" flag may not be present in earlier versions
 *	(meaning to crank up uucico for a local call, otherwise
 *	just queue it).
 *
 *    Special defines:
 *      PATHFILE is the basename of the dbm path database.
 *      LOGF  if defined is where a log of uucp mail is kept
 *      TIMEOUT is the sleep(2) time, in seconds, to wait
 *         if the database is unavailable or incomplete.
 *      CONSOLE is the pathname of the file to report major errors
 *      MYSITE is the site name to be used in reporting errors
 *	   via returned mail; if not defined, the sitename is
 * 	   derived from a call to gethostname(2)
 *	UUX is a sprintf(3) format string used to remotely execute
 *	   the rmail command on the next system.
 *      SENTINEL is the special "sitename" to look for in the
 *	   path database to indicate a complete database
 */

#include <ctype.h>
#include <dbm.h>
#include <stdio.h>
#include <sysexits.h>

#ifndef PATHFILE
#define PATHFILE  "/usr/lib/mail/uucp.hosts"
#endif PATHFILE

#ifndef CONSOLE
#define CONSOLE   "/dev/console"
#endif CONSOLE
FILE *console;

#ifndef TIMEOUT
#define TIMEOUT ((unsigned) 180)
#endif TIMEOUT

#ifndef UUX
#define UUX "/usr/bin/uux -p -a%s -L -gM %s!rmail \\(%s\\)\n"
#endif UUX

#ifndef SENTINEL
#define SENTINEL "@@@"
#endif SENTINEL

extern char *malloc (), *rindex (), *index ();
extern char *strcpy (), *strcat ();
extern int strlen ();
extern  FILE *popen ();

datum key, result;
char    workbuf[512];
char   *destination,
       *sender;

int     debug;

#ifdef LOGF
FILE	*logfile;
#endif LOGF


main (argc, argv)
int     argc;
char  **argv;
{
    register char  *stp,
                   *rtp;
    int     indx, retval;
    extern  void die (), checkpath ();


    destination = argv[3];	/* given destination (and user) */
    sender = argv[2];		/* given sender */

    if (argc != 4)
	die ("called with incorrect number of arguments.", EX_USAGE);
    debug = (argv[1][1] == 'D');

    console = fopen (CONSOLE, "a");
    if ((console == NULL) || debug)
	console = stderr;

#ifdef LOGF
    logfile = fopen (LOGF, "a");
    if (logfile == NULL)
    {
	fprintf (console, "\n*** uumail: Unable to open logfile %s\n", LOGF);
	logfile = console;
    }
    fprintf (logfile, "%s\t%s\t", sender, destination);
#endif LOGF

    for (indx = 0; indx < 5; indx++)
    {
	if ((retval = dbminit (PATHFILE)) >= 0)
	    break;
	
	if (debug)
	    fprintf (stderr, "Database unavailable.  Sleeping.\n");
	sleep (TIMEOUT);
    }

    if (retval < 0)
	die ("could not open routing database files.", EX_TEMPFAIL);

    key.dptr = SENTINEL;
    key.dsize = strlen (SENTINEL) + 1;
    for (indx = 0; indx < 5; indx++)
    {
	result = fetch (key);
	if (result.dsize > 0)
	    break;
	
	if (debug)
	    fprintf (stderr, "Database incomplete.  Sleeping.\n");
	sleep (TIMEOUT);
    }
    if (result.dsize <= 0)
	die ("routing database files incomplete or truncated.",
		EX_TEMPFAIL);


/* Now we back up through the address until we find a site we
 * know how to reach.  If we don't find any, it's an error.
 */

    strcpy (workbuf, destination);

    if (!(rtp = rindex (workbuf, '!')))
	die ("address in improper format.", EX_DATAERR);

    *rtp = '\0';
    while (stp = rindex (workbuf, '!'))
    {
	checkpath (rtp + 1, stp + 1, (int) (rtp - stp));
	*rtp = '!';
	rtp = stp;
	*rtp = '\0';
    }
    checkpath (rtp + 1, workbuf, (int) (rtp - workbuf) + 1);

 /* If we got to here, we don't have a path */

    *rtp = '!';
    die ("Unable to find path to any host in pathname.", EX_NOHOST);
}


/*  This routine does all the work.  If it finds a path it immediately
 *  will go ahead and do the mailing and exit.
 */

void checkpath (user, site, len)
char   *user,
       *site;
int     len;
{
    FILE * pipfd;
    int     comlen;
    char   *address,
           *restol,
           *command;

    key.dptr = site;
    key.dsize = len;
    result = fetch (key);
    if (result.dsize <= 0)
	return;	   /* result <= 0 implies no match */

 /* rewrite here */
    comlen = strlen (user) + result.dsize;
    address = malloc ((unsigned) comlen);
    if (address == NULL)
	die ("malloc cannot get memory for new address.\n", EX_SOFTWARE);

    sprintf (address, result.dptr, user);
#ifdef LOGF
    fprintf (logfile, "%s\n", address);
#endif LOGF
    comlen = strlen (address) + strlen (UUX) + 4;
    command = malloc ((unsigned) comlen);
    if (command == NULL)
	die ("malloc cannot get memory for uux command line.\n", EX_SOFTWARE);

    if ((restol = index (address, '!')) != NULL)
	*restol++ = '\0';
    sprintf (command, UUX, sender, address, restol);
    if (debug)
    {
	fprintf (stderr, "Command that would be executed: %s\n", command);
	pipfd = stderr;
    }
    else
    {
	pipfd = popen (command, "w");
	if (pipfd == NULL)
	    die ("cannot open pipe with popen(3).", EX_SOFTWARE);
    }

    while (fgets (workbuf, sizeof (workbuf), stdin))
	fputs (workbuf, pipfd);

    comlen = debug == 0 ? pclose (pipfd) : 0;
    if (comlen)
    {
	sprintf (workbuf, "execution of uux returned with error status %d",
	    comlen);
	die (workbuf, EX_UNAVAILABLE);
    }

    exit (EX_OK);
}


void die (message, errcode)
char   *message;
int     errcode;
{
#ifdef MYSITE
    char   *mysite = MYSITE;
#else
    char    mysite[64];
    gethostname (mysite, 64);
#endif


#ifdef LOGF
    fprintf (logfile, "Error: %s\n", message);
#endif LOGF

    fprintf (console, "\n\07*** Error in uumail!\n");
    fprintf (console, "    %s\n", message);
    fprintf (console, "    Mail from %s to %s being returned.\n\n", sender,
	    destination);

    fprintf (stderr, "Mailer at \"%s\": %s\n", mysite, message);
    exit (errcode);
}
