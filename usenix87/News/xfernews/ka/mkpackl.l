%{
#include "y.tab.h"
#define V(val) yylval = savestr(yytext); return val;
extern int yylval ;
%}

%%

[A-Za-z_][A-Za-z_0-9]*	{ V(WORD) }
\"([^"\n]|\\.)*\"	{ V(STRING) }
[0-9]+			{ V(NUMBER) }
:.*\n			{ yylval = savestr(yytext+1); return DIRECT; }
\'.\'			{ yylval = yytext[1]; return CCONST; }
"Usenet header lines:"	{ return NHTYPES; }
"Character lengths:"	{ return CHARTAB; }
"/*"	{
		char c1, c2 ;
/*
		ECHO ;
		while (putchar(input()) != '*' || putchar(input()) != '/') ;
		putchar('\n') ;
*/		while (input() != '*' || input() != '/') ;
	}
\#.*\n	{;}
[ \t\n]	{;}
.	{ yyerror("lexical error") ; }

%%

savestr(s)
	char *s ;
	{
	char *p ;

	if ((p = malloc(strlen(s) + 1)) == NULL)
		yyerror("Out of space") ;
	strcpy(p, s) ;
	return p ;
}
