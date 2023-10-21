/*  inicntr.c
*  	
*	Here the counters and their increments are all initialized to 1.
*
*			Programmer: Tony Marriott
*/
inicntr()
{
	extern cntr[];
	extern inc[];
	int i;

	for (i=0;i<=26;i++)
	{
		cntr[i] = 1;
		inc[i] = 1;
	}
}
