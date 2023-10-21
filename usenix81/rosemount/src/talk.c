/* talk - talk to other users
 * 13/May/81 by David L. Pederson
 * To use ``talk'', you will have to make a directory, TDIR, with
 * read/write permission for everyone
 */

# include <stdio.h>
# include <utmp.h>
# include <sgtty.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/dir.h>

# define err(s)	fputs(s, stderr)
# define UTMP	"/etc/utmp"
# define TDIR	"/tmp/talk"
# define SIZE	20	/* general size of many strings */
# define BUFS	200
# define HUGE	1000
# define GO	"<go ahead>\007\n"
# define BS	"\b \b"

int lockfd = -1;
FILE *talkfp = NULL;
char ftmp[SIZE];
char mmes[SIZE];
char ymes[SIZE];
struct sgttyb old_t, new_t;
struct tchars tc;
struct ident {
	char	i_nam[SIZE];	/* user name */
	char	i_tty[SIZE];	/* user tty */
} mine, your;



main(argc, argv)
char *argv[];
{
	int c;
	char *rindex(), *ttyname(), *r, junkstr[SIZE];
	extern bye_bye();

	if ((r = ttyname(0)) == NULL || (r = rindex(r, '/')) == NULL) {
		/* should only happen if input is a pipe */
		err("talk is confused\n");
		exit(1);
	}
	strcpy(mine.i_tty, r+1);
	if (getnam(mine.i_tty, mine.i_nam, HUGE) != 1) {
		/* utmp messed up or we are forked off and user logged off */
		err("talk is confused\n");
		exit(1);
	}

	switch(argc) {

	case 1:		/* answer someone else */
		if ((c = search("", your.i_tty, HUGE)) > 1) {
			namlst();
			exit(1);
		}
		else if (c < 1) {
			err("no talkers\n");
			exit(1);
		}
		else if (getnam(your.i_tty, your.i_nam, HUGE) != 1) {
			err("talk is confused\n");
			exit(1);
		}
		break;

	case 2:		/* talk name or talk tty */

		if ((c = gettty(argv[1], your.i_tty, HUGE)) > 1) {
			ttylst(argv[1]);
			exit(1);
		}
		else if (c == 1) {
			strncpy(your.i_nam, argv[1], SIZE);	/* default */
			your.i_nam[SIZE-1] = NULL;
		}
		else if (getnam(argv[1], your.i_nam, HUGE) == 1) {
			strncpy(your.i_tty, argv[1], SIZE);
			your.i_tty[SIZE-1] = NULL;
		}
		else {
			err("not on the system\n");
			exit(1);
		}
		break;

	default:
		err("usage:  talk [user or tty]\n");
		exit(1);
	}

	/* We now have valid user and tty names for the other person */

	strcpy(mmes, mine.i_nam);	strcat(mmes, " (");
	strcat(mmes, mine.i_tty);	strcat(mmes, ")");
	strcpy(ymes, your.i_nam);	strcat(ymes, " (");
	strcat(ymes, your.i_tty);	strcat(ymes, ")");
	ioctl(0, TIOCGETP, &old_t);	/* save old status */
	ioctl(0, TIOCGETC, &tc);	/* and eof char */
	new_t = old_t;
	new_t.sg_flags |= CBREAK;
	ioctl(0, TIOCSETP, &new_t);	/* half cooked mode */
	signal(SIGINT, bye_bye);
	signal(SIGQUIT, bye_bye);

	if (maketmp() >= 0) {
		if (search(your.i_tty, junkstr, HUGE) != 1)
			fprintf(stdout, "waiting for %s\n", ymes);
		while (search(your.i_tty, junkstr, HUGE) != 1)
			sleep(2);
		signal(SIGALRM, SIG_IGN);
		talk();
	}
	bye_bye();
}



ttylst(name)
char *name;
{
	int i;
	char term[SIZE];

	fprintf(stderr, "%s is on", name);
	for (i = 1; gettty(name, term, i) == i; i++) {
		if (i != 1)
			putc(',', stderr);
		fprintf(stderr, " %s", term);
	}
	putc('\n', stderr);
	return;
}



namlst()
{
	int i;
	char term[SIZE], name[SIZE];

	err("talkers on\n");
	for (i = 1; search("", term, i) == i; i++) {
		getnam(term, name, 1);
		fprintf(stderr, "%s on %s\n", name, term);
	}
	return;
}



getnam(tty, nam, num)		/* find nam given a tty */
char *tty, *nam;
{
	int utmpfd, count = 0;
	struct utmp user;

	if (tty == NULL)
		return 0;

	if ((utmpfd = open(UTMP, 0)) < 0) {
		fprintf(stderr, "can't open %s\n", UTMP);
		return 0;
	}
	while (read(utmpfd, (char *) &user, sizeof user) == sizeof user)
		if (user.ut_name[0] && ! strncmp(tty, user.ut_line, 8)) {
			strncpy(nam, user.ut_name, 8);
			nam[8] = NULL;
			if (++count >= num)
				break;
		}
	close(utmpfd);
	return count;
}



