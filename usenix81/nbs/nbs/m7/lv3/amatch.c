/*  amatch.c
*
*	This routine has been called to attempt to match the pattern beginning
* 	at position "from" of the input text.  "Omatch" will see if each char-
*	acter in the text pairs up with the pattern while "amatch" will 
*	recursively handle occurences of closures in the pattern.
*
*	Arguments:
*	   line - pointer to the input text string; from - position on the text
*	          we are attempting to match the pattern.
*	   pat - pointer to the pattern we are trying to match; 
*	   patpos - where the leftmost quote of the macro is.
*	   delim -  the rightmost "'" of the pattern. 
*
*	Calls: amatch, omatch, patsiz
*
*	UE's: None.
*
*				Programmers: T. Marriott & G. Skillman
*				8/1/80
*/

amatch(line,from,pat,patpos,delim,tagary)      
char    *pat, *line,delim;
int     from, *patpos, tagary[100][2];

{
	int	i, j, offset, end, k, prevpatpos, temp, patend;
	int	nonefndflag, sizlinentry;
	extern char	TAG,TAGEND,CLOSURE,EOS,LT,NOTFOUND;
	extern int	OFF,NO,FALSE;
	extern char	MACDELIM,MACTERM;

	j = *patpos;
	k = 0;
        offset = from;
	prevpatpos = 0;
	patend = 0;  /* Patend is used to send to recursive calls of amatch. */

	/* This flag is used when 'omatch' finds that the input doesn't match
	   the pattern but the next pattern construction is '*-' meaning that
	   we might have a match because of the '-' feature. */
	nonefndflag = 1;

        for (; pat[j] != delim; j = j + patsiz(pat,j))
	{
          if (pat[j] == CLOSURE)
	  {
		for (i = offset; line[i] != EOS;)
		   /* match as many as possible. */
		   if (omatch(line,&i,pat,prevpatpos,tagary,&sizlinentry) == NO) 
			  break;		        
		/* If we're at the end of the pattern, return. */
		j = j + 2;
		if (pat[j] == delim)
		{
			*patpos = j + 1;
			return(i);
		}


		/* i now points to the character that made us fail. 
		   Try to match the rest of the pattern against the 
		   rest of the input. Shrink closure by 1 'line entry' 
		   after each failure. */ 

		/* First we must set 'offset' which is the limit on how much
		   we can shrink the closure depending on whether it is a
		   '*' or '*-' type of closure. -1 and 1 were stored in the
		   next byte of the pattern respectively.  */
		if (pat[j - 1] == -1)    /* Remember zero can't be stored in */
			pat[j - 1] = 0;  /* the pattern file because it's EOS */

		/* If either pat[j-1] or nonefndflag is zero, then don't change
		   offset since it's not '*-' or else '*-' didn't match. */
		offset = offset - pat[j - 1] * nonefndflag * sizlinentry;

		for (; i >= offset; i = i - sizlinentry)
		{
		    k = amatch(line, i,  &pat[j], &patend, delim, tagary);
		     if (k > 0) /* if successful match of the rest of pattern. */
		         break;

		}
		offset = k;
		j = j + patend - 1; /* Move marker to end of pat. */
		nonefndflag = 1;
		break;   /* Break to return 'offset' to 'smatch'. */

          }  /* end: if closure. */

	  else   /* else if not a closure: */
		/* "omatch" will try to match this character (and also handle
		   stack and counter calls etc. ).  If it fails, it will return
		   "NO".    */
	  {
		
	      /* We don't want to repeat a tag character if the next
	         pattern turns out to be a closure entry. Instead we
		 want to repeat what came before the tag character. */
	       if ((pat[j] != TAG) && (pat[j] != TAGEND))
			prevpatpos = j;

               if (omatch(line,&offset,pat,j,tagary,&sizlinentry) == NO)
	       {
			/* Test for '*-' feature; pattern may still match. */
			temp = j + patsiz(pat,j);
			if (pat[temp] == CLOSURE && pat[temp + 1] == 1)
				nonefndflag = 0; /* Mark as none found here. */
			else
				return(FALSE); /* failed on non-closure */
		}
		/*  else omatch succeeded   */
	  }  /* end if closure */
	}  /* end for every pattern construction in the pattern. */
	*patpos = j + 1;
        return(offset);      /* success if offset > 0  */
}
