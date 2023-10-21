/*
 * rfuncs - functions for readnews.
 */

static char	*SccsId = "@(#)rfuncs.c	2.9	3/7/83";

#include "rparams.h"
#include "newsrc.h"

#ifdef notdef
/*
 * Figure out the number of the largest article in newsgroup ng,
 * and return that value.
 */
long
findngsize(ng)
char *ng;
{
	FILE *af;
	long s;
	char buf[100], n[100];

	af = xfopen(ACTIVE, "r");
	while (fgets(buf, sizeof buf, af)) {
		sscanf(buf, "%s %ld", n, &s);
		if (strcmp(n, ng) == 0) {
			fclose(af);
			return s;
		}
	}
	return 0;
}
#endif



xxit(status)
int	status;
{
	exit(status);
}


/*
 * Return true if the newsgroup was specified in the -n option.
 */

wewant(name)
	char *name;
	{
	return ngmatch(name, sublist);
}
