/*  getprocpat.c
*
*	This routine reads the processed macro from the processed macro file
*	and calls "read" to read the macro into the buffer "pat".  The 
* 	algorithm is basically: 1) Read the size of the macro.  2) Move
*	the file pointer to the beginning of the macro.  3) Call "read".
*	4) Move the file pointer to the begining of the same macro.
*	   The macros are terminated by ";", followed by the size of the
*	macro, followed by EOS.
*
*	Calls: seek (a C-language routine); read; ctoi
*
*				Programmmer: Tony Marriott & G. Skillman
*/
getprocpat(pat,patfd)

char	*pat;
int	patfd;
{
	char	c[2];
	char	bufy[20];
	int	startofpat,next,numchar;
	extern int	EOF,TRUE;
	extern char	EOS;


	/* We must get the size of the pattern next which is
	   stored in two halves as two characters. */
	if (read(patfd,c,2) == EOF)
		return(EOF);

	/* "ctoi" will put the two halves in the integer numchar. */
	ctoi(&numchar, c);
	if (numchar > 500)
	{
		read(patfd,bufy,20);
		write(1,bufy,20);
	}
	startofpat = (numchar + 2) * -1;
	seek(patfd,startofpat,1);
	if (read(patfd,pat,numchar + 3) != numchar + 3)
	{
		return(EOF);
	}
	else
	{
		next = startofpat - 4;
		seek(patfd,next,1);
		return(TRUE);
	}
}
