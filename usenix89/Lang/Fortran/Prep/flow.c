/* Flow control extensions and related routines */

#include "prep.h"

/* data declarations for the routines in the flow control set */
char	*case_exp[NESTING] ;		/* case expression storage */
char	*exp ;				/* general expression storage pointer */
char	alabel[NESTING][6] ;		/* again label storage */
char	blabel[NESTING][6] ;		/* begin label storage */
char	clabel[NESTING][6] ;		/* case label storage */
char	dlabel[NESTING][6] ;		/* do/end_do label storage */
char	elabel[NESTING][6] ;		/* leave_do label storage */

int	of_count[NESTING] ;   /* counters for of statements */
int	leave_do_flag[NESTING] ;   /* marks if leave_do in current loop */

int	alabel_count = 0 ;    /* alabel = alabel_count + 15000 */
int	blabel_count = 0 ;    /* blabel = blabel_count + 17500 */
int	clabel_count = 0 ;    /* clabel = clabel_count + 20000 */
int	dlabel_count = 0 ;    /* dlabel = dlabel_count + 12500 */
int	elabel_count = 0 ;    /* elabel = elabel_count + 22500 */

int	do_count = 0 ;        /* nesting counter for do/end_do */
int	begin_count = 0 ;     /* nesting counter for begin ... loops */
int	case_count = 0 ;      /* case nesting level */




/* FLOW_INIT
 *
 * Initialize the flow control routines
 */
flow_init()
{
int i ;

for ( i = 0; i < NESTING; i++ ) leave_do_flag[i] = FALSE ;
}



/* Function AGAIN_PROC
 *
 * Process again statements.
 * 3/2/86
 */

again_proc()     
{                  

/* on missing begin statement, abort */
if ( begin_count <= 0 ) {
	sprintf( errline, "Again: no matching begin: %s", in_buff ) ;
	abort( errline ) ;
}

/* construct the goto statement back to begin */
sprintf( out_buff, "      goto %s", blabel[begin_count] ) ;
dump( out_buff ) ;

/* construct label statement */
sprintf( out_buff, "%s continue", alabel[begin_count] ) ;
dump( out_buff ) ;

begin_count-- ;
IN_BUFF_DONE
}




/* Function BEGIN_PROC.C
 *
 * Process begin statements.  Construct a label for the
 * while, until, and again statements to branch to.  The
 * label for again is created here as well.
 *
 * P. R. OVE  3/2/86
 */

begin_proc() 
{
int	count ;
                      
/* keep track of the nesting */
begin_count++ ;
if ( begin_count >= NESTING ) {
	sprintf( errline, "Begin: nesting too deep: %s", in_buff ) ;
	abort( errline ) ;
}

/* make up a label (for begin) and store it in blabel[begin_count] */
count = 17500 + blabel_count ;
blabel_count++ ;
if ( count > 19999 ) {
	sprintf( errline, "Begin: too many labels: %s", in_buff ) ;
	abort( errline ) ;
}
sprintf( blabel[begin_count], "%d", count ) ;

/* make up a label (for again) and store it in alabel[begin_count] */
count = 15000 + alabel_count ;
alabel_count++ ;
if ( count > 17499 ) {
	sprintf( errline, "Begin: too many labels: %s", in_buff ) ;
	abort( errline ) ;
}
sprintf( alabel[begin_count], "%d", count ) ;

/* construct and dump the output record */
sprintf( out_buff, "%s continue", blabel[begin_count] ) ;
dump( out_buff ) ;

IN_BUFF_DONE
}                            




/* Function CASE_PROC
 *
 * Process again statements.
 * 11/9/85
 */

