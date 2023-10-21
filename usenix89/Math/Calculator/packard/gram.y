/*
 *	grammar for interpreter
 */

%{

# include	<math.h>
# include	"ic.h"

expr *buildOp();
expr *buildNum();
expr *buildVar();
expr *buildConst();
expr *buildStr();
double eeval();
int eval();
int ignorenl;
double	dotval;
extern int	yyfiledeep;

%}

%union {
	int		ival;
	char	*cval;
	double	dval;
	expr	*eval;
	symbol	*nval;
}

%token <cval>	STRING
%token <dval>	NUMBER
%token <ival>	NL ALL DOWN UP
%token <ival>	DEFINE QUIT READ SHELL EDIT
%token <ival>	WHILE IF ELSE FOR DO BREAK CONTINUE EXPR RETURN
%token <ival>	OP CP OS CS OC CC FUNC COMMA SEMI 
%token <nval>	NAME AUTO
%type <eval>	expr var stat optexpr statlist primary arglist oarglist
%type <eval>	auto names fargs ofargs aexpr

%nonassoc	<ival>	POUND
%right <ival>	ASSIGN
%right <ival>	QUEST COLON
%left <ival>	OR
%left <ival>	AND
%left <ival>	EQ NE
%left <ival>	LT GT LE GE
%left <ival>	PLUS MINUS
%left <ival>	TIMES DIVIDE MOD
%right <ival>	POW
%right <ival>	UMINUS BANG FACT
%nonassoc <ival>	INC DEC

%%
lines	:	lines pcommand
	|
			{ ignorenl = 0; }
	;
pcommand:	command
	|	error
			{ ignorenl = 0; } NL
	;
command	:	QUIT NL
			{ YYACCEPT; }
	|	expr NL
			{
				if ($1->e_tag != ASSIGN)
					printf ("%.15g\n", dotval = eeval($1));
				else
					eeval ($1);
				freetree ($1);
			}
	|	expr POUND expr NL
			{
				double	base;

				base = eeval ($3);
				dotval = eeval ($1);
				freetree ($1);
				freetree ($3);
				printinbase (base, dotval);
			}
	|	stat { eval ($1); freetree ($1); ignorenl = 0; } optnl
	|	DEFINE { ignorenl = 1; } func { ignorenl = 0; } optnl
	|	READ STRING
			{
				pushinput ($2);
			}
	|	NL
	;
optnl	:	NL
	|
	;
func	:	NAME OP { pushlevel(); } ofargs CP OC auto statlist CC
			{
				definefunc ($1, $4, $7, $8);
				poplevel();
			}
	;
ofargs	:	fargs
			{ fixstack ($1); $$ = $1; }
	|
			{ $$ = 0; }
fargs	:	NAME COMMA fargs
			{ $$ = buildOp ($2, buildVar($1), $3); }
	|	NAME
			{ $$ = buildOp (COMMA, buildVar ($1), (expr *) 0); }
	;
auto	:	AUTO names
			{ fixstack ($2); $$ = $2; }
	|
			{ $$ = 0; }
	;
names	:	NAME COMMA names
			{ $$ = buildOp (AUTO, buildVar ($1), $3); }
	|	NAME SEMI auto
			{ $$ = buildOp (AUTO, buildVar ($1), $3); }
	;
stat	:	IF ignorenl OP expr CP stat
			{ $$ = buildOp(IF, $4, $6); }
	|	IF ignorenl OP expr CP stat ELSE stat
			{ $$ = buildOp(ELSE, $4, buildOp(ELSE, $6, $8)); }
	|	WHILE ignorenl OP expr CP stat
			{ $$ = buildOp(WHILE, $4, $6); }
	|	DO ignorenl stat WHILE OP expr CP
			{ $$ = buildOp(DO, $3, $6); }
	|	FOR ignorenl OP optexpr SEMI optexpr SEMI optexpr CP stat
			{
				$$ = buildOp(FOR, buildOp(FOR, $4, $6),
				buildOp(FOR, $8, $10));
			}
	|	BREAK ignorenl SEMI
			{ $$ = buildOp(BREAK, (expr *) 0, (expr *) 0); }
	|	CONTINUE ignorenl SEMI
			{ $$ = buildOp(CONTINUE, (expr *) 0, (expr *) 0); }
	|	RETURN ignorenl expr SEMI
			{ $$ = buildOp (RETURN, (expr *) 0, $3); }
	|	expr ignorenl SEMI
			{ $$ = buildOp(EXPR, $1, (expr *) 0); }
	|	OC ignorenl statlist CC
			{ $$ = $3; }
	|	SEMI ignorenl
			{ $$ = buildOp((expr *) 0, (expr *) 0, (expr *) 0); }
	;
