/* Routines related to vector shorthand extensions */

#include "prep.h"

char	*initial_name[NESTING] ;	/* do loop initial values */
char	*limit_name[NESTING] ;		/* do loop limits */
char	*increment_name[NESTING] ;	/* do loop increments */
char	label[NESTING][6] ;		/* label storage (vector loops) */
char	var_name[NESTING][6] ;		/* do counter names */

int	var_count = 0 ;			/* number of vars used in do loops */
int	label_count = 0 ;		/* label = label_count + 10000 */



/* VEC_INIT
 *
 * Initialize the vec routines
 */
vec_init()
{
int i ;

for ( i = 0; i < NESTING; i++ ) sprintf( var_name[i], "i%03d", i ) ;
}



/* Function CSQB_PROC.C
 *
 * Process close square brackets.  Abort if called while
 * not in a vector loop, else finish off vector loop processing
 * with a call to end_vec.
 *
 * P. R. OVE  11/9/85
 */

csqb_proc() 
{
int	i, quote=1 ;

/* if vec_flag not set this call is an error */
if ( NOT vec_flag ) {
	sprintf( errline, "CSQB: not in vector loop: %s", in_buff ) ;
	abort( errline ) ;
}
                      
/* see what in_buff contains and replace unquoted ] by NULL */
for ( i = 0; in_buff[i] != NULL; i++ ) {
	switch ( in_buff[i] ) {
	
	case '\'' :	quote = -quote ;
			break ;
	case ']' :	if ( quote == 1 ) {
				in_buff[i] = NULL ;
				i-- ;		/* force termination */
				break ;
			}
	}
}

dump( in_buff ) ;	/* --> mem_store */
end_vec();		/* terminate vector loop */

IN_BUFF_DONE ;
}




/* Function DO_LIMITS_PROC
 *
 * Process do_limits statements: Parse variable string.
 *
 * P. R. OVE  11/9/85
 */

char	*tokens[MAX_TOKENS] ;

do_limits_proc()
{                  
int	i, j, k ;
char	*temp[MAX_TOKENS], *open_parens, *close_parens ;

/* free allocation from previous call */
free_loop_vars() ;

/* find the open and close delimeters */
open_parens = &in_buff[ strcspn( in_buff, "[({\'\"" ) ] ;
if ( NULL == ( close_parens = mat_del( open_parens ) ) ) {
	sprintf( errline, "DO_LIMITS: missing delimeter: %s", in_buff ) ;
	abort( errline ) ;
}
*close_parens = NULL ;	/* make arg string null terminated */


/* get the (initial,limit,increment) triples */
var_count = tokenize( open_parens+1, tokens ) ;

/* handle wierd numbers of tokens */
if ( var_count <= 0 ) abort( "ERROR: no variables found" ) ;
for ( i = NESTING; i < var_count; i++ ) {
	var_count = NESTING ; free( tokens[i] ) ; }


/* At this stage the tokens are strings like
 *
 *  "(initial , limit , increment)  ==>  do i = initial, limit, increment.
 *
 * If one is missing it is assumed to be the increment.  If two are
 * missing the single item is assumed to be the limit.  The parens are
 * unnecessary if there is only the limit.
 *
 * break out the tokens (delimeted by commas)
 */
alloc_loop_vars() ;
for ( i = 0; i < var_count; i++ ) {

	/* find the open and close delimeters if present, and handle them*/
	open_parens = &tokens[i][ strcspn( tokens[i], "[({\'\"" ) ] ;
	if ( NULL != ( close_parens = mat_del( open_parens ) ) ) {
		*close_parens = NULL ;
		*open_parens = BLANK ;
	}

	k = tokenize( tokens[i], temp ) ;

	/* case of too many tokens, ignore trailers */
	for ( j = 3; j < k; j++ ) { k = 3 ; free( temp[j] ) ; }

	switch ( k ) {
	case 1:	strcpy(initial_name[i], "1") ;
		sprintf(limit_name[i], "(%s)", temp[0]) ; free( temp[0] ) ;
		strcpy(increment_name[i], "1") ;
		break;

	case 2:	sprintf(initial_name[i], "(%s)", temp[0]) ; free( temp[0] ) ;
		sprintf(limit_name[i], "(%s)", temp[1]) ; free( temp[1] ) ;
		strcpy(increment_name[i], "1") ;
		break;

	case 3:	sprintf(initial_name[i], "(%s)", temp[0]) ; free( temp[0] ) ;
		sprintf(limit_name[i], "(%s)", temp[1]) ; free( temp[1] ) ;
		sprintf(increment_name[i], "(%s)", temp[2]) ; free( temp[2] ) ;
		break;

	default:strcpy(initial_name[i], "1") ;
		sprintf(limit_name[i], "(%s)", "undefined" ) ;
		strcpy(increment_name[i], "1") ;
		break;
	}
}				

IN_BUFF_DONE
}

