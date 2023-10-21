: Run this shell script with "sh" not "csh"
PATH=/bin:/usr/bin:/usr/ucb:/etc:$PATH
export PATH
all=FALSE
sed 's/^X//' <<'//go.sysin dd *' >Makefile
# Quickie makefile
CFLAGS	= -C
printf:		printf.c
	$(CC) $(CFLAGS) -o printf printf.c
# Edit appropriately
DESTPROG=/usr/local/bin/printf
DESTMAN =/usr/man/man1/printf.1
install:	printf
	cp printf $(DESTPROG)
	chmod 755 $(DESTPROG)
	strip $(DESTPROG)
	cp printf.1 $(DESTMAN)
	chmod 444 $(DESTMAN)
//go.sysin dd *
if [ x$1 = x-a ]; then
	all=TRUE
fi
echo Extracting printf.1
sed 's/^X//' <<'//go.sysin dd *' >printf.1
X.\"	@(#)printf.1	8-Jan-1987
X.\"
X.TH PRINTF 1 "8-Jan-1987"
X.AT 3
X.SH NAME
printf \- formatted print at shell command level
X.SH SYNOPSIS
X.B "printf <format-string>"
[
X.B arg1
] [
X.B arg2
] ...
X.SH DESCRIPTION
X.I Printf
duplicates \- as far as possible \- the standard C library routine of the
same name, at the shell command level. It is similar to
X.I echo,
except that it formats its arguments according to conversion specifications
given in the
X.B format-string,
before writing them to the standard output.
XFor a thorough explanation of format specifications, see the manual entry
for the printf subroutine.
X.PP
XFor the sake of perversity,
X.I printf
implements one format conversion
X.B not
supported by the standard printf subroutine: the %r conversion, which prints
integers as Roman numerals.
X.PP
As a convenience, within the
X.I format-string
and any string or character arguments,
X.I printf
converts "backslash notation" \- as defined in the ANSII draft C
standard \- into the appropriate control characters.
X.SH EXAMPLES
X.nf
X.na
X.sp 2
% printf 'Today is %s the %d of %s.\\n' Monday 1 April
Today is Monday the 1 of April.
X.sp 3
% printf 'Interesting Numbers\\n\\n\\tPie: %*.*f\\n\\tFoo: %g\\n' \\
	6 4 3.1415927 42
Interesting Numbers

	Pie: 3.1416
	Foo: 42
X.sp 3
% printf '%s %d, %r\\n' July 4 1776
July 4, MCCCCCCCLXXVI
X.sp 2
X.fi
X.ad
X.SH AUTHOR
XFred Blonder <fred@Mimsy.umd.edu>
X.SH "SEE ALSO"
echo(1), printf(3)

//go.sysin dd *
if [ `wc -c < printf.1` != 1420 ]; then
	made=FALSE
	echo error transmitting printf.1 --
	echo length should be 1420, not `wc -c < printf.1`
else
	made=TRUE
fi
if [ $made = TRUE ]; then
	chmod 644 printf.1
	echo -n '	'; ls -ld printf.1
fi
echo Extracting printf.c
sed 's/^X//' <<'//go.sysin dd *' >printf.c
#ifndef lint
static char sccsid[] = "@(#)printf.c	(U of Maryland) FLB 6-Jan-1987";
static char RCSid[] = "@(#)$Header: printf.c,v 1.4 87/01/29 20:52:30 fred Exp $";
#endif

X/*
 * Printf - Duplicate the C library routine of the same name, but from
 *	    the shell command level.
 *
 * Fred Blonder <fred@Mimsy.umd.edu>
 *
 * To Compile:
 %	cc -s -O printf.c -o printf
 *
 * $Log:	printf.c,v $
 * Revision 1.4  87/01/29  20:52:30  fred
 * Re-installed backslash-notation conversion for string & char arguments.
 * 
 * Revision 1.3  87/01/29  20:44:23  fred
 * Converted to portable algorithm.
 * Added Roman format for integers.
 * 	29-Jan-87  FLB
 * 
 * Revision 1.2  87/01/09  19:10:57  fred
 * Fixed bug in argument-count error-checking.
 * Changed backslash escapes within strings to correspond to ANSII C
 * 	draft standard.  (9-Jan-87 FLB)
 * 
 */

#include <stdio.h>
#include <sysexits.h>

X/****************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
register char *cp, *conv_spec, **argp, **ep;
char *strncpy(), *index(), *ctor();
double atof();

if (argc < 2) {
	fprintf(stderr,
		"printf: Usage: printf <format-string> [ arg1 . . . ]\n");
	exit(EX_USAGE);
	}

argp = &argv[2];	/* Point at first arg (if any) beyond format string. */
ep = &argv[argc];	/* Point beyond last arg. */

