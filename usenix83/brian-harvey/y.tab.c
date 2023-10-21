# define UNARY 257
# define TWOOP 258
# define ONEOP 259
# define NOOP 260
# define ONECOM 261
# define CSTRING 262
# define UINT 263
# define LTO 264
# define IFCOM 265
# define LEDIT 266
# define LIFTF 267
# define LPROC 268
# define LPEND 269
# define LAEND 270
# define LGO 271
# define CLIST 272
# define TWOCOM 273
# define NOCOM 274
# define RUNCOM 275
# define RNEND 276
# define REPCOM 277
# define THREECOM 278

# line 12 "logo.y"
 
#include "logo.h"

int multnum;
struct object *multarg = 0;
#include <setjmp.h>
extern jmp_buf runret;
jmp_buf yerrbuf;
int catching = 0;
int flagquit = 0;
extern struct runblock *thisrun;
extern int turtdes;
extern struct display *mydpy;
int errtold = 0;
int yyline =0;
char ibuf[IBUFSIZ] ={0};
char *ibufptr =NULL;
char *getbpt =0;
char titlebuf[100] ={0};
char *titleptr =NULL;
extern char *cpystr();
int letflag =0;
int topf =0;
int pflag =0;
char charib =0;
int endflag =0, rendflag = 0;
int traceflag =0;
int currtest = 0;
int argno =(-1);
int *stkbase =NULL;
int stkbi =0;
struct stkframe *fbr =NULL;
struct plist *proclist =NULL;
int pauselev = 0;
extern int psigflag,errpause;

