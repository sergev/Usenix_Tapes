/*	@(#)strlen.c	1.2	*/
/*LINTLIBRARY*/
/*
 * Returns the number of
 * non-NULL bytes in string argument.
 */

int
strlen(s)
register char *s;
{
	register n;

	n = 0;
	while(*s++)
		n++;
	return(n);
}