case_proc()     
{                  
int	n, count ;
char	*open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;

/* if char after case is not a blank, tab, or delimeter assume a */
/* variable name beginning with case                             */
if ((close_parens == NULL) && (open_parens == first_nonblank + name_length))
	return ;

/* keep track of the nesting */
case_count++ ;
if ( case_count >= NESTING ) {
	sprintf( errline, "Case: nesting too deep: %s", in_buff ) ;
	abort( errline ) ;
}

/* get logical expression, set to NULL if it is missing */
if ( open_parens == NULL ) { 
	case_exp[ case_count ][0] = NULL ;
}
else {  
	if ( close_parens == NULL ) {
		sprintf( errline, "Case: missing delimeter: %s", in_buff ) ;
		abort( errline ) ;
	}
	n = close_parens - open_parens - 1 ;
	GET_MEM( case_exp[case_count], n+5 ) ;
	case_exp[case_count][0] = '(' ;
	strncpy( case_exp[case_count] + 1, open_parens + 1, n ) ;
	case_exp[case_count][n+1] = ')' ;
	case_exp[case_count][n+2] = NULL ;
}                              


/* make label for continue to return to, store it in clabel[case_count] */
count = 20000 + clabel_count ;
clabel_count++ ;
if ( count > 22499 ) {
	sprintf( errline, "Case: too many labels: %s", in_buff ) ;
	abort( errline ) ;
}
sprintf( clabel[case_count], "%d", count ) ;

/* construct and dump the output record */
sprintf( out_buff, "%s continue", clabel[case_count] ) ;
dump( out_buff ) ;


/* signal that in_buff is empty */
IN_BUFF_DONE
}




/* Function CONTINUE_CASE_PROC
 *
 * Process continue_case statements (part of case construct).
 *
 * P. R. OVE  10/10/86
 */

continue_case_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if there is stuff on the line (open_parens != NULL) and no open
 * parens (close_parens == NULL) assume variable name */
if ( (open_parens != NULL) && (close_parens == NULL) ) return ;

/* on missing case statement, abort */
if ( case_count <= 0 ) {
	sprintf( errline, "CONTINUE_CASE: no matching CASE: %s", in_buff ) ;
	abort( errline ) ;
}
                                   
/* get the logical expression if there is one */
if (open_parens != NULL) {
	n = close_parens - open_parens - 1 ;
	GET_MEM( exp, n+5 ) ;
	exp[0] = '(' ;
	strncpy( exp + 1, open_parens + 1, n ) ;
	exp[n+1] = ')' ;
	exp[n+2] = NULL ;
}

/* construct and dump the jump back to the case statement */
if (open_parens != NULL) {
	strcpy( out_buff, "      if " ) ;
	strcat( out_buff, exp ) ;
	strcat( out_buff, " goto " ) ;
	strcat( out_buff, clabel[case_count] ) ;
	free( exp ) ;
}
else {
	strcpy( out_buff, "      goto " ) ;
	strcat( out_buff, clabel[case_count] ) ;
}

dump( out_buff ) ;

IN_BUFF_DONE
}




/* Function CONTINUE_DO_PROC
 *
 * Process continue_do statements (part of do/end_do construct).
 *
 * P. R. OVE  11/13/86
 */

continue_do_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if there is stuff on the line (open_parens != NULL) and no open
 * parens (close_parens == NULL) assume variable name like CONTINUE_DOit */
if ( (open_parens != NULL) && (close_parens == NULL) ) return ;

/* on missing do statement, abort */
if ( do_count <= 0 ) {
	sprintf( errline, "CONTINUE_DO: not in do/end_do loop: %s", in_buff ) ;
	abort( errline ) ;
}
                                    
/* get the logical expression if there is one */
if (open_parens != NULL) {
	n = close_parens - open_parens - 1 ;
	GET_MEM( exp, n+5 ) ;
	exp[0] = '(' ;
	strncpy( exp + 1, open_parens + 1, n ) ;
	exp[n+1] = ')' ;
	exp[n+2] = NULL ;
}

/* construct and dump the jump to the end_do label */
if (open_parens != NULL) {
	strcpy( out_buff, "      if " ) ;
	strcat( out_buff, exp ) ;
	strcat( out_buff, " goto " ) ;
	strcat( out_buff, dlabel[do_count] ) ;
	free( exp ) ;
}
else {
	strcpy( out_buff, "      goto " ) ;
	strcat( out_buff, dlabel[do_count] ) ;
}

dump( out_buff ) ;

IN_BUFF_DONE
}




/* Function CONTINUE_PROC
 *
 * Process continue statements (part of begin construct).
 *
 * P. R. OVE  10/10/86
 */

