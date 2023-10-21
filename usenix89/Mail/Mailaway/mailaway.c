/* mailaway.c -*-update-version-*-
** program to auto execute some shell script for each mail message waiting
** HFVR VERSION=Wed Mar 19 08:12:25 1986
**
** Modified by Ken Arnold, 8 April 1986 to:
**	(a)	Work with Berkeley Mail program
**	(b)	Run under non-sysV systems (only tested on 4.3bsd)
**	(c)	Add a -m flag to specifiy an alternate mail file
**	(d)	Handle general net addressing (at least normal ones)
**	(e) 	Fix a bug
**	(f)	Optimize a couple of things
** Called version 0.9.1
*/

#include 	<stdio.h>
#include	<ctype.h>
#include	<pwd.h>

#define equal		!strcmp
#define equaln		!strncmp
#define	TRUE	1
#define	FALSE	0
#define	TOSIZE		512		/* size of buffer to hold dests */
#define MAXLINES	50		/* max nr of from lines */
#define LINELENGTH	256		/* max length from line */

#ifndef SYSV
#define	MAILPROG	"/usr/ucb/mail"	/* where Berkeley mail program lives */
#define	strchr		index
#define	strrchr		rindex
#endif

char	fflag[LINELENGTH];	/* file to include */
char	xflag[LINELENGTH];	/* program to execute file (executor)*/
char	oflag[LINELENGTH];	/* options to program */
char	mflag[LINELENGTH];	/* alternate mail file to use */
int	dflag = FALSE;		/* to debug */

char 	TEMPA[]		= "/usr/tmp/AXXXXXX";	/* message file */
char	TEMPM[]		= "/usr/tmp/MXXXXXX";	/* copy of whole $MAIL */
char	TEMPS[]		= "/usr/tmp/SXXXXXX";	/* file to execute */
char	NAME[]		= "mailaway";		/* program name */
char 	subject[]	= "Subject:";		/* start of subject line */
char	rmtfrom[]	= " remote from ";
char	fwrdmsg[]	= " forwarded by ";
char	from[] 		= "From ";

FILE *f1;				/* input file */
char *lp;				/* pointer for reading file */
char	line[LINELENGTH];		/* line last read from f1 */
int frcnt;				/* nr of from lines in message */
char	frlist[MAXLINES][LINELENGTH];	/* from lines in message */

/* variables to determine from message */
char	SUBJECT[LINELENGTH];	/* whats behind Subject: */
char	SENDER[LINELENGTH];	/* login name of sender */
int	NROFLINES;		/* length of message */
int	SIZE;			/* size in chars. of message */
char	VIA[TOSIZE];		/* full return mail address */
char	MESSAGE[LINELENGTH];	/* filename where message is saved */
char	DEMON[6];		/* TRUE or FALSE */
char	LAST[6];		/* TRUE or FALSE */

/* EXTERNAL ROUTINES FROM OUR LIBRARIES */
extern	char	*strchr(/* s , c */);
/*	char	*s;
**	int	c;
**
** Returns a pointer to the first occurence of character s in string s,
** or a NULL pointer if c does not occur in the string s. 
** The null character terminating the string is considered part of the string.
*/

extern	char	*strrchr(/* s , c */);
/*	char	*s;
**	int	c;
**
** Returns a pointer to the last occurence of character s in string s,
** or a NULL pointer if c does not occur in the string s. 
** The null character terminating the string is considered part of the string.
*/

extern char	*getenv(/* sp */);
/*	char	*sp;
**
** Returns a pointer to the value of the environment variable named by
** "sp".  If no such variable is named, it can return either NULL or
** "", depending on the system.
*/

/* END OF EXTERNAL ROUTINES */

void	finit();

char	*one_in();

/* sindex looks for an occurance  of the string s2 in the string
** s1. If s2 does not ocurr, sindex returns -1. Otherwise, sindex
** returns the integer offset from s1 where s2 ocurrs.
*/
int sindex(s1,s2)
register char *s1;
register char *s2;
{	
	register char *p1;
	register char *p2;
	register int flag;
	register int ii;

	p1 = &s1[0];
	p2 = &s2[0];
	flag = -1;
	for(ii = 0; ; ii++) {
		while(*p1 == *p2) {
			if(flag < 0) flag = ii;
			if(*p1++ == 0) return(flag);
			p2++;
		}
		if(*p2 == 0) return(flag);
		if(flag >= 0) {
			flag = -1;
			p2 = &s2[0];
		}
		if(*s1++ == 0) return(flag);
		p1 = s1;
	}
}/*sindex*/

