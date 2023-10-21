
/* RCS Info: $Revision: $ on $Date: $
 *           $Source: $
 * Copyright (c) 1985 Wayne A. Christopher 
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Definitions for the nodes of directed graphs used by 
 * kafka. A set of rules for a given non-term is represented 
 * as a list of rule definitions. A rule definition is
 * represented as a list of arcs to other non-terminals or
 * arcs containing terminal strings, or in the case of 
 * computed terminals, a pointer to a function that returns
 * a pointer to the computed terminal (note that the function should
 * cast the pointer to an int before returning it if it doesn't
 * want complains from the compiler) .
 * Rules are tried in turn until the kn_func of one suceeds,
 * in which case either the rule is used or the value of the
 * kn_func is used as the terminal value, if the node being left
 * is of type KTCOMP. It is an error for none of the kn_funcs
 * to succeed. Note that knode is the first form of the graph
 * nodes, used when the specification is being parsed, and
 * kknode is the final form used in the actual program generated 
 * (likewise for karc and kcarc) .
 */

#define SSIZE	100	/* Max size of terminals and nonterms.  */

struct knode {
	int kn_type;		/* See below. */
	char kn_nodename[SSIZE];/* Terminal or non-term. */
	struct karc *kn_arc;	/* Pointer to list of arcs. */
	struct knode *kn_next;	/* Next rule for this non-term. */
	struct knode *kn_nnt;	/* Next non-term. */
	int kn_fnum;		/* The id number of the function. 0 if none. */
	int kn_nodenumber;	/* The (unique) id number of this node. */
};

/* This isn't too hot, but we are lazy... */

struct kknode {
	int kk_type;		/* See below. */
	char *kk_nodename;	/* Terminal or non-term. */
	struct kcarc *kk_arc;	/* Pointer to list of arcs. */
	struct kknode *kk_next;	/* Next rule for this non-term. */
	int (*kk_func)();	/* Pointer to associated function. */
	struct kknode *kk_link;	/* Overall node list. */
};

struct karc {
	struct karc *ka_narc;	/* Link. */
	struct knode *ka_to;	/* Thing pointed to. */
	char ka_toname[SSIZE];	/* Name of node pointed to. */
	int ka_type;		/* Type of thing pointed to. */
	int ka_arcnumber;	/* The unique id of this arc. */
};

struct kcarc {
	char *kc_toname;	/* Only terminal names. */
	struct kknode *kc_to;	/* Node pointed to. */
	struct kcarc *kc_narc;	/* Link. */
};

#define KTTERM	1	/* kn_arc and kn_next will be NULL. */
#define KTNTERM	2	/* Non-terminal. */
#define KTCOMP	3	/* Computed terminal. kn_arc->ka_func will
			 * be used to decide what the value of the
			 * terminal will be.
			 */

