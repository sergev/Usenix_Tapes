/*  rewrite.c
*
*	With this routine, we make the substitution in the original text.
*       Note that a buffer is used to create the new text. When the
*	new text is made, "stuffit" is called to insert the new text in 
*	place of the section of the old which was matched.
*
*	Arguments: 
*	   line - the input text; begin - where in line we are to begin
*	   replacement;  end - where in line we are to stop replacing
*	   pat - the pattern which contains the replacement text and the
*	   positions in line that are tagged.
*
*	Calls: dostck, docntr, ctoi, movec, stuffit, esc
*	
*	UE's: None
*                                Programmer: Tony Marriott & G. Skillman
*/
/* The following defines the maximum size of an input line: */
# include	"globdefin.h"
rewrite(line,begin,end,pat,patpos,tagary)   /* rewrite line[begin] to line[end]
				           according to specification of pat */
char	*line,*pat;
int	begin,*end,patpos;
int	tagary[100][2];
{
	extern char	EOS,MACTERM,MACDELIM,TAG,TAGEND,STACK;
	extern char	CNTR;
	extern int	RESCAN,OFF,ON;
	char	buff[MAXLINE],*string;
	char	*dostck(),*docntr();
	char	charac[2];
	int	i,j,k,m,n,eose,eoce;
	int	tagnum,numchar,tmp;

	j = 0; k = 0;m = 0;
	i = patpos;
	/* While we haven't reached the end of the replacement text. */
	/* (While we haven't found the rightmost single quote.) */
	for (;pat[i] != MACDELIM;i++)
	{
	  if (pat[i] == TAG && pat[i+1] >= 1 && pat[i+1] <= 99 )

	  /* We have a tag so get the tag's text and write it to buf. */

	  {
		tagnum = pat[++i];
		numchar = tagary[tagnum][1] - tagary[tagnum][0];
		movec(line,tagary[tagnum][0],buff,k,numchar);
		k = k + numchar;
	  }
	  else if (pat[i] == STACK)
		  if (pat[i + 1] == 1)
		  {
			/* Process a type 1 stack call. */
			n = 0;
			string = dostck(pat,i,line,tagary,&eose);
			/* "string" points to the stack entry. Now copy it to buf. */
			while (string[n] != EOS)
			{
				buff[k] = string[n];
				++n;
				++k;
			}
			i = eose-1;
		  }
		  else
		  {
			dostck(pat,i,line,tagary,&eose);
			i = eose - 1;
		  }
	  else if (pat[i] == CNTR)
		  if (pat[i + 1] == 1)
		  {
			/* Type 1 counter call. Copy the string returned onto buf. */
			n = 0;
			string = docntr(pat,i,&eoce);
			while (string[n] != EOS)
			{
				buff[k] = string[n];
				++n;
				++k;
			}
			i = eoce-1;
		  }
		  else
		  {
			docntr(pat,i,&eoce);
			i = eoce - 1;
		  }
	  else

	  /* else do a verbatim replacement */
		buff[k++] = esc(pat,&i);
	}
	/* While we haven't reached the semi-colon of the macro, process
	   type 2 and 3 stack and counter calls. */
	for (;pat[i] != MACTERM;)
	{
		if (pat[i] == STACK)
		{
			dostck(pat,i,line,tagary,&eose);
			i = eose;
		}
		else if (pat[i] == CNTR)
		{
			docntr(pat,i,&eoce);
			i = eoce;
		}
		else
			++i;
	}
	buff[k++] = EOS;

	/* put replacement text in "line" */

	stuffit(buff,line,begin,end);
}
