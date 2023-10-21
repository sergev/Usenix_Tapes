/*  makpat.c
*    
*      This routine is similar to the one in "Software Tools" which processes
*      the pattern portion of the macro. (Between the 1st pair of quotes. ) 
*
*      Parameters:
*         arg - the input macro which we are preprocessing.
*         argpos - the pointer to the position in arg we are looking at.
*         pat - the buffer we are putting the preprocessed macro in. 
*         patpos - where in the buffer we are currently inserting stuff.
*
*      Calls: skip, addset, getccl, procstck, proccntr, fatal_error, esc
*
*      UE's:  Delimeter or both delimeters missing from input macro.
*
*				Programmer: Tony Marriott & G. Skillman
*/
/* This defines the maximum pattern size: */

# include	"globdefin.h"
makpat(arg,argpos,pat,patpos)
int	*argpos,*patpos;
char	*arg,*pat;


{
	int	i,j,junk,head;
	char	tagstck[100], tagnum;
	extern char	ANY,BOL,EOL,CCL,CLOSURE,TAG,TAGEND,EOS,CHAR;
	extern char	STACK,CNTR,LINECONT,MACDELIM,COMMMENT,CC;
	extern int	NO;

	head = 0;
	tagnum = 1;
	j = *patpos;
	i = *argpos;

	/* Write 1st quote to the preprocessed buffer: */
	junk = addset(MACDELIM,pat,&j,MAXPAT);
	++i;
	/* while we haven't found the rightmost quote of pattern: */
	for (; arg[i] != MACDELIM && arg[i] != EOS; i++)
	  {
	    if (arg[i] == ANY && arg[i + 1] >= '1' && arg[i + 1] <= '6')
	    {
		++i;
		junk = addset(ANY, pat, &j, MAXPAT);
		junk = addset(arg[i], pat, &j, MAXPAT);
	    }
		
	   else if (arg[i] == BOL)
		   junk = addset(BOL,pat,&j,MAXPAT);

	   else if (arg[i] == EOL)
		   junk = addset(EOL,pat,&j,MAXPAT);

	   else if (arg[i] == CCL)
		   {
			if (getccl(arg,&i,pat,&j) == NO)
			   break;
		   }

	   else if (arg[i] == CLOSURE)
		{
			junk = addset(CLOSURE,pat,&j,MAXPAT);
			if (arg[i + 1] == '-')
			{
				junk = addset(1,pat,&j,MAXPAT);
				++i;
			}
			else
				junk = addset(-1,pat,&j,MAXPAT);
		}
	   
	   else if (arg[i] == TAG || arg[i] == TAGEND)
	   {
		junk = addset(arg[i],pat,&j,MAXPAT);
		/* Save a spot for the tag number. */
		if (arg[i] == TAG)
		{
			addset(tagnum,pat,&j,MAXPAT);
			tagstck[head] = tagnum;
			++head;
			tagnum = tagnum + 1;
		}
		else
		{
			--head;
			if (head < 0)
				fatal_error("MAKPAT: unbalanced tag braces");
			else
				addset(tagstck[head],pat,&j,MAXPAT);
		}
	   }
	   else if (arg[i] == LINECONT)
	   {
		 ++i;
	         skip(arg,&i,LINECONT);
	 	 --i;
	   }
	   else if (arg[i] == STACK)
		procstck(arg,&i,pat,&j);
	   else if (arg[i] == CNTR)
		proccntr(arg,&i,pat,&j);
	   else
		   {
		     junk = addset(CHAR,pat,&j,MAXPAT);
		     junk = addset(esc(arg,&i),pat,&j,MAXPAT);
		   }

	   }

	/* Add the right quote so we know where the pattern stops.  */ 
	addset(MACDELIM,pat,&j,MAXPAT);

	/* If there is stuff left on the tag stack: */
	if (head != 0)
		fatal_error("MAKPAT: unbalanced tag braces");


	/* If we encounter an EOS above without finding the delimeter: */
	if (arg[i] != MACDELIM)		/* terminated early */
	   fatal_error("MAKPAT: pattern terminated early");

	else
		/* Return the postions in the pattern and text where
		   we left off so that we can preprocess the replacement. */
	{
		*argpos = i;
		*patpos = j;
	}
}
