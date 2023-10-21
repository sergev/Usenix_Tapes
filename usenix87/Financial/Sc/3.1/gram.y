/*	SC	A Spreadsheet Calculator
 *		Command and expression parser
 *
 *		original by James Gosling, September 1982
 *		modified by Mark Weiser and Bruce Israel,
 *			University of Maryland
 *
 * 		more mods Robert Bond 12/86
 *
 */



%{
#include <curses.h>
#include "sc.h"
%}

%union {
    int ival;
    double fval;
    struct ent *ent;
    struct enode *enode;
    char *sval;
}

%type <ent> var
%type <fval> num
%type <enode> e term
%token <sval> STRING
%token <ival> NUMBER
%token <fval> FNUMBER
%token <sval> WORD
%token <ival> COL
%token S_FORMAT
%token S_LABEL
%token S_LEFTSTRING
%token S_RIGHTSTRING
%token S_GET
%token S_PUT
%token S_MERGE
%token S_LET
%token S_WRITE
%token S_TBL
%token S_PROGLET
%token S_COPY
%token S_SHOW
%token S_ERASE
%token S_FILL
%token S_GOTO

%token K_FIXED
%token K_SUM
%token K_PROD
%token K_AVG
%token K_ACOS
%token K_ASIN
%token K_ATAN
%token K_CEIL
%token K_COS
%token K_EXP
%token K_FABS
%token K_FLOOR
%token K_HYPOT
%token K_LN
%token K_LOG
%token K_PI
%token K_POW
%token K_SIN
%token K_SQRT
%token K_TAN
%token K_DTR
%token K_RTD
%token K_MAX
%token K_MIN

%left '?' ':'
%left '|'
%left '&'
%nonassoc '<' '=' '>'
%left '+' '-'
%left '*' '/'
%left '^'

%%
command:	S_LET var '=' e	{ let ($2, $4); }
	|	S_LABEL var '=' STRING
				{ label ($2, $4, 0); }
	|	S_LEFTSTRING var '=' STRING
				{ label ($2, $4, -1); }
	|	S_RIGHTSTRING var '=' STRING
				{ label ($2, $4, 1); }
	|	S_FORMAT COL NUMBER NUMBER
				{ fwidth[$2] = $3;
				  FullUpdate++;
				  modflg++;
				  precision[$2] = $4; }
	|	S_GET STRING	{ readfile ($2,1); }
	|	S_MERGE STRING	{ readfile ($2,0); }
	|	S_PUT STRING	{ (void) writefile ($2); }
	|	S_WRITE STRING	{ printfile ($2); }
	|	S_TBL STRING	{ tblprintfile ($2); }
	|       S_SHOW COL ':' COL  { showcol( $2, $4); }
	|       S_SHOW NUMBER ':' NUMBER  { showrow( $2, $4); }
	|	S_COPY var var ':' var { copy($2, $3, $5); }
	|	S_ERASE var ':' var { eraser($2, $4); }
	|	S_FILL var ':' var num num { fill($2, $4, $5, $6); }
	|	S_GOTO var	 {moveto($2); }
	|	/* nothing */
	|	error;