struct object *add(), *sub(), *mult(), *div(), *rem(), *and(), *or();
struct object *greatp(), *lessp(), *lmax(), *lmin(), *lis();
struct object *worcat(), *sencat(), *equal(), *lemp(), *comp();
struct object *lnump(), *lsentp(), *lwordp(), *length(), *zerop();
struct object *first(), *butfir(), *last(), *butlas(), *alllk();
struct object *lnamep(), *lrandd(), *rnd(), *sq(), *lpow(), *lsin();
struct object *lcos(), *latan(), *ltime(), *request(), *readlist();
struct object *cmprint(), *cmtype(), *cmoutput(), *lsleep();
struct object *cmlocal(), *assign(), *cmedit(), *lstop(), *show(), *erase();
struct object *help(), *describe(), *ltrace(), *luntrace();
struct object *lbyecom(), *getturtle(), *forward(), *back();
struct object *left(), *right(), *penup(), *cmpendown(), *clearscreen();
struct object *fullscreen(), *splitscreen(), *showturtle();
struct object *hideturtle(), *textscreen(), *cmpenerase(), *pencolor();
struct object *wipeclean(), *penmode(), *penreverse(), *shownp(), *towardsxy();
struct object *setcolor(), *setxy(), *setheading(), *ltopl();
struct object *cmfprint(), *cmftype(), *pots(), *fput(), *lput();
struct object *list(), *loread(), *lowrite(), *fileclose();
struct object *lfread(), *lfword(), *fileprint(), *filefprint();
struct object *filetype(), *fileftype(), *callunix(), *repcount();
#ifdef DEBUG
struct object *setdebquit(), *setmemtrace(), *setyaccdebug();
#endif
struct object *xcor(), *ycor(), *heading(), *getpen(), *cbreak();
struct object *readchar(), *keyp(), *intpart(), *round(), *toascii();
struct object *tochar(), *loflush(), *settest(), *memberp(), *item();
struct object *scrunch(), *setscrunch();
struct object *unpause(), *dopause(), *setipause(), *setqpause(); /* PAUSE */
struct object *seterrpause(), *clrerrpause();
#ifdef FLOOR
struct object *hitoot(), *lotoot(), *lampon(), *lampoff();
struct object *ftouch(), *btouch(), *ltouch(), *rtouch();
#endif
#ifndef SMALL
struct object *gprop(), *plist(), *pps(), *remprop();
#endif
struct lexstruct keywords[] =
{
	"sum",TWOOP,add,NULL,
	"difference",TWOOP,sub,"diff",
	"product",TWOOP,mult,NULL,
	"quotient",TWOOP,div,NULL,
	"remainder",TWOOP,rem,"mod",
	"both",TWOOP,and,"and",
	"either",TWOOP,or,"or",
	"greaterp",TWOOP,greatp,NULL,
	"lessp",TWOOP,lessp,NULL,
	"maximum",TWOOP,lmax,"max",
	"minimum",TWOOP,lmin,"min",
	"is",TWOOP,lis,NULL,
	"word",TWOOP,worcat,NULL,
	"sentence",TWOOP,sencat,"se",
	"equalp",TWOOP,equal,NULL,
	"emptyp",ONEOP,lemp,NULL,
	"not",ONEOP,comp,NULL,
	"numberp",ONEOP,lnump,NULL,
	"sentencep",ONEOP,lsentp,NULL,
	"wordp",ONEOP,lwordp,NULL,
	"count",ONEOP,length,NULL,
	"zerop",ONEOP,zerop,NULL,
	"first",ONEOP,first,"f",
	"butfirst",ONEOP,butfir,"bf",
	"last",ONEOP,last,"l",
	"butlast",ONEOP,butlas,"bl",
	"thing",ONEOP,alllk,NULL,
	"namep",ONEOP,lnamep,NULL,
	"random",NOOP,lrandd,NULL,
	"rnd",ONEOP,rnd,NULL,
	"sqrt",ONEOP,sq,NULL,
	"pow",TWOOP,lpow,NULL,
	"sin",ONEOP,lsin,NULL,
	"cos",ONEOP,lcos,NULL,
	"arctan",ONEOP,latan,"atan",
	"time",NOOP,ltime,NULL,
	"request",NOOP,request,NULL,
	"readlist",NOOP,readlist,"rl",
	"print",ONECOM,cmprint,"pr",
	"type",ONECOM,cmtype,NULL,
	"output",ONECOM,cmoutput,"op",
	"wait",ONECOM,lsleep,NULL,
	"local",ONECOM,cmlocal,NULL,
	"make",TWOCOM,assign,NULL,
	"if",IFCOM,0,NULL,
	"to",LTO,0,NULL,
	"end",LPEND,0,NULL,
	"stop",NOCOM,lstop,NULL,
	"edit",LEDIT,cmedit,"ed",
	"go",LGO,0,NULL,
	"show",ONECOM,show,"po",
	"erase",ONECOM,erase,"er",
	"help",NOCOM,help,NULL,
	"describe",ONECOM,describe,NULL,
	"trace",NOCOM,ltrace,NULL,
	"untrace",NOCOM,luntrace,NULL,
	"goodbye",NOCOM,lbyecom,"bye",
	"turtle",ONECOM,getturtle,"tur",
	"forward",ONECOM,forward,"fd",
	"back",ONECOM,back,"bk",
	"left",ONECOM,left,"lt",
	"right",ONECOM,right,"rt",
#ifdef FLOOR
	"hitoot",ONECOM,hitoot,"hit",
	"lotoot",ONECOM,lotoot,"lot",
	"lampon",NOCOM,lampon,"lon",
	"lampoff",NOCOM,lampoff,"loff",
#endif
	"penup",NOCOM,penup,"pu",
	"pendown",NOCOM,cmpendown,"pd",
	"clearscreen",NOCOM,clearscreen,"cs",
	"fullscreen",NOCOM,fullscreen,"full",
	"splitscreen",NOCOM,splitscreen,"split",
	"showturtle",NOCOM,showturtle,"st",
	"hideturtle",NOCOM,hideturtle,"ht",
	"textscreen",NOCOM,textscreen,"text",
	"penerase",NOCOM,cmpenerase,"pe",
	"pencolor",ONECOM,pencolor,"penc",
	"setcolor",TWOCOM,setcolor,"setc",
	"setxy",TWOCOM,setxy,NULL,
	"setheading",ONECOM,setheading,"seth",
	"wipeclean",NOCOM,wipeclean,"clean",
	"penmode",NOOP,penmode,NULL,
	"penreverse",NOCOM,penreverse,"px",
	"shownp",NOOP,shownp,NULL,
	"towardsxy",TWOOP,towardsxy,NULL,
#ifdef FLOOR
	"ftouch",NOOP,ftouch,"fto",
	"btouch",NOOP,btouch,"bto",
	"ltouch",NOOP,ltouch,"lto",
	"rtouch",NOOP,rtouch,"rto",
#endif
	"toplevel",NOCOM,ltopl,"top",
	"fprint",ONECOM,cmfprint,"fp",
	"ftype",ONECOM,cmftype,"fty",
	"pots",NOCOM,pots,NULL,
	"fput",TWOOP,fput,NULL,
	"lput",TWOOP,lput,NULL,
	"list",TWOOP,list,NULL,
	"openread",ONEOP,loread,"openr",
	"openwrite",ONEOP,lowrite,"openw",
	"close",ONECOM,fileclose,NULL,
	"fileread",ONEOP,lfread,"fird",
	"fileword",ONEOP,lfword,"fiwd",
	"fileprint",TWOCOM,fileprint,"fip",
	"filefprint",TWOCOM,filefprint,"fifp",
	"filetype",TWOCOM,filetype,"fity",
	"fileftype",TWOCOM,fileftype,"fifty",
	"unix",ONECOM,callunix,NULL,
	"run",RUNCOM,0,NULL,
	"repeat",REPCOM,0,NULL,
	"repcount",NOOP,repcount,NULL,
#ifdef DEBUG
	"debquit",NOCOM,setdebquit,NULL,
	"memtrace",NOCOM,setmemtrace,NULL,
	"yaccdebug",NOCOM,setyaccdebug,NULL,
#endif
	"xcor",NOOP,xcor,NULL,
	"ycor",NOOP,ycor,NULL,
	"heading",NOOP,heading,NULL,
	"getpen",NOOP,getpen,NULL,
	"cbreak",ONECOM,cbreak,NULL,
	"readchar",NOOP,readchar,"rc",
	"keyp",NOOP,keyp,NULL,
	"int",ONEOP,intpart,NULL,
	"round",ONEOP,round,NULL,
	"ascii",ONEOP,toascii,NULL,
	"char",ONEOP,tochar,NULL,
	"oflush",NOCOM,loflush,NULL,
#ifndef SMALL
	"gprop",TWOOP,gprop,NULL,
	"plist",ONEOP,plist,NULL,
	"pprop",THREECOM,0,NULL,
	"pps",NOCOM,pps,NULL,
	"remprop",TWOCOM,remprop,NULL,
#endif
	"test",ONECOM,settest,NULL,
	"iftrue",LIFTF,(struct object *(*)())1,"ift",
	"iffalse",LIFTF,0,"iff",
	"memberp",TWOOP,memberp,NULL,
	"item",TWOOP,item,"nth",
	"scrunch",NOOP,scrunch,NULL,
	"setscrunch",ONECOM,setscrunch,"setscrun",
	"continue",NOCOM,unpause,"co",
	"pause",NOCOM,dopause,NULL,
	"setipause",NOCOM,setipause,NULL,
	"setqpause",NOCOM,setqpause,NULL,
	"errpause",NOCOM,seterrpause,NULL,
	"errquit",NOCOM,clrerrpause,NULL,
	NULL,NULL,NULL,NULL,
};

#define uperror {errtold++;YYERROR;}

#define catch(X) {if(!setjmp(yerrbuf)){if(flagquit)errhand();catching++;X;catching=0;}else{catching=0;uperror}}
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

# line 605 "logo.y"


extern struct object *makelist();

