/*  maksub.c
*  
*	This routine is similar to the one found in "Software Tools" with
*	the addition of tags, stacks and counters.  The purpose here is to pre-
*	process the replacement part of the input macro and then write it
*	to the buffer "sub" begining at "subpos".
*
*	Parameters:
*	   arg - the input macro; argpos - where in arg the replacement part is.
*	   sub - the buffer we are writing to; subpos - where we are writing.
*
*	Calls:
*          addset, procstck, procntr, fatal, fatal_error, skip, atoi
*
*	UE's:  Error conditions include a missing left or right "'" and
*	       not enough space existing in the buffer to hold the 
*	       processed macro.
*
*			Programmers: T. Marriott and G. Skillman
*/
/* The following defines the maximum pattern size: */
# include	"globdefin.h"

maksub(arg,argpos,sub,subpos)
char	*arg,*sub;
int	*argpos,*subpos;
{
	int	i,j,junk;
	extern char	TAG,EOS,LINECONT,MACTERM,MACDELIM,CC;
	extern char	STACK,CNTR,ESCAPE;
	extern int	FALSE,NO;
	char	tagnum;

	i = *argpos;
	j = *subpos;

	/* Add the third quote to the preprocessed buffer: */
	junk = addset(MACDELIM, sub, &j, MAXPAT);
	++i;

	/* For all the characters until we find the terminating semicolon */
	for (;arg[i] != MACDELIM && arg[i] != EOS;i++)
	{
	  if (arg[i] == TAG)
	  {
		junk = addset(TAG,sub,&j,MAXPAT);
		++i;
		tagnum = atoi(&(arg[i]));
		junk = addset(tagnum,sub,&j,MAXPAT);
		while (arg[i] >= '0' && arg[i] <= '9')
			++i;
	  }
	  else if (arg[i] == LINECONT)
	  {
	    ++i;
	    skip(arg,&i,LINECONT);
	    --i;
	  }
	  else if (arg[i] == STACK)
		procstck(arg,&i,sub,&j);
	  else if (arg[i] == CNTR)
		proccntr(arg,&i,sub,&j);

		/* if ESCAPE is encountered, we want to escape the character
		   but we also want to write ESCAPE so that the character
		   will be escaped a second time when read by "rewrite". */
	  else 	if (arg[i] == ESCAPE)
	  {
		/* write ESCAPE, then the character to be escaped. */
		sub[j++] = arg[i++];
		/* i will be incremented by the loop. */
		sub[j++] = arg[i];
	  }
	  else
		junk = addset(arg[i],sub,&j,MAXPAT);
	}

	if (arg[i] == MACDELIM)
		junk = addset(arg[i++],sub,&j,MAXPAT);

	for (;arg[i] != MACTERM && arg[i] != EOS;++i)
		if (arg[i] == STACK)
			procstck(arg,&i,sub,&j);
		else if (arg[i] == CNTR)
			proccntr(arg,&i,sub,&j);
		else if (arg[i] == LINECONT)
		{
			++i;
			skip(arg,&i,LINECONT);
			--i;
		}

	addset(MACTERM,sub,&j,MAXPAT);

	if (arg[i] != MACTERM)
	    fatal_error("MAKSUB: substitution text terminated early");


             /* if addset says there wasn't enough room, then stop. */
	else if (addset(EOS,sub,&j,MAXPAT) == NO) /* no room */
	    fatal_error("MAKSUB: preprocessed macro too large");
	else
	{
		*argpos = i;
		*subpos = j - 1;
	}
}
