/*
** vn news reader.
**
** strtok.c - strtok() and strpbrk() string routines using UCB index().
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>

char *strpbrk (s,del)
char *s, *del;
{
	char *ptr,*index();
	if (s == NULL)
		return (NULL);
	for (; *del != '\0'; ++del)
		if ((ptr = index(s,*del)) != NULL)
			return (ptr);
	return (NULL);
}

char *strtok(str,delim)
char *str, *delim;
{
	char *tokstart, *tokend, *first_ch (), *last_ch();
	static char *save=NULL;

	if (str != NULL)
		save = str;

	if (save == NULL)
		return (NULL);

	tokstart = first_ch (save, delim);
	tokend = last_ch (tokstart, delim);
	save = first_ch (tokend, delim);
	*tokend = '\0';

	if (*tokstart == '\0')
		return (NULL);

	return (tokstart);
}

static char *first_ch (str,delim)
char *str,*delim;
{
	char *index ();
	char *f;

	for (f = str; *f != '\0' && index(delim,*f) != NULL; ++f)
		;

	return (f);
}

static char *last_ch (str,delim)
char *str,*delim;
{
	char *index ();
	char *f;

	for (f = str; *f != '\0' && index(delim,*f) == NULL; ++f)
		;

	return (f);
}
