/* hold.c -- temporarily disable unattended terminal.
 *
 * Written by Stuart A. Kurtz
 *	      The University of Chicago
 */

#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <strings.h>

#define bit(pos) (1 << (pos - 1))


FILE 
       *fopen();

void
	failsafe();

char
       *getlogin(),
       *getpass(),
       *crypt(),
       *malloc();

char
       *prompt_fmt = "Holding for %s -- ";

main()
{
    FILE *tty;

    struct passwd  *pwent;

    char   *prompt,	/* Used to store prompt: Holding for <uname> -- */
	   *pwbuf,	/* Here's where we store the password		*/
            trial[10],	/* We'll scarf passwd attempt in here, and	*/
           *trcy,	/* put their encrypted version here 		*/
           *uname;	/* User name.					*/

    /* Initialize */

    failsafe((int) (tty = fopen("/dev/tty","w")),
	     "Couldn't open tty\n",stderr);
    failsafe((int) (uname = getlogin()),
	     "Login name unavailable!?\n",tty);
    failsafe((int) (pwent = getpwnam(uname)),
	     "Password unavailable\n",tty);
    failsafe((int) (pwbuf = pwent->pw_passwd),
	     "Null password?!\n",tty);
    failsafe((int) (prompt = malloc((unsigned) (strlen(prompt_fmt) +
						strlen(uname)))),
             "Malloc failed\n",tty);

    (void) sprintf(prompt,prompt_fmt,uname);

    /* Only three signals can be generated from the terminal, SIGINT, SIGQUIT,
     * and SIGTSTP.  We'll only try to stop them.  Note in particular, that
     * SIGALRM is *not* set.  This is a feature.
     */

    (void) sigblock(bit(SIGINT) | bit(SIGQUIT) | bit(SIGTSTP));

    for (;;) {
	(void) strcpy (trial, getpass (prompt));
	trcy = crypt (trial, pwbuf);
	if (!strcmp (trcy, pwbuf))
	    break;
    }
}


void
failsafe(condition,message,fp)
    int condition;
    char *message;
    FILE *fp;
{
    if (condition)
        return;
    fprintf(fp,message);
    exit(1);
}
