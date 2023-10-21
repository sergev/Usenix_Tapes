/* Handle the hair of processing (but not expanding) inline functions
   Copyright (C) 1987 Free Software Foundation, Inc.
   Contributed by Michael Tiemann (tiemann@mcc.com)

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


/* Handle method declarations.  */
#include <stdio.h>
#include "config.h"
#include "tree.h"
#include "c-tree.h"
#include "overload.h"

/* TREE_LIST of the current inline functions that need to be
   processed.  */
struct pending_inline *pending_inlines;

# define INLINE_BUF_SIZE 8192
# define OB_INIT() (inline_bufp = inline_buffer)
# define OB_PUTC(C) (*inline_bufp++ = (C))
# define OB_PUTC2(C1,C2) (*inline_bufp++ = (C1), *inline_bufp++ = (C2))
# define OB_PUTS(S) (sprintf (inline_bufp, S), inline_bufp += sizeof (S) - 1)
# define OB_FINISH() (*inline_bufp++ = '\0')

/* Counter to help build parameter names in case they were omitted.  */
static int dummy_name = 0;
static int in_parmlist = 0;
/* Just a pointer into INLINE_BUFFER.  */
static char *inline_bufp;
/* Also a pointer into INLINE_BUFFER.  This points to a safe place to
   cut back to if we assign it 0, in case of error.  */
static char *inline_errp;
static char inline_buffer [INLINE_BUF_SIZE];
static void dump_type (), dump_decl ();
static void dump_init (), dump_unary_op (), dump_binary_op ();

tree wrapper_name, anti_wrapper_name;

void
init_method ()
{
  wrapper_name = get_identifier (WRAPPER_DECL_FORMAT);
  anti_wrapper_name = get_identifier (ANTI_WRAPPER_DECL_FORMAT);
}

/* Return a pointer to the end of the new text in INLINE_BUFFER.
   We cannot use `fatal' or `error' in here because that
   might cause an infinite loop.  */
static char *
new_text_len (s)
     char *s;
{
  while (*s++) ;

  if (s >= inline_buffer + INLINE_BUF_SIZE)
    {
      fprintf (stderr, "recompile c++ with larger INLINE_BUF_SIZE (%d)", INLINE_BUF_SIZE);
      abort ();
    }
  return s - 1;
}

/* Check that we have not overflowed INLINE_BUFFER.
   We cannot use `fatal' or `error' in here because that
   might cause an infinite loop.  */
static void
check_text_len (s)
     char *s;
{
  if (s >= inline_buffer + INLINE_BUF_SIZE)
    {
      fprintf (stderr, "recompile c++ with larger INLINE_BUF_SIZE (%d)", INLINE_BUF_SIZE);
      abort ();
    }
}

tree
make_anon_parm_name ()
{
  char buf[32];

  sprintf (buf, ANON_PARMNAME_FORMAT, dummy_name++);
  return get_identifier (buf);
}

void
clear_anon_parm_name ()
{
  /* recycle these names.  */
  dummy_name = 0;
}

