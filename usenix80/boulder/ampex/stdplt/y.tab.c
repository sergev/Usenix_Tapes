
# line 2 "graph.y"
# include "graph.h"
# include "stdio.h"
struct axtype *_axptr;
struct _data *_datptr;
int starflg;
int yyline;
int xyflg;
int oxyflg;
int i,j;
char *charp,*chrp;
char *sp;
int frag;
double atof();
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 335 "graph.y"



yylex()
  {
	extern int yylval;
	starflg = 0;
	while((*scanp != '%') && (*scanp != '\0'))
	  {
		ychar++;
		scanp++;
	  }
	if(*scanp++ == '\0')
		return(0);
	ychar++;
	*(scanp - 1) = 0;	/* To make previous string null terminated */
	if(*scanp == '*')
	  {
		(starflg)++;
		scanp++;
		ychar++;
	  }
	if(starflg)
		yylval = scanp + 1; /* pointer to string after token */
	else
		yylval = argp;
	ychar++;
	return(*scanp++);
  }
yyerror(s)
char *s;
  {
	char *str;

	ychar -= 5;
	str = strbeg;
	fprintf(stderr,"\n%s\n:  ",s);
	fprintf(stderr,"error near underbar\n");
	while(str - strbeg < ychar)
		putc(*str++,stderr);
	fprintf(stderr,"_%s\n",scanp);
  }
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 68
# define YYLAST 187
short yyact[]={

  48,  29,  55,   3,  52,  61,  73,  56,  33,  69,
  40,  58,   9,   5,  11,  13,   6,  44,  39,   4,
  55,  23,  52,  61,  28,  56,  26,  32,  31,  58,
   7,  42,  46,  45,  49,  47,  51,  63,  54,  50,
  11,  13,  37,  57,  60,  59,  12,  14,  41,  43,
  38,  53,  49,  36,  51,  62,  54,  50,  16,  35,
  19,  57,  60,  59,  48,  30,  55,  24,  52,  53,
  75,  56,  12,  14,  25,  58,  21,  64,  71,  27,
  18,  15,  67,  72,   8,  55,   2,  52,  66,  70,
  56,   1,   0,  40,  58,   0,  46,  45,  49,  47,
  51,  39,  54,  50,   0,   0,   0,  57,  60,  59,
  34,  55,   0,  52,  61,  53,  56,  49,   0,  51,
  58,  54,  50,   0,  71,  37,  57,  60,  59,  72,
   0,   0,   0,  38,  53,  70,   0,   0,   0,  65,
  74,   0,   0,  49,   0,  51,   0,  54,  50,  10,
   0,   0,  57,  60,  59,  17,  20,  22,   0,   0,
  53,   0,   0,   0,   0,  17,   0,   0,  20,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  68,
   0,   0,   0,   0,   0,   0,  74 };
short yypact[]={

-1000,-1000, -33, -52, -74, -74, -74, -74, -74,-1000,
-1000,-1000,-1000,-1000,-1000, -74,-1000,-1000, -74,-1000,
-1000,-1000,-1000,-1000, -66,-1000,  17,-1000,  -2, -48,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,-1000,  43 };
short yypgo[]={

   0,  91,  86,  84,  81,  80,  76,  12, 149,  67,
  65,  28,  27,   8,  58,  26, 110,  59,  53,  48,
   6,  60,  24,   1,   9 };
short yyr1[]={

   0,   1,   1,   1,   1,   2,   3,   3,   7,   9,
   9,  10,  10,  10,   8,   8,   8,   8,   4,   4,
  14,  15,  15,  11,  11,  11,  11,  11,  11,  11,
  11,  12,  12,  12,   5,   5,  21,  22,  22,  22,
  22,  22,  22,  13,  13,  13,  13,  19,  16,  16,
  16,  16,  16,  16,  16,  16,  17,  17,  20,  18,
   6,  23,  23,  24,  24,  24,  24,  24 };
