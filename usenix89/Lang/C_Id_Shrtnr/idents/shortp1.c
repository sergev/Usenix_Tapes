# include "stdio.h"
# define U(x) x
# define NLSTATE yyprev=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int     yyleng;
extern char yytext[];
int     yymorfg;
extern char *yysptr,
            yysbuf[];
int     yytchar;
FILE * yyin = {
    stdin
}, *yyout = {
    stdout
};
extern int  yylineno;
struct yysvf {
    struct yywork  *yystoff;
    struct yysvf   *yyother;
    int    *yystops;
};
struct yysvf   *yyestate;
extern struct yysvf yysvec[],
                   *yybgin;
# define YYNEWLINE 10
yylex () {
    int     nstr;
    extern int  yyprev;
    while ((nstr = yylook ()) >= 0)
yyfussy: switch (nstr) {
	    case 0: 
		if (yywrap ())
		    return (0);
		break;
	    case 1: 
		printit ();
		break;
	    case 2: 
		quote ();
		break;
	    case -1: 
		break;
	    default: 
		fprintf (yyout, "bad switch yylook %d", nstr);
	} return (0);
}
/* end of yylex */
FILE * idents, *fopen ();
#include "identp1.h"
main () {
    idents = fopen ("ident", "w");
    if (idents == NULL) {
	fprintf (stderr, "panic! Cannot open header file!\n");
	exit (1);
    }
    yylex ();
}
printit () {
    if (yyleng > IDENTLEN)
	fprintf (idents, "%s\n", yytext);
}
quote () {
    int     c;

    while (1) {
	c = input ();
	switch (c) {
	    case '"': 
		return (0);
	    case '\\': 
		c = input ();
	}
    }
}
yywrap () {
    return (1);
}
int     yyvstop[] = {
    0,

    2,
    0,

    1,
    0,

    1,
    0,
    0
};
# define YYTYPE char
struct yywork {
    YYTYPE verify, advance;
}               yycrank[] = {
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 1, 3,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    1, 4, 0, 0, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    1, 4, 1, 4, 1, 4, 1, 4,
                    4, 5, 4, 5, 4, 5, 4, 5,
                    4, 5, 4, 5, 4, 5, 4, 5,
                    4, 5, 4, 5, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 0, 0,
                    0, 0, 0, 0, 0, 0, 4, 4,
                    0, 0, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 4, 4,
                    4, 4, 4, 4, 4, 4, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 0, 0, 0, 0, 0, 0,
                    0, 0, 5, 5, 0, 0, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 5, 5, 5, 5, 5, 5,
                    5, 5, 0, 0, 0, 0, 0, 0,
                    0, 0
};
struct yysvf    yysvec[] = {
    0, 0, 0,
    yycrank + 1, 0, 0,
    yycrank + 0, yysvec + 1, 0,
    yycrank + 0, 0, yyvstop + 1,
    yycrank + 76, 0, yyvstop + 3,
    yycrank + 134, yysvec + 4, yyvstop + 5,
    0, 0, 0
};
struct yywork  *yytop = yycrank + 256;
struct yysvf   *yybgin = yysvec + 1;
char    yymatch[] = {
    00, 01, 01, 01, 01, 01, 01, 01,
    01, 01, 01, 01, 01, 01, 01, 01,
    01, 01, 01, 01, 01, 01, 01, 01,
    01, 01, 01, 01, 01, 01, 01, 01,
    01, 01, '"', 01, 01, 01, 01, 01,
    01, 01, 01, 01, 01, 01, 01, 01,
    '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', 01, 01, 01, 01, 01, 01,
    01, 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 01, 01, 01, 01, 'A',
    01, 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
    'A', 'A', 'A', 01, 01, 01, 01, 01,
    0
};
char    yyextra[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0
};
/*	ncform	4.1	83/08/11	*/

int     yylineno = 1;
# define YYU(x) x
# define NLSTATE yyprev=YYNEWLINE
char    yytext[YYLMAX];
struct yysvf   *yylstate[YYLMAX],
              **yylsp,
              **yyolsp;
