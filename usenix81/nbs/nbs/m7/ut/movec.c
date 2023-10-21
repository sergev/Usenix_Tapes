/*  movec.c
*
*	This routine is used to move the contents of tags into the substituted
*	text and to move substituted text into the original input text.
*
*	Parameters: iarr - contains the characters to be moved.
*	   i - where the characters to be moved are.
*	   jarr - where the characters are to be stored.
*	   j - where the characters are to be put in 'jarr'.
*
*				Programmer: Tony Marriott
*/
movec(iarr,i,jarr,j,n)   /* move n characters from iarr[i] to
                            jarr[j]              */

char	*iarr,*jarr;
int     i,j,n;

{
int     k;

	for (k = 1;k <= n;k++)
	{
         jarr[j+k-1] = iarr[i+k-1];
	}
}