short yyr2[]={

   0,   4,   4,   4,   4,   0,   2,   1,   2,   2,
   0,   1,   1,   1,   1,   1,   1,   1,   2,   1,
   2,   2,   0,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   2,   1,   2,   2,   2,   2,
   2,   2,   0,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   4,   2,   0,   1,   1,   1,   1,   1 };
short yychk[]={

-1000,  -1,  -2,  36,  71,  65,  68,  82,  -3,  -7,
  -8,  88, 120,  89, 121,  -4, -14,  -8,  -5, -21,
  -8,  -6,  -8,  -7,  -9, -14, -15, -21, -22, -23,
 -10, -11, -12, -13, -16, -17, -18, 108, 116,  84,
  76, -19, -20, 115,  83,  99,  98, 101,  66, 100,
 105, 102,  70, 117, 104,  68,  73, 109,  77, 111,
 110,  71, -11, -13, -19, -16, -17, -18,  -8, -24,
 -17, -19, -18, -20, -16, -23 };
short yydef[]={

   5,  -2,   0,   0,   0,   0,   0,   0,   1,   7,
  10,  14,  15,  16,  17,   2,  19,  22,   3,  35,
  42,   4,  62,   6,   8,  18,  20,  34,  36,   0,
   9,  11,  12,  13,  23,  24,  25,  26,  27,  28,
  29,  30,  31,  32,  33,  43,  44,  45,  46,  48,
  49,  50,  51,  52,  53,  54,  55,  56,  57,  59,
  47,  58,  21,  37,  38,  39,  40,  41,  62,  61,
  63,  64,  65,  66,  67,  60 };
#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

