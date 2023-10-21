/*
 * mk: detect a compilation command in a file, and execute it
 *	(formerly called 'compile')
 *
 * Usage: mk [-m marker] [-d submarker] [-D defn] [-s] [-n] file ...
 *
 * example marker line:
 *
 *	$Compile:   cc -o %F -O %f&
 *	$Compile (TEKECS): cc -o %F -DLOG -O %f&
 *	$Compile (DEBUG): cc -o %F -g -DDEBUG %f&
 *	
 *
 * this program searches for the first occurence of a marker (DEFLTMARK)
 * in the first block of the named file(s), grabs the line on which
 * the marker occurs, performs some filename substitutions on the line,
 * and prints the line (typically a shell command line) on the stdout.
 *
 * this programs currently makes the following substitutions:
 *
 *	%f	- full name, as spec'd on command line
 *	%F	- non-prefix, non-extension part of filename
 *		  (e.g.) 'foo' in 's.foo.c'
 *	%p	- prefix
 *	%x	- extension - this is the string following the LAST '.'
 *	%d	- directory part of filename
 *	%'name'	- the value defined for "name"
 *	%{name}	- the value of "name" from the environment
 *	\n	- newline
 *	\t	- tab
 *	\nnn	- (nnn = octal number) character escape
 *
 * command-line switches:
 *
 *	-n	- don't execute, just print (a la 'make')
 *	-s	- silent (a la 'make')
 *	-Dfoo=x	- define a variable which can be expanded by %'foo' or %"foo"
 *	-m mark	- specify alternate marker
 *	-d submark - select marker option	(e.g. compile -d DEBUG ...)
 *			$Compile (DEBUG): ...
 *			$Compile (PDP11): ...
 *
 * planned additions:
 *	%#[1..n]- the n'th character of the filename
 *	%P	- current working directory
 *	%r	- comma-extension (e.g. 'foo.c,v')
 *	-p	- a switch to turn off prefix processing
 *	-x c	- specify alternate extension delimeter (instead of '.')
 *
 *
 *	(c) Copyright 1983, Steven McGeady
 *
 *	This program may be redistributed to other computer sites, but
 *	not for profit, and providing that this notice remains intact.
 *
 *	All bug fixes and improvements should be mailed to the author.
 *
 *
 * Author:
 *	S. McGeady
 */


static char *SCCSid = "@(#)mk.c	1.4	mk - S. McGeady";

#include <stdio.h>
#include <ctype.h>


extern char *strcpy();
extern char *rindex();
extern char *index();
extern char *getenv();

extern char *translit();	/* forward reference */
extern char *valof();		/* forward reference */

/* #define	DEBUG(fmt, lst)	fprintf(stderr, fmt, lst); */

#define	BACKSL	'\134'
#define	NUMDEFS	25
#define	MAXLIN	BUFSIZ
#define	TRUE	1
#define	FALSE	0

#define	LEADCHAR	'$'
#define	DEFLTMARK	"Compile"
char *markstr	= DEFLTMARK;
char *submark	= NULL;

char *myname;
char *curfile;
int silent	= FALSE;
int exec	= TRUE;

struct names {
	char	*nm_name;
	char	*nm_value;
} nmlist[NUMDEFS];

main(argc, argv)
int argc;
char **argv;
{
	char buf[MAXLIN];
	char combuf[MAXLIN];
	register char *p;
	register int i;
	register FILE *fin;
	int retval = 0;

	if ((myname = rindex(argv[0], '/')) == NULL) {
		myname = argv[0];
	} else {
		myname++;
	}

#ifdef LOG
	log(argc, argv);
#endif

	if (argc < 2) {
		usage();
		exit(1);
	}

	/*
	 * main loop, process files
	 */

	for(i = 1; i < argc; i++) {
		if ((argv[i][0] == '-') && (argv[i][1] != '\0')) {
			p = &(argv[i][1]);
		loop:	/* sorry ... */
			switch (*p) {
			case 'n':	/* don't execute */
				exec = FALSE;
				if (p[1] != '\0') {
					p++;
					goto loop;
				}
				break;
			case 's':	/* act silently */
				silent = TRUE;
				if (p[1] != '\0') {
					p++;
					goto loop;
				}
				break;
			case 'm':	/* select alternate marker */
				if (p[1] == '\0') {
					if (++i < argc) {
						markstr = argv[i];
					} else {
						error("no marker specified after -m");
					}
				} else {
					markstr = ++p;
				}
				break;
			case 'd':	/* submarker selection */
				if (p[1] == '\0') {
					if (++i < argc) {
						submark = argv[i];
					} else {
						error("no submarker specified after -d");
					}
				} else {
					submark = ++p;
				}
				break;
			case 'D':	/* define */
				if (p[1] == '\0') {
					if (++i < argc) {
						define(argv[i]);
					} else {
						error("no definition after -D");
					}
				} else {
					++p;
					define(p);
				}
				break;
			default:	/* ??? */
				error("unrecognized switch -%c", *p);
				break;
			}
			continue;
		}
		curfile = argv[i];
		if (strcmp(curfile, "-") == 0) {	/* '-' indicate stdin */
			fin = stdin;
		} else if ((fin=fopen(curfile, "r")) == NULL) {
			error("cannot open %s", curfile);
			retval++;
			continue;
		}
		if (!find(buf, LEADCHAR, markstr, submark, fin)) {
			if (submark == NULL) {
				error("no marker \"%c%s:\" in %s",LEADCHAR,markstr,curfile);
			} else {
				error("no marker \"%c%s (%s):\" in %s",
					LEADCHAR,markstr,submark,curfile);
			}
			retval++;
		} else {
			translit(combuf, buf);
			if (exec){
				if (!silent) fprintf(stderr, "+ %s\n", combuf);
				fflush(stderr);
				system(combuf);
			} else if (!silent) {
				printf("%s\n", combuf);
				fflush(stdout);
			}
			
		}
		if (fin != stdin) fclose(fin);
	}
	exit(retval);
}

