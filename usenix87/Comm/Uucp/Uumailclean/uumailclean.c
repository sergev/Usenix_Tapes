#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BSD4_2

#ifdef	BSD4_2
#include <sys/dir.h>
#define HASSUBDIRS
#endif	BSD4_2

#ifdef	NDIR
#include <ndir.h>
#endif	NDIR

long Now;

/* Various UUCP specific defines */
#define CMDPRE	'C'
#define MAXBASENAME	14
#define SYSNSIZE	(MAXBASENAME-1-1-1-4)	/* system name length */
#define FILENAMELEN	(MAXBASENAME + 1)	/*  uucp filename length */
#define DEBUG(level, format, param) if (Debug >= level) fprintf(stderr, format, param); else

#ifndef SENDMAIL
#define SENDMAIL   "/usr/lib/sendmail"
#endif /* !SENDMAIL */
#define BLKSIZE    ((BUFSIZ/FILENAMELEN) - 1)


#ifdef HASSUBDIRS
  /* Hacks to deal with 4.2 BSD style subfiles */
  char CSubfilebuf[FILENAMELEN+5] = "C./";
  char XSubfilebuf[FILENAMELEN+5] = "X./";
# define Csubfile(x)	(strcpy(&CSubfilebuf[3], x)-3)
# define Xsubfile(x)	(strcpy(&XSubfilebuf[3], x)-3)
  char *Dsubfile();
#else /* !HASSUBDIRS */
# define Csubfile(x)	x
# define Dsubfile(x)	x
# define Xsubfile(x)	x
#endif /* !HASSUBDIRS */

char *index();
char *strcpy();

struct flist {
	char fname[BLKSIZE][FILENAMELEN];
	int nused;
	struct flist *next;
};

struct flist warnlist;
FILE *warnfp;

char *Spool = "/usr/spool/uucp";
char myuucpname[FILENAMELEN];

/*
 * uumailclean  -  this program searches through the uucp spool directory
 * looking for mail files.  Files which have been around for longer than
 * "failtime" hours will be returned to the sender.  If a file has been
 * around longer than "warntime" hours, then a warning message is sent (once)
 * to the sender. 
 *
 * Originally Written by Jim Crammond	<jim@cs.hw.ac.uk>	3/86 
 * Hacked to death by Rick Adams <rick@seismo.css.gov> 5/87
 *
 * This program contains no ATT code and may be freely used on sites without
 * a source license.
 */

#define WARNFILE "warnlist.mail"

int	warntime = 24 * 3,	/* default hours before sending a warning */
	failtime = 24 * 14;	/* default hours before returning the mail */

int Debug = 0;
int nflag = 0;

main(argc, argv)
char **argv;
{
	int c;
	extern int optind;
	extern char *optarg;

	while ((c = getopt(argc, argv, "S:f:w:x:h:n")) != EOF)
		switch (c) {
		case 'S':
			Spool = optarg;
			break;
		case 'f':
			failtime = atoi(optarg);
			break;
		case 'w':
			warntime = atoi(optarg);
			break;
		case 'x':
			Debug = atoi(optarg);
			break;
		case 'h':
			strncpy(myuucpname, optarg, FILENAMELEN);
			break;
		case 'n':
			nflag++;
			break;
		}

	if (myuucpname[0] == '\0') {
		register char *p;
		gethostname(myuucpname, FILENAMELEN);
		p = index(myuucpname, '.');
		if (p)
			*p = '\0';
	}

	if (chdir(Spool) < 0) {
		perror(Spool);
		exit(1);
	}

	init_warnedlist(WARNFILE);

#ifdef HASSUBDIRS
	checkfiles("C.");
#else /* !HASSUBDIRS */
	checkfiles(".");
#endif /* !HASSUBDIRS */

	exit(0);
}


/*
 * checkfiles  -  scan a directory looking for "old" control files. For each
 * one found, call fail or warn as appropriate. 
 */
