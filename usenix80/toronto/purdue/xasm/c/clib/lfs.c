lfs(target,string)
	char *target,*string;
{
	register char *t,*p,*s;

	t = string;
   while(1){
	s = t;
	p = target;
	for(;(*s!=*p)&&((*s!='\n')&&(*s!='\0'));*s++);
	if(((*s)== '\0')||(*s == '\n'))return(0);
	t= s + 1;		/*save stat of scan for when bomb*/
	while((*s++)==(*p++));	/*eat up char until not equal*/
	if((*--p)=='\0')return(1);
	}
}
