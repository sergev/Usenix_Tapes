/*
 *	RT11 EMULATOR
 *	formatted output
 *
 *  This underdeveloped printf() exists to keep the
 *  standard I/O library out of the RT11 emulator.
 *  Only %s, %o, and %d are implemented.
 *  All output is done with putchr().
 *
 *	Daniel R. Strick
 *	July 17, 1981
 */


int	_prbase;	/* base for integer output */


/*
 *  Print an unsigned integer.
 *  The base should not exceed 10.
 */
putint (un)
unsigned int un;
{
	register unsigned int n;
	register int r;

	n = un;
	r = n % _prbase;
	n = n / _prbase;
	if (n != 0)
		putint (n);
	putchr ('0' + r);
}


/*
 *  Print the argument string.
 */
putstr (str)
char *str;
{
	register int c;
	register char *s;

	s = str;
	while ((c = *s++) != '\0')
		putchr (c);
}


/*
 *  Print the parameters in the given format.
 */
/*VARARGS1*/
printf (format, params)
char *format;
int params;
{
	register int c;
	register int *pp;
	register char *cp;

	pp = &params;
	for (cp = format;  (c = *cp++) != '\0'; ) {
		if (c != '%') {
			putchr (c);
			continue;
		}
		_prbase = 10;
		switch (c = *cp++) {

			case 'c':
				putchr (*pp++);
				break;

			case 's':
				putstr ((char *) *pp++);
				break;

			case 'd':
				if ((c = *pp++) < 0) {
					putchr ('-');
					c = -c;
				}
				putint ((unsigned) c);
				break;

			case 'u':
				putint ((unsigned) *pp++);
				break;

			case 'o':
				_prbase = 8;
				putint ((unsigned) *pp++);
				break;

			default:
				putchr ('%');

			case '%':
				putchr (c);
		}
	}
}
