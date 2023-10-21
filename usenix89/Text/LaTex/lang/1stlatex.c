/*
**	1stlatex.w	1st_word to latex conversion program
**	version 1.1
**
**	Jerome M. Lang
**	Public Domain
**	7 March 1985.
**
**	Main file
*/
/*
**	1986-04-28	: added support for characters that need
**			   boldmath quoting (bold lowercase greek)
**				[shouldn't need this, YUK]
**	1986-04-28	: added double line spacing
*/

#define EXT	/* this will reserve storage for the global variable */
#include "1stlatex.h"

int		Ch=' '; /* last character read */
int		Prch; 	/* previous character read */
int		Eof = FALSE;	/* becomes true once we reached end of input */

#define BUFSIZE	4096
char	Inbuf[BUFSIZE];

/*
** The following is a translation table for the function input(),
** it returns the class of the character on input.
** Classes are as follows
**	B - Conditional page break, takes 1 char arg.
** 	C - Regular character
**	E - Expandable space (stretch space)
**	F - Form feed
**	H - Header (format line), ignore till end of line
**	I - Indent space
**	L - Line feed
**	N - Null, reserved by 1st_word
**	Q - Quote, characters that need to be quoted $ & % # _ { }
**		and extended character set.
**	R - Carriage return
**	S - Style change, 1 char arg.
**	T - Tab
**	U - Unimplemented
**	V - Variable space
**	X - FiXed space
**	Z - End of file has been reached
*/

char	Xlate[]= {
/*       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F */
/*00*/	'N','Q','Q','Q','Q','U','U','U','U','T','L','B','F','R','U','U',
/*10*/	'U','U','U','U','U','U','U','U','U','U','U','S','E','I','V','H',
/*20*/	'X','C','C','Q','Q','Q','Q','C','C','C','C','C','C','C','C','C',
/*30*/	'C','C','C','C','C','C','C','C','C','C','C','C','Q','C','Q','C',
/*40*/	'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',
/*50*/	'C','C','C','C','C','C','C','C','C','C','C','C','Q','C','Q','Q',
/*60*/	'C','C','C','C','C','C','C','C','C','C','C','C','C','C','C','C',
/*70*/	'C','C','C','C','C','C','C','C','C','C','C','Q','Q','Q','Q','Q',
/*80*/	'Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q',
/*90*/	'Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','U','Q','U','Q','Q',
/*A0*/	'Q','Q','Q','Q','Q','Q','Q','Q','Q','U','U','Q','Q','Q','Q','Q',
/*B0*/	'Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','U','U',
/*C0*/	'U','U','Q','U','U','U','U','U','U','U','U','Q','U','U','U','U',
/*D0*/	'U','U','Q','U','U','U','Q','Q','U','U','U','U','U','Q','Q','Q',
/*E0*/	'Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q',
/*F0*/	'Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','Q','U','U','U','U'
	};

/* ------------------------------------------------------------ */
#include "1stwart.c"
/* ------------------------------------------------------------ */

int
biosgetc(file)
FILEDESC	file;
{
	static char	*ptrinbuf;
	static long	count	= 0;
/*& Cconws("in biosgetc ");
/*& &*/
	if ( Eof )	return ( EOF );
/*& Cconws("not EOF ");
/*& &*/
	if ( count <= 0 )	/* fill'er up */
	{
#ifdef DEBUG
Cconws("before read\r\n ");
#endif
		count = Fread( file, (long)BUFSIZE, Inbuf );
		if ( count <= 0 )
		{
#ifdef DEBUG
Cconws("read failed\r\n ");
#endif
			Eof = TRUE;
			return ( EOF );
		}
		else
		{
#ifdef DEBUG
Cconws("read good\r\n ");
#endif
			ptrinbuf = Inbuf;
		}
	}
/*& Cconws("before return ");
/*& &*/
	count--;
	return ( ((unsigned int)( *ptrinbuf++ ) & 0x0FF));
}

int
input()
{
	int	c;

	if ( Eof ) return ('Z');

	c = biosgetc( Infile );
#ifdef DEBUG
		Cconout( (c==0x1b)? '%':c );
#endif
	if ( EOF == c )
	{
		Eof = TRUE;
		return ( 'Z' );
	}
	else
	{
		Prch = Ch;
		Ch = c & 0xFF ;
		return ( Xlate[Ch] );
	}
}


fatal(s)
char *s;
{
	fprintf(stderr, "fatal -%s\n",s);
	Cconin();
	exit(1);
}
	
main( argc, argv )
int	argc;
char	*argv[];
{

	FILE	*fopen();

	Outfile = stdout;

	if ( argc > 2 )
	{
		if ( (Outfile = fopen( argv[2], "w" )) == NULL )
		{
			fprintf( stderr, "1stlatex: can't write to %s", argv[2] );
			exit(1);
		}
#ifdef DEBUG
		Cconws( argv[2] );
#endif
	}
#ifdef DEBUG
	Cconws("<-");
#endif
	if ( argc > 1 )
	{
		if ( (Infile = Fopen( argv[1], 0) ) < 0 )
		{
			fprintf( stderr, "1stlatex: can't read %s", argv[1] );
			exit(1);
		}
#ifdef	DEBUG
		Cconws(argv[1]);
#endif
	}
	else
	{
		fatal("usage: 1stlatex infile [outfile]");
	}

	/* translate file */

	BEGIN charmode;
	underline = FALSE;

	wart();  /* <--- given by preprocessor */

	/* close up shop */

	doStyle( 0 );

	Fclose( Infile );
	fclose( Outfile );
}
