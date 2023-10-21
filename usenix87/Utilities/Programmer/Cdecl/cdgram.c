
# line 2 "cdgram.y"
#include <stdio.h>

#define	MB_SHORT	0001
#define	MB_LONG		0002
#define	MB_UNSIGNED	0004
#define MB_INT		0010
#define MB_CHAR		0020
#define MB_FLOAT	0040
#define MB_DOUBLE	0100

int modbits = 0;
int arbdims = 1;
char *savedtype;
char *savedname;
char *ds(), *cat();
char *index(), *malloc();
char prev;

# line 21 "cdgram.y"
typedef union  {
	char *dynstr;
	struct {
		char *left;
		char *right;
	} halves;
} YYSTYPE;
# define DECLARE 257
# define CAST 258
# define INTO 259
# define AS 260
# define HELP 261
# define EXPLAIN 262
# define FUNCTION 263
# define RETURNING 264
# define POINTER 265
# define TO 266
# define ARRAY 267
# define OF 268
# define NAME 269
# define NUMBER 270
# define STRUCTUNION 271
# define UNSIGNED 272
# define LONG 273
# define SHORT 274
# define INT 275
# define CHAR 276
# define FLOAT 277
# define DOUBLE 278
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 219 "cdgram.y"

#include "cdlex.c"

#define LORS	(MB_LONG|MB_SHORT)
#define UORL	(MB_UNSIGNED|MB_LONG)
#define UORS	(MB_UNSIGNED|MB_SHORT)
#define CORL	(MB_CHAR|MB_LONG)
#define CORS	(MB_CHAR|MB_SHORT)
#define CORU	(MB_CHAR|MB_UNSIGNED)

mbcheck()
{
	if ((modbits&LORS) == LORS)
		unsupp("conflicting 'short' and 'long'");
	if ((modbits&UORL) == UORL)
		unport("unsigned with long");
	if ((modbits&UORS) == UORS)
		unport("unsigned with short");
	if ((modbits&CORL) == CORL)
		unsupp("long char");
	if ((modbits&CORS) == CORS)
		unsupp("short char");
	if ((modbits&CORU) == CORU)
		unport("unsigned char");
}

savetype(s)
char *s;
{
	savedtype = s;
}

savename(s)
char *s;
{
	savedname = s;
}
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 39
# define YYLAST 278
yytabelem yyact[]={

    25,    31,    32,    33,    27,    28,    29,    30,    31,    32,
    33,    27,    28,    29,    30,    57,    20,    53,    19,    35,
    61,    37,    48,    36,    11,     7,    10,    62,    54,    51,
    67,    15,    16,    64,    41,    34,    65,    59,    56,    26,
    24,    17,    55,    49,    40,    14,     9,    38,    13,     2,
     1,    22,    39,    52,    12,    42,    18,    23,     0,     0,
     0,    44,    45,    47,    46,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    43,    60,     0,     0,     0,
    63,     0,     0,     0,     0,     0,     0,     0,    66,     0,
     0,     0,     0,    68,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    58,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    21,     0,     0,     0,     0,
     0,     0,     0,    50,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     8,     4,     5,     0,     0,     3,     6 };
yytabelem yypact[]={

 -1000,    15, -1000,    36,  -243,  -245, -1000, -1000,    35, -1000,
  -229,  -227,   -24,  -271, -1000,  -244,  -244,    34,    -6,   -24,
   -24, -1000, -1000,  -264, -1000,  -247, -1000, -1000, -1000, -1000,
 -1000, -1000, -1000, -1000,    33,   -11,  -253,  -238, -1000,    32,
 -1000,    -3, -1000,   -78, -1000,    -4, -1000, -1000, -1000, -1000,
  -244,  -249,  -241, -1000,  -244, -1000, -1000, -1000,   -60, -1000,
 -1000,    -5,  -244, -1000, -1000,  -234, -1000,  -244, -1000 };
yytabelem yypgo[]={

     0,    57,    40,    47,    39,    41,    56,    55,    53,    51,
    35,    50,    49,    48 };
yytabelem yyr1[]={

     0,    11,    11,    12,    12,    12,    12,    12,    12,     5,
     5,     6,     6,     6,     6,     7,     7,    10,    10,    10,
    10,    10,     8,     8,     3,    13,     9,     9,     9,     9,
     2,     2,     2,     2,     1,     1,     4,     4,     4 };
yytabelem yyr2[]={

     0,     0,     4,     5,    11,    11,     9,     2,     5,     2,
     5,     7,     5,     7,     3,     5,     7,     7,    13,     9,
     7,     3,     1,     3,     5,     1,     3,     3,     5,     5,
     3,     3,     3,     3,     3,     5,     3,     3,     3 };
