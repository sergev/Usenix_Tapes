#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: user.c,v 1.2 87/05/02 17:34:11 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/user.c,v $
 * $Revision: 1.2 $
 * $Log:	user.c,v $
 * Revision 1.2  87/05/02  17:34:11  paul
 * baseline for mod.sources release
 * 
 * Revision 1.1  87/01/26  23:48:47  paul
 * Initial revision
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
#include <errno.h>


extern	errno;


void
free_user(u)
	user	*u;
{
	void	free_entry();
	int	free();
	entry	*e;
	char	**env;

	for (e = u->crontab;  e != NULL;  e = e->next)
		free_entry(e);
	for (env = u->envp;  *env;  env++)
		(void) free(*env);
	free((char *) u->envp);
	(void) free((char *) u);
}


user *
load_user(name, uid, gid, dir, shell)
	char	*name;
	int	uid;
	int	gid;
	char	*dir;
	char	*shell;
{
	char	*malloc(), *savestr(), *sprintf(),
		**env_init(), **env_set();
	int	load_env();
	entry	*load_entry();

	char	filename[MAX_FNAME], envstr[MAX_TEMPSTR];
	FILE	*file;
	user	*u;
	entry	*e;
	int	status;

	sprintf(filename, SPOOLDIR, name);
	if (!(file = fopen(filename, "r")))
	{
		/* file can't be opened.  if it's because the file doesn't
		 * exist, it isn't critical -- it means that the user doesn't
		 * have a crontab file.  just return NULL, and let our caller
		 * worry about it, or not.
		 *
		 * if it isn't ENOENT, we have a problem that the caller
		 * probably can't handle; print the error before returning.
		 */
		if (errno != ENOENT)
			perror(filename);
		return NULL;
	}

	Debug(DPARS, ("load_user()\n"))

	/* file is open.  build user entry, then read the crontab file.
	 */
	u = (user *) malloc(sizeof(user));
	u->uid   = uid;
	u->gid   = gid;
	u->envp  = env_init();
	u->crontab = NULL;

	/*
	 * do auto env settings that the user could reset in the cron tab
	 */
	sprintf(envstr, "SHELL=%s", shell);
	u->envp = env_set(u->envp, envstr);

	sprintf(envstr, "HOME=%s", dir);
	u->envp = env_set(u->envp, envstr);

	while (ERR != (status = load_env(envstr, file)))
	{
		if (status == TRUE)
		{
			u->envp = env_set(u->envp, envstr);
		}
		else
		{
			if (NULL != (e = load_entry(file, NULL)))
			{
				e->next = u->crontab;
				u->crontab = e;
			}
		}
	}

	/*
	 * do automatic env settings that should have precedence over any
	 * set in the cron tab.
	 */
	(void) sprintf(envstr, "USER=%s", name);
	u->envp = env_set(u->envp, envstr);

	/*
	 * done. close file, return pointer to 'user' structure
	 */
	fclose(file);

	Debug(DPARS, ("...load_user() done\n"))

	return u;
}
