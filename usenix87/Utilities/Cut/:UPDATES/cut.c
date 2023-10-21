/*
 *  This acts the same as SV cut(1), except that the list of numbers
 *  does not have to be assending.
 *
 *  John Weald
 */
#include <stdio.h>
#include <ctype.h>

#define MAXLINE	1024		/* The max. length of a line		*/
#define DEBUG	0

extern void exit();

main(argc, argv)
int argc;
char *argv[];
{
	extern char *optarg;
	extern int optind;

	static int fields[MAXLINE];/* The field markers. True if this field */
				   /* is to be cut, False otherwise	*/
	FILE *fp;		   /* All the input files or stdin	*/
	char buf[MAXLINE];	   /* The input buffer			*/
	int c;			   /* The command line option		*/
	int err = 0;		   /* True if error in command line	*/
	int fflag = 0;		   /* True if -f on command line	*/
	int cflag = 0;		   /* True if -c on command line	*/
	int suppress = 0;	   /* Suppress lines with no delimitor	*/
	char fs = '\t';		   /* The field separator		*/


	while ((c = getopt(argc, argv, "f:d:c:s")) != EOF)
	{
		switch (c)
		{
		   case 'f':
			/* By Field */
			list(fields, optarg);
			fflag++;
			if (cflag)
				err++;
			break;

		   case 'c':
			/* By character */
			list(fields, optarg);
			/* Implied suppress */
			suppress++;
			cflag++;
			if (fflag)
				err++;
			break;

		   case 'd':
			/* A new field spererator */
			fs = *optarg;
			break;

		   case 's':
			suppress++;
			break;

		   default:
			prusage();
		}
	}

	if (!cflag && !fflag)
	{
		fprintf(stderr, "Must have one of -f or -c\n");
		err++;
	}
	if (err)
		prusage();


	/*
	 * Loop on all the files.
	 */
	do {
		if (optind == argc)
			fp = stdin;
		else if ((fp = fopen(argv[optind], "r")) == (FILE *)NULL)
		{
			fprintf(stderr, "Failed to open file %s\n", 
					argv[optind]);
			exit(1);
		}

		/*
		 * Loop on all lines in the file.
		 */
		while (fgets(buf, sizeof(buf), fp) != (char *)NULL)
		{
			cut(buf, fields, fs, suppress, cflag);
		}
		(void)fclose(fp);
	} while (++optind < argc);

	exit(0);
/* NOTREACHED */
}

/*
 * Cut the line. This handles both character and field cutting.
 * For character cutting the f array gives character positions, for
 * fields it gives the field number. It must be indexed by either the
 * character number or the field number.
 */
cut(in, f, fs, sup, c_or_f)
register char *in;		/* The input line			*/
int f[];			/* The field cutting flags		*/
char fs;			/* The field seperator			*/
int sup;			/* Suppress lines with no-delimitor?	*/
int c_or_f;			/* Cut by char. (true), or field (false)*/
{
	char obuf[MAXLINE];	/* Output buffer			*/
	register char *optr = obuf;
	register int i;		/* Character count			*/
	register int fld;	/* The field count			*/
	char *instart = in;	/* To print lines with no delimiters	*/

 	if (DEBUG)	printf("c_or_f=%d\n",c_or_f);	
	for (fld = 0, i = 0; i < MAXLINE; i++)
	{
		if (*in == '\n')
		{
			/* End of the line */

			*optr = '\0';
			/* Any thing to cut? */
			if (optr != obuf)
			{
				/* Get ride of trailing seperator */
				if (*(optr - 1) == fs)
					*(optr - 1) = '\0';
				puts(obuf);
			}
			else if (!sup)
				printf(instart);
			return;
		}
		if (DEBUG)   printf("%d",f[c_or_f ? i : fld]);
		if (f[c_or_f ? i : fld])
		{
			*optr++ = *in;
		}

		/* End of field? */
		if (*in++ == fs)
			fld++;
	}

	fprintf(stderr, "Line too long, maximum length is %d\n", MAXLINE);
	exit(1);
}

/*
 * Parse the list argument. The format is:
 *	n,n
 * where n is either a number or a range of numbers in the format
 *	m-l
 * m or l  may be absent, indicating the start or end of the lines respectivly.
 * Numbers must be in increasing order for m-l format, but not for n,n.
 * Field numbers start at 1, but index into fields array starts at 0.
 * 
 */
list(f, l)
int f[];		/* The fields				*/
char *l;		/* The list				*/
{
	int range = 0;	/* True if m-l format			 */
	int low, high;	/* the low and high numbers in a m-l pair*/
	int noop,i;

	low = high = 0; noop = 1;

	while (1)
	{
		switch(*l)
		{
		   case '\0':
			if (low > 0 && noop) f[low-1]++;
			return;

		   case ',':
			if (noop) noop--;
			l++;
		       	if (!range) f[low-1]++;
			else {
				if (isdigit( (int)*l ))
				{
					f[ (low=atoi(l))-1 ]++;
					while(isdigit( (int)*l ) ) l++;
				}
			}
			range = 0;
			low = 1;
			break;

		   case '-':
			l++;
			if (noop) noop--;
			range++;
			/* Is it m-<nothing> */
			if (isdigit((int)*l))
			{
				high = atoi(l);
				/* Skip the digits */
				while (isdigit((int) *l))
					l++;
			}
			else
				high = MAXLINE;

			/* Is the range the correct way around? */
			if (low > high)
			{
				fprintf(stderr, "Bad c/f list: %d > %d\n", 
							low, high);
				exit(1);
			}
			/* Set the field flags for the range */
			for(i = low - 1; i < high; i++)
				f[i]++;
			break;

		    default:
			/* either a number or an error */
			if (!isdigit((int) *l))
			{
				fprintf(stderr, "Bad c/f list at %s\n", l);
				exit(1);
			}
			low = atoi(l);
			if (low == 0)
			{
				fprintf(stderr, "Fields start at 1 not 0\n");
				exit(1);
			}
			/* Skip the digits */
			while (isdigit((int) *l))
				l++;
			break;
		}
	}
}

prusage()
{
	fprintf(stderr, "cut [-d<delimitor>] [-s] -c<list>|-f<list> [files]\n");
	exit(1);
}