continue_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if there is stuff on the line (open_parens != NULL) and no open
 * parens (close_parens == NULL) assume variable name like CONTINUEit */
if ( (open_parens != NULL) && (close_parens == NULL) ) return ;

/* on missing begin statement, abort */
if ( begin_count <= 0 ) {
	sprintf( errline, "CONTINUE: no matching BEGIN: %s", in_buff ) ;
	abort( errline ) ;
}
                                   
/* get the logical expression if there is one */
if (open_parens != NULL) {
	n = close_parens - open_parens - 1 ;
	GET_MEM( exp, n+5 ) ;
	exp[0] = '(' ;
	strncpy( exp + 1, open_parens + 1, n ) ;
	exp[n+1] = ')' ;
	exp[n+2] = NULL ;
}

/* construct and dump the back to the begin statement */
if (open_parens != NULL) {
	strcpy( out_buff, "      if " ) ;
	strcat( out_buff, exp ) ;
	strcat( out_buff, " goto " ) ;
	strcat( out_buff, blabel[begin_count] ) ;
	free( exp ) ;
}
else {
	strcpy( out_buff, "      goto " ) ;
	strcat( out_buff, blabel[begin_count] ) ;
}

dump( out_buff ) ;

IN_BUFF_DONE
}




/* Function DEFAULT_PROC
 *
 * Process default statements.
 *
 * P. R. OVE  11/9/85
 */

default_proc()     
{                  
char	*pntr ;

if ( case_count <= 0 ) {
	sprintf( errline, "DEFAULT: no matching CASE: %s", in_buff ) ;
	abort( errline ) ;
}

dump( "      else" ) ;

/* eliminate "default" from the input buffer */
pntr = line_end( first_nonblank + name_length ) ;
if ( pntr != NULL ) {
	strcpy( in_buff, "\t" ) ;
	strcat( in_buff, pntr ) ;
}
else { IN_BUFF_DONE }

}




/* Function DO_PROC
 *
 * Process do statements.  If there is a label (ala
 * fortran) just dump it to the output.  If no label
 * exists make one up in anticipation of an eventual
 * end_do statement.
 *
 * P. R. OVE  11/9/85
 */

do_proc() 
{
char	*after_do, *pntr ;
int	count ;
                      
/* return without processing if the first nonblank char after DO is a label
   or if there is no blank/tab after the DO */
pntr = first_nonblank + name_length ;
after_do = line_end( pntr ) ;
if ( ( strchr( "0123456789", *after_do ) != NULL ) | 
     ( after_do == pntr )                            ) return ;
                      
/* keep track of the nesting */
do_count++ ;
if ( do_count >= NESTING ) {
	sprintf( errline, "DO: nesting too deep: %s", in_buff ) ;
	abort( errline ) ;
}

/* make up a label and store it in dlabel[do_count] */
count = 12500 + dlabel_count ;
dlabel_count++ ;
if ( count > 14999 ) {
	sprintf( errline, "DO: too many labels: %s", in_buff ) ;
	abort( errline ) ;
}
sprintf( dlabel[do_count], "%d", count ) ;

/* make label for leave_do to jump to and store it in elabel[do_count] */
count = 22500 + elabel_count ;
elabel_count++ ;
if ( count > 24999 ) {
	sprintf( errline, "DO: too many labels: %s", in_buff ) ;
	abort( errline ) ;
}
sprintf( elabel[do_count], "%d", count ) ;

/* construct and dump the output record */
sprintf( out_buff, "      do %s %s", dlabel[do_count], after_do ) ;
dump( out_buff ) ;

IN_BUFF_DONE
}                            



/* Function END_CASE_PROC
 *
 * Process end_case statements.
 *
 * P. R. OVE  11/9/85
 */

end_case_proc()
{                  
	of_count[ case_count ] = 0 ;
	free( case_exp[ case_count ] ) ;
	case_count-- ;
	IN_BUFF_DONE

	if ( case_count < 0 ) { 
		case_count = 0 ;
		return ; }		
		
	dump( "      end if" ) ;
}