yylex1()
{
	register char *str;
	char s[100];
	char c;
	register pc;
	register i;
	NUMBER dubl;
	int floatflag;
	FIXNUM fixn;

	if (yyerrflag) return(1);
	else if (argno==0 && pflag!=1) {
		if (fbr->oldyyc==-2) fbr->oldyyc= -1;
		return(LAEND);
	} else if (endflag==1 && pflag>1) {
		endflag=0;
		return(LPEND);
	}
	else if (pflag==2) {
		pc= *(stkbase+stkbi++);
		if (stkbi==PSTKSIZ-1) {
			stkbase= (int *)(*(stkbase+PSTKSIZ-1));
			stkbi=1;
		}
		yylval= *(stkbase+stkbi++);
		if (pc==LPROC || pc==CSTRING || pc==UINT || pc==CLIST) {
			yylval=(int)localize((struct object *)yylval);
		}
		if (stkbi==PSTKSIZ-1) {
			stkbase= (int *)(*(stkbase+PSTKSIZ-1));
			stkbi=1;
		}
		if (pc== -1) return(0);
		else return(pc);
	} else if (letflag==1) {
		str=s;
		while (!index(" \t\n[](){}\";",(c = getchar()))) {
			if (c == '\\') c = getchar()|0200;
			else if (c == '%') c = ' '|0200;
			*str++ = c;
		}
		charib=c;
		*str='\0';
		yylval=(int)localize(objcpstr(s));
		letflag=0;
		return(CSTRING);
	} else if (letflag==2) {
		str = s;
		while (( (c=getchar())>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || ( c>='0' && c<='9' ) ) {
			if (c>='A' && c<='Z') c += 040;
			*str++ = c;
		}
		charib = c;
		*str = '\0';
		letflag = 0;
		yylval = (int)localize(objcpstr(s));
		return(CSTRING);
	}
	else if (letflag==3) {
		yylval = (int)makelist();
		letflag = 4;
		return(CLIST);
	}
	else if (letflag==4) {
		letflag = 0;
		return(yylval = getchar());
	}
	while ((c=getchar())==' ' || c=='\t')
		;
	if (rendflag) {
		getbpt = 0;
		rendflag = 0;
		return(RNEND);
	}

	if (c == '!')	/* comment feature */
		while ((c=getchar()) && (c != '\n')) ;

	if ((c<'a' || c>'z') && (c<'0' || c>'9') && c!='.') {
		yylval=c;
		if (c=='\"') letflag=1;
		if (c==':') letflag=2;
		if (c=='[') letflag=3;
		return(c);
	}
	else if ((c>='0' && c<='9')|| c=='.') {
		floatflag = (c=='.');
		str=s;
		while ((c>='0' && c<='9')||(c=='E')||(c=='e')||(c=='.')) {
			*str++=c;
			if (c=='.') floatflag++;
			if ((c=='e')||(c=='E')) {
				floatflag++;
				c = getchar();
				if ((c=='+')||(c=='-')) {
					*str++ = c;
					c = getchar();
				}
			} else c=getchar();
		}
		charib=c;
		*str='\0';
		if (floatflag) {
			sscanf(s,EFMT,&dubl);
			yylval=(int)localize(objdub(dubl));
		} else {
			sscanf(s,FIXFMT,&fixn);
			yylval=(int)localize(objint(fixn));
		}
		return(UINT);
	} else {
		yylval=(int)(str=s);
		*str++=c;
		c=getchar();
		while ((c>='a' && c<='z') || (c>='0' && c<='9')) {
			*str++=c;
			c=getchar();
		}
		charib=c;
		*str='\0';
		for (i=0; keywords[i].word; i++) {
			if (!strcmp(yylval,keywords[i].word) ||
			    !strcmp(yylval,keywords[i].abbr)) {
				yylval=i;
				return(keywords[i].lexret);
			}
		}
		yylval=(int)localize(objcpstr(s));
		return(LPROC);
	}
}

yylex() {
	int x;

	if (catching) return(yylex1());
	if (!setjmp(yerrbuf)) {
		if (flagquit) errhand();
		catching++;
		x = yylex1();
		catching=0;
		return(x);
	} else {
		catching=0;
		return(12345);	/* This should cause an error up there */
	}
}

int isuint(x)
int x;
{
	return(x == UINT);
}

int isstored(x)
int x;
{
	return(x==UINT || x==LPROC || x==CSTRING || x==CLIST);
}

yyprompt(clear) {
	register int i;

	if (!ibufptr && !getbpt && !pflag) {
		flagquit = 0;
		if (pauselev > 0) {
			for (i=pauselev; --i >=0; )
				putchar('-');
		}
		putchar('*');
		if ((turtdes<0) && clear)
			(*mydpy->state)('*');
		fflush(stdout);
	}
}
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
-1, 116,
	256, 43,
	60, 43,
	62, 43,
	61, 43,
	43, 43,
	45, 43,
	42, 43,
	47, 43,
	92, 43,
	94, 43,
	10, 43,
	59, 43,
	-2, 94,
-1, 136,
	60, 0,
	62, 0,
	61, 0,
	-2, 61,
-1, 138,
	60, 0,
	62, 0,
	61, 0,
	-2, 63,
-1, 140,
	60, 0,
	62, 0,
	61, 0,
	-2, 65,
-1, 143,
	60, 34,
	62, 34,
	61, 34,
	43, 34,
	45, 34,
	42, 34,
	47, 34,
	92, 34,
	94, 34,
	125, 34,
	41, 34,
	-2, 41,
-1, 160,
	42, 51,
	47, 51,
	92, 51,
	94, 51,
	261, 51,
	264, 51,
	266, 51,
	267, 51,
	270, 51,
	271, 51,
	273, 51,
	274, 51,
	276, 51,
	278, 51,
	-2, 49,
-1, 161,
	261, 52,
	264, 52,
	266, 52,
	267, 52,
	270, 52,
	271, 52,
	273, 52,
	274, 52,
	276, 52,
	278, 52,
	-2, 50,
	};
