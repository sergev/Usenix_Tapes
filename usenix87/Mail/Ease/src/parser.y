%{
/*	$Header: /usr/src/local/etc/ease/RCS/parser.y,v 1.3 85/12/10 18:02:11 jss Exp $	*/

/*
 *	parser.y -- EASE parser.
 *
 *		    Contains code for yacc(1) which produces a parser (y.tab.c)
 *		    for Ease, a specification format for sendmail configuration
 *		    files.
 *
 *	author   -- James S. Schoner, Purdue University Computing Center,
 *		    		      West Lafayette, Indiana  47907
 *
 *	date     -- July 2, 1985
 *
 *	Copyright (c) 1985 by James S. Schoner
 *
 *	All rights reserved.
 *
 */

#include <stdio.h>
#include "symtab.h"

extern void	   BindID ();
extern void	   EmitDef ();
extern char	  *ListAppend ();
extern char 	  *MakeCond ();
extern char	  *MakeRStr ();
extern char       *ConvOpt ();
extern char	  *ConvFlg ();
extern char	  *MacScan ();
extern char	  *ConvMat ();
extern void	   StartRuleset ();
extern char	  *MakePosTok ();
extern char	  *GetField ();
extern char	  *Bracket ();
extern char	  *MakeRSCall ();
extern char	  *CheckMailer ();
extern char	  *CheckRS ();
extern char	  *MakeField ();
extern char	   MakeMac ();
extern void	   AssignType ();
extern void	   RemoveSymbol ();
extern void	   yyerror ();

extern short RMatch;		/* ruleset match flag 		      */

char *Cbuf = " ";		/* character buffer   		      */
char *Mbuf = "$ ";		/* macro buffer	      		      */
char *Tsb;			/* pointer to temporary string buffer */
char *Flaglist;			/* pointer to header flag list	      */

%}

%union {			/* value stack element type    */
	int	  ival;		/* integer token 	       */
	char	  *psb;		/* string token		       */
	struct he *phe;		/* pointer to hash entry       */
	enum opts optval;	/* sendmail options	       */
	enum flgs flgval;	/* mailer flags		       */
	enum mats mpval;	/* mailer attribute parameters */
}

%start config

%token 	<phe>	IDENT
%token  <psb>	SCONST
%token  <ival>	ICONST SEPCHAR
%token BIND MACRO CLASS OPTIONS PRECEDENCE TRUSTED HEADER RULESET MAILER
%token IF RETRY NEXT RETURN RESOLVE CONCAT IFSET FOR CANON READCLASS
%token MPATH MFLAGS MSENDER MRECIPIENT MARGV MEOL MMAXSIZE
%token AAOPT AOPT BBOPT COPT DOPT DOPTI DOPTB DOPTQ DDOPT EOPT EOPTP EOPTE 
%token EOPTM EOPTW EOPTZ FFOPT FOPT GOPT HHOPT IOPT LLOPT MOPT NNOPT OOPT QQOPT
%token ROPT SSOPT SOPT TTOPT TOPT UOPT VOPT WWOPT XOPT XXOPT
%token FFLAG RFLAG SSFLAG NFLAG LFLAG SFLAG MFLAG FFFLAG DDFLAG MMFLAG XFLAG
%token PPFLAG UFLAG HFLAG AAFLAG UUFLAG EFLAG XXFLAG LLFLAG PFLAG IIFLAG CCFLAG
%token ASGN COMMA LBRACE RBRACE LPAREN RPAREN SEMI DOLLAR MATCH IN HOSTNUM
%token DEFINE FIELD COLON STAR HOST USER

%type	<psb>		mval strval ifcon conval ifres elseres nameset namelist
%type	<psb>		doptid eoptid idlist fcond dlist mflags route mdefs
%type	<psb>		matchaddr matchtok action actionstmt mailerspec mtdef
%type	<psb>		rwaddr rwtok ftype reftok rword cantok resolution
%type	<psb>		userspec hword hostid dheader
%type	<ival>		anychar
%type	<phe>		cdef
%type	<optval>	optid
%type	<flgval>	flagid
%type	<mpval>		mvar

%left COMMA
%left LPAREN RPAREN
%nonassoc SCONST

%%
config		:	/* empty */
		|	config blockdef
		|	error blockdef
		;

blockdef	:	BIND bindings
		|	MACRO macdefs
		|	CLASS classdefs
		|	OPTIONS optdefs
		|	PRECEDENCE precdefs
		|	TRUSTED tlist
		|	HEADER hdefs
		|	MAILER mlist
		|	RULESET rdef
		|	FIELD fdefs
		;

