strend(str)
register char *str;
{
	while(*str++);
	return(str-1);
}
