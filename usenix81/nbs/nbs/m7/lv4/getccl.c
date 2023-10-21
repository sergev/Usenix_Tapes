/*  getccl.c
*  	
*	This procedure is from "Software Tools" and is used to preprocess
*       character classes.  The routine writes a special symbol onto the
* 	buffer, pat, to indicate NCCL or CCL. Space is set aside in pat to
*	store the size of the class which is followed by the characters in
*	the class themselves.
*
*	Parameters:
*	   arg - the input string; i - current position on arg
*	   pat - the pattern buffer;  j - current position on pat
*
*	Calls: filset - writes the characters in the char. class to pat.
*	       addset - adds characters to pat.
*
*	UE's:   Returns 0 if the end bracket of the class is missing.
*
*				Programmer: Tony Marriott
*/
/* This following defines the maximum pattern size: */
# include	"globdefin.h"
getccl(arg,i,pat,j)		/* expand char class at arg[i]
				/* into pat[j]   */
char	*arg;
char	*pat;

int	*i;
int	*j;

{
	int	jstart,junk;
	extern char	NCCL,CCLEND,NOT,CCL,YES,NO;

	(*i)++;		/* skip over bracket */

	if (arg[*i] == NOT)
	  {
	    junk = addset(NCCL,pat,j,MAXPAT);
	    (*i)++;
	  }

	else
	    junk = addset(CCL,pat,j,MAXPAT);

	jstart = *j;
	junk = addset('0',pat,j,MAXPAT);
					/* leave room for count */
	filset(CCLEND,arg,i,pat,j,MAXPAT);

	pat[jstart] = *j - jstart - 1;

	if (arg[*i] == CCLEND)
	   return(YES);
	else
	   return(NO);
}
