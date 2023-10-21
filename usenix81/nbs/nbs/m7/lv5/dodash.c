/*  dodash.c
*
*	This routine extends the dash feature found in char. classes into
* 	a string of characters.  For example, a-d is converted to abcd.
*	"Software Tools" contains the original version of this program.
*
*	Calls: index, addset
*
*	Parameters: valid - the range of characters involved. (Either
*      		    digits, lower case letters or upper case letters.)
*	   array - the string where we found the dash.
*	   i - the position of the dash in array.
*	   set - where we are expanding the characters into.
*	   j - the next position in 'set'.
*	   maxset - the size of the array 'set'.
*
*				Programmer: Tony Marriott
*/
dodash(valid,array,i,set,j,maxset)  /* expand set notation */

char	*array,*set;
char 	*valid;

int	*i,*j;
int	maxset;

{
	int	limit,junk,k;
	extern int	YES;

	(*i)++; (*j)--;

	limit = index(valid,esc(array,i));

	for (k=index(valid,set[*j]); k <= limit; k++)
	{
		junk = addset(valid[k],set,j,maxset);
	}

	return(YES);
}
