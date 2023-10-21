/*% cc -O news.c -lc							*/

/* routines to perform the functions of bug, gripe, local, and news	*/
/* (herein referred to as the Gang of Four)				*/
/*									*/
/* written by chris leyerle, csrg, august 1980, for version 7 UNIX	*/
/* Mod. April 6/81 - strip leading '-' from argv[0] so this program
 *		may be used as a login shell.
 * 			B. Thomson.
 */

#include	<stdio.h>
#include	<signal.h>

#define	MAIL	"/bin/mail"
#define	USAGE	"Usage:  %s [spo]\n"

char	maildir[]	= "/usr/spool/mail/";
char	oldmaildir[]	= "/usr/spool/mail.old/";
char	*mailfile;	/* (doubles as the old mail file too!)		*/



main (argc, argv)
	int	argc;
	char	**argv;

	{
	char	option;			/* option flag of invocation	*/
	char	*getname();		/* strips pathname of invocation*/
	char	*strcat();		/* string concatenation		*/

	signal (SIGHUP, 0);
	signal (SIGINT, 0);
	signal (SIGQUIT, 1);

	argv[0] = getname (argv[0]);

	switch (argc)
		{
		case 1:
			option = 'p';
			break;
		case 2:
			option = *(argv[1]);
			break;
		default:
			fprintf (stderr, USAGE, argv[0]);
			exit (1);
		};

	switch (option)
		{
		case 'p':
			mailfile = strcat (maildir, argv[0]);
			execl (MAIL, MAIL, "-p", "-f", mailfile, '\0');
			break;
		case 's':
			execl (MAIL, MAIL, argv[0], '\0');
			break;
		case 'o':
			mailfile = strcat (oldmaildir, argv[0]);
			execl (MAIL, MAIL, "-p", "-f", mailfile, '\0');
			break;
		default:
			fprintf (stderr, USAGE, argv[0]);
			exit (1);
		};
	fprintf (stderr, "Can't exec mail\n");  /* of all the darned things...*/
	exit (1);
	};


char *getname (ptr)
	register char *ptr;		/* pathname to be stripped	*/

	{
	register char	*r2;		/* working pointer		*/
	char		*rindex();	/* find last occurrence function*/
	int		strlen();	/* string length function	*/

	for (r2 = ptr + strlen(ptr)-1; *r2 == '/'; r2--)
		;			/* strip trailing slashes	*/
	*++r2='\0';
	if ((r2 = rindex (ptr, '/')) == 0)
		r2 = ptr;
	if(*r2 == '-')
		/* Strip leading '-' prepended by login(1)
		 * if we were invoked as a nonstandard login shell.
		 */
		r2++;
	return(r2);
	};
