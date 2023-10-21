
%{

/*	lexical analyzer for leroy
 */

#undef input
#undef unput

typedef struct point {
	float xp;
	float yp;
} POINT;

#include "y.tab.h"

extern long inline;
double atof();

%}

P	[\.]
D	[0-9]
W	[\t ]
N	[\n]

%%

^{P}init{W}*	{return(INIT);}

^{P}clear{W}*	{return(CLEAR);}

^{P}page{W}*	{return(PAGE);}

^{P}use{W}*	{return(USE);}

^{P}map{W}*	{return(MAP);}

^{P}impose{W}*	{return(IMPOSE);}

^{P}frame{W}*	{return(FRAME);}

^{P}{W}*	|
^{P}set{W}*	{return(SET);}

^{P}draw{W}*	{return(DRAW);}

^{P}ticks{W}*	{return(TICKS);}

^{P}to{W}*	{return(TO);}

^{P}show{W}*	{return(SHOW);}

^{P}done{W}*	{return(DONE);}

^{P}write{W}*	{ return(DEST);}

^{P}up{W}*	{ return(UP);}

^{P}down{W}*	{ return(DOWN);}

^{P}left{W}*	{ return(LEFT);}

^{P}right{W}*	{ return(RIGHT);}

^{P}graph{W}*	|
^{P}graphxy{W}*	{ return(GRAPHXY); }

^{P}gscan{W}*	{ return(GSCAN); }

^{P}rotate{W}*	{ return(ROTATE); }

^{P}system{W}*	|
^{P}!{W}*	{ return(SYSTEM); }

^{P}close{W}*	{ return(CLOSE); }

^{P}exec{W}*	{ return(EXEC);}

^{P}clip{W}*	{ return(CLIP); }

^{P}noclip{W}*	{ return(NOCLIP); }

^{P}include{W}*	{return(INCLUDE);}

^{P}push{W}*	{return(PUSH);}

^{P}pop{W}*	{return(POP);}

^{P}plot{W}*	{return(PLOT);}

^{P}line{W}*	|
^{P}lline{W}*	{return(LLINE);}

^{P}rline{W}*	{return(RLINE);}

^{P}cline{W}*	{return(CLINE);}

^{P}circle{W}*	{return(CIRCLE);}

^{P}c\n		|
^{P}c[ \t]+.*\n	{ inline++; /* comment */ }

\n{P}\&{W}*	{ inline++; /* command continuation */ }

^{W}*\n		{inline++;}

\+\=		{ return( PLEQ); }

\-\=		{ return( MIEQ ); }

\*\=		{ return( MUEQ ); }

\/\=		{ return( DIEQ ); }


^[^\.\n].*	{
		ssave( &yylval.sval );
		return( TEXT );
		}

{W}+		{;}
{N}		{
		inline++;
		return(EOL);
		}

\"[^\"]*\"		|
[a-zA-Z][a-zA-Z0-9_\.]*	{
			ssave( &yylval.sval );
			return( VARIABLE );
			}

{D}+"."?{D}*	|
{D}*"."{D}+	|
{D}+"."?{D}*[eEdD][+-]?{D}+ |
{D}*"."{D}+[eEdD][+-]?{D}+	{
			yylval.fval = atof(yytext);
			return(VALUE);
			}

[^a-zA-Z0-9]	{ return(yytext[0]);}

%%

/*	strip a leading | or enclosing quotes but not both	*/

ssave(to)
char *(*to);
{	(*to) = (char *) malloc( (unsigned)(yyleng+1) );
	strncpy( (*to), yytext, yyleng);
	(*to)[yyleng] = '\0';
	if( (*to)[0] == '|' ) {
		int i;
		for( i=0; (*to)[i] != '\0'; i++) (*to)[i]=(*to)[i+1];
	}

	else if( (*to)[0] == '"' && (*to)[yyleng-1] == '"' && yyleng>2 ) {
		int i;
		for( i=0; i<yyleng-1; i++) (*to)[i] = (*to)[i+1];
		(*to)[yyleng-2]='\0';
	}
	return( 0 );
}

yywrap()
{
	return(1);
}

#include "lexio.c"
