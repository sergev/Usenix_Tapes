/*  getline.c
*
*	The macro is read from the file in this routine and stored in 
*	the array "buffer".  The file is read character by character until
*	the delimeter EOS is encountered. If EOS is encountered and the 
*	previous character was ESCAPE, then the next EOS is searched for.
*
*	Parameters: buffer - where we are reading into.
*		    where - the input file descriptor.
*		    delim - the last character we want to read (unless escaped).

*				Programmer: Tony Marriott
*/
# include	"globdefin.h"
getline(buffer,where,delim)	
char	*buffer,delim;
int	 where;
{
	int	i;
	extern int	EOF;
	extern char	ESCAPE;

	for (i = 0;read(where,&(buffer[i]),1) == 1;i++)
	{
	   	if (buffer[i] == delim)
		{
			if (buffer[i-1] == ESCAPE);
			else
			{
				buffer[++i] = '\0';
				if (i > MAXPAT)
				   fatal_error("GETLINE: input string too long");
				return(1);
			}
		}
	}

	return(EOF);
}
