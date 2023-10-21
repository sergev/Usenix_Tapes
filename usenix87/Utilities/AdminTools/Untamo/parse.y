%{
#include <stdio.h> 
#include <grp.h>
#include "untamo.h"
struct group *grp;
char oct[5];		/* rld kludge */
char *name;
int  num;
extern char *yytext;

  /*
   *			vvvvvvvvvvvvvvvvvvvvvvvvvvvvv
   *   			>>>>>>>>> IMPORTANT <<<<<<<<<
   *			^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   *  The order of the tokens in the *first line* is significant.  
   *
   *  They dictate which rules and exemptions have precence.  This order 
   *  states that RLD is the  most significant, and Untamo will use an
   *  rld rule first if it has a choice.  The second two %token lines may
   *  be ordered anyway.  DEFAULT is the least specific, but will always
   *  match.  It must always remain in the last position.
   */ 
%}

%token RLD TTY LOGIN GROUP CLUSTER DEFAULT

%token EXEMPT TIMEOUT SLEEP SESSION
%token NUM IDLE MULTIPLE NAME ALL 
%token THRESHOLD NL

%union {
	char *sb; 
	int nb;
       }

%type <sb> NAME
%type <nb> NUM LOGIN GROUP TTY RLD ALL IDLE MULTIPLE 
%type <nb> who exempt_type name_type

%start cmd_cmd

%%

cmd_cmd	: /*EMPTY*/
	| cmd_cmd exempt_cmd
	| cmd_cmd idle_cmd
	| cmd_cmd sleep_cmd
	| cmd_cmd session_cmd
	| cmd_cmd thresh_cmd
	| cmd_cmd error NL
	| cmd_cmd NL
	;

thresh_cmd : THRESHOLD MULTIPLE NUM NL
	{
		m_threshold = $3; 
	}
	| THRESHOLD SESSION NUM NL
	{
		s_threshold = $3; 
	}
	| THRESHOLD error NL
	{
		yyerror("Malformed threshold command.");
	}
	;
	

exempt_cmd	: EXEMPT who exempt_type NL
		{
			addlist(exmpt,$2,name,num,$3);
		}
		| EXEMPT error NL
		{
			yyerror("Malformed exempt command.");
		}
		;

session_cmd     : SESSION who NUM NL
		{
			addlist(session,$2,name,num,$3);
		}
		| SESSION error NL
		{
			yyerror("Malformed session command.");
		}
		;

idle_cmd	: TIMEOUT who NUM NL
		{
			addlist(rules,$2,name,num,$3);
		}
		| TIMEOUT DEFAULT NUM NL
		{
			addlist(rules, DEFAULT, NULL, 0, $3);
		}
		| TIMEOUT error NL
		{
			yyerror("Malformed timeout command.");
		}
		;

sleep_cmd	: SLEEP NUM NL
		{
			sleeptime = $2;
		}
		| SLEEP error NL
		{
			yyerror("Malformed sleep command.");
		}
		;

who		: name_type NAME
		{ 
			$$ = $1;
			name = $2;
			if ($1 == GROUP)  {
				grp = getgrnam(name);
				num = grp->gr_gid;
			}
		}
		| name_type NUM
		{
			$$ = $1;
			/*
			 * Kludge alert: here we must convert the
			 * rld number, which was read as decimal
			 * (lex doesn't know any better...) to an 
			 * octal so that it will jive with what
			 * findrld (in libacct) returns.
			 */
			if ($1 == RLD)  {
				sprintf(oct,"%d",$2);
				sscanf(oct,"%o",&num);
			} else
				num = $2;
		}
		;

name_type	: CLUSTER	{ $$ = CLUSTER; }
		| LOGIN 	{ $$ = LOGIN;   }
		| GROUP		{ $$ = GROUP;   }
		| TTY		{ $$ = TTY;     }
		| RLD		{ $$ = RLD;     }
		;

exempt_type	: ALL		{ $$ = ALL; 	 }
		| IDLE		{ $$ = IDLE; 	 }
		| MULTIPLE	{ $$ = MULTIPLE; }
		| SESSION	{ $$ = SESSION;  }
		;

%%
static int errorcnt = 0;

yyerror(sb)
char *sb;
{
	extern int linenum;
	char buf[128];

	sprintf(buf, "%s: line %d: %s\n", CONFIG, linenum, sb);
	error( buf );
	errorcnt++;
}

yywrap()
{
	extern int linenum;

	if( errorcnt > 0 ){
	    error( "Aborting due to config file syntax errors.\n");
	    exit( 1 );
	}
	linenum = 1;
	return 1;
}
