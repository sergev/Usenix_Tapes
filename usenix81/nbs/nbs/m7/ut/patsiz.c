/*  patsiz.c
*
*	"Patsiz" can be found in "Software Tools" being the routine which 
*	returns the number of bytes the entry took in the macro file.  The
*	calling routine uses this number to skip to the next position in 
* 	the file to begin reading the next part of the pattern.
*
*	Arguments: pat - the pattern array being examined.
*	   n - the character in the pattern being examined.
*
*	UE's:  "Patsize" will return a -1 if the pattern written to the file
*	was unrecognizable.
*
*			Programmmers: T. Marriott & G. Skillman
*/

patsiz(pat,n)			/* returns size of pattern entry at pat(n) */

char	pat[];			/* pattern */
int	n;			/*  offset  */

{
	extern char	EOL,CHAR,CCL,NCCL,TAGEND,TAG,CLOSURE,BOL,STACK,CNTR;
	extern char	ANY,ALPHNU;
	int	i;

	if (pat[n] == BOL || pat[n] == EOL)
		return(1);

	if (pat[n] == CCL || pat[n] == NCCL)
		return(pat[n+1]+2);

	if (pat[n] == CLOSURE || pat[n] == ANY || pat[n] == CHAR)
		return(2);

	if (pat[n] == TAG || pat[n] == TAGEND)
		return(2);

	if (pat[n] == STACK)
		return(pat[n + 2] + 1);
	if (pat[n] == CNTR)
		return(pat[n + 2] + 1);

	 return(-1);		/*  error */
}
