#
/*
* nohome
* prints a list of home-directories that
* no longer exist.  typically used
* as input to rmusers
* Two outputs are produced:
*	standard output gets a list of users to be zapped
*	file 'nohome.log' lists the user's name, dir and why zap.
*
* usage:
* nohome [-m]
* where the optional flag determines the criteria for 'zapping':
*       m       lack of home directory being modified
* The lack of a home directory is always a criteria for 'zapping'.
*
* The routine stilgud() is used to determine if a user
* should be zapped.  It can be easily changed to fit any
* requirements.
*
* R.Reeves, Dec 78
*/

#include <stdio.h>
#include <passwd.h>
#include "/usr2/sys/sbuf.h"

#define PASSWORD "/etc/6passwd"		/* the password file */
#define NOHOME "nohome.log"	/* log file of dirs to be zapped */

#define ERROR -1        /* function return codes */
#define OK 0
#define BAD -1
#define GOOD 0

FILE *fpw;		/* pointer to password file */
FILE *fhm;		/* pointer to nohome file */

int mflg 0;             /* list users who haven't modified their
			 * home dir lately */

char pwline[128];	/* line from 6passwd */
int now[2];		/* time buffer */

struct passwd pastr;	/* allocate room for the structure */
struct passwd *pwptr &pastr;    /* pwptr points to structure */
struct inode sbuf;      /* struct for stating homedirectories */

main(argc,argv)
char **argv;
{

	register char *p;

	extern _sobuf[];
	setbuf(stdout,_sobuf);	/* buffer output */

	if(argc > 1) {  /* interpret flags */

		/* clear all flags */
		mflg = 0;
		p = argv[1];
		while(*p) switch(*p++) {
			case '-':
				continue;
			case 'm':
				mflg++;
				continue;
			default:
				fprintf(stderr,"Bad flag '%c'\n",*p);
				exit();
		}
	}

	/* compute modify criteria time */
	/* now[0] is high order word */

	time(now);
	now[0] = now[0] - 245;	/* 6 months ago */
	if(mflg) fprintf(stderr,"lack of use date: %s\n",ctime(now));
#ifdef DEBUG
fprintf(stderr,"DEBUG: m=%d time=%s",mflg,ctime(now));
fprintf(stderr,"DEBUG: now[0]=%d now[1]=%d\n",now[0],now[1]);
#endif

	/* open password file for reading */

	if ((fpw = fopen(PASSWORD,"r")) == NULL) {
		fprintf(stderr,"can't open password file %s\n",PASSWORD);
		exit();
	}

	/* open log file to contain homedirs not found */

	if((fhm = fopen(NOHOME,"w")) == NULL) {
		fprintf(stderr,"can't open log file %s\n",NOHOME);
		exit();
	}

	/* check each entry in password file */

	while (getone() == OK)
		stilgud();

}

getone()
{
	/* get one user from password file and parse data */
	/* return ERROR if EOF, if all ok return OK */
	/* if password parse error, exit */

	register char *p;
	p = pwline;

	/* read line from password file */

	while ((*p = getc(fpw)) != EOF) {
		if( *p == '\n') {
			*p = '\0';
			if(parsepw(pwline,pwptr)) {
				fprintf(stderr,"bad line in password file:\n%s\n",
					pwline);
				exit();
			}
			return(OK);
		}
		*p++;
	}
	return(ERROR);
}

stilgud()
{
	/* check to see if user should be zapped
	 * return GOOD if user is still ok
	 * return BAD if user should be zapped
	 *
	* currently, if the directory doesn't exist
	* it is always flagged as 'zap'.
	* inclusion of the m flag means that
	* directory which haven't been modified lately
	* will be listed as 'zap' also
	 */

	register sret;	/* stat return value */

	sret = stat(pwptr->homedir,&sbuf);

	if(sret == ERROR)  {
		printf("%s\n",pwptr->user);
		fprintf(fhm,"%s\t%s\tNo home\n",
			pwptr->user,pwptr->homedir);
		return(BAD);	/* homedir not found */
	}

	if(mflg &&      /* check modify time */
		(sret != ERROR) &&  (
		((sbuf.i_mtime[0] == now[0]) && (sbuf.i_mtime[1] < now[1])) ||
		(sbuf.i_mtime[0] < now[0])
		)  ) {
			printf("%s\n",pwptr->user);
			fprintf(fhm,"%s\t%s\tNo use\n",
				pwptr->user,pwptr->homedir);
			return(BAD);
	}

	return(GOOD);
}
/*
 * parsepw(pwline, pwptr)
 *
 * Accepts a password line (such as returned by getpw(III) and getpwn(III))
 * and breaks it up into its component fields.  The pwline is destroyed and
 * the field information returned in the structure pointed to by pwptr
 * (which is 14(10) words long and described in parsepw(III)).
 *
 * Returns 0 if successful; -1 if the pwline does not have the expected
 * format.
 *
 * RR Gomes -- 14 April 1978
 * modified Nov 78, R Reeves for Purdue system
 * This routine should work with either the standard I/O package
 * or the new I/O package.
 *
*/

parsepw(pwline, pwptr)
char *pwline;
struct passwd *pwptr;
{
	register int colons, newlines;
	register char *cp;

	/* initialize pwptr */
	pwptr->user = "";
	pwptr->pw = "";
	pwptr->uid = -1;
	pwptr->gid = -1;
	pwptr->gcos = 0;
	pwptr->homedir = "";
	pwptr->shell = "";

	colons = newlines = 0;
	for(cp = pwline; *cp != '\0'; cp++)
		if(*cp == ':')
			{
			colons++;
			*cp = '\0';
			}
		else if(*cp == '\n')
			newlines++;

	if((colons != 6) || (newlines != 0))
		return(ERROR);
	cp = pwline;
	pwptr->user = cp;
	while(*cp++);
	pwptr->pw = cp;
	while(*cp++);
	pwptr->uid = atoi(cp);
	while(*cp++);
	pwptr->gid = atoi(cp);
	while(*cp++);
	pwptr->gcos = atoi(cp);
	while(*cp++);
	pwptr->homedir = cp;
	while(*cp++);
	pwptr->shell = cp;
	return(OK);
}