char    yysbuf[YYLMAX];
char   *yysptr = yysbuf;
int    *yyfnd;
extern struct yysvf *yyestate;
int     yyprev = YYNEWLINE;
yylook () {
    register struct yysvf  *yystate,
                          **lsp;
    register struct yywork *yyt;
    struct yysvf   *yyz;
    int     yych;
    struct yywork  *yyr;
# ifdef LEXDEBUG
    int     debug;
# endif
    char   *yylastch;
 /* start off machines */
# ifdef LEXDEBUG
    debug = 0;
# endif
    if (!yymorfg)
	yylastch = yytext;
    else {
	yymorfg = 0;
	yylastch = yytext + yyleng;
    }
    for (;;) {
	lsp = yylstate;
	yyestate = yystate = yybgin;
	if (yyprev == YYNEWLINE)
	    yystate++;
	for (;;) {
# ifdef LEXDEBUG
	    if (debug)
		fprintf (yyout, "state %d\n", yystate - yysvec - 1);
# endif
	    yyt = yystate -> yystoff;
	    if (yyt == yycrank) {/* may not be any transitions */
		yyz = yystate -> yyother;
		if (yyz == 0)
		    break;
		if (yyz -> yystoff == yycrank)
		    break;
	    }
	    *yylastch++ = yych = input ();
    tryagain: 
# ifdef LEXDEBUG
	    if (debug) {
		fprintf (yyout, "char ");
		allprint (yych);
		putchar ('\n');
	    }
# endif
	    yyr = yyt;
	    if ((int) yyt > (int) yycrank) {
		yyt = yyr + yych;
		if (yyt <= yytop && yyt -> verify + yysvec == yystate) {
		    if (yyt -> advance + yysvec == YYLERR) {
				/* error transitions */
			unput (*--yylastch);
			break;
		    }
		    *lsp++ = yystate = yyt -> advance + yysvec;
		    goto contin;
		}
	    }
# ifdef YYOPTIM
	    else
		if ((int) yyt < (int) yycrank) {/* r < yycrank */
		    yyt = yyr = yycrank + (yycrank - yyt);
# ifdef LEXDEBUG
		    if (debug)
			fprintf (yyout, "compressed state\n");
# endif
		    yyt = yyt + yych;
		    if (yyt <= yytop && yyt -> verify + yysvec == yystate) {
			if (yyt -> advance + yysvec == YYLERR) {
				/* error transitions */
			    unput (*--yylastch);
			    break;
			}
			*lsp++ = yystate = yyt -> advance + yysvec;
			goto contin;
		    }
		    yyt = yyr + YYU (yymatch[yych]);
# ifdef LEXDEBUG
		    if (debug) {
			fprintf (yyout, "try fall back character ");
			allprint (YYU (yymatch[yych]));
			putchar ('\n');
		    }
# endif
		    if (yyt <= yytop && yyt -> verify + yysvec == yystate) {
			if (yyt -> advance + yysvec == YYLERR) {
				/* error transition */
			    unput (*--yylastch);
			    break;
			}
			*lsp++ = yystate = yyt -> advance + yysvec;
			goto contin;
		    }
		}
	    if ((yystate = yystate -> yyother) && (yyt = yystate -> yystoff) != yycrank) {
# ifdef LEXDEBUG
		if (debug)
		    fprintf (yyout, "fall back to state %d\n", yystate - yysvec - 1);
# endif
		goto tryagain;
	    }
# endif
	    else {
		unput (*--yylastch);
		break;
	    }
    contin: 
# ifdef LEXDEBUG
	    if (debug) {
		fprintf (yyout, "state %d char ", yystate - yysvec - 1);
		allprint (yych);
		putchar ('\n');
	    }
# endif
	    ;
	}
# ifdef LEXDEBUG
	if (debug) {
	    fprintf (yyout, "stopped at %d with ", *(lsp - 1) - yysvec - 1);
	    allprint (yych);
	    putchar ('\n');
	}
# endif
	while (lsp-- > yylstate) {
	    *yylastch-- = 0;
	    if (*lsp != 0 && (yyfnd = (*lsp) -> yystops) && *yyfnd > 0) {
		yyolsp = lsp;
		if (yyextra[*yyfnd]) {/* must backup */
		    while (yyback ((*lsp) -> yystops, -*yyfnd) != 1 && lsp > yylstate) {
			lsp--;
			unput (*yylastch--);
		    }
		}
		yyprev = YYU (*yylastch);
		yylsp = lsp;
		yyleng = yylastch - yytext + 1;
		yytext[yyleng] = 0;
# ifdef LEXDEBUG
		if (debug) {
		    fprintf (yyout, "\nmatch ");
		    sprint (yytext);
		    fprintf (yyout, " action %d\n", *yyfnd);
		}
# endif
		return (*yyfnd++);
	    }
	    unput (*yylastch);
	}
	if (yytext[0] == 0 /* && feof(yyin) */ ) {
	    yysptr = yysbuf;
	    return (0);
	}
	yyprev = yytext[0] = input ();
	if (yyprev > 0)
	    output (yyprev);
	yylastch = yytext;
# ifdef LEXDEBUG
	if (debug)
	    putchar ('\n');
# endif
    }
}
yyback (p, m)
int    *p;
{
    if (p == 0)
	return (0);
    while (*p) {
	if (*p++ == m)
	    return (1);
    }
    return (0);
}
 /* the following are only used in the lex library */
yyinput () {
    return (input ());
}
yyoutput (c)
int     c; {
    output (c);
}
yyunput (c)
int     c; {
    unput (c);
}
