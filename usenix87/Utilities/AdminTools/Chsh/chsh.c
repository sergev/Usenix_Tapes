/*
 *	This program was originally written by K. Richard Magill,
 *	and posted to mod.sources. It has been extensively
 *	modified by Jack Jansen. See below for details.
 *
 *	K. Richard Magill, 26-jan-86.
 *	Last Mod 26-jan-86, rich.
 *	Modified by Jack Jansen, 30-jan-86:
 *	- It now runs under V7.
 *	- It now uses (what I believe to be) standard
 *	  password file locking and backups.
 *	- Check the size of the new passwd file, abort if
 *	  it looks funny.
 *	- Check that there are no :colons: or \nnewlines\n in the
 *	  given string.
 *	- Use name from getlogin() if not given, and ask for
 *	  parameters if not given.
 *	- if SECURE is defined, don't let other people
 *	  muck finger/shell info.
 */
#define NOPUTPWENT  1		/* Define this if you don't have putpwent */
#define SECURE	1		/* Only owner/root can change stuff */
#define DEBUG	1		/* ALWAYS DEFINE THIS AT FIRST */
#define void	int		/* If your compiler doesn't know void */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

#define WATCH(s,x)	if(x){perror(s);return(-1);}

char *PASSWD = "/etc/passwd";
#ifndef DEBUG
char *BACKUP = "/etc/passwd.bak";
char *LOCK = "/etc/vipw.lock";
char *TEMP = "/etc/ptmp";
char *BAD_TEMP = "/etc/ptmp.bad";
#else
char *LOCK = "vipw.lock";
char *TEMP = "ptmp";
char *BAD_TEMP = "ptmp.bad";
#endif DEBUG
char ArgBuf[128];
char *Arg = ArgBuf;