ctrl(argv[1]);	/* Change backslash notation to control chars in fmt string. */

X/* Scan format string for conversion specifications, and do appropriate
   conversion on the corresponding argument. */
for (cp = argv[1]; *cp; cp++) {
register int dynamic_count;

	/* Look for next conversion spec. */
	while (*cp && *cp != '%') {
		putchar(*cp++);
		}

	if (!*cp)	/* End of format string */
		break;
		
	dynamic_count = 0;	/* Begin counting dynamic field width specs. */
	conv_spec = cp++;	/* Remember where this conversion begins. */

	for (;*cp; cp++) {	/* Scan until conversion character. */
		char conv_buf[BUFSIZ];	/* Save conversion string here. */
		register int conv_len;	/* Length of ``conv_buf''. */

		switch (*cp) {	/* Field-width spec.: Keep scanning. */
			case '.': case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7': case '8':
			case '9':
				continue;

			case '*':	/* Dynamic field-width spec */
				dynamic_count++;
				continue;

			case 's':	/* String */
				if (&argp[dynamic_count] >= ep) {
					fprintf(stderr,
					"printf: Not enough args for format.\n"
						);
					exit(EX_USAGE);
					}

				(void) strncpy(conv_buf, conv_spec,
					conv_len = cp - conv_spec + 1);
				conv_buf[conv_len] = '\0';

				switch (dynamic_count) {
					case 0:
						ctrl(*argp);
						printf(conv_buf, *argp++);
						break;

					case 1:
						{
						register int a1;

						a1 = atoi(*argp++);
						ctrl(*argp);
						printf(conv_buf, a1, *argp++);
						}
						break;

					case 2:
						{
						register int a1, a2;

						a1 = atoi(*argp++);
						a2 = atoi(*argp++);
						ctrl(*argp);
						printf(conv_buf, a1, a2, *argp++);
						}
						break;

					}
				goto out;

			case 'c':	/* Char */
				if (&argp[dynamic_count] >= ep) {
					fprintf(stderr,
					"printf: Not enough args for format.\n"
						);
					exit(EX_USAGE);
					}

				(void) strncpy(conv_buf, conv_spec,
					conv_len = cp - conv_spec + 1);
				conv_buf[conv_len] = '\0';

				switch (dynamic_count) {
					case 0:
						ctrl(*argp);
						printf(conv_buf, **argp++);
						break;

					case 1:
						{
						register int a1;

						a1 = atoi(*argp++);
						ctrl(*argp);
						printf(conv_buf, a1, **argp++);
						}
						break;

					case 2:
						{
						register int a1, a2;

						a1 = atoi(*argp++);
						a2 = atoi(*argp++);
						ctrl(*argp);
						printf(conv_buf, a1, a2, **argp++);
						}
						break;
					}
				goto out;

			case 'd':	/* Integer */
			case 'o':
			case 'x':
			case 'u':
				if (&argp[dynamic_count] >= ep) {
					fprintf(stderr,
					"printf: Not enough args for format.\n"
						);
					exit(EX_USAGE);
					}

				(void) strncpy(conv_buf, conv_spec,
					conv_len = cp - conv_spec + 1);
				conv_buf[conv_len] = '\0';

				switch (dynamic_count) {
					case 0:
						printf(conv_buf, atoi(*argp++));
						break;

					case 1:
						{
						register int a1;

						a1 = atoi(*argp++);
						printf(conv_buf, a1, atoi(*argp++));
						}
						break;

					case 2:
						{
						register int a1, a2;

						a1 = atoi(*argp++);
						a2 = atoi(*argp++);
						printf(conv_buf, a1, a2, atoi(*argp++));
						}
						break;

					}
				goto out;

			case 'f':	/* Real */
			case 'e':
			case 'g':
				if (&argp[dynamic_count] >= ep) {
					fprintf(stderr,
					"printf: Not enough args for format.\n"
						);
					exit(EX_USAGE);
					}

				(void) strncpy(conv_buf, conv_spec,
					conv_len = cp - conv_spec + 1);
				conv_buf[conv_len] = '\0';

				switch (dynamic_count) {
					case 0:
						printf(conv_buf, atof(*argp++));
						break;

					case 1:
						{
						register int a1;

						a1 = atoi(*argp++);
						printf(conv_buf, a1, atof(*argp++));
						}
						break;

					case 2:
						{
						register int a1, a2;

						a1 = atoi(*argp++);
						a2 = atoi(*argp++);
						printf(conv_buf, a1, a2, atof(*argp++));
						}
						break;

					}
				goto out;

			case 'r':	/* Roman (Well, why not?) */
				if (&argp[dynamic_count] >= ep) {
					fprintf(stderr,
					"printf: Not enough args for format.\n"
						);
					exit(EX_USAGE);
					}

				(void) strncpy(conv_buf, conv_spec,
					conv_len = cp - conv_spec + 1);
				conv_buf[conv_len] = '\0';
				conv_buf[conv_len - 1] = 's';

				switch (dynamic_count) {
					case 0:
						printf(conv_buf,
							ctor(atoi(*argp++)));
						break;

					case 1:
						{
						register int a1;

						a1 = atoi(*argp++);
						printf(conv_buf, a1,
							ctor(atoi(*argp++)));
						}
						break;

					case 2:
						{
						register int a1, a2;

						a1 = atoi(*argp++);
						a2 = atoi(*argp++);
						printf(conv_buf, a1, a2,
							ctor(atoi(*argp++)));
						}
						break;

					}
				goto out;

			case '%':	/* Boring */
				putchar('%');
				break;

			default:	/* Probably an error, but let user
					   have his way. */
				continue;
			}
		}
	out: ;
	}

