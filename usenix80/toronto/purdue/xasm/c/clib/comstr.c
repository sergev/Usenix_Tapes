comstr(s1,s2)
	char *s1,*s2;
{
	register char *p,*m;

	p= s1;
	m= s2;

	while((*p++ == *m++)&&(*(p-1) != '\0'));
	if((*--p == '\0')&&(*--m == '\0'))return(1);
	return(0);
}
