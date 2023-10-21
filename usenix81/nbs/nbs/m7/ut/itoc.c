	/*
	**	ITOC.C
	**		
	**		This routine takes an integer and puts it in the
	**		next two characters of the array passed.
	**
	**		In this version a code was set up because neither
	**		byte is allowed to be written to the file if zero.
	**		The left byte is set to:
	**		   -100 if the right byte is zero,
	**		   - 75 if the left  byte is zero,
	**		   - 50 if both bytes are zero.
	**
	**			Original long int version by C. Haas
	**/

itoc(theint, c)
char	*c;
int	theint;

{
	int tmp;
	tmp = theint;

	/* convert the integer to two characters: */
	*(c + 1) = theint;
	theint = theint / 256;
	*c = theint;

	/* if either half is zero, write a code instead: */
	if (tmp % 256 == 0)
	{
		*c = -100;
		*(c + 1) = -50;
		if (tmp == 0)
			*c = -50;
	}
	else
		if (tmp < 256)
			*c = -75;
}
