static char sccsid[] = "@(#)xmail.c	2.10";
/*
	xmail [ person ] ... [ -g grpname ] ... [ alias ] ...
	xmail [-lpqr] [ -f file ]

	This is an enhanced version of the UNIX 5.0 mail command.

	make:
		rm -f xmail
		cc -g -O xmail.c -o xmail
		#As superuser, do the following
		chown bin xmail
		chgrp mail xmail
		chmod 2755 xmail
*/

#include	<stdio.h>
#include	<sys/types.h>
#include	<pwd.h>
#include	<grp.h>
#include	<utmp.h>
#include	<signal.h>
#include	<ctype.h>
#include	<time.h>
#include	<sys/stat.h>
#include	<setjmp.h>
#include	<sys/utsname.h>

#define equal		!strcmp
#define equaln		!strncmp
#define	isdaemon(s)	(sindex(s, "uucp") != -1 || sindex(s, "**NSC**") != -1)
#define	SHOW(n)	((n)+1)
#define	ismail(n)	((n) >= 0 && (n) < nlet)

#define	EX_UNIQ	01	/* expand() requires single token in expansion */
#define	EX_ANY	02	/* expand() allows multiple tokens in expansion */

#define	SAME	0

#define	TRUE	1
#define	FALSE	0

#define CERROR		-1
#define CSUCCESS	0


#define A_OK		0		/* return value for access */
#define A_EXIST		0		/* access check for existence */
#define A_WRITE		2		/* access check for write permission */
#define A_READ		4		/* access check for read permission */

/*copylet flags */
#define	REMOTE		1		/* remote mail, add rmtmsg */
#define ORDINARY	2
#define ZAP		3		/* zap header and trailing empty line */
#define FORWARD		4
#define	LSIZE		256
#define	TOSIZE		512		/* size of buffer to hold dests */
#define	MAXLET		300		/* maximum number of letters */
#ifndef	MFMODE
#define	MFMODE		0660		/* create mode for `/usr/mail' files */
#endif

struct	let	{
	long	adr;
	char	change;
	int	llns;			/* lines in letter */
	char	*sender;		/* sender (e.g., sys1!sys2!login) */
	char	*subject;		/* subject of letter */
	char	*postmark;		/* date letter was received */
} let[MAXLET];
struct	let	NullLet;		/* readonly null letter */

/* This structure is set up by envelope() */
struct	{
	int	r_lines;	/* number of lines */
	int	r_nlcnt;	/* number of trailing empty lines */
	long	r_size;		/* number of bytes in letter */
} Readio;

long fsize();
struct	passwd	*getpwuid(), getpwent();
struct	group	*getgrnam();
struct	utsname utsn;
char	*calloc();
void	toc();
char	*fromtail();
char	*getdate();
char	*strtok();

char	*Mailpgm;
char	*Maillog;
char	lettmp[] = "/tmp/maXXXXX";
char	edtmp[]  = "/tmp/maedXXXXX";
char	*whom;
char	deadansw[] = "dead.answer";
FILE	*answf;				/* answer file pointer into lettmp */
char	tz[16];				/* timezone string */
char	from[] = "From ";
char	TO[] = "To: ";
char	PREPEND[] = "    ";
char	maildir[] = "/usr/mail/";
char	sendto[256];
char	*mailfile;
char	maillock[]	= ".lock";
char	dead[]		= "dead.letter";
char	rmtbuf[]	= "/tmp/marXXXXXX";
char	*rmtmsg		= " remote from %s\n";
char	rmtfrom[]	= " remote from ";
char	*forwmsg	= " forwarded by %s\n";
char	fwrdmsg[]	= " forwarded by ";
char	frwrd[]		= "Forward to ";
char	NoLetter[]	= "Not positioned on a letter\n";
char	*thissys;
char	mbox[]		= "/mbox";
char	curlock[50];
char	line[LSIZE];
char	resp[LSIZE];
char	*hmbox;
char	*home;
char	*email;
char	*my_name;
char	*getlogin();
char	**alias();
char	*finddest();
char	*frflsh();
char	*strchr();
char	*strpbrk();
char	*strcpy();
char	*malloc();
char	lfil[256];
char	*ctime();
char	*getenv();
char	*strrchr();
char	*mktemp();

FILE	*tmpf;
FILE	*malf;
FILE	*rmtf;
FILE	*logf;

int	error;
int	nlet	= 0;
int	curlet;
int	locked;
int	changed;
int	forward;
int	delete();
int	flgf = 0;
int	flgp = 0;
int	flge = 0;
int	flgl = 0;
int	delflg = 1;
int	ttyin;				/* input from a tty? */
int	ttyout;				/* output to a tty? */
int	rmail;				/* restricted mail; run by uucp */
int	DestSub;			/* Any alias or group substitutions? */
int	savdead();
int	(*saveint)();
int	(*setsig())();

long	ftell();
FILE	*readinput();
long	iop;

jmp_buf	sjbuf;

#ifdef USE_NUSEND
jmp_buf nusendfail;
int	nusendjmp = FALSE;
#endif

#ifdef USE_USEND
jmp_buf usendfail;
int	usendjmp = FALSE;
#endif

int umsave;

main(argc, argv)
char **argv;
{
	register i;
	register char *p;

	Mailpgm = argv[0];
	if ((Maillog = getenv("MAILLOG")) == NULL || *Maillog == '\0') {
		logf = NULL;
	} else {
		logf = fopen(Maillog, "a");
	}
	if((p=getenv("TZ")) == NULL)
		tz[0] = 0;
	else
	{
		tz[0] = ' ';
		strncpy(&tz[1], p, 3);
	}
	home = getenv("HOME");
	if((home == NULL) || (strlen(home) == 0))
		home = ".";
	umsave = umask(7);
	setbuf(stdout, malloc(BUFSIZ));
	mktemp(lettmp);
	unlink(lettmp);
	uname(&utsn);
	thissys = utsn.nodename;
	my_name = getenv("LOGNAME");
	if ((my_name == NULL) || (strlen(my_name) == 0))
		my_name = getlogin();
	if ((my_name == NULL) || (strlen(my_name) == 0))
		my_name = getpwuid(geteuid())->pw_name;
	if(setjmp(sjbuf)) done();
	for (i=2; i<18; i++)
		setsig(i, delete);
	setsig(1, done);
	tmpf = fopen(lettmp, "w");
	if (tmpf == NULL) {
		fprintf(stderr, "%s: cannot open %s for writing\n", Mailpgm, lettmp);
		error = 2;
		done();
	}
	ttyin  = isatty(0);
	ttyout = isatty(1);
	if((p=strrchr(argv[0], '/')) == NULL)
		p = argv[0];
	else
		p++;
	if(*p == 'r') rmail++;
	if (!rmail &&	/* no favors for rmail */
	   (argc == 1 || argv[1][0] == '+' ||
	   (argv[1][0] == '-' && argv[1][1] != 'g')))
		printmail(argc, argv);
	else
		sendmail(argc, argv);
	done();
}

int (*setsig(i, f))()
int i;
int (*f)();
{
	register int (*rc)();

	if((rc=signal(i, SIG_IGN))!=SIG_IGN)
		signal(i, f);
	return(rc);
}

