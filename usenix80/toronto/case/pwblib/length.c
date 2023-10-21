/*
 *	length - number of chars in string
 */

length(s)
register char *s;
{
	register n;

	n = 0;
	while (*s++)
		n++;
	return(n);
}

/*
 *	size - number of chars in string, including null byte
 */

size(s)
register char *s;
{
	return(length(s) + 1);
}