yytabelem yychk[]={

 -1000,   -11,   -12,   261,   257,   258,   262,    10,   256,    10,
   269,   269,    -3,   -13,    10,   260,   259,    -5,    -6,    42,
    40,   269,    -9,    -1,    -2,   271,    -4,   275,   276,   277,
   278,   272,   273,   274,   -10,   263,   267,   265,    -3,   -10,
    10,    40,    -7,    91,    -5,    -5,    -2,    -4,   269,    10,
   264,    40,    -8,   270,   266,    10,    41,    93,   270,    41,
   -10,   269,   268,   -10,    93,    41,   -10,   264,   -10 };
yytabelem yydef[]={

     1,    -2,     2,     0,     0,     0,    25,     7,     0,     3,
     0,     0,     0,     0,     8,    25,    25,     0,     9,     0,
     0,    14,    24,    26,    27,     0,    34,    30,    31,    32,
    33,    36,    37,    38,     0,     0,    22,     0,    21,     0,
     6,     0,    12,     0,    10,     0,    28,    35,    29,     4,
    25,     0,     0,    23,    25,     5,    11,    15,     0,    13,
    17,     0,    25,    20,    16,     0,    19,    25,    18 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"DECLARE",	257,
	"CAST",	258,
	"INTO",	259,
	"AS",	260,
	"HELP",	261,
	"EXPLAIN",	262,
	"FUNCTION",	263,
	"RETURNING",	264,
	"POINTER",	265,
	"TO",	266,
	"ARRAY",	267,
	"OF",	268,
	"NAME",	269,
	"NUMBER",	270,
	"STRUCTUNION",	271,
	"UNSIGNED",	272,
	"LONG",	273,
	"SHORT",	274,
	"INT",	275,
	"CHAR",	276,
	"FLOAT",	277,
	"DOUBLE",	278,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"prog : /* empty */",
	"prog : prog stat",
	"stat : HELP '\n'",
	"stat : DECLARE NAME AS adecl '\n'",
	"stat : CAST NAME INTO adecl '\n'",
	"stat : EXPLAIN type cdecl '\n'",
	"stat : '\n'",
	"stat : error '\n'",
	"cdecl : cdecl1",
	"cdecl : '*' cdecl",
	"cdecl1 : cdecl1 '(' ')'",
	"cdecl1 : cdecl1 cdims",
	"cdecl1 : '(' cdecl ')'",
	"cdecl1 : NAME",
	"cdims : '[' ']'",
	"cdims : '[' NUMBER ']'",
	"adecl : FUNCTION RETURNING adecl",
	"adecl : FUNCTION '(' NAME ')' RETURNING adecl",
	"adecl : ARRAY adims OF adecl",
	"adecl : POINTER TO adecl",
	"adecl : type",
	"adims : /* empty */",
	"adims : NUMBER",
	"type : tinit c_type",
	"tinit : /* empty */",
	"c_type : mod_list",
	"c_type : tname",
	"c_type : mod_list tname",
	"c_type : STRUCTUNION NAME",
	"tname : INT",
	"tname : CHAR",
	"tname : FLOAT",
	"tname : DOUBLE",
	"mod_list : modifier",
	"mod_list : mod_list modifier",
	"modifier : UNSIGNED",
	"modifier : LONG",
	"modifier : SHORT",
};
#endif /* YYDEBUG */
/*	@(#)yaccpar	1.9	*/

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 3:
# line 45 "cdgram.y"
{
			help();
			} break;
case 4:
# line 49 "cdgram.y"
{
			printf("%s %s%s%s",savedtype,yypvt[-1].halves.left,yypvt[-3].dynstr,yypvt[-1].halves.right);
#ifdef MKPROGRAM
			if (prev == 'f')
				printf("\n{\n}\n");
			else
				printf(";\n");
#else
			printf("\n");
#endif
			free(yypvt[-1].halves.left);
			free(yypvt[-1].halves.right);
			free(yypvt[-3].dynstr);
			} break;
case 5:
# line 64 "cdgram.y"
{
			if (prev == 'f')
				unsupp("Cast into function");
			else if (prev=='A' || prev=='a')
				unsupp("Cast into array");
			printf("(%s",savedtype);
			if (strlen(yypvt[-1].halves.left)+strlen(yypvt[-1].halves.right))
				printf(" %s%s",yypvt[-1].halves.left,yypvt[-1].halves.right);
			printf(")%s\n",yypvt[-3].dynstr);
			free(yypvt[-1].halves.left);
			free(yypvt[-1].halves.right);
			free(yypvt[-3].dynstr);
			} break;
case 6:
# line 78 "cdgram.y"
{ printf("declare %s as %s%s\n",savedname,yypvt[-1].dynstr,yypvt[-2].dynstr); } break;
case 8:
# line 81 "cdgram.y"
{
			yyerrok;
			} break;
case 10:
# line 88 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-0].dynstr,ds("pointer to "),NULL); } break;
case 11:
# line 92 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-2].dynstr,ds("function returning "),NULL); } break;
case 12:
# line 94 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-1].dynstr,ds("array "),yypvt[-0].dynstr); } break;
case 13:
# line 96 "cdgram.y"
{ yyval.dynstr = yypvt[-1].dynstr; } break;
case 14:
# line 98 "cdgram.y"
{
				savename(yypvt[-0].dynstr);
				yyval.dynstr = ds("");
			} break;
