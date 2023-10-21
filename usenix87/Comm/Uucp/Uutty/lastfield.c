/*
** Find the last field of a string.
*/
char *
lastfield(p,c)
	char *p;	/* Null-terminated string to scan */
	int   c;	/* Separator char, usually '/' */
{	char *r;

	r = p;
	while (*p)			/* Find the last field of the name */
		if (*p++ == c)
			r = p;
	return r;
}
