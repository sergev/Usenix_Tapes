/*
 * getopt(), a verson of getopt for those who do not have it and do not
 * want to steal the source from those that do.
 *
 * Same as getopt(3).
 *	Returns the next option letter in argv that matches a letter in optstr.
 *	Options are no longer parsed if the special option '--' is found
 *	or an argument that does not start in '-'.
 *	The global optind is set to the next index in argv to be processed.
 *	The global optarg is set to the argument, if the option has one.
 *	The global opterr can be set true if error messages are to be printed
 *	on the standard error file, or false if no message to be printed.
 *
 * ARGUMENTS:
 *	argc, argv - the argument count, and argument list
 *	optstr     - the list of valid options. The character ":" following
 *		     an option letter indicates that this option must have an 
 *		     argument. For example "abc:". This implies that the -:
 *		     is not  a valid option.
 *
 * RETURNS:
 *	Returns the next option letter, or EOF when all done. If an error
 *	encountered then the character '?' is returned.
 *
 * John Weald
 */
#include <stdio.h>

/*
 * Index into error array.
 */
#define BAD_OPT 0	/* option letter not in optstr			*/
#define MIS_ARG 1	/* option must have an argument			*/

char *optarg;
int optind = 1;		/* argv[0] is program name			*/
int opterr = 1;		/* If true print error message			*/

/*
 *
 * The basic data structures are optind, and the pointer "p."
 * optind keeps track of the next index into argv to parse arguments.
 * p is used to walk along the argv items looking for option letters or
 * arguments, when it is NULL the next  argv must be used . 
 * p is always left pointing to the previous option or NULL.
 * Consider the three equivalent valid argv's:
 *		1        2         3
 *		-a	 -b	   eric
 *	        -ab	eric
 *		-aberic
 * 
 */

int
getopt(argc, argv, optstr)
int argc;
char *argv[];
char *optstr;		/* The list of valid options			*/
{
	extern void err();	/* Forward reference			*/

	static char *p = (char *)NULL;

	/*
	 * parsed all the options in this argv[]?
	 */
	if (p == NULL || *++p == '\0' )
	{
		if (optind == argc)
			return(EOF);
		p = argv[optind];

		/*  a '-' by itself is not an option (e.g. see paste(1)) */
		if (*p++ != '-' || *p == '\0') 
			return(EOF);

		/* '--' marks the end of the option list.  */
		if (*p == '-')
		{
			optind++;
			return(EOF);
		}
	}

	optind++;
	/*
	 * Look for a valid option 
	 */
	while (*p != *optstr)
	{	
		if (*optstr == '\0')
		{
			/* Reached end of optstr, option not there */
			err(argv[0], BAD_OPT, *p);
			return((int)'?');
		}
		if (*++optstr == ':')
			optstr++;
	}

	/* If it does not need an argument then we are done.  */
	if (*(optstr + 1) != ':')
		return((int)*optstr);
	
	/*
	 * If there are more characters in this argv then it must
	 * be the argument.
	 */
	if (*++p != '\0')
	{
		optarg = p;
		p = (char *)NULL;
		return((int)*optstr);
	}

	/* 
	 * It needs an argument, but no more argv's left
	 */
	if (optind == argc)
	{
		err(argv[0], MIS_ARG, *optstr);
		p = (char *)NULL;
		return((int)'?');
	}

	/*
	 * Must be in next argv.
	 */
	optarg = argv[optind++];
	p = (char *)NULL;
	return((int)*optstr);
}

static void
err(a0, e, c)
char *a0;		/* argv[0]. i.e. the program name		*/
int e;
char c;
{
#ifdef NO_STDIO
	static char *errors[] = {
		": Illegal option -- ",
		": option requires an argument -- "
	};
	static char eend[] = "c\n";
	

	if (opterr)
	{
		(void)write(2, a0, strlen(a0));
		(void)write(2, errors[e], strlen(errors[e]));
		eend[0] = c;
		(void)write(2, eend, strlen(eend));
	}
#else
	static char *errors[] = {
		"%s: Illegal option -- %c\n",
		"%s: option requires an argument -- %c\n"
	};

	if (opterr)
		fprintf(stderr, errors[e], a0, c);
#endif
}
