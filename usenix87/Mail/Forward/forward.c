/* %M%	%R%	%G%	%T%	*/
/* mail forwarding service */
/* Programmed 3/29/84 by: dan sunday, JHU/APL (aplvax) */
/* modified 6/12/84 - matt diaz (aplvax) */
/*
	- scan /usr/spool/mail for pending mail
	- for each user found:
		1) find his home from /etc/passwd
		2) chk his home for a .forward file
	- for each .forward file:
		1) forward his mail to the machine named in .forward
		2) rm his mail file after forwarded
*/


#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

#define MAILDIR	"/usr/spool/mail"
#define MSIZE 256
#define FORWARD ".forward"	/* name of forward file */

char	*postoffice = MAILDIR;	/* mail directory */
int	pofd;			/* postoffice file descriptor */
struct direct	maildir;
struct stat	mailstat;	/* user's mail status */
char	user [MSIZE];		/* user name in postoffice */
char	usermail [MSIZE];	/* user mail file name */
char	home [MSIZE];		/* user's home from /etc/passwd */
char	sendto [MSIZE];		/* system to send mail to */
char	tempmail [MSIZE];	/* temp file for forwarding */
char	fwdcmd [MSIZE];		/* the actual mail forwarding command */

main()
{
	struct passwd	*pp;
	FILE	*fwf;		/* forward file */

	if ((pofd=open(postoffice,0)) < 0) {
		fprintf(stderr,"can't open %s\n", postoffice);
		exit(1);
	}

	sprintf( tempmail, "%s/%s", MAILDIR, "fmXXXXXX");
	mktemp(tempmail);
	while (read(pofd, &maildir, sizeof(maildir))) {
		if (maildir.d_ino == 0)
			continue;

		if (maildir.d_name[0] == '.')
			continue;

		/* get user name */
		strncpy(user, maildir.d_name, 14);

		/* make sure there is some mail */
		sprintf(usermail, "%s/%s", postoffice, user);
		stat(usermail, &mailstat);
		if (mailstat.st_size == 0)
			continue;		/* no mail */

		/* get user's home from /etc/passwd */
		if ((pp = (struct passwd *)getpwnam(user)) == NULL)
			continue;

		/* check for a forwarding address */
		sprintf(home, "%s/%s", pp->pw_dir, FORWARD);
		if ((fwf = fopen(home, "r")) == NULL)
			continue;

		fgets(sendto, MSIZE, fwf);
		sendto [strlen(sendto) - 1] = '\0';
		fclose(fwf);

		/* set up the mail forwarding command */
		sprintf(fwdcmd,"/bin/mv %s %s", usermail, tempmail);
#ifdef DEBUG
		printf("CMD: %s\n", fwdcmd);
#endif	
		/* move off to avoid mail interference */
		system( fwdcmd );

		sprintf(fwdcmd,"/bin/mail %s <%s",
			sendto, tempmail);
#ifdef DEBUG
		printf("CMD: %s\n", fwdcmd);
#endif
		/* do it */
		system( fwdcmd );
		sleep(7);	/* mksure it's done */

		/* and then */
		unlink(tempmail);
	}
	exit(0);
}