/* release allocation from previous call */
free_loop_vars() {
int	i ;

for ( i = 0; i < var_count; i++ ) {
	free( tokens[i] ) ;
	free( initial_name[i] ) ;
	free( limit_name[i] ) ;
	free( increment_name[i] ) ;
}
}

/* allocate space for do loop variables */
alloc_loop_vars() {
int	i, size ;

for ( i = 0; i < var_count; i++ ) {
	size = strlen( tokens[i] ) + 10 ;
	GET_MEM( initial_name[i], size ) ;
	GET_MEM( limit_name[i], size ) ;
	GET_MEM( increment_name[i], size ) ;
}
}




/* Function END_VEC.C
 *
 * This routine is called when a cluster of vector arithmetic
 * is ready to be terminated (a closing ] has been found
 * or the statement was a single line vector * statement.  The
 * core of the loop has by now been pushed into MEM_STORE and
 * will now be extracted and processed.  On completion MEM_STORE
 * is released.
 *
 * P. R. OVE  11/9/85
 */

end_vec() 
{
int	i, j ;

/* reset the flag */
vec_flag = FALSE ;

make_do() ;	/* write the initial do loop statements */

if ( NOT UNROLLING ) {
	/* process all of the pushed statements through transvec */
	for ( i = 0; i < mem_count; i++ )
		transvec( mem_store[i], 0 ) ;

	make_continue() ;	/* write continue statements */
}

else {
	/* process the statements though transvec unroll_depth times */
	for ( j = 0; j < unroll_depth; j++ ) {
		for ( i = 0; i < mem_count; i++ )
			transvec( mem_store[i], j ) ;
	}
	make_continue() ;

	/* write the clean up part of the unrolled loop */
	make_labels() ;
	make_clean_do() ;
	for ( i = 0; i < mem_count; i++ )
		transvec( mem_store[i], 0 ) ;
	make_continue() ;
}

/* release the memory held by MEM_STORE and return to main level */
while ( push(NULL) >= 0 ) ;
IN_BUFF_DONE
}




/* Make the initial do statements */
make_do() {
int	i ;

/* outermost do statement is different if unrolling is on */
i = var_count - 1 ;

if ( UNROLLING ) {
/* This section unrolls: do i = a, b, c   (depth = d)   into
 *
 *             b-a+c
 * do i = a, (-------)*(c*d) + a - c, c*d  
 *              c*d
 *
 * for the outermost loop.  Inner loops are unchanged.
 */
	sprintf( out_buff,
	"      do %s %s=%s,int((1.0*(%s-%s+%s))/(%s*%d))*%s*%d+%s-%s,%s*%d",
		label[i], var_name[i], initial_name[i],
		limit_name[i], initial_name[i], increment_name[i],
		increment_name[i], unroll_depth,
		increment_name[i], unroll_depth,
		initial_name[i], increment_name[i],
		increment_name[i], unroll_depth ) ;
	dump( out_buff ) ; }
else {
	sprintf( out_buff, "      do %s %s = %s, %s, %s",
		label[i], var_name[i],
		initial_name[i], limit_name[i], increment_name[i] ) ;
	dump( out_buff ) ; }

/* handle the rest of the do statements */
for ( i = var_count-2; i >= 0; i-- ) {
	sprintf( out_buff, "      do %s %s = %s, %s, %s",
		label[i], var_name[i],
		initial_name[i], limit_name[i], increment_name[i] ) ;
	dump( out_buff ) ; }
}




