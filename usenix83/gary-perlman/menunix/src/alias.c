/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
char *
alias (s, delim) char *s, *delim;
	{
	static	char	aliased[BUFSIZ];
	char	*alias = aliased;
	char	name[BUFSIZ], *nameptr = name;
	char	*getval (), *val;
	struct	stat status;
    checkalias:
	*alias = NULL;
	while (isspace (*s)) *alias++ = *s++;
	nameptr = name;
	*nameptr = NULL;
	while (isalnum (*s) || *s == '_')
		*nameptr++ = *s++;
	*nameptr = NULL;
	if (val = getval (name)) /* has a value */
	    if ((stat (val, &status) == 0) /* can get status */
		&& ((status.st_mode & S_IFMT) == S_IFDIR)) /* directory file */
		    strcpy (alias, name);
	    else strcpy (alias, val);
	else
	    strcpy (alias, name);
	while (*alias) alias++;
	while (*s)
		if (index (delim, *alias++ = *s++)) goto checkalias;
	*alias = NULL;
	return (aliased);
	}
