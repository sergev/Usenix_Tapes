/*
     Boyer/Moore/Gosper-assisted 'egrep' search, with delta0 table as in
     original paper (CACM, October, 1977).  No delta1 or delta2.  According to
     experiment (Horspool, Soft. Prac. Exp., 1982), delta2 is of minimal
     practical value.  However, to improve for worst case input, integrating
     the improved Galil strategies (Apostolico/Giancarlo, Siam. J. Comput.,
     February 1986) deserves consideration.

     Method: 	extract longest metacharacter-free string from expression.
		this is done using a side-effect from henry spencer's regcomp().
		use boyer-moore to match such, then pass submatching lines
		to rexexp() for short input, or standard 'egrep' for long
		input.  (this tradeoff is due to the general slowness of the
		regexp() nondeterministic machine on complex expressions,
		as well as the startup time of 'egrep' on short files.)
		alternatively, you may change the faster 'egrep' automaton
		to include boyer-moore directly.
     Future: 
     		beef up for multiple patterns ala bm/fgrep.  can do fast -n 
		via file rescan, but it's a luxury.  adapt 'fastfind'.
		internationalize for kanji.

     James A. Woods					Copyright (c) 1986
     NASA Ames Research Center
*/
#ifdef	V7
#define BSD
#define void	int
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regexp.h>		/* must be henry spencer's version */

#ifdef	BSD
#define	strchr	index
#endif
#ifndef EGREP
#define	EGREP	"/usr/bin/egrep"  /* prevent installation-dependent recursion */
#endif
#define BUFSIZE	8192
#define PATSIZE 1000
#define LARGE 	BUFSIZE + PATSIZE
#define FSIZE	50000		/* algorithm tradeoff at this length (ad hoc) */
#define NL	'\n'
#define	EOS	'\0'

extern char *optarg;
extern int optind;

int cflag, iflag, eflag, fflag, lflag;
int boyflag, rxflag;
int hflag;
int nfile, nsuccess;
long nmatch;

regexp *rspencer;
char *pattern, *patboy;
int delta0[256];		/* ascii only -- see note at gosper() */
char cmap[256];			/* (un)folded characters */
char str[BUFSIZE+2];
char linetemp[BUFSIZE];
char grepcmd[PATSIZE]; 

struct stat stbuf;
int fd;
FILE *egout, *mcilroy(), *popen();
char *strchr(), *strcpy(), *strncpy(), *strpbrk(), *malloc();
char *fold(), *sys5fold();

main ( argc, argv )
	int argc; char *argv[];
{
        int c;
        int errflag = 0;
        int oldegrep = 0;

        while ( (c = getopt ( argc, argv, "bchie:f:lnv" ) ) != EOF )

                switch(c) {

		case 'f':
			fflag++;
                case 'b':
		case 'n':
		case 'v':
			oldegrep++;	/* boyer-moore of little help here */
                        continue;
                case 'c':
                        cflag++;
                        continue;
                case 'e':
                        eflag++;
                        pattern = optarg;
                        continue;
		case 'h':
			hflag++;	/* shh ... for newshounds */
			continue;
                case 'i':
                        iflag++;
                        continue;
                case 'l':
                        lflag++;
                        continue;
                case '?':
                        errflag++;
        	}
        argc -= optind;
        if ( errflag || ((argc <= 0) && !fflag) )
		oops ( "usage: egrep [-bcilnv] [-e exp] [-f file] [strings] [file]" );
        if ( !eflag ) {
                pattern = argv[optind++];
                argc--;
        }
	if ( oldegrep || (strchr ( pattern, '\n' ) != NULL) ) {
		execvp ( EGREP, argv );
		oops ( "can't exec old 'egrep'" );
	}
        if ( iflag ) {
		strcpy ( pattern, fold ( pattern ) );
	}
	if ( strpbrk ( pattern, "^$.[]()?+*|\\" ) == NULL ) {	/* do boyer-moore only */
		boyflag++;
		rxflag = 0;
		patboy = pattern;
	}
	else { 	
		/*
		    first compile a fake regexp to isolate longest
		    metacharacter-free string
		*/
		{
			char *dummyp;
			dummyp = malloc ( (unsigned) strlen ( pattern ) + 5 );
			sprintf ( dummyp, "(.)*%s", pattern );
			rspencer = regcomp ( dummyp );
		}
		if ( rspencer -> regmust == NULL ) {		/* pattern too complicated */
			execvp ( EGREP, argv );
			oops ( "can't exec old 'egrep'" );
		}
		patboy = rspencer -> regmust;
		if ( (rspencer = regcomp ( pattern )) == NULL )
			oops ( "egrep: regcomp failure" );
	}
	gosper ( patboy );
        argv = &argv[optind];
        nfile = argc;
        if ( argc <= 0 ) {		/* process stdin */
                if ( lflag )
			exit ( 1 );
		fd = 0;
		if ( !boyflag )
			if ( (egout = mcilroy ( (char *) NULL )) == NULL )
				oops ( "egrep: no processes" );
                boyermoore ( (char *) NULL, patboy );
		if ( !boyflag )
			pclose ( egout );
        }
        else {
                while ( --argc >= 0 ) {
                	if ( (fd = open ( *argv, 0 )) <= 0 ) {
                        	fprintf ( stderr, "egrep: can't open %s\n", *argv );
                        	nsuccess = 2;
				argv++;
				continue;
			}
			if ( !boyflag ) {
				fstat ( fd, &stbuf );
				if ( stbuf.st_size < FSIZE )
					rxflag = 1;
				else {
					rxflag = 0;
					if ( (egout = mcilroy ( *argv )) == NULL )
						oops ( "egrep: no processes" );
				}
			}
                        boyermoore ( *argv, patboy );
			if ( !boyflag && !rxflag ) {
				fflush ( egout );
				pclose ( egout );
			}
			close ( fd );
			argv++;
                }
	}
        exit ( (nsuccess == 2) ? 2 : (nsuccess == 0) );
}

