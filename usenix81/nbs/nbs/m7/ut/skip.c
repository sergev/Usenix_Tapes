	/*	SKIP.c
	**
	**	This routine will skip through the array
	**	until delim is found. "i" will be 1 plus
	**	this position.
	**
	**			Programmmer: Tony Marriott
	*/

skip(array,i,delim)

char	*array,delim;
int	*i;
{
	extern char	ESCAPE;

	while (array[*i] != delim && array[*i-1] != ESCAPE)
	{
	  ++*i;
	}
	++*i;
}
