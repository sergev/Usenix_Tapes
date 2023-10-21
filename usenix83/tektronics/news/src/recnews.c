/*
 * recnews [to newsgroup] [from user]
 *
 * Process a news article which has been mailed to some group like msgs.
 * Such articles are in normal mail format and have never seen the insides
 * of netnews.  If the "to newsgroup" is included, the article is posted
 * to this newsgroup instead of trying to intuit it from the headers.
 * If the "from user" is included, the return address is forged to look
 * like that user instead of what getuid or a from line says.
 *
 * It is recommended that you always include the to newsgroup, since the
 * intution code is flakey and out of date.  The from user is probably
 * appropriate for arpanet mailing lists being funnelled at ucbvax but
 * not otherwise.  Sample lines in /usr/lib/aliases (if you run delivermail):
 *	worldnews: "|/usr/lib/news/recnews net.general"
 *		Allows you to mail to worldnews rather than using inews.
 *		Intended for humans to mail to.
 *	post-unix-wizards: "|/usr/lib/news/recnews fa.unix-wizards unix-wizards"
 *		Causes mail to post-unix-wizards to be fed into fa.unix-wizards
 *		and the return address forged as unix-wizards on the local
 *		machine.  post-unix-wizards (on the local machine) should
 *		be part of the master mailing list somewhere (on a different
 *		machine.)
 *
 * Recnews is primarily useful in remote places on the usenet which collect
 * mail from mailing lists and funnel them into the network.  It is also
 * useful if you like to send mail to some user instead of invoking
 * inews -t .. -n .. when you want to submit an article.  (Many mailers give
 * you nice facilities like editing the message.)  It is not, however,
 * essential to use recnews to be able to join usenet.
 *
 * WARNING: recnews disables the "recording" check - it has to because
 * by the time inews is run, it's in the background and too late to
 * ask permission.  If you depend heavily on recordings you probably
 * should now allow recnews (and thus the mail interface) to be used.
 */

static char *SccsId = "@(#)recnews.c	2.4	3/3/83";

#include "defs.h"

#include <stdio.h>
#include <ctype.h>

/*
 * Note: we assume there are 2 kinds of hosts using recnews:
 * Those that have delivermail (and hence this program will never
 * have to deal with more than one message at a time) and those on the arpanet
 * that do not (and hence all messages end with a sentenel).  It is
 * supposed that regular v7 type systems without delivermail or some
 * other automatic forwarding device will just use rnews.  We do
 * not attempt to tell where a message ends on all systems due to the
 * different conventions in effect.  (This COULD be fixed, I suppose.)
 */

/*
 * Kinds of lines in a message.
 */
#define FROM	001		/* From line */
#define SUBJ	002		/* Subject */
#define TO	003		/* To (newgroup based on this) */
#define BLANK	004		/* blank line */
#define EOM	005		/* End of message (4 ctrl A's) */
#define HEADER	006		/* any unrecognized header */
#define TEXT	007		/* anything unrecognized */

/*
 * Possible states program can be in.
 */
#define SKIPPING	0100	/* In header of message */
#define READING		0200	/* In body of message */

#define BFSZ 250

#define EOT	'\004'

char	from[BFSZ];		/* mailing address for replies */
char	sender[BFSZ];		/* mailing address of author, if different */
char	to[BFSZ];		/* Destination of mail (msgs, etc) */
char	subject[BFSZ];		/* subject of message */
char	newsgroup[BFSZ];	/* newsgroups of message */
char	cmdbuf[BFSZ];		/* command to popen */

extern	char	*strcat(), *strcpy();
extern	FILE	*popen();
char	*any();

