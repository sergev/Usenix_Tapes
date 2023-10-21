/*
 * nglist is the list of newsgroups in an article we want to follow up.
 * Do any special fascist processing to prevent certain kinds of followups.
 * In this case, there are two things we want to do:
 *	All followups to "net.general" are fed to "net.followup".
 *	However, if "net.general" is mentioned along with "net.news.group",
 *		just remove the net.general.
 */

#include "config.h"

char *index();

launder(nglist)
char	*nglist;
{
	char	*cp;
	char	outbuf[128];
	int	seen_group = 0;

	for (cp = index(nglist, 'n'); cp; cp = index(cp + 1, 'n'))
		if (strncmp("news.group", cp, 10) == 0)
			seen_group++;
	for (cp = index(nglist, 'n'); cp; cp = index(cp + 1, 'n'))
		if (strncmp("net.general", cp, 11) == 0) {
			/* 11 = strlen("net.general") */
			strcpy(outbuf, cp + 11);
			if (!seen_group) {
				strcpy(cp, "net.followup");
				cp += 12;  /* 12 = strlen("net.followup") */
			}
			if (cp[-1] == ',' && outbuf[0] == ',')
				cp--;
			strcpy(cp, outbuf);
		}
}
