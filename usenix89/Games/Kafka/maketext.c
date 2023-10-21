
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#include <stdio.h>
#include "kafgraf.h"
#include "defs.h"

struct kkelt kkstack[STACKSIZE];	/* The stack. */
struct kkelt *kksp;			/* The stack pointer. */
extern struct kknode *start;
extern struct kknode *nodelist;


/* This routine is the one that generates a string in the grammer,
 * beginning with the non-terminal s. Returns -1 if s is not the
 * name of a non-terminal.
 */

maketext(s)
char *s;
{
	struct kknode *stnode, *kknn;
	int kkcv;

	/* First find the start node. */

	/* dumpnlist();	Diagnostic. */

	for (stnode = nodelist; stnode != NULL; stnode = stnode->kk_link)
		if (!strcmp(stnode->kk_nodename, s))
			break;
	if (stnode == NULL) {
		/* Error condition. */
		return (-1);
	}

	/* Now to generate the text. The generator works like this: first
	 * select one of the rules for the given non-terminal to follow.
	 * If the non-terminal is actually a computed terminal, compute
	 * the value and output it. Otherwise, then push a pointer to the 
	 * it onto the stack along with a  pointer to the arc that we are 
	 * currently following and follow it. If it is a terminal arc, 
	 * output the terminal and return, otherwise repeat for the 
	 * new nt. When returning, pop the stack and go on to the next
	 * arc, or return if there is none.
	 */

	kksp = &kkstack[0];
	kksp->ke_node = stnode;
	kksp->ke_arc = stnode->kk_arc;
nextarc:
	if (kksp->ke_arc == NULL) {	/* We have reached the end of the 
					 * current rule.
					 */
		kksp--;
		if (kksp < &kkstack[0]) {	/* Done? */
			return (0);
		}
		/* There are more rules to go. */
		kksp->ke_arc = kksp->ke_arc->kc_narc;
		goto nextarc;
	}
	/* There are yet arcs to be followed in the current rule. */
	if (kksp->ke_arc->kc_to)	{  /* The arc is to a non-term. Follow it. */
		kknn = kksp->ke_arc->kc_to;
		if (kknn->kk_type == KTCOMP) {
			/* Compute the value of the terminal. Note that we
			 * don't bother declaring these functions to be
			 * char *.
			 */
			kkcv = (int) (*kknn->kk_func) ();
			if (kkcv == NULL) {	/* Doesn't like this rule. */
				kksp->ke_arc = kksp->ke_arc->kc_narc;
				goto nextarc;
			} else {	/* Cool. */
				kkoutput((char *) kkcv);
				kksp->ke_arc = kksp->ke_arc->kc_narc;
				goto nextarc;
			}
		}
		/* It must be a real non-terminal then. Now decide
		 * which rule to use.
		 */
		while (kknn) {
			if ((kknn->kk_func == NULL) || (*kknn->kk_func) () )
				break;
			kknn = kknn->kk_next;
		}
		if (kknn == NULL) {
			fprintf(stderr, "Error: no rules accepted for %s\n",
				kksp->ke_node->kk_nodename);
			exit(1);
		}
		kksp++;
		kksp->ke_node = kknn;
		kksp->ke_arc = kknn->kk_arc;
		goto nextarc;
	}
	/* It is a terminal. */
	kkoutput(kksp->ke_arc->kc_toname);
	kksp->ke_arc = kksp->ke_arc->kc_narc;
	goto nextarc;
}