int yydebug = 0; /* 1 for debugging */
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

	short yys[YYMAXDEPTH];
	short yyj, yym;
	register YYSTYPE *yypvt;
	register short yystate, *yyps, yyn;
	register YYSTYPE *yypv;
	register short *yyxi;

	yystate = 0;
	yychar = -1;
	yynerrs = 0;
	yyerrflag = 0;
	yyps= &yys[-1];
	yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
		if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
		*yyps = yystate;
		++yypv;
		*yypv = yyval;

 yynewstate:

	yyn = yypact[yystate];

	if( yyn<= YYFLAG ) goto yydefault; /* simple state */

	if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
	if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

	if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
		yychar = -1;
		yyval = yylval;
		yystate = yyn;
		if( yyerrflag > 0 ) --yyerrflag;
		goto yystack;
		}

 yydefault:
	/* default state action */

	if( (yyn=yydef[yystate]) == -2 ) {
		if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
		/* look through exception table */

		for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

		while( *(yyxi+=2) >= 0 ){
			if( *yyxi == yychar ) break;
			}
		if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
		}

	if( yyn == 0 ){ /* error */
		/* error ... attempt to resume parsing */

		switch( yyerrflag ){

		case 0:   /* brand new error */

			yyerror( "syntax error" );
		yyerrlab:
			++yynerrs;

		case 1:
		case 2: /* incompletely recovered error ... try again */

			yyerrflag = 3;

			/* find a state where "error" is a legal shift action */

			while ( yyps >= yys ) {
			   yyn = yypact[*yyps] + YYERRCODE;
			   if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
			      yystate = yyact[yyn];  /* simulate a shift of "error" */
			      goto yystack;
			      }
			   yyn = yypact[*yyps];

			   /* the current yyps has no shift onn "error", pop stack */

			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

			if( yydebug ) printf( "error recovery discards char %d\n", yychar );

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

		if( yydebug ) printf("reduce %d\n",yyn);
		yyps -= yyr2[yyn];
		yypvt = yypv;
		yypv -= yyr2[yyn];
		yyval = yypv[1];
		yym=yyn;
			/* consult goto table to find next state */
		yyn = yyr1[yyn];
		yyj = yypgo[yyn] + *yyps + 1;
		if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
		switch(yym){
			
case 5:
# line 22 "graph.y"
{ i = j = 0;} break;
case 14:
# line 35 "graph.y"
 {	
		nary[X]++;
		xyflg = X;
		_datptr = _xdata[i++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		_datptr->flgs =| MINMAX;
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} break;
case 15:
# line 48 "graph.y"
 {	
		nary[X]++;
		xyflg = X;
		_datptr = _xdata[i++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} break;
case 16:
# line 60 "graph.y"
 {
		nary[Y]++;
		xyflg = Y;
		_datptr = _ydata[j++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		_datptr->flgs =| MINMAX;
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} break;
case 17:
# line 73 "graph.y"
 {
		nary[Y]++;
		xyflg = Y;
		_datptr = _ydata[j++] = calloc(1,sizeof(struct _data));
		if(_datptr == 0)
			_feror("No space available: calloc");
		if(starflg)
			_datptr->flgs =| NODATA;
		else
			_datptr->dptr = (*argp++).dp;
		_axptr = &_ax[xyflg];
		} break;
case 21:
# line 91 "graph.y"
{
		if(first)
		  {
			if(xyflg != oxyflg)
			  {
				yyerror("Saxis: Non matching xy discreptor");
				exit();
			  }
		  }
		first = 1;
		oxyflg = xyflg;
		} break;
case 22:
# line 103 "graph.y"
 {
		if(first)
		  {
			if(xyflg != oxyflg)
			  {
				yyerror("Saxis: Non matching xy discreptor");
				exit();
			  }
		  }
		first = 1;
		oxyflg = xyflg;
	  } break;
case 26:
# line 120 "graph.y"
{	chrp = yypvt[-0];
		if(starflg)
		  {
			if(xyflg == X)
				charp = _axptr->lab = xlabbuf;
			else
				charp = _axptr->lab = ylabbuf;
			while((*chrp == ' ') || (*chrp == '\t'))
				chrp++;
			while((*chrp != '%') && (*chrp != '\0'))
				*charp++ = *chrp++;
			*charp = '\0';
		  }
		else
			_axptr->lab = (*argp++).cp;} break;
case 27:
# line 135 "graph.y"
{	
		if(starflg)
			frag = atoi(yypvt[-0]);
		else
			frag = (*argp++).ip;
		if((frag >= 0) && (frag <= 3))
			_axptr->flags |= frag;
		else
			printf("Bad Tic (%t) type %d\n",frag); } break;
case 28:
# line 144 "graph.y"
{	
		if(starflg)
			frag = atoi(yypvt[-0]);
		else
			frag = (*argp++).ip;
		if((frag >= 0) && (frag <= 3))
			_axptr->flags |= frag << 2;
		else
			printf("Bad Lab (%%L) type %o\n",frag); } break;
case 29:
# line 153 "graph.y"
{	chrp = yypvt[-0];
		_axptr->flags =| NAME;
		if(starflg)
		  {
			if(xyflg == X)
				charp = _axptr->lab = xlabbuf;
			else
				charp = _axptr->lab = ylabbuf;
			while((*chrp == ' ') || (*chrp == '\t'))
				chrp++;
			while((*chrp != '%') && (*chrp != '\0'))
				*charp++ = *chrp++;
			*charp = '\0';
		  }
		else
			_axptr->lab = (*argp++).cp;} break;
case 32:
# line 172 "graph.y"
 {
		if(starflg)
		  {
			if(xyflg == X)
				_axptr->y = atof(yypvt[-0]);	
			else
				_axptr->x = atof(yypvt[-0]);
		  }
		else
		  {
			if(xyflg == X)
				_axptr->y = argp->dv;
			else
				_axptr->x = argp->dv;
			argp += 4;
		  }
		_axptr->flags =| SHIFT;
	  } break;
case 33:
# line 190 "graph.y"
 {
		if(starflg)
		  {
			if(xyflg == X)
				_axptr->y = atof(yypvt[-0]);	
			else
				_axptr->x = atof(yypvt[-0]);
		  }
		else
		  {
			if(xyflg == X)
				_axptr->y = argp->dv;
			else
				_axptr->x = argp->dv;
			argp += 4;
		  }
		_axptr->flags =| SHIFT1;
	  } break;
case 43:
# line 225 "graph.y"
 {
		if(starflg)
		  {
			for(sp = yypvt[-0];*sp != '%';sp++)
				if(*sp == ' ')
					continue;
				else
					break;
			_datptr->symble = *sp;
		  }
		else
			_datptr->symble = (*argp++).cp;
		} break;
case 44:
# line 238 "graph.y"
 {
		if(starflg)
			_datptr->blubber = atoi(yypvt[-0]);
		else
			_datptr->blubber = (*argp++).ip;
		} break;
case 45:
# line 244 "graph.y"
{	chrp = yypvt[-0];
		if(starflg)
		  {
			charp = _datptr->labl = calloc(1,30);
			if(charp == 0)
				_feror("No space available: calloc");
			while((*chrp == ' ') || (*chrp == '\t'))
				chrp++;
			while((*chrp != '%') && (*chrp != '\0'))
				*charp++ = *chrp++;
			*charp = '\0';
		  }
		else
			_datptr->labl = (*argp++).cp;} break;
case 46:
# line 258 "graph.y"
 {
		if(starflg)
		  {
		_datptr->ddash = calloc(1,16);
		if(_datptr->ddash == 0)
			_feror("No space available: calloc");
		if(sscanf(yypvt[-0],"%f%f%f%f",_datptr->ddash,_datptr->ddash+1,
		  _datptr->ddash + 2,_datptr->ddash + 3) != 4)
		  {
		   fprintf(stderr,"Not enough arguments for 'B' command\n");	
		   fprintf(stderr,"%f %f %f %f\n",*_datptr->ddash
			,*(_datptr->ddash+1),
			*(_datptr->ddash + 2),*(_datptr->ddash + 3));
			_datptr->ddash = 0;
		  }
		  }
		else
			_datptr->ddash = (*argp++).fp;
		} break;
case 47:
# line 277 "graph.y"
{
		if(starflg)
			_datptr->npts = atoi(yypvt[-0]);
		else
			_datptr->npts = (*argp++).ip; } break;
case 48:
# line 284 "graph.y"
{_datptr->flgs =| INTEGER;} break;
case 49:
# line 285 "graph.y"
{_datptr->flgs =| INTEGER;} break;
case 50:
# line 286 "graph.y"
{_datptr->flgs =| FLOAT;} break;
case 51:
# line 287 "graph.y"
{_datptr->flgs =| DOUBLE;} break;
case 52:
# line 288 "graph.y"
{_datptr->flgs =| UNSIGNED;} break;
case 53:
# line 289 "graph.y"
{_datptr->flgs =| SHORT;} break;
case 54:
# line 290 "graph.y"
{_datptr->flgs =| LONG;} break;
case 55:
# line 291 "graph.y"
{_datptr->flgs =| LONG;} break;
case 56:
# line 293 "graph.y"
{
		_axptr->flags =| MINSET;
		if(starflg)
			min[xyflg] = atof(yypvt[-0]);
		else
		  {
			min[xyflg] = argp->dv;
			argp += 4;
		  }
	     } break;
case 57:
# line 304 "graph.y"
{	_axptr->flags =| MAXSET;
		if(starflg)
			max[xyflg] = atof(yypvt[-0]);
		else
		  {
			max[xyflg] = argp->dv;
			argp += 4;
		  }
	   } break;
case 58:
# line 313 "graph.y"
{
		if(starflg)
			frag = atoi(yypvt[-0]);
		else
			frag = (*argp++).ip;
		if((frag >= 0) && (frag <= 3))
			_axptr->flags |= frag << 5;
		else
			printf("Bad Gridtype (%%G) type %o\n",frag);
	   } break;
case 59:
# line 323 "graph.y"
{
		_axptr->flags =| LOGAXIS;
	    } break;
		}
		goto yystack;  /* stack new state and value */

	}
