# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
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
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#include <ctype.h>

char *visible();
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
	return ARRAY;
break;
case 2:
		return AS;
break;
case 3:
	return CAST;
break;
case 4:
	return DECLARE;
break;
case 5:
	return EXPLAIN;
break;
case 6:
return FUNCTION;
break;
case 7:
	return HELP;
break;
case 8:
	return INTO;
break;
case 9:
		return OF;
break;
case 10:
	return POINTER;
break;
case 11:
return RETURNING;
break;
case 12:
		return TO;
break;
case 13:
	{ yylval.dynstr = ds(yytext); return CHAR; }
break;
case 14:
	{ yylval.dynstr = ds(yytext); return DOUBLE; }
break;
case 15:
	{ yylval.dynstr = ds(yytext); return STRUCTUNION; }
break;
case 16:
	{ yylval.dynstr = ds(yytext); return FLOAT; }
break;
case 17:
	{ yylval.dynstr = ds(yytext); return INT; }
break;
case 18:
	{ yylval.dynstr = ds(yytext); return LONG; }
break;
case 19:
	{ yylval.dynstr = ds(yytext); return SHORT; }
break;
case 20:
	{ yylval.dynstr = ds(yytext); return STRUCTUNION; }
break;
case 21:
	{ yylval.dynstr = ds(yytext); return STRUCTUNION; }
break;
case 22:
{ yylval.dynstr = ds(yytext); return UNSIGNED; }
break;
case 23:
	{ yylval.dynstr = ds(yytext); return INT; }
break;
case 24:
{ yylval.dynstr = ds(yytext); return NAME; }
break;
case 25:
	{ yylval.dynstr = ds(yytext); return NUMBER; }
break;
case 26:
	;
break;
case 27:
	return *yytext;
