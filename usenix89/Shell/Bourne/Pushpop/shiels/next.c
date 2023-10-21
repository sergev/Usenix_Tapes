/*
 * NEXT.C:
 *
 * Skip to the next delimiter seperated object
 *
 */

#define iswhite(c) ( (c) == ' ' || (c) == '\t' || (c) == '\n' )


char	*next( linep, delim, esc )
char	**linep;
{
	/* Linep is the address of a character pointer that points to
	 * a string containing a series of delim seperated objects.
	 * Next will return a pointer to the first non-white object in
	 * *linep, replace the first delimiter it finds with a null, and
	 * advance *linep to point past the null (provided that it's not
	 * at end of string). 0 is returned when an empty string is passed
	 * to next(). White space may be used as a delimiter but
	 * in this case whitespace won't be skipped. A delimiter preceeded
	 * by "esc" is ignored. Quoted strings are copied verbatum.
	 */ 

	register char	*start, *end ;
	int		inquote = 0;

	if( !**linep )
		return 0;

	start = *linep;

	if( !iswhite(delim) )
		for( ; iswhite(*start) ; start++ )
			;

	for( end = start; *end && (*end != delim || inquote) ; end++ )
	{
		if( *end == esc  &&  *(end+1) )
			end++;

		 else if( *end == '"' || *end == '\'' )
			inquote = ~inquote;
	}

	if( *end )
		*end++ = '\0';

	*linep = end;
	return start;
}
