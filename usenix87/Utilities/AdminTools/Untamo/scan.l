
%{
/*
 *  Lex grammer to scan input file for untamo
 */
#include <stdio.h>
#include "y.tab.h"
#define makestr(Z)	((char *)strcpy(malloc(strlen(Z)+1),Z))
int linenum = 1;	/* current line number for error messages */
%}

%%
exempt		return EXEMPT;
timeout		return TIMEOUT;
sleep		return SLEEP;
login		return LOGIN;
group		return GROUP;
tty		return TTY;
rld		return RLD;
cluster		return CLUSTER;
default		return DEFAULT;
idle		return IDLE;
multiple	return MULTIPLE;
session		return SESSION;
threshold	return THRESHOLD;
all		return ALL;

[/A-Za-z][/A-Za-z0-9_]*	{   
		    yylval.sb = makestr(yytext);
		    return NAME; 
		}

[0-9]+		{
		    yylval.nb = atoi(yytext);
		    return NUM;
		}

"*".*		;
"\n"		{ 
		    linenum++; 
		    return NL;
		}
[ \t]*		;
.		{
		    static char *errormsg ="Illegal character ' '.";

		    errormsg[19] = yytext[0];
		    yyerror( errormsg );
		}