static void
dump_type_prefix (t, p)
     tree t;
     int *p;
{
  int old_p = 0;

  if (t == NULL_TREE)
    return;

  if (TREE_READONLY (t))
    OB_PUTS ("const ");
  if (TREE_VOLATILE (t))
    OB_PUTS ("volatile ");

  switch (TREE_CODE (t))
    {
    case ERROR_MARK:
      sprintf (inline_bufp, ANON_PARMNAME_FORMAT, dummy_name++);
      break;

    case UNKNOWN_TYPE:
      OB_PUTS ("<unknown type>");
      return;

    case TREE_LIST:
      dump_type (TREE_VALUE (t), &old_p);
      if (TREE_CHAIN (t))
	{
	  if (TREE_VALUE (TREE_CHAIN (t)) != void_type_node)
	    {
	      OB_PUTC (',');
	      dump_type (TREE_CHAIN (t), &old_p);
	    }
	}
      else OB_PUTS ("...");
      return;

    case POINTER_TYPE:
      if (TREE_MEMBER_POINTER (t))
	{
	  tree t1 = TREE_TYPE (t);
	  tree t2 = TYPE_DOMAIN (t);

	  *p += 1;
	  if (TREE_CODE (t1) == FUNCTION_TYPE)
	    {
	      if (in_parmlist)
		OB_PUTS ("auto ");

	      dump_type_prefix (TREE_TYPE (t1), &old_p);
	      OB_PUTC ('(');
	      dump_type (t2, p);
	      OB_PUTC2 (':', ':');
	      while (*p)
		{
		  OB_PUTC ('*');
		  *p -= 1;
		}
	    }
	  else
	    {
	      dump_type_prefix (t1, p);
	      OB_PUTC (' ');
	      dump_type (t2, p);
	      OB_PUTC2 (':', ':');
	      while (*p)
		{
		  OB_PUTC ('*');
		  *p -= 1;
		}
	    }
	}
      else
	{
	  *p += 1;
	  dump_type_prefix (TREE_TYPE (t), p);
	  while (*p)
	    {
	      OB_PUTC ('*');
	      *p -= 1;
	    }
	}
      return;

    case REFERENCE_TYPE:
      dump_type_prefix (TREE_TYPE (t), p);
      OB_PUTC ('&');
      return;

    case ARRAY_TYPE:
      dump_type_prefix (TREE_TYPE (t), p);
      return;

    case FUNCTION_TYPE:
      if (in_parmlist)
	OB_PUTS ("auto ");
      dump_type_prefix (TREE_TYPE (t), &old_p);
      OB_PUTC ('(');
      while (*p)
	{
	  OB_PUTC ('*');
	  *p -= 1;
	}
      return;

    case IDENTIFIER_NODE:
      sprintf (inline_bufp, "%s ", IDENTIFIER_POINTER (t));
      break;

    case RECORD_TYPE:
      sprintf (inline_bufp, "struct %s ", TYPE_NAME_STRING (t));
      break;

    case UNION_TYPE:
      sprintf (inline_bufp, "union %s ", TYPE_NAME_STRING (t));
      break;

    case ENUMERAL_TYPE:
      sprintf (inline_bufp, "enum %s ", TYPE_NAME_STRING (t));
      break;

    case TYPE_DECL:
      sprintf (inline_bufp, "%s ", IDENTIFIER_POINTER (DECL_NAME (t)));
      break;

    case INTEGER_TYPE:
      /* Normally, `unsigned' is part of the deal.  Not so if it comes
	 with `const' or `volatile'.  */
      if (TREE_UNSIGNED (t)
	  && (TREE_READONLY (t) || TREE_VOLATILE (t)))
	OB_PUTS ("unsigned ");
      /* fall through.  */
    case REAL_TYPE:
    case VOID_TYPE:
      sprintf (inline_bufp, "%s ", TYPE_NAME_STRING (t));
      break;

    default:
      abort ();
    }
  inline_bufp = new_text_len (inline_bufp);
}

static void
dump_type_suffix (t, p)
     tree t;
     int *p;
{
  int old_p = 0;

  if (t == NULL_TREE)
    return;

  switch (TREE_CODE (t))
    {
    case ERROR_MARK:
      sprintf (inline_bufp, ANON_PARMNAME_FORMAT, dummy_name++);
      break;

    case UNKNOWN_TYPE:
      return;

    case POINTER_TYPE:
      if (TREE_MEMBER_POINTER (t))
	{
	  tree t1 = TREE_TYPE (t);
	  tree t2 = TYPE_DOMAIN (t);

	  if (TREE_CODE (t1) == FUNCTION_TYPE)
	    {
	      OB_PUTC2 (')', '(');
	      t2 = TREE_CHAIN (TYPE_ARG_TYPES (t1));
	      if (t2 == NULL_TREE)
		{
		  OB_PUTS ("...");
		}
	      else if (TREE_VALUE (t2) != void_type_node)
		{
		  in_parmlist++;
		  dump_type (t2, p);
		  in_parmlist--;
		}
	      OB_PUTC (')');
	      dump_type_suffix (TREE_TYPE (t1), &old_p);
	    }
	  else
	    {
	      dump_type_suffix (t1, p);
	    }
	}
      else
	{
	  dump_type_suffix (TREE_TYPE (t), p);
	}
      return;

    case REFERENCE_TYPE:
      dump_type_suffix (TREE_TYPE (t), p);
      return;

    case ARRAY_TYPE:
      dump_type_suffix (TREE_TYPE (t), p);
      OB_PUTC2 ('[', ']');
      return;

    case FUNCTION_TYPE:
      OB_PUTC2 (')', '(');
      if (TYPE_ARG_TYPES (t) && TREE_VALUE (TYPE_ARG_TYPES (t)) != void_type_node)
	{
	  in_parmlist++;
	  dump_type (TYPE_ARG_TYPES (t), p);
	  in_parmlist--;
	}
      OB_PUTC (')');
      dump_type_suffix (TREE_TYPE (t), &old_p);
      return;

    case IDENTIFIER_NODE:
    case RECORD_TYPE:
    case UNION_TYPE:
    case ENUMERAL_TYPE:
    case TYPE_DECL:
    case INTEGER_TYPE:
    case REAL_TYPE:
    case VOID_TYPE:
      return;

    default:
      abort ();
    }
  inline_bufp = new_text_len (inline_bufp);
}

