%{
#include	<stdio.h>
long inline = 1;
int narrate = 0;
int ready = 0;
int dopop = 0;
int ltext = 0;
char *dest = "leroy.out";
double sqrt(), sin(), cos(), floor();
char *malloc();
char epath[100];

typedef struct point {
	float xp;
	float yp;
} POINT;
POINT	from = { 0.0,0.0};
POINT	to = { 10.0,7.5};
POINT	lleft = { 0.0, 0.0 };
POINT	uright = { 10.0, 7.5 };
POINT	llf = { 1.,1.};
POINT	urf = { 9.0, 6.5};
POINT	llb = {1.,1.};
POINT	urb = { 9.5, 6.5 };
POINT	tpt;
POINT	ull, uur;
POINT	ROTUR = {7.5,10.0};
POINT	vgetp(), vputp(), valter();
float	vput(), vget();
long	lget();

extern char *yytext;

%}

%token	INIT	CLEAR	PAGE	USE	MAP	FRAME
%token	SET	DRAW	TO	TEXT	SHOW	DONE
%token	DEST	UP	DOWN	LEFT	RIGHT	GRAPHXY
%token	PLEQ	MIEQ	MUEQ	DIEQ	IMPOSE	TICKS
%token	ROTATE	CLIP	NOCLIP	INCLUDE	PUSH	POP
%token	PLOT	LLINE	RLINE	CLINE	VALUE	VARIABLE
%token	CIRCLE	EXEC	GSCAN	SYSTEM	CLOSE	EOL

%left UMINUS
%right '=' PLEQ MIEQ MUEQ DIEQ
%left '+' '-'
%left '*' '/'
%left '~'

%start figure

%union {
	int intval;
	float fval;
	char *sval;
	POINT pval;
}

%type	<fval>	VALUE
%type	<sval>	VARIABLE
%type	<sval>	TEXT
%type	<sval>	DEST
%type	<pval>	expr

%%

figure:	
	|	figure action EOL elist
			{if(ltext>0) --ltext;}
	|	figure error
		{ yyerror("figure error");}
	;

action:		init
			{ init();}
	|	clear
			{ init(); clear_();}
	|	page
			{ init(); page( from, to);}
	|	use
			{ init(); use(ull,uur);}
	|	map
			{ init(); map(lleft,uright);}
	|	frame
			{ init(); frame(llf,urf);}
	|	set
	|	draw
	|	circle
	|	to
	|	show
	|	impose
			{ subbox(llb,urb);}
	|	dest
			{ if( ready!=0 ) yyerror("write too late");}
	|	graph
	|	gscan
	|	ticks
	|	line
	|	rotate
			{
			if( ready != 0 ) yyerror("rotate too late");
			else {
				to=ROTUR; uright=ROTUR;
				vputp("UR",ROTUR);
				vput("rotate",1.0);
				}
			}
	|	clip
	|	noclip
	|	include
	|	exec
	|	system
	|	plot
	|	push
	|	pop
	|	text
	|	close
	;

init:		INIT optionlist
	;

clear:		CLEAR optionlist
	;

page:		PAGE expr expr optionlist
			{ from = $2; to = $3;}
	;

use:		USE expr expr optionlist
			{ ull = $2; uur = $3;}
	;

map:		MAP expr expr optionlist
			{ lleft = $2; uright = $3;}
	;

impose:		IMPOSE expr expr optionlist
			{ llb=$2; urb=$3;}
	;

frame:		FRAME expr expr optionlist llist
			{ llf = $2; urf = $3;}
	;

set:		SET optionlist
	;

draw:		DRAW expr expr optionlist llist
			{ init(); draw($2,$3);}
	;

circle:		CIRCLE expr expr optionlist llist
			{ init();
			circle( $2, $3.xp);
			}
	;

ticks:		TICKS expr expr optionlist llist
			{ init(); ticks($2,$3);}
	;

to:		TO expr optionlist
			{
			vputp("x",$2);
			}
	|	UP expr
			{ valter("x",0.,$2.xp*vget("vs"));}
	|	UP
			{ valter("x",0.,vget("vs"));}
	|	DOWN expr
			{ valter("x", 0., -$2.xp*vget("vs"));}
	|	DOWN
			{ valter("x", 0., -vget("vs"));}
	|	LEFT expr
			{ valter("x", -$2.xp*vget("hs"),0.);}
	|	LEFT
			{ valter("x", -vget("hs"), 0.);}
	|	RIGHT expr
			{ valter("x", $2.xp*vget("hs"), 0.);}
	|	RIGHT
			{ valter("x", vget("hs"), 0.);}
	;

show:		SHOW
	|	show '!'
			{ vlist();}
	|	show VARIABLE
			{
			tpt=vgetp($2);
			fprintf(stderr,"%s = (%f,%f)\n",$2,tpt.xp,tpt.yp);
			free($2);
			}
	|	show expr
			{ printf("(%f,%f)\n",$2.xp,$2.yp); }
	;


dest:		DEST VARIABLE optionlist
			{ dest = $2; }
	;
