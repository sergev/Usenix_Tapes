/*
 * This program handles replies and followups for vnews.
 * Synopsys:
 *	reply -r file		# mail a reply
 *	reply -f file		# generate a followup
 */

#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "newsdefs.h"
#include "libextern.h"
#include "arthead.h"
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>


#define NOPOST 22

int followup;
char *original;
char *references;
int vnews;
struct arthead h;
int checksum;
int nchars;
FILE *tfp;
char tempfile[16];
char tempout[16];
int outfd;

FILE *ckfopen();
char *savestr();
int nopost();



main(argc, argv)
	char **argv;
	{
	int c;
	int pid;
	extern char *optarg;

	signal(SIGQUIT, SIG_IGN);
	while ((c = getopt(argc, argv, "r:f:v")) != EOF) {
		switch (c) {
		case 'r':
			followup = 0;
			original = optarg;
			break;
		case 'f':
			followup = 1;
			original = optarg;
			break;
		case 'v':
			vnews = 1;
			break;
		}
	}
	if (original == NULL)
		xerror("-f or -r must be specified");

	signal(SIGINT, nopost);

	pathinit();
	getuser();

	writetemp(followup);

	edit();
	if (samefile())
		nopost();

	/* fork off a child and let parent terminate */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
#ifdef SIGTSTP
	signal(SIGTSTP, SIG_IGN);	/* I hope this works */
#endif
	if ((pid = fork()) == -1)
		fprintf(stderr, "postreply: can't fork\n");
	if (pid > 0)
		_exit(0);

	nice(4);
	sprintf(bfr, "%s.BAK", tempfile);
	unlink(bfr);

	postit();
	xxit(0);
}


/*
 * Print a recorded message warning the poor luser what he is doing
 * and demand that he understands it before proceeding.  Only do
 * this for newsgroups listed in LIBDIR/recording.
 */
recording(ngrps)
char *ngrps;
{
	char recbuf[BUFLEN];
	FILE *fd;
	char nglist[BUFLEN], fname[BUFLEN];
	int  c, n, yes;

	sprintf(recbuf, "%s/recording", LIB);
	fd = fopen(recbuf, "r");
	if (fd == NULL)
		return 0;
	while ((fgets(recbuf, sizeof recbuf, fd)) != NULL) {
		sscanf(recbuf, "%s %s", nglist, fname);
		if (ngmatch(ngrps, nglist)) {
			fclose(fd);
			if (fname[0] == '/')
				strcpy(recbuf, fname);
			else
				sprintf(recbuf, "%s/%s", LIB, fname);
			fd = fopen(recbuf, "r");
			if (fd == NULL)
				return 0;
			while ((c = getc(fd)) != EOF)
				putc(c, stderr);
			fclose(fd);
			fprintf(stderr, "Do you understand this?  Hit <return> to proceed, <BREAK> to abort: ");
			n = read(2, recbuf, BUFLEN);
			c = recbuf[0];
			yes = (c=='y' || c=='Y' || c=='\n' || c=='\n' || c==0);
			if (n <= 0 || !yes)
				return -1;
			else
				return 0;
		}
	}
	fclose(fd);
	return 0;
}



/*
 * Generate replies and followups.
 */


writetemp() {
	FILE *artfp;
	char subj[132];
	char *p;
	char *replyname();

	checksum = 0;
	nchars = 0;
	artfp = ckfopen(original, "r");
	if (gethead(&h, artfp) == NULL)
		xerror("Original article has garbled header");
	fclose(artfp);


	scopyn(h.h_title, subj, 128);
	if (!prefix(subj, "Re:") && !prefix(subj, "re:")) {
		strcpy(bfr, subj);
		sprintf(subj, "Re: %s", bfr);
	}

	p = h.h_nbuf;
	if (hset(h.h_followto))
		p = h.h_followto;
	strcpy(bfr, p);
	launder(bfr);
	if (followup && recording(bfr))
		nopost();

	gentemp(tempfile);
	tfp = ckfopen(tempfile, "w");

	wstr("Command: %s\n", followup? "followup" : "reply");
	wstr("Newsgroups: %s\n", bfr);
	wstr("To: %s\n", replyname(&h, bfr));
	wstr("Subject: %s\n", subj);
	if (hset(h.h_keywords))
		wstr("Keywords: %s\n", h.h_keywords);
	wstr("Distribution: %s\n", hset(h.h_distribution)? h.h_distribution : "");
	bfr[0] = 0;				/* References */
	if (hset(h.h_references)) {
		strcpy(bfr, h.h_references);
		strcat(bfr, " ");
	}
	strcat(bfr, h.h_ident);
	references = savestr(bfr);
	wstr("References: %s\n", bfr);
	wstr("\n");
	fclose(tfp);
}



