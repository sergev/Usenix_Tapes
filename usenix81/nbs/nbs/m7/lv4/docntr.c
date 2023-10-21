/*  docntr.c
*
*	This routine is very similar to dostck except that it works on counters.
*	All counter calls are handled and the type of call is coded as follows:
*	   Type 1: Match or replace with the value of the counter.
*	   Type 2: Set the counter's value to n.
*	   Type 3: Set the counter's increment to n.
*
*	Parameters:
*	   pat - pointer to the pattern string; patpos - current position in pat
*	   eoce - end of counter entry postion 
*
*	Calls: itoa, fatal_error
*
*	UE's: "Fatal_error" is called in the event that an illegal code for the
*	      type of call is stored in the preprocessed pattern. This is not
*	      supposed to be able to happen but might if a user error was not
*	      caught earlier in the processing stage.
*
*				Programmer: Tony Marriott, G. Skillman
*/

int	cntr[26],inc[26];
docntr(pat,patpos,eoce)
char	*pat;
int	patpos;
int	*eoce; /* end of counter entry */
{
	extern int NO,YES;
	extern char CNTREND;
	int from,cntrid,after,incr,i,type,intnum;
	char num[100],charnum[2];

	from = patpos;
	i = patpos;
	cntrid = 0;
	after = NO;
	incr = NO;

	++i;
	type = pat[i];
	i = i + 2; /* skip over type of call & # of characters in the call. */
	while (pat[i] != CNTREND)
	{
		switch(type)
		{
			case 1:
			  /* If a + or - found, take time out to find out if
			     we are to increment/decrement now or later. */
			  if (pat[i] == '+' || pat[i] == '-')
				break;
			  else
			  {
			    	cntrid = pat[i] - 'a';
				break;
			  }
			case 2:
			  if (pat[i] == '+' || pat[i] == '-')
					break;
				/* The counter name is the second character */
			  else if (i == (from+3))
		
			  {
				cntrid = pat[i] - 'a';
				break;
			  }
			  else
			  {
				/* The new value of the cntr is in 2 halves: */
				charnum[0] = pat[i];
				charnum[1] = pat[++i];
				ctoi(&intnum,charnum);
				cntr[cntrid] = intnum;

				break;
			  }
			case 3:
			  if (pat[i] == '+' || pat[i] == '-')
				break;
			  else if (i == (from + 3))
			  {
				cntrid = pat[i] - 'a';
				break;
			  }
			  else
			  {
				/* The new value of the inc is in 2 halves: */
				charnum[0] = pat[i];
				charnum[1] = pat[++i];
				ctoi(&intnum,charnum);
				inc[cntrid] = intnum;
				break;
			  }
			default:
			{
				printf("pat[i] -- %c type -- %d\n",pat[i],pat[i]);
				fatal_error("DOCNTR: error in counter call");
			}
		}
		if (pat[i] == '+')
		{
			incr = YES;
			++i;
			if (pat[i] == -2)
			{
				cntrid = pat[i+1] - 'a';
				cntr[cntrid] = cntr[cntrid] + inc[cntrid];
			}
			else
				after = YES;
		}
		if (pat[i] == '-')
		{
			++i;
			incr = NO;
			if (pat[i] == -2)
			{
				cntrid = pat[i+1] - 'a';
				cntr[cntrid] = cntr[cntrid] - inc[cntrid];
				if (cntr[cntrid] <= 0)
					cntr[cntrid] = 1;
			}
			else
				after = YES;
		}
		++i;
	}
	if (type == 1)
		/* "itoa" will convert the counter to a string of characters
		   placed in "num". A pointer will be returned so that num
		   can be matched against or inserted into the text.  */
		itoa(cntr[cntrid],num);
	if (after == YES && incr == YES)
		cntr[cntrid] = cntr[cntrid] + inc[cntrid];
	if (after == YES && incr == NO)
		cntr[cntrid] = cntr[cntrid] - inc[cntrid];
	if (cntr[cntrid] <= 0)
		cntr[cntrid] = 1;
	++i;
	/* We return the next position in the pattern and a pointer to num. */
	*eoce = i;
	return(num);
}
