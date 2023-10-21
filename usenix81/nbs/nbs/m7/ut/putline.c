/*  putline.c
*
*	This routine is used to write the new line after the replacement has
*	occured.
*
*	Parameters: buffer - the buffer we are writting out.
*	   where - the output file descriptor.
*	   delim - the nenext to the last character we are wring out.
*
*			Programmmer: Tony Marriott
*/
putline(buffer,where,delim)	/* Put a line of data to */
				/* device "where". Delim */
				/* is the line delimiter */
char	*buffer;
int	where,delim;
{
	int	i;
	extern int	EOF,YES;

	for (i=0;write(where,&buffer[i],1) == 1;i++)
		if (buffer[i] == delim)
		{
			return(YES);
		}
	return(EOF);
}
