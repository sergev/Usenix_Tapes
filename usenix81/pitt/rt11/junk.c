/*
 *	RT11 EMULATOR
 *	miscellanea
 *
 *	Daniel R. Strick
 *	April 30, 1979
 */


copy (from, to, limit)
char *from;
char *to;
char *limit;
{
	register char *f, *t;

	f = from;
	t = to;
	while (t < limit)
		*t++ = *f++;
}


clear (start, limit)
char *start;
char *limit;
{
	register char *s;

	s = start;
	while (s < limit)
		*s++ = 0;
}


/*
 *  Return the radix50 code for the argument
 *  character or -1 if the character is not
 *  from the radix50 character set.
 */
radix50 (ac)
{
	register int c;

	c = ac;
	if (c >= 'a'  &&  c <= 'z')
		c += 'A' - 'a';
	if (c >= 'A'  &&  c <= 'Z')
		return (c-'A' + 001);
	if (c >= '0'  &&  c <= '9')
		return (c-'0' + 036);
	switch (c) {
		case ' ':
			return(000);
		case '$':
			return(033);
		case '?':
			return(034);
		case '%':
			return(035);
	}
	return (-1);
}


/*
 *  Return the character that has
 *  the argument radix50 character code.
 */
char r50txb[]	= " abcdefghijklmnopqrstuvwxyz$?%0123456789";

r50toa (ac)
unsigned int ac;
{
	return (r50txb[ac]);
}
