#if !defined(lint) && !defined(LINT)
static char rcsid[] = "$Header: env.c,v 1.3 87/05/02 17:34:00 paul Exp $";
#endif

/* $Source: /usr/src/local/vix/cron/env.c,v $
 * $Revision: 1.3 $
 * $Log:	env.c,v $
 * Revision 1.3  87/05/02  17:34:00  paul
 * baseline for mod.sources release
 * 
 * Revision 1.2  87/03/19  12:48:19  paul
 * suggestions from rs@mirror (Rich $alz):
 *    allow quotes around value string
 * 
 * Revision 1.1  87/02/13  00:26:58  paul
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


char **
env_init()
{
	extern char	*malloc();
	register char	**p = (char **) malloc(sizeof(char **));

	p[0] = NULL;
	return p;
}


char **
env_set(envp, envstr)
	char	**envp;
	char	*envstr;
{
	extern char	*realloc(), *savestr();
	register int	count, found;
	register char	**p;

	/*
	 * count the number of elements, including the null pointer;
	 * also set 'found' to -1 or index of entry if already in here.
	 */
	found = -1;
	for (count = 0;  envp[count] != NULL;  count++)
	{
		if (!strcmp_until(envp[count], envstr, '='))
			found = count;
	}
	count++;		/* for the null pointer
				 */

	if (found != -1)
	{
		/*
		 * it exists already, so just free the existing setting,
		 * save our new one there, and return the existing array.
		 */
		free(envp[found]);
		envp[found] = savestr(envstr);
		return envp;
	}

	/*
	 * it doesn't exist yet, so resize the array, move null pointer over
	 * one, save our string over the old null pointer, and return resized
	 * array.
	 */
	p = (char **) realloc(
			(char *)   envp,
			(unsigned) ((count+1) * sizeof(char **))
	);
	p[count] = p[count-1];
	p[count-1] = savestr(envstr);
	return p;
}


int
load_env(envstr, f)
	char	*envstr;
	FILE	*f;
{
	/* return	ERR = end of file
	 *		FALSE = not an env setting (file was repositioned)
	 *		TRUE = was an env setting
	 */
	char	*strcpy(), *sprintf();
	long	filepos = ftell(f);
	char	name[MAX_TEMPSTR], val[MAX_TEMPSTR];
	int	fields, strdtb();
	void	skip_comments();

	skip_comments(f);
	if (EOF == get_string(envstr, MAX_TEMPSTR, f, "\n"))
		return ERR;

	Debug(DPARS, ("load_env, read <%s>\n", envstr))

	name[0] = val[0] = '\0';
	fields = sscanf(envstr, "%[^ =] = %[^\n#]", name, val);
	if (fields != 2)
	{
		Debug(DPARS, ("load_env, not 2 fields (%d)\n", fields))
		fseek(f, filepos, 0);
		Set_LineNum(LINE_NUMBER - 1)		/* kludge */
		return FALSE;
	}

	/* 2 fields from scanf; looks like an env setting
	 */

	/*
	 * process value string
	 */
	{
		int	len = strdtb(val);

		if (len >= 2)
			if (val[0] == '\'' || val[0] == '"')
				if (val[len-1] == val[0])
				{
					val[len-1] = '\0';
					(void) strcpy(val, val+1);
				}
	}

	(void) sprintf(envstr, "%s=%s", name, val);
	Debug(DPARS, ("load_env, <%s> <%s> -> <%s>\n", name, val, envstr))
	return TRUE;
}


char *
env_get(name, envp)
	char	*name;
	char	**envp;
{
	char	*index();

	for (;  *envp;  envp++)
		if (!strcmp_until(*envp, name, '='))
			return index(*envp, '=') + 1;  /* we know it's there */
	return NULL;
}