void endpwent(), perror();
char *crypt(), *getpass(), *mktemp();
struct passwd *getpwent(), *getpwnam(), *fgetpwent();
char *index();

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	register struct passwd *p;
	FILE *fout;
	int target_id;			/* Who are we changing? */
	struct stat stat_buf;
	long  OldLen, NewLen;		/* Old/New length of passwd */
	long LenDiff;			/* Expected length dif. */
	int ShellMode;			/* True if chsh */
	char *UserName;			/* Who are we working for */

	if( strcmp(argv[0], "chsh") == 0 ) ShellMode = 1; else
	if( strcmp(argv[0], "chfn") != 0 ) {
	    fprintf(stderr,"Sorry, program name should be 'chsh' or 'chfn'.\n");
	    exit(1);
	}

	if( argc >= 2 ) {	/* Login name given */
		UserName = argv[1];
	} else {
		UserName = getlogin();
		printf("Changing %s for %s\n", ShellMode ? "login shell":
			"real name", UserName);
	}

	/* is this person real? */

	if ((p = getpwnam(UserName)) == NULL) {
		(void) fprintf(stderr, "%s: don't know %s\n",
			argv[0], UserName);
		return(-1);
	}	/* if person isn't real */

	/* do we have permission to do this? */
	target_id = p->pw_uid;

	if ((i = getuid()) != 0 && i != target_id) {
#ifdef SECURE
		fprintf(stderr,"Sorry, you don't have permission to do that.\n");
		exit(1);
#else
		char salt[3];

		salt[0] = p->pw_passwd[0];
		salt[1] = p->pw_passwd[1];
		salt[3] = '\0';

		if (*p->pw_passwd != '\0'
			&& strncmp(crypt(getpass("Password: "), salt),
			p->pw_passwd, 8)) {
			(void) fprintf(stderr, "Sorry.\n");
			return(-1);
		}	/* passwd didn't match */
#endif SECURE
	}	/* check for permission */

	/* If in verbose mode, print old info */
	if( argc <= 2 ) {
	    if( ShellMode ) {
		printf("Old shell: %s\n", p->pw_shell?p->pw_shell:"");
		printf("New shell: ");
		gets(Arg);
	    } else {
		printf("Old name: %s\n", p->pw_gecos?p->pw_gecos:"");
		printf("New name: ");
		gets(Arg);
	    }
	} else {
	    Arg = argv[2];
	}

	/* Check for dirty characters */
	if( index(Arg, '\n') || index(Arg, ':') ) {
	    fprintf(stderr,"%s: Dirty characters in argument.\n",argv[0]);
	    exit(1);
	}

	/* Check that the shell sounds reasonable */
	if( ShellMode ) {
	    if( *Arg != '/' ) {
		fprintf(stderr,"%s: shell name should be full path.\n",Arg);
		exit(1);
	    }
	    WATCH(Arg,stat(Arg,&stat_buf));
	    if( (stat_buf.st_mode & 0111) == 0 ) {
		fprintf(stderr,"%s is not an executable.\n");
		exit(1);
	    }
	}

	/* set up files */

	endpwent();	/* close passwd file */

	setpwent();

	/* Now, lock the password file */
	creat(LOCK,0600);	/* This might fail. No problem */
	if( link(LOCK,TEMP) < 0 ) {
		fprintf(stderr,"Sorry, password file busy.\n");
		exit(1);
	}
	WATCH(TEMP,(fout = fopen(TEMP, "w")) == NULL);

	while ((p = getpwent()) != NULL) {
		if (p->pw_uid == target_id) {
			if (!ShellMode ) {
				LenDiff = strlen(Arg)-strlen(p->pw_gecos);
				p->pw_gecos = Arg;
			} else {
				LenDiff = (-strlen(p->pw_shell));
				p->pw_shell = Arg == NULL ? "/bin/sh"
					: Arg;
				LenDiff += strlen(p->pw_shell);
			}
		}	/* if this is entry to be changed */

		WATCH("putpwent",putpwent(p, fout));
	}	/* while not eof (we couldn't recognize an error) */

	/* close files */
	endpwent();
	fclose(fout);

	/* Check that sizes are as expected */
	WATCH(TEMP, stat(TEMP, &stat_buf) );
	NewLen = stat_buf.st_size;
	WATCH(PASSWD, stat(PASSWD, &stat_buf) );
	OldLen = stat_buf.st_size;
	if( OldLen + LenDiff != NewLen ) {
	    fprintf(stderr,"Sorry, password file changed size: %ld, expected %ld.\n", NewLen, OldLen+LenDiff);
	    fprintf(stderr,"Warn your system administrator, please.\n");
	    WATCH(TEMP, link(TEMP,BAD_TEMP));
	    WATCH(TEMP,unlink(TEMP));
	    WATCH(LOCK,unlink(LOCK));
	    exit(1);
	}

#ifndef DEBUG
	/* remove old backup if it exists */
	WATCH(BACKUP,!stat(BACKUP, &stat_buf) && unlink(BACKUP));

	/* make current passwd file backup */
	WATCH("linking passwd to passwd.bak",link(PASSWD, BACKUP) || unlink(PASSWD));

	/* make new file passwd */
	WATCH("linking temp to passwd",link(TEMP, PASSWD) || unlink(TEMP));
	WATCH("chmod passwd", chmod(PASSWD, 0644));
#endif DEBUG

	/* Remove lock */
	WATCH(LOCK,unlink(LOCK));

#ifdef DEBUG
	printf("Now, check that %s looks reasonable.\n", TEMP);
#endif DEBUG
	/* must have succeeded */
	return(0);
}	/* main */

#ifdef NOPUTPWENT
putpwent(ent, fp)
    FILE *fp;
    struct passwd *ent;
    {

    fprintf(fp,"%s:%s:%d:%d:%s:%s:%s\n", ent->pw_name, ent->pw_passwd,
	ent->pw_uid, ent->pw_gid, ent->pw_gecos, ent->pw_dir,
	ent->pw_shell);
    return(0);
}
#endif NOPUTPWENT
