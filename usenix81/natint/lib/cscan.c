/*
 * Scan a character from a string
 * decoding the standard syntax: \nnn
 * and \b, \n, \p, \r, \s, \t
 * (where \p is a form feed or page eject
 * and \s is a space).
 * Return the number of characters consumed
 * from the string.
 */
cscan(str,cp) char *str, *cp;{
	register char *s;
	register int i, j;

	s= str;
	if((*cp= *s++)=='\\')
		switch(*cp= *s++){
			case '\0': s--; break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				for(i= *cp-'0', j=2; j>0 && *s>='0' && *s<='7'; j--)
					i= (i<<3) + *s++ -'0';
				*cp= i;
				break;
			case 'b': *cp= '\b'; break;
			case 'n': *cp= '\n'; break;
			case 'p': *cp= '\014'; break;
			case 'r': *cp= '\r'; break;
			case 's': *cp= ' '; break;
			case 't': *cp= '\t'; break;
			} 
	return s-str; }
