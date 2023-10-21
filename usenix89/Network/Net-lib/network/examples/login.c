#include <sys/ioctl.h>
#include <stdio.h>
#include "../netw.h"
/*
 * Gork is the routine that shows up on the other end of the pty.  As far as
 * it can tell, it is on its own terminal.  In this example,  I call
 * /bin/login so that I can log in.  Note:  On a machine where the /etc/utmp
 * file is locked up,  this sometimes leaves strange entries when you type
 * w(1) after to log back out. Don't worry,  they will go away after reboot
 * or you can use my rename(l) package to clear out the entry. 
 */

char          **env;
gork ()
{
	char           *buf[2];
	buf[0] = "login";
	buf[1] = 0;

	if (vfork () == 0)
		execve ("/bin/login", buf, 0);

	printf ("Connected.\n");

	(void) wait (0);

	/* clrutmp has no effect on machines where /etc/utmp is locked up */
	clrutmp ();

	printf ("\nDisconnected...\n");
}
int             s;
/* Read from the keyboard */
rread (fd)
	int             fd;
{
	int             n;
	char            buff[255];
	n = read (fd, buff, 255);
	write (s, buff, n);
}
main (argc, argv, argp)
	int             argc;
	char           *argv[];
char           *argp[];
{
	TCOND           tdef;

	char            buff[255];
	int             n;
	struct sgttyb   b,
	                sbuf;
	env = argp;

	loadtty (tdef);				 /* Load my terminal
						  * configuration */

	s = openpty (tdef, gork);		 /* Put the function gork on
						  * the other end of the file
						  * descripter using the
						  * terminal defined in tdef. */

	stty (tdef, RAW, 0);			 /* Change the definition to
						  * be raw */
	stty (tdef, ECHO, FALSE);		 /* Have it not echo */

	settty (tdef);				 /* Set my terminal to tdef */

	isignal (0, rread);			 /* Set rread to be called
						  * when something comes in
						  * on the keyboard */

	while (1)
	{
		n = read (s, buff, 255);	 /* Read from the keyboard */
		if (n == EOF)			 /* Has the other end closed? */
			break;			 /* Guess so */
		write (1, buff, n);		 /* Write out whats been
						  * gotten from the pty */
	}

	stty (tdef, COOKED, 0);			 /* Make tdef cooked again */
	stty (tdef, ECHO, TRUE);		 /* Turn echo back on */

	settty (tdef);				 /* Set it */
}
