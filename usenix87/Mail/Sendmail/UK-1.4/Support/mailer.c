/*
 *  mailer  -  JNT -> Sendmail interface.
 *
 *	This program takes a mail file in JNT format and converts the
 *	Janet header into an argument and then calls sendmail giving the
 *	message part of the mail file as standard input.
 *
 *	Written by Jim Crammond.	jim@hwcs	4/85
 */	
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <tdir.h>
#include <sysexits.h>
#include "local.h"

struct	tdirent	*gettdnam(), *td;

#define	CMDSIZ		5000			/*  command line size  */
#define SENDMAIL	"/usr/lib/sendmail"	/*  sendmail rah rah  */
#define DEADLETTER	"/tmp/dead.letter"	/* somewhere for trashed mail */

char	cmd[CMDSIZ];
char	line[BUFSIZ];
char	*file, *via;
FILE	*fopen(), *popen();
FILE	*fp, *outp, *logfd, *dlp;
long	iop, time();
char	*ctime();
char	*rindex();
int	LOGGING;

main(argc, argv)
int	argc;
char	*argv[];
{
	register char	*usersp, *c, *n;
	char	*f;
	int	ret;

#ifndef DEBUG
	chdir(SPOOLD);		/* somewhere to dump core */
#endif
	if ((logfd = fopen(MAIL_LOG, "a")) == NULL)
		LOGGING = 0;
	else
		LOGGING = 1;
	chmod(MAIL_LOG, 0666);
	time(&iop);
	LOG("\n\nMail server started %s", ctime(&iop));

	file = argv[1];
	via = argv[2];

	if ((fp = fopen(file, "r")) == NULL)
	{	LOG("could not open file\n");
		done();
	}

	/*
	 *  find out where this came from
	 */
	for (f = via; *f; f++)
		if (*f == '/')
		{	*f = '\0';
			break;
		}
	if ((td = gettdnam(via, 0)) != 0)
		via = td->td_name;

	/*
	 *  initialise command sendmail command line
	 */
	sprintf(cmd, "%s -oMs%s", SENDMAIL, via);
	usersp = cmd;
	while (*usersp != '\0')
		usersp++;

	/*
	 *  get list of addresses to send to in JNT-header form and
	 *  convert to a list of command line arguments.
	 */
	while (fgets(line, BUFSIZ, fp) != NULL)
	{	c = line;
		while (*c == ' ' || *c == '\t')
			c++;

		if (*c == '\n')
			break;

		*usersp++ = ' ';

		while (*c != '\0' && *c != '\n')
		{	if (*c == ',')
			{	*usersp++ = ' ';
				c++;
			}
			else
				*usersp++ = *c++;
		}
	}
	*usersp = '\0';

	/* call sendmail */
#ifdef DEBUG
	fprintf(stderr, "call: %s\n", cmd);
	done();
#endif DEBUG
	LOG("%s\n", cmd);
	outp = popen(cmd, "w");
	while (fgets(line, sizeof(line), fp) != NULL)
		fputs(line, outp);
	ret = pclose(outp);

	if (ret & 0377)
	{	/* sendmail failed - drop letter into "dead.letter" */
		LOG("%s failed, returning status %d\n", cmd, ret);
		if ((dlp = fopen(DEADLETTER, "a")) != NULL)
		{	fprintf(dlp, "%s failed, message body follows:\n", cmd);
			while (fgets(line, sizeof(line), fp) != NULL)
				fputs(line, dlp);
		}
		exit(EX_OSERR);
	}
	LOG("exit status %d\n", ret >> 8);
	done();
}

/* VARARGS1 */
LOG(fmt, args)
char	*fmt;
{
	if (LOGGING)
	{	fprintf(logfd, fmt, args);
		fflush(logfd);
	}
	return;
}

done()
{
	unlink(file);
	exit(EX_OK);
}
