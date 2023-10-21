/*
 *  beep.c  ---  demo program for the `beep()' routine.
 *
 *  Written by Jonathan C. Broome (broome@ucbvax.Berkeley) to 
 *  keep Jordan Hayes (jordan@ucbvax.Berkeley) happy - his dumb 
 *  terminal chokes on ^G !!!
 *
 *  Checks for visual bell (vb) string in termcap and uses it if
 *  found, else uses the normal ^G (\007).
 *
 *  Feel free to use this code wherever, it is in the public domain.
 */

#define isdigit(c)    ('0' <= c && c <= '9')
#define NULL  0

main ()
{
	beep ();
	exit (0);
}

beep ()
{
	char *getenv();
	char *termcap;
	char *vb;
	char vbuf[256];

	if ((termcap = getenv ("TERMCAP")) == NULL) {
		write (1, "\007", 1);
		return;
	}

	vb = vbuf;

	while (*termcap != NULL) {   /* find the vb string */
		if (strncmp(termcap, "vb=", 3) == 0) {
			termcap += 3;
			while (*termcap && *termcap != ':')
				*vb++ = *termcap++;
			break;
		}
		*termcap++;
	}
	*vb = '\0';

	if (!*vbuf)    /* didn't find a vb string */
		write (1, "\007", 1);
	else
		translate (vbuf);
}


/*
 *  A nice hokey routine to handle printing a (simple) termcap vb string.
 *  Note that it doesn't understand any of the `%' conversions, but what
 *  terminals have parameters in the vb string anyway???
 */

translate (buf)
char *buf;
{
	register char *i;
	int val = 0;

	i = buf;

	while (*i) {
		if (*i == '^') {                /* turn ^X to the real thing */
			val = *++i - '@';
			write (1, &val, 1);
			i++;
			val = 0;
		} else if (*i == '\\') {
			switch (*++i) {	        /* all the meta-escapes */
				case 'E': write (1, "\033", 1); i++; break;
				case 'r': write (1, "\r", 1); i++; break;
				case 'n': write (1, "\n", 1); i++; break;
				case 'b': write (1, "\b", 1); i++; break;
				case 'f': write (1, "\f", 1); i++; break;
				case 't': write (1, "\t", 1); i++; break;
				case '\\':write (1, "\\", 1); i++; break;
				default:        /* or octal form */
					while (*i && isdigit (*i))
						val = val * 8 + ((*i++) - '0');
					val &= 0177;   /* strip the high bit */
					write (1, &val, 1);
					val = 0;
					break;
			}
		} else                         /* normal character */
			write (1, i++, 1);
	}
}
