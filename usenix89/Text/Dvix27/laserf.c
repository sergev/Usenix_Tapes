#ifndef lint
static char sccsid[] = "@(#)lpf.c	4.12 (Berkeley) 7/16/83";
#endif

/*
 * 	filter which reads the output of nroff and converts lines
 *	with ^H's to overwritten lines.  Thus this works like 'ul'
 *	but is much better: it can handle more than 2 overwrites
 *	and it is written with some style.
 *	modified by kls to use register references instead of arrays
 *	to try to gain a little speed.
 *
 *	as a filter for the qume, literal really means it
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#define MAXWIDTH  132
#define MAXREP    10

#define	setup	"/usr/local/lib/laser/setup"	/* file on setup commands */
#define	RESET	"\033+X\r\n" 			/* laser reset command */
#define	OFFSET	"\033o"				/* offset output */
/* nroff doesn't let you use all eight bits, so ... */
#define	SO	'\016'				/* note to | 0200 into next */
int	soflag = 0; 	/* we've got to | 0200 next character */
int	nbs = 0;	/* no. of backspaces for literal mode */

char	buf[MAXREP][MAXWIDTH];
int	maxcol[MAXREP] = {-1};
int	lineno;
int	width = 132;	/* default line length */
int	length = 66;	/* page length */
int	indent;		/* indentation length */
int	npages = 1;
int	literal;	/* print control characters */
char	*name;		/* user's login name */
char	*host;		/* user's machine name */
char	*acctfile;	/* accounting information file */

struct stat	stbuf;	/* to find other execute bit for offsetting */

main(argc, argv)
	int argc;
	char *argv[];
{
	register FILE *p = stdin, *o = stdout;
	register int i, col;
	register char *cp;
	int done, linedone, maxrep;
	char ch, *limit;

	while (--argc) {
		if (*(cp = *++argv) == '-') {
			switch (cp[1]) {
			case 'n':
				argc--;
				name = *++argv;
				break;

			case 'h':
				argc--;
				host = *++argv;
				break;

			case 'w':
				if ((i = atoi(&cp[2])) > 0 && i <= MAXWIDTH)
					width = i;
				break;

			case 'l':
				length = atoi(&cp[2]);
				break;

			case 'i':
				indent = atoi(&cp[2]);
				break;

			case 'c':	/* Print control chars */
				literal++;
				break;
			}
		} else
			acctfile = cp;
	}

	/*
	 * setup printer
	 */
	if (name && (i = open(setup, O_RDONLY)) >= 0) {
		while ((col = read(i, buf, MAXREP)) > 0)
			fwrite(buf, col, 1, o);
		(void) close (i);
	} else {
		fprintf(o, "%s", RESET);
	}

	for (cp = buf[0], limit = buf[MAXREP]; cp < limit; *cp++ = ' ');
	done = 0;

	if (!literal) while (!done) {
		col = indent;
		maxrep = -1;
		linedone = 0;
		while (!linedone) {
			switch (ch = getc(p)) {
			case EOF:
				linedone = done = 1;
				ch = '\n';
				break;

			case '\f':
				lineno = length;
			case '\n':
				if (maxrep < 0)
					maxrep = 0;
				linedone = 1;
				break;

			case '\b':
				if (--col < indent)
					col = indent;
				break;

			case '\r':
				col = indent;
				break;

			case '\t':
				col = ((col - indent) | 07) + indent + 1;
				break;

			case '\031':
				/*
				 * lpd needs to use a different filter to
				 * print data so stop what we are doing and
				 * wait for lpd to restart us.
				 */
				if ((ch = getchar()) == '\1') {
					fflush(stdout);
					kill(getpid(), SIGSTOP);
					break;
				} else {
					ungetc(ch, stdin);
					ch = '\031';
				}

			default:
				if (col >= width || !literal && ch < ' ' &&
				    ch != '\033') {
					col++;
					break;
				}
				cp = &buf[0][col];
				for (i = 0; i < MAXREP; i++) {
					if (i > maxrep)
						maxrep = i;
					if (*cp == ' ') {
						*cp = ch;
						if (col > maxcol[i])
							maxcol[i] = col;
						break;
					}
					cp += MAXWIDTH;
				}
				col++;
				break;
			}
		}

		/* print out lines */
		for (i = 0; i <= maxrep; i++) {
			for (cp = buf[i], limit = cp+maxcol[i]; cp <= limit;) {
				putc(*cp, o);
				*cp++ = ' ';
			}
			if (i < maxrep)
				putc('\r', o);
			else
				putc(ch, o);
			if (++lineno >= length) {
				npages++;
				lineno = 0;
			}
			maxcol[i] = -1;
		}
	} else while (!done) {	/* really literal */
		ch = getc(p);
		if (soflag) {
			ch |= 0200;
			soflag = 0;
		}

		switch (ch) {
		case EOF:
			done = 1;
			break;


		case '\f':
			lineno = length;
		case '\n':
			nbs = 0;
			if (++lineno >= length) {
				npages++;
				lineno = 0;
			}

		default:
			if (nbs) {
				fprintf(o, "\033rl%d ", nbs*25);
				nbs = 0;
			}
			putc(ch, o);
			break;

		case '\b':
			nbs++;
			break;

		case SO:
			soflag++;
			break;

		}
	}

	if (!lineno) { 		/* charged a sheet just for moving to tof */
		npages--;
	}
	/*
	 * reset terminal
	 */
	fprintf(o, RESET);
	/*
	 * offset output according to other execute bit on printer
	 *
	if (!fstat(fileno(o), &stbuf)) {
		if (stbuf.st_mode & 01)
			fprintf(o, OFFSET);
		stbuf.st_mode ^= 01;
		fchmod(fileno(o), stbuf.st_mode);
	}
	 */

	if (name && acctfile && access(acctfile, 02) >= 0 &&
	    freopen(acctfile, "a", stdout) != NULL) {
		printf("%7.2f\t%s:%s\n", (float)npages, host, name);
	}
	exit(0);
}