printmail(argc, argv)
char **argv;
{
	int	flg, i, j, print, aret, stret, goerr = 0;
	int	oi;
	int	k;
	int	c;
	char	*p, *getarg();
	char	*expand();
	extern	char *optarg;
	int	answsave;
	int	ansletno;
	char	hbuf[256];
	char	tobuf[TOSIZE];
	char	*rp;
	int	imax;
	FILE	*textfp;
	char	sprompt[80];	/* prompt buffer fits on a line */

	if (argv[1][0] == '+') {
		forward = 1;
		argc--;
		argv++;
	}
	while((c=getopt(argc, argv, "f:lrpqie")) != EOF) switch(c) {
		case 'f':
			flgf = 1;
			mailfile = optarg;
			break;
		case 'p':
			flgp++;
		case 'q':
			delflg = 0;
		case 'i':
			break;
		case 'l':
			flgl = 1;
			break;
		case 'r':
			forward = 1;
			break;
		case 'e':
			flge = 1;
			break;
		case '?':
			goerr++;
	}
	if(goerr) {
		fprintf(stderr, "usage: %s [-lrpq] [-f file] [persons]\n",
			Mailpgm);
		error = 2;
		done();
	}

	/* speed up malloc()s by getting and freeing a big block */
	for (i = 0, c = 3000; i < 30; i++, c -= 100) {
		if ((p = malloc(c)) != NULL) {
			free(p);
			break;
		}
	}

	hmbox = malloc(strlen(home) + strlen(mbox) + 1);
	cat(hmbox, home, mbox);
	if(!flgf) {
		mailfile = getenv("MAIL");
		if((mailfile == NULL) || (strlen(mailfile) == 0)) {
			mailfile = malloc(strlen(maildir) + strlen(my_name) + 1);
			cat(mailfile, maildir, my_name);
		}
	}

	/* Open mailfile and set up array of letters and temp file */
	if (openmail() < 0)
		return;

	print = 1;
	i = 0;
	if (setjmp(sjbuf) == 0) {
		/* Print optional mail index */
		if (flgl) {
			toc();
			print = 0;
			i = -1;
		}
	}
	imax = nlet - 1;
	for (;;) {
		answsave = 0;
		j = forward ? i : nlet - i - 1;
		curlet = j;
		if( setjmp(sjbuf) == 0 && print != 0 && ismail(i)) {
			frflsh(0);
			copylet(j, stdout, ORDINARY, "");
		}
		if (flgp) {
			if (++i < nlet)
				continue;
			break;
		}
		setjmp(sjbuf);

		/* Build prompt message */
		sprompt[0] = '\0';
		if (newmail()) {
			strcat(sprompt, sprompt[0] ? "; " : "[");
			strcat(sprompt, "new mail arrived");
		}
		if (i == nlet) {
			strcat(sprompt, sprompt[0] ? "; " : "[");
			strcat(sprompt, "EOF");
		}
		if (!ismail(i)) {
			strcat(sprompt, sprompt[0] ? "; " : "[");
			strcat(sprompt, "unpositioned");
		} else if (i == imax) {
			strcat(sprompt, sprompt[0] ? "; " : "[");
			strcat(sprompt, "last");
		}
		if (sprompt[0])
			strcat(sprompt, "] ");
		if (flgl && ismail(i)) {
			for (p = sprompt; *p; ++p)
				;
			sprintf(p, "%d ", SHOW(i));
		}
		strcat(sprompt, "? ");
		fputs(sprompt, stdout);
		fflush(stdout);

		if (fgets(resp, LSIZE, stdin) == NULL)
			break;

		print = 1;
		DestSub = 0;
		/* Select letter by its number */
		if (isdigit(resp[0])) {
			flgl = 1;
			k = atoi(resp) - 1;
			if (k >= 0 && k < nlet) {
				i = k;
				continue;
			}
			print = 0;
			fprintf(stderr,
			    "Letter %d does not exist\n", SHOW(k));
			continue;
		}
		switch (resp[0]) {
		case '?':
			/* Request help */
			print = 0;
			help();
			break;
		case 'p':
			/* Print current letter */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			rp = resp+1;
			while (*rp && *rp != '\n' && isspace(*rp))
				++rp;
			if (*rp == '|') {
				++rp;
				pipelet(j, rp);
				print = 0;
			}
			break;
		case 'x':
			/* Exit without updating mailfile */
			changed = 0;
		case 'q':
			/* Quit */
			goto donep;
		case '>':
			/* Advance to next undeleted letter */
			oi = i;
			do {
				++i;
				j = forward ? i : nlet - i - 1;
			} while (i < nlet && let[j].change);
			if (i == nlet) {
				fprintf(stderr,
				    "Could not advance to undeleted letter\n");
				i = oi;
				print = 0;
			}
			break;
		case '<':
			/* Back up to undeleted letter */
			oi = i;
			do {
				--i;
				j = forward ? i : nlet - i - 1;
			} while (i >= 0 && let[j].change);
			if (i < 0) {
				fprintf(stderr,
				    "Could not back up to undeleted letter\n");
				i = oi;
				print = 0;
			}
			break;
		case '+':
		case 'n':
		case '\n':
			/* Advance to next letter */
			/*
			Allow user to go from "unpositioned" to first letter.
			*/
			++i;
			if (!ismail(i)) {
				fprintf(stderr, "Can't go past last letter\n");
				--i;
				print = 0;
			}
			break;
		case '-':
			/* Back up to previous letter */
			if (!ismail(i)) {
				fprintf(stderr,
					"Can't backup if unpositioned\n");
				print = 0;
			} else if (i == 0) {
				fprintf(stderr,
					"Can't backup from first letter\n");
				print = 0;
			} else {
				--i;
			}
			break;
		case 'a':
			/*
			 Answer mail. The answsave flag is used to tell the
			 's' option below to also write out the answer into
			 the save file. To send answers, we must trick the
			 sendmail routine into sending letter number (nlet+1)
			 of the answer file.
			 */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			if(resp[1] == 's')
			{
				if(resp[2] == '\n' || resp[2] == '\0')
					resp[1] = 0;
				else
					resp[1] = ' ';
				answsave = 1;
			}

			if (!*(whom = frflsh(1))) {
				fprintf(stderr, "Cannot compute respondee\n");
				break;
			}

			/* Get text of response from standard input */
			saveint = setsig(SIGINT, savdead);
			textfp = readinput(stdin, whom);
			setsig(SIGINT, saveint);
			fprintf(stdout, "\n");
			if (textfp == NULL)
				break;
			/*
			 *	Use the next free letter position, but
			 *	do not increment nlet. "ansletno" will
			 *	be the number of the letter. Don't
			 *	forget to put the end of the file in
			 *	let[ansletno+1].adr.
			 *	Also, always position write pointer
			 *	at address of end of last letter.
			 */
			ansletno = nlet;
			if((answf = fopen(lettmp, "r+")) == NULL) {
				fprintf(stderr, "Cannot open %s\n", lettmp);
				break;
			}
			if(fseek(answf, let[nlet].adr, 0) == -1)
				fprintf(stderr,"SEEK ERROR!!!\n");
			time(&iop);
			sprintf(hbuf, "%s%s ANSWER %24.24s%s\n",
				from, my_name, ctime(&iop), tz);
			hbuf[255] = 0;

			/* Prefix input with `From' header */
			strcpy(whom, finddest(whom, ""));
			envelope(answf, hbuf, whom, textfp);

/*
 *	Here is where we trick the sendmail routine.
 */
			let[ansletno+1].adr = let[nlet].adr + Readio.r_size;
			let[ansletno].llns = Readio.r_lines;
			fclose(tmpf);
			tmpf = fopen(lettmp, "r");
			if (error) {
				if(storedead(ansletno, deadansw))
					fprintf(stdout, "%s: cannot create %s\n", Mailpgm, deadansw);
				break;
			}

			/* Write log entry */
			if (logf != NULL)
				copylet(ansletno, logf, ORDINARY, "");

			if (DestSub)
				printf("Sending to: %s\n\n", whom);

			flg = 0;
			for (p = whom; (p = getarg(lfil, p)) != NULL; )
				if (send(ansletno, lfil, 0) != TRUE) {
					fprintf(stderr,
						"Cannot send to %s\n", lfil);
					flg++;
				}
			if (flg) {
				print = 0;
				if(storedead(ansletno, deadansw))
					fprintf(stdout, "%s: cannot create %s\n", Mailpgm, deadansw);
				umask(7);
			} else {
				let[j].change = resp[0];
				changed++;
				if (answsave == 0)
					++i;
			}
			if(answsave == 0)
				break;
		case 'y':
		case 'w':
		case 's':
			/* Save letter in file */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			if (resp[1] == '\n' || resp[1] == '\0')
				cat(resp+1, hmbox, "");
			else if(resp[1] != ' ') {
				printf("invalid command\n");
				print = 0;
				continue;
			}
			umask(umsave);
			flg = 0;
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
loop:
				if((aret=legal(lfil)))
					malf = fopen(lfil, "a");
				if ((malf == NULL) || (aret == 0))
				{
					fprintf(stderr, "%s: cannot append to %s\n", Mailpgm, lfil);
					if(!ttyin)
					{
						flg++;
						continue;
					}
					reenter(lfil, sizeof(lfil));
					while (lfil[0] == '!') {
						system(lfil+1);
						printf("!\n");
						reenter(lfil, sizeof(lfil));
					}
					if (lfil[0] == '\0') {
						/* never mind ... */
						printf("Message not saved\n");
						flg = 1;
						goto nevermind;
					}
					goto loop;
				}
				if(aret==2)
					chown(lfil, geteuid(), getgid());
				copylet(j, malf, resp[0]=='w'? ZAP: ORDINARY,"");
/*
 *	Here we save the answer into the save file.
 */
				if(answsave)
				{
					if(strchr(whom, '!'))
						copylet(ansletno, malf, ORDINARY, PREPEND);
					else
						copylet(ansletno, malf, REMOTE, "");
				}
				fclose(malf);
			}
			umask(7);
nevermind:
			if (flg)
				print = 0;
			else {
				let[j].change = resp[0];
				changed++;
				i++;
			}
			break;
		case 'm':
			/* Forward letter */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			if (resp[1] == '\n' || resp[1] == '\0') {
				i++;
				continue;
			}
			if (resp[1] != ' ') {
				printf("invalid command\n");
				print = 0;
				continue;
			}

			/* Translate aliases */
			strcpy(resp+1, finddest(resp+1, ""));

			if (DestSub)
				printf("Sending to: %s\n\n", resp+1);

			/* Write log entry */
			if (logf != NULL)
				copylet(j, logf, ORDINARY, "");

			flg = 0;
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; )
				if (sendrmt(j, lfil) != TRUE)
					flg++; /* couldn't send it */
			if (flg)
				print = 0;
			else {
				let[j].change = resp[0];
				changed++;
				i++;
			}
			break;
		case 'M':
			/* Forward letter with attached commentary */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			/*
			 * "Staple" a comment to letter and forward
			 */

			/* Prefix input with `From' header */
			for (rp = resp+1; *rp && isspace(*rp); rp++)
				;
			strcpy(tobuf, rp);

			/* Get text of comment from standard input */
			saveint = setsig(SIGINT, savdead);
			textfp = readinput(stdin, tobuf);
			setsig(SIGINT, saveint);
			fprintf(stdout, "\n");
			if (textfp == NULL)
				break;

			/*
			 *	Use the next free letter position, but
			 *	do not increment nlet. "ansletno" will
			 *	be the number of the letter. Don't
			 *	forget to put the end of the file in
			 *	let[ansletno+1].adr.
			 *	Also, always position write pointer
			 *	at address of end of last letter.
			 */
			ansletno = nlet;
			if((answf = fopen(lettmp, "r+")) == NULL) {
				fprintf(stderr, "Cannot open %s\n", lettmp);
				break;
			}
			if(fseek(answf, let[nlet].adr, 0) == -1)
				fprintf(stderr,"SEEK ERROR!!!\n");
			time(&iop);
			sprintf(hbuf, "%s%s %24.24s%s\n",
				from, my_name, ctime(&iop), tz);
			hbuf[255] = '\0';

			strcpy(tobuf, finddest(tobuf, ""));
			envelope(answf, hbuf, tobuf, textfp);

/*
 *	Here is where we trick the sendmail routine.
 */
			let[ansletno+1].adr = let[nlet].adr + Readio.r_size;
			let[ansletno].llns = Readio.r_lines;
			fclose(tmpf);
			tmpf = fopen(lettmp, "r");
			if (error) {
				if(storedead(ansletno, deadansw))
					fprintf(stdout, "%s: cannot create %s\n", Mailpgm, deadansw);
				break;
			}

			/* Write log entry */
			if (logf != NULL)
				copylet(ansletno, logf, ORDINARY, "");

			if (DestSub)
				printf("Sending to: %s\n\n", tobuf);

			flg = 0;
			for (p = tobuf; (p = getarg(lfil, p)) != NULL; )
				if (sendrmt(ansletno, lfil) != TRUE) {
					fprintf(stderr,
						"Cannot send to %s\n", lfil);
					flg++;
				}
			if (flg) {
				print = 0;
				if(storedead(ansletno, deadansw))
					fprintf(stdout, "%s: cannot create %s\n", Mailpgm, deadansw);
				umask(7);
			} else {
				let[j].change = resp[0];
				changed++;
				i++;
			}
			break;
		case '!':
			/* Escape to the shell */
			system(resp+1);
			printf("!\n");
			print = 0;
			break;
		case 'c':
			/* chdir */
			rp = strtok(resp, " \t\n");
			if (equal(rp, "cd")) {
				if (mailfile[0] != '/') {
					fprintf(stderr,
"Can't `cd' unless mail file begins with '/' (cd! overrides)\n", mailfile);
					goto after;
				}
			} else if (!equal(rp, "cd!"))
				goto dflt;

			/* Parse directory out of response */
			rp = strtok(NULL, " \t\n");
			expand(rp, EX_UNIQ);
			rp = strtok(rp, " \t\n");
			p = (rp && *rp) ? rp : home;
			if (chdir(p) != -1)
				fprintf(stderr, "%s\n", p);
			else
				fprintf(stderr, "%s: Bad directory\n", p);
after:
			print = 0;
			break;
		case 'd':
			/* Delete current letter */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			let[j].change = resp[0];
			changed++;
			i++;
			if (resp[1] == 'q')
				goto donep;
			break;
		case 'u':
			/* Undelete current letter */
			if (!ismail(i)) {
				fprintf(stderr, NoLetter);
				break;
			}
			if (let[j].change == 0) {
				fprintf(stderr,
				    "Letter %d was never deleted\n", SHOW(i));
			} else {
				let[j].change = 0;
				print = 0;
				i++;
			}
			break;
		case 'l':
			/* List table of contents */
			print = 0;
			i = -1;
			flgl = 1;
			toc();
			break;
		case 'f':
			/* Save changes and reopen current mailfile */
			/* Copy remaining mail back to mailfile */
			if (changed) {
				copyback();
				printf("Changes saved in %s\n", mailfile);
			}

			/* Make sure tmpf is open for writing */
			fclose(tmpf);
			if ((tmpf = fopen(lettmp, "w")) == NULL) {
				fprintf(stderr,
					"%s: cannot open %s for writing\n",
					Mailpgm,
					lettmp);
				error = 2;
				return;
			}

			/* Reopen mailfile */
			if (openmail() < 0)
				return;
			imax = nlet - 1;

			/* Optionally print mail index */
			if (flgl)
				toc();

			/* Initialize current position to null */
			print = 0;
			i = -1;
			break;
		default:
dflt:
			print = 0;
			fprintf(stderr, "Type ? for list of valid commands\n");
			break;
		}
	}
