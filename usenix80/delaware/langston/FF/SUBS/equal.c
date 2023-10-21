equal(a, b)
char    *a, *b;
{
	register char *ap, *bp;

	for (ap = a, bp = b; *ap++ == *bp; )
	    if (*bp++ == '\0')
		return(1);
	return(0);
}
