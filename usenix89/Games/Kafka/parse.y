%{

/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * The Kafka parser. It is called once, and builds the
 * necessary data structures for the specification. 
 */

#include <stdio.h>
#include <strings.h>
#include "kafgraf.h"

char buf[BUFSIZ];
extern FILE *inp;
extern struct karc *avarc;

%}

%start spec

%token LBRACE RBRACE NONTERM TERM RULEOP SSEP KESC 
%token COPEN CCLOSE SEMI

%%

/* The following definition is much like the one for 
 * yacc itself described in the user's guide.
 */

spec	:	defs SSEP rules tail
	;

tail	:	/* Empty. */
	|	SSEP	{	/* Deal with program section here. */ 
			int n;
			extern FILE *textp;

			do {
				n = fread(buf, 1, BUFSIZ, inp);
				fwrite(buf, 1, n, textp);
			} while (n == BUFSIZ);
			}
	;

defs	:	/* Empty. */
	|	defs def
	;

def	:	KESC   { /* Deal with Kafka escape here. 
			  * (None yet implemented.) 
			  */ 
			}	
	|	COPEN	{ /* Copy C code here. */ 
			copyccode();
			}
	;

rules	:	/* Empty. */
	|	rules rule
	;

rule	:	NONTERM 
			{ 	/* Got the NT name. Create a node. */
			newrule($1);
			}
			RULEOP rtail SEMI
	;

rtail	:	LBRACE	{	/* A computed terminal. */
			docompterm();
			}
	|	stuff LBRACE 
			{	/* We have to copy the C code and clean up. */
			int fnum;
			extern struct knode *n;
			
			fnum = newnum();
			transcribe(fnum);
			n->kn_fnum = fnum;
			avarc->ka_narc = NULL;
			}
	|	stuff	{	/* Just clean up. */
			n->kn_fnum = 0;
			avarc->ka_narc = NULL;
			}
	;

stuff	:	/* Empty. */
	|	stuff thing
	;

thing	:	NONTERM {
			dononterm($1);
			}
	|	TERM
			{
			doterm($1);
			}
	;

%%