bindings	:	/* empty */
		|	bindings IDENT ASGN RULESET ICONST SEMI {
				BindID ($2, $5, ID_RULESET);
			}
		|	error SEMI {
				yyerrok;
			}
		;

macdefs		:	/* empty */
		|	macdefs IDENT ASGN mval SEMI {
				EmitDef (def_macro, $2, $4, (char *) NULL);
			}
		|	error SEMI {
				yyerrok;
			}
		;

mval		:	strval				%prec COMMA {
				$$ = $1;
			}
		|	CONCAT LPAREN conval RPAREN {
				$$ = $3;
			}
		;

strval		:	SCONST {
				$$ = $1;
			}
		|	strval SCONST {
				$$ = ListAppend ($1, $2, (char *) NULL);
				free ($1);
			}
		;

conval		:	strval COMMA ifcon {
				$$ = ListAppend ($1, $3, (char *) NULL);
				free ($1);
				free ($3);
			}
		|	ifcon COMMA strval {
				$$ = ListAppend ($1, $3, (char *) NULL);
				free ($1);
				free ($3);
			}
		|	error {
				$$ = NULL;
			}
		;

ifcon		:	IFSET LPAREN IDENT COMMA ifres RPAREN {
				$$ = MakeCond ($3, $5);
			}
		;

ifres		:	mval elseres {
				if ($2 != NULL) {
					$$ = ListAppend ($1, $2, "$|");
					free ($1);
					free ($2);
				} else
					$$ = $1;
			}
		|	error {
				$$ = NULL;
			}
		;

elseres		:	/* empty */ {
				$$ = NULL;
			}
		|	COMMA mval {
				$$ = $2;
			}
		;

classdefs	:	/* empty */ 
		|	classdefs IDENT ASGN nameset {
				EmitDef (def_class, $2, $4, (char *) NULL);
			}
		|	error
		;

nameset		:	LBRACE namelist RBRACE SEMI {
				$$ = $2;
			}
		|	LBRACE RBRACE SEMI {
				$$ = NULL;
			}
		|	LBRACE error RBRACE SEMI {
				$$ = NULL;
			}
		|	READCLASS LPAREN strval RPAREN SEMI {
				$$ = MakeRStr ($3, (char *) NULL);
			}
		|	READCLASS LPAREN strval COMMA strval RPAREN SEMI {
				$$ = MakeRStr ($3, $5);
			}
		|	READCLASS LPAREN error RPAREN SEMI {
				$$ = NULL;
			}
		|	error SEMI {
				$$ = NULL;
				yyerrok;
			}
		;

namelist	:	IDENT {
				$$ = ListAppend ($1->psb, (char *) NULL, (char *) NULL);
				RemoveSymbol ($1);
			}
		|	strval {
				$$ = $1;
			}
		|	namelist COMMA IDENT {
				$$ = ListAppend ($1, $3->psb, " ");
				free ($1);
				RemoveSymbol ($3);
			}
		|	namelist COMMA strval {
				$$ = ListAppend ($1, $3, " ");
				free ($1);
				free ($3);
			}
		;

optdefs		:	/* empty */
		|	optdefs optid ASGN strval SEMI {
				EmitDef (def_option, (struct he *) NULL, ConvOpt ($2), $4);
			}
		|	optdefs DOPT ASGN doptid SEMI {
				EmitDef (def_option, (struct he *) NULL, ConvOpt (opt_d), $4);
			}
		|	optdefs EOPT ASGN eoptid SEMI {
				EmitDef (def_option, (struct he *) NULL, ConvOpt (opt_e), $4);
			}
		|	error SEMI {
				yyerrok;
			}
		;

