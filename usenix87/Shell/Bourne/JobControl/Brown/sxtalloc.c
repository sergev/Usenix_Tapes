/*
 *	Allocate a sxt-channel for a non-priveliged user, by
 *	chowning an unused one, so the user can pick it up later.
 *	Probably needs locks to prevent races, but too much effort to do that.
 *
 *	MUST BE SET-USER-ID "root".
 *
 *	Used by the "ssh" shell to do job-control.
 *
 *	Usage:
 *		sxtalloc 		- allocate sxt, print old ttyname on 
 *					  stdout;
 *					  exit status: 0=success, 1=failure.
 *		sxtalloc realtty status - de-allocate sxt, change utmp entry
 *					  back to "realtty";
 *					  exit status: "status"."
 *	Copyright (c) S. Brown, 1987
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/tty.h>
#include <sys/sxt.h>
#include <fcntl.h>
#include <utmp.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>

char sxt[] = "/dev/sxt/000";	/* virtual tty devices, channel 0 */
#define SXT_CHAN 9
char devsxt[] = "sxt000";	/* utmp dummy */
char devreal[50];
extern struct utmp *getutline();
extern char *ttyname();
extern char *strrchr();
char *getuser();
char *progname;

/*
 *	The "allow/deny" files, a la cron/at.
 */
#define ALLOW	"/usr/lib/ssh/sxt.allow"
#define DENY	"/usr/lib/ssh/sxt.deny"

#define UNAMESIZE 64	/* FAR too big! */

/*
 *	Verbose mode
 */
/*#define VERBOSE*/


main(argc,argv)
char **argv;
{
	int exitstat;

	progname = argv[0];
	if (argc==3){
		release(argv[1]);
		exitstat = atoi(argv[2]);
	} else if (argc==1)
		exitstat = allocate();
	else {
		fprintf(stderr, "%s: usage\n", argv[0]);
		exitstat = -1;
	}
	exit(exitstat);
	/*NOTREACHED*/
}


allocate()
{
	int fd;
	int chan = 0;
	int uid, gid;
	char *tty;
	int mode;
	struct utmp *u_entry;
	struct utmp u_start;
	register int i;
	struct stat statb;
	char *username;

	uid=getuid();
	username = getuser(uid);
	if (username==NULL){	/* you don't exist! */
#ifdef VERBOSE
		fprintf(stderr,"You don't exist. go away!\n");
#endif
		return(1);
	}
	if (allowed(username,ALLOW,DENY)==0){
#ifdef VERBOSE
		fprintf(stderr,"You are not allowed to use sxt devices\n");
#endif
		return(2);	/* 2 means "not permitted" */
	}
	do {
		fd = open(sxt,O_RDONLY|O_EXCL);
		if (fd==-1){
			if (errno!=EBUSY){
				fprintf(stderr,"%s: sxt failure on %s (errno %d)\n", progname, sxt,errno);
				return(1);	/* some strange error, like no chans unused */
			}
			chan++;		/* try next device */
			sxt[SXT_CHAN] = (chan/10)+'0';
			sxt[SXT_CHAN+1] = (chan%10)+'0';
		}
	} while (fd==-1);	/* continue until found an unused dev */

	gid=getgid();
	tty=ttyname(0);
	if (tty && stat(tty,&statb)==0){
		mode = statb.st_mode&0777;
		tty = strrchr(tty,'/');
		if (tty){
			tty++;
			strcpy(u_start.ut_line,tty);
			if ((u_entry=getutline(&u_start))!=(struct utmp *)0){
				strcpy(&devsxt[3], &sxt[SXT_CHAN]);
				devsxt[5] = '1';	/* chan 1 is control */
				strcpy(devreal, u_entry->ut_line);
				strcpy(u_entry->ut_line, devsxt);
				pututline(u_entry);
				endutent();
			} 
#ifdef VERBOSE
			else fprintf(stderr,"%s: can't change utmp\n", progname); 
#endif
		} else fprintf(stderr,"%s: tty has silly name\n", progname);
	} else {
		fprintf(stderr,"%s: warning - can't seem to find tty name\n", progname);
		mode = 0622;
	}

	for (i=0; i<MAXPCHAN; i++){	/* chown all the channels for that dev */

		sxt[SXT_CHAN+2] = i+'0';
		if (chown(sxt,uid,gid) != 0){	/* "real" id's */
			fprintf(stderr,"%s: chown failure on %s (errno %d)\n", progname, sxt, errno);
			return(1);	/* no point haning about */
		}
		if (chmod(sxt,mode) != 0){
			fprintf(stderr,"%s: chmod failure on %s (errno %d)\n", progname, sxt, errno);
			return(1);	/* no point haning about */
		}
	}
	printf("%s\n", devreal);
	return(0);
}



release(realtty)
char *realtty;
{
	char *tty;
	int mode;
	struct utmp u_start;
	struct utmp *u_entry;
	struct stat statb;
	int uid;

	uid=getuid();
	strcpy(devreal,"/dev/");
	if (realtty && *realtty){
		strcat(devreal,realtty);
		if (stat(devreal,&statb)!=0
			     || statb.st_uid!=uid
			     || (statb.st_mode&S_IFMT)!=S_IFCHR){
			fprintf(stderr,"%s: %s is not a tty belonging to you!\n",
				progname, realtty);
			return;
		}
	} else {
		fprintf(stderr,"%s: null tty name given\n", progname);
		return;
	}
	tty=ttyname(0);
	if (tty && stat(tty,&statb)==0){
		mode = statb.st_mode&0777;
		tty = strrchr(tty,'/');
		if (tty){
			tty++;
			strcpy(u_start.ut_line,tty);
			if ((u_entry=getutline(&u_start))!=(struct utmp *)0){
				strcpy(u_entry->ut_line, realtty);
				pututline(u_entry);
				endutent();
			} else fprintf(stderr,"%s: can't restore utmp\n", progname);
		} else fprintf(stderr,"%s: tty has silly name\n",progname); 
	} else {
		fprintf(stderr,"%s: warning - can't seem to find tty name\n", progname);
		mode = 0622;
	}
	if (chmod(devreal,mode)!=0)
		fprintf(stderr,"%s: cannot reset mode of real tty\n", progname);}


/*
 *	--------------------------------------------------------
 *
 *	The following stuff checks in the files
 *	sxt.allow and sxt.deny files to control access
 *
 */



struct stat globstat;
#define	exists(file)	(stat(file,&globstat) == 0)

char *getuser(uid)
int uid;
{
	struct passwd *nptr,*getpwuid();

	if ((nptr=getpwuid(uid)) == NULL) {
		return(NULL); 
	}
	return(nptr->pw_name);
}


allowed(user,allow,deny)
char *user,*allow,*deny;
{
	if (getuid() == 0) return(1);	/* root is a good guy */
	if ( exists(allow) ) {
		if ( within(user,allow) ) return(1);
	} else if ( exists(deny) ) {
		if ( within(user,deny) ) return(0);
		else return(1); 
	} else return(1);
	return(0);
}


within(username,filename)
char *username,*filename;
{
	char line[UNAMESIZE];
	FILE *cap;
	int i;

	if((cap = fopen(filename,"r")) == NULL)
		return(0);
	while ( fgets(line,UNAMESIZE,cap) != NULL ) {
		for ( i=0 ; line[i] != '\0' ; i++ ) {
			if ( isspace(line[i]) ) {
				line[i] = '\0';
				break; }
		}
		if ( strcmp(line,username)==0 ) {
			fclose(cap);
			return(1); }
	}
	fclose(cap);
	return(0);
}
