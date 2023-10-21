
#include <stdio.h>
#include <ctype.h>

/*
 * This program takes a newline separated file of article id's and
 * prints out a line with the id and the file name(s) containing the
 * articles.  Article id's are the first blank separated word on each
 * line and are surrounded by a '<' and '>' - just like the history file.
 * This only works on news systems that are configured with the dbm
 * routines.
 *
 * This does not understand any locking.
 *
 * Usage:  idart [file]
 *
 *	Ben Golding.  June 8, 1987.
 */

#define	HISTORY	"/usr/lib/news/history"

typedef struct	{
	char	*dptr;
	int	dsize;
} datum;

char	*malloc();
datum	fetch();

main(argc, argv)
int	argc;
char	*argv[];
{
	register i;
	char	id[512], fname[8192];
	datum	d, art;
	long	off;
	FILE	*hp;

	if (argc > 1) {
		if (freopen(argv[1], "r", stdin) == (FILE *)NULL) {
			perror(argv[1]);
			exit(1);
		}
	}
	if ((d.dptr = malloc(1024)) == 0) {
		perror("malloc");
		exit(1);
	}
	if (dbminit(HISTORY) != 0) {
		fprintf(stderr, "dbminit: %s\n", HISTORY);
		exit(1);
	}
	if ((hp = fopen(HISTORY, "r")) == (FILE *)NULL) {
		perror(HISTORY);
		exit(1);
	}
	while (scanf("%s%*[^\n]\n", id) != EOF) {
		d.dptr = &id[0];
		d.dsize = strlen(id) + 1;	/* includes the null!(?) */
		for (i = 0; id[i] != '\0'; i++)
			if (isupper(id[i]))
				id[i] = tolower(id[i]);
		art = fetch(d);
		if (art.dptr == (char *)NULL)
			fprintf(stderr, "%s: not found in history\n", id);
		else {
			bcopy(art.dptr, &off, sizeof(long));
			if (fseek(hp, off, 0) < 0) {
				perror(HISTORY);
				continue;
			}
			if (fscanf(hp, "%s%*s%*s%[^\n]\n", id, fname) != 2)
				fprintf(stderr, "%s: error in history\n", id);
			else {
				if (strcmp(fname, "") == 0
				||  strcmp(fname, "\t") == 0)
					fprintf(stderr, "%s: expired\n", id);
				else
					printf("%s\t%s\n", id, fname);
			}
		}
	}
	dbmclose();
	exit(0);
}
