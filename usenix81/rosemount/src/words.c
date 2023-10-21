#include <stdio.h>
#include <ctype.h>

#define	TOLOWER(C)	(isupper(C) ? tolower(C) : (C) )




/*
 * Outputs words that sound similer to input words.
 *
 *  Usage:  words [-s] [-o ofile] [ifile ...]
 */



char *swords = "/junk/swords" ;			/* Words sorted by soundex */
FILE *swstr ;

char *sort = "/usr/bin/sort" ;

char *filen[20] ;
int nfile, nf ;

int sflag ;										/* Sort only flag */
int xflag ;

int so[2], si[2] ;								/* Sort pipes */
FILE *tosort, *fromsort ;

char word[100], sdx[5] ;

extern char *getword(), *soundex() ;






main(argc, argv)
int argc ;
char **argv ;
{
	char *w, *s ;
	int fd ;


	argc-- ;
	argv++ ;
	while (argc) {
		if (**argv == '-') {
			while (*++*argv) {
				if (**argv == 's') sflag++ ;
				else if (**argv == 'x') xflag++ ;
				else if (**argv == 'o') {
					argc-- ;
					argv++ ;
					if (!argc) goto usage ;
					if( (fd = creat(*argv,0666)) < 0) {
						fprintf(stderr, "Can't creat: %s\n", *argv) ;
						exit (1) ;
						}
					close(fd) ;
					freopen( *argv, "w", stdout ) ;
					break;
					}
				else {
					usage:
						fprintf(stderr, "Usage: words [-s] [-o ofile] [ifile...]\n") ;
						exit (1) ;
					}
				}
			}
		else {
			if (nfile>19) {
				fprintf(stderr, "Too many files.\n") ;
				exit (1) ;
				}
			filen[nfile++] = *argv ;
			}
		argc-- ;
		argv++ ;
		}

	if (pipe(so)<0 || pipe(si)<0) {
			fprintf(stderr, "Can't open pipe.\n") ;
			exit(1) ;
		}

	switch (fork()) {
		case 0:
			fclose (stdin) ;
			if(dup( so[0] ) != 0) {
				fprintf( stderr, "Error in std. inp. dup.\n" );
				exit( 1 ) ;
				}
			fclose (stdout) ;
			if( dup( si[1] ) != 1 ) {
				fprintf( stderr, "Error in std. out. dup.\n" );
				exit( 1 );
				}
			close (so[0]) ;
			close (so[1]) ;
			close (si[0]) ;
			close (si[1]) ;

			execl(sort,sort+9,"-u",0) ;
			execl(sort+4,sort+9,"-u",0) ;

			fprintf(stderr, "Can't execl: %s\n", sort) ;
			exit (1) ;

		case -1:
			fprintf(stderr, "Can't fork.\n") ;
			exit (1) ;

		}

	close( so[0] );
	close( si[1] );
	tosort = fdopen(so[1], "w");
	fromsort = fdopen(si[0], "r");

	if (nfile) {
		if( freopen( filen[0], "r", stdin) == NULL ) {
			fprintf(stderr, "Can't open %s\n", filen[0]) ;
			close( so[1] ) ;
			close( si[0] ) ;
			wait( 0 ) ;
			exit (1) ;
			}
		nf++ ;
		}

	while (1) {						/* Read and send words to sort */
		while (w = getword(stdin)) {
			s = soundex(w) ;
			if( xflag ) fprintf( stderr, "%s %s\n", s, w ) ;
			fprintf(tosort, "%s%s\n", s, w) ;
			}
		if (nf >= nfile) break ;
		if( freopen( filen[nf++], "r", stdin) == NULL ) {
			fprintf(stderr, "Can't open %s\n", filen[nf-1]) ;
			close( so[1] ) ;
			close( si[0] ) ;
			wait( 0 );
			exit (1) ;
			}
		}

	fclose( tosort ) ;

	if (!sflag) {
		if ((swstr = fopen(swords, "r")) == NULL) {
			fprintf(stderr, "Can't open %s\n", swords) ;
			fclose( fromsort ) ;
			wait( 0 ) ;
			exit (1) ;
			}
		w = getword(swstr) ;
		}

	while (fscanf(fromsort, "%5s%s", sdx, word) != EOF) {
		if (sflag) fprintf(stdout, "%s\n", word) ;
		else {
			fprintf(stdout, "%s	", word) ;
			while (!sflag && strcmp(soundex(w), sdx) < 0) {
				if ((w = getword(swstr)) == 0) sflag++ ;
				}
			while (!sflag && strcmp(soundex(w), sdx) == 0) {
				fprintf(stdout, "%s ", w) ;
				if ((w = getword(swstr)) == 0) sflag++ ;
				}
			putc('\n', stdout) ;
			}
		}

	fclose( fromsort );
	wait( 0 ) ;
	exit (0) ;

	} /* main */







/*
 * Get a text word for fd.
 */

char *
getword(str)
FILE *str ;
{
	static char word[31] ;
	char c ;
	int i ;

	while ((c = getc(str)) != EOF && !isalpha(c)) {
		}

	for (i = 0; i < 30 && isalpha(c); i++) {
		word[i] = TOLOWER(c) ;
		if ((c = getc(str)) == EOF) break ;
		}

	word[i] = '\0' ;

	return (word[0] ? word : 0) ;

	} /* getword */






/*
 * Return soundex code string.
 */

char *
soundex(s)
char *s ;
{
	static char *sxcode = "01230120022455010623010202" ;
	static char x[6] ;
	register char *y, l;

	while ((!isalpha(*s) || TOLOWER(*s)<'a' || TOLOWER(*s)>'z') &&
		*s != '\0') {
		s++ ;
		}

	x[0] = isalpha(*s) ? TOLOWER(*s) : *s ;
	if (*s++ == '\0') return (x) ;

	x[1] = x[2] = x[3] = x[4] = '0' ;
	x[5] = l = '\0' ;

	y = x+1 ;
	while (*s && (y<(x+5))) {
		if (isalpha(*s) && sxcode[TOLOWER(*s)-'a'] != '0' &&
			sxcode[TOLOWER(*s)-'a'] != l) {
			*y++ = l = sxcode[TOLOWER(*s)-'a'] ;
			}
		else l = '\0' ;
		s++ ;
		}

	return (x) ;

	} /* soundex */
