/* misc routines */

#include "prep.h"




/* Function DUMP.C
 *
 *   Send a string to the output stream.  The string is a
 * fortran record constructed by PREP, which may be
 * longer than 72 characters after processing.  It is
 * broken up into pieces before output.  The string
 * must be null terminated.  The string is not affected
 * by this routine, so it is safe to do
 *       dump( "explicit text" ) ;
 *
 *   If inside a vector loop (vec_flag==TRUE) the record is
 * not broken up and is sent to mem_store rather than a file.
 *
 * P. R. OVE  11/9/85
 */

dump( string ) 
char 	*string ;

{
char	record[73], *pntr ;
int	i_str, i_rec = 0, i, i_tab, quote_flag = 0 ;

/* ignore empty lines sent here */
if ( NULL == line_end( string ) ) return ;

/* if in a vector loop write the string to mem_store */
if ( vec_flag ) {
	push( string ) ;
	return ;
}

/* loop until end of record */
for ( i_str = 0;; i_str++ ) {

	/* wrap up on end of line */
	if ( line_end( &string[i_str] ) == NULL ) {
       		record[i_rec] = NULL ;
		put_string( record ) ;
		break ; }

	/* break string if necessary */
	if ( i_rec >= 72 ) {                
		record[i_rec] = NULL ;
		put_string( record ) ;
		strcpy( record, "     *" ) ;
		i_str-- ;
		i_rec = 6 ;
		continue ;
	}

	/* toggle quote flag on quotes */
	if ( string[i_str] == '\'' ) quote_flag = ! quote_flag ;
		
	/* underline filtering */
	if ( (string[i_str]=='_') && (!underline_keep) && (!quote_flag) )
		continue ;

	/* tab handling */
	if ( string[i_str] == TAB ) {
		if (	i_rec >= 70 - tab_size ) {
			record[i_rec] = NULL ;
			put_string( record ) ;
			strcpy( record, "     *" ) ;
			i_rec = 6 ; }

		else {  /* replace tab by blanks */
			i_tab = ( ( i_rec + 1 )/tab_size ) 
			      * tab_size - i_rec + tab_size - 1 ;
			for ( i = 0; i < i_tab; i++ ) {
				record[i_rec] = BLANK ;
		                i_rec++ ; }
		}
		continue ;
	}

			
	/* default action */
	record[i_rec] = string[i_str] ;
	i_rec++ ;

}                       
}                          




/* GET_RECORD
 *
 * Get a record from the input stream, making sure that the buffer
 * does not overflow by increasing its size as necessary.  The 
 * string in_buff will contain the record on return.  In_buff will
 * always contain about ten percent of its default length in trailing 
 * blanks to play with.  Out_buff will have space allocated for it
 * as well, 4 times that of in_buff.  Returns a pointer to the 
 * terminating NULL character.  On EOF the previous input file
 * (assuming the present one was an include file) will be restored as
 * the input file.  If the filestack is empty return NULL.
 */

char	*get_rec()
{
int	i, j ;
char	*pntr, *area ;

/* fill the in_put buffer, enlarging it when nearly full in 
 * increments of DEF_BUFFSIZE.  On end of file the previous file
 * handle is popped from the include stack (if present).
 */
pntr = in_buff ;
i = 0 ;
while(1) {

	for (; i < allocation - DEF_BUFFSIZE/10 ; i++, pntr++ ) {
		*pntr = getc(in) ;
		if ( *pntr == EOF || *pntr == 0x1A ) {
			fclose(in) ;
			if ( NULL == popfile(&in) ) return( NULL ) ;
			pntr = in_buff-1 ;
			i = -1 ;
			continue ;
		}
		if ( *pntr == '\n' ) {
			*pntr = NULL ;
			return( pntr ) ;
		}
	}


	/* if control falls through to here, increase buffer sizes. */
	allocation += DEF_BUFFSIZE ;
	if ( NULL == ( in_buff = realloc( in_buff, allocation ) ) )
		abort( "Reallocation failed" ) ;
	if ( NULL == ( out_buff = realloc( out_buff, 4*allocation ) ) )
		abort( "Reallocation failed" ) ;
}

}



