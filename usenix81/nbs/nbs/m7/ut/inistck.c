/*  inistck.c
*
*	Recall that the 26 stacks have 20 elements each 50 chars. long.  The
*	actual storage space is in stckspace, access to each individual element
*	is through a corresponding pointer in stckptr.  Each stack has 1000
*	characters so the 9th entry in the 3rd stack (*stckptr [3][9]), for
*	example, is the (3*1000 + 9*50)th byte in stckspace.
*
*				Programmer: Tony Marriott
*/
inistck()
{
	extern char *stck[][21];
	extern int stckptr[];
	char stckspace[26000],i,j;

	for (i=0;i<=26;i++)
	{
		for (j=0;j<=20;j++)
			stck[i][j]=stckspace+(i*1000)+(j*50);
		/* The value of the stackpointers is never allowed to be < 1. */
		stckptr[i] = 1;
	}
}
