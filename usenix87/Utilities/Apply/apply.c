/*
	apply -- apply a command to a set of arguments

	last edit:	85/10/20	D A Gwyn

Usage:
	$ apply[ -a<c>][ -<n>][ -v][ -w] <command> <args ...>

	Runs the named <command> repeatedly, <n> (default 1) successive
	arguments from <args ...> passed to <command> each time.

	Special case:  If <n> is 0, runs <command> as if <n> were 1, but
	does not pass any arguments to the <command>.

	Special kludge:  If <command> contains <c> (default %) followed
	by a digit from 1 through 9, <n> is ignored and the <c><digit>
	pairs are replaced by the corresponding remaining unused <args>,
	consuming as many <args> as the largest <digit> in <command>.

	Added by DAG:  The -v (verbose) option prints each constructed
	command before executing it.  If -w is specified, commands that
	return nonzero exit status will cause a warning message.

Credit:
	This is a public domain implementation of the 8th Edition UNIX
	command of the same name, working entirely from the manual spec.
*/
#ifndef	lint
static char	sccsid[] = "@(#)apply.c	1.1";
#endif

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>

extern void	exit();
extern int	atoi();

#define	MAX_CMD_LEN	BUFSIZ		/* allowed command size */

static int	verbose = 0;		/* set for command printout */
static int	warn = 0;		/* set for failure warnings */
static char	c = '%';		/* magic substitution prefix */
static int	n = 1;			/* real arguments per command */
static int	inc = 1;		/* args consumed per command */
static char	*command;		/* command template */
static char	buf[MAX_CMD_LEN+1];	/* shell command built here */
static int	errors = 0;		/* 1 if any execution errors */

#define	ToDigit( c )	((c) - '0')	/* convert char to int digit */

static void	LeftOver(), PutBuf(), Usage();


main( argc, argv )
	int	argc;
	char	*argv[];
	{
	/* process options */

	while ( --argc > 0 && (*++argv)[0] == '-' )
		if ( (*argv)[1] == 'v' && (*argv)[2] == '\0' )
			verbose = 1;		/* "-v" */
		else if ( (*argv)[1] == 'w' && (*argv)[2] == '\0' )
			warn = 1;		/* "-w" */
		else if ( (*argv)[1] == 'a' )	/* "-a<c>" */
			{
			/* wish we could use getopt() here */

			if ( (c = (*argv)[2]) != '\0'
			   && (*argv)[3] != '\0'
			  || (c == '\0'
			   && (--argc <= 0
			    || (c = (*++argv)[0]) == '\0'
			    || (*argv)[1] != '\0'
			      )
			     )
			   )	{
				(void)fprintf( stderr,
					       "apply: bad -a value\n"
					     );
				Usage();
				}
			}
		else if ( isdigit( (*argv)[1] ) )	/* "-<n>" */
			{
			if ( (n = atoi( &(*argv)[1] )) > 0 )
				inc = n;
			/* else inc = 1; */
			}
		else	{
			(void)fprintf( stderr,
				       "apply: unknown option \"%s\"\n",
				       *argv
				     );
			Usage();
			}

	/* save command template */

	if ( --argc < 0 )
		{
		(void)fprintf( stderr, "apply: missing command\n" );
		Usage();
		}

	command = *argv++;

	/* execute constructed command repeatedly */

	while ( argc > 0 )		/* unconsumed <args> remain */
		{
		register char	*cp;	/* -> <command> template */
		register char	*bp;	/* -> construction buffer */
		register int	max = 0;	/* maximum <digit> */

		/* copy <command> template into construction buffer,
		   looking for and performing <c><digit> substitution */

		for ( bp = buf, cp = command; *cp != '\0'; ++cp )
			if ( *cp == c	/* <c> matched, try <digit> */
			  && isdigit( cp[1] ) && cp[1] != '0'
			   )	{
				register char	*ap;	/* -> arg[.] */
				register int	digit;	/* arg # */

				++cp;	/* -> <digit> */

				if ( (digit = ToDigit( *cp )) > max )
					max = digit;

				if ( digit > argc )
					LeftOver( argc );

				for ( ap = argv[digit - 1];
				      *ap != '\0';
				      ++ap
				    )
					PutBuf( bp++, *ap );
				}
			else
				PutBuf( bp++, *cp );

		if ( max > 0 )		/* substitution performed */
			{
			argc -= max;
			argv += max;
			}
		else	{
			register char	*ap;	/* -> arg[.] */
			register int	i;	/* arg # - 1 */

			if ( inc > argc )
				LeftOver( argc );

			/* append <n> arguments separated by spaces */

			for ( i = 0; i < n; ++i )
				{
				PutBuf( bp++, ' ' );

				for ( ap = argv[i]; *ap != '\0'; ++ap )
					PutBuf( bp++, *ap );
				}

			argc -= inc;
			argv += inc;
			}

		PutBuf( bp, '\0' );	/* terminate string */

		/* execute constructed command */

		if ( verbose )
			(void)fprintf( stderr, "apply: %s\n", buf );

		if ( system( buf ) != 0 )
			{
			errors = 1;
		
			if ( warn )
				(void)fprintf( stderr,
					       "apply: \"%s\" failed\n",
					       buf
					     );
			}
		}

	return errors;
	}


static void
PutBuf( bp, ch )			/* store character in buf[] */
	register char	*bp;		/* -> buf[.] */
	int		ch;		/* character to be stored */
	{
	if ( bp > &buf[MAX_CMD_LEN] )
		{
		(void)fprintf( stderr, "apply: command too long\n" );
		exit( 1 );
		}

	*bp = ch;
	}


static void
LeftOver( argc )			/* warn about LeftOver args */
	int	argc;			/* how many left over */
	{
	(void)fprintf( stderr,
		       "apply: %d arg%s left over\n",
		       argc,
		       argc == 1 ? "" : "s"
		     );
	exit( 1 );
	}


static void
Usage()					/* print usage message */
	{
	(void)fprintf( stderr,
	"Usage:\tapply [-a<c>] [-<n>] [-v] [-w] <command> <args ...>\n"
		     );
	exit( 2 );
	}
