/*
 *			         SORT.C
 *
 *	              Copyright (c) 1986 Allen I. Holub
 *			   All rights reserved.
 */

#define LINT_ARGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "getargs.h"

/*----------------------------------------------------------------------
 *	General purpose #defines.
 */

#define MAXBUF		(132 + 1)	/* Maximum input line length +1	*/
#define	MAXLINEC	1024		/* Maximum number of lines in   */
					/* an input file before merge	*/
					/* files start to be created	*/
#define MAXTMP		18		/* The maximum number of temp-	*/
					/* orary files that will be	*/
					/* created. Two FILE ptrs. are	*/
					/* needed for stdout, and	*/
					/* stderr during output		*/

#define	isnum(c1) 	( isdigit(c1) || (c1) == '-' )

/*----------------------------------------------------------------------
 *	Variables for getargs. The immediately following variables will
 *	be modified by getargs() according to what flags it finds on the
 *	command line.
 */

static int	Debug		=	0	;	/* Debug Mode */
static int	Verbose		=  0    ;	/* Say what you're doing */
static int	Noblanks	=  0	;	/* Blanks don't count	 */
static int	Numeric		=  0	;	/* Sort numbers by val	 */
static int	Primary		=  0	;	/* Primary sort key	 */
static int	Secondary	=  0	;	/* Secondary sort key	 */
static int	Dictorder	=  0	;	/* Use dictionary order  */
static int	Foldupper	=  0	;	/* Fold upper case	 */
static int	Reverse		=  0	;	/* Sort in reverse order */
static int	Delim		=  32	;	/* Field separator	 */
static char	*Mdir		=  ""	;	/* Put temp files here	 */
static int	Nodups		=  0	;	/* Don't print duplicate */
										/* lines.		 */
ARG	Argtab[] =
{
	{ 'b' , ARG_FBOOLEAN,	&Noblanks,		"ignore leading whitespace (Blanks)"},
	{ 'd' , ARG_FBOOLEAN,	&Dictorder,		"sort in Dictionary order"	    },
	{ 'f' , ARG_FBOOLEAN,	&Foldupper,		"Fold upper into lower case"	    },
	{ 'n' , ARG_FBOOLEAN,	&Numeric,		"sort Numbers by numeric value"     },
	{ 'p' , ARG_INTEGER,	&Primary,    	"use field <num> as Primary key"    },
	{ 'r' , ARG_FBOOLEAN,	&Reverse,		"do a reverse sort"		    },
	{ 's' , ARG_INTEGER,	&Secondary,  	"use field <num> as Secondary key"  },
	{ 't' , ARG_CHARACTER,	&Delim,			"use <C> to separate fields"	    },
	{ 'u' , ARG_FBOOLEAN,	&Nodups,		"delete duplicate lines in output"  },
	{ 'v' , ARG_FBOOLEAN,	&Verbose,		"(Verbose) diagnostics to stderr"   },
	{ 'w' , ARG_STRING,		(int *)&Mdir,	"prepend <str> to Temp file names"  },
	{ 'z' , ARG_FBOOLEAN,	&Debug,			"Debug Mode" }
};

#define NUMARGS 	(sizeof(Argtab) / sizeof(ARG))

/*----------------------------------------------------------------------
 *	Global variables. The Lines array is used for the initial
 *	sorting.
 */

static  int	Options;		/* Set by main if any options set */
static	char	*Lines[MAXLINEC];	/* Holds arrays of input lines	*/
static	int	Linec;			/* # of items in Lines		*/
static  char	**Argv;			/* Copies of argv and argc	*/
static  int	Argc;

/*----------------------------------------------------------------------
 *	The heap used in the merge process:
 */

typedef struct
{
	char	string[MAXBUF];		/* One line from the merge file	*/
	FILE	*file;			/* Pointer to input file	*/
}
HEAP;

HEAP	*Heap[ MAXTMP ];		/* The heap itself		*/

