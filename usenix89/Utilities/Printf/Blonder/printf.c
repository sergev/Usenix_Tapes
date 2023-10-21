#ifndef lint
static char sccsid[] = "@(#)printf.c	(U of Maryland) FLB 6-Jan-1987";
static char RCSid[] = "@(#)$Header: printf.c,v 1.2 87/01/09 19:10:57 fred Exp $";
#endif

/*
 * Printf - Duplicate the C library routine of the same name, but from
 *	    the shell command level.
 *
 * WARNING: Gross code. Extremely machine dependent. (Works on VAXen.)
 *
 * To Compile:
 %	cc -s -O printf.c -o printf
 *
 * $Log:	printf.c,v $
 * Revision 1.2  87/01/09  19:10:57  fred
 * Fixed bug in argument-count error-checking.
 * Changed backslash escapes within strings to correspond to ANSII C
 * 	draft standard.  (9-Jan-87 FLB)
 * 
 */

#include <stdio.h>
#include <sysexits.h>

/****************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
register char *cp;
register int *argp, *ep;
register unsigned long *sp;
char *conv_args, *sbrk(), *index();
double atof();
int conv_arg_size;
register int *tp;

if (argc < 2) {
	fprintf(stderr,
		"printf: Usage: printf <format-string> [ arg1 . . . ]\n");
	exit(EX_USAGE);
	}

argp = (int *)&argv[2];	/* Point at first arg (if any) beyond format string. */
ep = (int *)&argv[argc];	/* Point beyond last arg. */

conv_args = sbrk(0);	/* Remember current break. */

ctrl(argv[1]);	/* Change backslash notation to control chars in fmt string. */

tp = (int *)sbrk(sizeof(char *));	/* Put fmt string on arg list. */
*tp = (int)argv[1];

/* Scan format string for conversion specifications, and do appropriate
   conversion on the corresponding argument. */
for (cp = argv[1]; *cp; cp++) {

	/* Look for next conversion spec. */
	while (*cp && *cp != '%') {
		cp++;
		}

	if (!*cp)	/* % at end of string - error - ignore */
		break;

	for (cp++; *cp && *cp != '%' && (int)index("*.scdoxufeg0123456789", *cp);
									cp++) {

		if (argp >= ep) {
			fprintf(stderr, "printf: not enough args for format\n");
			exit(EX_USAGE);
			}

		switch (*cp) {	/* Field-width spec.: Keep scanning. */
			case '.': case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7': case '8':
			case '9':
				continue;

			case 's':	/* String: no conversion */
				ctrl((char *)*argp);
				tp = (int *)sbrk(sizeof(char *));
				*tp = *argp;
				argp++;
				goto out;

			case 'c':	/* Char: copy it */
				ctrl((char *)*argp);
				tp = (int *)sbrk(sizeof(int *));
				*(int *)tp = *(char *)*argp;
				argp++;
				goto out;

			case '*':	/* Dynamic field-width spec */
				tp = (int *)sbrk(sizeof(int));
				*tp = atoi((char *)*argp);
				argp++;
				continue;

			case 'd':	/* Integer */
			case 'o':
			case 'x':
			case 'u':
				tp = (int *)sbrk(sizeof(int));
				*tp = atoi((char *)*argp);
				argp++;
				goto out;

			case 'f':	/* Real */
			case 'e':
			case 'g':
				tp = (int *)sbrk(sizeof(double));
				*(double *)tp = atof((char *)*argp);
				argp++;
				goto out;
			}
		}
	out: ;
	}

/* Gross-out time! Copy converted argument list onto the stack. */
if (!(sp = (unsigned long *)alloca(conv_arg_size = (sbrk(0) - conv_args)))) {
	fprintf(stderr, "printf: Can't allocate stack space.\n");
	exit(EX_OSERR);
	}
bcopy(conv_args, sp, conv_arg_size);

printf();	/* Yes, this really does work. */

exit(EX_OK);
}

/****************************************************************************/

/* Convert backslash notation to control characters, in place. */

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

/****************************************************************************/
