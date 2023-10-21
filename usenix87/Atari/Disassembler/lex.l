%{
#undef ECHO
#include "dis.h"
int lineno = 0;
%}
%%
[ \t] { ; }
[\n] {	lineno++;
				return '\n';
}
\.EQ { 
				return EQ;
}
\.LI { 
				return LI;
}
\.eq { 
				return EQ;
}
\.li { 
				return LI;
}
".trace" {
				return TSTART;
}
".stop" {
				return TSTOP;
}
[0-9]+	{
				(void)sscanf(yytext, "%d", &token.ival); 
				return NUMBER;
}
\$[A-Fa-f0-9]+ {
				(void)sscanf(yytext+1, "%x", &token.ival);
				return NUMBER;
}
[A-Za-z][A-Za-z0-9_]* {
				token.sval = emalloc((unsigned) strlen(yytext)+1);
				(void) strcpy((char *) token.sval, (char *) yytext);
				return NAME;
}
\*.* {
				return COMMENT;
}
.			{ return yytext[0]; }
%%

char *
emalloc(n)
unsigned n;
{
	char *ptr, *malloc();

	if ((ptr = malloc(n)) == (char *) 0) {
		(void) fprintf(stderr,"out of core");
		exit(1);
	}
	return ptr;
}

int
yyerror(s)
char *s;
{
	(void) fprintf(stderr,"%s in line %d\n", s, lineno);
	exit(1);
}