boyermoore ( file, pattern )	/* "reach out and boyer-moore search someone" */
	char *file, *pattern;	/* -- soon-to-be-popular bumper sticker */
{
	register char *k, *strend, *s;
	register int j;
	int patlen;
	char *t;
	char *gotamatch();
	int nleftover, count;

	patlen = strlen ( pattern );
	nleftover = nmatch = 0;

#ifdef V7
#define read xread
#endif
	while ( (count = read ( fd, str + nleftover, BUFSIZE - nleftover )) > 0 ) {

#undef read
		count += nleftover;
		if ( count != BUFSIZE && fd != 0)
			str[count++] = NL;	/* insurance for broken last line */
		str[count] = 0;
		for ( j = count - 1; str[j] != NL && j >= 0; )
			j--;
		if ( j < 0 ) {			/* break up long line */
			str[count] = NL;
			str[++count] = EOS;
			strend = str + count;
			linetemp[0] = EOS;
			nleftover = 0;
		}
		else {				/* save partial line */
			strend = str + j;
			nleftover = count - j - 1;
			strncpy ( linetemp, str + j + 1, nleftover );
		}
		k = str + patlen - 1;

		for ( ; ; ) {
			/*
			    for a large class of patterns, upwards of 80% of 
			    match time is spent on the next line.  
			    we beat existing microcode (vax 'matchc') this way.
			*/
#ifndef V7
			while ( (k += delta0[*(unsigned char *)k]) < strend )
				;	
#else
			while ( (k += delta0[*(char *)k & 0377]) < strend )
				;
#endif
			if ( k < (str + LARGE) )
				break;
			k -= LARGE;

			j = patlen - 1;
			s = k - 1;
			while ( cmap[*s--] == pattern[--j] )
				;
			/*
			    delta-less shortcut for literati, but 
			    short shrift for genetic engineers.
			*/
			if ( j >= 0 )
				k++;
			else {			/* submatch */
				t = k;
				s = k - patlen + 1;
				do 
					;
				while ( *s != NL && --s >= str ); 
				k = s + 1;	/* at line start */

				if ( boyflag ) {
					if ( (k = gotamatch ( file, k )) == NULL )
						return;
				}
				else if ( !rxflag ) {	/* fob off to egrep */
					do
						putc ( *k, egout );
					while ( *k++ != NL );
				}
				else {				/* regexec() wants EOS */
					s = t;
					do
						;
					while ( *s++ != NL );
					*--s = EOS;

					if ( regexec ( rspencer, ((iflag) ? fold ( k ) : k) ) == 1 ) {
						*s = NL;
						if ( gotamatch ( file, k ) == NULL )
							return;
					}
					*s = NL;
					k = s + 1;
				}
				if ( k >= strend )
					break;
			}
		}
		strncpy ( str, linetemp, nleftover );
	}
	if ( cflag && (boyflag || rxflag) ) {
		if ( nfile > 1 )
			printf ( "%s:", file );
		printf ( "%ld\n", nmatch );
	}
}

