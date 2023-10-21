%{
# include <ctype.h>
# include <stdio.h>
%}

	/*******************************************************\
	*							*
	*	X_reference program for YACC files		*
	*	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~		*
	*	Cathy Taylor,					*
	*	c/o Department of Computing,			*
	*	University of Lancaster,			*
	*	Bailrigg, Lancaster, England.			*
	*							*
	*	Date : Sat Sep 13 19:14:26 BST 1986		*
	*							*
	\*******************************************************/

	/***********************************************\
	*						*
	*	Yacc Input Syntax			*
	*	~~~~~~~~~~~~~~~~~			*
	*	Adapted from the document		*
	*	'YACC - Yet Another Compiler Compiler'	*
	*		by				*
	*	   S. C. Johnson			*
	*	   *************			*
	*						*
	*	Date: Tue Jul  1 02:40:18 BST 1986	*
	*						*
	\***********************************************/

%token	IDENTIFIER CHARACTER NUMBER PER PERCURL ACT
%token	LEFT RIGHT NONASSOC TOKEN PREC TYPE START UNION
%token	COLON SEMICOLON COMMA OR LESS GREATER

%start	spec

%%

spec
	:	defs PER rules tail
			{
				printf("\n\n");
				yyclearin;
				return(0);
			}
	;

tail
	:	/* empty */
	|	PER 
	;

defs
	:	/* empty */
	|	def_bk
	;

def_bk
	:	def
	|	def_bk def
	;

def
	:	START IDENTIFIER
	|	UNION ACT
	|	PERCURL
	|	rword tag nlist
	;

rword
	:	TOKEN
	|	LEFT
	|	RIGHT
	|	NONASSOC
	|	TYPE
	;

tag
	:	/* empty */
	|	LESS IDENTIFIER GREATER
	;

nlist
	:	nmno
	|	nlist opt_comma nmno
	;

opt_comma
	:	/* empty */
	|	COMMA
	;

nmno
	:	IDENTIFIER opt_num
	;

opt_num
	:	/* empty */
	|	NUMBER
	;
rules
	:	rule
	|	rules rule
	;

rule
	:	IDENTIFIER
		{
			yyaction(ON_C_IDENT,line,yytext);
		}
		COLON body SEMICOLON
	;

body
	:	body_block
	|	body OR body_block
	;

body_block
	:	/* empty */
	|	body_block body_entity
	;

body_entity
	:	opt_prec id_ent
	|	ACT
	;

id_ent
	:	IDENTIFIER
		{
			yyaction(ON_IDENT,line,yytext);
		}
	|	CHARACTER
	;

opt_prec
	:	/* empty */
	|	PREC
	;


%%

# include	<stdio.h>
# include	"lex.yy.c"
# include	"xref.line.h"

# define	ON_C_IDENT	000
# define	ON_IDENT	001

struct	llist {
			int	line;
		struct	llist	*next;
		} ;

struct	table {
			char	*symbol;
		struct	llist	*desc;
		struct	llist	*occ;
		struct	table	*left;
		struct	table	*right;
		} *table = NULL;

extern	char	*malloc();

char	*nocore[] = "No more core left";

yyerror(mess)
char	*mess;
{
	printf("\n\n\n\t%s\n");
}

char	*m_alloc(nbytes)
unsigned	nbytes;
{
	char	*p;

	if ((p=malloc(nbytes)) != NULL)
	    return(p);
	yyerror(nocore);
	exit(3);
}

struct	llist	*save_ln(ln,ptr)
	int	ln;
struct	llist	*ptr;
{
	struct	llist	*q;

	ptr -> line = ln;
	q = (struct llist *) m_alloc(sizeof(struct llist) + 1);
	q -> next = ptr;
	return(q);
}

char	*strsave(s)
char	*s;
{
	char	*p;

	p = m_alloc(strlen(s)+1);
	strcpy(p,s);
	return(p);
}

	char	*mess = "Case bound error";

struct	table	*save_symbol(action,ln,text,ptr)
	int	action;
	int	ln;
	char	*text;
struct	table	*ptr;
{
	if (ptr == NULL) /* create new definition */
	{
	    ptr = (struct table *) m_alloc(sizeof(struct table)+1);
	    ptr -> symbol = strsave(text);
	    ptr -> left = ptr -> right = NULL;
	    ptr -> desc = (struct llist *) m_alloc(sizeof(struct llist)+1);
	    ptr -> occ = (struct llist *) m_alloc(sizeof(struct llist)+1);
	}
	else if ((strcmp(ptr -> symbol,text)) < 0)
	{
	    ptr -> left = save_symbol(action,ln,text,ptr -> left);
	    return(ptr);
	}
	else if ((strcmp(ptr -> symbol,text)) > 0)
	{
	    ptr -> right = save_symbol(action,ln,text,ptr -> right);
	    return(ptr);
	}
	switch (action)
	{   case ON_C_IDENT :	/* Add to list of definition lines */
	    {
		ptr -> desc = save_ln(ln,ptr -> desc);
		return(ptr);
	    }
	    case ON_IDENT :	/* Add to list of occurance lines */
	    {
		ptr -> occ = save_ln(ln,ptr -> occ);
		return(ptr);
	    }
	    default :
	    {
		yyerror(mess);
		exit(3);
	    }
	}
}

yyaction (action,ln,text)
	int	action;
	int	ln;
	char	*text;
{
	table = save_symbol(action,ln,text,table);
}

list_lines(ptr,comment)
struct	llist	*ptr;
	char	*comment;
{
	fprintf(stdout,"%s%d",comment,ptr -> line);
	if (ptr -> next == NULL)
	{
	    fprintf(stdout," ; ");
	    return(0);
	}
	fprintf(stdout,", ");
	list_lines(ptr -> next,comment);
}

/*******************************\
*				*
*	Fprint strings.		*
*				*
\*******************************/

	char	pre_decl_list_mark[] = "";
	char	pre_occ_list_mark[] = " occurs at line(s) ";
	char	declared_at_mark[] = "*";
	char	occurs_at_mark[] = "";
	char	token_maybe[] = "is not declared - token?? ";
	char	start_maybe[] = "never occurs on rhs of rule - start rule? ";

list_xref(root)
struct	table	*root;
{
	if (root == NULL)
	    return(0);
	list_xref(root -> right);
/*
*	List info for current string.
*/
	{
	    fprintf(stdout,"\n' %s ' -\n\t\t\t",root -> symbol);
	    if (root -> desc -> next == NULL)
	        fprintf(stdout,"%s",token_maybe);
	    else
		list_lines(root -> desc -> next,declared_at_mark);
	    if (root -> occ -> next == NULL)
	        fprintf(stdout,", %s",start_maybe);
	    else
	    {
	        fprintf(stdout,"%s",pre_occ_list_mark);
	        list_lines(root -> occ -> next,occurs_at_mark);
	    }
	}

	list_xref(root -> left);
}

nline(ln)
int	ln;
{
	printf("%4d :\t",ln);
}

main ()
{

	line = 0; nline(++line);
	yyparse ();
	list_xref(table);
	fprintf(stdout,"\n\n\tEnd of X-ref\n\t~~~~~~~~~~~~\n");
}
