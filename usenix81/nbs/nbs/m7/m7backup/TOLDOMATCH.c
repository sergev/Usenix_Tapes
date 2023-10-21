
omatch(line,i,pat,j)   /* match a single pattern at pat[j] */

char *line;		/* input line */
char *pat;		/* pattern */
int  *i;		/* offset in line */
int j;			/* offset in pat */

{
	int bump,junk,k,end;		/* marker on line */
	extern char CHAR,BOL,ANY,EOS,EOL,CCL,NCCL,TAG,TAGEND,LT;
	extern int TRUE,FALSE,NO,YES;

	{if (line[*i] == EOS ) return(NO);}

	bump = -1;

	if(pat[j] == CHAR)
	  {if (line[*i] == pat[j+1] ) bump = 1;}

	else if (pat[j] == BOL)
	  {if (*i == 0) bump = 0;}

	else if (pat[j] == ANY)
	   {if(line[*i] != EOS && line[*i] != LT) bump = 1;}

	else if (pat[j] == EOL )
	  {if(line[*i] == EOS || line[*i] == LT) bump = 0;}

	else if (pat[j] == CCL)
	   {if(locate(line[*i],pat,j+1) == YES) bump = 1;}

  	else if(pat[j] == NCCL )
	{
	  if(line[*i] != LT && locate(line[*i],pat,j+1) != YES)
		bump = 1; 
	}

	/* new addition to handle tags */

	else if(pat[j] == TAG || pat[j] == TAGEND)
	{
		if(*i == 0)
			pat[j+1] = -10;
		else
			pat[j+1] = *i;
		bump = 0;
	  }

	else
	{
		  printf("pat[j] -- %c",pat[j]);
		 fatal_error("OMATCH: Can't happen");
	}

	if(bump >= 0)
	  {
	    *i = *i + bump;
		
	    return(TRUE);		/* have a match */

          }

	else
	{
	  return(FALSE);
	}
}