FILE *
mcilroy ( file )	/* open a pipe to old 'egrep' for long files, */
	char *file;	/* ... where regexp() might be inefficient */
{
#ifdef BSD
	int iflagsave = 0;
	char *patsave;

	if ( iflag ) {	
		iflagsave = 1;
		iflag = 0;
		patsave = pattern;
		pattern = sys5fold ( pattern );		/* uncripple -i */
	}
#endif
	/*
	    -l via -c is sneaky, but we're short a good prwopen() 
	*/
	if ( lflag && !cflag )
		sprintf ( grepcmd, "%s -c %s '%s' | sed -n '/^0$/!s|.*|%s|p'",
			            EGREP, (iflag ? "-i" : " "), pattern, file );
	else if ( nfile <= 1 )
		sprintf ( grepcmd, "%s %s %s '%s'", EGREP,
				    (cflag ? "-c" : " "), (iflag ? "-i" : " "), pattern );
	else
		sprintf ( grepcmd, "%s %s %s '%s' | sed -n 's|^|%s:|p'", EGREP,
				    (cflag ? "-c" : " "), (iflag ? "-i" : " "), pattern, file );
	/*
	    we thank mr. thompson for an especial ndfa simulation.
	    (consult cacm, june 1968, or aho et. al., design & analysis ...,
	    algorithm 9.1).
	*/
#ifdef BSD
	if ( iflagsave ) {
		iflag = 1;
		pattern = patsave;
	}
#endif
	return ( popen ( grepcmd, "w" ) );
}

gosper ( pattern )		/* compute "boyer-moore" delta table */
	char *pattern;		/* ... HAKMEM lives ... */
{
	register int j, patlen;
	/*
	    for multibyte characters (e.g. kanji), there are ways.
	    extrapolating 256 to 64K may be unwise, in favor of more
	    dynamics within boyermoore() itself. 
	*/
	patlen = strlen ( pattern );

	for ( j = 0; j < 256; j++ ) {
		delta0[j] = patlen;
		cmap[j] = (char) j;	/* could be done at compile time */
	}
	for ( j = 0; j < patlen - 1; j++ )
		delta0[pattern[j]] = patlen - j - 1;
	delta0[pattern[patlen - 1]] = LARGE;

	if ( iflag ) {
		for ( j = 0; j < patlen - 1; j++ )
			if ( islower ( (int) pattern[j] ) )
				delta0[toupper ((int) pattern[j])] = patlen - j - 1;
		if ( islower ( (int) pattern[patlen - 1] ) )
			delta0[toupper ((int) pattern[patlen - 1])] = LARGE;
		for ( j = 'A'; j <= 'Z'; j++ )
			cmap[j] = (char) tolower ( (int) j );
	}
}

char *
gotamatch ( file, s )		/* print, count, or stop on full match */
	register char *file, *s;
{
	nsuccess = 1;
	if ( cflag ) {		/* -c overrides -l, for some reason */
		nmatch++;
		do
			;
		while ( *s++ != NL );
	}
	else if ( lflag ) {
		puts ( file );
		return ( NULL );
	}
	else {
		if ( nfile > 1 )
			printf ( "%s:", file );
		do
			putchar ( *s );
		while ( *s++ != NL );
	}
	return ( (hflag && !cflag) ? NULL : s );
}


char *
fold ( line )
	char *line;
{
	static char fline[BUFSIZE];
	register char *s, *t = fline;

	for ( s = line; *s != EOS; s++ )
		*t++ = (isupper ((int) *s) ? (char) tolower ((int) *s) : *s );
	*t = EOS;
	return ( fline );
}
	
#ifdef BSD
char *
sys5fold ( line )	/* a workaround kludge for the old alma mater, e.g. */
	char *line;	/* ... "ZIP.*PY" becomes "[zZ][iI][pP].*[pP][yY]" */
{
	register char *p, *s;
	static char pline[BUFSIZE];
	char c, stencil[5];
	int bdanger = 0;

	pline[0] = EOS;
	for ( s = line; *s != EOS; s++ ) {
		if ( *s == '[' )
			bdanger = 1;
		if ( bdanger == 1 && *s == ']' )
			bdanger = 0;
		if ( isalpha ( (int) *s ) ) {
			c = ( (islower ( *s )) ? (char) toupper ( (int) *s ) :
						 (char) tolower ( (int) *s ) );
			sprintf ( stencil, ((bdanger) ? "%c%c" : "[%c%c]"), *s, c ); 
		}
		else {
			stencil[0] = *s;
			stencil[1] = EOS;
		}
		strcat ( pline, stencil );
	}
	*s = EOS;
	return ( pline );
}
#endif

oops ( message ) 
	char *message;
{
	fprintf ( stderr, "%s\n", message );
	exit ( 2 );
}