find(bp, lead, mark, smark, f)
char *bp;
char lead;
char *mark;
char *smark;
register FILE *f;
{
	char buf[MAXLIN];
	char smbuf[MAXLIN];
	int found = FALSE;
	register int i;
	register int c;
	register char *p;
	register char *xp;

	if (smark && strlen(smark) > 0) {
		sprintf(smbuf, "(%s)", smark);
	}
	for (i=0; i < BUFSIZ; i++) {
		if ((c = getc(f)) == EOF) break;
		if (c == lead) {
			if (fgets(buf, MAXLIN, f) == NULL) break;
			if (strncmp(mark, buf, strlen(mark)) == 0) {
				p = &(buf[strlen(mark)]);
				xp = p;
				if ((p = index(p, ':')) == NULL) {
					continue;
				}
				if (smark) {
					if ((xp=index(xp, '(')) != NULL) {
						if (strncmp(xp, smbuf, strlen(smbuf)) != 0) {
							continue;	/* submarker compare fails */
						}
					} else { /* no submarker */
						continue; /* fail if no submarker found */
					}
				}
				found = TRUE;


				while(isspace(*++p)) /* skip leading spaces */
					;
				xp = p;
				/* terminate line on '$$' or '\n' */
				while(*xp) {
					if((xp = index(xp, LEADCHAR)) == NULL) {
						break;
					} else if (xp[1] == LEADCHAR) {
						*xp = '\0';
						break;
					}
					xp++;
				}
				if ((xp=rindex(p, '\n')) != NULL) {
					*xp = '\0';
				}
				strcpy(bp, p);
				break;
			}
		}
	}
	return(found);
}

