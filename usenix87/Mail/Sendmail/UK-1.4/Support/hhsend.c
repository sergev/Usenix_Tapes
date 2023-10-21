/*
 *  hhsend  -  Sendmail -> JNT mailer interface.
 *
 *	This program takes a hostname and a list of users to send to as
 *	arguments and a mail message as standard input and creates a file
 *	containing a Janet header followed by the mail message, then
 *	calls mhhcp to send it over the network.
 *
 *	Written by Jim Crammond.	jim@hwcs	4/85
 */	
#include <stdio.h>
#include <signal.h>
#include <sysexits.h>
#include <sys/wait.h>
#include "local.h"

#define	MAILMODE	(0660)		/* mode of created spool file */

#ifdef DEBUG
#undef  SPOOLD
#define SPOOLD "."
#endif DEBUG

FILE	*mailf = NULL;
char	sfile[ sizeof(SPOOLD) + 14 ];
int	delete();
char	*index();

main(argc, argv)
int	argc;
char	*argv[];
{
	char	*myname;
	char	*host;
	char	**users;
	int	nusers;

	myname = argv[0];

	if (argc < 3)
	{	fprintf("usage: %s host users...\n", myname);
		exit(EX_USAGE);
	}

	host = argv[1];
	users = &argv[2];
	nusers = argc - 2;

	/*  catch signals  */
	signal(SIGHUP, delete);
	signal(SIGINT, delete);
	signal(SIGQUIT, delete);
	signal(SIGTERM, delete);

	/*  send the mail  */
	sendmail(host, users, nusers);

	/*  NOTREACHED  */
}

/*
 *  sendmail  -  creates a mail file for mhhcp and exec mhhcp.
 */
sendmail(host, users, nusers)
char	*host;
char	*users[];
int	nusers;
{
	register char	*auser, *p;
	char	line[BUFSIZ];
	union	wait status;

	/*  create spool file  */
	sprintf(sfile, "%s/M%06d", SPOOLD, getpid() );

	mailf = fopen(sfile, "w");
	if (mailf == NULL)
	{	fprintf(stderr, "cannot create mail spool file %s\n", sfile);
		exit(EX_CANTCREAT);
	}
	chmod(sfile, MAILMODE);


	/*  write out JNT-header  */
	while (nusers > 1)
	{	auser = users[ --nusers ];
		/* convert any '@'s in user part to '%'s */
		while ((p = index(auser, '@')) != NULL)
			*p = '%';

		fprintf(mailf, "%s@%s,\n", auser, host);
	}
	auser = users[0];
	/* convert any '@'s in user part to '%'s */
	while ((p = index(auser, '@')) != NULL)
		*p = '%';

	fprintf(mailf, "%s@%s\n\n", auser, host);


	/*  write out header + message from stdin (supplied by sendmail)  */
	while (fgets(line, sizeof(line), stdin) != NULL)
		fputs(line, mailf);
	
	fclose(mailf);

#ifdef DEBUG
	fprintf(stderr,"spool file is %s\n", sfile);
	exit(0);
#endif

	switch (fork())
	{	case -1:
			exit(EX_TEMPFAIL);
		case 0:
			execl(MHHCPPATH, MHHCP, sfile, host, 0);
			exit(EX_UNAVAILABLE);
		default:
			wait(&status);
			if (status.w_retcode == 1)
				exit(EX_OK);
			else
				exit(EX_NOHOST);
	}
}


/*
 *  delete  -  delete mail file and exit.  Called on any interrupt.
 */
delete()
{	
	if (mailf != NULL)
	{	fclose(mailf);
		unlink(sfile);
	}
	exit(EX_TEMPFAIL);
}
