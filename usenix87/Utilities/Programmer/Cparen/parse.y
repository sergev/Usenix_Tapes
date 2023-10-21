/*
 * Copyright (c) 1984 by
 * Tektronix, Incorporated Beaverton, Oregon 97077
 * All rights reserved.
 *
 * Permission is hereby granted for personal, non-commercial
 * reproduction and use of this program, provided that this
 * notice is included in any copy.
 */
%term	MUL	1	/* start of assignable ops (e.g. + vs +=)	*/
%term	DIV	2
%term	MOD	3
%term	PLUS	4
%term	MINUS	5
%term	LS	6	/* left-shift <<	*/
%term	RS	7	/* right-shift >>	*/
%term	AND	8
%term	OR	9
%term	ER	10	/* exclusive-or ^	*/
%term	ASSIGN	11	/* end of assignable ops (e.g. + vs +=)		*/
%term	MULASG	12	/* assign version of assignable ops		*/
%term	DIVASG	13
%term	MODASG	14
%term	PLUSASG	15
%term	MINUSASG 16
%term	LSASG	17
%term	RSASG	18
%term	ANDASG	19
%term	ORASG	20
%term	ERASG	21	/* end of assign version of ops			*/
%term	LT	22	/* less-than <		*/
%term	LE	23
%term	GT	24
%term	GE	25
%term	EQ	26
%term	NE	27
%term	ANDAND	28
%term	OROR	29
%term	CM	30	/* comma ,		*/
%term	COMOP	31	/* comma operator ,	*/
%term	QUEST	32	/* ?			*/
%term	COLON	33
%term	UMINUS	34	/* unary minus -	*/
%term	INCR	35	/* (post- or pre-) increment ++	*/
%term	DECR	36	/* (post- or pre-) decrement --	*/
%term	INDR	37	/* indirect *		*/
%term	ADDR	38	/* address-of &		*/
%term	NOT	39
%term	COMPL	40	/* complement ~		*/
%term	SIZEOF	41
%term	CAST	42
%term	BRACK	43	/* brackets []		*/
%term	ISSTRUCT 44
%term	ISUNION	45
%term	ENUM	46
%term	STREF	47	/* structure reference (-> or .)	*/
%term	DOT	48	/* subfield .		*/
%term	CALL	49
%term	LIT	51	/* literal. A dummy operator	*/
%term	KEYWORD	52	/* a statement keyword	*/
%term	SM	53	/* semi-colon ;		*/
%term	LC	54	/* left curly-brace {	*/
%term	RC	55	/* right curly-brace }	*/
%term	FLUFF	56	/* syntactic fluff: comments and whitespace	*/



%left	CM
%right	ASOP ASSIGN
%right	QUEST COLON
%left	OROR
%left	ANDAND
%left	OR
%left	ER
%left	AND
%left	EQUOP
%left	RELOP
%left	SHIFTOP
%left	PLUS MINUS
%left	MUL DIVOP
%right	UNOP
%right	INCOP SIZEOF
%left	LB LP STROP

%{
#define PARSER
#include <stdio.h>
#include "cparen.h"

static char *rcsid = "$Header: parse.y,v 1.14 84/04/05 09:22:07 bradn Stable $";

struct node *bld();
%}

%start	unit_list
%type	<nodep>	e term elist cast_type null_decl funct_idn atype struct_dcl
		enum_dcl unit unit_list
%token	<lexval> RELOP CM DIVOP PLUS MINUS SHIFTOP MUL EQUOP AND OR ER
		ANDAND OROR ASSIGN QUEST COLON ASOP INCOP UNOP SIZEOF
		LP RP LB RB STROP STRUCT ENUM NAME TYPE LT LE GT GE DIV
		MOD LS RS EQ NE INCR DECR NOT COMPL STREF DOT ISSTRUCT
		ISUNION COMOP INDR ADDR CAST BRACK CALL UMINUS
		SM LIT KEYWORD


%%

unit_list:
	  unit
		{$$ = NOP;}
	| unit_list unit
		{$$ = NOP;}
	;
unit:	  COLON
		{$$ = NOP;}
	| SM
		{$$ = NOP;}
	| e
		{$$ = $1;}
	| KEYWORD
		{$$ = NOP;}
	;
e:	e RELOP e
		{bop:	consider($2->l_val, $1, $3);
			$$ = bld($2->l_val, BINARY, $1->n_ledge, $3->n_redge);
		}
	| e CM e
		{ $2->l_val = COMOP; goto bop; }
	| e DIVOP e
		{ goto bop; }
	| e PLUS e
		{ goto bop; }
	| e MINUS e
		{ goto bop; }
	| e SHIFTOP e
		{ goto bop; }
	| e MUL e
		{ goto bop; }
	| e EQUOP e
		{ goto bop; }
	| e AND e
		{ goto bop; }
	| e OR e
		{ goto bop; }
	| e ER e
		{ goto bop; }
	| e ANDAND e
		{ goto bop; }
	| e OROR e
		{ goto bop; }
	| e MUL ASSIGN e
		{ abop:
			$2->l_val = asgof($2->l_val);
			consider($2->l_val, $1, $4);
			$$ = bld($2->l_val, BINARY, $1->n_ledge, $4->n_redge);
		}
	| e DIVOP ASSIGN e
		{ goto abop; }
	| e PLUS ASSIGN e
		{ goto abop; }
	| e MINUS ASSIGN e
		{ goto abop; }
	| e SHIFTOP ASSIGN e
		{ goto abop; }
	| e AND ASSIGN e
		{ goto abop; }
	| e OR ASSIGN e
		{ goto abop; }
	| e ER ASSIGN e
		{ goto abop; }
	| e QUEST e COLON e
		{	consider($2->l_val, $1, NIL);
			consider($4->l_val, $3, $5);
			$$ = bld($2->l_val, BINARY, $1->n_ledge, $5->n_redge);
		}
	| e ASOP e
		{
			$2->l_val = asgof($2->l_val);
			consider($2->l_val, $1, $3);
			$$ = bld($2->l_val, BINARY, $1->n_ledge, $3->n_redge);
		}
	| e ASSIGN e
		{ goto bop; }
	| term
	;
