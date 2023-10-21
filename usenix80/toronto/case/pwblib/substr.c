char substr___[] "~|^`substr.c	1.1";
/*
	Sets aresult to be the value of as
	starting at origin and alen characters long.

  Note: The copying of as to aresult stops if either the
	specified number (alen) characters have been copied,
	or if the end of as is found.
*/

substr(as, aresult, origin, alen)
char *as, *aresult;
int origin, alen;
{
	register char *s, *result;
	register int len;

	s = as + origin;
	result = aresult;
	len = alen;
	++len;
	while (--len && (*result++ = *s++)) ;
	aresult[alen]= '\0';
	return(aresult);
}
