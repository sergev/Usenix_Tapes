/*
	pg -- paginate input files (can be used as end of a pipeline)


	This source file may be made available to others, so long as
	Geotronics Corporation (Austin, Texas) is given proper credit.


				REVISION HISTORY

21-Dec-1980     D A Gwyn        Created sometime before this date.
06-May-1982     D A Gwyn        Added options, interrupt handling.
10-May-1982     D A Gwyn        Obtain terminal type from environment.
31-May-1982     D A Gwyn        Added -e option, prompt filename.
01-Jun-1982     D A Gwyn        Changed stdin prompt to "More?".


Usage:
	% pg[ -e][ -h screen_height][ -t term][ -w line_width] file ...
    or
	% program | pg[ -e][ -h screen_height][ -t term][ -w line_width]

	where `term' is a terminal designator (e.g., "d1" or "vt100"),
	`line_width' is the number of characters on a line (default 80),
	`screen_height' is the number of lines on a screen (default 24).

Method:
	Copies argument files (or standard input, if no file args) to
	standard output one screenload at a time.  After each screen,
	"More of <file>?" (just "More?" if standard input) is asked; any
	response except "n" or EOT will present the next screen.  "Next
	file (<file>)?" is asked before each file except the first.

	SIGINT aborts the current file; SIGQUIT aborts "pg".  These are
	disabled when the "-e" flag is specified, to preserve escape
	sequence integrity.

	Environment variable "TERM" is used to set up terminal type; the
	-t option overrides this.  An unknown terminal type is scrolled
	rather than painting its screen from the top down.
*/

#include        <ctype.h>
#include        <sgtty.h>
#include        <signal.h>
#include        <stdio.h>

#include        <std.h>
#include        <ttypes.h>

extern char     _sobuf[BUFSIZ];

int      ttype = -1;             /* unknown terminal type */

extern bool     isatty();
extern char     *getenv();
extern int      gtty(), strcmp(), stty();
extern void     exit(), sleep();

static void     clear(), intr(), quit();
static bool     more(), setterm();

static struct sgttyb    tty;
static int              omode;          /* original terminal mode */
static int              rmode;          /* raw mode during prompt */
static int              linesize = 80;  /* chars before wrap-around */
static int              pagesize = 24;  /* # of lines on terminal */
static int              filnum = 1;     /* file being processed */
static bool             eflag = NO;     /* support escape sequences */
static bool             neednl = NO;    /* newline needed on stdout */
static bool             typeit;         /* turned off by SIGINT */

main( argc, argv )
	int     argc;
	char    *argv[];
	{
	char    *fname;                 /* filename file for prompt */

	if ( !isatty( fileno( stdout ) ) )
		{
		fprintf( stderr, "pg: output not a terminal\n" );
		exit( 1 );
		}
	setbuf( stdout, _sobuf );

	if ( freopen( "/dev/tty", "r", stderr ) == NULL )
		exit( 2 );
	gtty( fileno( stderr ), &tty );
	omode = tty.sg_flags;           /* save for exit */
	rmode = (omode | RAW) & ~ECHO;  /* corresponding raw mode */
	/* after this point, cannot write on stderr */
	{
	register char   *tname;         /* terminal name */

	if ( (tname = getenv( "TERM" )) != NULL )
		setterm( tname );       /* if possible */
#ifdef DEBUG
	printf( "$TERM=%s\n", tname == NULL ? "(null)" : tname );
#endif
	}
	while ( argc > 1 && (*++argv)[0] == '-' )
		{                       /* process option */
#ifdef DEBUG
		printf( "process option %s\n", *argv );
#endif
		switch( (int)(*argv)[1] )
			{
			int                     temp;

		case 'e':               /* "-e" */
			eflag = YES;
			break;

		case 'h':               /* "-h screen_height" */
			--argc;
			if ( sscanf( *++argv, "%d", &temp ) != 1
			  || temp <= 0
			   )
				printf( "pg: bad -h \"%s\"\n", *argv );
			else
				pagesize = temp;
			break;

		case 't':               /* "-t term" */
			--argc;
			if ( !setterm( *++argv ) )
				printf( "pg: bad -t \"%s\"\n", *argv );
			break;

		case 'w':               /* "-w line_width" */
			--argc;
			if ( sscanf( *++argv, "%d", &temp ) != 1
			  || temp <= 0
			   )
				printf( "pg: bad -w \"%s\"\n", *argv );
			else
				linesize = temp;
			break;

		default:
			printf( "pg: unknown option \"%s\"\n", *argv );
			break;
			}

		--argc;
		}

	if ( eflag )
		{
		signal( SIGINT, SIG_IGN );
		signal( SIGQUIT, SIG_IGN );
		}
	else    {
		signal( SIGINT, intr );
		signal( SIGQUIT, quit );
		}

	fname = argc > 1 ? *argv : NULL;        /* first file, if any */
	do      {
		register character      c;
		register int            col = 0, line = 1;
		character               lastc = '\n';

		clear();
		if ( argc > 1 )
			{
			if ( freopen( fname, "r", stdin ) == NULL )
				printf( "**** can't open \"%s\" ****\n",
					fname
				      );
			else
				printf( "======== %s ========\n", fname
				      );
			++line;
			}

		typeit = YES;           /* NO when interrupt caught */
		while ( typeit && (c = getchar()) != EOF )
			{
			if ( line >= pagesize )
				{
				if ( !more( NO, fname ) )
					break;  /* next file */

				clear();
				line = 1;
				}

			switch ( c = toascii( c ) )
				{
			case '\v':
			case '\f':
				line = pagesize;
				c = '\n';
				/* fall through */
			case '\n':
				++line;
				/* fall through */
			case '\r':
				col = 0;
				break;
			case '\b':
				if ( col > 0 )
					--col;
				break;
			case '\t':
				col = col + 8 & ~7;
				break;
			case 0177:      /* DEL */
				ungetc( '/', stdin );
				c = '^';
				++col;
				break;
			case 033:       /* ESC */
				if ( eflag )
					{
					do      {
						if ( col < linesize )
							putchar( c );
						if ( isalpha( c ) )
						/* end esc sequence */
							break;

						if ( (c = getchar())
						     == EOF
						   )
							typeit = NO;
						else
							c = toascii( c
								   );
						} while ( typeit
						       && !iscntrl( c )
							);
					if ( typeit && iscntrl( c ) )
						ungetc( c, stdin );
					continue;       /* read next */
					}
				/* else fall through */
			default:
				if ( c < 040 )  /* map ctrl char */
					{
					ungetc( c + 0100, stdin );
					c = '^';
					}
				++col;
				break;
				}

			if ( col < linesize )
				putchar( lastc = c );
			}
		/* end of file */

		if ( lastc != '\n' )
			putchar( '\n' );

		fname = *++argv;
		} while ( ++filnum < argc && more( YES, fname ) );

	if ( neednl )
		putchar( '\n' );
	exit( 0 );
	}