static void
dump_type (t, p)
     tree t;
     int *p;
{
  int old_p = 0;

  if (t == NULL_TREE)
    return;

  if (TREE_READONLY (t))
    OB_PUTS ("const ");
  if (TREE_VOLATILE (t))
    OB_PUTS ("volatile ");

  switch (TREE_CODE (t))
    {
    case ERROR_MARK:
      sprintf (inline_bufp, ANON_PARMNAME_FORMAT, dummy_name++);
      break;

    case UNKNOWN_TYPE:
      OB_PUTS ("<unknown type>");
      return;

    case TREE_LIST:
      dump_type (TREE_VALUE (t), &old_p);
      if (TREE_CHAIN (t))
	{
	  if (TREE_VALUE (TREE_CHAIN (t)) != void_type_node)
	    {
	      OB_PUTC (',');
	      dump_type (TREE_CHAIN (t), &old_p);
	    }
	}
      else OB_PUTS ("...");
      return;

    case POINTER_TYPE:
      if (TREE_MEMBER_POINTER (t))
	{
	  tree t1 = TREE_TYPE (t);
	  tree t2 = TYPE_DOMAIN (t);

	  *p += 1;
	  if (TREE_CODE (t1) == FUNCTION_TYPE)
	    {
	      if (in_parmlist)
		OB_PUTS ("auto ");

	      dump_type (TREE_TYPE (t1), &old_p);
	      OB_PUTC ('(');
	      dump_type (t2, p);
	      OB_PUTC2 (':', ':');
	      while (*p)
		{
		  OB_PUTC ('*');
		  *p -= 1;
		}
	      OB_PUTC2 (')', '(');
	      t2 = TREE_CHAIN (TYPE_ARG_TYPES (t1));
	      if (t2 == NULL_TREE)
		{
		  OB_PUTS ("...");
		}
	      else if (TREE_VALUE (t2) != void_type_node)
		{
		  in_parmlist++;
		  dump_type (t2, p);
		  in_parmlist--;
		}
	      OB_PUTC (')');
	    }
	  else
	    {
	      dump_type (t1, p);
	      OB_PUTC (' ');
	      dump_type (t2, p);
	      OB_PUTC2 (':', ':');
	      while (*p)
		{
		  OB_PUTC ('*');
		  *p -= 1;
		}
	    }
	}
      else
	{
	  *p += 1;
	  dump_type (TREE_TYPE (t), p);
	  while (*p)
	    {
	      OB_PUTC ('*');
	      *p -= 1;
	    }
	}
      return;

    case REFERENCE_TYPE:
      dump_type (TREE_TYPE (t), p);
      OB_PUTC ('&');
      return;

    case ARRAY_TYPE:
      dump_type (TREE_TYPE (t), p);
      OB_PUTC2 ('[', ']');
      return;

    case FUNCTION_TYPE:
      if (in_parmlist)
	OB_PUTS ("auto ");
      dump_type (TREE_TYPE (t), &old_p);
      OB_PUTC ('(');
      while (*p)
	{
	  OB_PUTC ('*');
	  *p -= 1;
	}
      OB_PUTC2 (')', '(');
      if (TYPE_ARG_TYPES (t) && TREE_VALUE (TYPE_ARG_TYPES (t)) != void_type_node)
	{
	  in_parmlist++;
	  dump_type (TYPE_ARG_TYPES (t), p);
	  in_parmlist--;
	}
      OB_PUTC (')');
      return;

    case IDENTIFIER_NODE:
      sprintf (inline_bufp, "%s ", IDENTIFIER_POINTER (t));
      break;

    case RECORD_TYPE:
      sprintf (inline_bufp, "struct %s ", TYPE_NAME_STRING (t));
      break;

    case UNION_TYPE:
      sprintf (inline_bufp, "union %s ", TYPE_NAME_STRING (t));
      break;

    case ENUMERAL_TYPE:
      sprintf (inline_bufp, "enum %s ", TYPE_NAME_STRING (t));
      break;

    case TYPE_DECL:
      sprintf (inline_bufp, "%s ", IDENTIFIER_POINTER (DECL_NAME (t)));
      break;

    case INTEGER_TYPE:
      /* Normally, `unsigned' is part of the deal.  Not so if it comes
	 with `const' or `volatile'.  */
      if (TREE_UNSIGNED (t)
	  && (TREE_READONLY (t) || TREE_VOLATILE (t)))
	OB_PUTS ("unsigned ");
      /* fall through.  */
    case REAL_TYPE:
    case VOID_TYPE:
      sprintf (inline_bufp, "%s ", TYPE_NAME_STRING (t));
      break;

    default:
      abort ();
    }
  inline_bufp = new_text_len (inline_bufp);
}