/* make the do statements for the clean up part of the unrolled loop */
make_clean_do() {
int	i ;

/* make the outer do statement.
 * This section unrolls: do i = a, b, c   (depth = d)   into
 *
 *          b-a+c
 * do i = (-------)*(c*d) + a, b, c
 *           c*d
 *
 * for the outermost loop.  Inner loops are unchanged.  The initial
 * value is the first element that missed the main do loop */
i = var_count - 1 ;
sprintf( out_buff,
	"      do %s %s=int((1.0*(%s-%s+%s))/(%s*%d))*%s*%d+%s,%s,%s",
	label[i], var_name[i],
	limit_name[i], initial_name[i], increment_name[i],
	increment_name[i], unroll_depth,
	increment_name[i], unroll_depth,
	initial_name[i], limit_name[i], increment_name[i] ) ;
dump( out_buff ) ;

/* make the remaining do statements */
for ( i = var_count-2; i >= 0; i-- ) {
	sprintf( out_buff, "      do %s %s = %s, %s, %s",
		label[i], var_name[i],
		initial_name[i], limit_name[i], increment_name[i] ) ;
	dump( out_buff ) ;
}
}


/* make the continue statements */
make_continue() {
int	i ;

for ( i = 0; i < var_count; i++ ) {
	sprintf( out_buff, "%s continue", label[i] ) ;
	dump( out_buff ) ; }
}




/* Function MAKE_LABELS.C
 *
 * Make var_count labels, starting with label_count
 * + 10000.
 *
 * P. R. OVE  11/9/85
 */

make_labels()
{                  
int	i, count ;
                    
for ( i = 0; i < var_count; i++ ) {
 	
	count = 10000 + label_count ;
	label_count++ ;              
	if ( count > 12499 ) { 
		sprintf( errline, "MAKE_LABELS: too many labels: %s", in_buff ) ;
		abort( errline ) ;
	}
	sprintf( label[i], "%d", count ) ;
}
}



/* Function OSQB_PROC.C
 *
 *   Process open square brackets.  This routine will be
 * called when an open square bracket is found in the
 * record (start cluster of vector arithmetic).  It sets
 * up the labels and sets vec_flag so that dump will direct
 * output to mem_store instead of the output file.
 *   The initial do statements are not written here, so that
 * unrolling can be turned off if there are too many lines
 * ( > line_limit ) in the loop.  Endvec will write them.
 *   If a closing ] is also found in the same record then
 * the statement is passed through transvec immediately, since
 * it has already been processed by the rest of the preprocessor.
 *
 * P. R. OVE  11/9/85
 */

osqb_proc() 
{
int	i, quote=1 ;

/* if default loop limits have not been set abort here */
if ( var_count <= 0 ) {
	sprintf( errline, "Vector loop without default limits set: %s", in_buff ) ;
	abort( errline ) ;
}

make_labels() ;		/* get a list of labels */

vec_flag = TRUE ;	/* now force output --> mem_store */
                      
/* see what in_buff contains and replace unquoted [] by blanks */
for ( i = 0; in_buff[i] != NULL; i++ ) {

	switch ( in_buff[i] ) {
	
	case '\'' :	quote = -quote ;
			break ;
	case '[' :	if ( quote == 1 ) {
				in_buff[i] = BLANK ;
				break ;
			}
	case ']' :	if ( quote == 1 ) {
				vec_flag = FALSE ;
				in_buff[i] = BLANK ;
				break ;
			}
	}
}

/* if there is a closing ] process the line now */
if ( NOT vec_flag ) {
	vec_flag = TRUE ;	/* force line to mem_store */
	dump( in_buff ) ;
	end_vec() ;		/* flag will be reset here */
}
else dump( in_buff ) ;		/* this will go to mem_store */

IN_BUFF_DONE ;
}