# define YYNPROD 111
# define YYLAST 1566
short yyact[]={

  17,  75, 110,  96,  55, 122,  76,  74, 101, 166,
  67, 147,  56,  79, 119,  45,  70,  15, 112,  46,
  17,  12, 123,  17,  24, 113,  72,   2, 145,  17,
  30, 115,  68, 169,  21,  28,  17,  17,  44,  29,
  11,  17,  20,  65,  17, 176, 109,  24,  26,  18,
  63,  80,  77,  30,  78,  19,  81,  69,  28,  22,
  80,  14,  73,  13,  73,  81,  54,  17,  24,  18,
  89,  26,  18,   5,  30,   1,   0,   0,  18,  28,
   0,  25,   0,  15,  15,  18,  18,  12,  12,   0,
  18,  24,  26,  18,  17, 146, 121,  30,   0,   0,
 118,  82,  28,  79,  25,   0,  11,  11,   0,   0,
  82,   0,  79,  40,   0,  26,  18,  17,  24, 148,
 165,   0, 144,   0,  30,  25,   0,  17,  15,  28,
   0,   0,  12,   0,   0, 154,  40,   0, 167,   0,
   0,  24,  26,  18,   0,   0,   0,  30,  25,  80,
  77,  11, 106,   0,  81,   0,   0,  40,   0,  80,
  77,   0,  78,   0,  81,  26,  18,  84,  83,  85,
   0,   0,   0, 170,   0,  25,  18,  84,  83,  85,
  40,  15,   0, 173, 177,  12,   0,   0,   0,   0,
   0,   0,   0,  15,   0,   0,   0,  12,  25,  82,
   0,  79,   0, 167,  11,   0,   0,  40,   0,  82,
   0,  79,   0,   0,   0,   0,  11,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  40,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0, 114,   0,  37,  38,
  39,  16,   0, 116,  36,  34,   4,  35,  31, 178,
 120,  10,  71,   6,   9,  32,  62,  33,   7, 114,
   0,  37,  38,  39,  16,  60, 116,  36,  34,   4,
  35,  31,  58,  53,  10,   0,   6,   9,  32, 117,
  33,   7,  37,  38,  39,  16,   0,  23,  36,  34,
   4,  35,  31, 179,   0,  10,   0,   6,   9,  32,
   0,  33,   7,   3,   0,  37,  38,  39,  16,   0,
  23,  36,  34,   4,  35,  31,   0,   0,  10,   0,
   6,   9,  32,   0,  33,   7,   0,   0,   0,   0,
   0,   0,  37,  38,  39,  16,   0,  23,  36,  34,
   4,  35,  31,   0,   0,  10,   0,   6,   9,  32,
   0,  33,   7,   0,   0,  37,  38,  39,  24,   0,
  23,   0,  47, 175,  30,  31,  80,  77,   0, 106,
   0,  81,  32,   0,  33,   0,   0,   0,   0,   0,
   0,  24,  26,   0,  84,  83,  85,  30,   0,  80,
  77,   0, 106,   0,  81,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  26,  24,  84,  83,  85,
   0,   0,  30,   0,   0,  25,  82,  28,  79,   0,
   0,   0,   0,   0,   0,   0,  24,   0,   0,   0,
  26,   0,  30,   0,   0,   0,   0,  28,  25,  82,
   0,  79,   0,   0,   0,   0,   0,  40,  24,   0,
  26,   0,   0,   0,  30,   0, 147,  80,  77,  28,
  78,   0,  81,  25,   0,   0,  80,  77,  24,  78,
  40,  81,  26,   0,  30,  84,  83,  85,   0,  28,
   0,   0,   0,  25,  84,  83,  85,   0,  24,   0,
   0,   0,  26,   0,  30,  40,   0,   0,   0,  28,
   0,   0,   0,   0,   0,  25,   0,  82,  24,  79,
   0,   0,  26,   0,  30,  40,  82,   8,  79,  28,
   0,   0,  41,   0,   0,  25,   0,  52,  24,  57,
  59,   0,  26,  61,  30,   0,   0,  40,   0,  28,
 146,   0,   0,   0,   0,  25,   0,   0,   0,   0,
   0,   0,  26,   0,   0,   0,   0,  40,   0,   0,
 102,   0,   0,   0,  24,  25,   0,   0,   0,   0,
  30,   0,   0,   0,   0,  28,   0,  40,   0,   0,
 153,   0,  37,  38,  39,  25,   0,  23,  26,  47,
   0,   0,  31,   0,   0,   0,   0,  40,   0,  32,
   0,  33,   0,   0,   0,  37,  38,  39,   0,   0,
  23,   0,  47, 151,  24,  31,   0,  40,   0,   0,
  30,  25,  32, 158,  33,  28, 163,   0, 161,   0,
  37,  38,  39,   0,   0,  23,   0,  47,  26,   0,
  31,   0,   0,   0,   0,   0,   0,  32, 155,  33,
  37,  38,  39,  40,   0,  23,   0,  47,   0,   0,
  31,   0,   0,   0,   0,   0,   0,  32, 172,  33,
 143,  25,  37,  38,  39,   0,   0,  23,   0,  47,
 174,   0,  31,   0,   0,   0,   0,   0,   0,  32,
 141,  33,  37,  38,  39,   0,   0,  23,   0,  47,
   0,   0,  31,  40,   0,   0,   0,   0,   0,  32,
 139,  33,  37,  38,  39,   0,   0,  23,   0,  47,
   0,   0,  31,   0,   0,   0,   0,   0,   0,  32,
 137,  33,  37,  38,  39,   0,   0,  23,   0,  47,
   0,   0,  31,   0,   0,   0,   0,   0,   0,  32,
 135,  33,  37,  38,  39,  24,   0,  23,   0,  47,
   0,  30,  31,   0,   0,   0,  28,   0,   0,  32,
   0,  33,   0,   0,   0,   0,   0,   0,  17,  26,
   0,   0,   0,   0,   0,  24, 133,   0,  37,  38,
  39,  30,   0,  23,   0,  47,  28,   0,  31,   0,
   0,   0,   0,   0,   0,  32,   0,  33,  24,  26,
  80,  77,  25,  78,  30,  81,   0,   0,   0,  28,
   0,   0,   0,   0,   0,   0,   0,  18,  84,  83,
  85,   0,  26,   0,   0,  24, 131,   0,  37,  38,
  39,  30,  25,  23,  40,  47,  28,   0,  31,   0,
   0,   0,   0,   0,   0,  32,   0,  33,  24,  26,
  82,   0,  79,   0,  30,  25,   0,   0,   0,  28,
   0,   0,   0,   0,  40,   0,   0,   0,  24,  17,
   0,   0,  26,   0,  30,   0,   0,   0,   0,  28,
   0,   0,  25,   0,   0,   0,   0,  40,  24,   0,
   0,   0,  26,   0,  30,   0,   0,   0,   0,  28,
   0,  80,  77,   0,  78,  25,  81,   0,   0,   0,
   0,   0,  26,   0,  40,   0,  24,   0,  18,  84,
  83,  85,  30,   0,   0,  25,   0,  28,   0,   0,
   0,  17,   0,   0,   0,   0,  24,  40,   0,   0,
  26,   0,  30,   0,   0,  25,   0,  28,   0,   0,
   0,  82,   0,  79,   0,   0,   0,  40,   0,   0,
  26,   0,   0,  80,  77,   0,  78, 129,  81,  37,
  38,  39,   0,  25,  23,   0,  47,  40,  24,  31,
  18,  84,  83,  85,  30,   0,  32,   0,  33,  28,
   0,   0,   0,  25,   0,   0,   0, 127,   0,  37,
  38,  39,  26,   0,  23,  40,  47,   0,   0,  31,
   0,   0,   0,  82, 164,  79,  32,   0,  33,   0,
 125,   0,  37,  38,  39,  40,   0,  23,   0,  47,
   0,  24,  31,   0,   0,  25,   0,  30,   0,  32,
   0,  33,  28,   0,   0,   0,   0,  94,   0,  37,
  38,  39,   0,   0,  23,  26,  47,   0,   0,  31,
   0,   0,   0,   0,   0,   0,  32,  40,  33,   0,
 100,   0,  37,  38,  39,   0,   0,  23,   0,  47,
   0,   0,  31,   0,   0,   0,   0,   0,  25,  32,
  98,  33,  37,  38,  39,   0,   0,  23,   0,  47,
   0,   0,  31,   0,   0,   0,   0,   0,   0,  32,
  87,  33,  37,  38,  39, 159,   0,  23,   0,  47,
  40,   0,  31,   0,  24,   0,   0,   0,   0,  32,
  30,  33,   0,   0,   0,  28,   0,   0,  64,   0,
  37,  38,  39,   0,   0,  23,   0,  47,  26,   0,
  31,   0,   0,   0,   0,   0,   0,  32,  51,  33,
  37,  38,  39,  24,   0,  23,   0,  47,   0,  30,
  31,   0,   0,   0,  28,   0,   0,  32,   0,  33,
   0,  25,   0,   0,   0,   0,   0,  26,   0,   0,
   0,  17,   0,   0,   0,   0,   0,   0,   0,   0,
  49,   0,  37,  38,  39,   0,   0,  23,   0,  47,
   0,   0,  31,  40,   0,   0,   0,   0,   0,  32,
  25,  33,   0,  80,  77,   0,  78,   0,  81,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  18,  84,  83,  85,   0,   0,   0,   0,   0,   0,
   0,   0,  40,  43,   0,  37,  38,  39,   0,   0,
  23,   0,  47,   0,   0,  31,   0,   0,   0,   0,
   0,   0,  32,  82,  33,  79,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  37,  38,
  39,   0,   0,  23,   0,  47,   0,   0,  31,   0,
   0,   0,   0,   0,   0,  32,   0,  33,   0,   0,
   0,   0,   0,   0,   0,   0,  27,   0,   0,   0,
   0,   0,  42,  48,  50,   0,   0,  88,  38,  39,
   0,   0,  23,   0,  47,   0,  66,  31,   0,   0,
   0,   0,   0,   0,  32,  86,  33,  90,   0,  91,
  92,  93,  95,   0,  97,  99,   0,   0,   0,   0,
   0,   0,   0,   0, 104, 105,   0, 107,   0,   0,
   0, 108,   0,   0,   0,   0,   0, 103,   0,   0,
   0,   0, 111,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0, 124, 126, 128, 130, 132, 134,
 136, 138, 140,   0,   0, 142,   0,   0,   0, 149,
 150,   0,   0,   0, 152,   0,   0,   0, 156,   0,
   0, 157,   0, 160, 162,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0, 168,
   0,   0,   0,   0,   0,   0,   0, 171,   0,   0,
   0,   0,   0, 156, 171,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0, 156 };