ignorenl:	{ ignorenl = 1; }
	;
optexpr	:	expr
			{ $$ = $1; }
	|
			{ $$ = 0; }
	;
statlist:	stat statlist
			{ $$ = buildOp(OC, $1, $2); }
	|	stat
			{ $$ = buildOp(OC, $1, (expr *) 0); }
	;
var	:	NAME
			{ $$ = buildVar($1); }
	|	var OS expr CS
			{ $$ = buildOp ($2, $1, $3); }
	;
expr	:	primary
	|	expr PLUS expr
			{
			binop:
				$$ = buildOp($2, $1, $3);
			}
	|	expr MINUS expr
			{ goto binop; }
	|	expr TIMES expr
			{ goto binop; }
	|	expr DIVIDE expr
			{ goto binop; }
	|	expr MOD expr
			{ goto binop; }
	|	expr POW expr
			{ goto binop; }
	|	expr QUEST expr COLON expr
			{ $$ = buildOp(QUEST, $1, buildOp(COLON, $3, $5)); }
	|	expr AND expr
			{ goto binop; }
	|	expr OR expr
			{ goto binop; }
	|	var ASSIGN expr
			{ goto binop; }
	|	expr EQ expr
			{ goto binop; }
	|	expr NE expr
			{ goto binop; }
	|	expr LT expr
			{ goto binop; }
	|	expr GT expr
			{ goto binop; }
	|	expr LE expr
			{ goto binop; }
	|	expr GE expr
			{ goto binop; }
	;
primary	:	MINUS primary %prec UMINUS
			{ $$ = buildOp(UMINUS, $2, (expr *) 0); }
	|	BANG primary
			{ $$ = buildOp(BANG, $2, (expr *) 0); }
	|	primary BANG	%prec FACT
			{ $$ = buildOp(FACT, (expr *) 0, $1); }
	|	INC var
			{ $$ = buildOp(INC, $2, (expr *) 0); }
	|	var INC
			{ $$ = buildOp(INC, (expr *) 0, $1); }
	|	DEC var
			{ $$ = buildOp(DEC, $2, (expr *) 0); }
	|	var DEC
			{ $$ = buildOp(DEC, (expr *) 0, $1); }
	|	NUMBER
			{ $$ = buildNum($1); }
	|	var
			{ $$ = $1; }
	|	OP expr CP
			{ $$ = $2; }
	|	NAME OP oarglist CP
			{
				switch ($1->s_type) {
				case UNDEF:
					$1->s_level = 0;
					$1->s_type = FUNCTYPE;
				case FUNCTYPE:
				case BUILTIN:
					break;
				default:
					yyerror ("illegal use of identifier as function");
					YYERROR;
				}
				$$ = buildOp ($2, buildVar ($1), $3);
			}
		;
oarglist:	arglist
	|
			{ $$ = 0; }
	;
arglist	:	arglist COMMA aexpr
			{ $$ = buildOp ($2, $3, $1); }
	|	aexpr
			{ $$ = buildOp (COMMA, $1, (expr *) 0); }
	;
aexpr	:	expr
			{ $$ = $1; }
	|	STRING
			{ $$ = buildStr ($1); }
	;
%%

# include	<stdio.h>

yywrap ()
{
	return 1;
}

yyerror (s)
char	*s;
{
	extern char	*yyfile;
	extern int	yylineno;
	if (yyfiledeep)
		fprintf (stderr, "\"%s\": line %d, %s\n", yyfile, yylineno, s);
	else
		fprintf (stderr, "%s\n", s);
}

eerror (s)
char	*s;
{
	fprintf (stderr, "%s\n", s);
}
