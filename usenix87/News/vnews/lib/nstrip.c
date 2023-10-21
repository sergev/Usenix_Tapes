/*
 * Strip trailing newlines, blanks, and tabs from 's'.
 * Return 1 if newline was found, else 0.
 */
nstrip(s)
register char *s;
{
	register char *p;
	register int rc;

	rc = 0;
	p = s;
	while (*p)
		if (*p++ == '\n')
			rc = 1;
	while (--p >= s && (*p == '\n' || *p == ' ' || *p == '\t'));
	*++p = '\0';
	return(rc);
}