/* Function END_DO_PROC
 *
 * Process end_do statements.  Use the label indexed
 * by the current value of do_count (the do nesting
 * index).
 *
 * P. R. OVE  11/9/85
 */

end_do_proc() 
{
                      
/* signal error if no matching do has been found */
if ( do_count <= 0 )  {
	sprintf( errline, "END_DO: no matching do: %s", in_buff ) ;
	abort( errline ) ;
}

/* construct and dump the normal do loop continue statement */
sprintf( out_buff, "%s continue", dlabel[do_count] ) ;
dump( out_buff ) ;

/* construct and dump the leave_do label if needed */
if ( leave_do_flag[do_count] == TRUE ) {
	sprintf( out_buff, "%s continue", elabel[do_count] ) ;
	dump( out_buff ) ;
	leave_do_flag[do_count] = FALSE ;
}

do_count -= 1 ;
IN_BUFF_DONE
}                            




/* Function LEAVE_DO_PROC
 *
 * Process leave_do statements.
 *
 * P. R. OVE  3/2/86
 */

leave_do_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if there is stuff on the line (open_parens != NULL) and no              */
/* open parens (close_parens == NULL) assume variable name like LEAVE_DOit */
if ( (open_parens != NULL) && (close_parens == NULL) ) return ;

/* on missing do statement, abort */
if ( do_count <= 0 ) {
	sprintf( errline, "LEAVE_DO: not in do/end_do loop: %s", in_buff ) ;
	abort( errline ) ;
}
                                    
/* get the logical expression if there is one */
if (open_parens != NULL) {
	n = close_parens - open_parens - 1 ;
	GET_MEM( exp, n+5 ) ;
	exp[0] = '(' ;
	strncpy( exp + 1, open_parens + 1, n ) ;
	exp[n+1] = ')' ;
	exp[n+2] = NULL ;
}

/* construct and dump the jump out of the loop */
if (open_parens != NULL) {
	strcpy( out_buff, "      if " ) ;
	strcat( out_buff, exp ) ;
	strcat( out_buff, " goto " ) ;
	strcat( out_buff, elabel[do_count] ) ;
	free( exp ) ;
}
else {
	strcpy( out_buff, "      goto " ) ;
	strcat( out_buff, elabel[do_count] ) ;
}

leave_do_flag[do_count] = TRUE ;

dump( out_buff ) ;

IN_BUFF_DONE
}




/* Function LEAVE_PROC
 *
 * Process leave statements.
 *
 * P. R. OVE  3/2/86
 */

leave_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if there is stuff on the line (open_parens != NULL) and no           */
/* open parens (close_parens == NULL) assume variable name like LEAVEit */
if ( (open_parens != NULL) && (close_parens == NULL) ) return ;

/* on missing begin statement, abort */
if ( begin_count <= 0 ) {
	sprintf( errline, "LEAVE: no matching begin: %s", in_buff ) ;
	abort( errline ) ;
}
                                    
/* get the logical expression if there is one */
if (open_parens != NULL) {
	n = close_parens - open_parens - 1 ;
	GET_MEM( exp, n+5 ) ;
	exp[0] = '(' ;
	strncpy( exp + 1, open_parens + 1, n ) ;
	exp[n+1] = ')' ;
	exp[n+2] = NULL ;
}

/* construct and dump the jump to again */
if (open_parens != NULL) {
	strcpy( out_buff, "      if " ) ;
	strcat( out_buff, exp ) ;
	strcat( out_buff, " goto " ) ;
	strcat( out_buff, alabel[begin_count] ) ;
	free( exp ) ;
}
else {
	strcpy( out_buff, "      goto " ) ;
	strcat( out_buff, alabel[begin_count] ) ;
}

dump( out_buff ) ;

IN_BUFF_DONE
}



/* Function OF_PROC
 *
 * Process of statements.
 *
 * P. R. OVE  11/9/85
 */

