#
/*
 * login [ name ]
 *
 * modified, 4-nov-76, G Goble, add 16 bit uid, remove gid
 * modified, 27-mar-77, G Goble, add jimmy's motd processor
 * modified, 26-sep-77, G Goble, use GCOS field for "nice" pri
 *	Log all root logins on /dev/tty8 (sys console device)
 *	for compatibility with /bin/su - TGI [ 11/17 10:35 ]
 * modified, 25-sep-78, C Schleifer, add motd flags '+' and '-'
 * major rewrite,  1-dec-78, R Reeves and TGI, fix bug where
 *      users with 8 character login names cannot login on tty:
 *      reorganize program to use getpwn, be structured and
 *      eliminate those ---- goto's.
 *      define DEBUG to turn on additional output and turnoff
 *      various chown's, run motd's ....etc
 */

#include <signals.h>    /* various signal defns */
#include <passwd.h>     /* password file structure */
#define ROOT	0

#define UTMPFIL "/etc/utmp"        /* utmp file */
#define WTMPFIL "/usr/adm/wtmp"    /* wtmp file */

struct passwd pastr;	/* define structure storage */
struct passwd *pwptr  &pastr;
    /* pwptr is pointer to password file line structure */

struct {
	char    uname[8];
	char	tty;
	char	ifill;
	int	time[2];
	int	ufill;
} utmp;                         /* struct for the utmp file */

struct {
	int	junk[5];
	int	size;
	int	more[12];
} statb;                        /* stat buffer */

char    logname[9];     /* user name typed */
char    logntry[9];     /* what he really typed at the login prompt */
			/* logname is subset of logntry
			 * if logntry = 'con p\0'
			 * then logname = 'con\0'
			 * and utmp.name = 'con     '   (8 chars, no \0)
			 */
char	*ttyx;
extern fout;

#define ERROR -1
#define OK 0

main(argc, argv)
char **argv;
{
	register t, fd;
	int now[2];

	signal(SIGQIT, 1);
	signal(SIGINT, 1);
	nice(0);

	ttyx = "/dev/ttyx";
	if ((utmp.tty=ttyn(0)) == 'x') {
		write(1, "Sorry.\n", 7);
		exit();
	}
	ttyx[8] = utmp.tty;

	dup(1);		/* make stdin */
	fout = dup(1);	/* make stderr */
	time(now);

	if (argc < 2 || argv[1][0] == 0) {
		if (getpid() & 01)	/* prevent phosphor burns */
			putchar('\n');
		printf("\nElectrical Engineering Dept. PDP-11/70\n");
		printf("Purdue/EE Network UNIX System	  TTY%c\n%s\n\n",
				utmp.tty, ctime(now));
		flush();
	}

	/* try to get the guy to give us the right info */

	getlgn(argc,argv);      /* this receives name and password
				 * and returns when someone gets
				 * a pair that matches
				 */

#ifndef DEBUG
	run("/bin/motd","+");   /* update last login time */
#endif

	if (chdir(pwptr->homedir) < 0) {
		write(1, "No directory.\n", 14);
		sleep(2);
		exit();
	}

	time(utmp.time);

	/* if root login, output to tty8 */
	if (pwptr->uid == ROOT && fork() == 0)
		tell8gud();

	nice( - (pwptr->gcos));     /* get priority from GCOS field */

	if ((fd = open(UTMPFIL, 1)) >= 0) {
		t = utmp.tty;
		seek(fd, t*16, 0);
		write(fd, &utmp, 16);
		close(fd);
	}

	if ((fd = open(WTMPFIL, 1)) >= 0) {
		seek(fd, 0, 2);
		write(fd, &utmp, 16);
		close(fd);
	}

#ifndef DEBUG
	chown(ttyx, pwptr->uid);        /* user owns the terminal file */
#endif

	if (setuid(pwptr->uid) == -1) {
		write(1,"setuid() failed\n",16);
		sleep(2);
		exit();
	}

	if (stat(".mail", &statb) >= 0 && statb.size)
		write(1, "You have mail.\n", 15);

	if (*pwptr->shell == '\0')
		pwptr->shell = "/bin/sh";
	execl(pwptr->shell, "-", 0);
	write(1, "No shell.\n", 10);
	sleep(2);
	exit();
}

run(job, args)
char *job;
char *args;
{
	/* fork a job under the uid in the password structure */

	register pid, p;
	int s;

	if (pid=fork()) {
		if (pid == -1) {
			printf("couldn't fork %s\n", job);
			flush();
		}
		while ((p = wait(&s)) != -1)
			if (p == pid)
				break;
	} else {
		if (setuid(pwptr->uid) == -1) {
			write(1,"setuid() failed\n",16);
			exit(1);
		}
		execl(job, job, args, 0);
		printf("can't exec %s!\n", job);
		flush();
		exit(1);
	}
}

