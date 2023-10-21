zeropad(str)
register char *str;
{
char *p;

	for(p = str; *str == ' '; str++)
		*str = '0';
	return(p);
}