char *
translit(dst, src)
register char *dst;
register char *src;
{
	register char *tp;
	register char *xp;
	register char c;
	int radix;

	/*fprintf(stderr, "translit(%s)\n", src);/*DBG*/

	while (*src) {

		switch (*src) {

		case '%': 
			switch (*++src) {
			case '\'':	/* %'name' == define */
			case '"':
				c = *src++;
				if ((xp = index(src, c&0377)) == NULL) {
					break;
				}
				*xp = '\0';
				strcpy(dst, valof(src));
				while(*dst++)
					;
				dst--;
				src = xp;
				break;

			case '{':	/* %{name} == get name from env */
				xp = ++src;
				if ((tp = index(src, '}')) == NULL) {
					break;
				}
				*tp = '\0';
				src = tp;
				if ((tp = getenv(xp)) == NULL) {
					tp = "";
				}
				strcpy(dst, tp);
				while(*dst++)
					;
				dst--;
				break;

			case 'f':	/* full filename */
				strcpy(dst, curfile);
				dst += strlen(curfile);
				break;

			case 'F':	/* file part only */
				if ((tp = rindex(curfile, '/')) != NULL) {
					tp++;
				} else {
					tp = curfile;
				}
				if ((xp = rindex(tp, '.')) != NULL) {
					*xp = '\0';
					strcpy(dst, tp);
					dst += strlen(tp);
					*xp = '.';
				} else {
					strcpy(dst, tp);
					dst += strlen(tp);
				}
				break;

			case 'x':	/* . extension */
				if ((tp = rindex(curfile, '.')) == NULL) {
					break;
				}
				strcpy(dst, ++tp);
				dst += strlen(tp);
				break;

			case 'p':	/* prefix */
				if ((tp = index(curfile, '.')) == NULL) {
					break;
				}
				*tp = '\0';
				strcpy(dst, curfile);
				*tp = '.';
				break;

			case 'd':	/* directory part */
				if ((tp = rindex(curfile, '/')) == NULL) {
					break;
				}
				c = *++tp;
				*tp = '\0';
				strcpy(dst, curfile);
				dst += strlen(curfile);
				*tp = c;
				break;

			default:	/* unrecognized chars are copied thru */
				*dst++ = *src;
				break;
			}		/* end of % codes switch */
			src++;
			break;

		case BACKSL:

			radix = 8;
			switch (*++src) {
				case 'n':	/* newline */
					*dst++ = '\n';
					break;

				case BACKSL:
					*dst++ = BACKSL;
					break;

				case NULL:
					*dst++ = src[-1];
					break;

				case 't':
					*dst++ = '\t';
					break;

				case '0':
					if (src[1] == '0') src++;
					/*FALLTHROUGH*/

				case '1': case '2': case '3':
				case '4': case '5': case '6':
				case '7': case '8': case '9':

					{
					register int i;
					char numbuf[10];
					char *p;
					register int base;
					register int num;
					int c;

					for (i=0; i < (radix==16 ? 2 : 3);i++) {
						numbuf[i] = *src++;
						if (radix == 8 && (numbuf[i] > '7' || numbuf[i] < '0')) {
							numbuf[i] = NULL;
							src -= 2;
							break;
						}
					}
					numbuf[(radix==16 ? 2 : 3)] = NULL;

					base = 1;
					num = 0;
					for (i=(radix==16 ? 1:(numbuf[2]==NULL ? 1:2));i>=0;i--) {
						if (numbuf[i] > 'a') numbuf[i] -= 'a' - 'A';
						if (radix != 16) {
							c = numbuf[i] -  '0';
						} else {
							if (numbuf[i] >= '0' && numbuf[i] <= '9') {
								c = numbuf[i] = '0';
							} else if (numbuf[i] >= 'A' && numbuf[i] <= 'F') {
								c = numbuf[i]-'A'+10;
							} else {
								c = 0;
							}
						}
						num += c*base;
						base *= radix;
					}

					*dst++ = num;
					}
					break;

				default:
					break;

			} /* end of backslash codes switch */
			src++;
			break;

		/********
		case '$':
			if (*++src != '$') {
				*dst++ = '$';
				break;
			}
			/*FALLTHROUGH
		case '\n':
			*src = '\0';
			break;
		*********/

		default:
			*dst++ = *src++;
			break;
		} /* end of outer switch */
	}

	*dst = '\0';
	return(dst);
}

static int nmcur = 0;

define(str)
char *str;
{
	register int i;
	register struct names *nm = nmlist;

	/*fprintf(stderr, "define('%s')\n", str);/*DBG*/

	nm = &nmlist[nmcur];
	nm->nm_name = str;
	if ((nm->nm_value = index(str, '=')) != NULL) {
		*nm->nm_value++ = '\0';
	} else {
		nm->nm_value = "";
	}
	for(i=0; i < nmcur; i++) {
		if (strcmp(nm->nm_name, nmlist[i].nm_name) == 0) {
			nmlist[i].nm_value = nm->nm_value;
			nmcur--;
			break;
		}
	}
	nmcur++;
	return;
}

char *
valof(str)
char *str;
{
	register struct names *nm;

	/*fprintf(stderr, "valof('%s')\n", str);/*DBG*/

	for(nm=nmlist; nm < &nmlist[nmcur]; nm++) {
		if (strcmp(nm->nm_name, str) == 0) {
			return(nm->nm_value);
		}
	}
	return("");
}

/*VARARGS*/
error(fmt, a1, a2, a3, a4, a5, a6, a7)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6, *a7;
{
	fprintf(stderr, "%s: ", myname);
	fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6, a7);
	fprintf(stderr, "\n");
	fflush(stderr);
}

usage(){
	 fprintf(stderr,
		"Usage: %s [-m marker] [-d submarker] [-D defn] [-s] [-n] file ...\n", myname);
}

#ifdef LOG

#define	LOGFILE	"/cc/stevenm/tmp/mk.log"
extern char *ctime();

log(ac, av)
int ac;
char **av;
{
	long t;
	register char *p, *q;
	char pwbuf[MAXLIN];
	FILE *flog;

	getpw(getuid(), pwbuf);
	p = index(pwbuf, ':');
	*p = '\0';
	/* don't log the author */
	if (!strcmp("stevenm", pwbuf) || !strcmp("mcg", pwbuf)) {
		return;
	}
	if ((flog = fopen(LOGFILE, "a")) != NULL) {
		fprintf(flog, "%s: ", pwbuf);
		while(ac--) {
			fprintf(flog, "%s ", *av++);
		}
		time(&t);
		q = p = ctime(&t);
		while(*p++ != '\n');
		p[-1] = '\0';
		fprintf(flog, "(%s)\n", q);
		fclose(flog);
	}
}

#endif
