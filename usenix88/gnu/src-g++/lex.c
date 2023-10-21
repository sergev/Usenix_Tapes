/* Separate lexical analyzer for GNU C++.
   Copyright (C) 1987 Free Software Foundation, Inc.
   Hacked by Michael Tiemann (tiemann@mcc.com)

This file is part of GNU CC.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU CC General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU CC, but only under the conditions described in the
GNU CC General Public License.   A copy of this license is
supposed to have been given to you along with GNU CC so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


/* This file is the lexical analyzer for GNU C++.  */

#include <stdio.h>
#include "config.h"
#include "tree.h"
#include "parse.tab.h"
#include "parse.h"
#include "c-tree.h"
#include "flags.h"
#include "obstack.h"

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free

extern int xmalloc ();
extern void free ();

extern double atof ();

/* This obstack is needed to hold text.  It is not safe to use
   TOKEN_BUFFER because `check_newline' calls `yylex'.  */
static struct obstack inline_text_obstack;
static char *inline_text_firstobj;

#define YYEMPTY		-2
int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/

/* the declaration found for the last IDENTIFIER token read in.
   yylex must look this up to detect typedefs, which get token type TYPENAME,
   so it is left around in case the identifier is not a typedef but is
   used in a context which makes it a reference to a variable.  */
tree lastiddecl;

/* C++ extensions */
tree ridpointers[];		/* need this up here */

/* Return something to represent absolute declarators containing a *.
   TARGET is the absolute declarator that the * contains.
   TYPE_QUALS is a list of modifiers such as const or volatile
   to apply to the pointer type, represented as identifiers.

   We return an INDIRECT_REF whose "contents" are TARGET
   and whose type is the modifier list.  */
   
tree
make_pointer_declarator (type_quals, target)
     tree type_quals, target;
{
  return build (INDIRECT_REF, type_quals, target);
}

/* Return something to represent absolute declarators containing a &.
   TARGET is the absolute declarator that the & contains.
   TYPE_QUALS is a list of modifiers such as const or volatile
   to apply to the reference type, represented as identifiers.

   We return an ADDR_EXPR whose "contents" are TARGET
   and whose type is the modifier list.  */
   
tree
make_reference_declarator (type_quals, target)
     tree type_quals, target;
{
  if (target && TREE_CODE (target) == ADDR_EXPR)
    {
      error ("cannot declare references to references");
      return target;
    }
  if (target && TREE_CODE (target) == INDIRECT_REF)
    {
      error ("cannot declare pointers to references");
      return target;
    }
  return build (ADDR_EXPR, type_quals, target);
}

/* Given a chain of STRING_CST nodes,
   concatenate them into one STRING_CST
   and give it a suitable array-of-chars data type.  */

tree
combine_strings (strings)
     tree strings;
{
  register tree value, t;
  register int length = 1;
  int wide_length = 0;
  int wide_flag = 0;

  if (TREE_CHAIN (strings))
    {
      /* More than one in the chain, so concatenate.  */
      register char *p, *q;

      /* Don't include the \0 at the end of each substring,
	 except for the last one.
	 Count wide strings and ordinary strings separately.  */
      for (t = strings; t; t = TREE_CHAIN (t))
	{
	  if (TREE_TYPE (t) == int_array_type_node)
	    {
	      wide_length += (TREE_STRING_LENGTH (t) - 1);
	      wide_flag = 1;
	    }
	  else
	    length += (TREE_STRING_LENGTH (t) - 1);
	}

      /* If anything is wide, the non-wides will be converted,
	 which makes them take more space.  */
      if (wide_flag)
	length = length * UNITS_PER_WORD + wide_length;


      p = (char *) oballoc (length);

      /* Copy the individual strings into the new combined string.
	 If the combined string is wide, convert the chars to ints
	 for any individual strings that are not wide.  */

      q = p;
      for (t = strings; t; t = TREE_CHAIN (t))
	{
	  int len = TREE_STRING_LENGTH (t) - 1;
	  if ((TREE_TYPE (t) == int_array_type_node) == wide_flag)
	    {
	      bcopy (TREE_STRING_POINTER (t), q, len);
	      q += len;
	    }
	  else
	    {
	      int i;
	      for (i = 0; i < len; i++)
		((int *) q)[i] = TREE_STRING_POINTER (t)[i];
	      q += len * UNITS_PER_WORD;
	    }
	}

      *q = 0;

      value = make_node (STRING_CST);
      TREE_STRING_POINTER (value) = p;
      TREE_STRING_LENGTH (value) = length;
      TREE_LITERAL (value) = 1;
    }
  else
    {
      value = strings;
      length = TREE_STRING_LENGTH (value);
      if (TREE_TYPE (value) == int_array_type_node)
	wide_flag = 1;
    }

  TREE_TYPE (value)
    = build_array_type (wide_flag ? integer_type_node : char_type_node,
			make_index_type (build_int_2 (length - 1, 0)));
  TREE_LITERAL (value) = 1;
  TREE_STATIC (value) = 1;
  return value;
}

/* Given a TOKEN and its estimated tree code CODE, produce a name which
   can be recognized by lookup_name.  Based on the number of PARMS,
   build an appropriate operator fnname.  This function is needed because
   until we know how many parameters we have, we cannot reliably tell
   what function indeed we are trying to declare.

   NPARMS is the number of additional parameters that this operator
   will ultimately have.  If NPARMS == -1, then we are just building
   a name, and should not complain.

   This would be a good candidate for memoizing.  */
