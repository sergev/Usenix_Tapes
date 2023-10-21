/*
 * Postnews: post a news message to Usenet.  This C version replaces a shell
 * script, and does more intelligent prompting and filtering than possible
 * in a shell script.
 */

#ifndef lint
static char	*SccsId = "@(#)postnews.c	1.16	9/18/84";
#endif !lint

/* #include "params.h" */
#include "config.h"
#include "defs.h"
#include "libextern.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>

char tempfname[50];		/* file name used for making article */
char original[BUFLEN];		/* file name of original, used in followup */
char homedir[BUFLEN];		/* HOME environment setting */
char ccname[BUFLEN];		/* file name for article copy */

/* article header information */
char subject[BUFLEN];
char distribution[BUFLEN];
char references[BUFLEN];
char newsgroups[BUFLEN];
char moderator[BUFLEN];

char *Progname = "postnews";		/* for xerror */

time_t fmodtime;
int ismod = 0;
char buf[BUFLEN];

struct distr {
	char abbr[24];
	char descr[128];
} distr[16];

extern	struct	passwd *getpwnam(), *getpwuid(), *getpwent();
FILE *ckfopen();

main(argc, argv)
char *argv[];
{
	init();

	if (argc == 2) {
		if (strncmp(SPOOL, argv[1], strlen(SPOOL)))
			xerror("Can only followup to articles in %s", SPOOL);
		followup(argv[1]);
		strcpy(original, argv[1]);
	} else
	if (askyes("Is this message in response to some other message? ","no")) {
		char ng[BUFLEN], num[BUFLEN];
		long i, j;
		register char *c;
		int fd;
		char canpost;

		getpr("In what newsgroup was the article posted? ",ng);
		if (!valid_ng(ng, &i, &j, &canpost))
			if (canpost != 'n' )
				byebye("There is no such newsgroup.");
			else
				byebye("You are not allowed to post to that group.");

		for(;;) {
			getpr("\nWhat was the article number? (type ? for help) ", num);
			if (num[0] == 0)
				continue;
			else if (num[0] == '/') {
				artlist(ng, num + 1) ;
				continue ;
			}
			else if (num[0] == '?') {
				printf("Valid article numbers are from %ld to %ld\n", j, i);
				printf("Type /string to get a list of articles containing string\nin the author or title fields.\n");
				continue;
			}
			sprintf(original, "%s/%s", SPOOL, ng);
			for (c=original+strlen(SPOOL)+1; *c ;++c)
				if (*c == '.')
					*c = '/';
			strcat(original, "/");
			strcat(original, num);

			if ((fd=open(original,0)) >= 0) {
				close(fd);
				printf("\narticle %s\n", original);
				if (article_line(original, "From: ", buf))
					printf("%s\n", buf);
				if (article_line(original, "Subject: ", buf))
					printf("%s\n", buf);
				if (askyes("Is this the one you want? ", ""))
					break;
			} else
				printf("I can't find that article.\n");
		}

		followup(original);
	} else {
		do {
			getpr("Subject: ", subject);
		} while (*subject == '\0');

		while (!get_newsgroup())
			;
		get_distribution();
	}

	if (pre_checks())
		exit(1);
	edit_article();
	post_checks();

	post_article();
}

/*
 * Find out the topic of interest.
 */
get_newsgroup()
{
	int n;
	long i;
	char canpost;

	printf("Newsgroups (enter one at a time, end with a blank line):\n");
	printf("For a list of newsgroups, type ?\n");
	n = 0;
	newsgroups[0] = '\0';

	for(;;) {
		getpr("> ", buf);
		if (buf[0] == '\0')
			if (n == 0)
				return FALSE;
			else
				return TRUE;
		if (buf[0] == '?'){
			/* too lazy to do it myself.... */
			printf("These are the currently active groups:\n");
			sprintf(buf,"exec cat %s/newsgroups", LIB);
			system(buf);
			continue;
		}
		if (valid_ng(buf, &i, &i, &canpost)) {
			if (n++ != 0)
				strcat(newsgroups, ",");
			strcat(newsgroups, buf);
		} else {
			if (canpost == 'n')
				printf("You are not allowed to post to %s\n",
					buf);
			else
				printf("%s is not a valid newsgroup.\n", buf);
		}
	}
}