/* returns pointer to string with sender address in it
** input is frlist[] with lines like:
**
** [>]From USER DATE 				or
** [>]From USER DATE forwarded by WHOCARES	or
** [>]From USER DATE remote from MACHINE
**
** some lines might be duplicated
** we save only the last USER and string together all MACHINEs
** except if the previous machine equals the current one
** forwarded by lines are simply ignored
** address format is mach!mach!....!user or user
** if flg TRUE then it is also printed to stdout
*/
char *frflsh(flg)
{	
	static char address[TOSIZE] ;
	char user[80];
	char mach1[80]  ;	/* previous machine name */
	char mach2[80]  ; 	/* current machine name */
	register int i;
	register int j;
	register char *p;
	register char *q;

	strcpy(address,"");
	strcpy(mach1,"");

	for (i = 0 ; i < frcnt ; i++) { /* examine every From line */
		p = frlist[i] ;
		if ( *p == '>' ) p++;	/* skip leading > */
		if ( equaln(p, from , 5 ) ) {
			p += 5;			/* skip FromSP */
		} else {
			continue; /* does not match FromSP , try next line */
		}

		/* get user from line and skip past it */
		/* but first skip past spaces and tabs */
		while ( isspace(*p) )
			p++;
		q = user;
		while ( *p != '\0' && !isspace(*p))
			*q++ = *p++;
		*q = '\0';

		/* get machine name */
		strcpy(mach1,mach2);	/* save previous name */

		/* see if 'remote from' in line */
		j = sindex(frlist[i], rmtfrom) ;
		if ( j == -1 ) continue ; /* no so try next line */

		p = &frlist[i][j+sizeof(rmtfrom) -1];	/* skip past remote from */
		while ( *p != '\0' && isspace(*p))	/* skip spaces */
		q = mach2;
		while ( *p != '\0' && !isspace(*p))
			*q++ = *p++;
		*q = '\0';

		/* append to address if different from previous machine mach1 */
		if ( equal(mach1,mach2) ) {
			continue;	/* equal so skip it */
		} else {
			strcat(address,mach2);
			strcat(address,"!");
		}
	}/* for */

	strcat(address,user);
	if (flg) printf("Sender=%s\n",address);
	return(address);
}/*frflsh*/

/* Determine if line is genuine article.
** Should be of 2 forms:
** From.*[0-9][0-9]:[0-9][0-9].*
** 	or
** >From.*[0-9][0-9]:[0-9][0-9].*
**
** The [0-9] stuff is the date representation by ctime.
** Some systems use only the 09:43 part of the time (i.e.
** no seconds representation).
*/
Isfrom(line)
register char *line;	/* line to parse */
{	
	register char *p;

	if(equaln(line, from, 5))
	{
		if((p=strchr(line, ':')) != NULL)
		{
colontest:
			if(	isdigit(p[-2]) &&
			    isdigit(p[-1]) &&
			    isdigit(p[1]) &&
			    isdigit(p[2]))      return(1);
		}
		/*
		 *	If more colons, try again.
		 */
		if(p=strchr(++p, ':'))
			goto colontest;
	}
	return(0);
}/*Isfrom*/

