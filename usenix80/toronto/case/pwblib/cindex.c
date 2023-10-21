char cindex___[] "~|^`cindex.c	1.1";
/*
	Returns offset of char c in string s.
	Returns -1 if c is not in s.
*/

cindex(s, c)
char *s, c;
{
	register char *r, a;

	a = c;
	r = s;
	while (*r)
		if (*r++ == a)
			return(r-s-1);
	return(-1);
}