donep:
	if (changed)
		copyback();
}


openmail()
{
	int	aret;
	int	stret;
	struct	stat stbuf;
	char	frwrdbuf[256];

	stret = stat(mailfile, &stbuf);
	if ((aret=access(mailfile, 4)) == 0)
		malf = fopen(mailfile, "r");
	if (!stret && aret) {
		fprintf(stderr, "%s: permission denied!\n", Mailpgm);
		error = 2;
		return -1;
	}
	else if (flgf && (aret || (malf == NULL))) {
		fprintf(stderr, "%s: cannot open %s\n", Mailpgm, mailfile);
		error = 2;
		return -1;
	}
	else if(aret || (malf == NULL) || (stbuf.st_size == 0)) {
		if(!flge) fprintf(stdout, "No mail.\n");
		error = 1;
		return -1;
	}
	lock(mailfile);
	if(areforwarding(mailfile)) {
		if(flge) {
			unlock();
			error = 1;
			return -1;
		}
		printf("Your mail is being forwarded to ");
		fseek(malf, (long)(sizeof(frwrd) - 1), 0);
		fgets(frwrdbuf, sizeof(frwrdbuf), malf);
		printf("%s", frwrdbuf);
		if(getc(malf) != EOF)
			printf("and your mailbox contains extra stuff\n");
		unlock();
		return -1;
	}
	if(flge) {
		unlock();
		return -1;
	}
	copymt(malf, tmpf);
	fclose(malf);
	fclose(tmpf);
	unlock();
	tmpf = fopen(lettmp, "r");
	changed = 0;
	return 0;
}


help()
{
	static	char	*mess[] = {
		"?\t\tthis message",
		"q\t\tquit",
		"x\t\texit without changing mail",
		"f\t\tsave changes and reopen mail file",
		"p\t\tprint letter",
		"p|cmd\t\tpipe to cmd (no delete)",
		"nnn\t\tprint letter number nnn",
		"s [file]\tsave (default $HOME/mbox)",
		"w [file]\tsame without header",
		"a\t\tanswer (deletes also)",
		"as [file]\tanswer and save (default $HOME/mbox)",
		"m users\t\tforward to users (deletes also)",
		"M users\t\tcomment and forward users (deletes also)",
		"-\t\tprint previous",
		"+\t\tnext (no delete)",
		"<\t\tprint previous undeleted letter",
		">\t\tprint next undeleted letter",
		"d\t\tdelete letter",
		"dq\t\tdelete letter and quit",
		"u\t\tundelete letter",
		"l\t\tlist table of contents",
		"!cmd\t\texecute cmd",
		"cd [directory]\tchange directory",
		"cd! [directory]\tchange directory",
		(char *) NULL
	};
	char	**mp;

	for (mp = mess; *mp != NULL; mp++)
		fprintf(stderr, "%s\n", *mp);
}

reenter(s, n)
char	*s;
int	n;
{
	fprintf(stdout, "Please reenter file name\n    or !command\n    or RETURN to cancel: ");
	fflush(stdout);
	if (fgets(s, n, stdin) == NULL)
		s[0] = '\0';
	else
		s[strlen(s) - 1] = '\0';	/* kill newline */
}


/* Any new mail arrive in the meantime? */
newmail()
{
	struct stat stbuf;

	stat(mailfile, &stbuf);
	return (stbuf.st_size != let[nlet].adr && !flgf);
}

copyback()	/* copy temp or whatever back to /usr/mail */
{
	register i, n, c;
	int new = 0, aret;
	struct stat stbuf;

	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	lock(mailfile);
	stat(mailfile, &stbuf);
	if (stbuf.st_size != let[nlet].adr) {
		/* new mail has arrived */
		malf = fopen(mailfile, "r");
		if (malf == NULL) {
			fprintf(stdout, "%s: can't re-read %s\n", Mailpgm, mailfile);
			error = 2;
			done();
		}
		fseek(malf, let[nlet].adr, 0);
		fclose(tmpf);
		tmpf = fopen(lettmp, "r+");
		fseek(tmpf, let[nlet].adr, 0);
		n = 0;
		while ((c = fgetc(malf)) != EOF)
		{
			fputc(c, tmpf);
			if(c == '\n')
				n++;
			if (!isascii(c))
				fprintf(stderr, "%s: Nonascii character (0%o) \
encountered during rewrite of %s\n", Mailpgm, mailfile);
		}
		fclose(malf);
		fclose(tmpf);
		tmpf = fopen(lettmp, "r");
		let[nlet].llns = n;
		let[++nlet].adr = stbuf.st_size;
		new = 1;
	}
	if((aret=access(mailfile, 2)) == 0)
		malf = fopen(mailfile, "w");
	if ((malf == NULL) || (aret)) {
		fprintf(stderr, "%s: can't rewrite %s\n", Mailpgm, mailfile);
		error = 2;
		done();
	}
	n = 0;
	for (i = 0; i < nlet; i++)
		if (let[i].change == 0) {
			copylet(i, malf, ORDINARY, "");
			n++;
		}
	fclose(malf);

	/*
	The standard /bin/mail command will delete the mail file
	if it becomes empty and it has default permissions.
	If it has nondefault permissions, it is not deleted so that
	the permissions are preserved.

	Xmail does the following:
	If MFUNLINK is defined, then we behave the same as /bin/mail.
	If MFUNLINK is undefined, then we delete the file iff it
	resides in the system mail directory (/usr/mail).

	Should we delete empty mailbox with default permissions?
	*/
	if (
		(n == 0)
		&& ((stbuf.st_mode & 0777) == MFMODE)
#ifdef	MFUNLINK
	/* Yes */
	)
#else
	/* Yes, if file not in /usr/mail */
		&& !imatch(maildir, mailfile)
	)
#endif
		unlink(mailfile);
	if (new && !flgf)
		fprintf(stdout, "new mail arrived\n");
	unlock();
}

