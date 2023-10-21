/*  reinitlist.c
*
*	 This procedure reinitializes the linked list which contained the
*	   numbers of the macros which were only to be used once per input
*	   line.  This procedure will remark these macros to indicate that
*	   they have not been used yet.
*
*	Parameters: scanlist - the list of macro #'s with '<'.
*	   availhead - next available node in 'scanlist'.
*
*			Programmer G. Skillman
*/
# include	"globdefin.h" /* This will define the size of the "once only" list. */

reinitlist(scanlist, availhead)

int	scanlist [2][MAXSCANOFFS], availhead;

{
	int	index;
	extern int	RESCANMAC; /* This  value used in the flag field to show */
			      /* that the macro has not been used yet. */
	
	index = 1; /* get the first node of the list. */
	while (index <= availhead)  /* while not at the end of macro list: */
	{
		scanlist [1][index] = RESCANMAC;
		index = index + 1;
	}

}