short yypact[]={

-1000,  57,-1000,-1000,  31,1017, 964, 922,-1000,  27,
-252,  26,  19,-1000,-1000,  10,-1000,-1000,-1000, 902,
-1000,-1000,   6,-1000,-255,-271,-256, 434, 874,-1000,
1149,-1000,1110,1110, 811,1110,-265, 854, 834,-1000,
-250,-1000,1201,-1000,-1000,-1000,-1000, 811, 357,-1000,
 357,-1000,-1000,-1000,1110,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-268,-1000,1110, 434,  13,  13,   4,
-1000,-1000,-1000,-257,-1000, -71,-1000, 784, 761, 731,
 590, 540, 504, 484, 464, 444,-1000,-1000, 424, -30,
 425, 434, 357, 357,-1000, 941,-1000, 334,-1000, 434,
-1000, 402,-1000,-1000, 357, 879, 382, 357, 778,-247,
-1000, 434,  84,-1000,-1000,-1000,-1000,-1000,-1000,-1000,
-1000,-1000,-1000,-1000,  18,-1000,  18,-1000,-1000,-1000,
 -81,-1000, -81,-1000, -81,-1000,   9,-1000,   9,-1000,
   9,-1000, 334,-1000, -30,-1000,-1000,-1000,-1000, 434,
 107,-1000, 434,-1000, -30,-1000, 357, 357,-1000,-1000,
-1000,-1000, 117,-1000,-1000, -10,-1000,-1000, 357,-1000,
-1000, 434,-1000,-1000,-1000,-1000,-1000,  34,-1000,-1000 };
short yypgo[]={

   0,  75,  25, 527,  73,1396,  66,  38,  19,  63,
  61,  15,  39,  33,  28,  59,  57,  26,  55,  50,
  46,  10,  45,  43,  18,  42,  31,  34 };
