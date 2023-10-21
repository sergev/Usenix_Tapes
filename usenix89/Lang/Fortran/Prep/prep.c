/* Program PREP.C
 *
 * Preprocessor for FORTRAN 77.
 * Adds the additional features:
 *
 *  1) Vector arithmetic:
 *     a(#,#,1) = b(#,#) + 1
 *
 *   [ a(#) = b(#)*c(#) - 100
 *     x = y
 *     d(#) = e(#) 		]
 *
 *  2) Case construct:
 *     case ( exp1 )
 *     of   ( exp2 )  line of code
 *                    line of code
 *                    continue_case
 *     of   ( exp3 )  line of code
 *     default        line of code
 *                    line of code
 *     end_case
 *
 *  3) do i = 1, 10
 *        line of code
 *        line of code
 *     leave_do (optional expression)
 *        line of code
 *     continue_do (optional expression)
 *        line of code
 *     end_do
 *
 *  4) forth style begin/while/until/again construct:
 *     begin ... again
 *     begin ... while (exp1) ... again
 *     begin ... until (exp1)
 *     leave (optional expression) to exit current level
 *     continue (optional expression) to go back to beginning
 *
 *  5) Vector loop unrolling to any depth, for loops 
 *     that can be expressed as in #1 above.
 *
 *  6) Macro processing, define a macro "name" with:
 *     : name(a,b,c)	a = a + func( c, d ) ;
 *
 *  7) Included files:
 *     #include "filename"
 *
 *  8) Conditional compilation:
 *     #ifdef, #ifndef, #else, #endif
 *
 *    The nesting limit for all loops is defined by the constant
 * NESTING in file prepdefs.h.  All underline characters are removed,
 * as are comments if com_keep is NULL.
 *    Any delimeters (){}[]'" may be used in the logical expressions
 * ( i.e.  leave [i .eq. 1] ).
 *    The flow control directives are permitted inside vector
 * loops, but since they will inhibit Cray vectorization of those
 * loops it may be best to avoid this.  One of the reasons for
 * using the vector shorthand is that it encourages programming
 * in a style that can be easily vectorized.
 *    Some attempts have been made to avoid ratfor syntax to that
 * both preprocessors can be used, but this has never been checked.
 *    The number of parameters allowed in a macro is set by the constant
 * MAX_MAC_PARMS in file prepdefs.h (20 is probably more than enough).
 *    Although the syntax is similar to forth, the spirit of
 * forth is totally absent.  The macros are really macros,
 * not colon definitions, and recursive macro definitions will cause
 * an error during expansion.  Postfix notation would only cause
 * confusion, being in conflict with fortran conventions, and is
 * not used.
 *    The macro processor can be considered a pre-preprocessor.  The
 * order of translation is:
 *
 *	1) file inclusion & conditional compilation
 *	2) macro processing
 *	3) flow control extensions
 *	4) vector statements
 *
 * Note that because of this the flow control syntax can be modified
 * at the macro level.
 *
 * Switches:
 *   -c		keep comments (truncated at column 72)
 *   -u		keep underline characters
 *   -m		only do macro substitution (==> -c and -u as well, and
 *		prevents file includes (except -i switch).
 *   -i	<file>	include <file> before processing
 *   -d <name>  define <name> as a macro, using :name 1;
 *   -r n	unroll vector loops to depth n
 *   -l n	unroll loops with n or fewer lines
 *   -(other)	write message about allowed switches
 *
 * P. R. OVE  11/9/85
 */

#define	MAIN	1
#include "prep.h"

main( argc, argv )
int	argc ;
char	*argv[] ;
{
int 	i, j, maxlength, lines ;
char	*text, *semi ;


init() ;
parmer( argc, argv ) ;	/* process command line switches */

/* copyright notice */
fprintf( stderr,
	"PREP: Copyright P.R.Ove.\n" ) ;

/* Main loop, loop until true end of file */
while ( NULL != get_rec() ) {

	/* if an #ifdef has turned off processing, keep looking for #endif.. */
	if ( ignore_flag ) {
		if (NULL != (semi = strchrq(in_buff,';'))) *semi = NULL ;
		preproc( rec_type(0) ) ;
		continue ;
	}

	if ( comment_filter() ) continue ; /* TRUE ==> nothing left */

	/* if only doing macro expansion: */
	if ( macro_only ) {
		if ( NULL != (text = mac_proc()) ) { /* NULL ==> macro def */
			put_string( text ) ;
			free( text ) ;
		}
		continue ;
	}

	/* handle file inclusion & #ifdefs */
	if ( NULL == preproc(rec_type(0))) continue ;

	/* expand macros in in_buff, result pointed to by text */
	if ( NULL == (text = mac_proc()) ) continue ; /* NULL ==> macro def */

	/* count lines in text, delimit with NULLs, and find the longest line */
	for ( maxlength=0, i=0, j=0, lines=1;; i++, j++ ) {
		if ( text[i] == '\n' ) {
			text[i] = NULL ;
			if ( j>maxlength ) maxlength = j ;
			j = -1 ;
			lines++ ;
			continue ;
		}
		if ( text[i] == NULL ) {
			if ( j>maxlength ) maxlength = j ;
			break ;
		}
	}

	/* if necessary expand the output buffer size */
	if ( maxlength > allocation ) {
		allocation = maxlength + maxlength/10 ;
		if ( NULL == (in_buff = realloc( in_buff, allocation )) )
			abort( "reallocation failed" ) ;
		if ( NULL == (out_buff = realloc( out_buff, 4*allocation )) )
			abort( "reallocation failed" ) ;
	}

	/* send each line through the passes */
	for ( j=0, i=0; j<lines; j++, i+=strlen(&text[i])+1 ) {
		strcpy( in_buff, &text[i] ) ;
		passes() ;
	}
	
	/* free the storage created by mac_proc */
	free( text ) ;
}
fclose( out ) ;		/* SVS seems to need this explicit close */
}



/* Do preprocessor passes 1, 2, and 3 on text in in_buff.  Output is
 * also done here.
 */
passes()
{

/* process the statement until it is NULL */
while (1) {

	if ( NULL == preproc( rec_type(1) ) ) break ;

	if ( NULL == preproc( rec_type(2) ) ) break ;

	if ( NULL == preproc( rec_type(3) ) ) break ;
}
}




/* initialization */
init() {

flow_init() ;
vec_init() ;

/* Allocate some space for the buffers. */
allocation = DEF_BUFFSIZE ;
GET_MEM( in_buff, allocation ) ;
GET_MEM( out_buff, 4*allocation ) ;
}



/* error exit */
abort( string )
char	*string ;
{
	fprintf( stderr, "%s\n", string ) ;
	fprintf( out, "%s\n", string ) ;
	fclose( out ) ;
	exit(1) ;
}