/*
 * Find out how widely the author wants the message distributed.
 */
get_distribution()
{
	register int i;
	register char *r;
	char def[BUFLEN];
	char *index();

	strcpy(def, newsgroups);
	r = index(def, '.');
	if (r) {
		*r = '\0';
		if (strcmp(def, "net") == 0)
			strcpy(def, "world");
	} else
		strcpy(def, "local");

	if (strcmp(def,"to") == 0) {
		distribution[0] = '\0';
		return;		/* He's probably testing something */
	}
	if (ngmatch("net.test", newsgroups))
		strcpy(def, "local");
	for(;;) {
		sprintf(buf, "Distribution (default='%s', '?' for help) : ", def);
		getpr(buf, distribution);
		if (distribution[0] == '\0')
			strcpy(distribution, def);

		/* Did the user ask for help? */
		if (distribution[0] == '?') {
			printf("How widely should your article be distributed?\n");
			for (i=0; distr[i].abbr[0]; i++)
				printf("%s\t%s\n", distr[i].abbr, distr[i].descr);
			continue;
		}

		/* Check that it's a proper distribution */
		for (i=0; distr[i].abbr[0]; i++) {
			if (strncmp(distr[i].abbr, distribution, sizeof(distr[0].abbr)) == 0) {
				/* Found a match. Do any special rewriting. */
				if (strcmp(distribution, "world") == 0)
					strcpy(distribution, "net");
				return;
			}
		}

		printf("Type ? for help.\n");
	}
}

/*
 * Do sanity checks before the author types in the message.
 */
pre_checks()
{
	check_mod();
	if (recording(newsgroups))
		return 1;
	return 0;
}

/*
 * Check to see if the newsgroup is moderated.
 */
check_mod()
{
	register FILE *fd;
	char ng[64], mod[BUFLEN];

	sprintf(buf, "%s/%s", LIB, "moderators");
	fd = ckfopen(buf, "r");

	while (!feof(fd)) {
		if (fgets(buf, sizeof buf, fd) == NULL) {
			fclose(fd);
			return;
		}
		twosplit(buf, ng, mod);
		if (ngmatch(newsgroups, ng)) {
			strcpy(moderator, mod);
			ismod = 1;
			return;
		}
	}
}

/*
 * Set up the temp file with headers.
 */
edit_article()
{
	FILE *tf, *of;
	char *editor;
	char *endflag = "";
	char *p;
	char *getenv();
	struct stat stbuf;

	editor = getenv("EDITOR");
	strcpy(tempfname, "/tmp/postXXXXXX");
	mktemp(tempfname);

	/* insert a header */
	tf = ckfopen(tempfname, "w");
	fprintf(tf, "Subject: %s\n", subject);
	fprintf(tf, "Newsgroups: %s\n", newsgroups);
	if (distribution[0] != '\0')
		fprintf(tf, "Distribution: %s\n", distribution);
	if (ismod)
		fprintf(tf, "To: %s\n", moderator);

	if (references[0] != '\0') {
		fprintf(tf, "References: %s\n\n", references);

		of = ckfopen(original, "r");
		while (fgets(buf, BUFSIZ, of) != NULL)
			if (buf[0] == '\n')	/* skip headers */
				break;
		while (fgets(buf, BUFSIZ, of) != NULL)
			fprintf(tf, "> %s", buf);
		fclose(of);
	}

	fprintf(tf, "\n*** REPLACE THIS LINE WITH YOUR MESSAGE ***\n");
	fflush(tf);
	fstat(fileno(tf), &stbuf);
	fmodtime = stbuf.st_mtime;
	fclose(tf);

	/* edit the file */
	if (editor == NULL)
		editor = DFTEDITOR;

	p = editor + strlen(editor) - 2;
	if (strcmp(p, "vi") == 0 && references[0] == '\0')
		endflag = "+";

	sprintf(buf, "exec %s %s %s", editor, endflag, tempfname);

	system(buf);
}