/* fillfrom: fills frlist[] with FROM lines and sets frcnt to nr of lines 
** also set all mail variables
*/
int fillfrom()
{ 	
	char *ptr;
	register char	*sp;
	auto char	tmp[sizeof VIA];

	frcnt = 0;
	/* set all message variables to defaults */
	strcpy(SUBJECT,"");
	strcpy(SENDER,"");
	NROFLINES = 0;
	SIZE = 0;
	strcpy(VIA,"");
	strcpy(MESSAGE,"");
	strcpy(DEMON,"FALSE");
	strcpy(LAST,"FALSE");

	/* skip until From is found */
	while ( lp != NULL && !Isfrom(line)) {
		lp = fgets(line, LINELENGTH, f1);
	}
	if ( NULL == lp ) return(FALSE);	/* EOF */

	/* copy in first From line */
	strcpy(frlist[frcnt] , line);
	NROFLINES++;
	SIZE = SIZE + strlen(line);
	frcnt++;

	/* copy in >From lines */
	lp = fgets(line, LINELENGTH, f1);
	while ( lp != NULL && Isfrom(line+1)) {
		strcpy(frlist[frcnt] , line);
		NROFLINES++;
		SIZE = SIZE + strlen(line);
		frcnt++;
		lp = fgets(line, LINELENGTH, f1);
	}
	if  ( NULL == lp ) return(FALSE);	/* EOF */
	strcpy(VIA, frflsh(0));	/* get VIA */

	/* get SENDER */
	strcpy(tmp, VIA);
	ptr = tmp;
	if (one_in(tmp, "!%@") != NULL)
		while (two_in(ptr, "!%@")) {
			if (*ptr == '@') {	/* @mach:<rest of route> */
				ptr++;
				while (*ptr != ':' && *ptr != '@')
					ptr++;
				if (*ptr == '@') /* handle @mach1@mach2 */
					ptr--;
				else
					ptr++;	/* skip over ':' */
			}
			else if ((sp = strrchr(ptr, '@')) != NULL)
				*sp = '\0';	/* strip off trailing name */
			else if ((sp = strrchr(ptr, '%')) != NULL)
				*sp = '\0';	/* strip off trailing name */
			else {
				while (*ptr != '!')
					ptr++;
				ptr++;		/* skip over the ! or % */
			}
		}
	strcpy(SENDER, ptr);

	/* determine if demon, default = FALSE */
	if ( (equal (ptr , "**NSC**" ))  ||
	    (equaln(ptr , "NUSEND",6))  ||
	    (equal (ptr , "uucp"    ))  ||
	    (equal (ptr , "**RJE**" ))  ||
	    (equal (ptr , "root"    ))  ||
	    (equal (ptr , "nhcms"   ))  ||
	    (equal (ptr , "demon"   ))  ||
	    (equal (ptr , "deamon"  ))  ||
	    (equal (ptr , "daemon"  ))
	    ) {
		strcpy(DEMON,"TRUE");
	}/*fi*/

	/* now look through text until subject or start next message */
	while ( lp != NULL && !Isfrom(line)) {
		if (equaln(line,subject,strlen(subject))) {
			/* skip past spaces */
			ptr = &line[strlen(subject)];
			while (*ptr == ' ') {
				ptr++;
			}
			strcpy(SUBJECT,ptr);
			SUBJECT[strlen(SUBJECT)-1] = '\0'; /* take of CR */
			break;
		}/*fi*/
		NROFLINES++;
		SIZE = SIZE + strlen(line);
		lp = fgets(line, LINELENGTH, f1);
	}

	/* now just look through text until start of next message or EOF */
	while ( lp != NULL && !Isfrom(line)) {
		NROFLINES++;
		SIZE = SIZE + strlen(line);
		lp = fgets(line, LINELENGTH, f1);
	}

	if  ( NULL == lp ) strcpy(LAST,"TRUE");	/* EOF means last message */
	return(TRUE);
}/*fillfrom*/

/* copy mail file to TEMPM so we can process it */
void readmail()
{
	char cmd[LINELENGTH];

	mktemp(TEMPM);
	unlink(TEMPM);
#ifdef SYSV
	sprintf(cmd, "/bin/mail -p -r -f %s > %s", mflag, TEMPM);
#else
	sprintf(cmd, "cp %s %s", mflag, TEMPM);
#endif
	if (dflag) printf("cmd=%s\n",cmd);
	system(cmd);
}/*readmail*/

/* check to see if message was saved okay */
chkmsg()
{
	FILE *f2;

	f2 = fopen(MESSAGE,"r");
	if (f2 == NULL) {
		fprintf(stderr,"%s: ERROR cannot open message file %s\n",NAME,
			MESSAGE);
		exit(1);
	}
	fclose(f2);
}/*chkmsg*/

/* save message into temp file after deleting previous version */
#ifndef SYSV
/*
 * We don't use sockets here so that it will work on any UNIX system
 * which has Berkeley Mail.  Portability is more important than the
 * efficieny and elegance of sockets.
 */
