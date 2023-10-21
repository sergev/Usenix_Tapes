/*
 * PATH-FILE MERGING PROGRAM
 *
 *	@(#)pathmerge.c	1.2	9/19/86
 *	@(#) Ronald S. Karr <tron@mesa.nsc.com>
 *	@(#) National Semiconductor, Sunnyvale
 *
 *	This program takes a set of sorted path files, as produced
 *	by pathalias, and generates on the standard output a merge
 *	of the path information, with one path given for each
 *	hostname.  Precedence in preferred paths goes to the files
 *	given earliest in the argument list.  One of the filenames
 *	given in the argument list can be `-' for the standard
 *	input.
 */
#include <stdio.h>

extern errno;
char *malloc();

#define SMLBUF	512

main(argc,argv)
	register int argc;
	char *argv[];
{
	register int i;

	/* current input line, per file */
	register char **s  = (char **)malloc((argc - 1) * sizeof(char **));
	/* true if the per-file line is valid */
	register short *p  = (short *)malloc((argc - 1) * sizeof(short *));
	/* per-file file descriptors */
	register FILE **fd = (FILE **)malloc((argc - 1) * sizeof(FILE **));

	register int k;

	argv++; argc--;			/* we don't care about the prog name */

	if (argc == 0) exit(0);		/* an unusual case */

	/* initialize all of the variables */
	k = 0;
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-") == 0) {
			if (k) {
				fprintf(stderr, "cannot open the standard input more than once\n");
			}
			fd[i] = stdin;
		} else {
			if ((fd[i] = fopen(argv[i], "r")) == NULL) {
				perror(argv[i]);
				exit(errno);
			}
		}
		s[i] = (char *)malloc(SMLBUF);
		p[i] = 0;		/* lines not valid yet */
	}

	/* take care of special case of one file */
	if (argc == 1) {
		while ((i = getc(fd[0])) != EOF) putchar(i);
		exit(0);
	}

	/* main operating loop */
	for (;;) {

		/* for files that need a line, read one in */
		k = 0;			/* count how many are at end of file */
		for (i = 0; i < argc; i++) {
			if (p[i] == 0) {
				if (fgets(s[i], SMLBUF, fd[i]) != NULL) p[i]++;
				else k++;
			}
		}
		if (k == argc) exit(0);		/* if all files then done */

		/* find the alphabetically least line */
		k = 0;
		for (i = 1; i < argc; i++) {
			if (p[k] == 0) k = i;	/* only use valid lines */
			if (p[i] == 0) continue;	/* no line yet */
			switch (cmp(s[i], s[k])) {

			case 0:
				/* if two files have the same hostname
				 * don't use the second
				 */
				p[i] = 0;
				break;
			case -1:
				k = i;
				break;
			}
		}
		fputs(s[k], stdout);		/* write out that line */
		p[k] = 0;			/* read in the next */
	}
}

/* compare two strings up to the first tab character
 * to determine if the first is less than, equal to,
 * or greater than the second, returning -1, 0 and 1
 * respectively.
 */
cmp(s, t)
	register char *s, *t;
{
	while (*s == *t && *s != '\t' && *s != '\0') s++, t++;
	if (*s == *t) return (0);
	if (*s == '\t') return (-1);
	if (*t == '\t') return (1);
	return (*s<*t? -1: 1);
}
