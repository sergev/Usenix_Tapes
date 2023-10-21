char *satoi(str, ip)
register char *str;
register int *ip;
{
register sum 0;
register char c;
char *p;

	p = str;
	while((c = *str++) >= '0' && c <= '9')
		sum = (sum*10) + (c - '0');
	*ip = sum;
	return(str-1);
}