tree
build_operator_fnname (decl, parms, nparms)
     tree decl;
     tree parms;
     int nparms;
{
  extern char *tree_code_name[]; /* print-tree.c */
  extern char *tree_code_err_name[]; /* print-tree.c */
  extern char *tree_code_err_asg_name[]; /* print-tree.c */

  tree rval;
  int token = (int)TREE_OPERAND (decl, 0);
  enum tree_code code = (enum tree_code)TREE_OPERAND (decl, 1);
  char buf[1024];
  int saw_class = nparms;

  while (parms)
    {
      tree type;
      if (TREE_VALUE (parms) == void_type_node)
	break;

      if (! saw_class)
	{
	  type = TREE_VALUE (parms);
	  if (TREE_CODE (type) == REFERENCE_TYPE)
	    type = TREE_TYPE (type);
	  if (TREE_CODE (type) == POINTER_TYPE)
	    type = TREE_TYPE (type);
	  if (IS_AGGR_TYPE (type))
	    saw_class++;
	}
      nparms++;
      parms = TREE_CHAIN (parms);
    }

  if (TREE_CODE (decl) == TYPE_EXPR)
    {
      /* @@ may need to perform type instantiation here.  */
      if (nparms > 1)
	error ("wrong number of arguments to type conversion operator");

      /* The grammar will swallow an "()" if one was given.
	 We attempt to correct for this lossage here.  */
      if (TREE_OPERAND (decl, 1)
	  && TREE_CODE (TREE_OPERAND (decl, 1)) == CALL_EXPR)
	{
	  extern FILE *finput;
	  rval = do_typename_overload (groktypename (build_tree_list (TREE_OPERAND (decl, 0), NULL_TREE)));
	  yychar = '(';
	  ungetc (')', finput);
	}
      else
	{
	  rval = do_typename_overload (groktypename (build_tree_list (TREE_OPERAND (decl, 0), TREE_OPERAND (decl, 1))));
	}
      return rval;
    }
  else switch (token)
    {
      /* what ever it was, we know it is correct */
    case 0:
      break;

      /* AC/DC */
    case '+':
      if (nparms == 1)
	code = CONVERT_EXPR;
      else if (nparms == 2)
	code = PLUS_EXPR;
      else
	{
	  error ("wrong number of parameters to `operator %c'", token);
	  code = PLUS_EXPR;
	}
      break;
    case '&':
      if (nparms == 1)
	code = ADDR_EXPR;
      else if (nparms == 2)
	code = BIT_AND_EXPR;
      else
	{
	  code = BIT_AND_EXPR;
	  error ("wrong number of parameters to `operator %c'", token);
	}
      break;
    case '*':
      if (nparms == 1)
	code = INDIRECT_REF;
      else if (nparms == 2)
	code = MULT_EXPR;
      else
	{
	  code = MULT_EXPR;
	  error ("wrong number of parameters to `operator %c'", token);
	}
      break;
    case '-':
      if (nparms == 1)
	code = NEGATE_EXPR;
      else if (nparms == 2)
	code = MINUS_EXPR;
      else
	{
	  code = MINUS_EXPR;
	  error ("wrong number of parameters to `operator %c'", token);
	}
      break;

    case POINTSAT:
      if (nparms == 1 || nparms < 0)
	{
	  code = COMPONENT_REF;
	}
      else
	{
	  error ("wrong number of parameters to `operator ->()'");
	}
      break;

    case METHOD_REF:
      switch (nparms)
	{
	case 0:
	case 1:
	  error ("too few arguments to `operator ->()(...)'");
	  break;
	case -1:
	case 2:
	case 3:
	  break;
	default:
	  error ("too few arguments to `operator ->()(...)'");
	  break;
	}
      break;

    case ASSIGN:
    case '=':
      if (nparms != 2 && nparms >= 0)
	{
	  if (code == NOP_EXPR)
	    error ("wrong number of parameters to `operator %s'",
		   tree_code_err_name [(int) MODIFY_EXPR]);
	  else
	    error ("wrong number of parameters to `operator %s'",
		   tree_code_err_asg_name [(int) code]);
	}
      if (code == NOP_EXPR)
	sprintf (buf, OPERATOR_FORMAT, tree_code_name [(int) MODIFY_EXPR]);
      else
	sprintf (buf, OPERATOR_ASSIGN_FORMAT, tree_code_name [(int) code]);
      rval = get_identifier (buf);
      TREE_OVERLOADED (rval) = 1;
      return rval;

    default:
      fatal ("don't know token type");
    }

  if (! saw_class)
    {
      error ("`operator %s' must have at least one class type",
	     tree_code_err_name [(int) code]);
    }

  sprintf (buf, OPERATOR_FORMAT, tree_code_name [(int) code]);
  rval = get_identifier (buf);
  TREE_OVERLOADED (rval) = 1;
  return rval;
}

char *
operator_name_string (name)
     tree name;
{
  extern char *tree_code_name[]; /* print-tree.c */
  extern char *tree_code_err_name[]; /* print-tree.c */
  extern char *tree_code_err_asg_name[]; /* print-tree.c */
  char *opname = IDENTIFIER_POINTER (name)
    + sizeof (OPERATOR_FORMAT) - sizeof ("%s");
  int i, assign;

  /* Works for builtin and user defined types.  */
  if (IDENTIFIER_GLOBAL_VALUE (name)
      && TREE_CODE (IDENTIFIER_GLOBAL_VALUE (name)) == TYPE_DECL)
    return IDENTIFIER_POINTER (name);

  if (! strncmp (opname, "assign", 6))
    {
      opname += 7;
      assign = 1;
    }
  else
    assign = 0;

  for (i = (int)COMPONENT_REF; i < (int)END_OF_TREE_CODES; i++)
    {
      if (! strncmp (opname, tree_code_name[i], strlen (tree_code_name[i])))
	break;
    }

  if (i == (int)END_OF_TREE_CODES)
    abort ();

  if (assign)
    return tree_code_err_asg_name[i];
  else
    return tree_code_err_name[i];
}

int lineno;			/* current line number in file being read */

FILE *finput;			/* input file.
				   Normally a pipe from the preprocessor.  */
static FILE *finput1;		/* Real input files: 1 is main input file */
static FILE *finput2;		/* 2 is input file for inline functions */

/* lexical analyzer */

static int maxtoken;		/* Current nominal length of token buffer */
char *token_buffer;		/* Pointer to token buffer.
				 Actual allocated length is maxtoken + 2.  */

static dollar_seen = 0;		/* Nonzero if have warned about `$'.  */

#define MAXRESERVED 9

/* frw[I] is index in `reswords' of the first word whose length is I;
   frw[I+1] is one plus the index of the last word whose length is I.
   The length of frw must be MAXRESERVED + 2 so there is an element
   at MAXRESERVED+1 for the case I == MAXRESERVED.  */

static char frw[MAXRESERVED+2] =
/*  { 0, 0, 0, 2, 5, 13, 19, 29, 31, 35 }; non-extended */
  { 0, 0, 0, 2, 6, 15, 22, 35, 40, 46, 48, };

/* Table of reserved words.  */

struct resword { char *name; short token; enum rid rid;};

#define NORID RID_UNUSED

