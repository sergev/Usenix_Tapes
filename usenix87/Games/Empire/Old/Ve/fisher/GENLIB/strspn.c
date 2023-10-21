strspn(s, cset)
char	*s;
char	*cset;
{
	char *s1, *s2;

	for( s1 = s; *s1 != '\0'; s1++ ) {
		for( s2 = cset; *s2 != '\0' && *s2 != *s1; s2++ )
			;
		if( *s2 == '\0' )
			break;
	}
	return(s1 - s);
}