copymt(f1, f2)	/* copy mail (f1) to temp (f2) */
FILE *f1, *f2;
{
	char	*topic();

	long nextadr;
	register int i;
	register int lines = 0;
	char	*lp;
	char	*sp;
	char	*cp;
	int	cnt;
	char	subject[LSIZE];
	char	heuristic[LSIZE];
	char	firstln[LSIZE];


	/* Do cleanup */
	for (i = 0; i < nlet; i++) {
		if (let[i].sender)
			free(let[i].sender);
		if (let[i].subject)
			free(let[i].subject);
		if (let[i].postmark)
			free(let[i].postmark);
		let[i] = NullLet;		/* ..STRUCTURE ASSIGNMENT.. */
	}
	nlet = nextadr = 0;
#if 0
	let[0].adr = 0;
#endif
	line[LSIZE-2] = 0;

	lp = fgets(line, LSIZE, f1);
	while (lp != NULL) {
		if (!Isfrom(line)) {
			fprintf(stderr, "%s: File %s not in mail format\n",
			    Mailpgm, mailfile);
			unlock();
			exit(1);
		}
		if(nlet >= (MAXLET-2)) {
			fprintf(stderr,
			    "%s: Too many letters (more than %d): split %s\n",
			    Mailpgm, MAXLET-3, mailfile);
			unlock();
			exit(1);
		}
		if(nlet > 0) {
			let[nlet-1].llns = lines;
			lines = 0;
		}
		let[nlet].postmark = getdate(line);
		let[nlet].adr = nextadr;
		let[nlet].change = 0;
		nextadr += strlen(line);
		fputs(line, f2);
		lines++;
		fradd(line);

		/* Copy remaining ">From" lines */
		lp = fgets(line, LSIZE, f1);
		while (lp != NULL && Isfrom(line+1)) {
			nextadr += strlen(line);
			fputs(line, f2);
			lines++;
			fradd(line);
			lp = fgets(line, LSIZE, f1);
		}

		/* Save address of sender of this letter */
		sp = frflsh(0);
		if ((cp = calloc(strlen(sp) + 1, 1)) != NULL) {
			strcpy(cp, sp);
		}
		let[nlet].sender = cp;

		/* Copy text lines after "From" and ">From"s */
		subject[0] = '\0';
		heuristic[0] = '\0';
		firstln[0] = '\0';
		while (lp != NULL && !Isfrom(line)) {
			/* First priority subject */
			if (subject[0] == '\0'
			&& (sp = topic(lp)) != NULL) {
				strncpy(subject, sp, sizeof subject);
				subject[sizeof subject - 1] = '\0';
			}

			/* Second priority subject */
			if (subject[0] == '\0'
			&& heuristic[0] == '\0'
			&& strlen(lp) > 12
			&& strncmp(lp, PREPEND, sizeof PREPEND - 1)
			&& strncmp(lp, TO, sizeof TO - 1)) {
				strncpy(heuristic, lp, sizeof heuristic);
				heuristic[sizeof heuristic - 1] = '\0';
			}

			/* Third priority subject */
			if (subject[0] == '\0'
			&& heuristic[0] == '\0'
			&& firstln[0] == '\0'
			&& *lp
			&& !allblank(lp)
			&& strncmp(lp, TO, sizeof TO - 1)) {
				strncpy(firstln, lp, sizeof firstln);
				firstln[sizeof firstln - 1] = '\0';
			}
			nextadr += strlen(line);
			fputs(line, f2);
			lines++;

			/* Handle long lines (> LSIZE bytes) */
			if (line[LSIZE-2] && line[LSIZE-2] != '\n') {
				do {
					line[LSIZE-2] = 0;
					if((lp = fgets(line, LSIZE, f1)) == NULL)
						break;
					nextadr += strlen(line);
					fputs(line,f2);
				} while (line[LSIZE-2] && line[LSIZE-2] != '\n');
			}
			line[LSIZE-2] = 0;
			lp = fgets(line, LSIZE, f1);
		}

		/* If letter has no subject, use first "significant" line */
		if (subject[0])
			sp = subject;
		else if (heuristic[0])
			sp = heuristic;
		else if (firstln[0])
			sp = firstln;
		else
			sp = NULL;
		if (sp != NULL) {
			while (*sp && isspace(*sp))
				++sp;
			if ((cp = strchr(sp, '\n')) != NULL)
				cnt = cp - sp;
			else
				cnt = strlen(sp);
			cnt = (cnt > 44) ? 44 : cnt;
			if ((cp = calloc(cnt + 1, 1)) != NULL) {
				strncpy(cp, sp, cnt);
				cp[cnt] = '\0';
				let[nlet].subject = cp;
			}
		} else {
			let[nlet].subject = NULL;
		}
		++nlet;
	}
	let[nlet].adr = nextadr;	/* last plus 1 */
	let[nlet-1].llns = lines;
}

allblank(s)
register	char	*s;
{
	while (*s) {
		if (!isspace(*s))
			return 0;
		++s;
	}
	return 1;
}

char	*
topic(lp)
char	*lp;
{
	static	char	*keyword[] = {
		"SUBJECT:",
		"Subject:",
		"RE:",
		"Re:",
		"re:",
		NULL
	};
	register	char	*sp;
	register	char	**kp;
	int	kn;

	/* skip over leading white space */
	while (*lp && isspace(*lp))
		++lp;

	/* see if remainder of line begins with a subject keyword */
	sp = NULL;
	for (kp = keyword; *kp; ++kp) {
		if (strncmp(lp, *kp, (kn = strlen(*kp))) == 0) {
			sp = lp + kn;
			break;
		}
	}

	/* return pointer to part of line after keyword or NULL */
	return sp;
}

void
toc()
{
	extern int bfrlns;
	extern int lpp;

	int	i;
	int	j;
	int	pgflg = 0;
	char	tocline[LSIZE];
	int	more;

	/* Assume direct access to mail (-l flag) */
	flgl = 1;

	if((nlet+1) >= 18 && ttyout) {
		pgflg = 1;
		bfrlns = ( (lpp-11) < 0) ? 0 : (lpp-11);
		pgflsh(0);		/* scratch anything in buffer */
	}

	sprintf(tocline, "You have %d messages in %s\n\n", nlet, mailfile);
	pgflg ? pgputs(tocline) : fputs(tocline, stdout);

	sprintf(tocline, "%3s  %-3.3s  %-12.12s  %-12.12s %s\n",
	    " NO", "LNS", "RECEIVED", "FROM", "SUBJECT");
	pgflg ? pgputs(tocline) : fputs(tocline, stdout);

	for (i = 0, more = 1; i < nlet && more == 1; ++i) {
		j = forward ? i : (nlet - i - 1);
		sprintf(tocline, "%3d%c %3d  %-12.12s  %-12.12s %-.40s\n",
		    SHOW(i),
		    (let[j].change) ? let[j].change : ' ',
		    let[j].llns,
		    let[j].postmark ? let[j].postmark : "",
		    fromtail(let[j].sender, 12),
		    let[j].subject ? let[j].subject : "");

		if (pgflg) {
			more = pgputs(tocline);
		} else {
			fputs(tocline, stdout);
		}
	}

	if (pgflg) {
		pgputs("\n");
		pgflsh(1);
	} else {
		fputs("\n", stdout);
	}
}

char	*
getdate(line)
char	*line;
{
	static	char	*month[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
		(char *) NULL
	};
	char	**mp;
	int	i;
	int	n;
	char	*cp;
	char	*tp;

	/* Find the month in the "From" line */
	for (mp = month; *mp != NULL; mp++) {
		if ((i = sindex(line, *mp)) >= 0) {
			/* Scan past spaces after month */
			cp = &line[i+3];
			while (*cp && isspace(*cp))
				++cp;

			/* Scan past digits in day */
			while (*cp && !isspace(*cp))
				++cp;

			/* Include time if it's there */
			if ((tp = strchr(cp, ':')) != NULL
			    && isdigit(tp[-2])
			    && isdigit(tp[-1]) 
			    && isdigit(tp[1]) 
			    && isdigit(tp[2]))
				    cp = &tp[3];

			/* Save postmark (e.g., "Aug  2") */
			n = cp - &line[i];
			if ((cp = calloc(n+1, 1)) != NULL) {
				strncpy(cp, &line[i], n);
				cp[n] = '\0';
			}
			break;
		}
	}
	return (*mp == NULL || cp == NULL) ? NULL : cp;
}


char	*
fromtail(s, n)
char	*s;
int	n;
{
	static	char	sbuf[32];
	char	*r;
	char	*or;

	if (strlen(s) <= n)
		return s;

	for (or = s; (strlen(or)+3)>n && (r = strchr(or, '!')); or = r+1)
		;

	if (or == s)
		return s;

	strcpy(sbuf, "..!");	/* assumes uname() is at least 3 chars */
	strcat(sbuf, or);
	return sbuf;
}

areforwarding(s)
char *s;
{
	FILE *fd;
	char fbuf[256], *p;
	int c;

	fd = fopen(s, "r");
	if(fd == NULL)
		return(0);
	fread(fbuf, sizeof(frwrd) - 1, 1, fd);
	if(strncmp(fbuf, frwrd, sizeof(frwrd) - 1) == 0) {
		for(p = sendto; (c = getc(fd)) != EOF && c != '\n';)
			if(c != ' ') *p++ = c;
		*p = 0;
		fclose(fd);
		return(1);
	}
	fclose(fd);
	return(0);
}

copylet(n, f, type, prepnd)
FILE *f;
char *prepnd;
{	int ch, k;
	register int i=0;
	register int fromon;
	register int pgflg;
	register char *p;
	char frline[LSIZE];
	char lbuf[1024];
	register int lines;
	extern int bfrlns;
	extern int lpp;

	if(prepnd == NULL)
		prepnd = "";
	lines = let[n].llns;
	if (fseek(tmpf, let[n].adr, 0) < 0) {
		fprintf(stderr, "%s: Cannot seek to %d line letter %d at %ld\n",
			Mailpgm, lines, n, let[n].adr);
		return FALSE;
	}
	pgflg = 0;
	bfrlns = ( (lpp-11) < 0) ? 0 : (lpp-11);
	if(lines >= 18 && f == stdout && type==ORDINARY && ttyout && !flgp) {
		pgflg = 1;
		pgflsh(0);		/* scratch anything in buffer */
	}
	fgets(lbuf, sizeof(lbuf), tmpf);
			
	if(type == ORDINARY && f == stdout)
		fradd(lbuf);
	lbuf[strlen(lbuf)-1] = 0;

	/* Sanity test -- first line must be a `From ...' line */
	if (!equaln(lbuf, from, 5)) {
		fprintf(stderr, "INTERNAL ERROR! `From' expected, `%s' found\n\
\tletter %d, seek at %ld, %d lines",
			lbuf, n, let[n].adr, lines);
		return FALSE;
	}

	ascck(lbuf);
	if(type!=ZAP)
		if(pgflg)
		{
			if(*prepnd)
				pgputs(prepnd);
			pgputs(lbuf);
		}
		else
		{
			if(*prepnd)
				fputs(prepnd,f);
			fputs(lbuf,f);
		}
	if(type==REMOTE)
		fprintf(f, rmtmsg, thissys);
	else if(type==FORWARD)
		fprintf(f, forwmsg, my_name);
	else if(type==ORDINARY)
		if(pgflg)
			pgputs("\n");
		else
			fputc('\n',f);
	fromon = 1;
	lbuf[sizeof(lbuf)-2] = 0;
	while( --lines > 0 )
	{
		if(fgets(lbuf, sizeof(lbuf), tmpf) == NULL)
			break;
		ascck(lbuf);

		/* save the last ">From ... " line if it exists. */
		if(*(p=lbuf) == '>')
			p++;
		if(*p != 'F')
			p = NULL;
		if(fromon && p && type == ORDINARY && f == stdout && Isfrom(p))
			fradd(lbuf);
		else
			fromon = 0;
		if(pgflg) {
			if(*prepnd)
				pgputs(prepnd);
			if((i=pgputs(lbuf)) <= 0) {
				pgflsh(0);
				if(i == 0)
					fputc('\n', f);
				return TRUE;
			}
		} else {
			if(*prepnd)
				fputs(prepnd, f);
			fputs(lbuf, f);
		}
		/*
		 *	Handle long lines
		 */
		if(lbuf[sizeof(lbuf)-2] && lbuf[sizeof(lbuf)-2] != '\n')
		{
			do
			{
				lbuf[sizeof(lbuf)-2] = 0;
				fgets(lbuf, sizeof(lbuf), tmpf);
				ascck(lbuf);
				if(pgflg) {
					if(pgputs(lbuf) <= 0)
						return TRUE;
				} else
					fputs(lbuf, f);
			} while(lbuf[sizeof(lbuf)-2] && lbuf[sizeof(lbuf)-2] == '\n');
		}
		lbuf[sizeof(lbuf)-2] = 0;
	}
	if(pgflg) {
		pgflsh(1);
		return TRUE;
	} else {
		fflush(f);
		return ferror(f) ? FALSE : TRUE;
	}
}