short yyr1[]={

   0,   1,   1,   1,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   4,   4,
   5,   5,  12,  12,  12,  12,  12,  12,  12,  12,
  12,  12,  12,  11,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  13,  13,  10,  10,  10,  10,  15,  16,  16,  17,
   7,   7,   7,  19,  19,  23,  23,  20,  21,  21,
  21,  21,  22,  22,  24,  18,   8,  25,  25,  25,
  25,  26,   9,  27,  27,   6,   6,  14,  14,   3,
   3 };
short yyr2[]={

   0,   0,   2,   2,   2,   3,   3,   2,   4,   4,
   2,   5,   5,   2,   1,   2,   2,   4,   4,   2,
   2,   2,   2,   2,   1,   1,   2,   2,   1,   1,
   1,   1,   3,   3,   2,   2,   2,   1,   4,   4,
   3,   3,   3,   1,   2,   3,   2,   3,   3,   3,
   3,   2,   2,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   1,   1,   3,
   1,   2,   3,   2,   3,   2,   2,   1,   2,   2,
   5,   4,   2,   0,   1,   1,   2,   1,   0,   3,
   2,   2,   1,   2,   1,   1,   3,   2,   3,   4,
   2,   1,   3,   4,   3,   0,   1,   1,   1,   1,
   1 };
short yychk[]={

-1000,  -1,  -2, 256, 266,  -4, 273, 278,  -3, 274,
 271,  -7,  -8,  -9, -10, -11, 261,  10,  59, -18,
 -25, -27, -15, 263,  34,  91,  58,  -5,  45, -12,
  40, 268, 275, 277, 265, 267, 264, 258, 259, 260,
 123,  -3,  -5, 256,  -7, -11,  -8, 265,  -5, 256,
  -5, 256,  -3, 256,  -6, 256, 264,  -3, 256,  -3,
 256,  -3, 256, -19, 256, -23,  -5, -21, -21, -16,
  10, 256, -17,  58, 262, 272, 262,  43,  45,  94,
  42,  47,  92,  61,  60,  62,  -5, 256, 258, -12,
  -5,  -5,  -5,  -5, 256,  -5, 268,  -5, 256,  -5,
 256, 258,  -3, 256,  -5,  -5,  45,  -5,  -5, -20,
 270,  -5, -24,  -2, 256, -26, 263, 276, -26,  10,
 256, -17, 262,  93,  -5, 256,  -5, 256,  -5, 256,
  -5, 256,  -5, 256,  -5, 256,  -5, 256,  -5, 256,
  -5, 256,  -5, 256, -13, -14, 125,  41, -14,  -5,
  -5,  -3,  -5, 256, -13, 256,  -5,  -5,  -3, 256,
  -5, 256,  -5,  -3, 256, -21, 256,  -2,  -5, -13,
 -14,  -5,  -3, -14,  -3, 256, -22, -24, 269, 269 };
short yydef[]={

   1,  -2,   2,   3,  29,   0,   0,   0,  14,   0,
 105,  30,  68,  24,  25,  31,  28, 109, 110,  83,
  88,  88,   0,  43,   0,   0,   0,   0,   0,  67,
   0,  95,   0,   0,   0,   0,   0,   0,   0,  37,
   0,   4,   0,   7,  30,  31,  68,   0,   0,  10,
   0,  13,  15,  16,   0,  19, 106,  20,  21,  22,
  23,  26,  27,   0,  82,  84,  85,   0,   0,   0,
  73,  75,  77,   0,  44,   0,  46,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  51,  52,   0,  67,
   0,  97,   0,   0, 100,   0,  76,   0,  34,  35,
  36,   0,   5,   6,   0,   0,   0,   0,   0,  88,
  87,  86,   0,  90,  91,  96,  -2, 101, 102,  72,
  74,  78,  79,  45,  47,  48,  49,  50,  53,  54,
  55,  56,  57,  58,  59,  60,  -2,  62,  -2,  64,
  -2,  66,  70,  -2,   0,  42, 107, 108,  69,  98,
   0, 104,  32,  33,   0,  40,  70,   0,   8,   9,
  -2,  -2,   0,  17,  18,   0,  81,  89,  32,  71,
  39,  99, 103,  38,  11,  12,  80,   0,  92,  93 };
#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*	parser for yacc output	*/

#ifdef YYDEBUG
int yydebug = 0; /* 1 for debugging */
#endif
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

#ifdef YYDEBUG
	if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
#endif
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

#ifdef YYDEBUG
			   if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
#endif
			   --yyps;
			   --yypv;
			   }

			/* there is no state on the stack with an error shift ... abort */

	yyabort:
			return(1);


		case 3:  /* no shift yet; clobber input char */

#ifdef YYDEBUG
			if( yydebug ) printf( "error recovery discards char %d\n", yychar );
#endif

			if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
			yychar = -1;
			goto yynewstate;   /* try again in the same state */

			}

		}

	/* reduction by production yyn */

#ifdef YYDEBUG
		if( yydebug ) printf("reduce %d\n",yyn);
#endif
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
			
case 2:
# line 244 "logo.y"
{
		if (psigflag) dopause();
		yyprompt(1);
	} break;
case 3:
# line 248 "logo.y"
{
		if (!errtold) {
			logoyerror();
		}
		errtold = 0;
		errwhere();
		if ((!errpause&&!pauselev) || !fbr) errzap();
		yyerrok;yyclearin;
		yyprompt(0);
	} break;
case 4:
# line 259 "logo.y"
{
		catch(doedit(););
		yyval = -1;
	} break;
case 5:
# line 263 "logo.y"
{
		catch(yyval=(int)(*keywords[yypvt[-2]].lexval)(yypvt[-1]););} break;
case 6:
# line 265 "logo.y"
{cm1er1(yypvt[-2]);uperror;} break;
case 7:
# line 266 "logo.y"
{cm1er2(yypvt[-1]);uperror;} break;
case 8:
# line 267 "logo.y"
{
		catch((*keywords[yypvt[-3]].lexval)(yypvt[-2],yypvt[-1]);); yyval = -1;} break;
case 9:
# line 269 "logo.y"
{cm2er1(yypvt[-3]);uperror;} break;
case 10:
# line 270 "logo.y"
{cm2er2(yypvt[-1]);uperror;} break;
case 11:
# line 271 "logo.y"
{
#ifndef SMALL
		catch(pprop(yypvt[-3],yypvt[-2],yypvt[-1]););
#endif
		yyval = -1;
	} break;
