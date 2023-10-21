/*  dostck.c
*
*	All types of stack calls are handled in this routine. 
*	 This routine will read what type of call it is and the stack
*	id, perform the operation, and return the location of the element of the
*	stack that was referenced.
*
*	Type 1 call: Match or replace with what's pointed to on the named stack.
*	Type 2 call: Put tag n on the named stack. (The pointer is not moved.)
*	Type 3 call: Set the pointer to the given position on the named stack.
*	Type 4 call: Place the indicated text on the indicated stack.
*
*	Arguments: pat - a pointer to the pattern; patpos - current pattern pos
*	           buf - pointer to the input text used to get the tags to put
*		         on the stack in a type 2 stack call.
*		   tagary - where the locations of the tag in the input line
*			    are stored.
*		   eose - end of stack entry location.
*
*	Calls: movec, fatal_error
*
*	UE's: If we find a illegal code for the call type, then fatal_error
*	      is executed.  This supposedly can't happen but if an error in the
*	      input has been undetected up to this point, there is a chance it
*	      could happen.
*
*					Programmers Tony Marriott & G. Skillman
*/
char	*stck[26][21];
int	stckptr[26];
dostck(pat,patpos,buf,tagary,eose)

char	*pat,*buf;
int	patpos,tagary[][2];
int	*eose; /* end of stack entry */
{
	int	from,stckid,after,inc,i,type,tagnum,numchar,num,stckpos;
	extern int	NO,YES,EOS;
	extern char	*stck[][21],STCKEND;
	extern int	stckptr[];

	from = patpos;
	i = patpos;
	stckid = 0;
	after = NO;
	inc = NO;

	++i;
	type = pat[i];
	/* Skip over type of stack call & the # chars in the call. */
	i = i + 2;
	while (pat[i] != STCKEND)
	{
		switch(type)
		{
			case 1:
			  
			  /* If + or -, take time out to decide whether to 
				increment/decrement now or later.  */
			  if (pat[i]=='+' || pat[i]=='-')
				break;
			  else
			  {
				stckid=pat[i]-'a';
				if (stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				break;
			  }
			case 2:
			  if (pat[i]=='+' || pat[i]=='-')
				break;
				/* The tag number should be the 3rd character
				   in this type of stack call.    */
			  else if (i == (from+3))
			  {
				tagnum = pat[i];
				break;
			  }
			  else
			  {
				stckid = pat[i] - 'a';
				if (stckptr[stckid] <= 0)
					stckptr[stckid] = 1;

				/* Tagary contains the locations of tagged 
				   strings in the input text. */
				numchar	= tagary[tagnum][1]-tagary[tagnum][0];

				/* Call "movec" to move the tagged part of the
				   input text onto the stack. */
				movec(buf,tagary[tagnum][0],stck[stckid][stckptr[stckid]],0,numchar);
				/* Append an EOS to the stack entry. */
				*(stck[stckid][stckptr[stckid]]+numchar) = EOS;
				break;
			  }
			case 3:
			  if (pat[i] == '+' || pat[i] == '-')
				break;
			  else if (i == (from+3))
			  {
				/* The stack name is the 3rd character in this
				   type of stack call.  Stack pointers are not
				   permitted to go bellow 1.  */
				stckid = pat[i]-'a';
				if (stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				break;
			  }
			  else
			  {
				/* num should not be > 20 as a stack pointer */
				num = pat[i];
				stckptr[stckid] = num;
				break;
			  }
			case 4:
			  if (pat[i] == '+' || pat[i] == '-')
				break;
			  if (pat[i] != ':')
			  {
              			  stckid = pat[i] - 'a';
              			  if (stckptr[stckid] <= 0)
              				  stckptr[stckid] = 1;
				  break;
			  }
			  ++i;
              		  numchar = pat[i];
              		  ++i;
              		  movec(pat,i,stck[stckid][stckptr[stckid]],0,numchar);
              		  *(stck[stckid][stckptr[stckid]] + numchar) = EOS;
			  i = i + numchar;
			  break;

			default:
			  fatal_error("DOSTCK: error in stack call");
		}	/* End switch on type of call. */

		if (pat[i] == '+')
		{
			inc = YES;
			++i;
			/* "-2" is the code put on the pattern to indicate that
			   incrementing/decrementing was to be done first. */
			if (pat[i] == -2)
			{
				stckid = pat[i+1]-'a';
				if (stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				++stckptr[stckid];
			}
			else
				after =  YES;
		}
		if (pat[i] == '-')
		{
			++i;
			inc = NO;
			if (pat[i] == -2)
			{
				stckid = pat[i+1] - 'a';
				if (stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				--stckptr[stckid];
				if (stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
			}
			else
				after = YES;
		}
		++i;
	}	/* End while ')' hasn't been found in pat. */
	stckpos = stckptr[stckid];
	if (after == YES && inc == YES)
		++stckptr[stckid];
	if (after == YES && inc == NO)
		--stckptr[stckid];
	if (stckptr[stckid] <= 0)
		stckptr[stckid] = 1;
	++i;
	/* Return the position on the pattern we can continue from when we 
	   return and also the position of the stack entry the pattern refered
	   to incase it was a type 1 stack call.  */
	*eose = i;
	return(stck[stckid][stckpos]);
}