of_proc()     
{                  
int	n ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if no open parens assume variable name like OFile */
/* (no open parens <==> close_parens will be NULL)   */
if ( close_parens == NULL ) return ;

/* abort on missing case statement */
if ( case_count <= 0 ) {
	sprintf( errline, "OF: missing CASE statement: %s", in_buff ) ;
	abort( errline ) ;
}

/* keep track of "of's" for each case level */
of_count[ case_count ] += 1 ;

/* get the logical expression */
n = close_parens - open_parens - 1 ;
GET_MEM( exp, n+5 ) ;
exp[0] = '(' ;
strncpy( exp + 1, open_parens + 1, n ) ;
exp[n+1] = ')' ;
exp[n+2] = NULL ;

/* construct the "if" or "if else" statement.  If there is a case */
/* logical expression us .eq. to determine the result             */
if ( case_exp[ case_count ][0] == NULL ) {
	if ( of_count[ case_count ] != 1 ) {
		strcpy( out_buff, "      else if " ) ; }
     	else {
		strcpy( out_buff, "      if " )      ; }
	strcat( out_buff, exp ) ;
	strcat( out_buff, " then " ) ; }
else {
	if ( of_count[ case_count ] != 1 ) {
		strcpy( out_buff, "      else if (" ) ; }
     	else {
		strcpy( out_buff, "      if (" )      ; }
	strcat( out_buff, case_exp[ case_count ] ) ;
	strcat( out_buff, ".eq." ) ;
	strcat( out_buff, exp ) ;
	strcat( out_buff, ") then " ) ; }
                                   
dump( out_buff ) ;

/* eliminate "of stuff" from the input buffer */
pntr = line_end( close_parens + 1 ) ;
if ( pntr != NULL ) {
	strcpy( in_buff, "\t" ) ;
	strcat( in_buff, pntr ) ;
}
else { IN_BUFF_DONE }

free( exp ) ;
}




/* Function UNTIL_PROC
 *
 * Process until statements.
 *
 * P. R. OVE  3/2/86
 */

until_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if no open parens assume variable name like UNTILon */
/* (no open parens <==> close_parens will be NULL)   */
if ( close_parens == NULL ) return ;

/* on missing begin statement, abort */
if ( begin_count <= 0 ) {
	sprintf( errline, "UNTIL: no matching begin: %s", in_buff ) ;
	abort( errline ) ;
}
                                    
/* get the logical expression */
n = close_parens - open_parens - 1 ;
GET_MEM( exp, n+5 ) ;
exp[0] = '(' ;
strncpy( exp + 1, open_parens + 1, n ) ;
exp[n+1] = ')' ;
exp[n+2] = NULL ;

/* construct and dump the conditional jump to begin */
sprintf( out_buff, "      if (.not.%s) goto %s",
	exp, blabel[begin_count] ) ;
dump( out_buff ) ;

/* construct a label statement (for leave to jump to) */
sprintf( out_buff, "%s continue", alabel[begin_count] ) ;
dump( out_buff ) ;

begin_count-- ;
free( exp ) ;
IN_BUFF_DONE
}




/* Function WHILE_PROC
 *
 * Process while statements.
 *
 * P. R. OVE  3/2/86
 */

while_proc()     
{                  
int	n, count ;
char	*pntr, *open_parens, *close_parens ;

/* get the comparison expression */
open_parens = line_end( first_nonblank + name_length ) ;
close_parens = mat_del( open_parens ) ;
                                           
/* if no open parens assume variable name like WHILEon */
/* (no open parens <==> close_parens will be NULL)   */
if ( close_parens == NULL ) return ;

/* on missing begin statement, abort */
if ( begin_count <= 0 ) {
	sprintf( errline, "WHILE: no matching begin: %s", in_buff ) ;
	abort( errline ) ;
}

/* get the logical expression */
n = close_parens - open_parens - 1 ;
GET_MEM( exp, n+5 ) ;
exp[0] = '(' ;
strncpy( exp + 1, open_parens + 1, n ) ;
exp[n+1] = ')' ;
exp[n+2] = NULL ;

/* construct and dump the output record */
strcpy( out_buff, "      if (.not." ) ;
strcat( out_buff, exp ) ;
strcat( out_buff, ") goto " ) ;
strcat( out_buff, alabel[begin_count] ) ;
dump( out_buff ) ;

free( exp ) ;
IN_BUFF_DONE
}
