%{
/*
 * Copyright (c) 1984 by
 * Tektronix, Incorporated Beaverton, Oregon 97077
 * All rights reserved.
 *
 * Permission is hereby granted for personal, non-commercial
 * reproduction and use of this program, provided that this
 * notice is included in any copy.
 */
# include "stdio.h"
# include "cparen.h"
# include "y.tab.h"

static char *rcsid = "$Header: lex.l,v 1.12 84/07/17 09:45:18 bradn Stable $";

#define	MAXTYPES 100
char *typnam[MAXTYPES] = {	/* 'type' identifiers	*/
	"char",
	"double",
	"float",
	"int",
	"long",
	"short",
	"unsigned",
	"void",
	0			/* command-line types go here	*/
};
char **typend;			/* first free spot in typnam[]	*/

struct spec {			/* special keywords		*/
	char *s_name;		/* keyword text			*/
	int s_tok;		/* token to return		*/
	int s_val;		/* value to return		*/
};
struct spec specnam[] = {
	{"sizeof", SIZEOF, SIZEOF},
	{"struct", STRUCT, ISSTRUCT},
	{"union", STRUCT, ISUNION},
	{"enum", ENUM, ENUM},
	{"goto", KEYWORD, KEYWORD},
	{"return", KEYWORD, KEYWORD},
	{"if", KEYWORD, KEYWORD},
	{"else", KEYWORD, KEYWORD},
	{"while", KEYWORD, KEYWORD},
	{"do", KEYWORD, KEYWORD},
	{"for", KEYWORD, KEYWORD},
	{"switch", KEYWORD, KEYWORD},
	{"case", KEYWORD, KEYWORD},
	{0,0,0}
};

int tok;	/* a temp	*/
%}
%%

"<"	{yylval.lexval = nxttok(LT); return(RELOP);}
"<="	{yylval.lexval = nxttok(LE); return(RELOP);}
">"	{yylval.lexval = nxttok(GT); return(RELOP);}
">="	{yylval.lexval = nxttok(GE); return(RELOP);}
","	{yylval.lexval = nxttok(CM); return(CM);}
"/"	{yylval.lexval = nxttok(DIV); return(DIVOP);}
"%"	{yylval.lexval = nxttok(MOD); return(DIVOP);}
"+"	{yylval.lexval = nxttok(PLUS); return(PLUS);}
"-"	{yylval.lexval = nxttok(MINUS); return(MINUS);}
"<<"	{yylval.lexval = nxttok(LS); return(SHIFTOP);}
">>"	{yylval.lexval = nxttok(RS); return(SHIFTOP);}
"*"	{yylval.lexval = nxttok(MUL); return(MUL);}
"=="	{yylval.lexval = nxttok(EQ); return(EQUOP);}
"!="	{yylval.lexval = nxttok(NE); return(EQUOP);}
"&"	{yylval.lexval = nxttok(AND); return(AND);}
"|"	{yylval.lexval = nxttok(OR); return(OR);}
"^"	{yylval.lexval = nxttok(ER); return(ER);}
"&&"	{yylval.lexval = nxttok(ANDAND); return(ANDAND);}
"||"	{yylval.lexval = nxttok(OROR); return(OROR);}
"="	{yylval.lexval = nxttok(ASSIGN); return(ASSIGN);}
"?"	{yylval.lexval = nxttok(QUEST); return(QUEST);}
":"	{yylval.lexval = nxttok(COLON); return(COLON);}
"=/"	{yylval.lexval = nxttok(DIV); return(ASOP);}
"=%"	{yylval.lexval = nxttok(MOD); return(ASOP);}
"=+"	{yylval.lexval = nxttok(PLUS); return(ASOP);}
"=-"	{yylval.lexval = nxttok(MINUS); return(ASOP);}
"=<<"	{yylval.lexval = nxttok(LS); return(ASOP);}
"=>>"	{yylval.lexval = nxttok(RS); return(ASOP);}
"=*"	{yylval.lexval = nxttok(MUL); return(ASOP);}
"=&"	{yylval.lexval = nxttok(AND); return(ASOP);}
"=|"	{yylval.lexval = nxttok(OR); return(ASOP);}
"=^"	{yylval.lexval = nxttok(ER); return(ASOP);}
"++"	{yylval.lexval = nxttok(INCR); return(INCOP);}
"--"	{yylval.lexval = nxttok(DECR); return(INCOP);}
"!"	{yylval.lexval = nxttok(NOT); return(UNOP);}
"~"	{yylval.lexval = nxttok(COMPL); return(UNOP);}
"("	{yylval.lexval = nxttok(LP); return(LP);}
")"	{yylval.lexval = nxttok(RP); return(RP);}
"["	{yylval.lexval = nxttok(LB); return(LB);}
"]"	{yylval.lexval = nxttok(RB); return(RB);}
"->"	{yylval.lexval = nxttok(STREF); return(STROP);}
"."	{yylval.lexval = nxttok(DOT); return(STROP);}
";"	{yylval.lexval = nxttok(SM); return(SM);}
"{"	{/* ignore curly-braces */ (void) nxttok(LC);}
"}"	{/* ignore curly-braces */ (void) nxttok(RC);}
0x[0-9a-zA-Z]+[Ll]?	{/* hex constant */
			yylval.lexval = nxttok(NAME); return(NAME);
		}