gettty(nam, tty, num)		/* find tty given a nam */
char *nam, *tty;
{
	int utmpfd, count = 0;
	struct utmp user;

	if (nam == NULL)
		return 0;

	if ((utmpfd = open(UTMP, 0)) < 0) {
		fprintf(stderr, "can't open %s\n", UTMP);
		return 0;
	}
	while (read(utmpfd, (char *) &user, sizeof user) == sizeof user)
		if (user.ut_name[0] && ! strncmp(nam, user.ut_name, 8)) {
			strncpy(tty, user.ut_line, 8);
			tty[8] = NULL;
			if (++count >= num)
				break;
		}
	close(utmpfd);
	return count;
}



bye_bye()	/* close up shop */
{
	signal(SIGALRM, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	ioctl(2, TIOCSETP, &old_t);	/* restore terminal */
	if (lockfd >= 0)
		unlink(ftmp);
	if (talkfp != NULL) {
		fprintf(talkfp, "%s is done talking\007\n", mmes);
		fclose(talkfp);
	}
	exit(0);
}



/* The interlock file name is the same as the sender's tty name.
 * The file contains the recievers tty name
 */
maketmp()
{
	int len;
	char othertty[SIZE];

	strcpy(ftmp, TDIR);
	strcat(ftmp, "/");
	strcat(ftmp, mine.i_tty);
	if ((lockfd = creat(ftmp, 0644)) < 0) {
		err("can't make interlock file\n");
		return (lockfd = -1);
	}
	len = strlen(your.i_tty);
	if (write(lockfd, your.i_tty, len) != len) {
		err("can't write to interlock file\n");
		close(lockfd);
		unlink(ftmp);
		return (lockfd = -1);
	}
	close(lockfd);
	strcpy(othertty, "/dev/");
	strcat(othertty, your.i_tty);
	if ((talkfp = fopen(othertty, "w")) == NULL) {
		fprintf(stderr, "%s won't talk\n", ymes);
		unlink(ftmp);
		return (lockfd = -1);
	}
	setbuf(talkfp, NULL);		/* unbuffered */
	fprintf(talkfp, "%s would like to talk\007\n", mmes);
	return lockfd;
}



talk() {
	register int c, colcnt;

	fprintf(talkfp, "%s is talking\n", mmes);

	colcnt = 0;
	while ((c = getchar()) != EOF && c != tc.t_eofc) {
		if (! colcnt && c == '!') {	/* good at beginning of line */
			do_sys();
			colcnt = 0;
		}
		else if (c == old_t.sg_erase) {
			fputs(BS, talkfp);
			fputs(BS, stdout);
			colcnt = (colcnt <= 0 ? 0 : colcnt-1);
		}
		else if (c == '\001') {
			fputs(GO, talkfp);
			fputs(GO, stdout);
		}
		else {
			putc(c, talkfp);
			putchar(c);
			colcnt = (c == '\n' ? 0 : colcnt+1);
		}
	}
	return;
}



search(match, tty, max)
char *match;	/* if none-null, must match this to d_name */
char *tty;	/* place to return the last tty name found */
{
	int dirfd, count = 0;
	struct direct dir;

	if ((dirfd = open(TDIR, 0)) < 0) {
		fprintf(stderr, "can't open %s\n", TDIR);
		return 0;
	}
	while (read(dirfd, (char *) &dir, sizeof dir) == sizeof dir)
		if (dir.d_ino
		&& dir.d_name[0] != '.'
		&& (*match == NULL || ! strncmp(dir.d_name, match, 8))
		&& lookup(dir.d_name)) {
			strncpy(tty, dir.d_name, 8);
			tty[8] = NULL;
			if (++count >= max)
				break;
		}
	close(dirfd);
	return count;
}



lookup(name)	/* look for my tty in file name */
char *name;
{
	int fd, len;
	char buf[SIZE], fullnam[SIZE];

	strcpy(fullnam, TDIR);
	strcat(fullnam, "/");
	strcat(fullnam, name);
	if ((fd = open(fullnam, 0)) < 0)
		return 0;
	len = read(fd, buf, SIZE-1);
	close(fd);
	if (len <= 0)
		return 0;
	buf[len] = NULL;
	if (strcmp(mine.i_tty, buf) == 0)
		return 1;
	return 0;
}



do_sys()	/* do a system command */
{
	char buf[BUFS];

	fputs("please wait\n", talkfp);
	putchar('!');
	ioctl(2, TIOCSETP, &old_t);	/* restore terminal */
	if (fgets(buf, BUFS, stdin) != NULL)
		system(buf);
	ioctl(0, TIOCSETP, &new_t);	/* half cooked mode */
	printf("!\n");
	fputs("back again\n", talkfp);
	return;
}
