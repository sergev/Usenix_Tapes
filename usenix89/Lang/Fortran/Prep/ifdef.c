/* Routines related to conditional compilation.  Ignore_flag is
 * a global external that controls input.  If ignore_flag is TRUE
 * input is ignored.  File inclusion stuff is also here.
 */

#include "prep.h"

int	ifdef_list[NESTING], ifdef_count ;




/* Function IFDEF_PROC
 *
 * #ifdef name1 name2 name3....namen
 * 
 * Different from the cpp conditional compilation directive, since
 * in PREP the symbols | and & (and nearly anything) are legal macro
 * names.  Here the instructions in the #if block will be kept if
 * ANY of the names are defined.  The names must be separated by
 * blanks or tabs.
 */

ifdef_proc()     
{                  
int	i ;
char	*name, *pntr ;

/* keep track of the nesting */
ifdef_count++ ;
if ( ifdef_count >= NESTING ) {
	sprintf( errline, "#Ifdef: nesting too deep: %s", in_buff ) ;
	abort( errline ) ;
}

/* see if any of the tokens is a macro name */
i = ifdef_count - 1 ;
ifdef_list[i] = FALSE ;
for (pntr = first_nonblank + name_length;; pntr = NULL ) {
	if ( NULL == ( name = strtok( pntr, " \t" ) ) ) break ;
	if ( mac_query(name) >= 0 ) {
		ifdef_list[i] = TRUE ;
		break ;
	}
}

/* set a flag to inhibit input if any ifdef flags are FALSE */
ignore_flag = FALSE ;
for ( i=0; i<ifdef_count; i++ )
	if ( ifdef_list[i] == FALSE ) ignore_flag = TRUE ;

/* signal that in_buff is empty */
IN_BUFF_DONE
}



/* Function IFNDEF_PROC
 *
 * #ifndef name1 name2 name3....namen
 * 
 * Here the instructions in the #ifndef block will be kept if
 * ANY of the names are NOT defined.  The names must be separated by
 * blanks or tabs.
 */

ifndef_proc()     
{                  
int	i ;
char	*name, *pntr ;

/* keep track of the nesting */
ifdef_count++ ;
if ( ifdef_count >= NESTING ) {
	sprintf( errline, "#Ifdef: nesting too deep: %s", in_buff ) ;
	abort( errline ) ;
}

/* see if any of the tokens is not a macro name */
i = ifdef_count - 1 ;
ifdef_list[i] = FALSE ;
for (pntr = first_nonblank + name_length;; pntr = NULL ) {
	if ( NULL == ( name = strtok( pntr, " \t" ) ) ) break ;
	if ( mac_query(name) < 0 ) {
		ifdef_list[i] = TRUE ;
		break ;
	}
}

/* set a flag to inhibit input if any ifdef flags are FALSE */
ignore_flag = FALSE ;
for ( i=0; i<ifdef_count; i++ )
	if ( ifdef_list[i] == FALSE ) ignore_flag = TRUE ;

/* signal that in_buff is empty */
IN_BUFF_DONE
}



/* ELSE_PROC
 *
 * #else conditional compilation directive.
 */
else_proc()
{
int	i ;

/* on missing #ifdef statement, abort */
if ( ifdef_count <= 0 ) {
	sprintf( errline, "#Else: no matching ifdef: %s", in_buff ) ;
	abort( errline ) ;
}

ifdef_list[ ifdef_count-1 ] = NOT ifdef_list[ ifdef_count-1 ] ;

/* set a flag to inhibit input if any ifdef flags are FALSE */
ignore_flag = FALSE ;
for ( i=0; i<ifdef_count; i++ )
	if ( ifdef_list[i] == FALSE ) ignore_flag = TRUE ;

/* signal that in_buff is empty */
IN_BUFF_DONE
}



/* ENDIF_PROC
 *
 * #endif conditional compilation directive.
 */
endif_proc()
{
int	i ;

/* on missing #ifdef statement, abort */
if ( ifdef_count <= 0 ) {
	sprintf( errline, "#Endif: no matching ifdef: %s", in_buff ) ;
	abort( errline ) ;
}

ifdef_count-- ;

/* set a flag to inhibit input if any ifdef flags are FALSE */
ignore_flag = FALSE ;
for ( i=0; i<ifdef_count; i++ )
	if ( ifdef_list[i] == FALSE ) ignore_flag = TRUE ;

/* signal that in_buff is empty */
IN_BUFF_DONE
}



/* INCLUDE_PROC
 *
 * Handle file inclusion
 *
 * P. R. OVE  11/9/85
 */
 
include_proc()     
{                  
char	*pntr, *open_parens, *close_parens, *name ;

/* This routine could be called when the conditional compilation
 * flag has been set (#include is in the same group).
 */
if ( ignore_flag ) { IN_BUFF_DONE ; return ; }

/* get the file name */
open_parens = line_end( first_nonblank + name_length ) ;
if ( NULL == ( close_parens = mat_del( open_parens ) ) ) {
	sprintf( errline, "INCLUDE: syntax: %s", in_buff ) ;
	abort( errline ) ;
}
name = open_parens+1 ;
*close_parens = NULL ;

/* push the old input file handle onto the filestack */
if ( NULL == pushfile(&in) ) {
	sprintf( errline, "INCLUDE: nesting too deep: %s", in_buff ) ;
	abort( errline ) ;
}

/* open the new file */
if ( NULL == ( in = fopen( name, "r" ) ) ) {
	sprintf( errline, "INCLUDE: can't open file: %s", name ) ;
	abort( errline ) ;
}

IN_BUFF_DONE ;
}


/* push a file handle onto the filestack.  return NULL on error. */
int	pushfile(handleaddress)
FILE	*(*handleaddress) ;
{
	if ( include_count >= NESTING ) return(NULL) ;
	filestack[include_count] = *handleaddress ;
	include_count++ ;
	return(1) ;
}


/* pop a file handle from the filestack.  return NULL on error */
int	popfile(handleaddress)
FILE	*(*handleaddress) ;
{
	if ( include_count <= 0 ) return(NULL) ;
	include_count-- ;
	*handleaddress = filestack[include_count] ;
	return(1) ;
}

