/*  closmatch.c
*
*	This subroutine originated in "Software Tools" but has been completely
*	altered due to the additions of stacks, counters and tags.
*
*	     This routine is actually recursive since it is called by "amatch"
*	and it calls "amatch" itself.  The reason "amatch" is called here is
*	demonstrated by this example:
*		pattern:  'Prokofieff' = 'whatever';
*	        input  :  'P*ff'       = 'is here.';
*	Note that we will fail if we take the shortest closure because 'f'
*	does not match 'i'.  However, if we use the 2nd 'f' in the pattern
*	as our delimeter, we can succeed.
*	   "Closmatch" will attempt the first one, then call "amatch" to see
*	if the rest of the pattern matches.  If it doesn't, then it will try
*	using the next occurence of the closure delimeter as the delimeter
*	and then call "amatch" again to see if that one worked.
*/
closmatch(line,pat,i,j,flag)	/* match a closure entry in pat */

char *line,*pat;
int *i,*j,*flag;
{
	extern char EOS,LT,MACDELIM,EOL,TAG,TAGEND;
	extern int YES,FALSE,ON;
	/* "lasti" contains the previous value of "i" before "omatch" is 
	   called.  It is then used to see if "i" has been incremented
	   upon return from "omatch".  If it wasn't, then "i" is incremented
	   by this subprogram.
	   
	   "tagpos" enables us to remember where on the pattern we are to store
	   the location of the tag on the input text. The location of the 
	   begining of the tag is stored in pat[tagpos[begin]], the location
	   of the end in pat[tagpos[end]]. The variables "begin" and "end"
	   simply mean the 0th and 1st dimensions respectively. */
	int lasti,tagpos[2];
	int begin = 0;
	int end = 1;

	lasti = -1;
	tagpos[begin] = -1; tagpos[end] = -1;


	/* skip over closure entry */
	++(*j);

	if(pat[*j] == TAGEND)
	{
	  tagpos[end] = *j + 1;
	  *j = *j + patsiz(pat,*j);
	}
	if(pat[*j] == TAG)
	{
	  tagpos[begin] = *j + 1;
	  *j = *j + patsiz(pat,*j);
	}

	for(;line[*i] != EOS;)
        {
  	  if(lasti == *i) ++(*i);
	  lasti = *i;
	  if(omatch(line,i,pat,*j) != YES) continue;
	  if(amatch(line,*i-1,&pat[*j],MACDELIM) == FALSE && pat[*j] != EOL)
	    continue;
  
	  /* found delimiting character !! */

          *flag = ON;

	  /* set up tag positions */

	  if( tagpos[begin] >= 0)
	  {
		/* if tag started at the beginning of the line, then we store
		   a -10 because 0 means end of string.   */
		if((*i-1) == 0)
			pat[tagpos[begin]] = -10;
		else
			pat[tagpos[begin]] = *i - 1;
	  }
	  if(tagpos[end] >= 0) 
		pat[tagpos[end]] = *i - 1;

	  lasti = *i;
	  if(pat[*j] == EOL)
  	  {
	    if(tagpos[begin] >= 0)
	    {
	      pat[tagpos[begin]] = *i;
	      break;
	    }
	    else if(tagpos[end] >= 0)
  	    {
	      pat[tagpos[end]] = *i;
  	      break;
  	    }
  	    else break;
  	  }
	  if(lasti == *i);
	  else --(*i);
	  break;

	}
}
