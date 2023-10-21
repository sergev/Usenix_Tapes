/*
 * mult.c
 * dennis bednar 08 08 85  Original creation.
 * dennis bednar 01 09 86 Added -F flag, added debug flag.
 * report bugs/suggestions etc. to dennis@rlgvax.uucp
 *
 * mult read the input (stdin or file(s)), comparing adjacent lines.
 * In the normal case, the second, and succeeding copies of repeated
 * lines are output to stdout.
 * Note that repeated lines must be adjacent, see sort(1).
 * This tool is sort of the opposite of uniq.
 *
 * -fn = use field number n in each line for the comparison, n = 1 = first.
 * Note - in the 2 lines " abc    def" and "abc  def", "abc" is field # 1,
 * and "def" is field number 2, multiple white space chars are field separators.
 *
 * -a = output 1st of multiple occurences
 * Note - this flag is very useful in conjunction with -fn flag.
 * Example: trying to find all include files which are in multiple dirs:
 * with input sorted by 1st column:
stdio.h		/usr/include
stdio.h		/tmp/junk
 * we would use both "-f1 -a" flags to print only those lines in which
 * include files were in more than one directory, but not outputing
 * those lines in which include files were in only one directory.
 *
 */

#include <stdio.h>

char	*cmd;	/* in case of error */
int	aflag;	/* 1 if -a */
int	dflag;	/* 1 if -d debug */
int	fflag;	/* 1 if -fn */
char	Fflag = '\0';	/* field separator, 0 = white space, else one char */
int	fieldnum;	/* value of # in -f# option, valid if fflag == 1 */

extern	char	*u_errmesg();

/* f/w ref */
char	*find_field();

main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i;
	FILE	*infp;


	cmd = argv[0];

	/* loop thru args, stopping at end of args or first file name */
	for (i = 1; i < argc; ++i)
		{
		if (argv[i][0] != '-')
			break;	/* found first non-option, ie 1st filename */
		if (strcmp(argv[i], "-a") == 0)
			{
			aflag=1;
			continue;
			}

		/* get debug flag */
		if (strcmp(argv[i], "-d") == 0)
			{
			++dflag;	/* enable debugging */
			printf("Debugging on\n");
			continue;	/* goto next argument */
			}

		/* get field number */
		if (strncmp(argv[i], "-f", 2) == 0)
			{
			if (fflag)
				goto usage;	/* only one -fn allowed */
			fflag = 1;
			if (argv[i][2] == '\0')
				goto usage;
			fieldnum = atoi(argv[i]+2);
			if (fieldnum <= 0)
				{
				fprintf(stderr, "%s: 'field number' must be positive\n", cmd);
				goto usage;
				}
			continue;
			}

		/* get field separator character */
		if (strncmp(argv[i], "-F", 2) == 0)
			{
			if (Fflag)
				goto usage;	/* only one -Fc allowed */
			Fflag = argv[i][2];	/* save field separator char */
			if (argv[i][2] == '\0')
				goto usage;	/* no field separator */
			continue;
			}
usage:
		fprintf(stderr, "usage: %s [-a] [-d] [-fn] [-Fc] [file ...]\n", cmd);
		fprintf(stderr, "        outputs 2nd, 3rd, ... of multiple lines\n");
		fprintf(stderr, "        -a = also output 1st one of multiple lines\n");
		fprintf(stderr, "        -d = debug\n");
		fprintf(stderr, "        -fn = use field number n to compare instead of line, 1=1st field,\n");
		fprintf(stderr, "              with white space as field separator\n");
		fprintf(stderr, "        -Fc = means use character 'c' as the field separator\n");
		exit(1);
		}

	if (i == argc)		/* no file names given */
		mult(stdin);	/* so read from stdin */
	else
		for ( ;i < argc; ++i)	/* use given file names */
			{
			infp = fopen( argv[i], "r");
			if (infp == (FILE *)NULL)
				{
				fprintf(stderr, "%s: cant open %s: %s\n", cmd, argv[i], u_errmesg());
				continue;
				}
			mult( infp );
			fclose(infp);
			}
}

