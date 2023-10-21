%{

/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * The lexical analyzer for kafka.
 */

#include "y.tab.h"

char lookahead[100];	/* For pushback of characters. */
int nlk = 0;		/* # characters pushed back. */
extern FILE *inp;	/* The input file. */
extern int yylval;

%}

%%

\<[^ \t\n]*\>		{ /* A non-terminal. */
			yytext[yyleng - 1] = '\0';
			yylval = (int) (yytext + 1);
			return (NONTERM);
			}

[ \t\n]*		{ /* White space. */
			}

^#.*$			{ /* A comment. Ignore. */
			}

[^ \"\t\n=\<\>;\{\}\%]*	{ /* An unquoted terminal string. */
			yylval = (int) yytext;
			return (TERM);
			}

\"[^\"]*\"		{ /* A quoted terminal. */
			yytext[yyleng - 1] = '\0';
			yylval = (int) (yytext + 1);
			return (TERM);
			}

=			{ /* The rule operator. */
			return (RULEOP);
			}

^\%\%			{ /* The section seperator. */
			return (SSEP);
			}

^\%[A-Za-z].*$		{ /* A kafka escape. */
			yylval = (int) yytext[1];
			return (KESC);
			}

^\%\{			{ /* Begin def section C code. */
			return (COPEN);
			}

^\%\}			{ /* End def section C code. */
			return (CCLOSE);
			}

\{			{ /* Open brace. */
			return (LBRACE);
			}

\}			{ /* Close brace. */
			return (RBRACE);
			}

\;			{ /* Semicolon. */
			return (SEMI);
			}


%%

/* The redefined input and unput functions -- they must be usable by
 * other routines. 
 */

int lineno = 0;

char input()
{
	int c;
	if (nlk == 0) {
		c = getc(inp);
		if (c == '\n') 
			lineno++;
		if (c == EOF) 
			return (0);
		else
			return (c);
	} else {
		c = lookahead[--nlk];
		return (c);
	}
}

unput(c)
char c;
{
	lookahead[nlk++] = c;
	return;
}

/* Don't need this thing... */

yywrap()
{
	return (1);
}

