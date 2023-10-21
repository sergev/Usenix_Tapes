%{
# include	"ic.h"
# include	"y.tab.h"
extern char	*strcpy(), *malloc();
extern symbol	*lookUp();
extern double	atof();
extern int	ignorenl;
extern int	noprompt;
extern double	dotval;
#undef		input
#undef		unput
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?\
			(yylineno++,yytchar):yytchar)==EOF?popinput():yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
#define		YYINDEEP	20
FILE		*yyinstack[YYINDEEP];
FILE		**yyinpt = yyinstack + YYINDEEP;
int		linenos[YYINDEEP];
int		*linenopt = linenos + YYINDEEP;
char		*yyfile = "";
char		*fnames[YYINDEEP];
char		**fnamept = fnames + YYINDEEP;
int		yyfiledeep = 0;

%}
%%
"/*"		skipcomment();
auto		return AUTO;
define		return DEFINE;
quit		return QUIT;
exit		return QUIT;
shell		return SHELL;
edit		return EDIT;
read		return READ;
while		{ yylval.ival = WHILE; return WHILE; }
for		{ yylval.ival = FOR; return FOR; }
do		{ yylval.ival = DO; return DO; }
if		{ yylval.ival = IF; return IF; }
else		{ yylval.ival = ELSE; return ELSE; }
break		{ yylval.ival = BREAK; return BREAK; }
continue	{ yylval.ival = CONTINUE; return CONTINUE; }
return		{ yylval.ival = RETURN; return RETURN; }
";"		{ yylval.ival = SEMI; return SEMI; }
","		{ yylval.ival = COMMA; return COMMA; }
"."		{ yylval.dval = dotval; return NUMBER; }
\n		{ if (!ignorenl) { yylval.ival = NL; return NL; } }
"("		{ yylval.ival = OP; return OP; }
")"		{ yylval.ival = CP; return CP; }
"["		{ yylval.ival = OS; return OS; }
"]"		{ yylval.ival = CS; return CS; }
"{"		{ yylval.ival = OC; return OC; }
"}"		{ yylval.ival = CC; return CC; }
"+"		{ yylval.ival = PLUS; return PLUS; }
"-"		{ yylval.ival = MINUS; return MINUS; }
"*"		{ yylval.ival = TIMES; return TIMES; }
"/"		{ yylval.ival = DIVIDE; return DIVIDE; }
"%"		{ yylval.ival = MOD; return MOD; }
"!"		{ yylval.ival = BANG; return BANG; }
"#"		{ yylval.ival = POUND; return POUND; }
"^"		{ yylval.ival = POW; return POW; }
"="		{ yylval.ival = ASSIGN; return ASSIGN; }
"++"		{ yylval.ival = INC; return INC; }
"--"		{ yylval.ival = DEC; return DEC; }
"=="		{ yylval.ival = EQ; return EQ; }
"!="		{ yylval.ival = NE; return NE; }
"<"		{ yylval.ival = LT; return LT; }
">"		{ yylval.ival = GT; return GT; }
"<="		{ yylval.ival = LE; return LE; }
">="		{ yylval.ival = GE; return GE; }
"&&"		{ yylval.ival = AND; return AND; }
"||"		{ yylval.ival = OR; return OR; }
"?"		{ yylval.ival = QUEST; return QUEST; }
":"		{ yylval.ival = COLON; return COLON; }
" "		;
"\t"		;
\"([^\n\"]|\\\")*\"	{
			register char	*c, *s;
			yytext[yyleng - 1] = '\0';
			yylval.cval = malloc (yyleng - 1);
			c = yylval.cval;
			s = yytext + 1;
			while (*s) {
				if (*s == '\\') {
					switch (*++s) {
					case '0':
						*c++ = '\0';
						break;
					case 'b':
						*c++ = '\b';
						break;
					case 'n':
						*c++ = '\n';
						break;
					case 't':
						*c++ = '\t';
						break;
					case 'f':
						*c++ = '\f';
						break;
					default:
						*c++ = *s;
					}
				} else
					*c++ = *s;
				++s;
			}
			*c = '\0';
			return STRING;
		}
0[0-7]*		{
		yylval.dval = (double) atoo (yytext);
		return NUMBER;
		}
0x[0-9a-fA-F]+	{
		yylval.dval = (double) atox (yytext+2);
		return NUMBER;
		}
(([0-9]+((\.[0-9]*)?))|(\.[0-9]+))(([Ee][-+]?[0-9]+)?) {
		yylval.dval = atof (yytext);
		return NUMBER;
		}
[a-zA-Z][0-9a-zA-Z_]* {
		yylval.nval = lookUp (yytext);
		return NAME;
		}
.		fprintf (stderr, "character \\%o ignored\n", *yytext);
%%

skipcomment ()
{
	int	c;
	
	c = input();
	for (;;) {
		while (c != '*')
			c = input();
		c = input();
		if (c == '/')
			return;
	}
}

atox (s)
register char	*s;
{
	register int	result;
	register int	digit;
	
	result = 0;
	for (;;) {
		switch (*s) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			digit = *s - '0';
			break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			digit = *s - 'a' + 10;
			break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			digit = *s - 'A' + 10;
			break;
		default:
			return result;
		}
		result = (result << 4) + digit;
		++s;
	}
}

atoo (s)
register char	*s;
{
	register int	result;
	
	result = 0;
	while ('0' <= *s && *s <= '7')
		result = (result << 3) + *s++ - '0';
	return result;
}

lexfile(s)
char	*s;
{
	FILE	*f;
	f = fopen (s, "r");
	if (f == NULL) {
		fprintf (stderr, "cannot open file %s\n", s);
		return 0;
	}
	++yyfiledeep;
	yyin = f;	
	yyfile = s;
	return 1;
}

lexstdin()
{
	--yyfiledeep;
	fclose (yyin);
	yyin = stdin;
}
	
pushinput (s)
char	*s;
{
	FILE	*f;
	
	if (yyinpt == yyinstack) {
		fprintf (stderr, "files nested too deeply\n");
		return;
	}
	f = fopen (s, "r");
	if (f == NULL) {
		fprintf (stderr, "cannot open file %s\n", s);
		return;
	}
	++yyfiledeep;
	*--yyinpt = yyin;
	*--linenopt = yylineno;
	*--fnamept = yyfile;
	yyin = f;
	yylineno = 1;
	yyfile = s;
	return;
}

popinput ()
{
	int	c;

	do {
		fclose (yyin);
		if (yyinpt == yyinstack + YYINDEEP)
			return 0;
		yyin = *yyinpt++;
		yylineno = *linenopt++;
		yyfile = *fnamept++;
		--yyfiledeep;
	} while ((c = getc(yyin)) == EOF);
	return c;
}