/* If this is not in lexicographical order, you will lose.  */
static struct resword reswords[]
  = {{"do", DO, NORID},
     {"if", IF, NORID},
     {"asm", ASM, NORID},
     {"for", FOR, NORID},
     {"int", TYPESPEC, RID_INT},
     {"new", NEW, NORID},
     {"auto", SCSPEC, RID_AUTO},
     {"case", CASE, NORID},
     {"char", TYPESPEC, RID_CHAR},
     {"else", ELSE, NORID},
     {"enum", ENUM, NORID},
     {"goto", GOTO, NORID},
     {"long", TYPESPEC, RID_LONG},
     {"this", THIS, NORID},
     {"void", TYPESPEC, RID_VOID},
     {"break", BREAK, NORID},
     {"class", AGGR, RID_CLASS},
     {"const", TYPE_QUAL, RID_CONST},
     {"float", TYPESPEC, RID_FLOAT},
     {"short", TYPESPEC, RID_SHORT},
     {"union", AGGR, RID_UNION},
     {"while", WHILE, NORID},
     {"delete", DELETE, NORID},
     {"double", TYPESPEC, RID_DOUBLE},
     {"extern", SCSPEC, RID_EXTERN},
     {"friend", TYPE_QUAL, RID_FRIEND},
     {"inline", SCSPEC, RID_INLINE},
     {"public", PUBLIC, NORID},
     {"return", RETURN, NORID},
     {"signed", TYPESPEC, RID_SIGNED},
     {"sizeof", SIZEOF, NORID},
     {"static", SCSPEC, RID_STATIC},
     {"struct", AGGR, RID_RECORD},
     {"switch", SWITCH, NORID},
     {"typeof", TYPEOF, NORID},
     {"default", DEFAULT, NORID},
     {"noalias", TYPE_QUAL, RID_NOALIAS},
     {"private", PRIVATE, NORID},
     {"typedef", SCSPEC, RID_TYPEDEF},
     {"virtual", SCSPEC, RID_VIRTUAL},
     {"continue", CONTINUE, NORID},
     {"operator", OPERATOR, NORID},
     {"overload", OVERLOAD, NORID},
     {"register", SCSPEC, RID_REGISTER},
     {"unsigned", TYPESPEC, RID_UNSIGNED},
     {"volatile", TYPE_QUAL, RID_VOLATILE},
     {"__alignof", ALIGNOF, NORID},
     {"protected", PROTECTED, NORID}};

/* The elements of `ridpointers' are identifier nodes
   for the reserved type names and storage classes.
   It is indexed by a RID_... value.  */

tree ridpointers[(int) RID_MAX];

static tree line_identifier;   /* The identifier node named "line" */

void check_newline ();

void
init_lex ()
{
  extern char *malloc ();

  obstack_init (&inline_text_obstack);
  inline_text_firstobj = (char *) obstack_alloc (&inline_text_obstack, 0);

  /* Start it at 0, because check_newline is called at the very beginning
     and will increment it to 1.  */
  finput1 = finput;
  finput2 = fopen ("/dev/null", "r");
  lineno = 0;
  current_function_decl = NULL;
  current_function_name = NULL;
  line_identifier = get_identifier ("line");

  maxtoken = 40;
  token_buffer = malloc (maxtoken + 2);
  ridpointers[(int) RID_INT] = get_identifier ("int");
  ridpointers[(int) RID_CHAR] = get_identifier ("char");
  ridpointers[(int) RID_VOID] = get_identifier ("void");
  ridpointers[(int) RID_FLOAT] = get_identifier ("float");
  ridpointers[(int) RID_DOUBLE] = get_identifier ("double");
  ridpointers[(int) RID_SHORT] = get_identifier ("short");
  ridpointers[(int) RID_LONG] = get_identifier ("long");
  ridpointers[(int) RID_UNSIGNED] = get_identifier ("unsigned");
  ridpointers[(int) RID_SIGNED] = get_identifier ("signed");
  ridpointers[(int) RID_CONST] = get_identifier ("const");
  ridpointers[(int) RID_VOLATILE] = get_identifier ("volatile");
  ridpointers[(int) RID_AUTO] = get_identifier ("auto");
  ridpointers[(int) RID_STATIC] = get_identifier ("static");
  ridpointers[(int) RID_EXTERN] = get_identifier ("extern");
  ridpointers[(int) RID_TYPEDEF] = get_identifier ("typedef");
  ridpointers[(int) RID_REGISTER] = get_identifier ("register");
  ridpointers[(int) RID_NOALIAS] = get_identifier ("noalias");

  /* C++ extensions. These are probably not correctly named. */
  class_type_node = make_node (CLASS_TYPE);
  TREE_TYPE (class_type_node) = class_type_node;
  ridpointers[(int) RID_CLASS] = class_type_node;

  record_type_node = make_node (RECORD_TYPE);
  TREE_TYPE (record_type_node) = record_type_node;
  ridpointers[(int) RID_RECORD] = record_type_node;

  union_type_node = make_node (UNION_TYPE);
  TREE_TYPE (union_type_node) = union_type_node;
  ridpointers[(int) RID_UNION] = union_type_node;

  enum_type_node = make_node (ENUMERAL_TYPE);
  TREE_TYPE (enum_type_node) = enum_type_node;
  ridpointers[(int) RID_ENUM] = enum_type_node;

  ridpointers[(int) RID_INLINE] = get_identifier ("inline");
  ridpointers[(int) RID_VIRTUAL] = get_identifier ("virtual");
  ridpointers[(int) RID_FRIEND] = get_identifier ("friend");
}

void
reinit_parse_for_function ()
{
  current_base_init_list = NULL_TREE;
  current_member_init_list = NULL_TREE;
}

/* Called from the top level: if there are any pending inlines to
   do, then do them now.  */