static bool
more( file, fname )
	register bool   file;           /* determines prompt type */
	char            *fname;         /* filename for prompt */
	{
/* possible funny characters to take care of in RAW mode: */
#define CEOF    0004
#define CINTR   0177
#define CQUIT   0034
	register character      c;

	fflush( stdout );
	signal( SIGTERM, quit );
	tty.sg_flags = rmode;
	stty( fileno( stderr ), &tty );

	printf( file ? "Next file (%s)? "
		     : fname == NULL ? "More? "
				     : "More of %s? ",
		fname
	      );
	fflush( stdout );
	neednl = YES;

	while ( (c = toascii( getc( stderr ) )) == CINTR && file )
		;                       /* avoid possible race */

	tty.sg_flags = omode;
	stty( fileno( stderr ), &tty );
	signal( SIGTERM, SIG_DFL );

	if ( c == CINTR )
		return NO;              /* (file == NO in this case) */

	if ( c == CQUIT )
		quit();

	return c != CEOF && tolower( c ) != 'n';
	}

static bool
setterm( name )                         /* set up terminal type */
	register char   *name;          /* terminal designator */
	{
	/* table of recognized terminal types - add your favorites */
	static struct term
		{
		char    *code;          /* termcap designator */
		int     index;          /* value for `ttype' */
		}       table[] =
	{
		"vt100",        VT100,  /* DEC VT100 */
		"d1",           VT100,
		"adm3a",        ADM3A,  /* LSI ADM3A */
		"la",           ADM3A,
		"vk170",        VK170,  /* DEC VK170 */
		"dv",           VK170,
		"su",           -1,     /* standard unknown type */
		NULL,           -1      /* must be last entry */
	};
	register struct term    *tp;

	for ( tp = table; tp->code != NULL; ++tp )
		if ( strcmp( name, tp->code ) == 0 )
			break;

	ttype = tp->index;              /* -1 if not in table */

	return tp->code != NULL;
	}

static void
clear()                                 /* clear terminal screen */
	{
	switch ( ttype )
		{
	case VT100:
		fputs( "\033<\033[H\033[J", stdout );
		break;
	case ADM3A:
		putchar( '\032' );
		break;
	case VK170:
		putchar( '\f' );
		sleep( 2 );             /* let settle */
		break;
	default:
		if ( neednl )
			putchar( '\n' );
		break;
		}

	neednl = NO;
	}

static void
intr()                                  /* called on SIGINT */
	{
	signal( SIGINT, intr );         /* re-arm immediately */

	neednl = YES;                   /* probably garbage on screen */
	typeit = NO;                    /* ask for interruption */
#ifdef DEBUG
	printf( "\n** INTR **\n" );
	fflush( stdout );
#endif
	}

static void
quit()          /* called on SIGQUIT, also on SIGTERM in raw mode */
	{
	signal( SIGINT, SIG_IGN );      /* protect until mode reset */
	signal( SIGQUIT, SIG_IGN );
	signal( SIGTERM, SIG_IGN );

	tty.sg_flags = omode;
	stty( fileno( stderr ), &tty );

	neednl = YES;                   /* probably garbage on screen */
	clear();

#ifdef DEBUG
	printf( "\n** QUIT **\n" );
#endif
	exit( 3 );
	}
