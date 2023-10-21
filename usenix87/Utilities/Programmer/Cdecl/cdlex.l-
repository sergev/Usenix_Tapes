%{
#include <ctype.h>

char *visible();
%}
N	[0-9]
A	[A-Z_a-z]
AN	[0-9A-Z_a-z]
%%
array		return ARRAY;
as			return AS;
cast		return CAST;
declare		return DECLARE;
explain		return EXPLAIN;
function	return FUNCTION;
help		return HELP;
into		return INTO;
of			return OF;
pointer		return POINTER;
returning	return RETURNING;
to			return TO;

char		{ yylval.dynstr = ds(yytext); return CHAR; }
double		{ yylval.dynstr = ds(yytext); return DOUBLE; }
enum		{ yylval.dynstr = ds(yytext); return STRUCTUNION; }
float		{ yylval.dynstr = ds(yytext); return FLOAT; }
int			{ yylval.dynstr = ds(yytext); return INT; }
long		{ yylval.dynstr = ds(yytext); return LONG; }
short		{ yylval.dynstr = ds(yytext); return SHORT; }
struct		{ yylval.dynstr = ds(yytext); return STRUCTUNION; }
union		{ yylval.dynstr = ds(yytext); return STRUCTUNION; }
unsigned	{ yylval.dynstr = ds(yytext); return UNSIGNED; }

{A}{AN}*	{ yylval.dynstr = ds(yytext); return NAME; }
{N}+		{ yylval.dynstr = ds(yytext); return NUMBER; }

[\t ]		;
[*[\]()\n]		return *yytext;
.			{
				printf("bad character '%s'\n",visible(*yytext));
				return *yytext;
			}
%%
char *
visible(c)
{
	static char buf[5];

	c &= 0377;
	if (isprint(c)) {
		buf[0] = c;
		buf[1] = '\0';
	} else
		sprintf(buf,"\\%02o",c);
	return buf;
}