ascck(s)
char	*s;
{
	char	*sp = s;

	if (s != NULL) {
		while (*sp) {
			if (!isascii(*sp))
				fprintf(stderr,
				    "%s: Nonascii character 0%o\nLine: %s\n",
				    Mailpgm, *sp, s);
			++sp;
		}
	}
}


sendmail(argc, argv)
char **argv;
{
	struct group *pgrp;
	char *gp;
	register int i;
	register char **pal;
	register char *p;
	register char *q;
	int	aret;
	char	**args;
	char	hbuf[256];
	char	tobuf[TOSIZE];
	char	dst[64];		/* buffer for one destination */
	char	*tp;
	FILE	*textfp;

	time(&iop);
	sprintf(hbuf, "%s%s %24.24s%s\n", from, my_name, ctime(&iop), tz);
	hbuf[255] = 0;
	tobuf[0] = 0;
	DestSub = 0;
	if(1 || argc > 2) {		/* (Always) send Copy to message */
		p = tobuf;
		for(i = 1; i < argc; i++)
		{
			q = argv[i];
			*p++ = ' ';
			while(*p++ = *q++)
				if(p >= &tobuf[sizeof(tobuf)-1])
				{
					fprintf(stderr, "%s: Too many addressees\n", Mailpgm);
					error = 2;
					return;
				}
			p--;
		}
		tobuf[TOSIZE - 1] = '\0';
		strcpy(tobuf, finddest(tobuf, ""));
	}

	/* Get text of letter from standard input */
	saveint = setsig(SIGINT, savdead);
	curlet = -1;
	textfp = readinput(stdin, tobuf);
	setsig(SIGINT, saveint);
	if (textfp == NULL)
		return;

	envelope(tmpf, hbuf, tobuf, textfp);

	nlet = 1;
	let[0].adr = 0;
	let[0].llns = Readio.r_lines;
	let[1].adr = Readio.r_size;

	tmpf = fopen(lettmp, "r");
	if (tmpf == NULL) {
		fprintf(stderr, "%s: cannot reopen %s for reading\n",
			Mailpgm, lettmp);
		error = 2;
		return;
	}
	if (error == 0) {
		/* Write log entry */
		if (logf != NULL)
			copylet(0, logf, ORDINARY, "");

		if (DestSub)
			printf("Sending to: %s\n\n", tobuf);

		for (tp = tobuf; tp = getarg(dst, tp); ) {
			if (equal(dst, "-g")) {
				if ((tp = getarg(dst, tobuf)) == NULL)
					break;
				if ((pgrp=getgrnam(gp = dst)) == NULL) {
					error++;
					fprintf(stderr, "Unknown group %s\n",
						gp);
					continue;
				}
				for(i = 0; pgrp->gr_mem[i] && pgrp->gr_mem[i][0]; i++)
				{
					if (!send(0, pgrp->gr_mem[i], 0))
						error++;
					else
						fprintf(stdout, "\t%s group: %s\n",
							gp, pgrp->gr_mem[i]);
				}
				continue;
			}
			for (pal = alias(dst, 1); *pal; pal++)
				if (!send(0, *pal, 0))
					error++;
		}
	}
	if (error) {
		if(storedead(0, dead))
		{
			fprintf(stdout, "%s: cannot create %s\n", Mailpgm, dead);
			error = 2;
			return;
		}
	}
	fclose(tmpf);
}

savdead()
{
	setsig(SIGINT, saveint);
	error++;
}

#ifdef	LOCAL
/*
 * send mail to remote system taking fowarding into account
 *	n	-> index into mail table
 *	name	-> mail destination
 * returns
 *	TRUE	-> sent mail
 *	FALSE	-> can't send mail
 */
sendrmt(n, name)
register char *name;
{
# define NSCCONS	"/usr/nsc/cons/"
	register char *p;
	register local;
	FILE *rmf, *popen();
	char rsys[64], cmd[200];
	char remote[30];

	/*
	 * assume mail is for remote
	 * look for bang to confirm that
	 * assumption
	 */
	local = 0;
	while (*name=='!')
		name++;
	for(p=rsys; *name!='!'; *p++ = *name++)
		if (*name=='\0') {
			local++;
			break;
		}
	*p = '\0';
	if ((!local && *name=='\0') || (local && *rsys=='\0')) {
		fprintf(stderr, "null name\n");
		return(FALSE);
	}
	if (local)
		sprintf(cmd, "mail %s", rsys);
	if (strcmp(thissys, rsys) == SAME) {
		local++;
		sprintf(cmd, "mail %s", name+1);
	}

	/*
	 * send local mail or remote via uux
	 */
	if (!local) {
#ifdef USE_NUSEND
		/*
		 * If mail can't be sent over NSC network
		 * use uucp.
		 */
		if (setjmp(nusendfail) == 0) {
			nusendjmp = TRUE;
			sprintf(remote, "%s%s", NSCCONS, rsys);
			if (access(remote, A_EXIST) != CERROR) {
				/*
				 * Send mail over NSC network
				 */
				sprintf(cmd, "nusend -d %s -s -e -!'rmail %s' - 2>/dev/null",
					rsys, name);
#ifdef DEBUG
printf("%s\n", cmd);
#endif
				if ((rmf=popen(cmd, "w")) != NULL) {
					copylet(n, rmf, local? FORWARD: REMOTE, "");
					if (pclose(rmf) == 0) {
						nusendjmp = FALSE;
						return(TRUE);
					}
				}
			}
		}
		nusendjmp = FALSE;
#endif

#ifdef USE_USEND
		if (setjmp(usendfail) == 0) {
			usendjmp = TRUE;
			sprintf(cmd, "usend -s -d%s -uNoLogin -!'rmail %s' - 2>/dev/null",
				rsys, name);
#ifdef DEBUG
printf("%s\n", cmd);
#endif
			if ((rmf=popen(cmd, "w")) != NULL) {
				copylet(n, rmf, local? FORWARD: REMOTE, "");
				if (pclose(rmf) == 0) {
					usendjmp = FALSE;
					return(TRUE);
				}
			}
		}
		usendjmp = FALSE;
#endif

		/*
		 * Use uux to send mail
		 */
		if (strchr(name+1, '!'))
			sprintf(cmd, "uux - %s!rmail \\(%s\\)", rsys, name+1);
		else
			sprintf(cmd, "uux - %s!rmail %s", rsys, name+1);
	}
#ifdef DEBUG
printf("%s\n", cmd);
#endif
	/*
	 * copy letter to pipe
	 */
	if ((rmf=popen(cmd, "w")) == NULL)
		return(FALSE);
	if (copylet(n, rmf, local? FORWARD: REMOTE, "") == FALSE) {
		fprintf(stderr, "mail: cannot pipe to mail command\n");
		pclose(rmf);
		return(FALSE);
	}

	/*
	 * check status
	 */
	return(pclose(rmf)==0 ? TRUE : FALSE);
}
#else
sendrmt(n, name)
char *name;
{
	FILE *rmf, *popen();
	register char *p;
	char rsys[256], cmd[256];
	register local;

	local = 0;
	if (*name=='!')
		name++;
	for(p=rsys; *name!='!'; *p++ = *name++)
		if (*name=='\0') {
			local++;
			break;
		}
	*p = '\0';
	if ((!local && *name=='\0') || (local && *rsys=='\0')) {
		fprintf(stdout, "null name\n");
		return(0);
	}
	if (local)
		sprintf(cmd, "%s %s", Mailpgm, rsys);
	else {
		if (strchr(name+1, '!'))
			sprintf(cmd, "uux - %s!rmail \\(%s\\)", rsys, name+1);
		else
			sprintf(cmd, "uux - %s!rmail %s", rsys, name+1);
	}
	if ((rmf=popen(cmd, "w")) == NULL)
		return(0);
	copylet(n, rmf, local? FORWARD: REMOTE, "");
	return(pclose(rmf)==0 ? 1 : 0);
}
#endif

send(n, name, level)	/* send letter n to name */
int n;
char *name;
{
	char	file[50], *p;
	struct	passwd	*pwd, *getpwname();

	if(level > 20) {
		fprintf(stdout, "unbounded forwarding\n");
		return(0);
	}
	if (strcmp(name, "-") == 0)
		return(1);
	for(p=name; *p!='!' &&*p!='\0'; p++)
		;
	if (*p == '!') {
		return(sendrmt(n, name));
	}
	cat(file, maildir, name);
	if(areforwarding(file))
		return(send(n, sendto, level+1));
	setpwent();	/* rewind password file -- dumb design */
	pwd = getpwnam(name);
	if (pwd == NULL) {
		fprintf(stdout, "%s: can't send to %s\n", Mailpgm, name);
		return(0);
	}
	cat(file, maildir, name);
	lock(file);
	if(access(file, 0) < 0) {
		umask(0);
		close(creat(file, MFMODE));
		umask(7);
		chown(file, pwd->pw_uid, getegid());
	}
	malf = fopen(file, "a");
	if (malf == NULL) {
		fprintf(stdout, "%s: cannot append to %s\n", Mailpgm, file);
		unlock();
		return(0);
	}
	copylet(n, malf, ORDINARY, "");
	fclose(malf);
	unlock();
	return(1);
}

delete(i)
{
	setsig(i, delete);
	if(i>3)
	{
		fprintf(stderr, "%s: error signal %d\n", Mailpgm, i);
		if(fork() == 0)
		{
			signal(3, SIG_DFL);
			kill(getpid(), 3);	/* generate a core dump */
			exit(1);
		}
	}
	else
		fprintf(stderr, "\n");
	if(delflg)
		longjmp(sjbuf, 1);
	done();
}

