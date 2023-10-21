	/*
	**   FATAL_ERROR.C
	**
	**	Print error message on terminal and die.
	**	Called with two character strings as arguments.
	**
	**			C. Haas		Nov. 8, 1979
	*/


fatal_error(c1, c2)
char *c1, *c2;
{
	extern int MACNUM;
	printf(c1, c2);
	printf("\n");
	printf("The error occured on macro # %d\n",MACNUM);
	exit(1);
}