optid		:	AAOPT {
				$$ = opt_A;
			}
		|	AOPT {
				$$ = opt_a;
			}
		|	BBOPT {
				$$ = opt_B;
			}
		|	COPT {
				$$ = opt_c;
			}
		|	DDOPT {
				$$ = opt_D;
			}
		|	FFOPT {
				$$ = opt_F;
			}
		|	FOPT {
				$$ = opt_f;
			}
		|	GOPT {
				$$ = opt_g;
			}
		|	HHOPT {
				$$ = opt_H;
			}
		|	IOPT {
				$$ = opt_i;
			}
		|	LLOPT {
				$$ = opt_L;
			}
		|	MOPT {
				$$ = opt_m;
			}
		|	NNOPT {
				$$ = opt_N;
			}
		|	OOPT {
				$$ = opt_o;
			}
		|	QQOPT {
				$$ = opt_Q;
			}
		|	ROPT {
				$$ = opt_r;
			}
		|	SSOPT {
				$$ = opt_S;
			}
		|	SOPT {
				$$ = opt_s;
			}
		|	TTOPT {
				$$ = opt_T;
			}
		|	TOPT {
				$$ = opt_t;
			}
		|	UOPT {
				$$ = opt_u;
			}
		|	VOPT {
				$$ = opt_v;
			}
		|	WWOPT {
				$$ = opt_W;
			}
		|	XOPT {
				$$ = opt_x;
			}
		|	XXOPT {
				$$ = opt_X;
			}
		;

doptid		:	DOPTI {
				$$ = ConvOpt (d_opt_i);
			}
		|	DOPTB {
				$$ = ConvOpt (d_opt_b);
			}
		|	DOPTQ {
				$$ = ConvOpt (d_opt_q);
			}
		;

eoptid		:	EOPTP {
				$$ = ConvOpt (e_opt_p);
			}
		|	EOPTE {
				$$ = ConvOpt (e_opt_e);
			}
		|	EOPTM {
				$$ = ConvOpt (e_opt_m);
			}
		|	EOPTW {
				$$ = ConvOpt (e_opt_w);
			}
		|	EOPTZ {
				$$ = ConvOpt (e_opt_z);
			}
		;

precdefs	:	/* empty */
		|	precdefs IDENT ASGN ICONST SEMI {
				BindID ($2, $4, ID_PREC);
				EmitDef (def_prec, $2, (char *) NULL, (char *) NULL);
			}
		;

tlist		:	/* empty */
		|	tlist LBRACE IDENT idlist RBRACE SEMI {
				EmitDef (def_trusted, (struct he *) NULL, 
					 ListAppend ($3->psb, $4, " "), (char *) NULL);
				free ($4);
				RemoveSymbol ($3);
			}
		|	tlist LBRACE RBRACE SEMI
		|	error SEMI {
				yyerrok;
			}
		;

hdefs		:	/* empty */
		|	hdefs FOR fcond dheader SEMI {
				EmitDef (def_header, (struct he *) NULL, $3, $4);
			}
		|	hdefs FOR fcond LBRACE { Flaglist = $3; } 
				dheaders RBRACE SEMI
		|	hdefs DEFINE dlist SEMI {
				EmitDef (def_header, (struct he *) NULL, (char *) NULL, $3);
			}
		|	error SEMI {
				yyerrok;
			}
		;

fcond		:	LPAREN RPAREN {
				$$ = NULL;
			}
		|	LPAREN mflags RPAREN {
				$$ = $2;
			}
		|	LPAREN error RPAREN {
				$$ = NULL;
			}
		;

mflags		:	flagid {
				$$ = ListAppend (ConvFlg ($1), (char *) NULL, (char *) NULL);
			}
		|	mflags COMMA flagid {
				$$ = ListAppend ($1, ConvFlg($3), (char *) NULL);
				free ($1);
			}
		;

flagid		:	FFLAG {
				$$ = flg_f;
			}
		|	RFLAG {
				$$ = flg_r;
			}
		|	SSFLAG {
				$$ = flg_S;
			}
		|	NFLAG {
				$$ = flg_n;
			}
		|	LFLAG {
				$$ = flg_l;
			}
		|	SFLAG {
				$$ = flg_s;
			}
		|	MFLAG {
				$$ = flg_m;
			}
		|	FFFLAG {
				$$ = flg_F;
			}
		|	DDFLAG {
				$$ = flg_D;
			}
		|	MMFLAG {
				$$ = flg_M;
			}
		|	XFLAG {
				$$ = flg_x;
			}
		|	PPFLAG {
				$$ = flg_P;
			}
		|	UFLAG {
				$$ = flg_u;
			}
		|	HFLAG {
				$$ = flg_h;
			}
		|	AAFLAG {
				$$ = flg_A;
			}
		|	UUFLAG {
				$$ = flg_U;
			}
		|	EFLAG {
				$$ = flg_e;
			}
		|	XXFLAG {
				$$ = flg_X;
			}
		|	LLFLAG {
				$$ = flg_L;
			}
		|	PFLAG {
				$$ = flg_p;
			}
		|	IIFLAG {
				$$ = flg_I;
			}
		|	CCFLAG {
				$$ = flg_C;
			}
		;

