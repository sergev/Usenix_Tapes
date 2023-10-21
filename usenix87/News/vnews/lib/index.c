/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 *
 * These are the v7 index and rindex routines.  The makefile does not
 * place them it the library because every version of UN*X that is
 * supported by this release of netnews has these routines (possibly
 * named strchr/strrchr).  If you are running V6, use these routines.
 */

char *
index(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
}

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 */

char *
rindex(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}
