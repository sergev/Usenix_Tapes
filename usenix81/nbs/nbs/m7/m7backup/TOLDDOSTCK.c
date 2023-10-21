char *stck[26][50];
int stckptr[26];
dostck(pat,patpos,buf,tagary)

char *pat,*buf;
int patpos,tagary[][10];
{
	int from,stckid,after,inc,i,type,tagnum,numchar,num;
	extern int NO,YES;
	extern char *stck[][50],STCKEND;
	extern stckptr[];

	from = patpos;
	i = patpos;
	stckid = 0;
	after = NO;
	inc = NO;

	++i;
	type = pat[i];
	++i;
	while(pat[i] != STCKEND)
	{
		switch(type)
		{
			case 1:
			  if(pat[i]=='+' || pat[i]=='-')
				break;
			  else
			  {
				stckid=pat[i]-'a';
				if(stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				break;
			  }
			case 2:
			  if(pat[i]=='+' || pat[i]=='-')
				break;
			  else if(i == (from+2))
			  {
				tagnum = pat[i];
				break;
			  }
			  else
			  {
				stckid = pat[i] - 'a';
				if(stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				numchar = tagary[tagnum][2]-tagary[tagnum][1];
				movec(buf,tagary[tagnum][1],stck[stckid][stckptr[stckid]],0,numchar);
				*(stck[stckid][stckptr[stckid]]+numchar)='\0';
				break;
			  }
			case 3:
			  if(pat[i] == '+' || pat[i] == '-')
				break;
			  else if(i == (from+2))
			  {
				stckid = pat[i]-'a';
				if(stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				break;
			  }
			  else
			  {
				num = pat[i];
				stckptr[stckid] = num;
				break;
			  }
			default:
			  fatal_error("DOSTCK: error in stack call");
		}

		if(pat[i] == '+')
		{
			inc = YES;
			++i;
			if(pat[i] == -2)
			{
				stckid = pat[i+1]-'a';
				if(stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				++stckptr[stckid];
			}
			else
				after =  YES;
		}
		if(pat[i] == '-')
		{
			++i;
			inc = NO;
			if(pat[i] == -2)
			{
				stckid = pat[i+1] - 'a';
				if(stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
				--stckptr[stckid];
				if(stckptr[stckid] <= 0)
					stckptr[stckid] = 1;
			}
			else
				after = YES;
		}
		++i;
	}
	if(type == 1)
	{
		stuffit(stck[stckid][stckptr[stckid]],pat,from+1,i+1);
	}
	if(after == YES && inc == YES)
		++stckptr[stckid];
	if(after == YES && inc == NO)
		--stckptr[stckid];
	if(stckptr[stckid] <= 0)
		stckptr[stckid] = 1;
	++i;
	return(i);
}