dheader		:	/* empty */ {
				$$ = NULL;
			}
		|	DEFINE dlist {
				$$ = $2;
			}
		|	error {
				$$ = NULL;
			}
		;

dheaders	:	/* empty */
		|	dheaders DEFINE dlist SEMI {
				EmitDef (def_header, (struct he *) NULL, Flaglist, $3);
			}
		|	error
		;

dlist		:	LPAREN strval COMMA mval RPAREN {
				$$ = ListAppend ($2, MacScan ($4), " ");
				free ($2);
				free ($4);
			}
		|	LPAREN error RPAREN {
				$$ = NULL;
			}
		;

mlist		:	/* empty */
		|	mlist IDENT LBRACE mdefs RBRACE SEMI {
				EmitDef (def_mailer, $2, $4, (char *) NULL);
			}
		|	mlist IDENT LBRACE RBRACE SEMI {
				EmitDef (def_mailer, $2, (char *) NULL, (char *) NULL);
			}
		|	error SEMI {
				yyerrok;
			}
		;

mdefs		:	mtdef {
				$$ = $1;
			}
		|	mdefs COMMA mtdef {
				$$ = ListAppend ($1, $3, ", ");
				free ($1);
				free ($3);
			}
		;	

mtdef		:	mvar ASGN mval {
				$$ = ListAppend (ConvMat ($1), MacScan ($3), "=");
				free ($3);
			}
		|	MFLAGS ASGN LBRACE mflags RBRACE {
				$$ = ListAppend (ConvMat (mat_flags), $4, "=");
			}
		|	MSENDER ASGN IDENT {
				$$ = ListAppend (ConvMat (mat_sender), CheckRS ($3), "=");
			}
		|	MRECIPIENT ASGN IDENT {
				$$ = ListAppend (ConvMat (mat_recipient), CheckRS ($3), "=");
			}
		|	error {
				$$ = NULL;
			}
		;

mvar		:	MPATH {
				$$ = mat_path;
			}
		|	MARGV {
				$$ = mat_argv;
			}
		|	MEOL {
				$$ = mat_eol;
			}
		|	MMAXSIZE {
				$$ = mat_maxsize;
			}
		;

rdef		:	/* empty */
		|	rdef IDENT { StartRuleset ($2); } rulelist
		;

rulelist	:	LBRACE ruledefs RBRACE {
				RMatch = FALSE;
			}
		|	error {
				RMatch = FALSE;
			}
		;

ruledefs	:	/* empty */ {
				RMatch = TRUE;
			}
		|	ruledefs IF LPAREN matchaddr RPAREN actionstmt {
				EmitDef (def_ruleset, (struct he *) NULL, 
					 ListAppend ($4, $6, "\t"), (char *) NULL);
			free ($4);
			free ($6);
			}
		|	error SEMI {
				yyerrok;
			}
		;

matchaddr	:	/* empty */ {
				$$ = NULL;
			}
		|	matchaddr matchtok {
				$$ = ListAppend ($1, $2, (char *) NULL);
				free ($1);
			}
		|	error {
				$$ = NULL;
			}
		;

matchtok	:	IDENT {
				$$ = GetField ($1);
			}
		|	anychar {
				*Cbuf = $1;
				$$ = ListAppend (Cbuf, (char *) NULL, (char *) NULL);
			}
		|	mval {
				$$ = MacScan ($1);
			}
		|	DOLLAR IDENT {
				Mbuf[1] = MakeMac ($2, ID_MACRO);
				$$ = ListAppend (Mbuf, (char *) NULL, (char *) NULL);
			}
		;

actionstmt	:	action LPAREN rwaddr RPAREN SEMI {
				$$ = ListAppend ($1, $3, (char *) NULL);
				free ($3);
			}
		|	RESOLVE LPAREN resolution RPAREN SEMI {
				$$ = $3;
			}
		|	error SEMI {
				$$ = NULL;
				yyerrok;
			}
		;

action		:	RETRY {
				$$ = NULL;
			}
		|	NEXT {
				$$ = "$:";
			}
		|	RETURN {
				$$ = "$@";
			}
		;

