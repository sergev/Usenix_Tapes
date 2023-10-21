/*
 * 92.c - postprocess nroff and col output to optimize head motion on some line
 *          printers (notably my Okidata 92).  This makes for less noisy
 *	    printing of man pages, thus the name.  The general idea is to
 *	    collect one line at a time, and if overstriking is present, to
 *	    overstrike a whole line's worth at a time, reducing the number of
 *	    times the head changes direction.  If enabled, the hardware will
 *	    be asked to do this (see below).
 *
 *	Also, capable of introducing hardware underline and bolding.
 *   
 *	Since the Okidata cannot do reverse paper motion, this program
 *	    assumes that output has been filtered by col(1), and that the
 *	    input contains no escape sequences other than ESC-9 (forward
 *	    half-line feed).  The output may contain quite a few.
 *
 *	Warning: the output of this program must be printed on the RAW
 *	    line-printer if half-line motions are used, because the cooked
 *	    printer driver mungs the codes which set line-spacing.  You can
 *	    cat to /dev/rawlp, or 'lp -o raw'
 *
 *	Warning: there is a limit of 8 overstrikes per character position.
 *
 *	The code contains flags which can be used to affect its operation:
 *		underflag: if set, hardware underlining is used; if off,
 *			it is performed as an overstrike.
 *		boldflag: if set, hardware bolding is used; if off, it is
 *			performed as an overstrike.
 *		nlflag: if set, a hardware shortcut to repeated newlines is
 *			used; if off, it is not used.
 *	If all of these are set to zero, and no half-line motions are attempted,
 *	    the filter may be used for some other kinds of printers with
 *	    different control codes.
 *
 *    Original:  by Kevin O'Gorman - kevin@kosman - May 17, 1987.
 *    Original placed in the public domain by the author, with a request that
 *	these authorship lines be left intact.
 *		 
 */

#include	<stdio.h>
#include	<assert.h>
#define	numlns	8
#define NDEBUG

extern void send();
extern void putns();
extern void putnl();

char line[numlns][BUFSIZ];	/* where I build the line and overstrikes */
int  lenln[numlns];
int  under[BUFSIZ];		/* flag for underscoring */
int  bold[BUFSIZ];		/* flag for bolding */
int  underflag = 1;		/* use hardware underline */
int  boldflag = 1;		/* use hardware bolding (standout) */
int  nlflag = 1;		/* use shortcut for repeated newlines */
int  evenflag = 0;
int  pagelen = 66 * 2;
int  linelen = 80;
int  firstflag = 1;

/* The following are named for termcap capabilities */
char *capus = "\033C";		/* Okidata specific (for now) */
char *capue = "\033D";
int  capug = 0;                /* not used: beware */
char *capso = "\033H";
char *capse = "\033I";

/* The following do not exist in termcap, but are needed to describe the 
/* Okidata:  You get half-line spacing by changing the height of a line,
     which is then handled by line-feed.  This affects the length of a page
     in inches (since page size is specified in lines), so the code has to
     keep track of where we're at.  I keep track of it in half-lines, and
     reset to normal at the end of a page.
     */
char *capHL = "\033%9\014";     /* starts half-line motions */
char *capTF = "\0335";	       /* resets top-of-form (needed after halflines) */
char *capFL = "\0336";         /* returns to 6-lines-per-inch */
char *capVM = "\033\013%.2d";    /* spaces paper by lines */

main(argc, argv)
	int argc;
	char **argv;
{
	int i;
	FILE *fd = (FILE *) NULL;

	/* scan the arguments */
	for (i=1; i<argc; i++) {
		if (argv[i][0] != '-') break;
		switch (argv[i][1]) {
		case 'w':
			sscanf(&argv[i][2],"%d",&linelen);
			break;
		case 'l':
			sscanf(&argv[i][2],"%d",&pagelen);
			pagelen *= 2;    /* we count in half-lines */
			break;
		case 'e':
			evenflag = 1;
			break;
		default:
			printf(
		"Usage: %s [-e] [-lPagelen] [-wLinelen] [file ...]\n",argv[0]);
			exit(1);
		}
	}


	/* now look for a file argument */
	/* if there is none at all, use the standard input */
	if (argc == i) {
		send(stdin);
	} else {
		for ( ; i < argc; i++) {
			if (fd == (FILE *) NULL) fd=fopen(argv[i],"r");
			  else                   fd=freopen(argv[i],"r",fd);
			if (fd == (FILE *) NULL) {
				perror(argv[i]);
				exit(1);
			} else {
				send(fd);
			}
		}
		fclose(fd);
	}
	exit(0);
}

