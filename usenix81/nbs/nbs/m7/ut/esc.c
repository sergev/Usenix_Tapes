/*  esc.c
*
*	This is the "Software Tools" routine which implements the escape feature
*	enabling the user to match a special character that would otherwise be 
*	taken to have some special meaning.  The calling routine has read the 
*	next character and wants to know what character to write to the pre-
*	processed macro file.
*	     If the character it found was the escape character it returns the
*	next character unless the next character is "EOS" in which case it is
*	assumed that the escape character is taken as a literal.
*
*	Parameters: array - the orignal macro we are examining.
*	   i - the current position in 'array'.
*
*	UE's: An error occurs if the EOS characer is encountered.
*
*			Programmer: Tony Marriott
*/
esc(array,i)	/* map array[*i] into escaped character */

char 	*array;
int	*i;

{
	extern char ESCAPE,EOS,LETN,LETT,LETN,TAB,NEWLINE;

	if ( array[*i] != ESCAPE )
		if (array[*i] != EOS)
			return ( array[*i]);
		else
			fatal_error("ESC: END OF STRING ENCOUNTERED TOO SOON");

	else if ( array[*i+1] == EOS )
	    return(ESCAPE);

	else
	    {
	     (*i)++;
	     if (array[*i] == LETN)
	       return(NEWLINE);
	     
	     else if (array[*i] == LETT)
	       return(TAB);

	     else return(array[*i]);
	    }
}
