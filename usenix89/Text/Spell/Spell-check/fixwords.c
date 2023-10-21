
/* RCS Info: $Revision: 1.2 $ on $Date: 86/04/02 10:33:17 $
 *           $Source: /ic4/faustus/src/spellfix/RCS/fixwords.c,v $
 * Copyright (c) 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Defined:
 */

#include "spellfix.h"
#include <curses.h>
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/ioctl.h>

/* Figure out what the word should be, and return the correction. Every time
 * we call update the screen, we check to see if the user has typed anything.
 * Since it is too easy for the output buffer to fill up before the user
 * can type anything, we temporarily reset the quit character to space and
 * trap it.
 */

static jmp_buf jbuf;

char *
fixword(buf, dict, nwords, numdicts, file)
	char *buf;
	char ***dict;
	int *nwords;
	char *file;
{
	char word[BUFSIZ], context[BUFSIZ];
	static char xbuf[BUFSIZ];
	register char *s;
	register int i, j, k;
	int nsaved = 0, upper, ss, numw;
	char *saved[NSAVE];
	int scores[NSAVE], dnum;
	char **words = NULL, *getbuf();
	static struct timeval nulltime = { 0, 0 } ;
	int readfds, writefds = 0, exceptfds = 0, c;
	FILE *fp, *popen();

	for (s = buf, i = 0; isvalid(*s); s++, i++)
		word[i] = *s;
	word[i] = '\0';
	sprintf(xbuf, "/usr/ucb/grep -w %s %s", word, file);
	if (!(fp = popen(xbuf, "r"))) {
		fprintf(stderr, "Can't run %s\n", xbuf);
		exit(1);
	}

	upper = 20 + strlen(word) * 10;
	if (!upper) {
		return (NULL);
	}

	fgets(context, BUFSIZ, fp);
	pclose(fp);
	for (s = context; *s && (*s != '\n'); s++)
		;
	*s = '\0';
	s[79] = '\0';

	for (i = 0, j = 0; context[i] && word[j]; i++)
		if (context[i] == word[j])
			j++;
		else
			j = 0;
	if (word[j]) {
		fprintf(stderr, "Help, can't find it!\n");
		return (word);
	}
	i -= strlen(word);
	for (k = 0, j = 0; k < i; k++)
		if (context[k] == '\t')
			j = (j | 07) + 1;
		else
			j++;

	/* Print the header... */
	clear();
	sprintf(xbuf, "Misspelled word: %s.    Context:", word);
	mvaddstr(0, 0, xbuf);
	mvaddstr(2, 0, context);
	for (i = 0; i < j; i++)
		xbuf[i] = ' ';
	for (k = strlen(word); k > 0; k--)
		xbuf[i++] = '-';
	xbuf[i] = '\0';
	mvaddstr(3, 0, xbuf);
	mvaddstr(22, 0, "Hit space to stop...  ");
	clrtoeol();
	refresh();
	siginit();

	if (setjmp(jbuf)) {
		refresh();
		mvcur(COLS - 1, LINES - 1, 0, 0);
		mvcur(0, 0, 22, 0);
		goto getcom;
	}
	for (dnum = 0; dnum < numdicts; dnum++) {
		words = dict[dnum];
		numw = nwords[dnum];

		for (i = 0; i < numw; i++) {
			ss = compare(word, words[i], upper);
			if (ss == NOCHANCE)
				continue;

			/* Stick this word in its proper place. */
			for (j = 0; j < nsaved; j++)
				if (ss < scores[j])
					break;

			for (k = 0; k < nsaved; k++)
				if (!strcmp(saved[k], words[i]))
					break;
			if (k != nsaved)
				continue;

			if (j == NSAVE) {
				continue;
			} else if (j == nsaved) {
				saved[j] = words[i];
				scores[j] = ss;
				if (ss < upper)
					ss = upper;
				nsaved++;
			} else {
				for (k = (nsaved < NSAVE) ? nsaved : nsaved - 1;
						k > j; k--) {
					saved[k] = saved[k - 1];
					scores[k] = scores[k - 1];
				}
				if (nsaved < NSAVE)
					nsaved++;
				saved[j] = words[i];
				scores[j] = ss;
			}
			/* Update the screen... */
			for (k = 0; k < nsaved; k++) {
				sprintf(xbuf, "%c %-32s %-5d\n", 'a' + k,
						saved[k], scores[k]);
				mvaddstr(k + 4, 8, xbuf);
			}
			mvaddstr(22, 0, "Hit space to stop...  ");
			clrtoeol();
			refresh();
		}
	}
getcom: ;
	sigend();
	move(23, 0);
	clrtoeol();
	mvaddstr(22, 0, "Command (? for help): ");
	clrtoeol();
	refresh();
	c = getch();
	switch (c) {
		case 'N':
			return (word);
		case 'Q':
			endwin();
			puts("\bBye then...\n");
			return ((char *) 1);
		case 'W':
			return (NULL);
		case 'E':
			move(22, 0);
			clrtoeol();
			mvaddstr(22, 0, "Enter word: ");
			refresh();
			strcpy(xbuf, getbuf());
			move(22, 0);
			clrtoeol();
			for (s = xbuf; isvalid(*s); s++)
				;
			*s = '\0';
			return (xbuf);
		case '?':
			mvaddstr(22, 0,
"N = ok as is, Q = quit, W = write & quit, E = enter correction,");
			mvaddstr(23, 0, 
"? = help, any other character = select word.   (hit space to continue) ");
			refresh();
			getch();
			goto getcom;
		default:
			i = c - 'a';
			if ((i < 0) || (i >= nsaved)) {
				mvaddstr(22, 0, "Command (? for help): ");
				clrtoeol();
				mvaddstr(23, 0, "No such word or function.\n");
				refresh();
				sleep(1);
				goto getcom;
			}
			return (saved[i]);
	}
}

static struct tchars tcbuf;

static
sigquit()
{
	longjmp(jbuf, 1);
	/* NOTREACHED */
}

static
sigint()
{
	ioctl(0, TIOCSETC, &tcbuf);
	fprintf(stderr, "\nQuit\n");
	endwin();
	exit(1);
}

static
siginit()
{
	char oquit;

	ioctl(0, TIOCGETC, &tcbuf);
	oquit = tcbuf.t_quitc;
	tcbuf.t_quitc = ' ';
	ioctl(0, TIOCSETC, &tcbuf);
	tcbuf.t_quitc = oquit;
	signal(SIGQUIT, sigquit);
	signal(SIGINT, sigint);
	return;
}

static
sigend()
{
	ioctl(0, TIOCSETC, &tcbuf);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	return;
}

