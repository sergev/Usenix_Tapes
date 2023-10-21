#ifndef lint
#ifndef NSCCS
static char sccsid[] = "%W%";
#endif
#endif

char *
dname(nam)
char *nam;
{
	register char *slash;
	register char *cp;

	/*
	 * Find last slash in nam.
	 */

	for (slash = cp = nam; *cp != '\0'; cp++)
		if (*cp == '/')
			slash = cp;

	if (slash == nam)
		nam = (*slash == '/' ? "/" : ".");
	else
		*slash = '\0';
	return(nam);
}
