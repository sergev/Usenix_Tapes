verify(str1,str2)
register char *str1, *str2;
{
register char *p1;

	p1 = str1;
	while(*str1++ == *str2++);
	return(str1-p1);
}