wstr(fmt, a1, a2, a3, a4)
	char *fmt;
	{
	char buf[1024];
	register char *p;

	sprintf(buf, fmt, a1, a2, a3, a4);
	for (p = buf ; *p ; ) {
		checksum += *p++;
		nchars++;
	}
	fputs(buf, tfp);
}



/*
 * Check to see if the newsgroup is moderated.  If so, the moderator's
 * name is returned in moderator.
 */
#ifdef notdef
check_mod(newsgroups, moderator)
char *newsgroups;
char *moderator;
{
	register FILE *fd;
	char ng[64], mod[BUFLEN];

	sprintf(bfr, "%s/%s", LIB, "moderators");
	if ((fd = fopen(bfr, "r")) == NULL)
		return 0;
	while (1) {
		if (fgets(bfr, LBUFLEN, fd) == NULL) {
			fclose(fd);
			return 0;
		}
		twosplit(bfr, ng, mod);
		if (ngmatch(newsgroups, ng)) {
			strcpy(moderator, mod);
			fclose(fd);
			return 1;
		}
	}
}



/*
 * Split a line of the form
 *		text whitespace text
 * into two strings.	Also trim off any trailing newline.
 * This is destructive on src.
 */
twosplit(src, dest1, dest2)
char *src, *dest1, *dest2;
{
	register char *p;

	nstrip(src);
	for (p=src; isalnum(*p) || ispunct(*p); p++)
		;
	*p++ = 0;
	for ( ; isspace(*p); p++)
		;
	strcpy(dest1, src);
	strcpy(dest2, p);
}
#endif



edit() {
	char *p;
	int pid;
	int status;
	char *arg[5];
	register char **ap;
	char *getenv(), *rindex();

	if ((arg[0] = getenv("EDITOR")) == NULL)
		arg[0] = DFTEDITOR;
	if ((p = rindex(arg[0], '/')) != NULL)
		p++;
	else
		p = arg[0];
	ap = &arg[1];
	if (strcmp(p, "vi") == 0) {
		*ap++ = "+";
		*ap++ = tempfile;
		*ap++ = original;
	} else if (strcmp(p, "gmacs") == 0
		|| strcmp(p, "gemacs") == 0
		|| strcmp(p, "gem") == 0) {
		*ap++ = original;
		*ap++ = tempfile;
	} else {
		*ap++ = tempfile;
		*ap++ = original;
	}
	*ap = NULL;

	if ((pid = fork()) == -1)
		xerror("Can't fork");
	if (pid == 0) {
		signal(SIGQUIT, SIG_DFL);
		execvp(arg[0], arg);
		_exit(124);
	}
	signal(SIGINT, SIG_IGN);
	status = pwait(pid);
	if (status == 124 << 8)
		xerror("Can't exec %s", arg[0]);
}



samefile() {
	struct stat statb;
	register FILE *fp;
	register int c;

	if (stat(tempfile, &statb) < 0)
		xerror("%s has vanished!", tempfile);
	if (statb.st_size == 0L)
		return 1;
	if (statb.st_size != nchars)
		return 0;
	fp = ckfopen(tempfile, "r");
	while ((c = getc(fp)) != EOF)
		checksum -= c;
	return (checksum == 0);
}