/* Function LINE_END
 *
 * Return a NULL pointer if the string contains only
 * blanks and tabs or if it is a NULL string.  Else
 * return a pointer to the first offending character.
 *
 * P. R. OVE  11/9/85
 */

char	*line_end( string ) 
char 	*string ;

{
	for (; *string != NULL; string++ )
		if ( (*string != BLANK) && (*string != TAB) ) return(string) ;

	return( NULL ) ;
}




/* Function MAT_DEL
 *
 * Given pointer to a delimeter this routine finds its
 * partner and returns a pointer to it.  On failure a
 * NULL pointer is returned.  The supported delimeters
 * are:
 *
 *   '  "  ( )  [ ]  { }  < >
 *
 * ' and " are supported only in the forward direction
 * and no nesting is detected.
 * In all cases the search is limited to the current
 * line (bounded by NULLs).
 *
 * P. R. OVE  11/9/85
 */


char *mat_del( pntr )
char	*pntr ;

{
int	nest_count = 0, i, direction ;
char	target ;

if ( pntr == NULL ) return( NULL ) ;

/* get the target character and direction of search */
	switch( *pntr ) {

		case '(' :	{ target = ')' ;
				  direction = 1 ;
				  break ;          }

		case ')' :	{ target = '(' ;
				  direction = -1 ;
				  break ;          }

		case '[' :	{ target = ']' ;
				  direction = 1 ;
				  break ;          }

		case ']' :	{ target = '[' ;
				  direction = -1 ;
				  break ;          }

		case '{' :	{ target = '}' ;
				  direction = 1 ;
				  break ;          }

		case '}' :	{ target = '{' ;
				  direction = -1 ;
				  break ;          }

		case '<' :	{ target = '>' ;
				  direction = 1 ;
				  break ;          }

		case '>' :	{ target = '<' ;
				  direction = -1 ;
				  break ;          }

		case '\'':	{ target = '\'' ;
				  direction = 1 ;
				  break ;          }

		case '\"':	{ target = '\"' ;
				  direction = 1 ;
				  break ;          }

		default:	  return( NULL ) ;
				
	}

/* find the match */
	for ( i = direction; pntr[i] != NULL; i += direction ) {
		
		if ( pntr[i] == target ) {

			if ( nest_count == 0 ) {
				break ;	}
			else {
				nest_count-- ;
				continue ; }
                }
		
		if ( pntr[i] == pntr[0] ) nest_count++ ;
	}

	if ( &pntr[i] == NULL ) return( NULL ) ;
	return( &pntr[i] ) ;
}




/* PARMER
 *
 * Processes the command line parameters.
 */