void
do_pending_inlines ()
{
  if (finput == finput1)
    {
      struct pending_inline *t =
	(struct pending_inline *) obstack_alloc (&inline_text_obstack,
						 sizeof (struct pending_inline));
      struct pending_inline *tail = pending_inlines;

      t->next = NULL;
      t->lineno = lineno;
      t->filename = input_filename;
      t->fndecl = NULL_TREE;
      t->token = yychar;
      t->token_value = yylval.itype;
      while (tail->next) tail = tail->next;
      tail->next = t;
      t = pending_inlines;
      pending_inlines = pending_inlines->next;
      finput = finput2;
#ifndef hp9000s300
      setbuffer (finput2, t->buf, t->len);
      finput2->_cnt = finput2->_bufsiz - 1;
#else
      setvbuf(finput2,t->buf,_IOFBF,t->len);
      finput2->_cnt = t->len-1;
#endif
      lineno = t->lineno;
      input_filename = t->filename;
#ifdef DO_INLINES_THE_OLD_WAY
      yychar = yylex ();
#else
      yychar = PRE_PARSED_FUNCTION_DECL;
      yylval.ttype = t->fndecl;
#endif
    }
}

/* Since inline methods can refer to text which has not yet been seen,
   we store the text of the method in the DECL_INITIAL of the
   FUNCTION_DECL.  After parsing the body of the class definition,
   the FUNCTION_DECL's are scanned to see which ones have this field set.
   Those are then digested one at a time.  When we get smarter we will
   be able to do inline function expansion, but not for now.

   This function's FUNCTION_DECL will have an IDENTIFIER_NODE
   as its DECL_INITIAL, and a bit set in its common so that we know
   to watch out for it.  */

void
consume_string (this_obstack)
     register struct obstack *this_obstack;
{
  register char c;
  do
    {
      c = getc (finput);
      if (c == '\\')
	{
	  obstack_1grow (this_obstack, c);
	  c = getc (finput);
	  obstack_1grow (this_obstack, c);
	  continue;
	}
      if (c == '\n')
	{
	  if (pedantic)
	    warning ("ANSI C forbids newline in string constant");
	  lineno++;
	}
      obstack_1grow (this_obstack, c);
    }
  while (c != '\"');
}

void
reinit_parse_for_method (yychar)
     int yychar;
{
  register char c = 0;
  int blev = 1;
  int starting_lineno;
  char *inline_text_currentobj = (char *)obstack_alloc (&inline_text_obstack, 0);
  int len;

  starting_lineno = lineno;
  if (yychar != '{')
    {
      if (yychar != ':')
	yychar = '{';
      obstack_1grow (&inline_text_obstack, yychar);
      while (c >= 0)
	{
	  int this_lineno = lineno;

	  c = skip_white_space ();

	  /* Don't lose our cool if there are lots of comments.  */
	  if (lineno - this_lineno)
	    if (lineno - this_lineno == 1)
	      obstack_1grow (&inline_text_obstack, '\n');
	    else
	      {
		char buf[12];
		sprintf (buf, "\n# %d \"", lineno);
		len = strlen (buf);
		obstack_grow (&inline_text_obstack, buf, len);

		len = strlen (input_filename);
		obstack_grow (&inline_text_obstack, input_filename, len);
		obstack_1grow (&inline_text_obstack, '\"');
		obstack_1grow (&inline_text_obstack, '\n');
	      }

	  /* strings must be read differently that text.  */
	  if (c == '\"')
	    {
	      obstack_1grow (&inline_text_obstack, c);
	      consume_string (&inline_text_obstack);
	    }
	  else while (c > ' ')	/* ASCII dependent! */
	    {
	      obstack_1grow (&inline_text_obstack, c);
	      if (c == '{') goto main_loop;
	      if (c == '\"')
		consume_string (&inline_text_obstack);
	      c = getc (finput);
	    }
	  if (c == '\n')
	    lineno++;
	  obstack_1grow (&inline_text_obstack, c);
	}
    }
  else obstack_1grow (&inline_text_obstack, '{');

 main_loop:
  while (c >= 0)
    {
      int this_lineno = lineno;

      c = skip_white_space ();

      /* Don't lose our cool if there are lots of comments.  */
      if (lineno - this_lineno)
	if (lineno - this_lineno == 1)
	  obstack_1grow (&inline_text_obstack, '\n');
	else
	  {
	    char buf[12];
	    sprintf (buf, "\n# %d \"", lineno);
	    len = strlen (buf);
	    obstack_grow (&inline_text_obstack, buf, len);

	    len = strlen (input_filename);
	    obstack_grow (&inline_text_obstack, input_filename, len);
	    obstack_1grow (&inline_text_obstack, '\"');
	    obstack_1grow (&inline_text_obstack, '\n');
	  }

      while (c > ' ')
	{
	  obstack_1grow (&inline_text_obstack, c);
	  if (c == '{') blev++;
	  else if (c == '}')
	    {
	      blev--;
	      if (blev == 0)
		goto done;
	    }
	  else if (c == '\"')
	    consume_string (&inline_text_obstack);
	  c = getc(finput);
	}
      if (c == '\n')
	lineno++;
      obstack_1grow (&inline_text_obstack, c);
    }
 done:
  obstack_1grow (&inline_text_obstack, '\0');
  current_base_init_list = NULL_TREE;
  current_member_init_list = NULL_TREE;

  len = obstack_object_size (&inline_text_obstack);
    
  {
    struct pending_inline *t;
    /* `inline_text_currentobj' may no longer point to the real
       base of the stuff we are growing.  */
    char *buf = obstack_base (&inline_text_obstack);

    obstack_finish (&inline_text_obstack);

    t = (struct pending_inline *) obstack_alloc (&inline_text_obstack,
						 sizeof (struct pending_inline));
    t->buf = buf;
    t->len = len;
    t->lineno = starting_lineno;
    t->filename = input_filename;
    t->token = YYEMPTY;
    DECL_INITIAL (current_function_decl) = (tree)t;
    /* We make this declaration private (static in the C sense).  */
    TREE_PUBLIC (current_function_decl) = 0;
  }
}

static int
skip_white_space ()
{
  register int c;
  register int inside;

  c = getc (finput);

  for (;;)
    {
      switch (c)
	{
	case '/':
	  c = getc (finput);
	  if (c != '*' && c != '/')
	    {
	      ungetc (c, finput);
	      return '/';
	    }

	  if (c == '/')
	    {
	      while (c != EOF)
		{
		  c = getc (finput);
		  if (c == '\n')
		    {
		      ungetc (c, finput);
		      break;
		    }
		}
	      if (c == EOF)
		{
		  error ("unterminated comment");
		  return EOF;
		}
	      c = getc (finput);
	      break;
	    }

	  c = getc (finput);

	  inside = 1;
	  while (inside)
	    {
	      if (c == '*')
		{
		  while (c == '*')
		    c = getc (finput);

		  if (c == '/')
		    {
		      inside = 0;
		      c = getc (finput);
		    }
		}
	      else if (c == '\n')
		{
		  lineno++;
		  c = getc (finput);
		}
	      else if (c == EOF)
		error ("unterminated comment");
	      else
		c = getc (finput);
	    }

	  break;

	case '\n':
	  check_newline ();

	case ' ':
	case '\t':
	case '\f':
	case '\r':
	case '\b':
	  c = getc (finput);
	  break;

	case '\\':
	  c = getc (finput);
	  if (c == '\n')
	    lineno++;
	  else
	    error ("stray '\\' in program");
	  c = getc (finput);
	  break;

	default:
	  return (c);
	}
    }
}



