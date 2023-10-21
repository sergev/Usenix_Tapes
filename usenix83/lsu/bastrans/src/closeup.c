closeup(s,c)
char *s;
int c;
{
	int cnt = 0;
	int i,j;
	for (i=j=0;*(s+i)!='\0';i++) {
		if (*(s+i)!=c)
		{
			*(s+j++) = *(s+i);
			cnt=0;
		}
		else if (cnt++ == 0)
			*(s+j++) = *(s+i);
	}
	*(s+j)='\0';
}