#endif
savemsg()
{
	char cmd[LINELENGTH];
#ifndef SYSV
	register char	c;
	auto int	i;
	static int	message_no = 1;
	static int	pto[2] = { -1, -1 };	/* pipe to mail program */
	static int	pfrom[2] = { -1, -1 };	/* pipe to mail program */
	static FILE	*pmail[2] = { NULL, NULL };
#endif

	unlink(MESSAGE);	/* previous message*/
	strcpy(MESSAGE,TEMPA);
	mktemp(MESSAGE);
	unlink(MESSAGE);
#ifdef SYSV
	sprintf(cmd,"echo 's %s' | /bin/mail -r >/dev/null",MESSAGE);
#else
	/*
	 * If we haven't yet set the mail program running, do so, and
	 * set up the pipes in both directions.
	 *	pmail[0]:	input to the Mail program
	 *	pmail[1]:	output from the Mail program
	 */

	if (pmail[0] == NULL) {
		if (pipe(pto) < 0 || pipe(pfrom) < 0)
			error("cannot set up pipe");
		if ((i = fork()) < 0)
			error("cannot fork mail program");
		if (i == 0) {		/* child process */
			(void) close(fileno(stdin));
			(void) dup(pto[0]);
			(void) close(fileno(stdout));
			(void) dup(pfrom[1]);
			(void) close(pto[0]);
			(void) close(pto[1]);
			(void) close(pfrom[0]);
			(void) close(pfrom[1]);
			(void) execl(MAILPROG, "Mail", "-f", TEMPM, NULL);
			perror(MAILPROG);
			exit(-1);
		}
		/* parent process */
		pmail[0] = fdopen(pfrom[0], "r");
		pmail[1] = fdopen(pto[1], "w");
		(void) close(pfrom[1]);
		(void) close(pto[0]);

		/*
		 * wait until it's ready for commands
		 */

		/* "Mail verssion ..." */
		if (fgets(cmd, sizeof cmd, pmail[0]) == NULL)
			error("premature EOF from mail file");
		/* "<file name> <num> messages ... " */
		if (fgets(cmd, sizeof cmd, pmail[0]) == NULL)
			error("premature EOF from mail file");
		/* pick the number out of the line */
		if (sscanf(cmd, "\"%*[^\"]\": %d", &i) != 1)
			error("badly formed message count line");
		/* toss away one line per message */
		while (i--)
			if (fgets(cmd, sizeof cmd, pmail[0]) == NULL)
				error("premature EOF from mail file");
	}

	sprintf(cmd, "s %d %s\n", message_no++, MESSAGE);
#endif
	if (dflag) {
		printf("UNEXECUTED cmd=%s\n",cmd);
	} else {
#ifdef SYSV
		system(cmd);
#else
		/*
		 * Put the command down the pipe, and then wait for the
		 * response.  Mail will only generate one-line responses
		 * to the commands we give it.
		 */
		(void) fputs(cmd, pmail[1]);
		(void) fflush(pmail[1]);
		fgets(cmd, sizeof cmd, pmail[0]);
#endif
		chkmsg();
	}/*fi*/
}/*savemsg*/

void init()
{
	readmail();
	/* open mail and read first line */
	f1 = fopen(TEMPM,"r");
	if ( NULL == f1 ) {
		fprintf(stderr,"%s: ERROR cannot open copy of mail file\n",NAME);
		exit(1);
	}
	lp = fgets(line, LINELENGTH, f1);
}

void finit()
{
	unlink(MESSAGE);
	unlink(TEMPM);
}/*finit*/

void usage()
{
	printf("Usage: %s [-d] [-V] [-H] [-f input] [-x prgm] [-o options]\n",NAME);
}/*usage*/

/* parse options */
void options(argc,argv)
int	argc;
char	*argv[];
{
	int	ch;
	extern char *optarg;

	while ((ch=getopt(argc,argv,"vVhHdf:x:o:m:")) != EOF) {
		switch (ch) {
		case 'V' :
		case 'v' :
			printf("%s: version 0.9.1\n",NAME);
			exit(0);
			break;
		case 'h' :
		case 'H' :
			usage();
			exit(0);
			break;
		case 'd' :
			dflag = TRUE;
			break;
		case 'f' :
			sscanf(optarg,"%s",fflag);
			break;
		case 'x' :
			sscanf(optarg,"%s",xflag);
			break;
		case 'o' :
			strcpy(oflag,optarg);
			break;
		case 'm' :
			strcpy(mflag,optarg);
			break;
		case '?' :
		default :
			usage();
			exit(1);
			break;
		}/*switch*/
	}/*while*/
}/*options*/