clock(at) {
	register *p;
	register char *c;
	static char tbuf[12];

	p = localtime(at) + 4 * sizeof p;
	c = tbuf;

	*c++ = ++*p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c++ = '/';
	*c++ = *--p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c++ = ' ';
	*c++ = *--p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c++ = ':';
	*c++ = *--p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c = '\0';
	return(tbuf);
}
getlgn(argc,argv)
char **argv; int argc;
{
/*
 * getlgn does the actual verification of the user.
 * It first gets a login name from the user and calls getpwn
 * to find the correct entry in /etc/6passwd.
 * parsepw is then called to split the 6passwd line into
 * seperate parts for ease in manipulating.
 * If a password exists in the password file,
 * getpass is called to get the password from the user,
 * encrypt the user's response and compare
 * it with the encrypted password.
 * If any routine fails, getlgn responds 'Login incorrect'
 * and loops back to the beginning of getlgn to start the
 * sequence over.
 * Bad password attempts are reported by running "motd -".
 * Bad root attempts are logged on tty8.
 * In the midst of this mess, the variables logntry, logname
 * and utmp.name are set up.
 */

	register char *np, *namep;
	char c;
	char pwbuf[9];          /* buffer for what the user typed */
	char pbuf[128];		/* buffer for line from password file */

  for(;;) {

/* get the name from the arglist if possible */

	namep = logntry;
	if (argc > 1 && argv[1][0] != 0) {
		np = argv[1];
		while (namep<logntry+8 && *np)
			*namep++ = *np++;
		*namep = '\0';
		argc = 0;
	} else  do {

/* prompt the user for the login name, repeat till he does it right */

		namep = logntry;
		write(1, "Login: ", 7);
		while ((c = getchar()) != '\n') {
			if (c <= 0)
				exit();
			if (namep < logntry+8)
				*namep++ = c;
		}
		*namep = '\0';
	} while ((namep == logntry) ||  /* loop if return or */
		 (*logntry == ' ') ||   /* leading space or */
		 (*logntry == '\t'));   /* tab typed */

/* fill in the utmp.name field and pad it to 8 chars with spaces */
	namep = logntry;
	np = utmp.uname;
	while(*namep != '\0')
		*np++ = *namep++;
	while (np < utmp.uname+8)
		*np++ = ' ';

/* grab just the login name typed and truncate it with a \0 */
	namep = logntry;
	np = logname;
	while(*namep != '\0' && *namep != ' ') *np++ = *namep++;
	*np = '\0';

/* now we have his login name, try to
 * get his entry from the password file
 * If that succeeds, get the password.
 *
 * The following IF will return(OK) if the user specifies
 * everything correctly. Otherwise the IF will fall out the
 * bottom, type "Login incorrect." and branch back to the top
 * of getlgn to begin the verification procedure again.
 */

	if ((getpwn(logname, pbuf) == OK) && (parsepw(pbuf,pwptr) == OK)) {

		/* he exists. if no password then OK */
		if (*pwptr->pw == '\0') return(OK);

		/* get password, if password is correct, then OK */
		if (getpass("Password: ",1,pwptr->pw,pwbuf) == OK)
			return(OK);

		/* login attempt failed. if root, tell the world */
		if (pwptr->uid == ROOT && fork() == 0)
			tell8bad(pwbuf);

#ifndef DEBUG   /* of course, tell the real owner */
		run("/bin/motd","-");  /* Bump bad password attempts */
#endif
	}

/* finally, tell the user he messed up
 * if his name isn't in the password file, or if his password line
 * didn't parse right or if he specified the wrong password.
 */

	write(1, "Login incorrect.\n", 17);

    /* and repeat this until the user corrects his sin */
    }
}

tell8bad(pwbuf)
char *pwbuf;    /* what the bad attempt was */
{
	/* write to tty8 to announce
	 * bad root password attempt
	 */

	if ((fout = open("/dev/tty8", 1)) > 0) {
		time(utmp.time);
		printf("%s: attempted %s login: \"%s\" at tty%c\n",
		    clock(utmp.time), logntry, pwbuf,
		    utmp.tty);
		flush();
	}
	exit();
}

tell8gud() {

	/* write to tty8 to announce
	 * root login
	 * This should be forked before being called.
	 */

	if ((fout = open("/dev/tty8", 1)) > 0) {
		printf("%s: %s logged in on tty%c\n",
		    clock(utmp.time), logntry, utmp.tty);
		flush();
	}
	exit();
}
/*
 * getpass writes a prompt to the given file descriptor,
 * turns off the echo bit for the tty, accepts a password,
 * restores the echo bit, and compares the given encrypted
 * password with the encryption of the user's entry.
 * getpass returns -1 if the compare failed and 0 if successful.
 * getpass calls exit if the user types a control-d as part of
 * the password.
 * If the expected password is null, then getpass returns OK
 * since no password exists.
 *
 * prompt	prompt string
 * fd		fd of input file
 * pass		the encrypted password expected
 * pwbuf	the actual password typed in (returned by getpass)
 */

