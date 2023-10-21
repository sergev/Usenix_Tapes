/*
 * checkmail: watch user(s) mailbox
 *	      By M. Ries
 *            Based upon a program of similar name,
 *            Kernighan & Pike, "The UNIX Programming Environment".
 *
 * No warranty of suitability for any purpose, either expressed
 * or implied, is made.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

char   *progname;
char   *maildir = "/usr/spool/mail";/* system dependent */

#define	OPTSTRING "bhl:n:p:s:"
#define	FOREVER for(;;) 

main (argc, argv)
int     argc;
char  **argv;
{
    struct stat buf;
    char   *name,
           *getlogin ();
    extern int  optind;
    extern char *optarg;
    int     C;
    int     lastsize = 0;
    int     nmflg  = 0;		/* new mail flag */
    int     errflg = 0;		/* error flag */
    int     bkgflg = 0;		/* background-mode flag */
    int     nvalue = 4;		/* default nice value */
    unsigned    svalue = 300;	/* default sleep interval */

    if ((name = getlogin ()) == NULL) {
	error ("* Can't get login name", (char *) 0);
    }
    while ((C = getopt (argc, argv, OPTSTRING)) != EOF)
	switch (C) {
	    case 'b': 		/* background-mode flag */
		bkgflg++;
		break;
	    case 'h': 		/* help flag */
		errflg++;
		break;
	    case 'l': 		/* login name to check */
		name = optarg;
		break;
	    case 'n': 		/* called as nice_value */
	    case 'p': 		/* called as priority? */
		nvalue = atoi (optarg);
		break;
	    case 's': 		/* sleep interval */
		svalue = atoi (optarg);
		break;
	    default: 		/* flag usage */
		errflg++;
	}
    if (errflg) {
	printf ("  USAGE:\n");
	printf ("\t-b  => place program in background mode\n");
	printf ("\t-h  => this display\n");
	printf ("\t-l  => indicate alternate login to check for new mail (default=logname)\n");
	printf ("\t-n  => indicate nice value for background execution (default=4)\n");
	printf ("\t-s  => indicate sleep interval to use in background mode (default=60)\n");
	exit (1);
    } /*endif error*/

    if (chdir (maildir) == -1)
	error ("* Can't change directory to %s", maildir);
    if (bkgflg) {
	(void) signal (SIGINT, SIG_IGN);
	(void) nice (nvalue);
	if (fork ())
	    exit (0);
    } /*endif background*/

    FOREVER {
	/* really should just loop through an argv name array */
	if (stat (name, &buf) == -1)/* no mailbox */
	    buf.st_size = 0;
	if (buf.st_size > lastsize) {
	    if (!nmflg) {/* must be old mail */
	         printf ("\n  ***** THERE IS MAIL!  (%s) *****\n", name);
		 nmflg = 1; /* set new mail flag for next pass */
            } else {
	         printf ("\n  ** NEW MAIL HAS ARRIVED! (%s) **\n", name);
            }
	}/*endif*/
	lastsize = buf.st_size;
	if (!bkgflg)
	    exit (0);
	(void) sleep (svalue);
    } /*end FOREVER*/
} /*end main*/

error (s1, s2)
char   *s1,
       *s2;
{
    extern int  errno,
                sys_nerr;
    extern char *sys_errlist[],
               *progname;

    if (progname)
	fprintf (stderr, "%s: ", progname);
    fprintf (stderr, s1, s2);
    if (errno > 0 && errno < sys_nerr)
	fprintf (stderr, " (%s)", sys_errlist[errno]);
    exit (1);
} /*end error */