case 12:
# line 277 "logo.y"
{
		if (!errtold) {
			extra();
			puts("Pprop takes three inputs.");
		}
		uperror;
	} break;
case 13:
# line 284 "logo.y"
{
		if (!errtold) {
			missing();
			puts("Pprop takes three inputs.");
		}
		uperror;
	} break;
case 14:
# line 291 "logo.y"
{ yyval= -1; } break;
case 15:
# line 292 "logo.y"
{
		catch((*keywords[yypvt[-1]].lexval)();); yyval= -1;} break;
case 16:
# line 294 "logo.y"
{
		if (!errtold) {
			extra();
			pf1("%s takes no inputs.\n",keywords[yypvt[-1]].word);
		}
		uperror;
	} break;
case 17:
# line 301 "logo.y"
{
		catch(go(yypvt[-1]););
		yyval= -1;
		} break;
case 18:
# line 305 "logo.y"
{goerr1();uperror;} break;
case 19:
# line 306 "logo.y"
{goerr2();uperror;} break;
case 20:
# line 307 "logo.y"
{
		if (yypvt[-1]!= -1) {
			if (!errtold)
				pf1("You don't say what to do with %l.",yypvt[-1]);
			uperror;
		}
		yyval= -1;
	} break;
case 21:
# line 315 "logo.y"
{
		if (!errtold)
			printf("Too many inputs to procedure.\n");
		uperror;
	} break;
case 22:
# line 320 "logo.y"
{
		if ((yypvt[-1] != -1) && !endflag) {
			if (!errtold)
				pf1("You don't say what to do with %l.",yypvt[-1]);
			uperror;
		}
		yyval = yypvt[-1];
	} break;
case 23:
# line 328 "logo.y"
{
		if (!errtold)
			extra();
		uperror;
	} break;
case 24:
# line 333 "logo.y"
{
		if ((yypvt[-0] != -1) && !endflag) {
			if (!errtold)
				pf1("You don't say what to do with %l.",yypvt[-0]);
			uperror;
		}
		yyval = yypvt[-0];
	} break;
case 25:
# line 341 "logo.y"
{
		if (yypvt[-0]== -1)
			uperror
		else
			catch(proccreate(yypvt[-0]););
			yyval = -1;
	} break;
case 26:
# line 348 "logo.y"
{
		if (thisrun && !pflag) {
			yyval = yypvt[-1];
		} else {
			if(yypvt[-1] != -1){
				if (!errtold)
					pf1("You don't say what to do with %l\n",yypvt[-1]);
				uperror;
			}
		}
	} break;
case 27:
# line 359 "logo.y"
{
		if (!errtold) {
			printf("There's something extra on the line.\n");
			printf("Too many inputs to procedure.\n");
		}
		uperror;
	} break;
case 30:
# line 367 "logo.y"
{
			if(yypvt[-0]== -1){
				if (!errtold) {
					puts("Your procedure doesn't have an output.");
					puts("It can't be used as an input.");
				}
				uperror;
			}
		} break;
case 32:
# line 378 "logo.y"
{
			catch(yyval=(int)(*keywords[yypvt[-2]].lexval)(yypvt[-1],yypvt[-0]););
		} break;
case 33:
# line 381 "logo.y"
{op2er1(yypvt[-2],yypvt[-1]);uperror;} break;
case 34:
# line 382 "logo.y"
{op2er2(yypvt[-1]);uperror;} break;
case 35:
# line 383 "logo.y"
{
			catch(yyval=(int)(*keywords[yypvt[-1]].lexval)(yypvt[-0]););
		} break;
case 36:
# line 386 "logo.y"
{op1err(yypvt[-1]);uperror;} break;
case 37:
# line 387 "logo.y"
{
			catch(yyval=(int)(*keywords[yypvt[-0]].lexval)(););
		} break;
case 38:
# line 390 "logo.y"
{
			catch(yyval=multiop(yypvt[-2],globcopy(multarg)););
			lfree(multarg);
			multarg = 0;
		} break;
case 39:
# line 395 "logo.y"
{
			catch(yyval=multiop(yypvt[-2],globcopy(multarg)););
			lfree(multarg);
			multarg = 0;
		} break;
case 40:
# line 400 "logo.y"
{op2er2(yypvt[-1]);uperror;} break;
case 41:
# line 401 "logo.y"
{op2er2(yypvt[-1]);uperror;} break;
case 42:
# line 402 "logo.y"
{yyval=yypvt[-1];} break;
case 44:
# line 404 "logo.y"
{ yyval=yypvt[-0]; } break;
case 45:
# line 405 "logo.y"
{ yyval=yypvt[-1]; } break;
case 46:
# line 406 "logo.y"
{
		catch(yyval=(int)alllk(yypvt[-0]););
		} break;
case 47:
# line 409 "logo.y"
{
		catch(yyval=(int)add(yypvt[-2],yypvt[-0]););
	} break;
case 48:
# line 412 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 49:
# line 413 "logo.y"
{
		catch(yyval=(int)sub(yypvt[-2],yypvt[-0]););
	} break;
case 50:
# line 416 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 51:
# line 417 "logo.y"
{
		catch(yyval=(int)opp(yypvt[-0]););
	} break;
case 52:
# line 420 "logo.y"
{unerr('-');uperror;} break;
case 53:
# line 421 "logo.y"
{
		catch(yyval=(int)lpow(yypvt[-2],yypvt[-0]););
	} break;
case 54:
# line 424 "logo.y"
{ inferr(yypvt[-2],yypvt[-1]);uperror; } break;
case 55:
# line 425 "logo.y"
{
		catch(yyval=(int)mult(yypvt[-2],yypvt[-0]););
	} break;
case 56:
# line 428 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 57:
# line 429 "logo.y"
{
		catch(yyval=(int)div(yypvt[-2],yypvt[-0]););
	} break;
case 58:
# line 432 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 59:
# line 433 "logo.y"
{
		catch(yyval=(int)rem(yypvt[-2],yypvt[-0]););
	} break;
