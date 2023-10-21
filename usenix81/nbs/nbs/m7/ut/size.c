	/*
	**     SIZE.C
	**	
	**	This routine returns the # of characters in a character array.
	**
	**			Programmer: Tony Marriott
	*/

size(array)
char	*array;
{

	long i;
	extern char	EOS,LT;

	i = 0;
	while (array[i] != EOS)
		++i;
	return(i);

}