exit(EX_OK);
}

X/****************************************************************************/

X/* Convert backslash notation to control characters, in place. */

ctrl(s)
register char *s;
{
register char *op;
static int val;

for (op = s; *s; s++)
	if (*s == '\\')
		switch (*++s) {
			case '\0':	/* End-of-string: user goofed */
				goto out;

			case '\\':	/* Backslash */
				*op++ = '\\';
				break;

			case 'n':	/* newline */
				*op++ = '\n';
				break;

			case 't':	/* horizontal tab */
				*op++ = '\t';
				break;

			case 'r':	/* carriage-return */
				*op++ = '\r';
				break;

			case 'f':	/* form-feed */
				*op++ = '\f';
				break;

			case 'b':	/* backspace */
				*op++ = '\b';
				break;

			case 'v':	/* vertical tab */
				*op++ = '\13';
				break;

			case 'a':	/* WARNING! DANGER! DANGER! DANGER! */
				*op++ = '\7';
				break;

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				{	/* octal constant */
				register int digits;

				val = 0;
				(void) sscanf(s, "%3o", &val);
				*op++ = val;
				for (digits = 3; s[1] &&
					(int)index("01234567", s[1])
					&& --digits > 0;
						s++);
				}
				break;

			case 'x':	/* hex constant */
				s++;
				{
				register int digits;

				val = 0;
				(void) sscanf(s, "%3x", &val);
				*op++ = val;
				for (digits = 3; *s && s[1] &&
					(int)index("0123456789abcdefABCDEF",
									s[1])
					&& --digits > 0;
						s++);
				}
				break;

			}
	else
		*op++ = *s;

out:

*op = '\0';
}

X/****************************************************************************/

X/* Convert integer to Roman Numerals. (Have have you survived without it?) */

struct roman {
	unsigned r_mag;
	char r_units, r_fives;
	} roman[] = {
		1000, 'M', '\0',
		 100, 'C', '\0',
		  10, 'X', 'L',
		   1, 'I', 'V'
		};

char *
ctor(x)
register int x;
{
register struct roman *mp;
static char buf[BUFSIZ];
register char *cp = buf;

X/* I've never actually seen a roman numeral with a minus-sign.
   Probably ought to print out some appropriate latin phrase instead. */
if (x < 0) {
	*cp++ = '-';
	x = -x;
	}

for (mp = roman; x; mp++) {
	register unsigned units;

	units = x / mp->r_mag;
	x = x % mp->r_mag;

	if (cp > &buf[BUFSIZ-2])
		return "???";

	if (units == 9 && mp > roman) {	/* Do inverse notation: Eg: ``IX''. */
		*cp++ = mp->r_units;
		*cp++ = mp[-1].r_units;
		}
	else if (units == 4 && mp->r_fives) {
		/* Inverse notation for half-decades: Eg: ``IV'' */
		*cp++ = mp->r_units;
		*cp++ = mp->r_fives;
		}
	else {	/* Additive notation */
		if (units >= 5 && mp->r_fives) {
			*cp++ = mp->r_fives;
			units -= 5;
			}
		while (units--) {
			*cp++ = mp->r_units;
			if (cp > &buf[BUFSIZ-5])
				return "???";
			}
		}
	}

*cp = '\0';

return buf;
}

X/****************************************************************************/
//go.sysin dd *
if [ `wc -c < printf.c` != 9057 ]; then
	made=FALSE
	echo error transmitting printf.c --
	echo length should be 9057, not `wc -c < printf.c`
else
	made=TRUE
fi
if [ $made = TRUE ]; then
	chmod 644 printf.c
	echo -n '	'; ls -ld printf.c
fi