/*
 * Do sanity checks after the author has typed in the message.
 */
post_checks()
{
	char group[BUFLEN];
	char s[BUFLEN + 24];
	char *c;
	struct stat stbuf;

	if (stat(tempfname, &stbuf) < 0) {
		printf("File deleted - no message posted.\n");
		unlink(tempfname);
		exit(1);
	}

	if (stbuf.st_mtime == fmodtime || stbuf.st_size < 5) {
		printf("File not modified - no message posted.\n");
		unlink(tempfname);
		exit(1);
	}

	/* Sanity checks for certain newsgroups */
	if (ngmatch(newsgroups, "all.wanted") && ngmatch(distribution,"net,na,usa,att,btl")) {
		printf("Is your message something that might go in your local\n");
		printf("newspaper, for example a used car ad, or an apartment\n");
		printf("for rent? ");
		if (askyes("","")) {
			printf("It's pointless to distribute your article widely, since\n");
			printf("people more than a hundred miles away won't be interested.\n");
			printf("Please use a more restricted distribution.\n");
			get_distribution();
			modify_article(tempfname,"Distribution: ",distribution,"replace");
		}
	}

	if (ngmatch(newsgroups, "all.jokes")) {
		if (askyes("Could this be offensive to anyone? ","")) {
			getpr("Whom might it offend? ", group);
			sprintf(s," - offensive to %s (ROT13)",group);
			modify_article(tempfname, "Subject: ", s, "append");
			encode(tempfname);
		}
	}

	if (ngmatch(newsgroups, "net.general")) {
		c = newsgroups;
		while (*c != ',' && *c)
			++c;
		if (*c == ',') {
			printf("Everybody reads net.general, so it doesn't make sense to\n");
			printf("post to newsgroups in addition to net.general.	If your\n");
			printf("article belongs in one of these other newsgroups, then you\n");
			printf("should not post to net.general.	If it is important enough\n");
			printf("for net.general, then you shouldn't post it in other places\n");
			printf("as well.	Please reenter the newsgroups.\n");
			get_newsgroup();
			modify_article(tempfname,"Newsgroups: ",newsgroups,"replace");
		}
		else {
			printf("net.general is for important announcements.\n");
			printf("It is not for items for which you couldn't think\n");
			printf("of a better place - those belong in net.misc.\n");
			if (!askyes("Are you sure your message belongs in net.general? ","")) {
				get_newsgroup();
				modify_article(tempfname, "Newsgroups: ", newsgroups, "replace");
			}
		}
	}
}

/*
 * Save a copy of the article in the users NEWSARCHIVE directory.
 */
save_article()
{
	FILE *in, *out;
	int c;
	time_t timenow, time();
	char *today, *ctime();


	in = ckfopen(tempfname, "r");
	out = ckfopen(ccname, "a");
	timenow = time((time_t)0);
	today = ctime(&timenow);
	fprintf(out,"From postnews %s",today);
	while ((c=getc(in)) != EOF)
		putc(c, out);
	putc('\n', out);
	fclose(in);
	fclose(out);
}

/*
 * Post the article to the net.
 */
post_article()
{
	extern char MAILPARSER[];
	int status;

	printf("%s article...\n", ismod ? "Mailing" : "Posting" );
	if (ismod)
		sprintf(buf, "exec %s -t < %s", MAILPARSER, tempfname);
	else
		sprintf(buf, "exec %s -wi < %s", POSTNM, tempfname);
	status = system(buf);

	if (status) {
		printf("Article not %s - exit status %d\n", ismod ? "mailed" : "posted", status);
		savedead();
	} else {
		printf("Article %s successfully.\n", ismod ? "mailed" : "posted" );
		if (ccname[0]) {
			if (ismod)
				save_article();
			printf("A copy has been saved in %s\n", ccname);
		}
	}
	unlink(tempfname);
}

/*
 * Save an article that couldn't be posted.
 */