int parmer ( argc, argv )
int	argc ;
char	*argv[] ;
{
int	i ;
	
/* default io streams */
in = stdin ;
out = stdout ;

/* use in_buff to hold file inclusion command if found (1 only) */
IN_BUFF_DONE ; 		/* clear the buffer */


for ( i = 1; i < argc; i++ ) {

	/* assume data file name if not a switch */
	if ( argv[i][0] != '-' ) {
		sprintf( dataf, "%s.p", argv[i] ) ;
		if ( NULL != ( in = fopen( dataf, "r" ) ) ) {
			sprintf( dataf, "%s.f", argv[i] ) ;
			out = fopen( dataf, "w" ) ;
		}
		else in = stdin ;
	}
	
	else {
	/* switches */
		switch ( argv[i][1] ) {

		case 'c' : case 'C' :
			com_keep = TRUE ;	break ;

		case 'u' : case 'U' :
			underline_keep = TRUE ;	break ;

		case 'r' : case 'R' :
			i++ ;
			if ( i < argc ) {
			if ( argv[i][0] == '-' ||
				NULL == sscanf(argv[i],"%d",&unroll_depth)) {
				unroll_depth = DEF_UNROLL_DEPTH ;
				i-- ;
				break ;
			}}
			else	unroll_depth = DEF_UNROLL_DEPTH ;
			break ;

		case 'l' : case 'L' :
			i++ ;
			if ( i < argc ) {
			if ( argv[i][0] == '-' ||
				NULL == sscanf(argv[i],"%d",&line_limit)) {
				line_limit = DEF_LINE_LIMIT ;
				i-- ;
				break;
			}}
			else	line_limit = DEF_LINE_LIMIT ;
			break ;

		case 'm' : case 'M' :
			macro_only = TRUE ;
			underline_keep = TRUE ;
			com_keep = TRUE ;
			break ;
		
		case 'i' : case 'I' :
			i++ ;
			if ( i < argc && *in_buff != '#' ) {
				sprintf(in_buff,
					"#include \"%s\"", argv[i] ) ;
				break ;
			}
			else goto ERROR ;
		
		case 'd' : case 'D' :
			i++ ;
			if ( i < argc ) {
				sprintf(out_buff, ":%s 1;", argv[i] ) ;
				define_macro( out_buff ) ;
				break ;
			}
		
ERROR:
	
default:fprintf( stderr, "\nUnrecognized switch: %s\n", argv[i]);
	fprintf( stderr,
	"\nAllowed switches:\n\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
	" -c		keep comments",
	" -u		keep underline characters",
	" -m		expand macros only",
	" -i <file>	include <file> before processing (1 only)",
	" -d <name>	define <name> as a macro ( :name 1; )",
	" -r n		unroll vector loops to depth n",
	" -l n		unroll loops with n or fewer lines only",
	"All items must be separated by blanks"
	) ;
	abort( "\n" ) ;
	}
	}
}

/* process the file include statement if present */
preproc( rec_type(0) ) ;
}




/* Function PREPROCESS.C
 *
 * The guts of the preprocessor PREP.  Variable tipe
 * contains the type of record code:
 *
 *  BEGIN statement
 *  AGAIN statement
 *  WHILE statement
 *  UNTIL statement
 *  CONTINUE statement
 *  LEAVE statement
 *
 *  CASE statement
 *  OF statement
 *  DEFAULT statement
 *  CONTINUE_CASE statement
 *  END_CASE statement
 *  DO_LIMITS statement
 *  UNROLL statement
 *
 *  DO statement
 *  LEAVE_DO statement
 *  CONTINUE_DO statement
 *  END_DO statement
 *
 *  [  (start of clustered vector arithmetic)
 *  ]  (  end  "     "        "       "     )
 *  #  vectored arithmetic statement
 *  normal (normal fortran statement)
 *
 *  INCLUDE files
 *  conditional compilation:
 *	 #ifdef, #ifndef, #else, #endif
 *
 * Returns 1 if the buffer is still full, else 0.  The individual
 * routines are expected to clear the buffer (in_buff) to signal
 * that no more processing is required.
 *
 * P. R. OVE  11/9/85
 */

int preproc(tipe)
int tipe ;
{

switch ( tipe ) {

	case unknown :		break ;
	case normal :		strcpy( out_buff, in_buff ) ;
				dump( out_buff ) ;
				in_buff[0] = NULL ;
				break ;
	case type_begin :	begin_proc() ; break ;
	case type_again :	again_proc() ; break ;
	case type_while :	while_proc() ; break ;
	case type_until :	until_proc() ; break ;
	case type_continue :	continue_proc() ; break ;
	case type_leave :	leave_proc() ; break ;
	case type_case :	case_proc() ; break ;
	case type_of :		of_proc() ; break ;
	case type_default :	default_proc() ; break ;
	case type_continue_case:continue_case_proc() ; break ;
	case type_end_case :	end_case_proc() ; break ;
	case type_do_limits :	do_limits_proc() ; break ;
	case type_unroll :	unroll_proc() ; break ;
	case type_do :		do_proc() ; break ;
	case type_end_do :	end_do_proc() ; break ;
	case type_leave_do :	leave_do_proc() ; break ;
	case type_continue_do :	continue_do_proc() ; break ;
	case type_osqb :	osqb_proc() ; break ;
	case type_vec : 	vec_proc() ; break ;
	case type_csqb :	csqb_proc() ; break ;
	case type_include :	include_proc() ; break ;
	case type_ifdef :	ifdef_proc() ; break ;
	case type_ifndef :	ifndef_proc() ; break ;
	case type_else :	else_proc() ; break ;
	case type_endif :	endif_proc() ; break ;
                      
}

return( line_end( in_buff ) != NULL ) ;
}