/*----------------------------------------------------------------------*/

pheap( str, n )
char *str;
{
	int	i;

	printf("+--------------------------\n");
	printf("| %s, heap is:\n", str);
	for(i = 0; i < n ; i++ )
	{
		printf("|%02d: %s", i, *(Heap[i]->string) ? 
					 Heap[i]->string  : "(null)\n" );
	}
	printf("+--------------------------\n");
}

/*----------------------------------------------------------------------*/

int		stoi(instr)
register char	**instr;
{
	/*	Convert string to integer updating *instr to point
	 *	past the number. Return the integer value represented
	 *	by the string.
	 */

	register int	num  = 0 ;
	register char	*str	 ;
	int		sign = 1 ;

	str = *instr;

	if( *str == '-' )
	{
		sign = -1 ;
		str++;
	}

	while( '0' <= *str  &&  *str <= '9' )
		num = (num * 10) + (*str++ - '0') ;

	*instr = str;
	return( num * sign );
}

/*----------------------------------------------------------------------*/

int	dedupe(argc, argv)
int	argc;
char	**argv;
{
	/*	Compress an argv-like array of pointers to strings so that
	 *	adjacent duplicate lines are eliminated. Return the
	 *	argument count after the compression.
	 */

	register int	i	;
	int		nargc	;
	char		**dp	;

	nargc = 1;

	dp = &argv[1];

	for ( i=1  ;  i < argc ;  i++ )
	{
		if( strcmp(argv[i-1], argv[i]) != 0 )
		{
			*dp++ = argv[i];
			nargc++;
		}
	}

	return( nargc );
}

/*----------------------------------------------------------------------*/

static char	*skip_field(n, str)
int		n;
char		*str;
{
	/*	Skip over n fields. The delimiter is in the global
	 *	variable Delim. Return a pointer to either the character
	 *	to the right of the delimiter, or to the '\0'.
	 */

	while( n > 0   &&   *str )
	{
		if( *str++ == Delim)
			--n;
	}

	return(str);
}

/*----------------------------------------------------------------------
 *	 	Comparison functions needed for sorting.
 *
 *	ssort() will call either argvcmp or qcmp, passing them pointers
 *	to linev entries. qcmp() calls two workhorse functions, qcmp1()
 *	and qcmp2(). The workhorse functions will also be called by the
 *	reheap() subroutine used to manipulate merge files.
 */

static int	argvcmp( s1p, s2p )
char		**s1p, **s2p;
{
	return( strcmp( *s1p, *s2p ) );
}

/*----------------------------------------------------------------------*/

static int	qcmp( str1p, str2p )
char		**str1p;
char		**str2p;
{
	/*	Takes care of all the sorting of fields, calling qcmp1
	 *	to do the actual comparisons. Assuming here that
	 *	Secondary won't be set unless Primary is set too.
	 */

	return( qcmp1( *str1p, *str2p ) );
}

/*----------------------------------------------------------------------*/

static int	qcmp1( str1, str2 )
char		*str1, *str2;
{
	/*	Workhorse comparison function. Takes care of sorting
	 *	fields. If a primary sort field is specified then
	 *	qcmp1() skips to that field and calls qcmp2 to do the
	 *	actual comparison. If the primary fields are equal, then
	 *	the secondary fields are compared in the same way.
	 */

	int	rval;

	if( !Primary )
		return qcmp2( str1, str2 );
	else
	{
		rval = qcmp2(	skip_field(Primary - 1, str1),
				skip_field(Primary - 1, str2)	);

		if( !rval && Secondary )
		{
			/* The two primary keys are equal, search the	*/
			/* secondary keys if one is specified		*/

			rval = qcmp2(	skip_field(Secondary - 1, str1),
					skip_field(Secondary - 1, str2)	);
		}

		return rval;
	}
}

/*----------------------------------------------------------------------*/

