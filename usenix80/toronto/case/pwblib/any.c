any(c, str)
char c, *str;
{
	while(*str)
		if(c == *str++)
			return(1);
	return(0);
}

anystr(s1, s2)
register char *s1, *s2;
{
char *p1;

	for(p1 = s1; *s1; s1++)
		if(any(*s1,s2))
			return(s1-p1);
	return(-1);
}
