%{
#include <ctype.h>
#include <stdio.h>
#include "nodes.h"
#define	DEFAULTTIME	32766	/* on error */
#define	LOCAL		25	/*local area network*/
#define	DEDICATED	95	/*high speed dedicated*/
#define	DIRECT		200	/*local call*/
#define	DEMAND          300	/*normal call (long distance, anytime)*/
#define	DIALED          300	/*normal call (long distance, anytime)*/
#define	HOURLY		500	/*hourly poll*/
#define	EVENING		1800	/*time restricted call*/
#define	DAILY		5000	/*daily poll*/
#define	POLLED		5000	/*daily poll*/
#define	WEEKLY		30000	/* irregular poll */
#define DEAD		32767
#define	HIGH		-5
#define	LOW		5
#define A		100	/* A-F are not standard so who knows? */
#define B		300	/* these values are just guesses      */
#define C		700
#define D		1500
#define F		15000
#define	DEFAULT	30000	/* assume worst working case (ie !dead) */
#define ARPA		5	/* arpa-net??? this is a guess too      */

%}

%start list

%token	DIGIT

%left '+'  '-'
%left '*' '/'
%left UMINUS

%%

list	:	e '\n'
	{
		return($1);
	}

e	:	'(' e ')'
	{
		$$ = $2;
	}
	|	e '+' e
	{
		$$ = $1 + $3;
	}
	|	e '-' e
	{
		$$ = $1 - $3;
	}
	|	e '*' e
	{
		$$ = $1 * $3;
	}
	|	e '/' e
	{
		$$ = $1 / $3;
	}
	|	'-' e	%prec UMINUS
	{
		$$ = -($2);
	}
	|	DIGIT
	{
		$$ = $1;
	}
	;
%%
static char *yylexptr;
yyparse1(string)
char *string;
{
	int tmp;

	yylexptr = string;
	tmp = yyparse();
	if ((tmp < 0) && (tmp != NETVAL)) {
		fprintf(stderr,"Negative cost:\n");
		fprintf(stderr,"\"%s\"\n", string);
	}
	else if (tmp == 0) {
		return(1);
	}
	/* This is for assholes who think stuff like WEEKLY*2 is
	 * reasonable.
	 */
	if(tmp > DEAD)
		tmp = DEAD;
	return(tmp);
}
yylex()
{
	int c;
	char *s;
	char lbuf[64];

	while(isspace(*yylexptr))
		yylexptr++;
	if(*yylexptr == 0){
		yylval = 0;
		return('\n');
	}
	if(isalpha(*yylexptr)){
		for(s = lbuf; isalpha(*yylexptr); *s++ = *yylexptr++)
			;
		*s = 0;
		yylval = myweight(lbuf);
		return(DIGIT);
	}
	if(isdigit(*yylexptr)){
		for(s = lbuf; isdigit(*yylexptr); *s++ = *yylexptr++)
			;
		*s = 0;
		yylval = atoi(lbuf);
		return(DIGIT);
	}
	return(*yylexptr++);
}

myweight(s)
register char *s;
{
	if(strcmp(s, "LOCAL") == 0)
		return(LOCAL);
	if(strcmp(s, "DEDICATED") == 0)
		return(DEDICATED);
	if(strcmp(s, "DIRECT") == 0)
		return(DIRECT);
	if(strcmp(s, "DEMAND") == 0)
		return(DEMAND);
	if(strcmp(s, "DIALED") == 0)
		return(DIALED);
	if(strcmp(s, "HOURLY") == 0)
		return(HOURLY);
	if(strcmp(s, "EVENING") == 0)
		return(EVENING);
	if(strcmp(s, "DAILY") == 0)
		return(DAILY);
	if(strcmp(s, "POLLED") == 0)
		return(POLLED);
	if(strcmp(s, "WEEKLY") == 0)
		return(WEEKLY);
	if(strcmp(s, "DEAD") == 0)
		return(DEAD);
	if(strcmp(s, "HIGH") == 0)
		return(HIGH);
	if(strcmp(s, "LOW") == 0)
		return(LOW);
	if(strcmp(s, "A") == 0)
		return(A);
	if(strcmp(s, "B") == 0)
		return(B);
	if(strcmp(s, "C") == 0)
		return(C);
	if(strcmp(s, "D") == 0)
		return(D);
	if(strcmp(s, "F") == 0)
		return(F);
	if(strcmp(s, "ARPA") == 0)
		return(ARPA);
	if(strcmp(s, "DEFAULT") == 0)
		return(DEFAULT);
	return(DEFAULTTIME);
}
yyerror(s)
char *s;
{
	printf("syntax error %s\n", s);
}
