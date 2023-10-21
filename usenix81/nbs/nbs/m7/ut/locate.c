/*  locate.c
*
*	"Locate.c" is contained in the book "Software Tools".
*	"YES", 1, is returned if the character is found, "NO", 0, otherwise.
*
*	Parameters: c - the character we are looking for.
*	   pat - the array we are lookng in.
*	   offset - the location in pat of the size of the character class.
*
*			Programmer: Tony Marriott
*/
locate(c,pat,offset)		/* look for c in char class at pat(offset)*/

char c;			/* character to be looked for */
char *pat;		/* pattern to be searched */
int offset;		/* offset within pattern */

{

	int	i;
	extern int	YES,EOF,NO;

	for (i=offset + pat[offset]; i > offset; i--)
	     {
		if ( c == pat[i])  return(YES);
	     }
	return(NO);
}
