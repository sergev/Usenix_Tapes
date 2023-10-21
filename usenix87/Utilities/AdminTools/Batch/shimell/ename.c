#include <stdio.h>

extern char *sname();

#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

/*
 * Returns a pointer to the file type (file
 * extension) or NULL if one cannot be found.
 * Eg.
 * /usr.xyz/file.plm	returns ".plm"
 * /usr.xyz/file	returns NULL
 */

char *
ename(file)
register char *file;
{
	register char *dot = NULL;

	for (file = sname(file); *file; file++)
		if (*file == '.')
			dot = file;
	return(dot);
}
