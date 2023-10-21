/*
 * Notify user of new mail;
 *
 *	CopyRight (c) 1985 Shawn F. Mckay, All Rights Reserved.
 *
 * Permission is granted for NON-PROFIT use of any kind.
 *
 * Date: 15-May-1985
 * Author: Shawn F. Mckay (mit-eddie!shawn)
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	BELL	(037 & 'G')

int quit ();
long touched();
char	*getlogin();
char	uname[80];

/*
 * Format:
 *
 * % mailer [sleep-time-in-seconds]
 */

main (argc, argv)
int argc;
char **argv;
{
	int	fh = 0;
	int	Sleep_t = 45;
	long	when = 0;
	long	now  = 0;
	char	*temp = NULL;
	char	fname[80];
	register int i = 0;

	if (argc > 1)
		Sleep_t = atoi (argv[1]);

	fh = fork();
	if (fh) exit (-1);

	if ((temp = getlogin()) == NULL) {
		printf ("%s: Unable to find your username, aborting.\n",
		argv[0]);
		exit (-1);
	} else
		strcpy (uname, temp);

	sprintf (fname, "/usr/spool/mail/%s", uname);

	signal (SIGHUP, quit);
	
	for (i=SIGINT;i < SIGPROF+1;++i)
		signal (i, SIG_IGN);

	when = touched (fname);

	for (;;) {
		sleep (Sleep_t);
		if (strcmp (uname, getlogin()))
			exit (-1);
		if ((now = touched (fname)) == 0)
			continue;
		if (now != when) {
			when = now;
			msg ();
		}
	}
}

/*
 * Touched: Return last access time
 */

long touched (fname)
char *fname;
{
	struct	stat st;
	
	if (stat (fname, &st) == -1) {
		printf ("The file \"%s\" was not found.\n", fname);
		printf ("Mail watch is now terminated.\n");
		exit (-1);
	}
	
	if (st.st_size < 1)
		return (0);

	return (st.st_mtime);
}

msg ()
{
	printf ("\n%c[New mail has arrived for %s]\n",
	BELL, getlogin());
	return (1);
}

quit ()
{
	exit (0);
}