static void
dump_decl (t)
     tree t;
{
  int p = 0;

  if (t == NULL_TREE)
    return;

  switch (TREE_CODE (t))
    {
    case ERROR_MARK:
      sprintf (inline_bufp, " /* decl error */ ");
      break;

    case PARM_DECL:
      dump_type_prefix (TREE_TYPE (t), &p);
      if (DECL_NAME (t))
	dump_decl (DECL_NAME (t));
      else
	{
	  sprintf (inline_bufp, ANON_PARMNAME_FORMAT, dummy_name++);
	  break;
	}
      dump_type_suffix (TREE_TYPE (t), &p);
      return;

    case CALL_EXPR:
      dump_decl (TREE_OPERAND (t, 0));
      OB_PUTC ('(');
      in_parmlist++;
      dump_decl (TREE_OPERAND (t, 1));
      in_parmlist--;
      t = tree_last (TYPE_ARG_TYPES (TREE_TYPE (t)));
      if (!t || TREE_VALUE (t) != void_type_node)
	OB_PUTS ("...");
      OB_PUTC (')');
      return;

    case ARRAY_REF:
      dump_decl (TREE_OPERAND (t, 0));
      OB_PUTC ('[');
      dump_decl (TREE_OPERAND (t, 1));
      OB_PUTC (']');
      return;

    case TYPE_DECL:
      sprintf (inline_bufp, "%s ", IDENTIFIER_POINTER (DECL_NAME (t)));
      break;

    case TYPE_EXPR:
      abort ();
      break;

    case IDENTIFIER_NODE:
      if (OPERATOR_NAME_P (t))
	sprintf (inline_bufp, "operator %s ", operator_name_string (t));
      else if (OPERATOR_TYPENAME_P (t))
	{
	  OB_PUTS ("operator ");
	  dump_type (TREE_TYPE (t), &p);
	  return;
	}
      else
	sprintf (inline_bufp, "%s ", IDENTIFIER_POINTER (t));
      break;

    case BIT_NOT_EXPR:
      OB_PUTC2 ('~', ' ');
      dump_decl (TREE_OPERAND (t, 0));
      return;

    case SCOPE_REF:
      sprintf (inline_bufp, "%s :: ", IDENTIFIER_POINTER (TREE_OPERAND (t, 0)));
      inline_bufp += sizeof ("%s :: ") + IDENTIFIER_LENGTH (TREE_OPERAND (t, 0));
      dump_decl (TREE_OPERAND (t, 1));
      return;

    case INDIRECT_REF:
      OB_PUTC ('*');
      dump_decl (TREE_OPERAND (t, 0));
      return;

    case ADDR_EXPR:
      OB_PUTC ('&');
      dump_decl (TREE_OPERAND (t, 0));
      return;

    default:
      abort ();
    }
  inline_bufp = new_text_len (inline_bufp);
}