static int	qcmp2( str1, str2 )
char		*str1;
char		*str2;
{
	/*	Workhorse comparison function. Deals with all command line
	 *	options except fields. Returns
	 *
	 *		0	 if	str1 == str2
	 *		positive if	str1  > str2
	 *	        negative if	str1  < str2
	 *
	 *	This is a general purpose (and therefore relatively slow)
	 *	routine. Use strcmp() if you need a fast compare.
	 *	Comparison stops on reaching end of string or on encountering
	 *	a Delim character (if one exists). So make sure Delim is
	 *	set to '\0' if you're not sorting by fields.
	 */

	register unsigned int 	c1, c2;

	if( Noblanks )
	{
		/*
		 *	Skip past leading whitespace in both strings
		 */

		while( isspace(*str1) )
			str1++;

		while( isspace(*str2) )
			str2++;
	}

	do
	{
		if( Numeric && isnum(*str1) && isnum(*str2) )
		{
			/* Add 0xff to the two numeric values so that
			 * c1 and c2 can't be confused with a Delim
			 * character later on.
			 */

			c1 = stoi( &str1 ) + 0xff ;
			c2 = stoi( &str2 ) + 0xff ;

			if( c1 == c2 )
				continue;
			else
				break;
		}

		c1 = *str1++;
		c2 = *str2++;

		if(Dictorder)
		{
			/*	Skip past any non-alphanumeric or blank
			 *	characters.
			 */

			while( c1 && !isalnum(c1) )
				c1 = *str1++ ;

			while( c2 && !isalnum(c2) )
				c2 = *str2++ ;
		}


		if(Foldupper)
		{
			/* Map c1 and c2 to upper case	*/

			c1 = toupper( c1 );
			c2 = toupper( c2 );
		}

		/* Keep processing while the characters are the same
		 * unless we've reached end of string or a delimiter.
		 */
	}
	while( (c1==c2)  &&  c1  &&  c1 != Delim );

	if( Delim )			/* If we're sorting on a field	*/
	{				/* and we've found a delimiter  */
		if(c1 == Delim)		/* then map the delimiter to a	*/
			c1 = 0;		/* zero for purposes of		*/
					/* comparison.			*/
		if(c2 == Delim)
			c2 = 0;
	}

	return( Reverse ? (c2 - c1) : (c1 - c2) );
}

/*----------------------------------------------------------------------*/

FILE	*nextfile()
{
	/*	Return a FILE pointer for the next input file or NULL
	 *	if no more input files exist (ie. all of the files
	 *	on the command line have been processed) or a file
	 *	can't be opened. In this last case print an error message.
	 *	If Argc == 1 the first time we're called, then standard
	 *	input is returned (the first time only , NULL is returned
	 *	on subsequent calls).
	 */

	FILE		*fp;
	static  int	first_time = 1 ;

	if( first_time )
	{
		first_time = 0;
		if( Argc == 1 )
			return stdin;
	}

	if( Argc-- > 1 )
	{
		if( !(fp = fopen(*++Argv, "r")) )
			fprintf(stderr, "sort: can't open %s for read\n",
								*Argv );
		else if( Verbose )
			fprintf(stderr, "sort: reading:  %s\n", *Argv );

		return fp;
	}

	return NULL;
}

/*----------------------------------------------------------------------*/

gtext()
{
	/*	Get text from the appropriate input source and put
	 *	the lines into Linev, updating Linec. Return non-zero
	 *	if more input remains. Note that non-zero will
	 *	be returned if there are exactly MAXLINEC lines in
	 *	the input, even though there isn't any more actual
	 *	input available. If malloc can't get space for the line,
	 *	we'll remember the line in buf and return 1.
	 */

	register int	c;
	static FILE	*fp 		=  0;
	static char	buf[ MAXBUF ]   = {0};		/* Input buffer */
	int		maxcount;
	char		**lv;

	if( !fp )			/* This is only true the first	*/
		fp = nextfile();	/* time we're called.		*/

	lv    = Lines ;
	Linec = 0     ;

	for( maxcount = MAXLINEC;  --maxcount >= 0 ;)
	{
		if( !*buf )
			while( fgets(buf, MAXBUF, fp) == NULL )
				if( !(fp = nextfile()) )
					return( 0 );	  /* No more input */

		if( *lv = malloc(strlen(buf) + 1) )
		{
			strcpy( *lv++, buf );
			*buf = '\0';
			Linec++;
		}
		else
			return 1;
	}

	return( maxcount < 0 );  /* Return 1 if there's more input to get */
}

