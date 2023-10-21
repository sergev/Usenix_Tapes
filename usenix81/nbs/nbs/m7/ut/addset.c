/*  addset.c
*
*	'Addset' is from "Software Tools" and simply writes c to set
*	and increments j if there's room enough for c.
*
*	Parameters: c - the character to be added.
*	   set - the buffer we're adding 'c' to.
*	   j - where in 'set' we are adding 'c'.
*	   maxsize - the size of the array 'set'.

*			Programmer: Tony Marriott
*/
addset(c,set,j,maxsiz)		/* put c in set[j] if it fits */
				/* increment j, test with maxsiz */
char	*set;
char	c;

int	*j,maxsiz;

{
	if (*j > maxsiz) 
		return(0);
	
	else
	    { set[*j] = c;
              (*j)++;
	      return(1);
	    }
}
