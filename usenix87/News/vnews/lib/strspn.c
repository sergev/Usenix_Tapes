/*
 * strspn - find length of initial segment of s1 consisting entirely
 * of characters from s2
 */

int
strspn(s1, s2)
char *s1;
char *s2;
{
	register char *scan1;
	register char *scan2;
	register int count;

	count = 0;
	for (scan1 = s1; *scan1 != '\0'; scan1++) {
		for (scan2 = s2; *scan2 != '\0'; scan2++)
			if (*scan1 == *scan2)
				break;
		if (*scan2 == '\0')
			return(count);
		count++;
	}
	return(count);
}