break;
case 28:
		{
				printf("bad character '%s'\n",visible(*yytext));
				return *yytext;
			}
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
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
int yyvstop[] = {
0,

28,
0,

26,
28,
0,

27,
0,

27,
28,
0,

25,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

24,
28,
0,

25,
0,

24,
0,

24,
0,

2,
24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

9,
24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

12,
24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

17,
24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

3,
24,
0,

13,
24,
0,

24,
0,

24,
0,

15,
24,
0,

24,
0,

24,
0,

24,
0,

7,
24,
0,

8,
24,
0,

18,
24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

23,
24,
0,

1,
24,
0,

24,
0,

24,
0,

24,
0,

16,
24,
0,

24,
0,

24,
0,

24,
0,

19,
24,
0,

24,
0,

21,
24,
0,

24,
0,

24,
0,

14,
24,
0,

24,
0,

24,
0,

24,
0,

24,
0,

20,
24,
0,

24,
0,

4,
24,
0,

5,
24,
0,

24,
0,

10,
24,
0,

24,
0,

24,
0,

6,
24,
0,

24,
0,

22,
24,
0,

11,
24,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,6,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,7,	7,24,	7,24,	
7,24,	7,24,	7,24,	7,24,	
7,24,	7,24,	7,24,	7,24,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,8,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,9,	0,0,	
1,10,	1,11,	1,12,	1,13,	
10,28,	1,14,	1,15,	14,36,	
17,39,	1,16,	0,0,	10,29,	
1,17,	1,18,	0,0,	1,19,	
1,20,	1,21,	1,22,	1,23,	
2,9,	15,37,	2,10,	2,11,	
2,12,	2,13,	11,30,	2,14,	
2,15,	9,26,	9,27,	2,16,	
16,38,	18,40,	2,17,	2,18,	
11,31,	2,19,	2,20,	2,21,	
2,22,	2,23,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
19,41,	21,44,	22,45,	23,46,	
26,47,	28,48,	29,49,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	30,50,	31,51,	32,52,	
33,53,	8,25,	34,54,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	8,25,	8,25,	8,25,	
8,25,	12,32,	20,42,	13,34,	
35,55,	36,56,	37,57,	38,58,	
40,59,	41,60,	42,61,	12,33,	
13,35,	43,62,	20,43,	45,63,	
46,65,	47,66,	48,67,	49,68,	
50,69,	51,70,	52,71,	53,72,	
54,73,	45,64,	55,74,	56,75,	
57,76,	58,77,	59,78,	60,79,	
61,80,	62,81,	63,82,	64,83,	
65,84,	66,85,	69,86,	70,87,	
72,88,	73,89,	74,90,	78,91,	
79,92,	80,93,	81,94,	82,95,	
83,96,	86,97,	87,98,	88,99,	
90,100,	91,101,	92,102,	94,103,	
96,104,	97,105,	99,106,	100,107,	
101,108,	102,109,	104,110,	107,111,	
109,112,	110,113,	112,114,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-23,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+0,	0,		yyvstop+8,
yycrank+2,	0,		yyvstop+11,
yycrank+94,	0,		yyvstop+14,
yycrank+15,	yysvec+8,	yyvstop+17,
yycrank+7,	yysvec+8,	yyvstop+20,
yycrank+25,	yysvec+8,	yyvstop+23,
yycrank+107,	yysvec+8,	yyvstop+26,
yycrank+111,	yysvec+8,	yyvstop+29,
yycrank+6,	yysvec+8,	yyvstop+32,
yycrank+11,	yysvec+8,	yyvstop+35,
yycrank+21,	yysvec+8,	yyvstop+38,
yycrank+6,	yysvec+8,	yyvstop+41,
yycrank+22,	yysvec+8,	yyvstop+44,
yycrank+51,	yysvec+8,	yyvstop+47,
yycrank+114,	yysvec+8,	yyvstop+50,
yycrank+42,	yysvec+8,	yyvstop+53,
yycrank+44,	yysvec+8,	yyvstop+56,
yycrank+44,	yysvec+8,	yyvstop+59,
yycrank+0,	yysvec+7,	yyvstop+62,
yycrank+0,	yysvec+8,	yyvstop+64,
yycrank+42,	yysvec+8,	yyvstop+66,
yycrank+0,	yysvec+8,	yyvstop+68,
yycrank+42,	yysvec+8,	yyvstop+71,
yycrank+61,	yysvec+8,	yyvstop+73,
yycrank+86,	yysvec+8,	yyvstop+75,
yycrank+69,	yysvec+8,	yyvstop+77,
yycrank+70,	yysvec+8,	yyvstop+79,
yycrank+76,	yysvec+8,	yyvstop+81,
yycrank+79,	yysvec+8,	yyvstop+83,
yycrank+110,	yysvec+8,	yyvstop+85,
yycrank+113,	yysvec+8,	yyvstop+87,
yycrank+106,	yysvec+8,	yyvstop+89,
yycrank+113,	yysvec+8,	yyvstop+91,
yycrank+0,	yysvec+8,	yyvstop+93,
yycrank+119,	yysvec+8,	yyvstop+96,
yycrank+109,	yysvec+8,	yyvstop+98,
yycrank+115,	yysvec+8,	yyvstop+100,
yycrank+115,	yysvec+8,	yyvstop+102,
yycrank+0,	yysvec+8,	yyvstop+104,
yycrank+126,	yysvec+8,	yyvstop+107,
yycrank+127,	yysvec+8,	yyvstop+109,
yycrank+136,	yysvec+8,	yyvstop+111,
yycrank+118,	yysvec+8,	yyvstop+113,
yycrank+121,	yysvec+8,	yyvstop+115,
yycrank+128,	yysvec+8,	yyvstop+117,
yycrank+139,	yysvec+8,	yyvstop+119,
yycrank+129,	yysvec+8,	yyvstop+121,
yycrank+131,	yysvec+8,	yyvstop+123,
yycrank+143,	yysvec+8,	yyvstop+125,
yycrank+143,	yysvec+8,	yyvstop+127,
yycrank+131,	yysvec+8,	yyvstop+129,
yycrank+133,	yysvec+8,	yyvstop+131,
yycrank+142,	yysvec+8,	yyvstop+134,
yycrank+136,	yysvec+8,	yyvstop+136,
yycrank+130,	yysvec+8,	yyvstop+138,
yycrank+134,	yysvec+8,	yyvstop+140,
yycrank+132,	yysvec+8,	yyvstop+142,
yycrank+139,	yysvec+8,	yyvstop+144,
yycrank+146,	yysvec+8,	yyvstop+146,
yycrank+152,	yysvec+8,	yyvstop+148,
yycrank+132,	yysvec+8,	yyvstop+150,
yycrank+0,	yysvec+8,	yyvstop+152,
yycrank+0,	yysvec+8,	yyvstop+155,
yycrank+157,	yysvec+8,	yyvstop+158,
yycrank+147,	yysvec+8,	yyvstop+160,
yycrank+0,	yysvec+8,	yyvstop+162,
yycrank+159,	yysvec+8,	yyvstop+165,
yycrank+141,	yysvec+8,	yyvstop+167,
yycrank+142,	yysvec+8,	yyvstop+169,
yycrank+0,	yysvec+8,	yyvstop+171,
yycrank+0,	yysvec+8,	yyvstop+174,
yycrank+0,	yysvec+8,	yyvstop+177,
yycrank+143,	yysvec+8,	yyvstop+180,
yycrank+146,	yysvec+8,	yyvstop+182,
yycrank+145,	yysvec+8,	yyvstop+184,
yycrank+163,	yysvec+8,	yyvstop+186,
yycrank+153,	yysvec+8,	yyvstop+188,
yycrank+161,	yysvec+8,	yyvstop+190,
yycrank+0,	yysvec+8,	yyvstop+192,
yycrank+0,	yysvec+8,	yyvstop+195,
yycrank+151,	yysvec+8,	yyvstop+198,
yycrank+165,	yysvec+8,	yyvstop+200,
yycrank+162,	yysvec+8,	yyvstop+202,
yycrank+0,	yysvec+8,	yyvstop+204,
yycrank+163,	yysvec+8,	yyvstop+207,
yycrank+168,	yysvec+8,	yyvstop+209,
yycrank+160,	yysvec+8,	yyvstop+211,
yycrank+0,	yysvec+8,	yyvstop+213,
yycrank+155,	yysvec+8,	yyvstop+216,
yycrank+0,	yysvec+8,	yyvstop+218,
yycrank+162,	yysvec+8,	yyvstop+221,
yycrank+172,	yysvec+8,	yyvstop+223,
yycrank+0,	yysvec+8,	yyvstop+225,
yycrank+164,	yysvec+8,	yyvstop+228,
yycrank+164,	yysvec+8,	yyvstop+230,
yycrank+162,	yysvec+8,	yyvstop+232,
yycrank+172,	yysvec+8,	yyvstop+234,
yycrank+0,	yysvec+8,	yyvstop+236,
yycrank+177,	yysvec+8,	yyvstop+239,
yycrank+0,	yysvec+8,	yyvstop+241,
yycrank+0,	yysvec+8,	yyvstop+244,
yycrank+169,	yysvec+8,	yyvstop+247,
yycrank+0,	yysvec+8,	yyvstop+249,
yycrank+170,	yysvec+8,	yyvstop+252,
yycrank+181,	yysvec+8,	yyvstop+254,
yycrank+0,	yysvec+8,	yyvstop+256,
yycrank+179,	yysvec+8,	yyvstop+259,
yycrank+0,	yysvec+8,	yyvstop+261,
yycrank+0,	yysvec+8,	yyvstop+264,
0,	0,	0};
struct yywork *yytop = yycrank+282;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'(' ,'(' ,'(' ,01  ,01  ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'(' ,01  ,'(' ,01  ,'A' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,
'A' ,'A' ,'A' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	@(#)ncform	1.2	*/
int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
yyback(p, m)
	int *p;
{
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
yyinput(){
	return(input());
	}
yyoutput(c)
  int c; {
	output(c);
	}
yyunput(c)
   int c; {
	unput(c);
	}