case 15:
# line 105 "cdgram.y"
{ yyval.dynstr = ds("of "); } break;
case 16:
# line 107 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-1].dynstr,ds(" of "),NULL); } break;
case 17:
# line 111 "cdgram.y"
{
			if (prev == 'f')
				unsupp("Function returning function");
			else if (prev=='A' || prev=='a')
				unsupp("Function returning array");
			yyval.halves.left = yypvt[-0].halves.left;
			yyval.halves.right = cat(ds("()"),yypvt[-0].halves.right,NULL);
			prev = 'f';
			} break;
case 18:
# line 121 "cdgram.y"
{
			if (prev == 'f')
				unsupp("Function returning function");
			else if (prev=='A' || prev=='a')
				unsupp("Function returning array");
			yyval.halves.left = yypvt[-0].halves.left;
			yyval.halves.right = cat(ds("("),yypvt[-3].dynstr,ds(")"));
			yyval.halves.right = cat(yyval.halves.right,yypvt[-0].halves.right,NULL);
			prev = 'f';
			} break;
case 19:
# line 132 "cdgram.y"
{
			if (prev == 'f')
				unsupp("Array of function");
			else if (prev == 'a')
				unsupp("Inner array of unspecified size");
			if (arbdims)
				prev = 'a';
			else
				prev = 'A';
			yyval.halves.left = yypvt[-0].halves.left;
			yyval.halves.right = cat(yypvt[-2].dynstr,yypvt[-0].halves.right,NULL);
			} break;
case 20:
# line 145 "cdgram.y"
{
			if (prev == 'a')
				unsupp("Pointer to array of unspecified dimension");
			if (prev=='a' || prev=='A' || prev=='f') {
				yyval.halves.left = cat(yypvt[-0].halves.left,ds("(*"),NULL);
				yyval.halves.right = cat(ds(")"),yypvt[-0].halves.right,NULL);
			} else {
				yyval.halves.left = cat(yypvt[-0].halves.left,ds("*"),NULL);
				yyval.halves.right = yypvt[-0].halves.right;
			}
			prev = 'p';
			} break;
case 21:
# line 158 "cdgram.y"
{
			savetype(yypvt[-0].dynstr);
			yyval.halves.left = ds("");
			yyval.halves.right = ds("");
			prev = 't';
			} break;
case 22:
# line 167 "cdgram.y"
{
			arbdims = 1;
			yyval.dynstr = ds("[]");
			} break;
case 23:
# line 172 "cdgram.y"
{
			arbdims = 0;
			yyval.dynstr = cat(ds("["),yypvt[-0].dynstr,ds("]"));
			} break;
case 24:
# line 179 "cdgram.y"
{ mbcheck(); yyval.dynstr = yypvt[-0].dynstr; } break;
case 25:
# line 183 "cdgram.y"
{ modbits = 0; } break;
case 26:
# line 187 "cdgram.y"
{ yyval.dynstr = yypvt[-0].dynstr; } break;
case 27:
# line 189 "cdgram.y"
{ yyval.dynstr = yypvt[-0].dynstr; } break;
case 28:
# line 191 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-1].dynstr,ds(" "),yypvt[-0].dynstr); } break;
case 29:
# line 193 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-1].dynstr,ds(" "),yypvt[-0].dynstr); } break;
case 30:
# line 197 "cdgram.y"
{ modbits |= MB_INT; yyval.dynstr = yypvt[-0].dynstr; } break;
case 31:
# line 199 "cdgram.y"
{ modbits |= MB_CHAR; yyval.dynstr = yypvt[-0].dynstr; } break;
case 32:
# line 201 "cdgram.y"
{ modbits |= MB_FLOAT; yyval.dynstr = yypvt[-0].dynstr; } break;
case 33:
# line 203 "cdgram.y"
{ modbits |= MB_DOUBLE; yyval.dynstr = yypvt[-0].dynstr; } break;
case 34:
# line 207 "cdgram.y"
{ yyval.dynstr = yypvt[-0].dynstr; } break;
case 35:
# line 209 "cdgram.y"
{ yyval.dynstr = cat(yypvt[-1].dynstr,ds(" "),yypvt[-0].dynstr); } break;
case 36:
# line 213 "cdgram.y"
{ modbits |= MB_UNSIGNED; yyval.dynstr = yypvt[-0].dynstr; } break;
case 37:
# line 215 "cdgram.y"
{ modbits |= MB_LONG; yyval.dynstr = yypvt[-0].dynstr; } break;
case 38:
# line 217 "cdgram.y"
{ modbits |= MB_SHORT; yyval.dynstr = yypvt[-0].dynstr; } break;
	}
	goto yystack;		/* reset registers in driver code */
}
