imatch(prefix,str)
register char *prefix,*str;
{
	while(*prefix++ == *str++)
		if(*prefix == '\0')
			return(1);
	return(0);
}