main(argc, argv)
int argc;
char **argv;
{
	char buf[BFSZ];
	register char *p;
	register FILE *pipe;
	register int state, pathcnt;
	int t;

	if (argc > 1)
		strcpy(to, argv[1]);
	if (argc > 2)
		strcpy(from, argv[2]);
#ifdef debug
	printf("argv[0] is <%s>, argv[1] is <%s>, argv[2] is <%s>\n",
		argv[0], argv[1], argv[2]);
#endif
	state = SKIPPING;
	while (fgets(buf, BFSZ, stdin) != NULL) {
		t = type(buf) | state;
#ifdef debug
		printf("%o\t%s", t, buf);
#endif
		switch (t) {

		case FROM | SKIPPING:
			frombreak(buf, from);
			break;

		case FROM | READING:
			fputs(buf, pipe);
			break;

		case SUBJ | SKIPPING:
			p = any(buf, " \t");
			if (p == NULL)
				p = buf+8;
			strcpy(subject, p+1);
			subject[strlen(subject)-1] = 0;	/* trim trailing \n */
			break;

		case SUBJ | READING:
			fputs(buf, pipe);
			break;

		case TO | SKIPPING:
			if (to[0])
				break;		/* already have one */
			p = any(buf, " \t");
			if (p == NULL)
				p = buf;
			strcpy(to, p+1);	/* strip leading blank */
			to[strlen(to)-1] = 0;	/* strip trailing \n */
			break;
			
		case TO | READING:
			fputs(buf, pipe);
			break;

		/*
		 * Kludge to compensate for messages without real headers
		 */
		case HEADER | SKIPPING:
			break;

		case HEADER | READING:
			fputs(buf, pipe);
			break;

		case BLANK | SKIPPING:
			state = READING;
			findgroup(to, newsgroup);
			sprintf(cmdbuf, "%s -D -t \"%s\" -n %s -f %s", INEWS, subject, newsgroup, from);
#ifdef debug
			pipe = stdout;
			printf("BLANK: %s\n", cmdbuf);
#else
			pipe = popen(cmdbuf, "w");
			if (pipe == NULL) {
				perror("recnews: popen failed");
				exit(1);
			}
#endif
			if (sender[0])
				fputs(sender, pipe);
			break;

		case BLANK | READING:
			fputs(buf, pipe);
			break;

		case TEXT | SKIPPING:
			findgroup(to, newsgroup);
			state = READING;
			if (subject[0] == 0) {
				strcpy(subject, buf);
				if (subject[strlen(subject)-1] == '\n')
					subject[strlen(subject)-1] = '\0';
			}
			sprintf(cmdbuf, "%s -D -t \"%s\" -n %s -f %s", INEWS, subject, newsgroup, from);
#ifdef debug
			pipe = stdout;
			printf("TEXT: %s\n", cmdbuf);
#else
			pipe = popen(cmdbuf, "w");
			if (pipe == NULL) {
				fprintf(stderr, "pipe failed\n");
				exit(1);
			}
#endif
			if (sender[0])
				fputs(sender, pipe);
			break;

		case TEXT | READING:
			fputs(buf, pipe);
			break;
		}
	}
	exit(0);
}

#define itsa(kind) {lastcomma=thiscomma; lasthdr=(kind!=TEXT); return kind;}
type(p)
register char *p;
{
	char *firstbl;
	static char lasthdr=0;		/* prev line was a header */
	static char lastcomma=0;	/* prev line ended in a comma */
	int thiscomma;

	thiscomma = (p[strlen(p)-2] == ',');
	if (*p == ' ' && lasthdr && lastcomma)
		itsa(HEADER);		/* continuation line */
	firstbl = any(p, " \t");
	while (*p == ' ' || *p == '?' || *p == '\t')
		++p;

	if (*p == '\n' || *p == 0)
		itsa(BLANK);
	if (strncmp(p, ">From", 5) == 0 ||
	    strncmp(p, "From", 4) == 0)
		itsa (FROM);
	if (strncmp(p, "Subj", 4)==0 ||
	    strncmp(p, "Re:", 3)==0 ||
	    strncmp(p, "re:", 3)==0)
		itsa (SUBJ);
	if (strncmp(p, "To", 2)==0)
		itsa(TO);
	if (strncmp(p, "\1\1\1\1", 4)==0)
		itsa(EOM);
	if (firstbl && firstbl[-1] == ':' && isalpha(*p))
		itsa(HEADER);
	itsa(TEXT);
}

/*
 * Figure out who a message is from.
 */
frombreak(buf, fbuf)
register char *buf, *fbuf;
{
	register char *p;
	char wordfrom[BFSZ], uname[BFSZ], at[BFSZ], site[BFSZ];

	if (fbuf[0]) {	/* we already know who it's from */
		if (sender[0] == 0) {
#ifdef debug
			printf("sender set to: %s", buf);
#endif
			strcpy(sender, buf);
		}
		return;
	}
	/* break the line into tokens. */
	sscanf(buf, "%s %s %s %s", wordfrom, uname, at, site);
	if (isat(at))
		/*
		 * Some arpanet mail comes from "joe at mit-dms"
		 * instead of "joe@mit-dms", so handle it here.
		 */
		sprintf(fbuf, "%s@%s", uname, site);
	else
		strcpy(fbuf, uname);
}

isat(str)
char *str;
{
	if (strcmp(str, "@")==0) return TRUE;
	if (strcmp(str, "at")==0) return TRUE;
	if (strcmp(str, "AT")==0) return TRUE;
	return FALSE;
}

findgroup(to, group)
char *to;
char *group;
{
#ifdef debug
	printf("findgroup(%s)\n", to);
#endif
#ifdef fussy
	/*
	 * Default unknown "to" fields to "general".  This gives you
	 * tight control over which newsgroups exist.
	 */
	if (strcmp(to, "msgs")==0)
		strcpy(group, "msgs");
	else if (strcmp(to, "allmsgs")==0)
		strcpy(group, "NET.allmsgs");
	else if (strcmp(to, "csmsgs")==0)
		strcpy(group, "NET.csmsgs");
	else
		strcpy(group, "general");
#else
	/*
	 * Allow any newsgroup.  This way you don't have to recompile
	 * recnews everytime you add a newsgroup.
	 */
	strcpy(group, to);
#endif
}

/*
 * Return the ptr in sp at which a character in sq appears;
 * NULL if not found
 *
 */

char *
any(sp, sq)
char *sp, *sq;
{
	register c1, c2;
	register char *q;

	while (c1 = *sp++) {
		q = sq;
		while (c2 = *q++)
			if (c1 == c2)
				return(--sp);
	}
	return(NULL);
}
