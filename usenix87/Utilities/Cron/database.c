#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: database.c,v 1.2 87/05/02 17:33:31 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/database.c,v $
 * $Revision: 1.2 $
 * $Log:	database.c,v $
 * Revision 1.2  87/05/02  17:33:31  paul
 * baseline for mod.sources release
 * 
 * Revision 1.1  87/01/26  23:46:36  paul
 * Initial revision
 * 
 */

/* Copyright 1987 by Vixie Enterprises
 * All rights reserved
 *
 * Distribute freely, except: don't sell it, don't remove my name from the
 * source or documentation (don't take credit for my work), mark your changes
 * (don't get me blamed for your possible bugs), don't alter or remove this
 * notice.  Commercial redistribution is negotiable; contact me for details.
 *
 * Send bug reports, bug fixes, enhancements, requests, flames, etc., and
 * I'll try to keep a version up to date.  I can be reached as follows:
 * Paul Vixie, Vixie Enterprises, 329 Noe Street, San Francisco, CA, 94114,
 * (415) 864-7013, {ucbvax!dual,ames,ucsfmis,lll-crg,sun}!ptsfa!vixie!paul.
 */


#include "cron.h"
#include <pwd.h>


void
free_database(db)
	user	*db;
{
	void	free_user();
	user	*u;

	for (u = db;  u != NULL;  u = u->next)
		free_user(u);
}


user *
load_database()
{
	user		*load_user();

	struct passwd	*pw;
	user		*db, *u;

	Debug(DPARS, ("load_database()\n"))

	db = NULL;
	while (NULL != (pw = getpwent()))
	{
		Debug(DPARS, ("\t%s\n", pw->pw_name))

		u = load_user(
			pw->pw_name,
			pw->pw_uid,
			pw->pw_gid,
			pw->pw_dir,
			pw->pw_shell
		);
		if (u == NULL)
		{
			/* no crontab for this user
			 */
			continue;
		}
		u->next = db;
		db = u;
	}
	endpwent();
	return db;
}