/* Make the token buffer longer, preserving the data in it.
   P should point to just beyond the last valid character in the old buffer.
   The value we return is a pointer to the new buffer
   at a place corresponding to P.  */

static char *
extend_token_buffer (p)
     char *p;
{
  int offset = p - token_buffer;

  maxtoken = maxtoken * 2 + 10;
  token_buffer = (char *) realloc (token_buffer, maxtoken + 2);
  if (token_buffer == 0)
    fatal ("virtual memory exceeded");

  return token_buffer + offset;
}

/* At the beginning of a line, increment the line number
   and handle a #line directive immediately following  */

void
check_newline ()
{
  register int c;
  register int token;

  while (1)
    {
      lineno++;

      /* Read first nonwhite char on the line.  */

      c = getc (finput);
      if (c != '#')
	{
	  /* If no #, unread the character,
	     except don't bother if it is whitespace.  */
	  if (c == ' ' || c == '\t')
	    return;
	  ungetc (c, finput);
	  return;
	}

      c = getc (finput);

      /* Skip whitespace.  */

      while (1)
	{
	  if (! (c == ' ' || c == '\t'))
	    break;
	  c = getc (finput);
	}

      /* If a letter follows, then if the word here is `line', skip
	 it and ignore it; otherwise, ignore the line, with an error
	 if the word isn't `pragma'.  */

      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
	{
	  if (c == 'p')
	    {
	      if (getc (finput) == 'r'
		  && getc (finput) == 'a'
		  && getc (finput) == 'g'
		  && getc (finput) == 'm'
		  && getc (finput) == 'a'
		  && ((c = getc (finput)) == ' ' || c == '\t'))
		goto noerror;
	    }

	  else if (c == 'l')
	    {
	      if (getc (finput) == 'i'
		  && getc (finput) == 'n'
		  && getc (finput) == 'e'
		  && ((c = getc (finput)) == ' ' || c == '\t'))
		goto linenum;
	    }

	  error ("undefined or invalid # directive");
	noerror:

	  while ((c = getc (finput)) && c >= 0 && c != '\n');

	  continue;
	}

    linenum:

      /* Skip whitespace.  */

      while (1)
	{
	  if (! (c == ' ' || c == '\t'))
	    break;
	  c = getc (finput);
	}

      /* If the # is the only nonwhite char on the line,
	 just ignore it.  Check the new newline.  */
      if (c == '\n')
	continue;

      /* Something follows the #; read a token.  */

      ungetc (c, finput);
      token = yylex ();

      if (token == CONSTANT
	  && TREE_CODE (yylval.ttype) == INTEGER_CST)
	{
	  /* subtract one, because it is the following line that
	     gets the specified number */

	  int l = TREE_INT_CST_LOW (yylval.ttype) - 1;

	  /* Is this the last nonwhite stuff on the line?  */
	  c = getc (finput);
	  while (c == ' ' || c == '\t')
	    c = getc (finput);
	  if (c == '\n')
	    {
	      /* No more: store the line number and check following line.  */
	      lineno = l;
	      continue;
	    }
	  ungetc (c, finput);

	  /* More follows: it must be a string constant (filename).  */

	  token = yylex ();
	  if (token != STRING || TREE_CODE (yylval.ttype) != STRING_CST)
	    {
	      error ("invalid #line");
	      return;
	    }

	  input_filename
	    = (char *) permalloc (TREE_STRING_LENGTH (yylval.ttype) + 1);
	  strcpy (input_filename, TREE_STRING_POINTER (yylval.ttype));
	  lineno = l;

	  if (main_input_filename == 0)
	    main_input_filename = input_filename;
	}
      else
	error ("undefined or invalid # directive");

      /* skip the rest of this line.  */
      while ((c = getc (finput)) != '\n' && c >= 0);
    }
}



#define isalnum(char) (char >= 'a' ? char <= 'z' : char >= '0' ? char <= '9' || (char >= 'A' && char <= 'Z') : 0)
#define isdigit(char) (char >= '0' && char <= '9')
#define ENDFILE -1  /* token that represents end-of-file */


static int
readescape ()
{
  register int c = getc (finput);
  register int count, code;

  switch (c)
    {
    case 'x':
      code = 0;
      count = 0;
      while (1)
	{
	  c = getc (finput);
	  if (!(c >= 'a' && c <= 'f')
	      && !(c >= 'A' && c <= 'F')
	      && !(c >= '0' && c <= '9'))
	    {
	      ungetc (c, finput);
	      break;
	    }
	  code *= 16;
	  if (c >= 'a' && c <= 'f')
	    code += c - 'a' + 10;
	  if (c >= 'A' && c <= 'F')
	    code += c - 'A' + 10;
	  if (c >= '0' && c <= '9')
	    code += c - '0';
	  count++;
	}
      if (count == 0)
	error ("\\x used with no following hex digits");
      return code;

    case '0':  case '1':  case '2':  case '3':  case '4':
    case '5':  case '6':  case '7':
      code = 0;
      count = 0;
      while ((c <= '7') && (c >= '0') && (count++ < 3))
	{
	  code = (code * 8) + (c - '0');
	  c = getc (finput);
	}
      ungetc (c, finput);
      return code;

    case '\\': case '\'': case '"':
      return c;

    case '\n':
      lineno++;
      return -1;

    case 'n':
      return TARGET_NEWLINE;

    case 't':
      return TARGET_TAB;

    case 'r':
      return TARGET_CR;

    case 'f':
      return TARGET_FF;

    case 'b':
      return TARGET_BS;

    case 'a':
      return TARGET_BELL;

    case 'v':
      return TARGET_VT;

    case 'E':
      return 033;

    case '?':
      return c;
    }
  if (c >= 040 && c <= 0177)
    warning ("unknown escape sequence `\\%c'", c);
  else
    warning ("unknown escape sequence: `\\' followed by char code 0x%x", c);
  return c;
}

