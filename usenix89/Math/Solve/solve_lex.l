%{
#undef input
#undef output
#undef unput
#include "solve.h"
#include "y.tab.h"
int lineno = 0;
%}
%%
[ \t]			{ ; }
[\n]			{	lineno++;
				return '\n';
			}
[0-9]+\.?|[0-9]*\.[0-9]+ {
				sscanf(yytext, "%lf", &yylval.fval);
				return NUMBER;
			}
[a-zA-Z][a-zA-Z0-9\_]* 	{
				yylval.sval = emalloc(strlen(yytext)+1);
				strcpy(yylval.sval, yytext);
				return NAME;
			}
.			{ return yytext[0]; }
