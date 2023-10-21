%{
#include "solve.h"

char *current_var;
%}

%union {
	double fval;
	EXPRESSION *eval;
	char *sval;
}
%type <eval>	e
%token	<sval> 	NAME
%token	<fval>	NUMBER

%left '+' '-'
%left '*' '/'
%right '^' 
%left UNARYMINUS
%%
command:	/* nothing */
	|	'\n'
	|	NAME {current_var = $1;} '=' e '\n' {assign($1, $4);} command
	;

e:		NAME			{ $$ = new(ID, $1); }
	|	NUMBER			{ $$ = new(CONST, &($1)); }
	|	'-' e %prec UNARYMINUS	{ $$ = new(MINUS, $2); }
	|	e '+' e			{ $$ = new(ADD, $1, $3); }
	|	e '-' e			{ $$ = new(SUB, $1, $3); }
	|	e '*' e			{ $$ = new(MULT, $1, $3); }
	|	e '/' e			{ $$ = new(DIV, $1, $3); }
	|	e '%' e			{ $$ = new(MOD, $1, $3); }
	|	e '^' e			{ $$ = new(POW, $1, $3); }
	|	'(' e ')'		{ $$ = $2; }
	;
%%