static void
dump_init (t)
     tree t;
{
  extern char *tree_code_err_name[]; /* in print-tree.c  */
  int dummy;

  switch (TREE_CODE (t))
    {
    case VAR_DECL:
    case PARM_DECL:
      sprintf (inline_bufp, " %s ", IDENTIFIER_POINTER (DECL_NAME (t)));
      break;

    case FUNCTION_DECL:
      {
	tree name = DECL_NAME (t);

	if (DESTRUCTOR_NAME_P (name))
	  sprintf (inline_bufp, " ~%s ",
		   IDENTIFIER_POINTER (DECL_ORIGINAL_NAME (t)));
	else if (OPERATOR_NAME_P (name))
	  sprintf (inline_bufp, "operator %s ", OPERATOR_NAME_STRING (t));
	else if (OPERATOR_TYPENAME_P (name))
	  {
	    dummy = 0;
	    OB_PUTS ("operator ");
	    dump_type (TREE_TYPE (name), &dummy);
	  }
	else if (WRAPPER_NAME_P (name))
	  sprintf (inline_bufp, " ()%s ",
		   IDENTIFIER_POINTER (DECL_ORIGINAL_NAME (t)));
	else if (ANTI_WRAPPER_NAME_P (name))
	  sprintf (inline_bufp, " ~()%s ",
		   IDENTIFIER_POINTER (DECL_ORIGINAL_NAME (t)));
	else sprintf (inline_bufp, " %s ",
		      IDENTIFIER_POINTER (DECL_ORIGINAL_NAME (t)));
      }
      break;

    case CONST_DECL:
      dummy = 0;
      OB_PUTC2 ('(', '(');
      dump_type (TREE_TYPE (t), &dummy);
      OB_PUTC (')');
      dump_init (DECL_INITIAL (t));
      OB_PUTC (')');
      return;

    case INTEGER_CST:
      sprintf (inline_bufp, " %d ", TREE_INT_CST_LOW (t));
      break;

    case REAL_CST:
      sprintf (inline_bufp, " %g ", TREE_REAL_CST (t));
      break;

    case STRING_CST:
      {
	char *p = TREE_STRING_POINTER (t);
	int len = TREE_STRING_LENGTH (t) - 1;
	int i;

	check_text_len (inline_bufp + len + 2);
	OB_PUTC ('\"');
	for (i = 0; i < len; i++)
	  {
	    register char c = p[i];
	    if (c == '\"' || c == '\\')
	      OB_PUTC ('\\');
	    if (c >= ' ' && c < 0177)
	      OB_PUTC (c);
	    else
	      {
		sprintf (inline_bufp, "\\%03o", c);
		inline_bufp = new_text_len (inline_bufp);
	      }
	  }
	OB_PUTC ('\"');
      }
      return;

    case COMPOUND_EXPR:
      dump_binary_op (",", t, 1);
      break;

    case COND_EXPR:
      OB_PUTC ('(');
      dump_init (TREE_OPERAND (t, 0));
      OB_PUTS (" ? ");
      dump_init (TREE_OPERAND (t, 1));
      OB_PUTS (" : ");
      dump_init (TREE_OPERAND (t, 2));
      OB_PUTC (')');
      return;

    case X_COND_EXPR:
      dump_binary_op ("?:", t, 2);
      return;

    case CALL_EXPR:
      OB_PUTC ('(');
      dump_init (TREE_OPERAND (t, 0));
      OB_PUTC ('(');
      dump_init (TREE_OPERAND (t, 1));
      OB_PUTC2 (')', ')');
      return;

    case MODIFY_EXPR:
    case PLUS_EXPR:
    case MINUS_EXPR:
    case MULT_EXPR:
    case TRUNC_DIV_EXPR:
    case TRUNC_MOD_EXPR:
    case MIN_EXPR:
    case MAX_EXPR:
    case LSHIFT_EXPR:
    case RSHIFT_EXPR:
    case BIT_IOR_EXPR:
    case BIT_XOR_EXPR:
    case BIT_AND_EXPR:
    case BIT_ANDTC_EXPR:
    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
    case LT_EXPR:
    case LE_EXPR:
    case GT_EXPR:
    case GE_EXPR:
    case EQ_EXPR:
    case NE_EXPR:
      dump_binary_op (tree_code_err_name[(int) TREE_CODE (t)], t,
		      sizeof (tree_code_err_name[(int) TREE_CODE (t)]));
      return;

    case CEIL_DIV_EXPR:
    case FLOOR_DIV_EXPR:
    case ROUND_DIV_EXPR:
      dump_binary_op ("/", t, 1);
      return;

    case CEIL_MOD_EXPR:
    case FLOOR_MOD_EXPR:
    case ROUND_MOD_EXPR:
      dump_binary_op ("%", t, 1);
      return;

    case COMPONENT_REF:
      dump_binary_op (".", t, 1);
      return;

    case CONVERT_EXPR:
      dump_unary_op ("+", t, 1);
      return;

    case ADDR_EXPR:
      dump_unary_op ("&", t, 1);
      return;

    case INDIRECT_REF:
      dump_unary_op ("*", t, 1);
      return;

    case NEGATE_EXPR:
    case BIT_NOT_EXPR:
    case TRUTH_NOT_EXPR:
    case PREDECREMENT_EXPR:
    case PREINCREMENT_EXPR:
      dump_unary_op (tree_code_err_name [(int)TREE_CODE (t)], t,
		     sizeof (tree_code_err_name[(int) TREE_CODE (t)]));
      return;

    case POSTDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
      OB_PUTC ('(');
      dump_init (TREE_OPERAND (t, 0));
      OB_PUTS (tree_code_err_name[(int)TREE_CODE (t)]);
      OB_PUTC (')');
      return;

      /*  This list is incomplete, but should suffice for now.
	  It is very important that `sorry' does not call
	  `report_error_function'.  That could cause an infinite loop.  */
    default:
      sorry ("that operation not supported for default parameters");

      /* fall through to ERROR_MARK...  */
    case ERROR_MARK:
      *inline_errp = '\0';
      inline_bufp = inline_errp + 1;
      return;
    }
  inline_bufp = new_text_len (inline_bufp);
}