/* PUSH
 *
 * Push a string onto the MEM_STORE.  Space is allocated for it and
 * a pointer kept in the array mem_store (array of pointers).  The
 * index to mem_store at which the current string is stored is returned.
 * If the input string is a NULL pointer the last entry is removed.
 * Global variable mem_count keeps track of the total number of pointers
 * in use.
 */

int push( string )
char	*string ;
{
int	i ;

if ( string != NULL ) {
	if ( mem_count >= STORE_SIZE - 1 ) {
		sprintf( errline, "PUSH out of memory pointers: %s", in_buff ) ;
		abort( errline ) ;
	}
	GET_MEM( mem_store[ mem_count ], strlen( string ) ) ;
	strcpy( mem_store[ mem_count ], string ) ;
	mem_count++ ;
	return( mem_count - 1 ) ;
}

if ( mem_count > 0 ) {
	mem_count-- ;
	free( mem_store[ mem_count ] ) ;
	return( mem_count - 1 ) ;
}

/* already empty if it gets here */
return( -1 ) ;
}



/* Function REC_TYPE.C
 *
 * Determine the type of a record.
 *
 * P. R. OVE  11/9/85
 */

char	*strchrq() ;

int	rec_type( group )
int	group ;
{                  
char	combuff[16], *string ;
int	i ;

if (in_buff[0] == NULL) return((int)unknown) ;
string = in_buff ;

/* go to first nonblank character, save a pointer to it */
while ( *string != NULL ) {
	if ( *string != TAB && *string != BLANK ) {	
		first_nonblank = string ;
		break ;
	}
	string++ ;
}

/* copy the initial characters into combuff */
for ( i = 0; (i < 15) && (*string != NULL); i++ ) {
	combuff[i] = string[i] ;
}
combuff[15] = NULL ;

strupr( combuff ) ;  /* convert to upper case */


	 
/* check for commands by group */
switch ( group ) {


/* group 0 commands: file includes & conditional compilation */
case 0 : {
	if ( MATCH( "#INCLUDE" ) ) return((int)type_include) ;
	if ( MATCH( "#IFDEF" ) )   return((int)type_ifdef) ;
	if ( MATCH( "#IFNDEF" ) )  return((int)type_ifndef) ;
	if ( MATCH( "#IF" ) )      return((int)type_ifdef) ;
	if ( MATCH( "#ELSE" ) )    return((int)type_else) ;
	if ( MATCH( "#ENDIF" ) )   return((int)type_endif) ;
		                   return((int)unknown) ;
}


/* group 1 commands: case's OF and DEFAULT commands are done first so
   that it is legal to have:  of ( 'a' ) leave_do, for instance.
*/
case 1 : {
	if ( MATCH( "OF" ) )        return((int)type_of) ;
	if ( MATCH( "DEFAULT" ) )   return((int)type_default) ;
			            return((int)unknown) ;
}


/* group 2 commands: flow control extensions and parameter changes */
case 2 : {
	if ( MATCH( "DO_LIMITS" ) ) return((int)type_do_limits) ;
	if ( MATCH( "DO LIMITS" ) ) return((int)type_do_limits) ;

	if ( MATCH( "DO" ) )        return((int)type_do) ;
	if ( MATCH( "END_DO" ) )    return((int)type_end_do) ;
	if ( MATCH( "END DO" ) )    return((int)type_end_do) ;
	if ( MATCH( "ENDDO" ) )     return((int)type_end_do) ;
	if ( MATCH( "LEAVE_DO" ) )  return((int)type_leave_do) ;
	if ( MATCH( "LEAVE DO" ) )  return((int)type_leave_do) ;
	if ( MATCH( "CONTINUE_DO")) return((int)type_continue_do) ;
	if ( MATCH( "CONTINUE DO")) return((int)type_continue_do) ;

	if ( MATCH( "CASE" ) )      return((int)type_case) ;
	if ( MATCH( "END_CASE" ) )  return((int)type_end_case) ;
	if ( MATCH( "END CASE" ) )  return((int)type_end_case) ;
	if (MATCH("CONTINUE_CASE")) return((int)type_continue_case) ;
	if (MATCH("CONTINUE CASE")) return((int)type_continue_case) ;

	if ( MATCH( "BEGIN" ) )     return((int)type_begin) ;
	if ( MATCH( "AGAIN" ) )     return((int)type_again) ;
	if ( MATCH( "WHILE" ) )     return((int)type_while) ;
	if ( MATCH( "UNTIL" ) )     return((int)type_until) ;
	if ( MATCH( "LEAVE" ) )     return((int)type_leave) ;
	if ( MATCH( "CONTINUE" ) )  return((int)type_continue) ;

	if ( MATCH( "UNROLL" ) )    return((int)type_unroll) ;
				    return((int)unknown) ;
}


/* group 3 commands: vector processing */
case 3: {
	if ( MATCH( "[" )	)             return((int)type_osqb) ;
	if ( strchrq( string, ']' ) != NULL ) return((int)type_csqb) ;
	if ( strchrq( string, '#' ) != NULL ) return((int)type_vec) ;
					      return((int)normal) ;
}
} /* end switch case */


/* control should never get here */
sprintf( errline, "REC_TYPE: invalid group %d", group ) ;
abort( errline ) ;
return((int)unknown) ;	/* here to avoid compiler warning (Gould) */
}



