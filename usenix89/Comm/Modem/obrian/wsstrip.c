/*	wsstrip.c						*/
/*	Program to strip out funny characters from a wordstar	*/
/*	document file kermited over in binary format.		*/
/*								*/
/*	Written March 19, 1985 by John Paul O'Brien		*/
/*		for Nova University Computer Center		*/
/*								*/


#include <stdio.h>

main()
{
char	ch;

	while ( (ch = getchar()) != EOF )	/* get all chars from stdin */
	{
		ch &= 0x7f;			/* strip off all high bits  */
		if ( (ch != '\r') &&		/* see if not cr or ^Z	    */
		     (ch != 0x1a) )		/* If not, then candidate   */
						/* for output.		    */
		{
			if ( (ch < ' ') &&	/* see if is a control char */
			     (ch != '\n') &&	/* excluding lf,	    */
			     (ch != '\t') &&	/* excluding tab,	    */
			     (ch != 0x07) )	/* and bell		    */
			{			/* if control character but */
						/* not one of the exclusions */
						/* then don't output it	    */
			}
			else
			{
				putchar(ch);	/* if ok then put to stdout */
			}
		}
	}
}
