index(str1, str2)
register char *str1, *str2;
{
register char *p1;

	p1 = str1;
	while(*str1){
		if(*str1 == *str2){
			if(prefix(str2,str1))
				return(str1-p1);
		}
		str1++;
	}
	return(-1);
}

prefix(s1, s2)
register char *s1, *s2;
{
	while(*s1++ == *s2++);
	if(*(s1-1) == '\0')
		return(1);
	return(0);
}
