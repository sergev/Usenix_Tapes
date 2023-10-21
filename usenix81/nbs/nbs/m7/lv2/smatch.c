/*  smatch.c
*
*	This routine attempts to match the pattern against some portion of the
*	input text. If a match has been found, the replacement is performed
*	and an attempt is made to match the same macro on the rest of the line.
*	Once the macro fails, the first macro in the file is read again and
*	a match is sought.  This procedure also handles the "t" and "n" options.
*
*	Calls: amatch, rewrite, putline, getprocpat, docomand, seek,
*	       docalls
*
*	Parameters:
*	   line - pointer to the input string; patfd - pattern file descriptor
*	   scanlist - list of macros with "<" feature.
*	   totnummacs - the total # macros currently in the preprocessed file.
*	   availhead - the next available node in scanlist.
*
*	UE's: See lower level procedures.
*
*				Programmer: Tony Marriott & G. Skillman
*/
/* The following defines the maximum pattern size and maximum # of macros 
   with the "<" feature. */
# include	"globdefin.h"


smatch(line,patfd,scanlist,totnummacs,availhead)
			/* rewrite as necessary  */
char	*line;
int	patfd, *totnummacs;
int	scanlist [3][MAXSCANOFFS],*availhead;

{
	extern int	tswitch,nswitch;
	extern char	MACDELIM,MACTERM,CC,STACK,CNTR;
	extern int	EOS,TRUE,FALSE,STDOUT,MACNUM;
	extern int	OFF,STDIN,EOF,EOS,YES,DONTRESCAN;
	int	begin,end,i,j,nflag,flag,index,answer,remember;
	char	pat[MAXPAT];
	/* Array of tag locations; end of stack entry; end of counter entry */
	int	tagary[100][2], eose, eoce;


	nflag = FALSE;
	index = *availhead - 1;  /* Begin at the beginning of scanlist. */

	seek(patfd,-3,2); /* begin at the end of the macro file. */

	/* if '%' is the zeroth element, process the command. */
	if (line[0] == CC)
	{
	   /* 'docomand' returns 'YES' if it was a command. */
	   answer = docomand(line,patfd,totnummacs,scanlist,availhead,&index);
	   if (answer == YES)
		return; /* Since it's a command, we don't process any further.*/
	}
	/* Do until none of the macros match anything in the input line. */
	while (getprocpat(pat,patfd) != EOF)
	{
	     flag = FALSE;
   	     /* if the macro is listed in scanlist and the macro has
   	        been processed for this input, then skip it. */
   	     if (scanlist [0][index] == MACNUM && scanlist [1] [index] == DONTRESCAN)
	     {
		     index = index - 1;
		     MACNUM = MACNUM - 1;
   	     	     continue;  
	     }
	     /* Process stack & counter calls that appear before the pattern. */
	     j = 0;
	     docalls(pat,&j,line,tagary);


	     /* Try to match the pattern starting at i. If we fail, 
                try starting at i+1:  */
             remember = j;
   	     for (i = 0;line[i] != EOS;i++)
   	     {
   	         begin = i;
      	         if ((end = amatch(line,i,pat,&j,MACDELIM,tagary)) != FALSE)

   	          /* found a match !! */

      	         {
	     	     nflag = TRUE;
	     	     flag  = TRUE;
   	             if (tswitch == TRUE)
   	             {
   	     	       printf("\n  oldline: ");
   	     	     	       putline(line,STDOUT,EOS);
   	     	       printf("  macro#: %d",MACNUM);
   	             }

		     /* Process stack & cntr calls before replacement symbol. */
                     docalls(pat,&j,line,tagary);

		     rewrite(line,begin,&end,pat,j,tagary); /* rewrite line */
	     	     i = end - 1; /* skip over replaced text */

   	     	     if (tswitch == TRUE)
	     	     {
	     	     	     printf("\n  newline: ");
   	     	     	     putline(line,STDOUT,EOS);
	     	     	     printf("\n");
	     	     }
		     if (line[0] == CC)
			/* 'docomand' returns 'YES' if it's a command. */
			if (docomand(line,patfd,totnummacs,scanlist,availhead,
				 &index) == YES)
				return;

   	         } /* end 'if' amatch succeeded at position i of input line. */

                 j = remember;

   	       } /* end 'for' each position of the input line. */

	       if (flag == TRUE)  /* if the macro matched at least once. */
	       {
 	    	     /* if the macro is not  to be rescanned for this input line: */
   	     	     if (scanlist [0][index] == MACNUM)
   	     	     	     scanlist [1][index] = DONTRESCAN;

	     	     /* Go to the end of the macro file and start over. */
	     	     index = *availhead - 1;  /* Restart at the beginning of scanlist. */
      	      	     seek(patfd,-3,2); /* go to the end of the pattern file. */
	             MACNUM = *totnummacs;
	       }
	       else  /* If no matches occur, then go to next macro. */
	       {
		     /* if this macro had "<" but didn't match: */
	             if (MACNUM == scanlist[0][index])
	                  index = index - 1;
	             MACNUM = MACNUM - 1;
	       }

	}  /* end 'while' not at end of macro file. */

	/* if we are always to reprint the input string or
	   if the input string was changed by at least one macro: */
	if (nswitch == FALSE || nflag == TRUE)
	        putline(line,STDOUT,EOS);

}