postit() {
	int artfd;
	int i;

	gentemp(tempout);
	if ((outfd = creat(tempout, 0644)) < 0) {
		fprintf(stderr, "\7\nCan't create %s\nreply failed\n\7", tempout);
		xxit(1);
	}
	if (followup)
		writestr("Subject: followup failed\n\n", outfd);
	else
		writestr("Subject: reply failed\n\n", outfd);

	if (runpost()) {
		if ((artfd = open(tempfile, 0)) < 0)
			writestr("postreply: can't reopen article\n", outfd);
		else {
			writestr("\nYour article follows:\n", outfd);
			while ((i = read(artfd, bfr, LBUFLEN)) > 0)
				write(outfd, bfr, i);
			close(artfd);
		}
		close(outfd);
		unlink(tempfile);
		close(0);
		if (open(tempout, 0) != 0) {
			fprintf(stderr, "\7Fatal error in reply!\nCan't reopen %s\7\n", tempout);
			exit(1);
		}
#ifdef MAILER
		execl(MAILER, MAILER, username, (char *)0);
#endif
		execl("/bin/mail", "mail", username, (char *)0);
		fprintf(stderr, "\7Fatal error in reply!\nCan't exec mail program!\7\n");
		exit(1);
	}
}



runpost() {
	int pid;
	int status;
	int fildes[2];
	FILE *fp;
	int inewsfail;
	static char prog[] = POSTNM;

	if (pipe(fildes) < 0) {
		writestr("can't create pipe\n", outfd);
		return -1;
	}		
	while ((pid = fork()) == -1)
		sleep(10);
	if (pid == 0) {
#ifdef notdef
		close(0);
		if (open(tempfile, 0) != 0) {
			sprintf(bfr, "postreply: can't open %s\n", tempfile);
			writestr(bfr, outfd);
			_exit(2);
		}
#endif
		if (fildes[1] != 1) {
			close(1);
			dup(fildes[1]);
			close(fildes[1]);
		}
		close(fildes[0]);
		close(2);
		dup(1);
		execl(prog, prog, "-wiR", references, tempfile, (char *)0);
		writestr("postreply: can't exec postnm\n", 1);
		_exit(2);
	}
	close(fildes[1]);
	fp = fdopen(fildes[0], "r");
	inewsfail = 0;
	while (fgets(bfr, LBUFLEN, fp) != NULL) {
		writestr(bfr, outfd);
		if (prefix(bfr, "inews:"))
			inewsfail = 1;
	}
	status = pwait(pid);
	if (status != 0 || inewsfail) {
		if ((status & 0377) == 0)
			sprintf(bfr, "Exit status %d from postnm\n", status >> 8);
		else
			sprintf(bfr, "postreply: postnm died with signal %d\n", status & 077);
		writestr(bfr, outfd);
		return -1;
	}
	return 0;
}


#ifdef notdef
/*
 * Save a copy of the article in the users NEWSARCHIVE file.
 * The article is saved only if the user explicitly sets NEWSARCHIVE.
 * Currently, we save both news and mail replies, which is
 * rather questionable.
 */
save_article()
{
	register FILE *in, *out;
	register int c;
	time_t timenow, time();
	char *today, *ctime();
	char *ccname;

	if ((ccname = getenv("NEWSARCHIVE")) == NULL || ccname[0] == '\0')
		return 0;
	if ((in = fopen(tempfile, "r")) == NULL) {
		writestr("Can't reopen temp file for article save\n", outfd);
		return -1;
	}
	if ((out = fopen(ccname, "a")) == NULL) {
		sprintf(bfr, "Can't open %s to save article\n", ccname);
		writestr(bfr, outfd);
		return -1;
	}
	timenow = time((time_t)0);
	today = ctime(&timenow);
	fprintf(out, "From postreply %s", today);
	while ((c=getc(in)) != EOF)
		putc(c, out);
	putc('\n', out);
	fclose(in);
	fclose(out);
	return 0;
}
#endif


writestr(s, fd)
	char *s;
	int fd;
	{
	write(fd, s, strlen(s));
}


pwait(pid) {
	int status;

	while (wait(&status) != pid);
	return status;
}



gentemp(s)
	char s[16];
	{
	strcpy(s, "/tmp/repXXXXXX");
	mktemp(s);
}



xerror(fmt, a1, a2, a3, a4)
	char *fmt;
	{
	int c;

	fputs("postreply: ", stderr);
	fprintf(stderr, fmt, a1, a2, a3, a4);
	fputs("\nContinue?", stderr);
	while ((c = getchar()) != '\n' && c != EOF);
	xxit(1);
}



xxit(status) {
	if (tempfile[0])
		unlink(tempfile);
	if (tempout[0])
		unlink(tempout);
	exit(status);
}



nopost() {
	xxit(NOPOST);
}