checkfiles(dir)
char *dir;
{
	register DIR *dirp;
	register struct direct *dentp;
	struct stat stbuf;
	int hours;

	(void) time(&Now);

	DEBUG(5, "checkfiles(%s)\n", dir);
	if ((dirp = opendir(dir)) == NULL) {
		perror("directory unreadable\n");
		return;
	}

	while ((dentp = readdir(dirp)) != NULL) {
		if (dentp->d_name[0] != CMDPRE)
			continue;

		DEBUG(9, "stat %s\n", dentp->d_name);
		if (stat(Csubfile(dentp->d_name), &stbuf) == -1) {
			if (Debug >= 4) {
				fprintf(stderr, "stat failed: ");
				perror(Csubfile(dentp->d_name));
			}
			continue;
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
			continue;

		hours = (int) (Now - stbuf.st_mtime) / 3600;
		DEBUG(9, "%d hours old\n", hours);
		if (hours >= failtime)
			fail(dentp->d_name, hours);
		else if (hours >= warntime)
			warn(dentp->d_name, hours);
	}
}


/*
 * fail  -  send a failure message to the sender and delete the mail. 
 */
fail(cmdfile, hours)
char *cmdfile;
int hours;
{
	char dfile[FILENAMELEN], xfile[FILENAMELEN];
	char host[FILENAMELEN];
	register char *from, **to;
	char *sender(), **recipients();
	FILE *out, *popen();
	char cmd[BUFSIZ];


	DEBUG(4, "fail called on %s\n", cmdfile);
	getfnames(cmdfile, dfile, xfile);

	if ((to = recipients(xfile)) == NULL)
		return;
	if ((from = sender(dfile)) == NULL)
		return;
	strcpy(host, &cmdfile[2]);
	host[strlen(cmdfile) - SYSNSIZE] = '\0';

	sprintf(cmd, "%s -t", SENDMAIL);
	if (nflag) {
		out = stdout;
		fflush(stderr);
		printf("\nFile: %s\n", cmdfile);
	} else
		out = popen(cmd, "w");
	fprintf(out,
		"From: MAILER-DAEMON\nSubject: Failed Mail\nTo: %s\n\n",
		from);
	fprintf(out,
	"After %d days (%d hours), your message to the following people:\n\n",
		hours / 24, hours);

	/* put out recipents */
	while (*to) {
		fprintf(out, "\t%s  (host=%s)\n", *to, host);
		to++;
	}

	fprintf(out, "\ncould not be delivered.\n\n");
	fprintf(out, "   ----- Unsent message follows -----   \n");

	/* print all of the message */
	print_message(dfile, out, 0);

	if (nflag)
		return;
	pclose(out);

	(void) unlink(Csubfile(cmdfile));
	(void) unlink(Dsubfile(dfile));
	(void) unlink(Dsubfile(xfile));
}

/*
 * warn  -  send a warning message to the sender and add the control file to
 * the list of files for which warnings have been sent. 
 */
warn(cmdfile, hours)
char *cmdfile;
int hours;
{
	char dfile[FILENAMELEN], xfile[FILENAMELEN];
	char host[FILENAMELEN];
	char *from, **to;
	char *sender(), **recipients();
	FILE *out, *popen();
	char cmd[50];

	if (in_warnedlist(cmdfile))
		return;

	DEBUG(4, "warn called on %s\n", cmdfile);
	getfnames(cmdfile, dfile, xfile);

	if ((to = recipients(xfile)) == NULL)
		return;
	if ((from = sender(dfile)) == NULL)
		return;
	strcpy(host, &cmdfile[2]);
	host[strlen(cmdfile) - SYSNSIZE] = '\0';

	sprintf(cmd, "%s -t", SENDMAIL);
	if (nflag) {
		out = stdout;
		fflush(stderr);
		printf("\nFile: %s\n", cmdfile);
	} else
		out = popen(cmd, "w");
 
	fprintf(out, "From: MAILER-DAEMON\nSubject: Waiting Mail\nTo: %s\n\n",
		from);
	fprintf(out,
	"After %d days (%d hours), your message to the following people:\n\n",
		hours / 24, hours);

	/* put out recipents */
	while (*to) {
		fprintf(out, "\t%s  (host=%s)\n", *to, host);
		to++;
	}

	fprintf(out, "\nhas not yet been delivered. Attempts to deliver the message will\n");
	fprintf(out, "continue for %d more days. No further action is required by you.\n\n", (failtime - hours) / 24);
	fprintf(out, "   ----- Queued message begins -----   \n");

	/* print a summary of the message */
	print_message(dfile, out, 1);
	if (nflag)
		return;

	pclose(out);

	fprintf(warnfp, "%s\n", cmdfile);
}

/*
 * getfnames  -  read the control file to find the data and execute files
 * which contain the message and list of recipients. dfile is set to the
 * datafile, xfile to the execute file. 
 */
getfnames(cmdfile, dfile, xfile)
char *cmdfile;
char *dfile;
char *xfile;
{
	register FILE *fp;
	char buf[BUFSIZ];

	if ((fp = fopen(Csubfile(cmdfile), "r")) == NULL) {
		perror(Csubfile(cmdfile));
		return;
	}
	if (fgets(buf, BUFSIZ, fp) == NULL ||
		sscanf(buf, "%*s %s\n", dfile) < 1) {
		perror("fgets dfile");
		fclose(fp);
		return;
	}
	if (fgets(buf, BUFSIZ, fp) == NULL ||
		sscanf(buf, "%*s %s\n", xfile) < 1) {
		perror("fgets xfile");
		fclose(fp);
		return;
	}

	fclose(fp);
}

/*
 * recipients -  returns a list of recipients that the mail was intended for,
 * or NULL if the execute file is not a mail file. 
 */
char **
recipients(xfile)
char *xfile;
{
	static char rbuf[BUFSIZ];
	static char *tobuf[1000];	/* see uuxqt */
	register FILE *fp;
	register char *p, **t;

	DEBUG(7, "recipients(%s)\n", xfile);
	if ((fp = fopen(Dsubfile(xfile), "r")) == NULL) {
		perror(Dsubfile(xfile));
		return NULL;
	}

	while (fgets(rbuf, BUFSIZ, fp) != NULL) {
		if (rbuf[0] == 'C' && strncmp(rbuf, "C rmail ", 8) == 0) {

			/* turn into an array of addresses */
			for (p = &rbuf[8], t = tobuf; *p;) {
				while (*p == ' ' || *p == '\n')
					*p++ = '\0';
				*t = p;
				while (*p && *p != ' ' && *p != '\n')
						p++;
					if (*t != p)
						t++;
			}
			*t = NULL;
			fclose(fp);
			return tobuf;
		}
	}

	fclose(fp);
	return NULL;
}

/*
 * sender  -  returns the sender address from the uucp from line, or NULL if
 * not found. 
 */
char *
sender(dfile)
char *dfile;
{
	static char senderof[BUFSIZ];
	char buf[BUFSIZ];
	register FILE *fp;

	DEBUG(7, "sender(%s)\n", dfile);
	if ((fp = fopen(Dsubfile(dfile), "r")) == NULL) {
		perror(Dsubfile(dfile));
		return NULL;
	}

	if (fgets(buf, BUFSIZ, fp) == NULL)
		return NULL;
	fclose(fp);

	if (sscanf(buf, "From %s", senderof) == 1) {
		register int n = strlen(senderof);
		/* Try and guess if it's a mailing list and don't return it */
		if (n < 7 || strcmp(&senderof[n-7], "request") != 0) 
			return senderof;
		DEBUG(7, "Not warning mailing list %s\n", senderof);
	}
	return NULL;
}

/*
 * print_message  -  print the message in "dfile" on the stream "outp". If
 * the edited flag is set, then only print some interesting headers and the
 * first few lines of the body. 
 */
print_message(dfile, outp, edited)
char *dfile;
register FILE *outp;
int edited;
{
	register FILE *dfp;
	char buf[BUFSIZ];
	int iflg, linecount;

	if ((dfp = fopen(Dsubfile(dfile), "r")) == NULL)
		return;

	/* skip unix from line */
	fgets(buf, BUFSIZ, dfp);

	/* print header */
	iflg = 0;
	while (fgets(buf, BUFSIZ, dfp) != NULL && buf[0] != '\n') {
		if (edited) {
			if (buf[0] == '\t' || buf[0] == ' ') {
				if (iflg)
					fputs(buf, outp);
				continue;
			}

			if (!interested(buf)) {
				iflg = 0;
				continue;
			}
			iflg = 1;
		}
		fputs(buf, outp);
	}
	putc('\n', outp);

	/* print body */
	linecount = 0;
	while (fgets(buf, BUFSIZ, dfp) != NULL) {
		if (edited && ++linecount > 5) {
			fprintf(outp, ".....\n");
			break;
		}
		fputs(buf, outp);
	}
	fclose(dfp);
}

static char *headers[] = {
	"From:",
	"Date:",
	"To:",
	"Cc:",
	"Subject:",
	0};

/*
 * interested  -  determine whether "hdr" is considered interesting and thus
 * should be printed in edited mode. 
 */
interested(hdr)
char *hdr;
{
	register char **hp = headers;

	while (*hp) {
		if (strncmp(hdr, *hp, strlen(*hp)) == 0)
			return 1;
		hp++;
	}
	return 0;
}

/*
 * Initialise list of files for which warning messages have already been
 * sent. This involves reading the warnfile into a table, removing files
 * which no longer exist (i.e. been sent or deleted), and writing this out
 * again. 
 */
init_warnedlist(warnfile)
char *warnfile;
{
	register struct flist *wp;
	register char *p;
	register int i;
	char warned[FILENAMELEN];

	wp = &warnlist;
	wp->next = NULL;
	wp->nused = 0;

	if ((warnfp = fopen(warnfile, "r")) != NULL) {
		while (fgets(warned, FILENAMELEN, warnfp) != NULL) {
			if ((p = index(warned, '\n')) != NULL)
				*p = '\0';

			if (access(Csubfile(warned), 0) == 0) {
				if (wp->nused >= BLKSIZE) {
					wp->next = (struct flist *) malloc(sizeof(warnlist));
					wp = wp->next;
					wp->next = (struct flist *) NULL;
					wp->nused = 0;
				}
				strcpy(wp->fname[wp->nused], warned);
				wp->nused++;
			}
		}
		fclose(warnfp);
	}
	/*
	 * Rewrite warnedlist removing files that no longer exist. Could be
	 * really paranoid here and create a temporary file first, rather
	 * than overwrite; in case of crashed 
	 */
	if ((warnfp = fopen(warnfile, "w")) != NULL) {
		wp = &warnlist;
		while (wp) {
			for (i = 0; i < wp->nused; i++)
				fprintf(warnfp, "%s\n", wp->fname[i]);
			wp = wp->next;
		}
		fflush(warnfp);
	}
}

/*
 * Determine whether the given filename is in the warn list. Returns 1 if
 * found, 0 otherwise. 
 */
in_warnedlist(file)
register char *file;
{
	register struct flist *wp = &warnlist;
	register int i;

	while (wp) {
		for (i = 0; i < wp->nused; i++) {
			if (strcmp(file, wp->fname[i]) == 0)
				return 1;
		}
		wp = wp->next;
	}
	return 0;
}

char DLocalX[FILENAMELEN], DLocal[FILENAMELEN];

static char *dprefix[] = {
	DLocalX,	/* Outbound 'xqt' request files */
	DLocal,		/* Outbound data files */
	"D.",		/* Other "D." files (remember the "."!) */
	"C.",		/* "C." subdirectory */
	"X.",		/* "X." subdirectory */
	"TM.",		/* Temporaries for inbound files */
	0
};

/*
 * return (possibly) remapped string s
 */
char *
Dsubfile(as)
char *as;
{
	static	char fn1[FILENAMELEN*2], fn2[FILENAMELEN*2];
	register char *s, **p;
	register int n;
	static char *tptr = NULL;

	if (DLocalX[0] == '\0') {
		sprintf(DLocalX, "D.%sX", myuucpname);
		sprintf(DLocal, "D.%s", myuucpname);
	}

	/* Alternate buffers so "link(subfile(a), subfile(b))" works */
	if (tptr != fn1)
		tptr = fn1;
	else
		tptr = fn2;

	s = as;
	tptr[0] = '\0';

	/* look for first prefix which matches, and make subdirectory */
	for (p = &dprefix[0]; *p; p++) {
		if (strncmp(s, *p, n = strlen(*p))==0 && s[n] && s[n] != '/') {
			sprintf(tptr, "%s/%s", *p, s);
			return tptr;
		}
	}
	return as;
}
