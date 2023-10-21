	/*
	**	CTOI.C
	**
	**		Converts 2 characters bytes into the integer they 
	**		represent when combined.
	**
	**		In this version, a code is set up because neither byte
	**		can be allowed to equal zero. The 1st byte was set to:
	**		   -100 if the right byte was zero
	**		   - 75 if the left  byte was zero
	**		   - 50 if both bytes were zero.
	**
	**		G. Skillman 	June 17, 1980
	*/
ctoi(theint, charac)

int	*theint;
char	*charac;
{
	int	shftspace;
	if (*charac < 0)
		if (*charac == -100)
			*(charac + 1) = 0;
		else if (*charac == -75)
			*charac = 0;
		else if (*charac == -50)
		{
			*charac = 0;
			*(charac + 1) = 0;
		}
	*theint	= 0;
	*theint	= *charac;
	*theint	= *theint << 8;
	shftspace = *(charac + 1);
	if (shftspace < 0)
	{
		*theint	= *theint + 0377;
		*theint	= *theint & shftspace;
	}
	else
		*theint	= *theint | shftspace;
}