savedead() {
	FILE *in, *out;
	char deadfile[256];
	int c;

	in = ckfopen(tempfname, "r");
	sprintf(deadfile, "%s/dead.article", homedir);
	out = ckfopen(deadfile, "a");
	while ((c = getc(in)) != EOF)
		putc(c, out);
	putc('\n', out);
	fclose(in);
	fclose(out);
	printf("Article saved in %s\n", deadfile);
}

/*
 * Initialization.
 */
init()
{
	FILE *fd;
	register char *p;
	int i;
	char *getenv();

	references[0] = '\0';
	distribution[0] = '\0';

	p = getenv("HOME");
	if (p == NULL) {
		p = getenv("LOGDIR");
		if (p == NULL) {
			struct passwd *pw;
			pw = getpwuid(getuid());
			if (pw == NULL) {
				fprintf(stderr,"You're not in /etc/passwd\n");
				exit(1);
			}
			p = pw->pw_dir;
		}
	}
	strcpy(homedir, p);


	p = getenv("NEWSARCHIVE");
	if (p != NULL)
		strcpy(ccname, p);
/*
	else
		sprintf(ccname, "%s/author_copy", homedir);
*/

	pathinit();
	sprintf(buf, "%s/%s", LIB, "distributions");
	fd = ckfopen(buf, "r");
	for (i=0; !feof(fd); i++) {
		if (fgets(buf, sizeof buf, fd) == NULL)
			break;
		twosplit(buf, distr[i].abbr,distr[i].descr);
	}
	fclose(fd);
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

/*
 * Get a yes or no answer to a question.	A default may be used.
 */
askyes(msg, def)
char *msg, *def;
{

	printf("%s", msg);
	buf[0] = 0;
	gets(buf);
	switch(buf[0]) {
	case 'y':
	case 'Y':
		return TRUE;
	case 'n':
	case 'N':
		return FALSE;
	case '\0':
		switch(*def) {
		case 'y':
		case 'Y':
			return TRUE;
		case 'n':
		case 'N':
			return FALSE;
		}
	default:
		printf("Please answer yes or no.\n");
		return askyes(msg, def);
	}
}

/*
 * Get a character string into buf, using prompt msg.
 */
getpr(msg, bfr)
char *msg, *bfr;
{
	static int numeof = 0;
	printf("%s", msg);
	gets(bfr);
	nstrip(bfr);
	if (feof(stdin)) {
		if (numeof++ > 3) {
			fprintf(stderr,"Too many EOFs\n");
			exit(1);
		}
		clearerr(stdin);
	}
}

byebye(mesg)
char *mesg;
{
	printf("%s\n", mesg);
	exit(1);
}

/*
 * make a modification to the header of an article
 *
 *	 fname -- name of article
 *	 field -- header field to modify
 *	 line	-- modification line
 *	 how	 -- 'a' or 'A' to append line to existing header line
 *			anything else to replace existing line
 *
 * example:
 *	 modify_article("/tmp/article" , "Subject: " , "new subject" , "replace");
 *
 *
 */
modify_article(fname, field, line, how)
char *fname, *field, *line, *how;
{
	FILE *fpart, *fptmp;
	char *temp2fname = "/tmp/postXXXXXX";
	int i;

	mktemp(temp2fname);

	fptmp = ckfopen(temp2fname, "w");
	fpart = ckfopen(fname, "r");

	i = strlen(field);
	while (fgets(buf, BUFLEN, fpart) != NULL) {
		if (strncmp(field, buf, i) == 0) {
			nstrip(buf);
			if (*how=='a' || *how=='A')
				/* append to current field */
				sprintf(buf, "%s%s\n", buf, line);
			else
				/* replace current field */
				sprintf(buf, "%s%s\n", field, line);
		}
		fputs(buf, fptmp);
	}

	fclose(fpart);
	fclose(fptmp);

	fptmp = ckfopen(temp2fname, "r");
	fpart = ckfopen(fname, "w");

	i = strlen(field);
	while (fgets(buf,BUFLEN,fptmp) != NULL)
		fputs(buf, fpart);

	fclose(fpart);
	fclose(fptmp);
	unlink(temp2fname);
}


/* verify that newsgroup exists, and get number of entries */
valid_ng(ng, maxart, minart, canpost)
char *ng;
long *maxart, *minart;
char *canpost;
{
	char ng_check[BUFLEN], ng_read[BUFLEN];
	char ACTIVE[FPATHLEN];
	FILE *fp;

	*minart = 1; *canpost = 'y';
	sprintf(ACTIVE, "%s/active", LIB);
	fp = ckfopen(ACTIVE, "r");
	while (fgets(ng_read, BUFLEN, fp) != NULL) {
		sscanf(ng_read, "%s %ld %ld %c", ng_check, maxart, minart, canpost);
		if (strcmp(ng_check, ng) == 0) {
			fclose(fp);
			if (*canpost == 'y')
				return TRUE;
			else
				return FALSE;
		}
	}
	*canpost = 'i';
	*maxart = 0;
	*minart = 0;
	fclose(fp);
	return FALSE;
}

/* get the line specified by field from an article */
article_line(article, field, line)
char *article, *field, *line;
{
	FILE *fp;
	char *c;
	int i = strlen(field);

	fp = ckfopen(article,"r");
	while ((c=fgets(line,BUFLEN,fp)) != NULL && strncmp(field,line,i) != 0)
		;
	fclose(fp);
	if (c != NULL) {
		nstrip(line);
		return TRUE;
	} else {
		line[0] = '\0';
		return FALSE;
	}
}


/* get the header information for a followup article */
followup(baseart)
register char *baseart;
{
	/* subject */
	if (article_line(baseart, "Subject: ", buf)) {
		if (!prefix(buf+9, "Re:"))
			sprintf(subject, "Re: %s", buf+9);
		else
			strcpy(subject, buf+9);
	} else
		strcpy(subject, "Re: orphan response");

	/* newsgroup */
	if (article_line(baseart, "Newsgroups: ", buf))
		strcpy(newsgroups, buf+12);
	if (ngmatch(newsgroups, "net.general"))
		strcpy(newsgroups,"net.followup");

	/* distribution */
	if (article_line(baseart, "Distribution: ", buf))
		strcpy(distribution, buf+14);

	/* references */
	if (article_line(baseart, "References: ", buf)) {
		strcpy(references, buf+12);
		strcat(references, " ");
	}
	if (article_line(baseart, "Message-ID: ", buf))
		strcat(references, buf+12);
}

encode(article)
char *article;
{
	FILE *fpart, *fphead, *fpcoded;
	char *headerfile = "/tmp/pheadXXXXXX";
	char *codedfile = "/tmp/pcodeXXXXXX";

	mktemp(headerfile);
	mktemp(codedfile);

	fpart = ckfopen(article, "r");

	/* place article header in "headerfile" file */
	fphead = ckfopen(headerfile, "w");
	while (fgets(buf, BUFLEN, fpart) != NULL) {
		fputs(buf, fphead);
		if (buf[0] == '\n')
			break;
	}
	fclose(fphead);

	/* place article body in "codedfile" file */
	fpcoded = ckfopen(codedfile, "w");
	while (fgets(buf, BUFLEN, fpart) != NULL)
		fputs(buf, fpcoded);
	fclose(fpcoded);
	fclose(fpart);

	/* encode body and put back together with header */
	rename(headerfile, article);

	sprintf(buf,"exec %s/%s 13 < %s >> %s\n", LIB, "caesar", codedfile, article);
	printf("Encoding article -- please stand by\n");
	if (system(buf)) {
		printf("encoding failed");
		exit(2);
	}
	unlink(codedfile);
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

	sprintf(recbuf, "%s/%s", LIB, "recording");
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
			fprintf(stderr, "Do you understand this?  Hit <return> to proceed, <BREAK> to abort: ");
			n = read(2, recbuf, 100);
			c = recbuf[0];
			yes = (c=='y' || c=='Y' || c=='\n' || c=='\n' || c==0);
			if (n <= 0 || !yes)
				return -1;
		}
	}
	return 0;
}

xxit(i)
{
	exit(i);
}


xerror(fmt, a1, a2, a3, a4)
	char *fmt;
	{
	fprintf(stderr, fmt, a1, a2, a3, a4);
	putc('\n', stderr);
	xxit(1);
}
