/*  filset.c
*
*    	This routine writes the characters in a character class to the buffer.
* 	Filset can be found almost verbatum in "Software Tools".
*
*	Parameters: delim - the terminating right bracket.
*	   array - the original macro being preprocessed.
*	   i - the current position in 'array'.
*	   set - the preprocessed macro buffer.
*	   j - the current position in 'set'.
*	   maxset - size of the array 'set'.
*
*	Calls:  dodash, index, addset, esc
*
*	UE's:	'NO' is returned if EOS is found before the delimeter, ']'.
*
*			Programmer: Tony Marriott
*/
filset(delim,array,i,set,j,maxset)  /* expand set at array(i) into
				       set[*j]. stop at delim */
char	*array,*set;
char	delim;


int	*i,*j,maxset;

{

	char	digits[11],lowalf[27],upalf[27];
	int	junk,k;
	extern char	EOS,ESCAPE,DASH;
	extern int	EOF,YES,NO;

	for (k = 0; k <= 9 ; k++)
		digits[k] = k + '0';
	for (k = 0; k <= 25 ; k++)
	{
		lowalf[k] = k + 'a';
		upalf[k] = k + 'A';
	}
	digits[10] = '\0';
	lowalf[26] = '\0';
	upalf[26] = '\0';

	for (;array[*i] != delim &&  array[*i] != EOS; (*i)++)

	  if (array[*i] == ESCAPE)
             junk = addset(esc(array,i),set,j,maxset);

	else if (array[*i] != DASH)
	     junk = addset(array[*i],set,j,maxset);

	else if ( *j <= 1 ||  array[*i+1] == EOS)  /* literal */
	     junk = addset(DASH,set,j,maxset);

	else if (index(digits,set[*j-1]) != EOF)
	       dodash(digits,array,i,set,j,maxset);

	else if (index(lowalf,set[*j-1]) != EOF)
	       dodash(lowalf,array,i,set,j,maxset);

	else if (index(upalf,set[*j-1]) != EOF)
	       dodash(upalf,array,i,set,j,maxset);

	else
	     junk = addset(DASH,set,j,maxset);

	if (array[*i] == delim)
		return(YES);
	else
		return(NO);
}