term: 		var		{ $$ = new_var('v', $1); }
	|	K_FIXED term	{ $$ = new ('f', (struct enode *)0, $2); }
	|       '@' K_SUM '(' var ':' var ')' 
				{ $$ = new (O_REDUCE('+'),
				(struct enode *)$4, (struct enode *)$6); }
	|       '@' K_PROD '(' var ':' var ')' 
				{ $$ = new (O_REDUCE('*'),
				(struct enode *)$4, (struct enode *)$6); }
	|       '@' K_AVG '(' var ':' var ')' 
				{ $$ = new (O_REDUCE('a'),
				(struct enode *)$4, (struct enode *)$6); }
	| '@' K_ACOS '(' e ')'	{ $$ = new(ACOS, (struct enode *)0, $4); }
	| '@' K_ASIN '(' e ')'	{ $$ = new(ASIN, (struct enode *)0, $4); }
	| '@' K_ATAN '(' e ')'	{ $$ = new(ATAN, (struct enode *)0, $4); }
	| '@' K_CEIL '(' e ')'	{ $$ = new(CEIL, (struct enode *)0, $4); }
	| '@' K_COS '(' e ')'	{ $$ = new(COS, (struct enode *)0, $4); }
	| '@' K_EXP '(' e ')'	{ $$ = new(EXP, (struct enode *)0, $4); }
	| '@' K_FABS '(' e ')'	{ $$ = new(FABS, (struct enode *)0, $4); }
	| '@' K_FLOOR '(' e ')'	{ $$ = new(FLOOR, (struct enode *)0, $4); }
	| '@' K_HYPOT '(' e ',' e ')'	{ $$ = new(HYPOT, $4, $6); }
	| '@' K_LN '(' e ')'	{ $$ = new(LOG, (struct enode *)0, $4); }
	| '@' K_LOG '(' e ')'	{ $$ = new(LOG10, (struct enode *)0, $4); }
	| '@' K_POW '(' e ',' e ')'	{ $$ = new(POW, $4, $6); }
	| '@' K_SIN '(' e ')'	{ $$ = new(SIN, (struct enode *)0, $4); }
	| '@' K_SQRT '(' e ')'	{ $$ = new(SQRT, (struct enode *)0, $4); }
	| '@' K_TAN '(' e ')'	{ $$ = new(TAN, (struct enode *)0, $4); }
	| '@' K_DTR '(' e ')'	{ $$ = new(DTR, (struct enode *)0, $4); }
	| '@' K_RTD '(' e ')'	{ $$ = new(RTD, (struct enode *)0, $4); }
	| '@' K_MAX '(' e ',' e ')'	{ $$ = new(MAX, $4, $6); }
	| '@' K_MIN '(' e ',' e ')'	{ $$ = new(MIN, $4, $6); }
	|	'(' e ')'	{ $$ = $2; }
	|	'+' term	{ $$ = $2; }
	|	'-' term	{ $$ = new ('m', (struct enode *)0, $2); }
	|	NUMBER		{ $$ = new_const('k', (double) $1); }
	|	FNUMBER		{ $$ = new_const('k', $1); }
	|	K_PI	{ $$ = new_const('k', (double)3.14159265358979323846); }
	|	'~' term	{ $$ = new ('~', (struct enode *)0, $2); }
	|	'!' term	{ $$ = new ('~', (struct enode *)0, $2); }
	;

e:		e '+' e		{ $$ = new ('+', $1, $3); }
	|	e '-' e		{ $$ = new ('-', $1, $3); }
	|	e '*' e		{ $$ = new ('*', $1, $3); }
	|	e '/' e		{ $$ = new ('/', $1, $3); }
	|	e '^' e		{ $$ = new ('^', $1, $3); }
	|	term
	|	e '?' e ':' e	{ $$ = new ('?', $1, new(':', $3, $5)); }
	|	e '<' e		{ $$ = new ('<', $1, $3); }
	|	e '=' e		{ $$ = new ('=', $1, $3); }
	|	e '>' e		{ $$ = new ('>', $1, $3); }
	|	e '&' e		{ $$ = new ('&', $1, $3); }
	|	e '|' e		{ $$ = new ('|', $1, $3); }
	|	e '<' '=' e	{ $$ = new ('~', (struct enode *)0,
						new ('>', $1, $4)); }
	|	e '!' '=' e	{ $$ = new ('~', (struct enode *)0, 
						new ('=', $1, $4)); }
	|	e '>' '=' e	{ $$ = new ('~', (struct enode *)0, 
						new ('<', $1, $4)); }
	;

var:		COL NUMBER	{ $$ = lookat($2 , $1); };

num:		NUMBER		{ $$ = (double) $1; }
	|	FNUMBER		{ $$ = $1; }
	|	'-' num		{ $$ = -$2; }
	|	'+' num		{ $$ = $2; }
	;