[0-9]+[Ll]?	{/* decimal or octal constant */
			yylval.lexval = nxttok(NAME); return(NAME);
		}
[0-9]+[Ee]([+-][0-9])?[0-9]*		|
\.[0-9]+([Ee]([+-][0-9])?[0-9]*)?	|
[0-9]+\.[0-9]*([Ee]([+-][0-9])?[0-9]*)?	{/* floating constant */
			yylval.lexval = nxttok(NAME); return(NAME);
		}
\'([^'\n]|(\\[\n\']))+\'	{/* char constant */
				yylval.lexval = nxttok(NAME); return(NAME);
			}
\"([^"\n]|(\\[\n\"]))*\"	{/* string constant */
				yylval.lexval = nxttok(NAME); return(NAME);
			}
[a-zA-Z_$][a-zA-Z0-9_$]*	{/* identifier, type, or keyword */
				yylval.lexval = nxttok(NAME);

				/* take care of magic keywords	*/

				if ((tok =
				   specof(yytext, &yylval.lexval->l_val))) {
					return(tok);
				}

				/* take care of types and identifiers	*/

				if (istype(yytext)) {
					return(TYPE);
				} else {
					return(NAME);
				}
			}
[ \t\n]+	{/* whitespace */ (void) nxttok(FLUFF);}
\/\*([^\*]|(\*[^\/]))*\*?\*\/	{/* comment */ (void) nxttok(FLUFF);}
.	{fprintf(stderr, "%s: unknown character `%c'\n", cmd, yytext[0]);}

%%

static
initype()	/* init the type list					*/
{
	if (typend) {
		return;
	}
	for (typend = &typnam[0]; *typend; typend++)
		;
}

newtype(s)	/* insert this string into the list of type strings	*/
char *s;
{
	char **t;

	initype();
	for (t = &typnam[0]; t < typend && strcmp(s, *t) != 0; t++)
		;
	if (t <typend) {
		/* this type already exists - no need to insert it again */
		return;
	}
	if (typend >= &typnam[MAXTYPES]) {
		fprintf(stderr, "%s: out of memory\n", cmd);
		exit(1);
	}
	*typend++ = s;
}

static int
istype(s)	/* returns 'this string is a TYPE name'			*/
char *s;
{
	char **t;


	initype();
	for (t = &typnam[0]; t < typend && strcmp(s, *t) != 0; t++)
		;
	return(t < typend);
}

static int	/* token to return (or zero, if not a special keyword)	*/
specof(t, vp)
char *t;	/* possible keyword text	*/
int *vp;	/* where to put value		*/
{
	struct spec *sp;

	for (sp = &specnam[0]; sp->s_name && strcmp(t, sp->s_name) != 0;
	  sp++)
		;
	if (sp->s_name) {
		*vp = sp->s_val;
		return(sp->s_tok);
	}
	return(0);
}

struct lexed *
nxttok(val)			/* insert the val at the end of the list */
int val;
{
	/* curtok: current token in the input list	*/
	static struct lexed *curtok = &firsttok;

	curtok = instok(val, yytext, curtok);
	return(curtok);
}

struct lexed *
instok(val, text, after)	/* insert token into input list		*/
int val;			/* token value (yylval'ish)		*/
char *text;			/* associated text (to be copied)	*/
struct lexed *after;		/* node to insert after			*/
{
	char *malloc();
	struct lexed *node;


	if (!(node = (struct lexed *) malloc(sizeof(struct lexed)))) {
		fprintf(stderr, "%s: out of memory.\n", cmd);
		exit(1);
	}
	if (!(node->l_text = malloc(strlen(text) + 1))) {
		fprintf(stderr, "%s: out of memory.\n", cmd);
		exit(1);
	}
	(void) strcpy(node->l_text, text);
	node->l_val = val;

	/* insert into the list after the node 'after'	*/

	after->l_next->l_prev = node;
	node->l_next = after->l_next;
	node->l_prev = after;
	after->l_next = node;

	return(node);
}
