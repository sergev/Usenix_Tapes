%{
#include "lex.h"

struct klist {
	char *keyword;
	enum keyword keyvalue;
} klist[] = {
	"exec",		K_EXEC,
	"maxexec",	K_MAXEXEC,
	"maxqueue",	K_MAXQUEUE,
	"supervisor",	K_SUPERVISOR,
	"mail",		K_MAIL,
	"mailsupervisor", K_MAILSUPERVISOR,
	"console",	K_CONSOLE,
	"nice",		K_NICE,
	"restart",	K_RESTART,
	"time",		K_TIMECMD,
	"day",		K_DAY,
	"off",		K_OFF,
	"drain",	K_DRAIN,
	"on",		K_ON,
	"loadstop",	K_LOADSTOP,
	"loadsched",	K_LOADSCHED,
	"cpulimit",	K_CPULIMIT,
	"rlimitcpu",	K_RLIMITCPU,
	"rlimitfsize",	K_RLIMITFSIZE,
	"rlimitdata",	K_RLIMITDATA,
	"rlimitstack",	K_RLIMITSTACK,
	"rlimitcore",	K_RLIMITCORE,
	"rlimitrss",	K_RLIMITRSS,
	0
};

%}

%%

[0-9]+				return K_NUMBER;

[/0-9a-zA-Z_.-]+	{
				register struct klist *kp;
				for(kp = klist ; kp->keyword ; kp++)
					if(strcmp(kp->keyword, yytext) == 0)
						return kp->keyvalue;
				return K_VARIABLE;
			}

[0-9][0-9]:[0-9][0-9] 		return K_TIME;

\n				return K_LINE;

[ \t]				;

.				return K_SYNTAX;

%%

lexfile(f)
FILE *f;
{
	char *p;
	struct yysvf ***q;
	extern struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
	extern int *yyfnd;

	yylineno = 1;
	bzero(yytext, YYLMAX*sizeof yytext[0]);
	bzero(yylstate, sizeof yylstate);
	yylsp = yyolsp = 0;
	bzero(yysbuf, YYLMAX*sizeof yysbuf[0]);
	yysptr = yysbuf;
	yyfnd = 0;
	yyprevious = YYNEWLINE;
	yyleng = 0;
	yymorfg = 0;
	yytchar = 0;
	yyin = f;
	yyestate = 0;
}

yywrap()
{
	return 1;
}
