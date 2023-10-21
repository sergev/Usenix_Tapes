/*	Printf.c subroutine for M6800, it uses integer multiply and divide,
 * and implements the following conversions:
 * %c - character.
 * %d - decimal, signed decimal number is produced.
 * %o - octal number with one leading zero.
 * %s - prints a null terminated string.
 * %h - hex number with 1 leading zero.
 * %? - prints the char following the % if not one of the above.
 *
 *	R. A. Hammond - June 1978
 *	W. D. Sincoskie - August 1978 (modifications)
 */

printf(format, argv)
char *format;		/* the format description string	*/
int argv;		/* the values refered to by format	*/
{
    char *strp;		/* for %s strings	*/
    int dnum;		/* for numbers		*/
    int *argp;		/* to get at the argv	*/
    int delay;

    for(delay=0; delay<6000; delay++);
    argp = &argv;

    while (*format != '\0')
	{
	if (*format != '%') putchar(*format);
	else
	    {
	    format++;
	    switch(*format)
		{
		case 'c':	/* character	*/
			putchar(*argp++);
			break;
		case 'd':	/* signed decimal	*/
			dnum = *argp++;
			if (dnum < 0)
				{
				dnum = -dnum;
				if(dnum < 0)	/* still -?	*/
				    {
				    printf("-32768");
				    break;
				    }
				putchar('-');
				}
			printd(dnum);
			break;
		case 'o':	/* octal number	*/
			printo(*argp++);
			break;
		case 'h':	/* hexadecimal number */
			printh(*argp++);
			break;
		case 's':	/* null terminated string	*/
			strp = *argp++;
			if (*strp == 0) printf("null");
			while (*strp) putchar(*strp++);
			break;
		default:	/* print character	*/
			putchar(*format);
		}
	    }
	format++;
	}
}
/* recursive integer printing routine	*/

printd(num)
int num;
{
int newnum;

if ((newnum = num/10) != 0)
	printd(newnum);

putchar((num - (newnum*10)) + '0');
}

/* recursive octal print routine	*/

printo(num)
int num;
{
if (num != 0)
	printo((num>>3) & 017777);

putchar((num & 07) + '0');
}

/* recursive hexadecimal print routine	*/

printh(num)
int num;
{
if (num != 0)
	printh((num>>4) & 07777);

num = num & 017;
if (num < 10)
	putchar(num + '0');
else
	putchar((num-10) + 'a');
}