done()
{
	unlock();
	unlink(lettmp);
	unlink(rmtbuf);
	unlink(edtmp);
	exit(error);
}

lock(file)
char *file;
{
	int f, i;

	if (locked)
		return;
	cat(curlock, file, maillock);
	for (i=0; i<10; i++) {
		f = creat(curlock, 0);
		if (f >= 0) {
			close(f);
			locked = 1;
			return;
		}
		sleep(2);
	}
	fprintf(stderr, "%s: %s not creatable after %d tries\n", Mailpgm, curlock, i);
	error = 2;
	done();
}

unlock()
{
	unlink(curlock);
	locked = 0;
}

cat(to, from1, from2)
register char *to, *from1, *from2;
{
	register int i, j;

	j = 0;
	for (i=0; from1[i]; i++)
		to[j++] = from1[i];
	for (i=0; from2[i]; i++)
		to[j++] = from2[i];
	to[j] = 0;
}

char *getarg(s, p)	/* copy p... into s, update p */
register char *s, *p;
{
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\n' || *p == '\0')
		return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
		*s++ = *p++;
	*s = '\0';
	return(p);
}

legal(file)
char *file;
{
	char	*sp, dfile[100];
	char	*expand();

	if (expand(file, EX_UNIQ) == NULL)	/* expand shell variables */
		return(0);
	if(!access(file, 0))			/* if file exists */
		if(!access(file, 2))		/* and can write it */
			return(1);		/* return 1 */
		else	return(0);		/* else can't write it */
	else {
		if((sp=strrchr(file, '/')) == NULL)
			cat(dfile, ".", "");
		else {
			strncpy(dfile, file, sp - file);
			dfile[sp - file] = '\0';
		}
		if(access(dfile, 2)) return(0);
		return(2);
	}
}

