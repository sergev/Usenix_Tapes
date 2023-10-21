/*
 * unixname(), getname(), getpath()
 */

#include <stdio.h>
#include <ctype.h>

/*
 * Get rid of spaces in a MSDOS 'raw' name (one that has come from the
 * directory structure) so that it can be used for regular expression
 * matching with a unix file name.  Also used to 'unfix' a name that has
 * been altered by fixname().  Returns a pointer to the unix style name.
 */

char *
unixname(name, ext)
char *name;
char *ext;
{
	static char *ans;
	char *s, tname[9], text[4], *strcpy(), *strcat(), *malloc();
	char *strchr(), *strncpy();

	strncpy(tname, name, 8);
	s = strchr(tname, ' ');
	if (s != NULL)
		*s = NULL;
	tname[8] = NULL;

	strncpy(text, ext, 3);
	s = strchr(text, ' ');
	if (s != NULL)
		*s = NULL;
	text[3] = NULL;

	if (*text != NULL) {
		ans = malloc(strlen(tname)+1+strlen(text)+1);
		strcpy(ans, tname);
		strcat(ans, ".");
		strcat(ans, text);
	}
	else {
		ans = malloc(strlen(tname)+1);
		strcpy(ans, tname);
	}
	return(ans);
}

/*
 * Get name component of filename.  Translates name to upper case.  Returns
 * pointer to new name.  Could be NULL.
 */

char *
getname(name)
char *name;
{
	char *s, *malloc(), *strcpy(), *strrchr();
	static char *temp;

	if (*name == NULL)
		return(NULL);

	temp = malloc(strlen(name)+1);
	strcpy(temp, name);
				
	if (s = strrchr(temp, '/'))
		temp = s+1;
	if (s = strrchr(temp, '\\'))
		temp = s+1;

	for (s = temp; *s; ++s) {
		if (islower(*s))
			*s = toupper(*s);
	}
	return(temp);
}

/*
 * Get the path component of the filename.  Translates to upper case.
 * Returns pointer to path or NULL if no path exist.  Doesn't alter
 * leading separator, always strips trailing separator (unless it is the
 * path itself).
 */

char *
getpath(name)
char *name;
{
	char *s, *s1, *malloc(), *strcpy(), *strrchr();
	static char *temp;

	if (*name == NULL)
		return(NULL);

	temp = malloc(strlen(name)+1);
	strcpy(temp, name);

	s = strrchr(temp, '/');
	s1 = strrchr(temp, '\\');
					/* if both are NULL , no path */
	if (s1 == s)
		return(NULL);
					/* zap which ever is last separator */
	if (s1 > s)
		*s1 = NULL;
	if (s > s1)
		*s = NULL;
					/* translate to upper case */
	for (s = temp; *s; ++s) {
		if (islower(*s))
			*s = toupper(*s);
	}
					/* if null, put separator back */
	if (!strlen(temp))
		temp = "/";
	return(temp);
}