int
yylex ()
{
  register int c;
  register int value;
  int wide_flag = 0;

 relex:
  token_buffer[0] = c = skip_white_space ();
  token_buffer[1] = 0;

  yylloc.first_line = lineno;

  switch (c)
    {
    case EOF:
      token_buffer[0] = '\0';
      if (pending_inlines)
	{
	  struct pending_inline *t;
	  int bufpos = 0;

	  t = pending_inlines;
#ifdef DO_METHODS_THE_OLD_WAY
	  yylval.itype = t->token_value;
	  value = t->token;
#else
	  if (t->fndecl == 0)
	    {
	      yylval.itype = t->token_value;
	      value = t->token;
	    }
	  else
	    {
	      yylval.ttype = t->fndecl;
	      value = PRE_PARSED_FUNCTION_DECL;
	    }
#endif

	  yylloc.first_line = lineno = t->lineno;
	  input_filename = t->filename;

	  if (t->next)
	    {
	      /* The buffer we used will be freed at the
		 end of this function.  */
	      pending_inlines = pending_inlines->next;
#ifndef hp9000s300
	      setbuffer (finput2, t->buf, t->len);
	      finput2->_cnt = finput2->_bufsiz - 1;
#else
      	      setvbuf(finput2,t->buf,_IOFBF,t->len);
       	      finput2->_cnt = t->len-1;
#endif
	    }
	  else
	    {
	      pending_inlines = NULL;
	      finput = finput1;
	      obstack_free (&inline_text_obstack, inline_text_firstobj);
	    }
	  /* The space used by T will be freed after all inline
	     functions have been processed.  */
	  if (value <= 0)
	    goto relex;
	  else
	    goto done;
	}
      value = ENDFILE;
      break;

    case 'L':
      /* Capital L may start a wide-string or wide-character constant.  */
      {
	register int c = getc (finput);
	if (c == '\'')
	  {
	    wide_flag = 1;
	    goto char_constant;
	  }
	if (c == '"')
	  {
	    wide_flag = 1;
	    goto string_constant;
	  }
	ungetc (c, finput);
      }

    case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
    case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
    case 'K':		  case 'M':  case 'N':  case 'O':
    case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
    case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
    case 'Z':
    case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    case 'p':  case 'q':  case 'r':  case 's':  case 't':
    case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
    case 'z':
    case '_':
    case '$':
      {
	register char *p;

	p = token_buffer;
	while (isalnum(c) || (c == '_') || c == '$')
	  {
	    if (p >= token_buffer + maxtoken)
	      p = extend_token_buffer (p);
	  if (c == '$')
	    {
	      if (pedantic)
		{
		  if (! dollar_seen)
		    warning ("ANSI C forbids `$' (first use here)");
		  dollar_seen = 1;
		}
	    }

	    *p++ = c;
	    c = getc (finput);
	  }

	*p = 0;
	ungetc (c, finput);

	value = IDENTIFIER;
	yylval.itype = 0;

	/* Try to recognize a keyword.  */

	if (p - token_buffer <= MAXRESERVED)
	  {
	    /* Use binary search to find match.  */
	    register int lo = frw [p - token_buffer];
	    register int hi = frw [p - token_buffer + 1] - 1;
	    register int mid = (lo + hi) >> 1;
	    register char cmp, ch;

	    p = token_buffer;
	    ch = *p++;
	    while (lo <= hi)
	      {
		register struct resword *r = &reswords[mid];
		if ((cmp = (r->name[0] - ch))
		    || (cmp = strcmp (r->name+1, p)))
		  {
		    if (cmp < 0)
		      lo = mid + 1;
		    else
		      hi = mid - 1;
		    mid = (lo + hi) >> 1;
		  }
		else
		  {
		    if (r->rid)
		      yylval.ttype = ridpointers[(int) r->rid];
		    if ((! flag_no_asm
			 || ((int) r->token != ASM
			     && (int) r->token != TYPEOF
			     && !strcmp (r->name, "inline")))
			/* -ftraditional means don't recognize
			   typeof, const, volatile, noalias, signed or inline.  */
			&& (! flag_traditional
			    || ((int) r->token != TYPE_QUAL
				&& (int) r->token != TYPEOF
				&& strcmp (r->name, "signed")
				&& strcmp (r->name, "inline"))))
		      value = (int) r->token;
		    break;
		  }
	      }
	  }

      /* If we did not find a keyword, look for an identifier
	 (or a typename).  */

	if (value == IDENTIFIER)
	  {
	    tree tmp = get_identifier (token_buffer);
	    lastiddecl = lookup_name (tmp);

	    if (lastiddecl && TREE_CODE (lastiddecl) == TYPE_DECL)
	      {
		/* This call could blow away yylval.  */
		c = skip_white_space ();
		if (c == ':')
		  {
		    c = getc (finput);
		    if (c == ':')
		      value = TYPENAME_SCOPE;
		    else
		      {
			value = TYPENAME_COLON;
			ungetc (c, finput);
		      }
		  }
		else if (c == '.'
			 && current_function_name == NULL_TREE
			 && current_class_type == NULL_TREE)
		  {
		    c = getc (finput);
		    if (c == '.')
		      {
			c = getc (finput);
			if (c != '.')
			  error ("missing '.' in `...'");
			value = TYPENAME_ELLIPSIS;
			tmp = build_tree_list (NULL_TREE, build_tree_list (NULL_TREE, tmp));
		      }
		    else
		      {
			warning ("use of obsolete scope operator `.'; use `::' instead");
			ungetc (c, finput);
			value = TYPENAME_SCOPE;
		      }
		  }
		else
		  {
		    value = TYPENAME;
		    ungetc (c, finput);
		  }
	      }
	    yylval.ttype = tmp;
	  }
      }
      break;

    case '0':  case '1':  case '2':  case '3':  case '4':
    case '5':  case '6':  case '7':  case '8':  case '9':
    case '.':
      {
	register char *p;
	int base = 10;
	int count = 0;
	int largest_digit = 0;
	int numdigits = 0;
	/* for multi-precision arithmetic,
	   we store only 8 live bits in each short,
	   giving us 64 bits of reliable precision */
	short shorts[8];
	int floatflag = 0;  /* Set 1 if we learn this is a floating constant */

	for (count = 0; count < 8; count++)
	  shorts[count] = 0;

	p = token_buffer;
	*p++ = c;

	if (c == '0')
	  {
	    *p++ = (c = getc (finput));
	    if ((c == 'x') || (c == 'X'))
	      {
		base = 16;
		*p++ = (c = getc (finput));
	      }
	    else
	      {
		base = 8;
		numdigits++;
	      }
	  }

	/* Read all the digits-and-decimal-points.  */

	while (c == '.'
	       || (isalnum (c) && (c != 'l') && (c != 'L')
		   && (c != 'u') && (c != 'U')
		   && (!floatflag || ((c != 'f') && (c != 'F')))))
	  {
	    if (c == '.')
	      {
		if (base == 16)
		  error ("floating constant may not be in radix 16");
		floatflag = 1;
		base = 10;
		*p++ = c = getc (finput);
		/* Accept '.' as the start of a floating-point number
		   only when it is followed by a digit.
		   Otherwise, unread the following non-digit
		   and use the '.' as a structural token.  */
		if (p == token_buffer + 2 && !isdigit (c))
		  {
		    if (c == '.')
		      {
			c = getc (finput);
			if (c == '.')
			  {
			    *p++ = '.';
			    *p = '\0';
			    return ELLIPSIS;
			  }
			token_buffer[2] = '\0';
			ungetc (c, finput);
			return RANGE;
		      }
		    ungetc (c, finput);
		    token_buffer[1] = '\0';
		    value = '.';
		    goto done;
		  }
	      }
	    else
	      {
		/* It is not a decimal point.
		   It should be a digit (perhaps a hex digit).  */

		if (isdigit (c))
		  {
		    c = c - '0';
		  }
		else if (base <= 10)
		  {
		    if ((c&~040) == 'E')
		      {
			base = 10;
			floatflag = 1;
			break;   /* start of exponent */
		      }
		    error ("nondigits in number and not hexadecimal");
		    c = 0;
		  }
		else if (c >= 'a')
		  {
		    c = c - 'a' + 10;
		  }
		else
		  {
		    c = c - 'A' + 10;
		  }
		if (c >= largest_digit)
		  largest_digit = c;
		numdigits++;

		for (count = 0; count < 8; count++)
		  {
		    (shorts[count] *= base);
		    if (count)
		      {
			shorts[count] += (shorts[count-1] >> 8);
			shorts[count-1] &= (1<<8)-1;
		      }
		    else shorts[0] += c;
		  }
    
		if (p >= token_buffer + maxtoken - 3)
		  p = extend_token_buffer (p);
		*p++ = (c = getc (finput));
	      }
	  }

	if (numdigits == 0)
	  error ("numeric constant with no digits");

	if (largest_digit >= base)
	  error ("numeric constant contains digits beyond the radix");

	/* Remove terminating char from the token buffer and delimit the string */
	*--p = 0;

	if (floatflag)
	  {
	    tree type = double_type_node;
	    char f_seen = 0;
	    char l_seen = 0;

	    /* Read explicit exponent if any, and put it in tokenbuf.  */

	    if ((c == 'e') || (c == 'E'))
	      {
		if (p >= token_buffer + maxtoken - 3)
		  p = extend_token_buffer (p);
		*p++ = c;
		c = getc (finput);
		if ((c == '+') || (c == '-'))
		  {
		    *p++ = c;
		    c = getc (finput);
		  }
		if (! isdigit (c))
		  error ("floating constant exponent has no digits");
	        while (isdigit (c))
		  {
		    if (p >= token_buffer + maxtoken - 3)
		      p = extend_token_buffer (p);
		    *p++ = c;
		    c = getc (finput);
		  }
	      }

	    *p = 0;
	    yylval.ttype = build_real (atof (token_buffer));

	    while (1)
	      {
		if (c == 'f' || c == 'F')
		  {
		    if (f_seen)
		      error ("two `f's in floating constant");
		    f_seen = 1;
		    type = float_type_node;
		  }
		else if (c == 'l' || c == 'L')
		  {
		    if (l_seen)
		      error ("two `l's in floating constant");
		    l_seen = 1;
		    type = long_double_type_node;
		  }
		else
		  {
		    if (isalnum (c))
		      {
			error ("garbage at end of number");
			while (isalnum (c))
			  {
			    if (p >= token_buffer + maxtoken - 3)
			      p = extend_token_buffer (p);
			    *p++ = c;
			    c = getc (finput);
			  }
		      }
		    break;
		  }
		if (p >= token_buffer + maxtoken - 3)
		  p = extend_token_buffer (p);
		*p++ = c;
		c = getc (finput);
	      }

	    ungetc (c, finput);
	    *p = 0;

	    TREE_TYPE (yylval.ttype) = type;
	  }
	else
	  {
	    tree type;
	    int spec_unsigned = 0;
	    int spec_long = 0;

	    while (1)
	      {
		if (c == 'u' || c == 'U')
		  {
		    if (spec_unsigned)
		      error ("two `u's in integer constant");
		    spec_unsigned = 1;
		  }
		else if (c == 'l' || c == 'L')
		  {
		    if (spec_long)
		      error ("two `l's in integer constant");
		    spec_long = 1;
		  }
		else
		  {
		    if (isalnum (c))
		      {
			error ("garbage at end of number");
			while (isalnum (c))
			  {
			    if (p >= token_buffer + maxtoken - 3)
			      p = extend_token_buffer (p);
			    *p++ = c;
			    c = getc (finput);
			  }
		      }
		    break;
		  }
		if (p >= token_buffer + maxtoken - 3)
		  p = extend_token_buffer (p);
		*p++ = c;
		c = getc (finput);
	      }

	    ungetc (c, finput);

	    if (shorts[7] | shorts[6] | shorts[5] | shorts[4])
	      warning ("integer constant out of range");

	    /* This is simplified by the fact that our constant
	       is always positive.  */
	    yylval.ttype
	      = build_int_2 ((shorts[3]<<24) + (shorts[2]<<16) + (shorts[1]<<8) + shorts[0],
			     0);
    
	    if (!spec_long && !spec_unsigned
		&& int_fits_type_p (yylval.ttype, integer_type_node))
	      type = integer_type_node;

	    else if (!spec_long && base != 10
		&& int_fits_type_p (yylval.ttype, unsigned_type_node))
	      type = unsigned_type_node;

	    else if (!spec_unsigned
		&& int_fits_type_p (yylval.ttype, long_integer_type_node))
	      type = long_integer_type_node;

	    else
	      {
		type = long_unsigned_type_node;
		if (! int_fits_type_p (yylval.ttype, long_unsigned_type_node))
		  warning ("integer constant out of range");
	      }

	    TREE_TYPE (yylval.ttype) = type;
	  }

	value = CONSTANT; break;
      }

    case '\'':
    char_constant:
      c = getc (finput);
      {
	register int code = 0;

      tryagain:

	if (c == '\\')
	  {
	    c = readescape ();
	    if (c < 0)
	      goto tryagain;
	  }
	else if (c == '\n')
	  {
	    if (pedantic)
	      warning ("ANSI C forbids newline in character constant");
	    lineno++;
	  }

	code = c;
	token_buffer[1] = c;
	token_buffer[2] = '\'';
	token_buffer[3] = 0;

	c = getc (finput);
	if (c != '\'')
	  error ("malformatted character constant");

	/* If char type is signed, sign-extend the constant.  */
	if (TREE_UNSIGNED (char_type_node)
	    || ((code >> (BITS_PER_UNIT - 1)) & 1) == 0)
	  yylval.ttype = build_int_2 (code & ((1 << BITS_PER_UNIT) - 1), 0);
	else
	  yylval.ttype = build_int_2 (code | ((-1) << BITS_PER_UNIT), -1);

	if (flag_char_charconst)
	  TREE_TYPE (yylval.ttype) = char_type_node;
	else
	  TREE_TYPE (yylval.ttype) = integer_type_node;
	value = CONSTANT; break;
      }

    case '"':
    string_constant:
      {
	register char *p;

	c = getc (finput);
	p = token_buffer + 1;

	while (c != '"')
	  {
	    if (c == '\\')
	      {
		c = readescape ();
		if (c < 0)
		  goto skipnewline;
	      }
	    else if (c == '\n')
	      {
		if (pedantic)
		  warning ("ANSI C forbids newline in string constant");
		lineno++;
	      }

	    if (p == token_buffer + maxtoken)
	      p = extend_token_buffer (p);
	    *p++ = c;

	  skipnewline:
	    c = getc (finput);
	  }

	*p = 0;

	if (wide_flag)
	  {
	    /* If this is a L"..." wide-string, convert each char
	       to an int, making a vector of ints.  */
	    int *widebuf = (int *) alloca (p - token_buffer);
	    char *p1 = token_buffer + 1;
	    for (; p1 == p; p1++)
	      widebuf[p1 - token_buffer - 1] = *p1;
	    yylval.ttype = build_string ((p - token_buffer) * sizeof (int),
					 widebuf);
	    TREE_TYPE (yylval.ttype) = int_array_type_node;
	  }
	else
	  {
	    yylval.ttype = build_string (p - token_buffer, token_buffer + 1);
	    TREE_TYPE (yylval.ttype) = char_array_type_node;
	  }

	*p++ = '"';
	*p = 0;

	value = STRING; break;
      }
      
    case '+':
    case '-':
    case '&':
    case '|':
    case '<':
    case '>':
    case '*':
    case '/':
    case '%':
    case '^':
    case '!':
    case '=':
      {
	register int c1;

      combine:

	switch (c)
	  {
	  case '+':
	    yylval.code = PLUS_EXPR; break;
	  case '-':
	    yylval.code = MINUS_EXPR; break;
	  case '&':
	    yylval.code = BIT_AND_EXPR; break;
	  case '|':
	    yylval.code = BIT_IOR_EXPR; break;
	  case '*':
	    yylval.code = MULT_EXPR; break;
	  case '/':
	    yylval.code = TRUNC_DIV_EXPR; break;
	  case '%':
	    yylval.code = TRUNC_MOD_EXPR; break;
	  case '^':
	    yylval.code = BIT_XOR_EXPR; break;
	  case LSHIFT:
	    yylval.code = LSHIFT_EXPR; break;
	  case RSHIFT:
	    yylval.code = RSHIFT_EXPR; break;
	  case '<':
	    yylval.code = LT_EXPR; break;
	  case '>':
	    yylval.code = GT_EXPR; break;
	  }	

	token_buffer[1] = c1 = getc (finput);
	token_buffer[2] = 0;

	if (c1 == '=')
	  {
	    switch (c)
	      {
	      case '<':
		value = ARITHCOMPARE; yylval.code = LE_EXPR; goto done;
	      case '>':
		value = ARITHCOMPARE; yylval.code = GE_EXPR; goto done;
	      case '!':
		value = EQCOMPARE; yylval.code = NE_EXPR; goto done;
	      case '=':
		value = EQCOMPARE; yylval.code = EQ_EXPR; goto done;
	      }	
	    value = ASSIGN; goto done;
	  }
	else if (c == c1)
	  switch (c)
	    {
	    case '+':
	      value = PLUSPLUS; goto done;
	    case '-':
	      value = MINUSMINUS; goto done;
	    case '&':
	      value = ANDAND; goto done;
	    case '|':
	      value = OROR; goto done;
	    case '<':
	      c = LSHIFT;
	      goto combine;
	    case '>':
	      c = RSHIFT;
	      goto combine;
	    }
	else if ((c == '-') && (c1 == '>'))
	  { value = POINTSAT; goto done; }
	else if (c1 == '?')
	  if (c == '<')
	    {
	      value = MIN_MAX;
	      yylval.code = MIN_EXPR;
	      goto done;
	    }
	  else if (c == '>')
	    {
	      value = MIN_MAX;
	      yylval.code = MAX_EXPR;
	      goto done;
	    }

	ungetc (c1, finput);
	token_buffer[1] = 0;

	if ((c == '<') || (c == '>'))
	  value = ARITHCOMPARE;
	else value = c;
	goto done;
      }

    case ':':
      c = getc (finput);
      if (c == ':')
	{
	  token_buffer[1] = ':';
	  token_buffer[2] = '\0';
	  value = SCOPE;
	  yylval.itype = 1;
	}
      else {
	ungetc (c, finput);
	value = ':';
      }
      break;

    default:
      value = c;
    }

done:
  yylloc.last_line = lineno;

  return value;
}
