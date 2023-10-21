/*
 * Cancel the article whose header is in hp, by posting a control message
 * to cancel it.  The scope of the control message depends on who would
 * really be willing to cancel it.  It is sent as far as it will do any good.
 * notauthor is true iff the person posting this article is not the
 * real author of the article being cancelled.
 */

#include <stdio.h>
#include <sys/types.h>
#include "config.h"
#include "arthead.h"
#include "libextern.h"

cancel(ofp, hp, notauthor)
FILE *ofp;
struct arthead *hp;
int	notauthor;
{
	FILE	*inews;
	char	distgroup[64];
	int	pid;
	char	bfr[512];
	char	*index();
	FILE	*popen();

	/* fflush(stdout); */
	pid = fork();
	if (pid > 0)
		return 0;
	strcpy(distgroup, hp->h_nbuf);
	if (notauthor)
		sprintf(distgroup, "to.%s", FULLSYSNAME);
	else
		sprintf(distgroup, "%s", hp->h_nbuf);
	sprintf(bfr, "%s -t 'cmsg cancel %s' -n %s < /dev/null",
	    XINEWS, hp->h_ident, distgroup);
	if ((inews = popen(bfr, "w")) == NULL)
		fprintf(ofp, "Can't fork %s\n", XINEWS);
	else
		pclose(inews);
	if (pid == 0)
		_exit(0);
	return 0;
}
