/*
 * Copyright (c) 1984 by
 * Tektronix, Incorporated Beaverton, Oregon 97077
 * All rights reserved.
 *
 * Permission is hereby granted for personal, non-commercial
 * reproduction and use of this program, provided that this
 * notice is included in any copy.
 */

#define VARS		/* to declare all the variables in cparen.h	*/
#include <stdio.h>
#include "cparen.h"
#include "y.tab.h"

static char *rcsid = "$Header: cparen.c,v 1.10 84/04/05 09:21:42 bradn Stable $";

main(argc,argv)
int argc;
char **argv;
{
	char *tp, *ep, *cp;
	struct lexed *lp;	/* lex token pointer	*/	
	int atend;		/* a temp	*/
	char *rindex();


	if ((cmd = rindex(argv[0], '/'))) {
		cmd++;
	} else {
		cmd = argv[0];
	}

	/* scan the command line	*/

	tp = (char *) 0;
	while (++argv, --argc > 0) {
		cp = *argv;
		if (*cp == '-') {
			while (*++cp) {
				switch(*cp) {
				case 't':
					if (tp) {
fprintf(stderr, "%s: option -%c used more than once\n", cmd, *cp);
					    goto baduse;
					}
					if (++argv, --argc <= 0) {
fprintf(stderr, "%s: missing type list\n", cmd);
					    goto baduse;
					}
					tp = *argv;
					break;
				default:
fprintf(stderr, "%s: unknown option -%c\n", cmd, *cp);
					goto baduse;
				}
			}
		} else {
			fprintf(stderr, "%s: extra string `%s'\n", cmd, cp);
baduse:
			fprintf(stderr, "Usage:\n  %s [-t types]\n", cmd);
			exit(2);
		}
	}

	if (tp) {

		/* install whitespace-separated type names	*/

		atend = 0;
		while (!atend && *tp) {
			for (ep = tp;
			  *ep && *ep != ' ' && *ep != '\t' && *ep != '\n';
			  ep++)
				;
			atend = *ep == '\0';
			*ep = '\0';
			if (ep - tp > 0) {
				newtype(tp);
			}
			tp = ++ep;
		}
	}

	/* init the lexed token list	*/

	firsttok.l_text = "";
	firsttok.l_prev = (struct lexed *) 0;
	firsttok.l_next = &lasttok;
	firsttok.l_val = 0;
	lasttok.l_text = "";
	lasttok.l_prev = &firsttok;
	lasttok.l_next = (struct lexed *) 0;
	lasttok.l_val = 0;

	nonode.n_op = LIT;
	nonode.n_flags = 0;
	nonode.n_ledge = (struct lexed *) 0;
	nonode.n_redge = (struct lexed *) 0;


	if (yyparse() == 1) {
		exit(1);
	}

	for (lp = firsttok.l_next; lp != &lasttok; lp = lp->l_next) {
		fputs(lp->l_text, stdout);
	}

	exit(0);
}

consider(op, lc, rc)	/* consider putting parens around the children	*/
int op;			/* the parent operator				*/
struct node *lc;	/* the left child (or NIL)			*/
struct node *rc;	/* the right child (or NIL)			*/
{
	struct node **cpp;	/* child pointer-pointer (a temp)	*/
	struct node *cp;	/* child pointer			*/


	/* ignore children of the 'literal' operator	*/

	if (op == LIT) {
		return;
	}

	/* for each child ...	*/
	cpp = &lc;
	do {
		/* consider only non-null, non-dummy children	*/
		cp = *cpp;
		if (cp && cp != NOP) {

			if (haspar(cp)
			  || op == CAST && cpp == &lc
			  || cp->n_op == LIT
			  || cp->n_op == CAST && !hasr(cp)
			  || op == CM
			  || op == BRACK && cpp == &rc
			  || op == CALL  && cpp == &rc
			  || op == QUEST && cpp == &rc
			  || op == cp->n_op && (
				/*
				 * don't parenthesize things which
				 * might be rearranged in spite of it.
				 */
			         op == MUL
			      || op == PLUS
			      || op == AND
			      || op == ER
			      || op == OR
			    )
			  ) {
				/* do nothing	*/
			} else {
				/* add parentheses	*/
				cp->n_ledge =
				  instok(LP, "(", cp->n_ledge->l_prev);
				cp->n_redge =
				  instok(RP, ")", cp->n_redge);
				cp->n_flags |= HASPAR;
			}
		}

		/* move on to the next child	*/

		if (cpp == &lc) {
			cpp = &rc;
		} else {
			cpp = (struct node **) 0;
		}
	} while (cpp);
}