/* return pointer to copy of input but with special characters backslashed */
char *unquote(input)
register char input[];
{
	register int i;
	register int j;
	static char output[TOSIZE];

	i = 0;
	j = 0;
	while (input[i] != '\0') {
		if ( (input[i] == '\"') || 
		    (input[i] == '\\') ||
		    (input[i] == '\$') ||
		    (input[i] == '\`') ) {
			output[j] = '\\';
			j++;
		}
		output[j] = input[i];
		j++;
		i++;
	}/*while*/
	output[j] = '\0';
	return(output);
}/*unquote*/

/* put all variables into file */
void printit(file)
char file[];
{
	FILE *f2;

	f2 = fopen(file,"w");
	if (f2 == NULL) {
		fprintf(stderr,"%s: ERROR cannot create work file",NAME);
		exit(1);
	}
	fprintf(f2,"SUBJECT=\"%s\" ; export SUBJECT\n",unquote(SUBJECT));
	fprintf(f2,"SENDER='%s' ; export SENDER\n",SENDER);
	fprintf(f2,"NROFLINES='%d' ; export NROFLINES\n",NROFLINES);
	fprintf(f2,"SIZE='%d' ; export SIZE\n",SIZE);
	fprintf(f2,"VIA='%s' ; export VIA\n",VIA);
	fprintf(f2,"MESSAGE='%s' ; export MESSAGE\n",MESSAGE);
	fprintf(f2,"DEMON='%s' ; export DEMON\n",DEMON);
	fprintf(f2,"LAST='%s' ; export LAST\n",LAST);
	fprintf(f2,". %s\n",fflag);
	fflush(f2);
	fclose(f2);
}/*printit*/

/* make work file and execute it */
execmsg()
{
	char cmd[LINELENGTH];
	char file[40];

	unlink(file);	/* remove previous version*/
	strcpy(file,TEMPS);
	mktemp(file);
	unlink(file);
	printit(file);
	sprintf(cmd,"%s %s < %s",xflag,oflag,file);
	if (dflag) {
		printf("UNEXECUTED cmd=%s\n",cmd);
		printf("CONTENTS of %s:\n#=======\n",file);
		sprintf(cmd,"/bin/cat %s",file);
		system(cmd);
		printf("#=======\n");
	} else {
		system(cmd);
	}/*fi*/
	unlink(file);
}/*execmsg*/

int main(argc,argv)
int	argc;
char	*argv[];
{
	register char		*sp;
	register struct passwd	*pp;

	strcpy(fflag,"$HOME/.mailawayrc");
	strcpy(xflag,"/bin/sh");
	strcpy(oflag,"");
	if ((sp = getenv("MAIL")) == NULL)
		sp = "";
	strcpy(mflag, sp);
	options(argc,argv);
	if (mflag[0] == '\0') {
		if ((sp = getenv("USER")) == NULL) {
			if ((pp = getpwuid(getuid())) == NULL) {
				fprintf(stderr, "cannot find mail file name\n");
				finit();
				exit(1);
			}
			sp = pp->pw_name;
		}
		sprintf(mflag, "/usr/spool/mail/%s", sp);
	}
	init();

	/* now for each message */
	while (fillfrom()) {
		savemsg();
		execmsg();
	}/*while*/

	finit();
	return(0);
}/*main*/

/*
 * error:
 *	Print out an error message and then exit.
 */
error(str)
char	*str;
{
	fprintf(stderr, "mailway:%s\n", str);
	finit();
	exit(-1);
}

/*
 * one_in:
 *	If "str" contains at least one instance any char in the string
 *	"list" return a pointer to the first instance in "str",
 *	otherwise return NULL.
 */
char *
one_in(str, list)
register char	*str, *list;
{
	register char	*sp;

	for (; *list; list++)
		for (sp = str; *sp; sp++)
			if (*sp == *list)
				return sp;
	return NULL;
}

/*
 * two_in:
 *	Return TRUE if "str" contains at least two instances any char in
 *	the string "list".
 */
two_in(str, list)
char	*str, *list;
{
	char	*sp;

	if ((sp = one_in(str, list)) == NULL)
		return FALSE;
	if (one_in(++sp, list) == NULL)
		return FALSE;
	return TRUE;
}
