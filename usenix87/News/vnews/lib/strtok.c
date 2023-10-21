/*
 * Get next token from string s1 (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from s2.  Writes NULs into s1 to end tokens.  s2 need not
 * remain constant from call to call.
 */

#define	NULL	0

static char *scanpoint = NULL;

char *				/* NULL if no token left */
strtok(s1, s2)
char *s1;
register char *s2;
{
	register char *scan;
	char *tok;
	register char *scan2;

	if (s1 == NULL && scanpoint == NULL)
		return(NULL);
	if (s1 != NULL)
		scan = s1;
	else
		scan = scanpoint;

	/*
	 * Scan leading delimiters.
	 */
	for (; *scan != '\0'; scan++) {
		for (scan2 = s2; *scan2 != '\0'; scan2++)
			if (*scan == *scan2)
				break;
		if (*scan2 == '\0')
			break;
	}
	if (*scan == '\0') {
		scanpoint = NULL;
		return(NULL);
	}

	tok = scan;

	/*
	 * Scan token.
	 */
	for (; *scan != '\0'; scan++) {
		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
			if (*scan == *scan2++) {
				scanpoint = scan+1;
				*scan = '\0';
				return(tok);
			}
	}

	/*
	 * Reached end of string.
	 */
	scanpoint = NULL;
	return(tok);
}