/* save the lines here */

struct	t_line
	{
#define LINESIZE 2048
	char	linebuf [ LINESIZE ];
	} line [2];

/* use index for faster copy!! */
int	old = 0;		/* index of old line */
int	new = 1;	 	/* index of new line */

/* state flag to help decide actions based on state transitions */
#define S_START		0
#define S_UNIQLINE	1	/* saw 1st line or new one different than the old */
#define S_MULTLINE	2	/* saw new line which is same as the first */
int	state = S_START;

/* address of the first character in each line buffer */
#define OLDLINE line[old].linebuf
#define NEWLINE line[new].linebuf

mult( infp )
	FILE	*infp;
{
	int	isdiff;		/* 1 iff old line != new line */

	/* keep reading lines until eof */
	while (1)
		{

		/* this is not very efficient, but its the only way
		 * I could think of, otherwise main() gets ugly.
		 */

		/* read in next line from input */
		if (fgets(NEWLINE, LINESIZE, infp) == NULL)
			return;		/* EOF - no state transition */

		stripnl(NEWLINE);	/* remove ending newline from string */

		/* first time mult() is called, we must save the 1st line
		 * read as the 'oldline' for comparing against future 'newline's
		 */
		if (state == S_START)
			{
			swapline();	/* copy new line to old line */
			state = S_UNIQLINE;
			continue;		/* get next line */
			}

		/* compare the old vs new line, since needed in both states */
		/* compute it once to make code more efficient */

#define DIFF strcmp
		if (fflag)		/* compare by field ? */
			/* yes, pass the global fieldnum so that same_field()
			 * is kept modular, and reusable in other applications
			 */
			isdiff = !same_field(OLDLINE, NEWLINE, fieldnum);
		else			/* no compare entire line */
			isdiff = (DIFF(OLDLINE, NEWLINE));


		if (state == S_UNIQLINE)
			{
			if (isdiff)
				{
				swapline();
				/* stay in same state */
				}
			else
				{
				if (aflag)
					printf("%s\n", OLDLINE);
				printf("%s\n", NEWLINE);
				swapline();
				state = S_MULTLINE;
				}
			}
		else if (state == S_MULTLINE)
			{
			if (isdiff)
				{
				swapline();
				state = S_UNIQLINE;
				}
			else
				{
				printf("%s\n", NEWLINE);
				swapline();
				/* stay in multiple line state */
				}
			}
		}
}


/*
 * swap old line with new line
 * Called after read into new line, so that effect is same as copying
 * newline to old line, and discarding newline.
 */
swapline()
{
	register	int	t;	/* temp */

	t = old;
	old = new;
	new = t;
}


/*
 * return 1 iff field number 'fieldnum' (1=1st) is same in
 * old line vs. new line.
 */
same_field(oldline, newline, fieldnum)
	char	*oldline,
		*newline;
	int	fieldnum;

