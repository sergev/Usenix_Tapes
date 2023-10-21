/* checkmail
 *	Mail checking program
 *
 * Checkmail (and linked names) will look for and print mail
 * or just newmail found in /usr/spool/mail/loginname.
 *
 * The messages will be printed as:
 *	Name set in From: line............Message from Subject: line
 *
 * Written by Jeff Buhrt (buhrt@ee.purdue.edu) && Curtis Smith
 */

#include <stdio.h>
#include <pwd.h>
#include <strings.h>
#include <errno.h>

typedef enum { false = 0, true = 1 } boolean;

struct mstr {
	boolean	hasread;
	char	from[64];
	char	subject[64];
};

static char	idstring[]="Checkmail/checkmail.c Jeff Buhrt/Curtis Smith 8/9/86";

char		hostname[16];
char		mailname[] = "/usr/spool/mail/\000xxxxxxxx";
char *		username;
int		uid;
int		mailcount = 0;
int		mailunread = 0;
boolean		newmailonly = false;
struct mstr	maillist[256];
struct mstr *	m = maillist;
extern char *	sys_errlist[];
extern int	errno;

getmailname()
{
	struct passwd * passent;

	passent = getpwuid(uid);
	if (passent == (struct passwd *) 0) {
		fprintf(stderr, "User-id %d is not defined on machine %s\n",
			uid, hostname);
		exit(1);
	}
	strcat(mailname, passent->pw_name);
	username = passent->pw_name;
}

init()
{
	uid = getuid();
	gethostname(hostname, sizeof(hostname));
	hostname[sizeof(hostname)-1] = '\0';
}

cutepr(head, tail, dot)
char * head, * tail, dot;
{
	int dots, headlen, spaceleft, taillen;

	headlen = strlen(head);
	taillen = strlen(tail);
	if (headlen > 38)
		headlen = 38;
	spaceleft = 79 - headlen - 2;
	if (taillen > spaceleft)
		taillen = spaceleft;
	dots = 2 + spaceleft - taillen;
	printf("%*.*s", headlen, headlen, head);
	while (dots-- > 0)
		fputc(dot, stdout);
	printf("%*.*s\n", taillen, taillen, tail);
}

main(argc,argv)
int	argc;
char	**argv;
{
	register char	*name;

	if ((name = rindex(*argv, '/')) != NULL)
		name++;
	else
		name = *argv;

	if (((strcmp(name, "newmail") == 0) || strcmp(name, "NM") == 0))
		newmailonly = true;

	init();
	getmailname();
	readmail();
	header();
}

readmail()
{
	FILE * ms;
	boolean control, hasread;
	char * ep, * sp, inputline[512];

	ms = fopen(mailname, "r");
	if (ms == (FILE *) 0) {
		if (errno == ENOENT) {
			printf("No mail for %s on %s\n", username, hostname);
			exit(0);
		}
		fprintf(stderr, "%s on %s\n", sys_errlist[errno], mailname);
		exit(1);
	}
	control = false;
	while (fgets(inputline, sizeof(inputline), ms) != (char *) 0) {
		sp = index(inputline, '\n');
		if (sp != (char *) 0)
			*sp = '\0';
		if (strncmp(inputline, "From ", 5) == 0) {
			control = true;
			hasread = false;
			m->hasread = false;
			mailcount++;
			continue;
		}
		if ((control) && (*inputline == '\0')) {
			control = false;
			m++;
			if (hasread == false)
				mailunread++;
			continue;
		}
		if ((control) && (strncmp(inputline, "Subject: ", 9) == 0)) {
			strncpy(m->subject, inputline+9, sizeof(m->subject));
			continue;
		}
		if ((control) && (strncmp(inputline, "From: ", 6) == 0)) {
			sp = index(inputline, '(');
			ep = rindex(inputline, ')');
			if ((sp == (char *) 0) || (ep == (char *) 0)) {
				strncpy(m->from, inputline+6, sizeof(m->from));
				continue;
			}
			*ep = '\0';
			strncpy(m->from, sp+1, sizeof(m->from));
		}
		if ((control) && (strncmp(inputline, "Status: ", 8) == 0)) {
			hasread = true;
			m->hasread = true;
			continue;
		}
	}
	fclose(ms);
}

header()
{
	char head[128], tail[128];
	int count;
	struct mstr * mp;

	if (mailcount == 0) {
		printf("No messages for %s on %s\n", username, hostname);
		exit(0);
	}

	sprintf(head, "Mail (%s on %s)", username, hostname);
	if (mailunread != mailcount)
		sprintf(tail, "%d Letter%s (%d unread)",
			mailcount, mailcount == 1 ? "" : "s", mailunread);
	else
		sprintf(tail, "%d Letter%s", mailcount, mailcount == 1 ?
			"" : "s");
	cutepr(head, tail, ' ');
	fputc('\n', stdout);
	count = 1;
	for (mp = maillist; mp < m; mp++)
	{
		sprintf(head, "%c %2d. %s",
			mp->hasread == false ? '*' : ' ', count++, mp->from);

		if ((newmailonly == false) || (mp->hasread == false))
			cutepr(head, mp->subject, '.');
	}
}