case 60:
# line 436 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 61:
# line 437 "logo.y"
{
		catch(yyval=(int)equal(yypvt[-2],yypvt[-0]);)
	} break;
case 62:
# line 440 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 63:
# line 441 "logo.y"
{
		catch(yyval=(int)lessp(yypvt[-2],yypvt[-0]););
	} break;
case 64:
# line 444 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 65:
# line 445 "logo.y"
{
		catch(yyval=(int)greatp(yypvt[-2],yypvt[-0]););
	} break;
case 66:
# line 448 "logo.y"
{inferr(yypvt[-2],yypvt[-1]);uperror;} break;
case 68:
# line 450 "logo.y"
{
		if (yypvt[-0] == -1) {
			if (!errtold)
				puts("Run didn't output.\n");
			uperror;
		}
	} break;
case 69:
# line 457 "logo.y"
{yyval=yypvt[-1];} break;
case 70:
# line 458 "logo.y"
{
		catch(multarg = globcons(yypvt[-0],0););
		mfree(yypvt[-0]);
		multnum = 1;
	} break;
case 71:
# line 463 "logo.y"
{
		catch(multarg = globcons(yypvt[-1],multarg););
		mfree(yypvt[-1]);
		multnum++;
	} break;
case 72:
# line 468 "logo.y"
{
		strcpy(titleptr,"\n");
		yyval=yypvt[-2];
	} break;
case 73:
# line 472 "logo.y"
{
		strcpy(titleptr,"\n");
		yyval=yypvt[-1];
	} break;
case 74:
# line 476 "logo.y"
{
		mfree(yypvt[-2]);
		terr();
		yyval= -1;
	} break;
case 75:
# line 481 "logo.y"
{
		mfree(yypvt[-1]);
		terr();
		yyval= -1;
	} break;
case 76:
# line 486 "logo.y"
{
		titleptr=cpystr(titlebuf,"to ",
			((struct object *)(yypvt[-0]))->obstr,NULL);
		yyval=yypvt[-0];
	} break;
case 77:
# line 491 "logo.y"
{titleptr=cpystr(titleptr," :",
			((struct object *)(yypvt[-0]))->obstr,NULL);
		mfree(yypvt[-0]);
	} break;
case 78:
# line 495 "logo.y"
{titleptr=cpystr(titleptr," :",
			((struct object *)(yypvt[-0]))->obstr,NULL);
		mfree(yypvt[-0]);
	} break;
case 79:
# line 499 "logo.y"
{yyval=yypvt[-0];} break;
case 80:
# line 500 "logo.y"
{
		yyval=yypvt[-1];
		frmpop(yypvt[-1]);
	} break;
case 81:
# line 504 "logo.y"
{	uperror; } break;
case 82:
# line 505 "logo.y"
{
		if (!errtold) printf("Not enough inputs to %s\n",
			proclist->procname->obstr);
		uperror;
	} break;
case 85:
# line 511 "logo.y"
{
		catch(argassign(yypvt[-0]););
	} break;
case 86:
# line 514 "logo.y"
{
		catch(argassign(yypvt[-0]););
	} break;
case 87:
# line 517 "logo.y"
{procprep();} break;
case 88:
# line 518 "logo.y"
{yyline=1; yyval = -1;} break;
case 89:
# line 519 "logo.y"
{
		if (psigflag) dopause();
		if (thisrun && thisrun->str == (struct object *)(-1))
			yyprompt(1);	/* PAUSE */
		yyval=yypvt[-0];
	} break;
case 90:
# line 525 "logo.y"
{
		if (pflag) yyline++;
		if (psigflag) dopause();
		if (thisrun && thisrun->str == (struct object *)(-1))
			yyprompt(1);	/* PAUSE */
		yyval=yypvt[-0];
	} break;
case 91:
# line 532 "logo.y"
{
		if ((!errpause&&!pauselev) || !fbr) uperror;
		if (!errtold) {
			logoyerror();
		}
		errtold = 0;
		errwhere();
		yyerrok;yyclearin;
		if (thisrun && thisrun->str == (struct object *)(-1))
			yyprompt(0);	/* PAUSE */
	} break;
case 94:
# line 545 "logo.y"
{ yyline=((struct object *)yypvt[-0])->obint; mfree(yypvt[-0]); yyval = 0;} break;
case 95:
# line 546 "logo.y"
{
		catch(newproc(yypvt[-0]););
	} break;
case 96:
# line 549 "logo.y"
{
		unrun();
		yyval = yypvt[-1];
	} break;
case 97:
# line 553 "logo.y"
{
		catch(dorun(yypvt[-0],(FIXNUM)1););
	} break;
case 98:
# line 556 "logo.y"
{
		catch(dorep(yypvt[-1],yypvt[-0]););
	} break;
case 99:
# line 559 "logo.y"
{
		{
			int i;

			catch(i = truth(yypvt[-2]););
			if (i) {
				catch(dorun(yypvt[-1],(FIXNUM)1););
				mfree(yypvt[-0]);
			} else {
				catch(dorun(yypvt[-0],(FIXNUM)1););
				mfree(yypvt[-1]);
			}
		}
	} break;
case 100:
# line 573 "logo.y"
{
		if (!errtold) printf("Not enough inputs to if.\n");
		uperror;
	} break;
case 102:
# line 578 "logo.y"
{
		unrun();
		yyval = yypvt[-1];
	} break;
case 103:
# line 582 "logo.y"
{
		{
			int i;

			catch(i = truth(yypvt[-2]););
			if (i) {catch(dorun(yypvt[-1],(FIXNUM)1););}
			else {
				catch(dorun(0,(FIXNUM)1););
				mfree(yypvt[-1]);
			}
		}
	} break;
case 104:
# line 594 "logo.y"
{
		if ((int)keywords[yypvt[-2]].lexval==currtest) {
			catch(dorun(yypvt[-1],(FIXNUM)1););
		} else {
			catch(dorun(0,(FIXNUM)1););
			mfree(yypvt[-1]);
		}
	} break;
		}
		goto yystack;  /* stack new state and value */

	}