/*----------------------------------------------------------------------*/

char	*fname( num )
{
	/* Return a merge file name for the indicated merge pass.
	 */

	static char	name[ 16 ];

	if( num > MAXTMP )
	{
		fprintf(stderr,"sort: input file too large\n" );
		exit(1);
	}

	sprintf(name, "%smerge.%d", Mdir, num );
	return( name );
}

/*----------------------------------------------------------------------*/

outtext( passnum, more_to_go )
{
	/*	Print out all the strings in the Lines array and free all
	 *	the memory that they use. Output is sent to standard
	 *	output if this is pass 1 and there's no more input
	 *	to process, otherwise output is sent to a merge file.
	 */

	register char	**lv;
	register FILE	*fp;

	if( passnum == 1  &&  !more_to_go )
		fp = stdout;

	else if( !(fp = fopen( fname(passnum), "w" )) )
	{
		fprintf(stderr,"Can't open merge file %s for write\n",
							fname(passnum));
		exit(1);
	}
	else if( Verbose )
		fprintf(stderr,"sort: creating: %s\n", fname(passnum));

	for( lv = Lines ; --Linec >= 0; )
	{
		fputs( *lv,   fp );
		free ( *lv++	 );
	}

	fclose( fp );
}

/*----------------------------------------------------------------------*/


open_mergefiles( nfiles )
{
	/*	Open all the merge files and create the heap. "nfiles"
	 *	merge-files exist and the heap will have that many
	 *	elements in it. The heap is unsorted on exit.
	 */

	HEAP	**hp;
	int	i;

	for( hp = Heap, i = nfiles;   i > 0;  hp++, --i )
	{
		if( ! (*hp = (HEAP *) malloc(sizeof(HEAP))) )
		{
			fprintf( stderr, "sort: out of memory!" );
			exit( 1 );
		}

		if( !( (*hp)->file = fopen( fname(i), "r")  ))
		{
			fprintf(stderr,"sort: can't open %s for read",
							fname(i) );
			exit( 1 );
		}

		if( !fgets( (*hp)->string, MAXBUF, (*hp)->file ) )
		{
			fprintf(stderr,"sort: merge file %s is empty",
							fname(i) );
			exit( 1 );
		}

		if( Verbose )
			fprintf(stderr, "sort: merging:  %s\n", fname(i) );
	}
}

/*----------------------------------------------------------------------*/

mcmp( hpp1,  hpp2 )
HEAP	  **hpp1, **hpp2;
{
	/*	Comparison routine for sorting the heap. Is passed
	 *	two pointers to HEAP pointers and compares the
	 *	string fields of these using the same workhorse
	 *	functions used in the initial sorting phase.
	 */

	return  Options	? qcmp1  ((*hpp1)->string, (*hpp2)->string)
			: strcmp ((*hpp1)->string, (*hpp2)->string)
			;
}

/*----------------------------------------------------------------------*/

reheap( nfiles )
{
	/*	Reheap the Heap, assume that the first element (**Heap)
	 *	is the newly added one.
	 */

	register int	parent, child;
	HEAP		*tmp;

	for( parent = 0, child = 1; child < nfiles; )
	{
		/*	Find the smaller child. Then if the parent is less
		 *	than the smaller child, we're done. Otherwise
		 *	swap the parent and child, and continue the
		 *	reheaping process with a new parent.
		 */

		if( child+1 < nfiles )      /* if child+1 is in the heap */
		    if( mcmp(&Heap[child], &Heap[child+1]) > 0)
			child++;

		if( mcmp( &Heap[parent], &Heap[child]) <= 0)
			break;

		tmp	     = Heap[parent];	/* Exchange 		*/
		Heap[parent] = Heap[child ];
		Heap[child ] = tmp;

		parent = child;
		child  = parent << 1;		/*   child = parent * 2	*/
	}
}

