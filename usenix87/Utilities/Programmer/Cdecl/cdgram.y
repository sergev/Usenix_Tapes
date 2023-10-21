%{
#include <stdio.h>

#define	MB_SHORT	0001
#define	MB_LONG		0002
#define	MB_UNSIGNED	0004
#define MB_INT		0010
#define MB_CHAR		0020
#define MB_FLOAT	0040
#define MB_DOUBLE	0100

int modbits = 0;
int arbdims = 1;
char *savedtype;
char *savedname;
char *ds(), *cat();
char *index(), *malloc();
char prev;
%}

%union {
	char *dynstr;
	struct {
		char *left;
		char *right;
	} halves;
}

%token DECLARE CAST INTO AS HELP EXPLAIN
%token FUNCTION RETURNING POINTER TO ARRAY OF
%token <dynstr> NAME NUMBER STRUCTUNION UNSIGNED LONG SHORT
%token <dynstr> INT CHAR FLOAT DOUBLE
%type <dynstr> mod_list tname type modifier
%type <dynstr> cdecl cdecl1 cdims adims c_type
%type <halves> adecl

%start prog

%%
prog	: /* empty */
		| prog stat
		;

stat	: HELP '\n'
			{
			help();
			}
		| DECLARE NAME AS adecl '\n'
			{
			printf("%s %s%s%s",savedtype,$4.left,$2,$4.right);
#ifdef MKPROGRAM
			if (prev == 'f')
				printf("\n{\n}\n");
			else
				printf(";\n");
#else
			printf("\n");
#endif
			free($4.left);
			free($4.right);
			free($2);
			}
		| CAST NAME INTO adecl '\n'
			{
			if (prev == 'f')
				unsupp("Cast into function");
			else if (prev=='A' || prev=='a')
				unsupp("Cast into array");
			printf("(%s",savedtype);
			if (strlen($4.left)+strlen($4.right))
				printf(" %s%s",$4.left,$4.right);
			printf(")%s\n",$2);
			free($4.left);
			free($4.right);
			free($2);
			}
		| EXPLAIN type cdecl '\n'
			{ printf("declare %s as %s%s\n",savedname,$3,$2); }
		| '\n'
		| error '\n'
			{
			yyerrok;
			}
		;

cdecl	: cdecl1
		| '*' cdecl
			{ $$ = cat($2,ds("pointer to "),NULL); }
		;

cdecl1	: cdecl1 '(' ')'
			{ $$ = cat($1,ds("function returning "),NULL); }
		| cdecl1 cdims
			{ $$ = cat($1,ds("array "),$2); }
		| '(' cdecl ')'
			{ $$ = $2; }
		| NAME
			{
				savename($1);
				$$ = ds("");
			}
		;

cdims	: '[' ']'
			{ $$ = ds("of "); }
		| '[' NUMBER ']'
			{ $$ = cat($2,ds(" of "),NULL); }
		;

adecl	: FUNCTION RETURNING adecl
			{
			if (prev == 'f')
				unsupp("Function returning function");
			else if (prev=='A' || prev=='a')
				unsupp("Function returning array");
			$$.left = $3.left;
			$$.right = cat(ds("()"),$3.right,NULL);
			prev = 'f';
			}
		| FUNCTION '(' NAME ')' RETURNING adecl
			{
			if (prev == 'f')
				unsupp("Function returning function");
			else if (prev=='A' || prev=='a')
				unsupp("Function returning array");
			$$.left = $6.left;
			$$.right = cat(ds("("),$3,ds(")"));
			$$.right = cat($$.right,$6.right,NULL);
			prev = 'f';
			}
		| ARRAY adims OF adecl
			{
			if (prev == 'f')
				unsupp("Array of function");
			else if (prev == 'a')
				unsupp("Inner array of unspecified size");
			if (arbdims)
				prev = 'a';
			else
				prev = 'A';
			$$.left = $4.left;
			$$.right = cat($2,$4.right,NULL);
			}
		| POINTER TO adecl
			{
			if (prev == 'a')
				unsupp("Pointer to array of unspecified dimension");
			if (prev=='a' || prev=='A' || prev=='f') {
				$$.left = cat($3.left,ds("(*"),NULL);
				$$.right = cat(ds(")"),$3.right,NULL);
			} else {
				$$.left = cat($3.left,ds("*"),NULL);
				$$.right = $3.right;
			}
			prev = 'p';
			}
		| type
			{
			savetype($1);
			$$.left = ds("");
			$$.right = ds("");
			prev = 't';
			}
		;

adims	: /* empty */
			{
			arbdims = 1;
			$$ = ds("[]");
			}
		| NUMBER
			{
			arbdims = 0;
			$$ = cat(ds("["),$1,ds("]"));
			}
		;

type	: tinit c_type
			{ mbcheck(); $$ = $2; }
		;

tinit	: /* empty */
			{ modbits = 0; }
		;

c_type	: mod_list
			{ $$ = $1; }
		| tname
			{ $$ = $1; }
		| mod_list tname
			{ $$ = cat($1,ds(" "),$2); }
		| STRUCTUNION NAME
			{ $$ = cat($1,ds(" "),$2); }
		;

tname	: INT
			{ modbits |= MB_INT; $$ = $1; }
		| CHAR
			{ modbits |= MB_CHAR; $$ = $1; }
		| FLOAT
			{ modbits |= MB_FLOAT; $$ = $1; }
		| DOUBLE
			{ modbits |= MB_DOUBLE; $$ = $1; }
		;

mod_list: modifier
			{ $$ = $1; }
		| mod_list modifier
			{ $$ = cat($1,ds(" "),$2); }
		;

modifier: UNSIGNED
			{ modbits |= MB_UNSIGNED; $$ = $1; }
		| LONG
			{ modbits |= MB_LONG; $$ = $1; }
		| SHORT
			{ modbits |= MB_SHORT; $$ = $1; }
		;
%%
#include "cdlex.c"

#define LORS	(MB_LONG|MB_SHORT)
#define UORL	(MB_UNSIGNED|MB_LONG)
#define UORS	(MB_UNSIGNED|MB_SHORT)
#define CORL	(MB_CHAR|MB_LONG)
#define CORS	(MB_CHAR|MB_SHORT)
#define CORU	(MB_CHAR|MB_UNSIGNED)

mbcheck()
{
	if ((modbits&LORS) == LORS)
		unsupp("conflicting 'short' and 'long'");
	if ((modbits&UORL) == UORL)
		unport("unsigned with long");
	if ((modbits&UORS) == UORS)
		unport("unsigned with short");
	if ((modbits&CORL) == CORL)
		unsupp("long char");
	if ((modbits&CORS) == CORS)
		unsupp("short char");
	if ((modbits&CORU) == CORU)
		unport("unsigned char");
}

savetype(s)
char *s;
{
	savedtype = s;
}

savename(s)
char *s;
{
	savedname = s;
}
