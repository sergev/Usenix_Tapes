/*  index.c
*
*	'Index' returns the index of the char in the string array
*
*	Parameters: t - the character we are looking for.
*	   s - the array of characters we are looking in.
*
*				Programmer: Tony Marriott
*/
index(s,t)	
char	*s,t;
{
	int	i;
	extern char	EOS;
	extern int	EOF;

	for (i = 0;s[i] != EOS; i++)
	{
		if ( s[i] == t)
		{
			return(i);
		}
	}
	return(EOF);
}
