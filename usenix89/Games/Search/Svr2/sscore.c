#ifndef lint
static char rcsid[] = "$Header: sscore.c,v 1.4 85/07/08 17:22:57 matt Exp $";
#endif
/*
 *
 * Original by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * program to sort and print the player scoreboard.
 */


#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include "defines.h"
#include "structs.h"


struct totals {
	char	ptname[20];	/* player name */
	int	ptpoints;	/* total points */
	int	pthits;		/* total player hits */
	int	ptahits;	/* total alien hits */
	int	ptkills;	/* total player kills */
	int	ptkilled;	/* total times killed */
	int	ptlast;		/* last game's score */
	int	ptbest;		/* best score */
	int	ptgames;	/* # games played */
	int	average;	/* average score */
} ;

struct totals *malloc();

main() {
	register int i,
		j,
		found,
		next;
	register struct totals *lp;
	int	pointfd;
	struct	totals *list[2000];
	long	number;

	for (i=0; i<2000; i++)
		list[i] = NULL;
	if ((pointfd = open(POINTFILE, 0)) < 0)
		fatal("can't open points file %s", POINTFILE);
	printf("%-17s %-5s %-8s %-7s %-7s %-5s %-6s %-5s %-5s %-4s\n",
		"Player", "Games", "Total", "Hits(p)", "Hits(a)",
		"Kills", "Killed", "Best", "Last", "Avg");
	for (number=0; number<2000; number++) {
		lp = malloc(sizeof(struct totals));
		if (lp == NULL)
			fatal("out of memory\n");
		if (read(pointfd, lp, sizeof(t_totals)) <= 0) {
			free(lp);
			break;
		}
		if (lp->ptgames > 0)
			lp->average = lp->ptpoints/lp->ptgames;
		else
			lp->average = 0;
		list[number] = lp;
	}
	if ( number == 0 )
	        exit(0);
	found = 1;
	for (i=number-1; i && found; i--) {
		found = 0;
		for (j=0, next=1; j<i; j++, next++) {
			if (list[j]->ptbest < list[next]->ptbest) {
				lp = list[j];
				list[j] = list[next];
				list[next] = lp;
				found++;
			}
		}
	}
	for (i=0; i<2000; i++) {
		lp = list[i];
		if (lp == NULL || lp->ptpoints == 0)
			continue;
		printf("%-17s %-5d %-8d %-7d %-7d %-5d %-6d %-5d %-5d %-4d\n",
			lp->ptname, lp->ptgames, lp->ptpoints, lp->pthits,
			lp->ptahits, lp->ptkills, lp->ptkilled, lp->ptbest,
			lp->ptlast, lp->average);
	}
}

fatal(fmt, arg)
char *fmt;
{
	fprintf(stderr, "sscore:");
	fprintf(stderr, fmt, arg);
	fputc('\n', stderr);
	exit(1);
}