system(s)
char *s;
{
	int status, pid, w;
	register int (*istat)(), (*qstat)();

	if ((pid = fork()) == 0) {
		setgid(getgid());
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern FILE *fdopen();
extern int execl(), fork(), pipe(), close(), fcntl();
static int popen_pid[20];

FILE *
popen(cmd, mode)
char	*cmd, *mode;
{
	int	p[2];
	register int myside, yourside, pid;

	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;

		stdio = tst(0, 1);
		(void) close(myside);
		(void) close(stdio);
		(void) fcntl(yourside, 0, stdio);
		(void) close(yourside);
		setgid(getgid());	/* Heuristic restoral of group ID */
		(void) execl("/bin/sh", "sh", "-c", cmd, 0);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);
	popen_pid[myside] = pid;
	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclose(ptr)
FILE	*ptr;
{
	register int f, r;
	int status, (*hstat)(), (*istat)(), (*qstat)();

	f = fileno(ptr);
	(void) fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if(r == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
	return(status);
}
/*
 *	sindex looks for an occurance  of the string s2 in the string
 *	s1. If s2 does not ocurr, sindex returns -1. Otherwise, sindex
 *	returns the integer offset from s1 where s2 ocurrs.
 */
sindex(s1,s2)
	char *s1;
	char *s2;
{
	register char *p1;
	register char *p2;
	register int flag;
	int ii;
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
};
/*
 *	This routine is now a subroutine so it can be used by
 *	the answer capability. It can also do more fancy things
 *	like fork off the editor if the user needs the editor
 *	to type his letter.
 *
 *	return:	FILE *, pointing to file containing input, or
 *		NULL, if error or there was no input
 */
FILE	*
readinput(inf, tobuf)
FILE *inf;
char	*tobuf;
{
	register char *p;
	char buf[128];
	static first = 1;
	FILE *edfp;
	long sum;
	char	nline[LSIZE];
	char	oldtobuf[TOSIZE];

	if(first)
	{
		first = 0;
		mktemp(edtmp);
		unlink(edtmp);
	}
	if((edfp=fopen(edtmp,"w+")) == NULL)
	{
		fprintf(stderr, "Cannot creat temp file\n");
		return NULL;
	}
	while ((p = fgets(line, LSIZE, inf)) != NULL) {
		if( !rmail && ttyin && (line[0] == ':' || line[0] == ';')) {
			if(line[1] == '!')
			{
				char *cmd;

				if(line[2] == '\n')
					cmd = "sh -i";
				else
					cmd = &line[2];
				system(cmd);
				fprintf(stderr, "!\n");
				fprintf(stderr, ":\n");
				continue;
			} else if (line[1] == 's') {
				fputs("Subject: ", edfp);
				for (p = &line[2]; *p && isspace(*p); ++p)
					;
			} else if (line[1] == 't') {
				for (p = &line[2]; *p && isspace(*p); ++p)
					;
				if (*p == '\0') {
					printf("%s\n", finddest(tobuf, ""));
				} else {
					/* Caveat: tobuf must be big enough! */
					strcpy(oldtobuf, tobuf);
					strcpy(tobuf, finddest(p, oldtobuf));
					printf("%s\n", tobuf);
				}
				continue;
			}
			else if(line[2] == '\n')
			{
				switch (line[1]) {

				case 'v':
				case 'e':
					sprintf(buf, "%s %s", (line[1]=='v')?"vi":"ed", edtmp);
					fprintf(stderr, "%s\n", buf);
					fclose(edfp);
					system(buf);
					if((edfp=fopen(edtmp, "r+")) == NULL)
					{
						fprintf(stderr,"Cannot reopen temp file\n");
						return NULL;
					}
					fseek(edfp, 0L, 2);
					break;
				case 'r':
					if(curlet >= 0)
						copylet(curlet, edfp, ORDINARY, PREPEND);
					else
						fprintf(stderr, "Not answer mail, nothing to read\n");
					break;
				case 'x':
					fclose(edfp);
					return NULL;
				case 'p':
					fseek(edfp, 0L, 0);
					while(fgets(line, LSIZE, edfp) != NULL)
						fputs(line, stdout);
					fflush(stdout);
					fseek(edfp, 0L, 2);	/* as per man page on "w+" */
					break;
				default:
					fprintf(stderr, "Type :? for list of valid commands\n");
					break;
				case '?':
					fprintf(stderr, ":s	shorthand for \"Subject: \"\n");
					fprintf(stderr, ":t	print recipients\n");
					fprintf(stderr, ":t person ...\n\t\
change destination list to named persons\n");
					fprintf(stderr, ":v	vi editor\n");
					fprintf(stderr, ":e	ed\n");
					fprintf(stderr, ":r	read copy of letter into answer\n");
					fprintf(stderr, ":p	print letter to current position\n");
					fprintf(stderr, ":x	exit, no contents\n");
					fprintf(stderr, ":!cmd	execute shell cmd\n");
					fprintf(stderr, ":?	this message\n");
					fprintf(stderr, "\n`;' may be used instead of `:'\n");
					fflush(stderr);
					break;
				}
				fprintf(stderr, ":\n");
				continue;
			}
		}
		if (line[0] == '.' && line[1] == '\n')
			break;
		fputs(p, edfp);
	}
	sum = ftell(edfp);
	if (sum) {
		if (ttyin) {
			printf("EOT\n");
			fflush(stdout);
		}
	} else {
		fclose(edfp);
		edfp = NULL;
	}
	return edfp;
}

/* Prefix input with `From' header and optional 'To: ' line */
envelope(outf, fmbuf, tobuf, textfp)
FILE	*outf;
char	*fmbuf;
char	*tobuf;
FILE	*textfp;
{
	register	char	*p;
	char	line[LSIZE];

	Readio.r_size = ftell(textfp);
	Readio.r_nlcnt = 0;
	Readio.r_lines = 0;
	if (Readio.r_size) {
		fseek(textfp, 0L, 0);

		/* write out `From' header */
		Readio.r_size += strlen(fmbuf);
		for(p = fmbuf; p=strchr(p,'\n'); p++)
			Readio.r_lines++;
		fputs(fmbuf, outf);

		/* if there's a `To:' line, write it out */
		if (tobuf != NULL) {
			Readio.r_size += (strlen(tobuf) + sizeof TO);
			Readio.r_lines++;
			fprintf(outf, "%s%s\n", TO, tobuf);
		}

		while (fgets(line, LSIZE, textfp) != NULL) {
			if(line[0] == '\n')
				Readio.r_nlcnt++;
			else {
				/*
				 *	Although this version of mail can
				 *	parse "From " lines better than most
				 *	we have to escape the /^From / sequences
				 *	for older versions, so they don't get
				 *	confused.
				 */
				if(sindex(line, "From ") == 0) {
					Readio.r_size++;
					fputc('>', outf);
				}
				Readio.r_nlcnt = 0;
			}
			Readio.r_lines++;
			fputs(line, outf);
		}

		if (Readio.r_nlcnt < 2) {
			fputc('\n', outf);
			Readio.r_lines++;
			Readio.r_size++;
			Readio.r_nlcnt++;
		}

		fflush(outf);
	}

	fclose(textfp);
	fclose(outf);
}

static char globalias[] = "/etc/aliaslist";
static char alname[256];
static char *allist[128];

char **
alias(name, report)
register char *name;
int	report;
{
	register char *p;
	register char **pp;
	char buf[256];
	static char sbuf[256];
	static int opens = 0;
	static char alfile[256];
	static FILE *alfp;
	FILE *fp;
	char **alscan();
	char *getenv();

	if(opens == 0)
	{
		opens++;
		if(home)
		{
			strcpy(alfile, home);
			strcat(alfile, "/.alias");
			/*
			 *	This stays open for duration of execution.
			 */
			alfp = fopen(alfile,"r");
		}
	}
	p = name;
	if(alfp)
	{
		fseek(alfp, 0L, 0);
		pp = alscan(alfp, name, report);
		if(!equal(pp[0],name))
			return(pp);
	}
	if((fp=fopen(globalias,"r")) != NULL)
	{
		pp = alscan(fp,name, report);
		fclose(fp);
	}
	else
	{
		strcpy(sbuf, name);
		allist[0] = &sbuf[0];
		allist[1] = 0;
		pp = &allist[0];
	}
	return(pp);
}
char **
alscan(fp, name, report)
register FILE *fp;
register char *name;
int	report;
{
	static char buf[512];
	register char *p;
	register char *n, *nn;
	register char **pal;
	register int i = 0;
	char *strtok();

	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		buf[strlen(buf)-1] = 0;		/* newline */
		n = strtok(buf, " \t");
		nn = strtok(0, " \t");
		if(!*n || !*nn)
			continue;
		if(equal(n, name))
		{
			strcpy(alname,nn);
			n = strtok(alname, ",");
			do
			{
				allist[i++] = n;
			} while(n = strtok(0, ","));
			allist[i] = 0;
			DestSub += i;
			if (report) {
				for(pal = &allist[0]; *pal; pal++)
					fprintf(stderr,"%s\n", *pal);
			}
			return(&allist[0]);
		}
	}
	strcpy(buf, name);
	allist[0] = buf;
	allist[1] = 0;
	return(&allist[0]);
}

#define	BACKOUT()	fprintf(stderr, "Too many adressees\n");\
			strcpy(xbuf, oldtobuf);\
			goto out
char	*
finddest(strng, oldtobuf)
char	*strng;			/* map to list of addressees */
char	*oldtobuf;		/* substitution string for '%' */
{
	char	*sp, sbuf[TOSIZE];
	char	*tp, tbuf[TOSIZE];
	char	*xp, xbuf[TOSIZE];
	char	*cp;
	int	delta;
	char	**pal;
	struct	group	*pgrp;
	int	i;

	/* Substitute each '%' by previous list of addressees */
	for (sp = strng, xp = xbuf; *sp; ) {
		if (*sp == '%') {
			/* room for string so far, + previous , + '\0' ? */
			if (((xp - xbuf) + strlen(oldtobuf) + 1) > TOSIZE) {
				BACKOUT();
			}
			tp = oldtobuf;
			while (*xp++ = *tp++);
			--xp;	/* back up to null character */
			++sp;	/* skip % */
		} else {
			/* room for string so far, + next character, + '\0' ? */
			if ((xp - xbuf + 2) > TOSIZE) {
				BACKOUT();
			}
			*xp++ = *sp++;
		}
	}
	*xp = '\0';

	/* Expand shell metacharacter expressions */
	strcpy(sbuf, xbuf);
	if (expand(sbuf, EX_ANY) == NULL) {
		/* BACKOUT() -- expand() failed to execute echo(1) */
		strcpy(xbuf, oldtobuf);
		goto out;
	} else {
		/* indicate that there was a substitution */
		if (!equal(sbuf, xbuf)) {
			DestSub++;
		}
	}

	/* Now expand aliases and groups */
	tp = sbuf;
	xp = xbuf;
	while ((tp = getarg(tbuf, tp)) != NULL) {
		if (equal(tbuf, "-g")) {
			/* Get name of group */
			if ((tp = getarg(tbuf, tp)) == NULL)
				break;

			/* Translate group to group members */
			if ((pgrp = getgrnam(tbuf)) == NULL) {
				fprintf(stderr, "Unknown group %s\n", tbuf);
				continue;
			}

			/* Make sure translation fits */
			delta = 1;	/* leave room for null terminator */
			for (i = 0; pgrp->gr_mem[i] && pgrp->gr_mem[i][0]; i++)
				delta += strlen(pgrp->gr_mem[i]) + 1;
			if (xp - xbuf + delta > TOSIZE) {
				BACKOUT();
			}

			/* Replace name by its members */
			for (i = 0; pgrp->gr_mem[i] && pgrp->gr_mem[i][0]; i++) {
				if (xp != xbuf)
					*xp++ = ' ';
				for (cp = pgrp->gr_mem[i]; *xp++ = *cp++; )
					;
				--xp;
			}
			DestSub += i;
		} else {
			for (pal = alias(tbuf, 0); cp = *pal; pal++) {
				delta = ((xp != xbuf) ? 1 : 0) + strlen(cp);
				if (xp - xbuf + delta > (TOSIZE - 1)) {
					BACKOUT();
				}
				if (xp != xbuf)
					*xp++ = ' ';
				while (*xp++ = *cp++); --xp;
			}
		}
	}
	*xp = '\0';
out:
	/* Eliminate new lines from result */
	if ((xp = strchr(xbuf, '\n')) != NULL)
		*xp = '\0';
	return xbuf;
}

long
fsize(file)
char *file;
{
	struct	stat stbuf;

	if(stat(edtmp, &stbuf) == -1)
		printf("Cannot stat %s\n", edtmp);
	else
		return(stbuf.st_size);
	return(0L);
}
/*
 *	Determine if line is genuine article.
 *	Should be of 2 forms:
 *		From.*[0-9][0-9]:[0-9][0-9].*
 *			or
 *		>From.*[0-9][0-9]:[0-9][0-9].*
 *
 *	The [0-9] stuff is the date representation by ctime.
 *	Some systems use only the 09:43 part of the time (i.e.
 *	no seconds representation).
 */
Isfrom(line)
register char *line;
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
}

char	*frlist[50];
int	frcnt;

fradd(buf)
register char *buf;
{
	frlist[frcnt] = calloc(strlen(buf)+2, 1);
	strcpy(frlist[frcnt], buf);
	frcnt++;
}
char *
frflsh(flg)
{
	register int i;
	register char *p;
	register char *buf;
	register int cnt = 0;
	register int j;
	int fwrdflg = 0;
	char bigbuf[TOSIZE];
	char *parts[50];
	static char address[TOSIZE];

	if(frcnt == 0)
		return("");
	/*
	 *		PARSE THIS MESS
	 */

	buf = bigbuf;
	for(i = 0; i < frcnt; i++)
	{
		if((j=sindex(frlist[i], rmtfrom)) >= 0)
		{
			p = &frlist[i][j+sizeof(rmtfrom)-1];
			/*
			 *	Copy machine name into bigbuf
			 */
			parts[cnt++] = buf;
			while (!isspace(*p))
				*buf++ = *p++;
			*buf++ = 0;
		}
		else if((j=sindex(frlist[i], fwrdmsg)) >= 0)
		{
			p = &frlist[i][j+sizeof(fwrdmsg)-1];
			parts[cnt++] = buf;
			while (!isspace(*p))
				*buf++ = *p++;
			*buf++ = 0;
			if (isdaemon(parts[cnt - 1])) {
				buf = parts[--cnt];
			} else {
				fwrdflg = 1;
				break;
			}
		}
	}
	/*
	 *	Parse out the name. Names which have "uucp" or "**NSC**"
	 *	in them are not real. Hence, go back to previous
	 *	frlist line. If a "forwarded by" msg has been
	 *	seen, then the name was already got.
	 */
	if(fwrdflg == 0)
		for(i = frcnt-1; i >= 0; i--)
		{
			p = frlist[i];
			if(*p == '>')
				p++;
			p += 5;				/* Skip "From " */
			parts[cnt++] = buf;
			while(*p && *p != ' ' && *p != '\t' && *p != '\n')
				*buf++ = *p++;
			*buf++ = 0;
			if(sindex(parts[cnt-1], "uucp") == -1
			  && sindex(parts[cnt-1], "**NSC**") == -1)
				break;
			cnt--;
			buf = parts[cnt];
		}
	for(i = 0; i < frcnt; i++)
		cfree(frlist[i]);
	buf = address;
	for(i = 0; i < cnt; i++)
	{
		for(p = parts[i]; *p; p++)
			*buf++ = *p;
		*buf++ = '!';
	}
	*--buf = 0;
	if(flg)
		fprintf(stderr,"%s\n", address);
	frcnt = 0;
	return(address);
}

storedead(letno, file)
register int letno;
register char *file;
{
	int aret;
	FILE *malf;
	char dfile[256];
	register char *p;

	umask(umsave);
	strcpy(dfile, file);
	if((malf=fopen(dfile, "w")) == NULL)
	{
		if(!(p=strrchr(file, '/')))
		{
			cat(dfile, home, "/");
			strcat(dfile, file);
		}
		else
			cat(dfile, home, p);
		if((malf=fopen(dfile, "w")) == NULL)
			return(1);
	}
	if((malf=fopen(dfile, "w")) == NULL)
		return(1);
	umask(7);
	copylet(letno, malf, ZAP, "");
	fclose(malf);
	chmod(file, 0600);
	fprintf(stderr, "%s: text saved in %s\n", Mailpgm, dfile);
	return(0);
}

/*----------------------BEGINNING OF pgputs MODULE---------------------*/
#include	<termio.h>

char *bs(x)char *x;{return(x);}	/* Disable underline and bold processor */


#define INIT		register char *pav = instring;
#define GETC()		*pav++
#define PEEKC()		(*pav)
#define UNGETC()	(--pav)
#define ERROR(c)	return(regerr=c, (char *)-1)
#define RETURN(a)	return(0);

static int	regerr;
static char	exbuf[512];

#include "regexp.h"

#define SKIP		1
#define RE		2


void	rest(), reset();
char	*malloc(), *bbuf, *getenv();
char	*getpat();

/*
 *	Following four globals are user accessible.
 */
int		lpp	= 23;			/* lines per page */
int		cpl	= 79;			/* chars per line */
char		*prompt	= ":";			/* prompt string */
char		quit	= 'q';			/* quit character */
int		bfrlns	= 0;			/* cur # of lines buffered */

static int	totmem;
static int	curmem;
static struct	termio tb, stb;
static int	tfd;
static char	*skipmsg = "\r\n\r\n   --- Skipping %d lines --- \r\n\r\n";
static char	linbuf[260]; 
static char	msgbuf[128];
static int	eraselen;			/* how much to erase */
static int	plen;				/* length of prompt */
static int	exitcode;
static int	skipmode;			/* skip input */
static int	skipcnt;
static int	skipc;
static int	intr = 0;
static jmp_buf	jmpbuf;

/*
 *	paging subroutine.
 *	Returns:	-1	-- error (see errno)
 *			 0	-- user requested exit
 *			 1	-- success
 */
pgputs(buf)
register char *buf;
{
	register int lns;		/* lines occupied by current buffer */
	static int first = 1;

	if(first == 1)
	{
		pginit();
		first = 0;
	}
	exitcode = 1;
	if(!*buf)
		return(exitcode);
	if(skipmode && skipline(buf))
		return(exitcode);
	lns = (((screenlen(buf)-1) - 1) / cpl) + 1;
	if(bfrlns+lns > lpp)
		rest("");
	if(exitcode == 1)
	{
		if(skipmode && skipline(buf))
			return(exitcode);
		apnd(buf);
		bfrlns += lns;
	}
	return(exitcode);
}

static
apnd(lbuf)
char *lbuf;
{
	register char *p;
	register int len;
	char *bs();

	p = bs(lbuf);		/* underline and bold processor */
	if((curmem + (len=strlen(p))) > totmem)
		pgflsh(1);
	strcpy(&bbuf[curmem], p);
	curmem += len;
}

static void
rest(str)
char *str;
{
	char lbuf[260], *cp;
	static int lppsave;
	int oldsig;

	pgflsh(1);
	bfrlns = 0;
	if(exitcode != 1)
		return;
	if(lppsave)
	{
		lpp = lppsave;
		lppsave = 0;
	}
	if((tfd = open("/dev/tty", 2)) == -1)
	{
		exitcode = -1;
		return;
	}
	getty();
	if(setjmp(jmpbuf))
	{
		pgflsh(0);
		goto out;
	}
	intr = 0;
	oldsig = (int)signal(SIGINT, reset);
	for(;intr == 0;)
	{
		rawmode();
		write(tfd, prompt, plen);
		cp = lbuf;
		eraselen = plen;
		lbuf[0] = 0;
		while( read(tfd,cp,1) == 1)
		{
			if (*cp == '\n') {
				eraselen += (cp - &lbuf[0]);
				*cp = '\0';
				break;
			}
			else if(*cp == ' ' && cp == lbuf) {
				lppsave = lpp;
				lpp = 1;
				lbuf[0] = 0;
				break;
			}
			else if (*cp == tb.c_cc[VERASE]) {
				if (tb.c_lflag&ECHOE) {
					write(tfd, "\b \b", 3);
				}
				else	write(tfd, cp, 1);
				if (cp > lbuf) --cp;
			}
			else if (*cp == tb.c_cc[VKILL]) {
				write(tfd, cp, 1);
				if (tb.c_lflag&ECHOK)
				{
					eraselen += (cp-&lbuf[0]);
					erasep();
				}
				write(tfd, prompt, plen);
				eraselen = plen;
				cp = lbuf;
			}
			else write(tfd, cp++, 1);
		}

		if (isdigit(lbuf[0]))
		{
			unsigned i;

			i = atoi(lbuf);
			lpp = (i > 23) ? 23:i;
			break;
		}
		else if(lbuf[0] == '+' && lbuf[1] != '/')
		{
			erasep();
			skipc = skipcnt = atoi(&lbuf[1]);
			skipmode = SKIP;
			break;
		}
		else if(lbuf[0] == '/')
		{
			char *p =  &lbuf[0];

			erasep();
			p = getpat(p);
			if(compile(p, exbuf, &exbuf[512], '\0') == (char *)-1)
				break;
			skipmode = RE;
			skipc = skipcnt = 0;
			break;
		}
		else if(lbuf[0] == quit && lbuf[1] == '\0')
		{
			erasep();
			write(tfd, "\n", 1);
			exitcode = 0;
			lbuf[0] = 0;
			break;
		}
		else if(lbuf[0] == '?' && lbuf[1] == '\0')
		{
			write(tfd, "\n\r", 2);
			sprintf(msgbuf, "\t?		this msg\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			sprintf(msgbuf, "\t+nnn		skip nnn lines down\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			sprintf(msgbuf, "\t/RE		skip to regular expr\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			sprintf(msgbuf,"\t/		skip to next occurence of prev RE\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			sprintf(msgbuf,"\tq		quit\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			sprintf(msgbuf,"\t!cmd		shell command\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			sprintf(msgbuf,"\tnnn<newline>\tchange scroll length to nnn\n\r");
			write(tfd, msgbuf, strlen(msgbuf));
			continue;
		}
		else if(lbuf[0] == '!')
		{
			write(tfd, "\n", 1);
			resetty();
			signal(SIGINT, SIG_DFL);
			close(tfd);
			system((lbuf[0]=='!' ? &lbuf[1] : lbuf));
			(void)signal(SIGINT, reset);
			if((tfd = open("/dev/tty", 2)) == -1)
			{
				exitcode = -1;
				break;
			}
			continue;
		}
		else if(lbuf[0])
		{
			erasep();
			sprintf(msgbuf, "Type ? for list of paging options");
			eraselen = strlen(msgbuf);
			write(tfd, msgbuf, eraselen);
			sleep(4);
			erasep();
		}
		else
			break;
	}
	erasep();
out:
	resetty();
	signal(SIGINT, oldsig);
	close(tfd);
	return;
}

static void
reset(){
	intr++;
	exitcode = 0;
	longjmp(jmpbuf,1);
}

static
screenlen(s)
register char *s;
{
	register n;

	n = 0;
	while(*s) switch(*s++) {
		case '\t':
			n += 8 - n%8;
			break;
		case '\b':
			n--;
			break;
		default:
			n++;
		}
	return(n);
}

extern int fork(), execl(), wait(), (*signal())();


static
erasep()		/* erase prompt */
{
	register int i;

	write(tfd, "\r", 1);
	for(i = 0; i < eraselen; i++)
		write(tfd, " ", 1);
	write(tfd, "\r", 1);
}


/*
 *	strsrch -- strsrch() is a sub which calls the regexp(7)
 *	software to do regular expression matching. It has its own
 *	static buffer for saving compiled expressions which is inaccessible
 *	to the user.
 */


static
strsrch(pat, buf, len, fp)
char *pat;
char *buf;
FILE *fp;
{
	register int i = 0;

	if(compile(pat, exbuf, &exbuf[512], '\0') == (char *)-1)
		return(0);
	while(fgets(buf, len, fp) != NULL)
	{
		i++;
		if(step(buf, exbuf))
			return(i);
	}
	buf[0] = 0;
	return(-1);
}

static char *
getpat(str)
register char *str;
{
	static char buf[128];
	register char *p = &buf[0];
	register int len;

	if(*str == '/')
		str++;
	if(!*str)
		return(buf);
	if((len=strlen(str)) > sizeof(buf))
	{
		fprintf(stderr, "Pattern too long\n");
		buf[0] = 0;
		return(buf);
	}
	strcat(buf, str);
	str = &buf[len-1];
	if(*--str == '/')
		*str = 0;
	return(buf);
}


static
pginit()
{
	plen = strlen(prompt);
	bbuf = malloc((totmem=(cpl+1)*(24)+20));
	if(bbuf == NULL) {
		fputs("pg: malloc error\n", stderr);
		exit(2);
	}
	*bbuf = '\0';
}

pgflsh(flg)
{
	/*
	 *	Initialized yet?
	 */
	if(bbuf)
	{
		if(flg && curmem)
			if(write(1, bbuf, curmem) != curmem)
				exitcode = -1;
		*bbuf = '\0';
		curmem = 0;
	}
}

static
skipline(buf)
register char *buf;
{
	if(skipmode == SKIP)
	{
		if(skipcnt-- == 0)
		{
			sprintf(msgbuf,skipmsg, skipc);
			apnd(msgbuf);
			bfrlns = 3;
			skipmode = 0;
			return(0);
		}
	}
	else if(skipmode == RE)
	{
		if(step(buf, exbuf))
		{
			sprintf(msgbuf,skipmsg, skipcnt);
			apnd(msgbuf);
			bfrlns = 3;
			skipmode = 0;
			return(0);
		}
		skipcnt++;
	}
	return(1);
}
static
rawmode()
{
	tb.c_lflag &= ~(ECHONL|ECHO|ICANON);
	tb.c_cc[VMIN] = 1;
	tb.c_cc[VTIME] = 0;
	ioctl(tfd, TCSETA, &tb);
}
static
resetty()
{
	ioctl(tfd, TCSETA, &stb);
}
static
getty()
{
	ioctl(tfd, TCGETA, &tb);
	stb = tb;
}
/*--------------------------END OF pgputs() MODULE---------------------*/
/*
	Routine to expand filename that contains shell variables

	Uses popen to fork/exec an echo to expand name if it contains shell
	metacharacters ($*{}[]?).
	Returns name if name does not contain "expandable" characters.
	Returns NULL if can't do echo or expanded name is not unique,
	and the modifier indicates this check should be done.
	Otherwise returns expanded name.
*/

char *
expand(name, modifier)
char name[];
int	modifier;
{
	FILE *popen(), *fd;
	int pclose();
	char *cp;
	char command[512];	/* build command here */

	if (!strpbrk(name, "$*{}[]\\?"))    /* return if no shell type chars */
		return(name);

	sprintf(command, "echo %s\n", name);

	if ((fd = popen(command, "r")) == NULL) {
		fprintf(stderr, "Can't echo\n");
		return(0);
	}

	fgets(command, sizeof(command), fd);	/* get expanded name */
	pclose(fd);

	if ((cp = strchr(command, '\n')) != NULL)
		*cp = '\0';

	if (modifier & EX_UNIQ) {
		if (strchr(command, ' ')) {
			fprintf(stderr, "Ambiguous name\n");
			return(0);
		}
	}
	return(strcpy(name, command));
}


/*
	Execute the given command (com) with the indicated letter (let)
	passed as standard input to the command.
 */
pipelet(let, com)
int	let;
char	*com;
{
	static	char	lastcom[128];
	FILE	*popen(), *pp;
	char	*cp;

	/* zap newline in command line */
	for (cp = com; *cp; ++cp) {
		if (*cp == '\n') {
			*cp = '\0';
			break;
		}
	}
	if (com[0] == '|' && com[1] == '\0') {
		cp = lastcom;
		printf("%s\n", cp);
		fflush(stdout);
	} else {
		strcpy(lastcom, com);
		cp = com;
	}

	if ((pp = popen(cp, "w")) != NULL) {
		copylet(let, pp, ORDINARY, "");
		pclose(pp);
	} else {
		fprintf(stderr, "Cannot fork\n");
	}
}


/*
	initial match
	if `prefix' is a prefix of `string' return 1
	else return 0
*/

imatch(prefix,string)
register char *prefix, *string;
{
	while (*prefix++ == *string++)
		if (*prefix == 0)
			return(1);
	return(0);
}
