int cntr[26],inc[26];
docntr(pat,patpos)
char *pat;
int patpos;
{
	extern int NO,YES;
	extern char CNTREND;
	int from,cntrid,after,incr,i,type;
	char num[100];

	from = patpos;
	i = patpos;
	cntrid = 0;
	after = NO;
	incr = NO;

	++i;
	type = pat[i];
	++i;
	while(pat[i] != CNTREND)
	{
		switch(type)
		{
			case 1:
			  if(pat[i] == '+' || pat[i] == '-')
				break;
			  else
			  {
			    	cntrid = pat[i] - 'a';
				break;
			  }
			case 2:
			  if(pat[i] == '+' || pat[i] == '-')
					break;
			  else if(i == (from+2))
		
			  {
				cntrid = pat[i] - 'a';
				break;
			  }
			  else
			  {
				cntr[cntrid] = pat[i];
				break;
			  }
			case 3:
			  if(pat[i] == '+' || pat[i] == '-')
				break;
			  else if(i == (from + 2))
			  {
				cntrid = pat[i] - 'a';
				break;
			  }
			  else
			  {
				inc[cntrid] = pat[i];
				break;
			  }
			default:
			{
				printf("pat[i] -- %c type -- %d\n",pat[i],pat[i]);
				fatal_error("DOCNTR: error in counter call");
			}
		}
		if(pat[i] == '+')
		{
			incr = YES;
			++i;
			if(pat[i] == -2)
			{
				cntrid = pat[i+1] - 'a';
				cntr[cntrid] = cntr[cntrid] + inc[cntrid];
			}
			else
				after = YES;
		}
		if(pat[i] == '-')
		{
			++i;
			incr = NO;
			if(pat[i] == -2)
			{
				cntrid = pat[i+1] - 'a';
				cntr[cntrid] = cntr[cntrid] - inc[cntrid];
				if(cntr[cntrid] <= 0)
					cntr[cntrid] = 1;
			}
			else
				after = YES;
		}
		++i;
	}
	if(type == 1)
	{
		itoa(cntr[cntrid],num);
		stuffit(num,pat,from+1,i+1);
	}
	if(after == YES && incr == YES)
		cntr[cntrid] = cntr[cntrid] + inc[cntrid];
	if(after == YES && incr == NO)
		cntr[cntrid] = cntr[cntrid] - inc[cntrid];
	if(cntr[cntrid] <= 0)
		cntr[cntrid] = 1;
	++i;
	return(i);
}