static void
dump_binary_op (opstring, t, len)
     char *opstring;
     tree t;
     int len;
{
  OB_PUTC ('(');
  dump_init (TREE_OPERAND (t, 0));
  sprintf (inline_bufp, " %s ", opstring);
  inline_bufp += len + 2;
  dump_init (TREE_OPERAND (t, 1));
  OB_PUTC (')');
  check_text_len (inline_bufp);
}

static void
dump_unary_op (opstring, t, len)
     char *opstring;
     tree t;
     int len;
{
  OB_PUTC ('(');
  sprintf (inline_bufp, " %s ", opstring);
  inline_bufp += len + 2;
  dump_init (TREE_OPERAND (t, 0));
  OB_PUTC (')');
  check_text_len (inline_bufp);
}


/* Process the currently pending inline function definitions.
   This entails:
   (1) Creating a temporary file which contains return type,
       delarator name, and argment names and types of the
       function to be inlined.
   (2) Reading that file into a buffer which can then be
       made to look line another piece of inline code to
       process, stuffing that on the top of the inline
       stack, then letting the lexer and parser read from those
       two.

   If FIELD is a FRIEND_DECL (as opposed to a FIELD_DECL)
   then we do not put out a class prefix in front of it,
   and we do not hide the first parameter.  */

struct pending_inline *
hack_inline_prefix (cname, field)
     tree cname, field;
{
  struct pending_inline *t;
  tree name, fndecl;
  int p = 0;

  dummy_name = 0;
  name = DECL_NAME (field);
  fndecl = TREE_TYPE (field);
  if (TREE_INLINE (fndecl))
    strcpy (inline_buffer, "inline ");
  else
    strcpy (inline_buffer, "static ");
  inline_bufp = inline_buffer + strlen (inline_buffer);
  if (! OPERATOR_TYPENAME_P (name))
    dump_type_prefix (TREE_TYPE (TREE_TYPE (fndecl)), &p);
  if (TREE_CODE (field) == FIELD_DECL)
    {
      dump_type (cname, &p);
      inline_bufp[-1] = ':';
      *inline_bufp++ = ':';
      if (DESTRUCTOR_NAME_P (DECL_NAME (fndecl)))
	OB_PUTC ('~');
      else if (WRAPPER_NAME_P (DECL_NAME (fndecl)))
	OB_PUTC2 ('(', ')');
      else if (ANTI_WRAPPER_NAME_P (DECL_NAME (fndecl)))
	OB_PUTS ("~()");
    }
  dump_decl (name);
  OB_PUTC ('(');
  if (! DESTRUCTOR_NAME_P (DECL_NAME (fndecl)))
    {
      tree parmlist = DECL_ARGUMENTS (fndecl);
      tree typelist = TYPE_ARG_TYPES (TREE_TYPE (fndecl));

      if (TREE_CODE (field) == FIELD_DECL)
	{
	  parmlist = TREE_CHAIN (parmlist);
	  typelist = TREE_CHAIN (typelist);
	}

      in_parmlist++;
      while (parmlist)
	{
	  dump_decl (parmlist);
#if 0
	  if (TREE_PURPOSE (typelist))
	    {
	      inline_errp = inline_bufp;
	      OB_PUTS (" = (");
	      dump_init (TREE_PURPOSE (typelist));
	      OB_PUTC (')');
	      if (*inline_errp == '\0')
		inline_bufp = inline_errp;
	    }
#endif
	  if (TREE_CHAIN (parmlist))
	    OB_PUTC (',');
	  parmlist = TREE_CHAIN (parmlist);
	  typelist = TREE_CHAIN (typelist);
	}
      in_parmlist--;
      if (!typelist || TREE_VALUE (typelist) != void_type_node)
	OB_PUTS ("...");
    }
  OB_PUTC (')');

  if (! OPERATOR_TYPENAME_P (name))
    dump_type_suffix (TREE_TYPE (TREE_TYPE (fndecl)), &p);

  OB_FINISH ();
  check_text_len (inline_bufp);

  t = (struct pending_inline *)xmalloc (sizeof (struct pending_inline));
  t->len = inline_bufp - inline_buffer;
  t->buf = (char *)xmalloc (t->len);
  bcopy (inline_buffer, t->buf, t->len);
  t->lineno = lineno;
  t->filename = input_filename;
  t->token = 0;
  return t;
}

