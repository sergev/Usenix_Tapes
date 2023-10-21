/*
 * Copyright (c) 1984 by
 * Tektronix, Incorporated Beaverton, Oregon 97077
 * All rights reserved.
 *
 * Permission is hereby granted for personal, non-commercial
 * reproduction and use of this program, provided that this
 * notice is included in any copy.
 */

#ifdef RCSNAME
	static char *RCSNAME = "$Header: cparen.h,v 1.12 84/04/05 09:21:51 bradn Stable $";
#else

#define	TRUE	1
#define	FALSE	0

#ifdef VARS
char *cmd;			/* the name of this command (for error msgs) */
#else
extern char *cmd;
#endif

struct lexed {			/* one unit of the lex'ed input list */
	char *l_text;		/* text associated with this token	*/
	struct lexed *l_prev;	/* previous token in the list		*/
	struct lexed *l_next;	/* next token in the list		*/
	int l_val;		/* this token's value (yylval'ish)	*/
};

extern struct lexed *nxttok();	/* used by lex to add a token to the list */
extern struct lexed *instok();	/* used by everybody to insert tokens	*/

struct node {			/* a node of an expression subtree	*/
	int n_op;
	int n_flags;
#define	HASL	1		/* 'has a left child'			*/
#define	HASR	2		/* 'has a right child'			*/
#define	BINARY	(HASL | HASR)
#define	HASPAR	4		/* 'has enclosing parens'		*/
#define	hasl(np) ((np)->n_flags & HASL)
#define	hasr(np) ((np)->n_flags & HASR)
#define	haspar(np) ((np)->n_flags & HASPAR)
				/*
				 * n_ledge and n_redge are the left and right
				 * edges of the sublist of tokens associated
				 * with this node.
				 * If haspar() is true, these pointers refer
				 * to the left and right paren tokens in the
				 * input token list.
				 */
	struct lexed *n_ledge;
	struct lexed *n_redge;
};

extern struct node *bld();	/* used by yacc rules to return node info */

typedef union {
	int intval;
	struct lexed *lexval;
	struct node *nodep;
} YYSTYPE;
#ifndef PARSER			/* yacc generates the definition of yylval */
extern YYSTYPE yylval;
#endif

#define NIL	((struct node *) 0)
#ifdef VARS
struct node nonode;
#else
extern struct node nonode;
#endif
#define	NOP	(&nonode)	/* a no-op node pointer			*/

#ifdef VARS
struct lexed firsttok;		/* the left edge of the token list	*/
#else
extern struct lexed firsttok;
#endif
#ifdef VARS
struct lexed lasttok;		/* the right edge of the token list	*/
#else
extern struct lexed lasttok;
#endif

#endif