graph:		GRAPHXY VARIABLE optionlist llist
			{
			init();
			params();
			grplot( $2 );
			free( $2 );
			}
	|	GRAPHXY '<' VARIABLE '>' optionlist llist
			{
			init();
			params();
			strcpy(epath,"/usr/lib/leroy/");
			strcat(epath,$3);
			free($3);
			grplot(epath);
			}
		;

gscan:		GSCAN VARIABLE optionlist
			{
			gscan($2);
			free($2);
			}
		;

plot:		PLOT VARIABLE expr expr optionlist llist
			{
			init();
			plot($2, $3, $4);
			free($2);
			}
		;

line:		LLINE expr optionlist llist
			{ dline($2,0);}
	|	RLINE expr optionlist llist
			{ dline($2,1);}
	|	CLINE expr optionlist llist
			{ dline($2,2);}
	;

rotate:		ROTATE	optionlist
		;

clip:		CLIP optionlist
			{ vput("noclip",0.); }
		;

noclip:		NOCLIP optionlist
			{ vput("noclip", 1.); }
		;

include:	INCLUDE VARIABLE
			{
			iswitch($2);
			yyclearin;
			free($2);
			}
	|	INCLUDE '<' VARIABLE '>'
			{
			strcpy (epath, "/usr/lib/leroy/");
			strcat(epath,$3);
			iswitch(epath);
			yyclearin;
			free($3);
			}
		;

exec:		EXEC VARIABLE
			{
			fprintf(stderr,"exec not implemented");
			free($2);
			}
		;

system:		SYSTEM VARIABLE
			{
			system($2);
			free($2);
			}
		;

close:		CLOSE
			{
			done_();
			}
		;

push:		PUSH
			{ vpush();}
		;

pop:		POP
			{ vpop();}
		;

text:		TEXT
			{
			init();
			params();
			if(ltext>0) valter("x",0.,-vget("vs"));
			textout( $1 );
			free( $1 );
			ltext=2;
			}
	;


llist:
	|		'{'
			{ vpush(); dopop=1;}
		optionlist '}'
	;

elist:			{
			if( dopop==1 ) {
				vpop();
				dopop = 0;
				}
			}
	;

optionlist:
	|	 optionlist expr
	;

expr:		VARIABLE '=' expr
			{
			$$ = vputp( $1, $3);
			free($1);
			}
	|	VARIABLE PLEQ expr
			{
			tpt = vgetp( $1);
			tpt.xp += $3.xp;
			tpt.yp += $3.yp;
			$$ = vputp( $1, tpt);
			free( $1 );
			}
	|	VARIABLE MIEQ expr
			{
			tpt = vgetp( $1);
			tpt.xp -= $3.xp;
			tpt.yp -= $3.yp;
			$$ = vputp( $1, tpt);
			free( $1 );
			}
	|	VARIABLE MUEQ expr
			{
			tpt = vgetp( $1);
			tpt.xp *= $3.xp;
			tpt.yp *= $3.yp;
			$$ = vputp( $1, tpt);
			free ( $1 );
			}
	|	VARIABLE DIEQ expr
			{
			tpt = vgetp( $1 );
			tpt.xp /= $3.xp;
			tpt.yp /= $3.yp;
			$$ = vputp( $1, tpt);
			free($1);
			}
	|	'(' expr ')'
			{ $$ = $2; }
	|	'(' expr ',' expr ')'
			{
			$$.xp = $2.xp;
			$$.yp = $4.yp;
			}
	|	expr '+' expr
			{
			$$.xp = $1.xp + $3.xp;
			$$.yp = $1.yp + $3.yp;
			}
	|	expr '-' expr
			{
			$$.xp = $1.xp - $3.xp;
			$$.yp = $1.yp - $3.yp;
			}
	|	expr '*' expr
			{
			$$.xp = $1.xp * $3.xp;
			$$.yp = $1.yp * $3.yp;
			}
	|	expr '/' expr
			{
			$$.xp = $1.xp / $3.xp;
			$$.yp = $1.yp / $3.yp;
			}
	|	'-' expr	%prec UMINUS
			{
			$$.xp = - $2.xp;
			$$.yp = - $2.yp;
			}
	|	'~' expr
			{
			$$.xp = $2.yp;
			$$.yp = $2.xp;
			}
	|	'|' expr '|'
			{
			$$.xp = $$.yp = sqrt( $2.xp*$2.xp + $2.yp*$2.yp );
			}
	|	'<' expr '>'
			{
			$$.xp = cos( (double) (0.01745329*$2.xp) );
			$$.yp = sin( (double) (0.01745329*$2.xp) );
			}
	|	'[' expr ']'
			{
			$$.xp=$$.yp=(float)floor((double)$2.xp);
			}
	|	VALUE
			{ $$.xp = $$.yp = $1;}
	|	VARIABLE
			{
			$$ = vgetp( $1 );
			free( $1 );
			}

%%

#include "main.c"

#include "vlist.c"

#include "subs.c"

