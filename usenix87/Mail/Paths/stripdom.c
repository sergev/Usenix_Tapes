/*
 * LOCAL DOMAIN QUALIFICATION PROGRAM
 *
 *	@(#)stripdom.c	1.2	9/19/86
 *	@(#) Ronald S. Karr <tron@mesa.nsc.com>
 *	@(#) National Semiconductor, Sunnyvale
 *
 *	This program accepts on the standard input a pathalias path
 *	database and an argument list containing fully qualified
 *	domains that the current machine is in.  Then on the standard
 *	output this list is reproduced with any hosts in the given
 *	fully-qualified domains reproduced on separate lines in less
 *	qualified domains.  The output is ordered such that a sorted
 *	file on input will be sorted on output as well.
 *
 * USAGE:
 *	pathalias ... | stripdomain full-domain full-domain ... | ...
 *
 * NOTE:
 *	ordering of full-domains in the argument list is not important
 *	as the arguments are sorted by size before processing starts.
 *	Also:  Domains should be given with the initial dot.
 *
 * EXAMPLE:
 *	Given the command "stripdomain .nsc.com .sc.nsc.com" the following
 *	input:
 *		a.sc.nsc.com	m!a!%s
 *		b.b16.sc.nsc.com	n!b!%s
 *		c.nsc.com	o!c!%s
 *		d.ta.nsc.com	p!d!%s
 *		e	q!e!%s
 *		f.com	r!f!%s
 *
 *	produces the following output:
 *		a.sc	m!a!%s
 *		a.sc.nsc	m!a!%s
 *		a.sc.nsc.com	m!a!%s
 *		b.b16.sc	n!b!%s
 *		b.b16.sc.nsc	n!b!%s
 *		b.b16.sc.nsc.com	n!b!%s
 *		c.nsc	o!c!%s
 *		c.nsc.com	o!c!%s
 *		d.ta.nsc	p!d!%s
 *		d.ta.nsc.com	p!d!%s
 *		e	q!e!%s
 *		f.com	r!f!%s
 */

#include <stdio.h>
#include <ctype.h>

#define SMLBUF	512

/* read input strings into here */
char buf[SMLBUF];

main(argc,argv)
	int argc;
	register char *argv[];
{
	register char *p, *s;
	register i;
	static int lencompare();

	/* for quick reference, make a table of lengths for the
	 * the various arguments.
	 */
	register int *l = (int *)malloc((--argc) * sizeof(int));

	argv++;

	/* sort arguments by size (largest to smallest) */
	qsort(argv, argc, sizeof(char *), lencompare);
	for (i = 0; argv[i] != NULL; i++) {
		l[i] = strlen(argv[i]);
	}

	p = NULL;
	s = buf;

	/* main processing loop, just get and process characters */
	for (;;) switch (*s++ = getchar()) {

	/* mark the first white space we see in a line */
	case '\t':
	case ' ':
		if (p == NULL) {
			p = s - 1;
		}
		break;

	/* end of line processing */
	case '\n':
		s[-1] = 0;		/* terminate the line */
		s = p;			/* keep track of the mark */
		if (p == NULL) break;	/* something is odd, but ignore */

		/* look for local domain names before the mark */
		for (i = 0; argv[i] != NULL; i++) {
			p -= l[i];	/* note potential start of the domain */

			/* if it couldn't possibly fit, try the next one */
			if (p - buf < 0) {
				p += l[i];
				continue;
			}

			/* if it isn't a match, try the next one */
			if (strncmp(argv[i], p, l[i]) != 0) {
				p += l[i];
				continue;
			}

			/* found a match with a domain name */
			for (;;) {

				/* look for the NEXT dot */
				while (*++p != '.' && !isspace(*p)) ;

				/* no more dots in domain */
				if (isspace(*p)) break;

				/* output before the dot and from the mark */
				*p = '\0';
				fputs(buf, stdout);
				puts(s);
				*p = '.';
			}
			break;
		}
		p = NULL;
		s = buf;
		puts(s);		/* output the line as is */
		break;

	case EOF:
		exit(0);		/* all done */
	}
}

static int
lencompare(a,b)
	char **a,**b;
{
	register la,lb;

	la = strlen(*a);
	lb = strlen(*b);
	if (la == lb) return (0);
	if (la < lb) return (1);
	else return (-1);
}
