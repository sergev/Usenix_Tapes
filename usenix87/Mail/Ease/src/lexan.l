%{
/*	$Header: /usr/src/local/etc/ease/RCS/lexan.l,v 1.2 85/10/29 23:42:40 jss Exp $	*/

/*
 *	lexan.l -- Lexical Analyzer for EASE.
 *
 *		   Contains code for lex(1) which generates a lexical
 *		   analyzer (lex.yy.c) for Ease, a high-level specification 
 *		   format for sendmail configuration files.
 *
 *	author -- James S. Schoner, Purdue University Computing Center,
 *				    West Lafayette, Indiana  47907
 *
 *	date   -- July 1, 1985
 *
 *	Copyright (c) 1985 by James S. Schoner
 *
 *	All rights reserved.
 *
 */

#include "symtab.h"
#include "lexdefs.h"

#define  LEXnewline '\n'
#define	 LEXeof	    '\0'
#define  MaxFN	    200			/* maximum file name length */

extern struct he *LookupSymbol ();
extern void	  ErrorReport ();

int  Lcount;				/* line counter		    */
char FNbuf[MaxFN];			/* file name buffer	    */
short RMatch  = FALSE;			/* ruleset match flag  	    */

%}

/* lex-specific extensions */
%e 1100
%p 3700
%n 500

%%
	int INch;			/* any input character */

[ \t\f]+			; 	/* discard whitepsace  */
[\n]				Lcount++;
^\#[ \t]*[0-9]+[ \t]*\".*\"[ \t]*$	{
			        sscanf (yytext, "%*c%d%s", &Lcount, FNbuf);
			        INch = input ();
			        }
match				return (MATCH);
in				return (IN);
bind				return (BIND);
define				return (DEFINE);
macro				return (MACRO);
class				return (CLASS);
options				return (OPTIONS);
precedence			return (PRECEDENCE);
trusted				return (TRUSTED);
header				return (HEADER);
ruleset				return (RULESET);
mailer				return (MAILER);
host				return (HOST);
user				return (USER);
hostnum				return (HOSTNUM);
if				return (IF);
retry				return (RETRY);
next				return (NEXT);
return				return (RETURN);
resolve				return (RESOLVE);
for				return (FOR);
field				return (FIELD);
concat				return (CONCAT);
ifset				return (IFSET);
canon				return (CANON);
readclass			return (READCLASS);
Path				return (MPATH);
Flags				return (MFLAGS);
Sender				return (MSENDER);
Recipient			return (MRECIPIENT);
Argv				return (MARGV);
Eol				return (MEOL);
Maxsize				return (MMAXSIZE);
o_alias				return (AAOPT);
o_ewait				return (AOPT);
o_bsub				return (BBOPT);
o_qwait				return (COPT);
o_delivery			return (DOPT);
d_interactive			return (DOPTI);
d_background			return (DOPTB);
d_queue				return (DOPTQ);
o_rebuild			return (DDOPT);
o_handling			return (EOPT);
h_print				return (EOPTP);
h_exit				return (EOPTE);
h_mail				return (EOPTM);
h_write				return (EOPTW);
h_mailz				return (EOPTZ);
o_tmode				return (FFOPT);
o_usave				return (FOPT);
o_gid				return (GOPT);
o_fsmtp				return (HHOPT);
o_skipd				return (IOPT);
o_slog				return (LLOPT);
o_rsend				return (MOPT);
o_dnet				return (NNOPT);
o_hformat			return (OOPT);
o_qdir				return (QQOPT);
o_tread				return (ROPT);
o_flog				return (SSOPT);
o_safe				return (SOPT);
o_qtimeout			return (TTOPT);
o_timezone			return (TOPT);
o_dmuid				return (UOPT);
o_verbose			return (VOPT);
o_wizpass			return (WWOPT);
o_loadq				return (XOPT);
o_loadnc			return (XXOPT);
f_ffrom				return (FFLAG);
f_rfrom				return (RFLAG);
f_noreset			return (SSFLAG);
f_noufrom			return (NFLAG);
f_locm				return (LFLAG);
f_strip				return (SFLAG);
f_mult				return (MFLAG);
f_from				return (FFFLAG);
f_date				return (DDFLAG);
f_mesg				return (MMFLAG);
f_full				return (XFLAG);
f_return			return (PPFLAG);
f_upperu			return (UFLAG);
f_upperh			return (HFLAG);
f_arpa				return (AAFLAG);
f_ufrom				return (UUFLAG);
f_expensive			return (EFLAG);
f_dot				return (XXFLAG);
f_llimit			return (LLFLAG);
f_retsmtp			return (PFLAG);
f_smtp				return (IIFLAG);
f_addrw				return (CCFLAG);
[A-Za-z][A-Za-z0-9_-]*		{
				/* store identifiers in symbol table */
				yylval.phe = LookupSymbol (yytext);
				return (IDENT);
				}
["]((\\\n)|(\\\")|[^"\n])*	{
				if ((INch = input()) == LEXnewline) {
					ErrorReport ("End of line in string.\n");
					unput (INch);
				}
				yylval.psb = (char *) malloc (strlen (yytext) + 1);
				strcpy (yylval.psb, yytext + 1);
				return (SCONST);
				}
[0][0-7]*			{
				sscanf (yytext, "%o", &yylval.ival);  /* octal constant */
				return (ICONST);
				}
[-]?[1-9][0-9]*			{
				yylval.ival = atoi (yytext);
				return (ICONST);
				}
"="				return (ASGN);
","				return (COMMA);
"{"				return (LBRACE);
"}"				return (RBRACE);
"("				return (LPAREN);
")"				return (RPAREN);
";"				return (SEMI);
"$"				return (DOLLAR);
":"				return (COLON);
"*"				return (STAR);
"/*"				{
				/* eat C comments */
				INch = input ();
				while ((INch != '*') || 
				      ((INch = input ()) != '/')) {
					if (INch == LEXnewline)
						Lcount++;
					else
						if (INch == LEXeof) {
							ErrorReport ("End of file in comment.\n");
							break;
						}
					if (INch != '*')
						INch = input ();
				}
				}
[\\]?.				{
				if (RMatch) {	/* in rulesets, return literal character */
					yylval.ival = (yytext[0] == '\\') ? yytext[1] : yytext[0];
					return (SEPCHAR);
				} else {
					ErrorReport ("Illegal delimiter character");
					printf (": (octal code) \\%03o\n", *yytext);
				}
				}
%%
