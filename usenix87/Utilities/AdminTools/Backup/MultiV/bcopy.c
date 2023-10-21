#ifdef vax
bcopy(from, to, len)
	char *from, *to;
	int len;
{
	asm("	movc3	12(ap),*4(ap),*8(ap)");
}
#else 
#ifdef	SYSV
/*
 * Added by moderator, bcopy.c for Sys V.
 *
 */

extern char *memcpy();

char *
bcopy(from, to, count)
	char *from;
	char *to;
	int count;
{
	return (memcpy(to, from, count));
}
#else
bcopy(from, to, len)
	register char	*to, *from;
	register int	len;
{
	if (from > to) {
		while(len--)
			*to++ = *from++;
	} else {	/* not handled in standard version of bcopy() */
		to	+= len;
		from	+= len;
		while(len--)
			*--to = *--from;
	}
}
#endif
#endif
