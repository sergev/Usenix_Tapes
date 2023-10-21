#ifndef lint
static char rcsid[] = "$Header: score.c,v 2.1 85/04/10 17:31:40 matt Stab $";
#endif
/*
 *
 * search
 *
 * multi-player and multi-system search and destroy.
 *
 * Original by Dave Pare	1983
 * Ported & improved
 *      by Matt Crawford	1985
 *
 * routine to update player scores, formerly handled in
 * search.c, but now moved to the daemon's care.
 *
 * Copyright (c) 1983
 *
 * $Log:	score.c,v $
 * Revision 2.1  85/04/10  17:31:40  matt
 * Major de-linting and minor restructuring.
 * 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>

#include "defines.h"
#include "structs.h"

void putplayer(p)
register t_player *p;
{
	extern	long	lseek();
	extern	char	*strcpy();
	extern	int	pfd;
	int	index;
	int	found;
	t_totals	entry;

	found = 0;
	(void)lseek(pfd, 0L, 0);
	for (index=0; read(pfd, (char *)&entry, sizeof(t_totals)) > 0; index++)
		if (!strcmp(p->plname, entry.ptname)) {
			found++;
			break;
		}
	if (!found) {
		bzero((char *)&entry, sizeof(entry));
		strcpy(entry.ptname, p->plname);
	} else
		(void)lseek(pfd, (long)(index * sizeof(t_totals)), 0);
	if (p->status.killed == TRUE)
		entry.ptkilled++;
	if (entry.ptbest < p->points)
		entry.ptbest = p->points;
	entry.ptpoints += p->points;
	entry.ptgames++;
	entry.pthits += p->phits;
	entry.ptkills += p->pkills;
	entry.ptahits += p->ahits;
	entry.ptlast = p->points;
	write(pfd, (char *)&entry, sizeof(t_totals));
}
