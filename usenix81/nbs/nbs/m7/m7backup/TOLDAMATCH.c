
amatch(line,from,pat,delim)      /* look for match in pat starting
                                   at line[from]     */
char    *pat, *line,delim;
int     from;

{
	int i,j,offset,flag,end,junk[10][10],k;
	extern char CLOSURE,EOS,LT,STACK,NOTFOUND,CNTR;
	extern int OFF,NO,FALSE;
	extern char LT,EOS,MACDELIM,MACTERM;
	char buff[512];

	flag = OFF;

        offset = from;

        for( j = 0; pat[j] != delim; j = j + patsiz(pat,j))
	{
          if(pat[j] == CLOSURE)
	  {
	    i = offset;
	    flag = OFF;
	    closmatch(line,pat,&i,&j,&flag); /* match closure */
            offset = i;               /* character that caused*/
                                      /*  failure      */
	    if((line[i+1] == EOS || line[i] == EOS) && flag == OFF) return(FALSE);
          }
	  else if(pat[j] == STACK)
	  {
		i = offset;
		k = 0;
		movec(pat,j,buff,0,size(pat+j)+1);
		dostck(buff,0,line,junk);
		++k;
		while(index("_';&?^[{*%$#",buff[k]) == NOTFOUND)
		{
			if(buff[k] != line[i])
				return(FALSE);
			++k;
			++i;
		}
		if(i == offset)
			return(FALSE);
		offset = i;
	  }
	else if(pat[j] == CNTR)
	{
		i = offset;
		k = 0;
		movec(pat,j,buff,0,size(pat+j)+1);
		docntr(buff,0,line,junk);
		++k;
		while(index("_';&?^[{*%$#",buff[k]) == NOTFOUND)
		{
			if(buff[k] != line[i])
				return(FALSE);
			++k;
			++i;
		}
		if(i == offset)
			return(FALSE);
		offset = i;
	}
          else if (omatch(line,&offset,pat,j) == NO) /* non-closure */
	        return(FALSE);
	}
          /*  else omatch succeeded   */
      
          return(offset);      /* success  */
}
