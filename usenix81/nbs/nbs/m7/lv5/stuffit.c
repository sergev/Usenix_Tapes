/*  stuffit.c
*
*	'stuffit' inserts text between begin and end and shrinks or widens
*	the hole if it is too small or too large.
*
*	Arguments: buff1 - the string of text we are inserting.
*		   buff2 - the string of text we are inserting onto.
*		   begin - where we are starting; end - where we will end
*
*		   Note that 'end' is altered to point towards the end of
*		   the hole.
*
*	Calls: size, movec
*
*				Programmer: Tony Marriott
*/
stuffit(buff1,buff2,begin,end) /* stuffs the contents of buff1 */
			       /* into buff2 between array     */
			       /* positions begin to end       */
char	*buff1,*buff2;
int	begin,*end;
{
	int	bufsize,hole,hangover;
	int	underhang,i;
	int	endofbuf;

	bufsize = size(buff1);
	hole = *end - begin;

	if (bufsize > hole)
	
	/* not enough room; expand buff2 */

	{
	  endofbuf = size(buff2) - 1;
	  hangover = bufsize - hole;
	  for (i = endofbuf+1; i >= *end ;i--)
		buff2[i+hangover] = buff2[i];
	  movec(buff1,0,buff2,begin,bufsize);
	  *end = *end + hangover;
	}

	else if (bufsize < hole)

	/* too much room; shrink buff2 */

	{
	  endofbuf = size(buff2) - 1;
	  underhang = hole - bufsize;
	  for (i = *end; i <= endofbuf+1 ;i++)
		buff2[i-underhang] = buff2[i];
	  movec(buff1,0,buff2,begin,bufsize);
	  *end = *end - underhang;
	}

	else 
	
	/* just right; stick it in */

	  movec(buff1,0,buff2,begin,bufsize);
}