/* Pretty printing for announce_function.  If BUF is nonzero, then
   the text is written there.  The buffer is assued to be of size
   OVERLOAD_MAX_LEN.  CNAME is the name of the class that FNDECL
   belongs to, if we could not figure that out from FNDECL
   itself.  FNDECL is the declaration of the function we
   are interested in seeing.  PRINT_RET_TYPE_P is non-zero if
   we should print the type that this function returns.  */
char *
hack_function_decl (buf, cname, fndecl, print_ret_type_p)
     char *buf;
     tree cname, fndecl;
     int print_ret_type_p;
{
  tree name = DECL_NAME (fndecl);
  tree parmtypes = TYPE_ARG_TYPES (TREE_TYPE (fndecl));
  int p = 0;
  int spaces = 0;

  OB_INIT ();
  if (! cname && TREE_METHOD (fndecl))
    {
      tree arg_type = TREE_VALUE (TYPE_ARG_TYPES (TREE_TYPE (fndecl)));
      if (TREE_CODE (arg_type) != POINTER_TYPE)
	{
	  fprintf (stderr, "bad TREE_METHOD bit in hack_function_decl");
	  abort ();
	}
      else
	cname = TYPE_NAME (TREE_TYPE (arg_type));
    }
  if (print_ret_type_p && ! OPERATOR_TYPENAME_P (name))
    dump_type_prefix (TREE_TYPE (TREE_TYPE (fndecl)), &p);
  if (cname)
    {
      dump_type (cname, &p);
      inline_bufp[-1] = ':';
      *inline_bufp++ = ':';
      parmtypes = TREE_CHAIN (parmtypes);
    }
  if (DESTRUCTOR_NAME_P (name))
    {
      OB_PUTC ('~');
      parmtypes = TREE_CHAIN (parmtypes);
      dump_decl (DECL_ORIGINAL_NAME (fndecl));
    }
  else if (OPERATOR_NAME_P (name))
    {
      sprintf (inline_bufp, "operator %s ", OPERATOR_NAME_STRING (fndecl));
      inline_bufp += strlen (inline_bufp);
    }
  else if (OPERATOR_TYPENAME_P (name))
    {
      OB_PUTS ("operator ");
      dump_type (TREE_TYPE (name), &p);
    }
  else
    {
      if (WRAPPER_NAME_P (name))
	OB_PUTC2 ('(', ')');
      else if (ANTI_WRAPPER_NAME_P (name))
	OB_PUTS ("~()");

      dump_decl (DECL_ORIGINAL_NAME (fndecl));
    }

  OB_PUTC ('(');
  if (parmtypes)
    {
      in_parmlist++;
      if (TREE_VALUE (parmtypes) != void_type_node)
	spaces = 2;
      while (parmtypes && TREE_VALUE (parmtypes) != void_type_node)
	{
	  dump_type (TREE_VALUE (parmtypes), &p);
	  while (inline_bufp[-1] == ' ')
	    inline_bufp--;
	  if (TREE_PURPOSE (parmtypes))
	    {
	      inline_errp = inline_bufp;
	      OB_PUTS (" (= ");
	      dump_init (TREE_PURPOSE (parmtypes));
	      OB_PUTC (')');
	    }
	  OB_PUTC2 (',', ' ');
	  parmtypes = TREE_CHAIN (parmtypes);
	}
      in_parmlist--;
    }
  
  if (parmtypes)
    inline_bufp -= spaces;
  else
    OB_PUTS ("...");

  OB_PUTC (')');

  if (print_ret_type_p && ! OPERATOR_TYPENAME_P (name))
    dump_type_suffix (TREE_TYPE (TREE_TYPE (fndecl)), &p);

  OB_FINISH ();
  check_text_len (inline_bufp);
  
  if (buf == 0)
    return inline_buffer;

  if (strlen (inline_buffer) >= OVERLOAD_MAX_LEN)
    {
      fprintf (stderr, "hack_function_decl returns something too large");
      abort ();
    }
  strcpy (buf, inline_buffer);
  return buf;
}