void
send(fd)
	FILE *fd;
{
	register int c;
	register int l, cp, i; 
	int pages, linect, vertical;
	int halves; /* are we in half-line mode? */
	int evenit; /* flag is set if more than one \f ends the document */
	int pending; /* pending half-line spaces */
	int perlf;   /* the half-lines covered by each \n */
	char ch;

	/* first off, each of the overstrikes is set to max length */
	/* so that it will be completely blanked the first time */
	for (l=0; l<numlns; l++) {
		lenln[l] = linelen;
	}

	pending=0;
	linect=0;
	pages=0;
	c=0;
	cp=0;
	halves=0;
	perlf=2;
	while (c != EOF) { /* execute once per line */

		/* blank the line-buffers and set them to zero length */
		for (l=0; l<numlns; l++) {
			for (i=0; i<=lenln[l]; i++) line[l][i]=' ';
			lenln[l] = 0;
		}

		/* clear the underscore buffer and bold buffer */
		for (i=0; i<linelen ; i++) {
			under[i] = 0;
			bold[i] = 0;
		}

		/* read to the end of a line */
		for (;;) { /* character loop */
			if ((c = getc(fd)) == EOF) {
				if (linect > 1) {
					pages += ((linect-1)/pagelen);
					vertical=0;
				}
				break;
			}
			if (c=='\r') {cp = 0; continue; }
			if (c=='\t') {cp += 8 - (cp%8); continue;}
			if (c=='\033') {
				if ((c = getc(fd)) == EOF) {
					if (linect > 1) {
						pages += ((linect -1 )
							/pagelen);
						vertical = 0;
					}
					break;
				}
				if (c == '9') { 
					vertical = 1; 
					break; 
				}
				continue ; /* ignore other escape sequences */
			}
			if (c=='\n') { cp = 0; vertical = 2; break; }
			if (c=='\f') {
				vertical = -1; /* this is a flag */
				break;
			}
			if (c=='\0') continue;
			if (c=='\b') {
				if (cp>0) cp--;
			} else if (cp >= linelen || c == ' ') {
				cp ++;  /* truncate the line */
			} else if (underflag && (c == '_')) {
				under[cp]++;
				if (boldflag && under[cp] > 1) bold[cp] = 1;
				cp++ ;
				if (lenln[0]<cp) lenln[0] = cp;
			} else {
				for (l=0; l<numlns; l++) {
					if (boldflag && line[l][cp] == c) {
						bold[cp] = 1;
						break;
					}
					if (line[l][cp] == ' ') {
						line[l][cp] = c;
						break;
					}
				}
				cp++ ;
				if (lenln[l]<cp) lenln[l] = cp;
			}
		} /* end of character loop */

		assert(pending>=0);
		if (lenln[0] != 0 || c == EOF) {
			/*********
			/* getting here indicates that we
			/* have to perform any paper motions needed to get
			/* us to this spot.
			/*********

			/* end the previous line (if any) */
			if (!firstflag) {
				if (pending && (pending < perlf)) {
					halves = 1;
					perlf = 1;
					putns(capHL,4);
				} 
				putchar('\n');
				pending -= perlf;
				linect  += perlf;
				if (linect >= pagelen) { /* a rare case */
					linect -= pagelen;
					pages++;
				}
			}
			firstflag = 0;

			/* do spacing in whole pages.  This gets us back
			/* to normal spacing mode at each page-break, and gives
			/* us a chance to use \f spacing. */
			while (linect + pending >= pagelen) {
				i = pagelen - linect;
				if (halves) {
					putnl(i);
					putns(capTF,2);
					putns(capFL,2);
					halves = 0;
					perlf = 2;
				} else {
					putchar('\f');
				}
				pages ++;
				pending -= i;
				linect = 0;
			}

			/* now do intra-page spacing */
			/* do various things, depending on which has the least
			/* number of characters. */
			if ((i = pending/perlf) > 8) {  /* direct line skip */
				putnl(i);
				pending -= i * perlf;
				linect  += i * perlf;
			}
			while ( perlf <= pending ) { /* newlines in curr mode */
				putchar('\n');
				pending -= perlf;
				linect  += perlf;
			}
			if (!halves && (pending & 1)) {  /* halfline newline */
				halves = 1;  /* enter half-line counting */
				perlf = 1;
				putns(capHL,4);
			}
			if (pending) {
				putchar('\n');
				pending -= perlf ;
				linect += perlf ;
			}
			assert(pending==0);
		}

		/* start printing the overstrikes.  Each one after the
		/* first starts with a \r (carriage return) so that the
		/* printer knows where things go. */
		for (l=0; l<numlns; l++) {
			/* the first overstrike of length 0 is the end of
			/* the set */
			if (lenln[l] == 0) break;


			/* each overstrike starts at col 1 */
			if (l) putchar('\r');
			{
				int ustate = 0;
				int newustate;
				int bstate = 0;
				int newbstate;
				int anchor = 0;
				int op;
				for (op=0; op<lenln[0]; op++){
					/* underscore as many times as seen */
					if (underflag) {
						newustate = (l == 0) &&
							under[op];
					} else newustate = under[op] > l;
					newbstate = boldflag && (bold[op] != 0);
					if ((newustate != ustate) ||
					    (newbstate != bstate)) {
						if (op != anchor)
						putns(&line[l][anchor],
							op-anchor);
						anchor = op;
						if (ustate != newustate) {
						    if (newustate) {
							putns(capus,2);
						    } else {
							putns(capue,2);
						    }
						    ustate = newustate;
						}
						if (bstate != newbstate) {
						    if (newbstate) {
							putns(capso,2);
						    } else {
							putns(capse,2);
						    }
						    bstate = newbstate;
						}
					}
				} /* end of line */

				/* back off to last printing char */
				i = op;
				if (!ustate) {
					for (i=op; i > anchor; i--) {
						if (line[l][i-1] != ' ') {
							break;
						}
					}
				}

				/* output the last hunk of the line */
				if (i != anchor) {
					putns(&line[l][anchor],i-anchor);
				}

				/* end underline mode */
				if (ustate) putns(capue,2);

				/* end bold mode */
				if (bstate) putns(capse,2);
			}  /* end of overstrikes */
		}  /* end of the line */

		/* finally, add in the line-spacing which is required by the
		/* final character of the line: one or two halflines, or
		/* enough to finish a page. */
		if (vertical > 0) pending += vertical;
		if (vertical < 0) 
			pending = pagelen - ((pending + linect) % pagelen);
		vertical = 0;

	}  /* end of lines in the input */

	/* adjust for even-page endings.  If there's more than
	/* one page of whitespace at the end, or if the evenpage
	/* flag is set, we make sure the number of pages is even */
	if (pending > pagelen) evenflag = 1;
	pending = (linect ? pagelen : 0) - linect;
	/* if we 'will have' printed an odd number of
	/* pages, we might force another */
	if (((pages + (pending ? 1 : 0)) & 1) 
			&& evenflag) pending += pagelen;

	/* do spacing in whole pages.  This gets us back
	/* to normal spacing mode at each page-break, and gives
	/* us a chance to use \f spacing. */
	while (linect + pending >= pagelen) {
		i = pagelen - linect;
		if (halves) {
			putnl(i);
			putns(capTF,2);
			halves = 0;
			perlf = 2;
		} else {
			putchar('\f\n');
		}
		pages ++;
		pending -= i;
		linect = 0;
	}
	assert(pending==0);
	assert(linect==0);
}

void
putns(s,n)
	char *s;
	int n;
{
	register int i;

	for (i=0; i<n; i++) putchar(s[i]);
}

void
putnl(n)
	int n;
{
	while (n) {
		if (nlflag) {
			if (n>99) {
				putns(capVM,99);
				n -= 99;
			} else {
				putns(capVM,n);
				n = 0;
			}
		} else {
			putchar('\n');
			n-- ;
		}
	}
}