/* Comment and blank line filtering.
 *
 *    This routine also trims off characters after a ";", so that
 * this symbol is can be used for comments on the same line.  If
 * the first character on the line is ":" this is not done, since
 * removing the trailing semicolon would cause a macro def error.
 * The macro definition routine will eliminate anything after the
 * ";" anyway.  Blank lines are also killed off here.  Returns a
 * 1 if the line was entirely a comment and processed, else 0.
 */		
int comment_filter()
{
char	*start, *semi ;

start = line_end( in_buff ) ;

/* handle lines with comment character in 1st column, and blank lines */
if ( (*in_buff == 'c') || (*in_buff == 'C') ||
     (*in_buff == ';') || (start == NULL)	) {
		if ( com_keep ) {
		if ( NOT macro_only ) in_buff[72] = NULL ;
		if ( *in_buff == ';' ) *in_buff = 'c' ;
		put_string( in_buff ) ;
	}
	return(1) ;
}

/* trim off text after ; if not a macro def */
if ( NOT macro_only && *start != ':' ) {
	if ( NULL != ( semi = strchrq( in_buff, ';' ) ) ) *semi = NULL ;
}
return(0) ;
}



/* Look for unquoted character in string, where ' is the fortran quote char.
 * Returns a pointer to the character, or a NULL pointer if not present.
 */
char	*strchrq( string, c )
char	*string, c ;
{
int	i, quote=1 ;

for ( i = 0; string[i] != NULL; i++ ) {
	if ( string[i] == '\'' ) {
		quote = -quote ;
		continue ;
	}
	if ( string[i] == c && quote == 1 ) return( &string[i] ) ;
}

return( NULL ) ;	/* not found */
}





/* STRMATCH:  find the first occurrence of string2 in string1, return pointer
 * to the first character of the match.  Returns NULL pointer if no match.
 * This routine is fairly slow and should not be used where speed is
 * critical.
 */
#define NULL	0

char	*strmatch( string1, string2 )
char	*string1, *string2 ;
{
char	*pntr1, *pntr2 ;

 	for ( pntr1 = string1, pntr2 = string2 ; *pntr1 != NULL; pntr1++ ) {
		if ( *pntr1 == *pntr2 ) {
			pntr2++ ;
			if ( *pntr2 == NULL ) return( pntr1 - strlen(string2) + 1 ) ;
		}
		else pntr2 = string2 ;
	}

	/* failure if control reaches this point */
	return( NULL ) ;
}