term:	term INCOP
		{	consider($2->l_val, $1, NIL);
			$$ = bld($2->l_val, HASL, $1->n_ledge, $2);
		}
	| INCOP term
		{ unop:	consider($1->l_val, NIL, $2);
			$$ = bld($1->l_val, HASR, $1, $2->n_redge);
		}
	| MUL term
		{$1->l_val = INDR; goto unop;}
	| AND term
		{$1->l_val = ADDR; goto unop;}
	| MINUS term
		{$1->l_val = UMINUS; goto unop;}
	| UNOP term
		{ goto unop; }
	| SIZEOF LP cast_type RP	%prec SIZEOF
		{
			$$ = bld($1->l_val, HASR, $1, $4);
		}
	| SIZEOF term
		{ goto unop; }
	| LP cast_type RP term		%prec INCOP
		{
			consider(CAST, NOP, $4);
			$$ = bld(CAST, BINARY, $1, $4->n_redge);
		}
	| term LB e RB
		{	consider(BRACK, $1, $3);
			$$ = bld(BRACK, BINARY, $1->n_ledge, $4);
		}
	| term STROP NAME
		{	consider($2->l_val, $1, NOP);
			$$ = bld($2->l_val, BINARY, $1->n_ledge, $3);
		}
	| funct_idn RP
		{
			$$ = bld(CALL, HASL, $1->n_ledge, $2);
		}
	| funct_idn elist RP
		{
			$$ = bld(CALL, BINARY, $1->n_ledge, $3);
		}
	| NAME			/* includes ICON, FCON, and STRING	*/
		{ $$ = bld(LIT, 0, $1, $1); }
	| LP unit_list RP
		{ $$ = bld(LIT, HASPAR, $1, $3);}
	;
elist:	  e				%prec CM
		{$$ = $1;}
	| elist CM e
		{
			$$ = bld(LIT, 0, $1->n_ledge, $3->n_redge);
		}
	;
cast_type: atype null_decl
		{
			$$ = bld(LIT, 0, $1->n_ledge,
			  $2->n_redge ? $2->n_redge : $1->n_redge);
		}
	;
null_decl:	/* empty */
		{ $$ = NOP; }
	| LP null_decl RP LP RP
		{$$ = bld(LIT, 0, $1, $5);}
	| MUL null_decl
		{$$ = bld(LIT, 0, $1, $2->n_redge ? $2->n_redge : $1);}
	| null_decl LB RB
		{$$ = bld(LIT, 0, $1->n_ledge ? $1->n_ledge : $2, $3);}
	| null_decl LB e RB
		{
			$$ = bld(LIT, 0, $1->n_ledge ? $1->n_ledge : $2, $4);
		}
	| LP null_decl RP
		{$$ = bld(LIT, 0 , $1, $3);}
	;
funct_idn: NAME LP
		{ $$ = bld(LIT, 0, $1, $2); }
	| term LP
		{ $$ = bld(LIT, 0, $1->n_ledge, $2); }
	;
atype:	 TYPE
		{$$ = bld(LIT, 0, $1, $1);}
	| TYPE TYPE
		{$$ = bld(LIT, 0, $1, $2);}
	| TYPE TYPE TYPE
		{$$ = bld(LIT, 0, $1, $3);}
	| struct_dcl
		{$$ = $1;}
	| enum_dcl
		{$$ = $1;}
	;
struct_dcl: STRUCT NAME	
		{$$ = bld(LIT, 0, $1, $2);}
	;
enum_dcl:  ENUM NAME
		{$$ = bld(LIT, 0, $1, $2);}
	;
%%

yyerror(s)
char *s;
{
	fprintf(stderr,"%s: %s\n", cmd, s);
}

struct node *			/* the allocated node			*/
bld(op, flags, left, right)
int op;				/* the operation to be performed	*/
int flags;			/* the flags to set			*/
struct lexed *left;		/* points to the left token		*/
struct lexed *right;		/* points to the right token		*/
{
	struct node *res;


	if (!(res = (struct node *) malloc(sizeof(struct node)))) {
		fprintf(stderr,"%s: out of memory\n", cmd);
		exit(1);
	}
	res->n_op = op;
	res->n_flags = flags;
	res->n_ledge = left;
	res->n_redge = right;
	return(res);
}

static int
asgof(v)	/* convert the given op into the = form e.g. + to +=	*/
int v;
{
	if (v < ASSIGN) {
		v += ASSIGN;
	}
	return(v);
}