/*----------------------------------------------------------------------*/

merge( nfiles )
int	nfiles;				/* Number of merge files	*/
{
	open_mergefiles( nfiles );
	ssort( Heap, nfiles, sizeof(Heap[0]), mcmp );

	while( nfiles > 0 )
	{
		if( Debug )
			pheap( "Merge: top of while loop", nfiles );

		fputs( (*Heap)->string, stdout );

		if( !fgets((*Heap)->string, MAXBUF, (*Heap)->file) )
		{
			/* This input stream is exhausted. Reduce the
			 * heap size to compensate. Note that Heap+1
			 * is the same as &Heap[1];
			 */

			fclose( (*Heap)->file );
			if( --nfiles )
				memcpy(Heap, Heap+1, nfiles * sizeof(HEAP));
		}

		reheap( nfiles );
	}
}

/*----------------------------------------------------------------------*/

adjust_args()
{
	/*	Adjust various default arguments to fix mistakes made
	 *	on the command line. In particular Delim is always 0
	 *	unless either Primary or Secondary was set.
	 *	If a secondary field is specified without a Primary, then
	 *	1 is assumed for the primary. If no Delim is specified
	 *	then tab (\t) is assumed. "Options" is true if any of
	 *	the options that affect the sort order were specified
	 *	on the command line.
	 */

	if( !(Primary || Secondary) )
		Delim = 0;
	else
	{
/*		if( !Delim )
			Delim = ' ';*/

		if( !Primary )
			Primary = 1;
	}

	Options = Noblanks || Numeric || Dictorder || Foldupper 
						   || Reverse || Delim;
}

/*----------------------------------------------------------------------*/

void usage()
{
	fprintf( stderr, "Usage: sort [%c[bdfnruv][p<num>|s<num>][t<c>][w<str>]] [files]...\n", ARG_Switch );
	fprintf( stderr, "\n");
	fprintf( stderr, "Set the environment variable SWITCHAR to the character\n");
	fprintf( stderr, "you wish to use for the Switch Character\n");
	fprintf( stderr, "\n" );
	fprintf( stderr, "Case of the command line switches %s important\n",
		 ARG_ICase ? "is not" : "is" );
	fprintf( stderr, "\n" );
}

/*----------------------------------------------------------------------*/

main(argc, argv)
int	argc;
char	**argv;
{
	int	numpasses = 0;	/* Number of merge files used 		*/
	int	more_input;	/* True if input isn't exhausted	*/

	ctlc();
	ARG_ICase = 1;
	Argc = getargs( argc, argv, Argtab, NUMARGS );
	Argv = argv;
	adjust_args();

	do{
		more_input = gtext();

		if( Linec )
		{
			ssort(Lines, Linec, sizeof(*Lines), 
						Options ? qcmp : argvcmp);
			if( Nodups )
				Linec = dedupe(Linec, Lines);

			outtext( ++numpasses, more_input );
		}

	} while(  more_input );


	if( numpasses > 1 )		/* merge files were created	*/
	{
		fclose( stdin  );	/* Free up default file des-	*/
		fclose( stdaux );	/* criptors for unused streams	*/
		fclose( stdprn );	/* so that they can be used for	*/
					/* merge files.			*/
		merge( numpasses );

		for(; numpasses > 0 ; --numpasses )
		{
			unlink( fname(numpasses) );
			if( Verbose )
				fprintf(stderr, "sort: deleted:  %s\n",
							fname(numpasses));
		}
	}

	exit(0);
}