/* function STRTOKP

   Like Strtok, except that the original string is preserved (strtok
   puts null in there to terminate the substrings).  This routine
   uses mallocs to allow storage for the token.  The memory is
   reallocated for each new string.  Use just like strtok:
   
   Successively returns the tokens in string1, using the delimeters
   defined by string2.  If string1 is NULL (a NULL pointer) the 
   routine returns the next token in the string from the previous call.
   Otherwise the first token is returned.  A NULL pointer is returned
   on failure (no more tokens in the current string).
*/

char *strtokp( string1, string2 )
char	*string1, *string2 ;
{
static char	*spntr, *tpntr, *token ;
static int	called = NULL ;		/* called=NULL ==> initialize */
int	i ;

/* initialize on first call */
	if ( called == NULL ) {
		called = 1 ;
		GET_MEM( token, strlen(string1) ) ;
	}

/* if string1 is not NULL reset the routine */
	if ( string1 != NULL ) {
		spntr = string1 ;
		if ( NULL == ( token = realloc( token, strlen(string1)+1 )))
			abort("STRTOKP: reallocation error") ;
	}
	if ( *spntr == NULL ) return( NULL ) ;	/* end of original string */

/* skip	initial delimeter characters */
	for (; NULL != strchr( string2, *spntr ); spntr++ ) ;

/* copy characters to token until the next delimeter */
	tpntr = &token[0] ;
	for (; *spntr != NULL; spntr++ ) {
		if ( NULL != strchr( string2, *spntr ) ) break ;
		*tpntr = *spntr ;
		tpntr++ ;
	}
	*tpntr = NULL ;

/* return result to caller */
	if ( token[0] == NULL ) return( NULL ) ;
	return( &token[0] ) ;
}




/* strupr: convert a string to upper case.
 */

char	*strupr( string )
char	*string ;
{
int	i ;

	for ( i=0; i<strlen( string ); i++ )
		if ( string[i] > 96 && string[i] < 123 ) string[i] -= 32 ;

	return( string ) ;
}




/* Tokenize
 *
 * Break out arguments from a string.  Pntr is the argument string
 * and tokens is an array of pointers which will be assigned memory and have
 * the arguments returned.  The function returns the number of arguments
 * found.  Pairwise characters are monitored to ensure that expressions
 * are sexually balanced.  Unused parm pointers are returned NULL.
 * MAX_TOKENS determines the dimension of the array of pointers.
 * Commas are the only delimiters allowed to distinquish tokens.
 */
 
int	tokenize( pntr, tokens )
char	*pntr, *tokens[] ;
{
int	square = 0, curl = 0, parens = 0, apost = 1, quote = 1 ;
int	i, j, quit ;
char	*text, *txt ;

/* clear the pointers and make a copy of the string */
for ( i=0; i<MAX_TOKENS; i++ ) tokens[i] = NULL ;
GET_MEM( text, strlen(pntr) ) ;
strcpy( text, pntr ) ;

for ( i=0, j=0, quit=FALSE, txt=text; quit==FALSE; j++ ) {

	switch( text[j] ) {

	case '['  :	square += 1 ;	break ;
	case ']'  :	square -= 1 ;	break ;
	case '{'  :	curl   += 1 ;	break ;
	case '}'  :	curl   -= 1 ;	break ;
	case '('  :	parens += 1 ;	break ;
	case ')'  :	parens -= 1 ;	break ;
	case '\'' :	apost = -apost;	break ;
	case '\"' :	quote = -quote;	break ;
	case NULL :	
			GET_MEM( tokens[i], strlen(txt) ) ;
			strcpy( tokens[i], txt ) ;
			quit = TRUE ;
			break ;
	case ','  :	if (!square && !curl && !parens &&(apost==1)&&(quote==1)){
				text[j] = NULL ;
				GET_MEM( tokens[i], strlen(txt) ) ;
				strcpy( tokens[i], txt ) ;
				i += 1 ;
				txt = &text[j+1] ;
			}
	}
}

free( text ) ;
return( i+1 ) ;
}
