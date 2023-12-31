/*
** convert the host name on a pathalias line to lower case
*/

#ifndef lint
static char 	*sccsid="@(#)lcasep.c	2.1 (smail) 12/14/86";
#endif

#include <stdio.h>
#include <ctype.h>

# define lower(c) 		( isupper(c) ? c-'A'+'a' : c )

main(argc, argv)
int argc;
char *argv[];
{
	FILE *ifp, *ofp;
	char buf[BUFSIZ];
	register char *p;
	int c;

	extern int optind;
	extern char *optarg;

	ifp = stdin;
	ofp = stdout;

	while((c = getopt(argc, argv, "f:o:")) != EOF) {
		switch(c) {
		case 'f':
			if((ifp = fopen(optarg, "r")) == NULL) {
				(void) fprintf(stderr, "%s: can't open %s: ",
					argv[0], optarg);
				perror("");
				exit(1);
			}
			break;
		case 'o':
			if((ofp = fopen(optarg, "w+")) == NULL) {
				(void) fprintf(stderr, "%s: can't open %s: ",
					argv[0], optarg);
				perror("");
				exit(1);
			}
			break;
		default:
			(void) fprintf(stderr,
				"usage: %s [-f file] [-o outfile]\n", argv[0]);
			exit(1);
			/* NOTREACHED */
			break;
		}
	}

	while(fgets(buf, sizeof(buf), ifp) != NULL) {
		for(p = buf; *p != '\t' && *p != '\0' ; p++) {
			(void) fputc(lower(*p), ofp);
		}
		(void) fputs(p, ofp);
	}
}
