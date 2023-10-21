/*  streq.c
*
* 	This utility routine tests to see if the string buff1 occurs at
*	the beginning of buff2. 
*
*			Programmer: Tony Marriott
*/
streq(buff1,buff2)

char	*buff1,*buff2;
{
	int	i;
	extern char	EOS;
	extern int	NO,YES;

	for (i=0;buff1[i] != EOS;i++)
	{

		if (buff1[i] != buff2[i])
			return(NO);
	}
	return(YES);
}
