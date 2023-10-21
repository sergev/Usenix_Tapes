/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"

char *
getval (name) char *name;
	{
	int	i = 0;
	for (i = 0; i < nvar; i++)
		if (!strcmp (variables[i].name, name)) break;
	if (i == nvar) return (NULL);
	return (variables[i].value);
	}

trueval (name)
	{
	char	*v = getval (name);
	if (v == NULL) return (0);
	if (*v == NULL) return (1);
	if (strlen (v) == 1)
		if (*v == 't' || *v == 'y' || *v == '1') return (1);
		else return (0);
	else if (!strcmp (v, "true") || !strcmp (v, "yes")) return (1);
	return (0);
	}

char *
interpolate (s) char *s;
	{
	static	char	interbuf[BUFSIZ];
	char	*getval ();
	char	*sptr = s;
	char	*bufptr = interbuf;
	char	namebuf[BUFSIZ], *nameptr = namebuf;
	int	i;
	*bufptr = NULL;
	while (*sptr)
		{
		if (*sptr == varchar)
			{
			nameptr = namebuf;
			sptr++;
			while (isalnum (*sptr)) *nameptr++ = *sptr++;
			*nameptr = NULL;
			strcpy (bufptr, getval (namebuf));
			while (*bufptr) bufptr++;
			}
		else if	(*sptr == escapechar)
			{
			sptr++;
			*bufptr++ = *sptr++;
			}
		else	*bufptr++ = *sptr++;
		}
	*bufptr = NULL;
	return (interbuf);
	}

#define	begingeneric(c) (c == '{')
#define	endgeneric(c)   (c == '}' || end(c))
char *
getargs (args, initial) char *args, *initial;
	{
	static	char	arglist[BUFSIZ];
	char	genericbuf[100];
	char	*generic;
	char	*arglistptr = arglist;
	*arglistptr = NULL;
	if (args == NULL) return (arglist);
	for(;;)
		{
		while (!begingeneric (*args))
			if (*args == NULL)
				{
				*arglistptr = NULL;
				return (arglist);
				}
			else *arglistptr++ = *args++;
		*arglistptr = NULL;
		generic = genericbuf;
		while (!endgeneric (*args))
			*generic++ = *args++;
		*generic++ = *args++;
		*generic = NULL;
		if (trueval ("highlight")) standout ();
		mvprintw (RESPLINE, 0, genericbuf);
		if (trueval ("highlight")) standend ();
		printw (": ");
		clrtoeol (); refresh ();
		if (initial) strcpy (response, initial);
		else *response = NULL;
		if (getresponse (response) == NULL) return (NULL);
		move (LINES-1, 0); clrtoeol (); refresh ();
		strcat (arglist, response);
		while (*arglistptr) arglistptr++;
		}
	}

char *
getresponse (s) char *s;
	{
	char	*linedit ();
	int	x, y;
	getyx (stdscr, y, x);
	return (linedit (s, y, x));
	}