rwaddr		:	/* empty */ {
				$$ = NULL;
			}
		|	rwaddr rwtok {
				$$ = ListAppend ($1, $2, (char *) NULL);
				free ($1);
			}
		|	rwaddr IDENT LPAREN rwaddr RPAREN {
				$$ = ListAppend ($1, (Tsb = MakeRSCall ($2, $4)), (char *) NULL);
				free ($1);
				free ($4);
				free (Tsb);
			}
		|	error {
				$$ = NULL;
			}
		;

rwtok		:	anychar {
				*Cbuf = $1;
				$$ = ListAppend (Cbuf, (char *) NULL, (char *) NULL);
			}
		|	mval {
				$$ = MacScan ($1);
			}
		|	cantok {
				$$ = $1;
			}
		|	reftok {
				$$ = $1;
			}
		;

cantok		:	CANON LPAREN IDENT RPAREN {
				$$ = Bracket ($3->psb, TRUE);
				RemoveSymbol ($3);
			}
		|	CANON LPAREN SCONST RPAREN {
				$$ = Bracket (MacScan ($3), TRUE);
				free ($3);
			}
		|	CANON LPAREN reftok RPAREN {
				$$ = Bracket ($3, TRUE);
				free ($3);
			}
		;

reftok		:	DOLLAR IDENT {
				Mbuf[1] = MakeMac ($2, ID_MACRO);
				$$ = ListAppend (Mbuf, (char *) NULL, (char *) NULL);
			}
		|	DOLLAR ICONST {
				$$ = ListAppend (MakePosTok ($2), (char *) NULL, (char *) NULL);
			}
		;

anychar		:	SEPCHAR {
				$$ = $1;
			}
		|	COLON {
				$$ = ':';
			}
		|	STAR {
				$$ = '*';
			}
		|	SEMI {
				$$ = ';';
			}
		|	LBRACE {
				$$ = '{';
			}
		|	RBRACE {
				$$ = '}';
			}
		|	COMMA {
				$$ = ',';
			}
		|	ASGN {
				$$ = '=';
			}
		;

resolution	:	mailerspec COMMA route {
				$$ = ListAppend ($1, $3, (char *) NULL);
				free ($1);
				free ($3);
			}
		|	error {
				$$ = NULL;
			}
		;

mailerspec	:	MAILER LPAREN rword RPAREN {
				$$ = ListAppend ("$#", $3, (char *) NULL);
			}
		;

route		:	HOST LPAREN hword RPAREN COMMA userspec {
				$$ = ListAppend (Tsb = ListAppend ("$@", $3, (char *) NULL),
						 $6, (char *) NULL);
				free (Tsb);
				free ($6);
			}
		|	userspec {
				$$ = $1;
			}
		;

hword		:	hostid {
				$$ = $1;
			}
		|	HOSTNUM LPAREN reftok RPAREN {
				$$ = Bracket ($3, FALSE);
				free ($3);
			}
		;

hostid		:	/* empty */ {
				$$ = NULL;
			}
		|	hostid IDENT {
				$$ = ListAppend ($1, $2->psb, (char *) NULL);
				RemoveSymbol ($2);
				free ($1);
			}
		|	hostid rwtok {
				$$ = ListAppend ($1, $2, (char *) NULL);
				free ($1);
			}
		;

userspec	:	USER LPAREN rwaddr RPAREN {
				$$ = ListAppend ("$:", $3, (char *) NULL);
				free ($3);
			}
		;

rword		:	IDENT {
				$$ = CheckMailer ($1);
			}
		|	reftok {
				$$ = $1;
			}
		;

fdefs		:	/* empty */
		|	fdefs IDENT idlist COLON ftype SEMI {
				AssignType (ListAppend ($2->psb, $3, " "), $5);
				free ($3);
			}
		|	error SEMI {
				yyerrok;
			}
		;

idlist		:	/* empty */ {
				$$ = NULL;
			}
		|	idlist COMMA IDENT {
				$$ = ListAppend ($1, $3->psb, " ");
				free ($1);
			}
		;

ftype		:	MATCH LPAREN ICONST RPAREN cdef {
				$$ = ListAppend (MakeField ($3, $5, FALSE), 
				    		 (char *) NULL, (char *) NULL);
			}
		|	MATCH LPAREN ICONST STAR RPAREN {
				$$ = ListAppend (MakeField ($3, (struct he *) NULL, TRUE), 
						 (char *) NULL, (char *) NULL);
			}
		|	error {
				$$ = NULL;
			}
		;

cdef		:	/* empty */ {
				$$ = NULL;
			}
		|	IN IDENT {
				$$ = $2;
			}
		;