{
	char	*op,		/* old field ptr */
		*np;		/* new field ptr */

	op = find_field(oldline, fieldnum);
	if (dflag)		/* debug */
		{
		/* dump out the fields being compared */
		char *cp;
		printf("Old field %d = <", fieldnum);
		if (*op == '\0')	/* past last field in line */
			printf("UNDEF");
		else
			for (cp = op; *cp && !field_dlm(*cp); ++cp)
				printf("%c", *cp);
		printf("> ");
		printf("Old line = <%s>\n", oldline);
		}
	np = find_field(newline, fieldnum);
	if (dflag)
		{
		char *cp;
		printf("New field %d = <", fieldnum);
		if (*np == '\0')	/* past last field in line */
			printf("UNDEF");
		else
			for (cp = np; *cp && !field_dlm(*cp); ++cp)
				printf("%c", *cp);
		printf("> ");
		printf("New line = <%s>\n", newline);
		}

	if (*op == '\0' || *np == '\0')	/* is either field non-existent ? */
		return 0;		/* assume failed to match */

	/* compare fields until either one ends */
	/* a field ends with either a non-zero delimiter or a '\0' char */
	for ( ; *op || *np; ++op, ++np)	/* both strings not exhausted */
		{
		/* Important: Please note that field_dlm() checks for '\0' also */
		if (field_dlm(*op) && field_dlm(*np))	/* both reached end */
			return 1;	/* hit end of field */
		/* next cmp will handle case when only one field delimiter */
		if (*op != *np)		/* cmp both chars in the field */
			return 0;	/* failed to match */
		/* both matched, keep going */
		}

	/* both strings hit EOS, so matched that way */
	return 1;			/* matched */
}


/*
 * return 1 iff a field delimiter such as white space or end of string
 * a null char is always a field delimiter, because the null replaces
 * the last newline after the line has been read in.
 */
field_dlm(c)
	char	c;
{
	if (c == '\0')			/* is it a null at End of String ? */
		return 1;		/* yes, return true, because a delimiter */
	if (Fflag)			/* field separator defined ? */
		return (c == Fflag);	/* yes, see if it matches the one given */
	else				/* no, must check for white space */
		return (c == ' ' || c == '\t' || c == '\n');
}


/*
 * return ptr to 'num' nth field, 1 = first field in the buffer.
 * return ptr to '\0' if ask for a field not present
 */
char *
find_field (line, num)
	char	*line;
	int	num;
{
	char	*cp;	/* ptr to return */

	/* must ask for valid field number */
	if (num < 1)
		return (line+strlen(line));	/* '\0' */

	/* beginning of line */
	cp = line;

	while ( num-- > 0)
		{
		/* skip poss leading white space */

#define iswhite(c) ( (((c) & 0xff) == '\t') || (((c) & 0xff) == ' ') )

		if (Fflag)			/* using non-white field delimiter */
			;			/* so first char is field 1 */
		else				/* using white space field dlm */
			{
			while (*cp && iswhite(*cp))
				++cp;
			/* cp is now at '\0' EOS or 1st non-white */
			}


		/* stop if at beginning of desired field */
		if (num <= 0)
			break;

		/* else skip over this symbol to either End of String
		 * or next white space , or next delimiter.
		 */
		/* now find the last char of this symbol */
		if (Fflag)		/* non-white field delimiter */
			{
			while (*cp && !field_dlm(*cp))
				++cp;
			/* hit '\0' EOS or field delimiter */
			if (*cp)	/* fld */
				++cp;	/* so make it point to begin of next field */
			else
				;	/* don't go past end of string !!! */
			}
		else			/* white space delimiter */
			{
			while (*cp && !iswhite(*cp))
				++cp;
			/* cp points to EOS or next white space char */
			}
		}

	return cp;
}


/*
 * strip ending new line from string returned by fgets.
 * If not present as last char , then line too long.
 */
stripnl(s)
	char	*s;
{
	char	*cp;

	cp = &s[strlen(s) - 1];	/* ptr to last char of string */
	if (*cp == '\n')	/* is last char a new line */
		*cp = '\0';	/* yes, remove it */
	else
		{
		fprintf(stderr, "%s: error line <%s>... was too long\n", cmd, s);
		exit(1);
		}
}

/*
 * return the error message string using errno
 * More flexibility than perror(3).
 */
char *
u_errmesg()
{
	extern	int	errno;
	extern	int	sys_nerr;
	extern	char	*sys_errlist[];
static	char	buffer[50];

	if (errno < 0 || errno >= sys_nerr)
		{
		sprintf( buffer, "errno %d undefined (%d=max)", errno, sys_nerr);
		return(buffer);
		}

	return( sys_errlist[errno] );
}
