/*
 * getopt - get option letter from argv
 * This version of getopt was written by Henry Spencer
 * and is in the public domain.
 */

#include <stdio.h>
#include "config.h"

char	*optarg;	/* Global argument pointer. */
int	optind = 0;	/* Global argv index. */

static char	*scan = NULL;	/* Private scan pointer. */

extern char	*index();

int
getopt(argc, argv, optstring)
int argc;
char *argv[];
char *optstring;
{
	register char c;
	register char *place;

	optarg = NULL;

	if (scan == NULL || *scan == '\0') {
		if (optind == 0)
			optind++;
	
		place = argv[optind];
		if (optind >= argc || place[0] != '-' || place[1] == '\0')
			return(EOF);
		optind++;
		if (strcmp(place, "--")==0) {
			return(EOF);
		}
	
		scan = place + 1;
	}

	c = *scan++;
	place = index(optstring, c);

	if (place == NULL || c == ':') {
		fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
		return('?');
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		} else {
			if ((optarg = argv[optind]) == NULL) {
				fprintf(stderr, "%s: no arg for -%c option\n", c);
				return('?');
			}
			optind++;
		}
	}

	return(c);
}