/* Move inline function defintions out of structure so that they
   can be processed normally.  CNAME is the name of the class
   we are working from, METHOD_LIST is the list of method lists
   of the structure.  We delete friend methods here, after
   saving away their inline function definitions (if any).  */

/* Subroutine of `do_inline_method_hair'.  */
void prepare_inline (cname, methods)
     tree cname, methods;
{
  tree fndecl = TREE_TYPE (methods);
  if (DECL_INITIAL (fndecl))
    {
      struct pending_inline *t1, *t2;

      t2 = (struct pending_inline *)DECL_INITIAL (fndecl);
      t2->next = pending_inlines;
      t2->fndecl = fndecl;
#ifdef DO_METHODS_THE_OLD_WAY
      t1 = hack_inline_prefix (cname, methods);
      t1->next = t2;
#else
      t1 = t2;
#endif
      pending_inlines = t1;
      DECL_INITIAL (fndecl) = NULL_TREE;

      /* Allow this decl to be seen in global scope */
      pushdecl (TREE_TYPE (methods));
    }
}

void
do_inline_method_hair (cname, method_list)
     tree cname, method_list;
{
  while (method_list)
    {
      /* Do inline member functions.  */
      tree methods = TREE_VALUE (method_list);
      while (methods)
	{
	  prepare_inline (cname, methods);
	  methods = TREE_CHAIN (methods);
	}
      /* Do inline friend functions.  */
      methods = TREE_TYPE (method_list);
      while (methods)
	{
	  prepare_inline (cname, methods);
	  methods = TREE_CHAIN (methods);
	}
      method_list = TREE_CHAIN (method_list);
    }
}

/* Return non-zero if the effective type of INSTANCE is static.
   Used to determine whether the virtual function table is needed
   or not.  */
int
resolves_to_fixed_type_p (instance)
     tree instance;
{
  switch (TREE_CODE (instance))
    {
    case ADDR_EXPR:
      return resolves_to_fixed_type_p (TREE_OPERAND (instance, 0));
    case VAR_DECL:
    case PARM_DECL:
    case COMPONENT_REF:
      if (IS_AGGR_TYPE (TREE_TYPE (instance)))
	return (! TREE_HAS_METHOD_CALL_OVERLOADED (TREE_TYPE (instance)));
    default:
      return 0;
    }
}

/* Report a argument type mismatch between the best declared function
   we could find and the current argument list that we have.  */
report_type_mismatch (cp, parmtypes, name_kind, err_name)
     struct candidate *cp;
     tree parmtypes;
     char *name_kind, *err_name;
{
  int i = cp->u.bad_arg;
  tree ttf, tta;

  if (i == -2)
    error ("too few arguments for %s `%s'", name_kind, err_name);
  else if (i == -1)
    error ("too many arguments for %s `%s'", name_kind, err_name);
  else
    {
      char buf[OVERLOAD_MAX_LEN];

      ttf = TYPE_ARG_TYPES (TREE_TYPE (cp->function));
      tta = parmtypes;

      while (--i)
	{
	  ttf = TREE_CHAIN (ttf);
	  tta = TREE_CHAIN (tta);
	}
      hack_function_decl (buf, 0, cp->function, 0);
      inline_bufp = inline_buffer;
      dump_type (TREE_TYPE (TREE_VALUE (tta)), &i);
      OB_FINISH ();
      sprintf (inline_bufp, "bad argument %d for function `%s' (type was %s)",
	       cp->u.bad_arg - TREE_METHOD (cp->function), buf, inline_buffer);
      strcpy (buf, inline_bufp);
      error (buf);
    }
}