/* Function TRANSVEC.C
 *
 * Translate a record of vectored arithmetic and expand
 * out the # signs.  The resulting expanded record is
 * placed in out_buff and dumped.  The second argument
 * is related to unrolling, and is the amount to be
 * added to the index of the outermost loop.  This
 * should be zero if unrolling is off.  Quoted characters
 * are ignored ( ' is the fortran quote character ).
 *
 * P. R. OVE  11/9/85
 */

/* copy character verbatim to the output buffer */
#define	VERBATIM	out_buff[i_out] = string[i_in] ;\
			out_buff[i_out + 1] = NULL ;	\
			i_out++ ;


transvec( string, outer_loop_inc ) 
char	*string ;
int	outer_loop_inc ;
{
int	i_in, i_out = 0, i_var = 0, quote = 1 ;
char	*pntr ;

/* make string version of loop counter increment */
if ( UNROLLING ) {
	GET_MEM( pntr, strlen(increment_name[var_count-1])
		     + abs(outer_loop_inc) + 10 ) ;
	sprintf( pntr, "+%s*%d", increment_name[ var_count - 1 ],
		outer_loop_inc ) ;
}

/* loop over the input record */
for ( i_in = 0; string[i_in] != NULL; i_in++ ) {

/* pass characters straight through if quoted */
if ( string[i_in] == '\'' ) quote = -quote ;
if ( quote == -1 ) {
	VERBATIM ;
	continue ;
}

switch( string[i_in] ) {

	/* replace #'s with variable names */
	case '#' :	strcat( out_buff, var_name[i_var] ) ;
			i_out += 4 ;
			i_var++ ;   
			if ( i_var >= var_count ) {
				i_var = 0 ;
				if (UNROLLING && outer_loop_inc != 0) {
					strcat( out_buff, pntr ) ;
					i_out += strlen( pntr ) ;
				}
			}
			break ;

	/* reset variable counter */
	case ')' :	out_buff[i_out] = ')' ;
			out_buff[i_out + 1] = NULL ;
			i_out++ ;
			i_var = 0 ;
			break ;

	/* copy character verbatim */
	default : 	VERBATIM ;

}
}

if (UNROLLING) free( pntr ) ;
dump( out_buff ) ;

IN_BUFF_DONE ;
}




/* Function UNROLL_PROC
 *
 * Change the unrolling depth.  If depth is less than 2 unrolling is off.
 *
 * P. R. OVE  6/18/86
 */

unroll_proc()     
{                  
int	n ;
char	*open_parens, *close_parens ;

/* get the expression delimeters */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if there is stuff on the line (open_parens != NULL) and no            */
/* open parens (close_parens == NULL) assume variable name like UNROLLit */
if ( (open_parens != NULL) && (close_parens == NULL) ) return ;

/* get the depth if it is there (error ==> depth = 0 (OFF)) */
if (open_parens != NULL) {
	n = close_parens - open_parens - 1 ;
	*close_parens == NULL ;
	unroll_depth = atoi( open_parens + 1 ) ;
}
else {	unroll_depth = DEF_UNROLL_DEPTH ; }

IN_BUFF_DONE
}




/* Function VEC_PROC.C
 *
 * This routine's functions when a "naked"
 * (with out surrounding [ ]) vector statement is found.
 * The action depends on whether vec_flag is set or not.
 * If set:
 *   The record is dumped (to mem_store).
 * If not:
 *   It is handled by placing a [ at the beginning and a
 * ] at the end and then starting over.  OSQB_PROC will
 * then trap it and pass it to END_VEC to be processed.
 *
 * P. R. OVE  11/9/85
 */

vec_proc()
{
int	i, length ;

/* if default loop limits have not been set abort here */
if ( var_count <= 0 ) {
	sprintf( errline, "Vector loop without default limits set: %s", in_buff ) ;
	abort( errline ) ;
}
                      
if ( vec_flag ) {
	dump( in_buff ) ;	/* --> mem_store */
	IN_BUFF_DONE ;
}
else {
	length = strlen( in_buff ) ;
	for ( i = length - 1; i >= 0; i-- ) in_buff[i+1] = in_buff[i] ;
	in_buff[ length + 1 ] = ']' ;
	in_buff[ length + 2 ] = NULL ;
	in_buff[ 0 ] = '[' ;
}
}