struct {
	int	speeds;
	char	erase, kill;
	int	tflags;
} ttyb;

#define	ECHO	010

getpass(prompt,fd,pass,pwbuf)
char *prompt, *pass, *pwbuf;
int fd;
{
	register char *p, *np, *namep;
	int sflags;
	char c;

	/* if encrypted password is null,
	 * return ok since no password exists
	 */
	if (*pass == '\0') return(OK);

	/* turn off tty echo */
	gtty(0, &ttyb);
	sflags = ttyb.tflags;
	ttyb.tflags =& ~ECHO;
	stty(0, &ttyb);

	/* write prompt for the user */
	p = prompt;
	while(*p++);
	write(fd,prompt,p-prompt-1);
	namep = pwbuf;

	/* get the user's guess! */
	while ((c=getchar()) != '\n') {
		if (c <= 0) {
			/* restore the tty bits */
			ttyb.tflags = sflags;
			stty(0, &ttyb);
			write(fd, "\n", 1);
			exit();
		}
		if (namep<pwbuf+8)
			*namep++ = c;
	}
	*namep = '\0';

	/* restore the tty bits */
	ttyb.tflags = sflags;
	stty(0, &ttyb);
	write(fd, "\n", 1);

	/* encrypt the user's answer and compare with the real thing */
	namep = crypt(pwbuf);
	if (strcmp(namep,pass)) {
		return(ERROR);
	}
	return(OK);
}
/*
 * getpwn(name, buf)
 *
 * Copies the password file entry for the given user name into buf.
 * On return buf is null-terminated and contains no newline character.
 *
 * Returns 0 if successful, -1 if not.
 *
 * Written by Ron Gomes as a modification of getpw -- April 1978.
 * modified for Purdue Unix, Nov 78, R. Reeves
 *	
 * this routine uses the standard unix i/o system
 * rather than the new i/o package.
*/


/* change def of PASSWORD to get a version of getpwn
 * that uses /etc/passwd or /etc/6passwd
 * then edit the program some.
 * if the seek file doesn't exist, getpwn
 * will try to use the standard password file (slowly)
 */

#define PASSWORD "/etc/6passwd"
#define HASH     "/etc/6passwd.hash"

getpwn(name, buf)
char *name, *buf;
{
	int pbuf[259];
	int use;
	int pwf;
	int hashfd;
	int hashind;
	int nohash;
	register char *bp, *np;
	register char c;

	nohash = 0;
	if ((hashfd = open(HASH, 0)) < 0  ||
	  seek(hashfd, name[0]*2, 0) < 0    ||
	  read(hashfd, &hashind, 2) != 2)
		nohash++;
	close(hashfd);

	if ((pwf = open(PASSWORD,0)) < 0)
		return(ERROR);
rewind:
	if (nohash)
		hashind = 0;
	seek(pwf, hashind, 0);

	pbuf[0] = pwf;
	pbuf[1] = 0;
	pbuf[2] = 0;

	for (;;) {

		/* use PASSWORD file */
		/* read in one line from password file */
		bp = buf;
		while((c=getc(pbuf)) != '\n') {
			if(c <= 0)
				{
				pwndone(pbuf,pwf,buf,ERROR);
				return(ERROR);
				}
			*bp++ = c;
		}
		*bp = '\0';

	       /* end PASSWORD code */

		/* compare the name in the password line with
		* the desired user name 
		*/

		np = name;
		bp = buf;	/* point to password line buffer */
		use = 0;
		while (*bp != ':'){
			if (*bp++ != *np++){
				use++;
				break;
			}
		}
		if (use)
			continue;
		if(*np == '\0') {
			/* found */
			pwndone(pbuf,pwf,buf,OK);
			return(OK);
		}

		/* not found yet, if EOF then not there */

		if(*bp == '\0') {
			if (nohash++ == 0)
				goto rewind;    /* try without hashing */
			pwndone(pbuf,pwf,buf,ERROR);
			return(ERROR);
		}
	}
}

pwndone(pbuf,pwf,buf,err)
char *pbuf;     /* pointers to password file */
int pwf;        /* fd of password file */
char *buf;      /* buffer for line from password file */
int err;        /* -1 if error detected */
{
	register char *i;

/* clean up before exiting from getpwn */
	close(pwf);
	*pbuf++ = 0;
	*pbuf++ = 0;
	*pbuf = 0; 	/* clear out pointers */

/* if error, then clear out the buf field */
	i = buf;
	if (err == ERROR)
		while (*i != '\0')
			*i++ = '\0';

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
