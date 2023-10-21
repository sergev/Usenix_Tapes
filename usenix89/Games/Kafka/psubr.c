
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Routines for the parser. These create and maintain the graph that the
 * parser is responsible for.
 */

#include "kafgraf.h"
#include <stdio.h>
#include <strings.h>

struct knode *n;
struct knode *hashtab[256];
struct karc *avarc;
int i, c;

/* Enter a new rule. */

newrule(name)
char *name;
{
	struct knode *prev;
	unsigned char hashind, phash();

	hashind = phash(name);
	if (hashtab[hashind]) {
		for (n = hashtab[hashind]; n != NULL; n = n->kn_nnt) {
			if (!strcmp(n->kn_nodename, name)) {
				while (n->kn_next)
					n = n->kn_next;
				n->kn_next = (struct knode *)
				    malloc(sizeof (struct knode));
				n = n->kn_next;
				goto gotit;
			}
			prev = n;
		}
		if (n == NULL) {
			n = prev->kn_nnt = (struct knode *) 
			    malloc(sizeof (struct knode));
		}
	} else {
		hashtab[hashind] = (struct knode *)
				  malloc(sizeof (struct knode));
		n = hashtab[hashind];
	}
gotit:
	strcpy(n->kn_nodename, name);
	n->kn_nnt = n->kn_next = NULL;
	n->kn_type = KTNTERM;
	n->kn_nodenumber = newnum2();
	avarc = n->kn_arc = 0;	/* Set n->kn_arc to the arc when you
				 * find one.
				 */
	/* The new node is now in place. Now to collect the arcs... */
}

/* Enter a computed terminal. */


docompterm()
{
	int fnum;

	n->kn_type = KTCOMP;
	n->kn_next = n->kn_nnt = NULL;
	n->kn_arc = NULL;
	fnum = newnum();
	n->kn_fnum = fnum;
	/* Now we have to read in the actual function 
	 * definition. This is kept in "kaf.text.c", along
	 * with the rest of the user-supplied C code.
	 */
	if (transcribe(fnum)) {
		fprintf(stderr, "Premature end of file.\n");
		exit(1);
	}
}

/* Enter a nonterminal in a rule. */

dononterm(name)
char *name;
{

	/* Found a non-terminal. Note that throughout
	 * this process n will point to the main functor
	 * and avarc will point the last used arc. Thus
	 * when the end is found avarc->narc must be set = NULL.
	 * (This will also occur immediately if the rule
	 * is determined to be a computed terminal.)
	 * Also references in arcs cannot be resolved 
	 * until all rules are in...
	 */
	if (avarc) {
		avarc->ka_narc = (struct karc *) malloc(sizeof (struct karc));
		avarc = avarc->ka_narc;
	} else {	/* This is the first one. */
		avarc = (struct karc *) malloc(sizeof (struct karc));
		n->kn_arc = avarc;
	}
	strcpy(avarc->ka_toname, name);
	avarc->ka_type = KTNTERM;
	avarc->ka_arcnumber = newnum2();
}


/* Enter a terminal. */

doterm(name)
char *name;
{ 

	/* Just set ka_toname to the name of the terminal and set ka_type
	 * to be KTTERM. 
	 */

	if (avarc) {
		avarc->ka_narc = (struct karc *) malloc(sizeof (struct karc));
		avarc = avarc->ka_narc;
	} else {	/* This is the first arc. */
		avarc = (struct karc *) malloc(sizeof (struct karc));
		n->kn_arc = avarc;
	}
	strcpy(avarc->ka_toname, name);
	avarc->ka_type = KTTERM;
	avarc->ka_arcnumber = newnum2();
}

/* Copy C code in defs section. */

copyccode()
{
	int c;
	extern FILE *textp;

	for (;;) {
		while ((c = input()) != '\n') {
			if (c == '\0') {
				fprintf(stderr, "Unexpected EOF.\n");
				exit(1);
			}
			putc(c, textp);
		}
		c = input();
		if (c == '%') {
/* { boo hiss */	if ((c = input()) == '}')
				return;
			putc('\n', textp);
			putc('%', textp);
			putc(c, textp);
			continue;
		}
		putc('\n', textp);
		unput(c);
	}
}

