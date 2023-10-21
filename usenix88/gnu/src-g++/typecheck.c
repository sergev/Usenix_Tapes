/* Build expressions with type checking for C compiler.
   Copyright (C) 1987, 1988 Free Software Foundation, Inc.
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


/* This file is part of the C front end.
   It contains routines to build C expressions given their operands,
   including computing the types of the result, C-specific error checks,
   and some optimization.

   There are also routines to build RETURN_STMT nodes and CASE_STMT nodes,
   and to process initializations in declarations (since they work
   like a strange sort of assignment).  */

#include "config.h"
#include <stdio.h>
#include "tree.h"
#include "c-tree.h"
#include "flags.h"
#include "overload.h"



void mark_addressable ();
static tree convert_for_assignment ();
static tree convert_for_initialization ();
int compparms ();
int comp_target_types ();
static tree shorten_compare ();
static void binary_op_error ();
static tree pointer_int_sum ();
static tree pointer_diff ();
static tree convert_sequence ();
/* static */ tree unary_complex_lvalue ();
static tree process_init_constructor ();
tree digest_init ();
tree truthvalue_conversion ();
void incomplete_type_error ();
void readonly_warning ();

/* Return the _TYPE node describing the data type
   of the data which NODE represents as a C expression.
   Arrays and functions are converted to pointers
   just as they are when they appear as C expressions.  */

tree
datatype (node)
     tree node;
{
  register tree type = TREE_TYPE (node);
  if (TREE_CODE (type) == ARRAY_TYPE)
    return TYPE_POINTER_TO (TREE_TYPE (type));
  if (TREE_CODE (type) == FUNCTION_TYPE)
    return build_pointer_type (type);
  return type;
}

/* Do `exp = require_complete_type (exp);' to make sure exp
   does not have an incomplete type.  (That includes void types.)  */

tree
require_complete_type (value)
     tree value;
{
  tree type = TREE_TYPE (value);

  /* First, detect a valid value with a complete type.  */
  if (TYPE_SIZE (type) != 0
      && type != void_type_node)
    return value;

  incomplete_type_error (value, type);
  return error_mark_node;
}

/* Print an error message for invalid use of an incomplete type.
   VALUE is the expression that was used (or 0 if that isn't known)
   and TYPE is the type that was invalid.  */

void
incomplete_type_error (value, type)
     tree value;
     tree type;
{
  char *errmsg;

  /* Avoid duplicate error message.  */
  if (TREE_CODE (type) == ERROR_MARK)
    return;

  if (value != 0 && (TREE_CODE (value) == VAR_DECL
		     || TREE_CODE (value) == PARM_DECL))
    error ("`%s' has an incomplete type",
	   IDENTIFIER_POINTER (DECL_NAME (value)));
  else
    {
    retry:
      /* We must print an error message.  Be clever about what it says.  */

      switch (TREE_CODE (type))
	{
	case RECORD_TYPE:
	  errmsg = "invalid use of undefined type `struct %s'";
	  break;

	case UNION_TYPE:
	  errmsg = "invalid use of undefined type `union %s'";
	  break;

	case ENUMERAL_TYPE:
	  errmsg = "invalid use of undefined type `enum %s'";
	  break;

	case VOID_TYPE:
	  error ("invalid use of void expression");
	  return;

	case ARRAY_TYPE:
	  if (TYPE_DOMAIN (type))
	    {
	      type = TREE_TYPE (type);
	      goto retry;
	    }
	  error ("invalid use of array with unspecified bounds");
	  return;

	default:
	  abort ();
	}

      if (TREE_CODE (TYPE_NAME (type)) == IDENTIFIER_NODE)
	error (errmsg, IDENTIFIER_POINTER (TYPE_NAME (type)));
      else
	/* If this type has a typedef-name, the TYPE_NAME is a TYPE_DECL.  */
	error ("invalid use of incomplete typedef `%s'",
	       TYPE_NAME_STRING (type));
    }
}

/* Return a variant of TYPE which has all the type qualifiers of LIKE
   as well as those of TYPE.  */

static tree
qualify_type (type, like)
     tree type, like;
{
  int constflag = TREE_READONLY (type) || TREE_READONLY (like);
  int volflag = TREE_VOLATILE (type) || TREE_VOLATILE (like);
  /* @@ Must do member pointers here.  */
  return build_type_variant (type, constflag, volflag, 0);
}

/* Return the common type of two types.
   We assume that comptypes has already been done and returned 1;
   if that isn't so, this may crash.

   This is the type for the result of most arithmetic operations
   if the operands have the given two types.

   We do not deal with enumeral types here because they have already been
   converted to integer types.  */

tree
commontype (t1, t2)
     tree t1, t2;
{
  register enum tree_code form1;
  register enum tree_code form2;

  /* Save time if the two types are the same.  */

  if (t1 == t2) return t1;

  /* Treat an enum type as the unsigned integer type of the same width.  */

  if (TREE_CODE (t1) == ENUMERAL_TYPE)
    t1 = type_for_size (TYPE_PRECISION (t1), 1);
  if (TREE_CODE (t2) == ENUMERAL_TYPE)
    t2 = type_for_size (TYPE_PRECISION (t2), 1);

  form1 = TREE_CODE (t1);
  form2 = TREE_CODE (t2);

  switch (form1)
    {
    case INTEGER_TYPE:
    case REAL_TYPE:
      /* If only one is real, use it as the result.  */

      if (form1 == REAL_TYPE && form2 != REAL_TYPE)
	return t1;

      if (form2 == REAL_TYPE && form1 != REAL_TYPE)
	return t2;

      /* Both real or both integers; use the one with greater precision.  */

      if (TYPE_PRECISION (t1) > TYPE_PRECISION (t2))
	return t1;
      else if (TYPE_PRECISION (t2) > TYPE_PRECISION (t1))
	return t2;

      /* Same precision.  Prefer longs to ints even when same size.  */

      if (t1 == long_unsigned_type_node
	  || t2 == long_unsigned_type_node)
	return long_unsigned_type_node;

      if (t1 == long_integer_type_node
	  || t2 == long_integer_type_node)
	{
	  /* But preserve unsignedness from the other type,
	     since long cannot hold all the values of an unsigned int.  */
	  if (TREE_UNSIGNED (t1) || TREE_UNSIGNED (t2))
	    return long_unsigned_type_node;
	  return long_integer_type_node;
	}

      /* Otherwise prefer the unsigned one.  */

      if (TREE_UNSIGNED (t1))
	return t1;
      else return t2;

    case POINTER_TYPE:
#if 0
      /* For two pointers, do this recursively on the target type,
	 and combine the qualifiers of the two types' targets.  */
      {
	tree target = commontype (TYPE_MAIN_VARIANT (TREE_TYPE (t1)),
				   TYPE_MAIN_VARIANT (TREE_TYPE (t2)));
	int constp
	  = TREE_READ_ONLY (TREE_TYPE (t1)) || TREE_READ_ONLY (TREE_TYPE (t2));
	int volatilep
	  = TREE_VOLATILE (TREE_TYPE (t1)) || TREE_VOLATILE (TREE_TYPE (t2));
	return build_pointer_type (build_type_variant (target, constp, volatilep, 0));
      }
#endif
      return build_pointer_type (commontype (TREE_TYPE (t1), TREE_TYPE (t2)));

    case ARRAY_TYPE:
      {
	tree elt = commontype (TREE_TYPE (t1), TREE_TYPE (t2));
	/* Save space: see if the result is identical to one of the args.  */
	if (elt == TREE_TYPE (t1) && TYPE_DOMAIN (t1))
	  return t1;
	if (elt == TREE_TYPE (t2) && TYPE_DOMAIN (t2))
	  return t2;
	/* Merge the element types, and have a size if either arg has one.  */
	return build_array_type (elt, TYPE_DOMAIN (TYPE_DOMAIN (t1) ? t1 : t2));
      }

    case FUNCTION_TYPE:
      /* Function types: prefer the one that specified arg types.
	 If both do, merge the arg types.  Also merge the return types.  */
      {
	tree valtype = commontype (TREE_TYPE (t1), TREE_TYPE (t2));
	tree p1 = TYPE_ARG_TYPES (t1);
	tree p2 = TYPE_ARG_TYPES (t2);
	int len;
	tree newargs, n;
	int i;

	/* Save space: see if the result is identical to one of the args.  */
	if (valtype == TREE_TYPE (t1) && ! TYPE_ARG_TYPES (t2))
	  return t1;
	if (valtype == TREE_TYPE (t2) && ! TYPE_ARG_TYPES (t1))
	  return t2;

	/* Simple way if one arg fails to specify argument types.  */
	if (TYPE_ARG_TYPES (t1) == 0)
	  return build_function_type (valtype, TYPE_ARG_TYPES (t2));
	if (TYPE_ARG_TYPES (t2) == 0)
	  return build_function_type (valtype, TYPE_ARG_TYPES (t1));

	/* If both args specify argument types, we must merge the two
	   lists, argument by argument.  */

	len = list_length (p1);
	newargs = 0;

	for (i = 0; i < len; i++)
	  newargs = tree_cons (0, 0, newargs);

	n = newargs;

	for (i = 0; p1;
	     p1 = TREE_CHAIN (p1), p2 = TREE_CHAIN (p2), n = TREE_CHAIN (n), i++)
	  {
	    int any_warning = 0;

	    if (TREE_PURPOSE (p1) && !TREE_PURPOSE (p2))
	      {
#if 0
		if (! any_warning)
		  {
		    warning ("default argument given in prototype and not in declaration of function");
		    any_warning++;
		  }
#endif
		TREE_PURPOSE (p2) = TREE_PURPOSE (p1);
	      }
	    else if (! TREE_PURPOSE (p1))
	      ;
	    else
	      {
		if (!tree_init_cst_equal (TREE_PURPOSE (p1),
					  TREE_PURPOSE (p2)))
		  error ("redeclaration of default argument %d", i);
	      }
	    TREE_PURPOSE (n) = TREE_PURPOSE (p2);
	    TREE_VALUE (n) = commontype (TREE_VALUE (p1), TREE_VALUE (p2));
	  }

	return build_function_type (valtype, newargs);
      }

    case RECORD_TYPE:
    case UNION_TYPE:
    case CLASS_TYPE:
      if (check_base_type (t1, t2))
	return t1;
      error ("commontype called with uncommon aggregate types (compiler error)");
      return t1;

    default:
      return t1;
    }
}

/* Return 1 if TYPE1 and TYPE2 are compatible types for assignment
   or various other operations.  This is what ANSI C speaks of as
   "being the same".

   For C++: argument STRICT says we should be strict about this
   comparison:

	1 : strict (compared according to ANSI C)
	0 : <= (compared according to C++)
	-1: <= or >= (relaxed)

   Otherwise, pointers involving base classes and derived classes
   can be mixed as legal: i.e. a pointer to a base class may be assigned
   to a pointer to one of its derived classes, as per C++. A pointer to
   a derived class may be passed as a paramter to a function expecting a
   pointer to a base classes. These allowances do not commute. In this
   case, TYPE1 is assumed to be the base class, and TYPE2 is assumed to
   be the derived class.  */
int
comptypes (type1, type2, strict)
     tree type1, type2;
     int strict;
{
  register tree t1 = type1;
  register tree t2 = type2;

  /* Suppress errors caused by previously reported errors */

  if (t1 == t2 || TREE_CODE (t1) == ERROR_MARK || TREE_CODE (t2) == ERROR_MARK)
    return 1;

  /* Treat an enum type as the unsigned integer type of the same width.  */

  if (TREE_CODE (t1) == ENUMERAL_TYPE)
    t1 = type_for_size (TYPE_PRECISION (t1), 1);
  if (TREE_CODE (t2) == ENUMERAL_TYPE)
    t2 = type_for_size (TYPE_PRECISION (t2), 1);

  if (t1 == t2)
    return 1;

  if (TREE_CODE (t1) == REFERENCE_TYPE)
    t1 = TREE_TYPE (t1);
  if (TREE_CODE (t2) == REFERENCE_TYPE)
    t2 = TREE_TYPE (t2);

  /* Different classes of types can't be compatible.  */

  if (TREE_CODE (t1) != TREE_CODE (t2)) return 0;

  switch (TREE_CODE (t1))
    {
    case RECORD_TYPE:
    case UNION_TYPE:
      if (TYPE_MAIN_VARIANT (t1) == TYPE_MAIN_VARIANT (t2))
	return 1;
      if (strict <= 0)
	goto look_hard;
      return 0;

    case POINTER_TYPE:
      if (TREE_MEMBER_POINTER (t1) != TREE_MEMBER_POINTER (t2))
	return 0;
      if (TREE_MEMBER_POINTER (t1)
	  && ! comptypes (TYPE_DOMAIN (t1), TYPE_DOMAIN (t2), strict))
	return 0;
      t1 = TREE_TYPE (t1);
      t2 = TREE_TYPE (t2);
      if (t1 == t2)
	return 1;
      if (TREE_CODE (t1) == VOID_TYPE
	  || TREE_CODE (t2) == VOID_TYPE)
	return 1;
      if (strict <= 0)
	{
	  if (IS_AGGR_TYPE (t1) && IS_AGGR_TYPE (t2))
	    {
	      tree tt1, tt2;
  look_hard:
	      t1 = TYPE_MAIN_VARIANT (t1);
	      t2 = TYPE_MAIN_VARIANT (t2);
	      tt1 = t1, tt2 = t2;
	      while (TYPE_BASELINK (t2))
		{
#if 0
		  if (! TREE_VIA_PUBLIC (t2))
		    return 0;
#endif
		  t2 = TYPE_BASELINK (t2);
		  if (t1 == t2)
		    return 1;
		}
	      if (strict < 0)
		{
		  while (TYPE_BASELINK (tt1))
		    {
#if 0
		      if (! TREE_VIA_PUBLIC (tt1))
			return 0;
#endif
		      tt1 = TYPE_BASELINK (tt1);
		      if (tt1 == tt2)
			return -1;
		    }
		}
	    }
	  return 0;
	}
      else
	return comptypes (t1, t2, strict);

    case FUNCTION_TYPE:
      return ((TREE_TYPE (t1) == TREE_TYPE (t2)
	       || comptypes (TREE_TYPE (t1), TREE_TYPE (t2), strict))
	      && compparms (TYPE_ARG_TYPES (t1), TYPE_ARG_TYPES (t2), strict));

    case ARRAY_TYPE:
      /* Target types must match.  */
      if (!(TREE_TYPE (t1) == TREE_TYPE (t2)
	    || comptypes (TREE_TYPE (t1), TREE_TYPE (t2), strict)))
	return 0;
      {
	tree d1 = TYPE_DOMAIN (t1);
	tree d2 = TYPE_DOMAIN (t2);

	/* Sizes must match unless one is missing or variable.  */
	if (d1 == 0 || d2 == 0 || d1 == d2
	    || TREE_CODE (TYPE_MIN_VALUE (d1)) != INTEGER_CST
	    || TREE_CODE (TYPE_MIN_VALUE (d2)) != INTEGER_CST
	    || TREE_CODE (TYPE_MAX_VALUE (d1)) != INTEGER_CST
	    || TREE_CODE (TYPE_MAX_VALUE (d2)) != INTEGER_CST)
	  return 1;

	return ((TREE_INT_CST_LOW (TYPE_MIN_VALUE (d1))
		 == TREE_INT_CST_LOW (TYPE_MIN_VALUE (d2)))
		&& (TREE_INT_CST_HIGH (TYPE_MIN_VALUE (d1))
		    == TREE_INT_CST_HIGH (TYPE_MIN_VALUE (d2)))
		&& (TREE_INT_CST_LOW (TYPE_MAX_VALUE (d1))
		    == TREE_INT_CST_LOW (TYPE_MAX_VALUE (d2)))
		&& (TREE_INT_CST_HIGH (TYPE_MAX_VALUE (d1))
		    == TREE_INT_CST_HIGH (TYPE_MAX_VALUE (d2))));
      }
    }
  return 0;
}

/* Return 1 if TTL and TTR are pointers to types that are equivalent,
   ignoring their qualifiers.

   C++: add the ability to assign a pointer to a base class to a
   pointer to a derived class.  */

int
comp_target_types (ttl, ttr)
     tree ttl, ttr;
{
  ttl = TYPE_MAIN_VARIANT (ttl);
  ttr = TYPE_MAIN_VARIANT (ttr);
  if (ttl == ttr)
    return 1;

  if (TREE_CODE (ttr) != TREE_CODE (ttl))
    return 0;

  if (TREE_CODE (ttr) == ARRAY_TYPE || TREE_CODE (ttr) == FUNCTION_TYPE
      || (TREE_CODE (ttr) == POINTER_TYPE
	  && TREE_MEMBER_POINTER (ttl) == TREE_MEMBER_POINTER (ttr)))
    {
      if (comp_target_types (TREE_TYPE (ttl), TREE_TYPE (ttr)))
	return (TREE_MEMBER_POINTER (ttl) ?
		comp_target_types (TYPE_DOMAIN (ttl), TYPE_DOMAIN (ttr)) : 1);
    }

  /* for C++ */
  else if (IS_AGGR_TYPE (ttl))
    return comptypes (TYPE_POINTER_TO (ttl), TYPE_POINTER_TO (ttr), 0);

  return 0;
}

/* Subroutines of `comptypes'.  */

/* Return 1 if two parameter type lists PARMS1 and PARMS2
   are equivalent in the sense that functions with those parameter types
   can have equivalent types.
   If either list is empty, we win.
   Otherwise, the two lists must be equivalent, element by element.

   C++: See comment above about TYPE1, TYPE2, STRICT.  */
int
compparms (parms1, parms2, strict)
     tree parms1, parms2;
     int strict;
{
  register tree t1 = parms1, t2 = parms2;

  /* An unspecified parmlist matches any specified parmlist
     whose argument types don't need default promotions.  */

  if (t1 == 0)
    return compparms1 (t2);
  if (t2 == 0)
    return compparms1 (t1);

  while (1)
    {
      if (t1 == 0 && t2 == 0)
	return 1;
      /* If one parmlist is shorter than the other,
	 they fail to match, unless STRICT is <= 0.  */
      if (t1 == 0 || t2 == 0)
	{
	  if (strict > 0)
	    return 0;
	  if (strict == 0)
	    return t1 && TREE_PURPOSE (t1);
	  return (t1 && TREE_PURPOSE (t1) || TREE_PURPOSE (t2));
	}
      if (! comptypes (TREE_VALUE (t1), TREE_VALUE (t2), strict))
	{
	  if (strict > 0)
	    return 0;
	  if (strict == 0)
	    return TREE_VALUE (t2) == void_type_node && TREE_PURPOSE (t1);
	  return TREE_PURPOSE (t1) || TREE_PURPOSE (t2);
	}
      if (TREE_PURPOSE (t1) && TREE_PURPOSE (t2))
	{
	  if (! tree_init_cst_equal (TREE_PURPOSE (t1), TREE_PURPOSE (t2)))
	    return 0;
	}

      t1 = TREE_CHAIN (t1);
      t2 = TREE_CHAIN (t2);
    }
}

/* Return 1 if PARMS specifies a fixed number of parameters
   and none of their types is affected by default promotions.  */

int
compparms1 (parms)
     tree parms;
{
  register tree t;
  for (t = parms; t; t = TREE_CHAIN (t))
    {
      register tree type = TREE_VALUE (t);

      if (TREE_CHAIN (t) == 0 && type != void_type_node)
	return 0;

      if (type == float_type_node)
	return 0;

      if (TREE_CODE (type) == INTEGER_TYPE
	  && TYPE_PRECISION (type) < TYPE_PRECISION (integer_type_node))
	return 0;
    }
  return 1;
}

/* Return an unsigned type the same as TYPE in other respects.

 C++: must make these work for type variants as well.  */

tree
unsigned_type (type)
     tree type;
{
  type = TYPE_MAIN_VARIANT (type);

  if (type == signed_char_type_node || type == char_type_node)
    return unsigned_char_type_node;
  if (type == integer_type_node)
    return unsigned_type_node;
  if (type == short_integer_type_node)
    return short_unsigned_type_node;
  if (type == long_integer_type_node)
    return long_unsigned_type_node;
  return type;
}

/* Return a signed type the same as TYPE in other respects.  */

tree
signed_type (type)
     tree type;
{
  if (type == unsigned_char_type_node || type == char_type_node)
    return signed_char_type_node;
  if (type == unsigned_type_node)
    return integer_type_node;
  if (type == short_unsigned_type_node)
    return short_integer_type_node;
  if (type == long_unsigned_type_node)
    return long_integer_type_node;
  return type;
}

/* Return a type the same as TYPE except unsigned or
   signed according to UNSIGNEDP.  */

tree
signed_or_unsigned_type (unsignedp, type)
     int unsignedp;
     tree type;
{
  if (TREE_CODE (type) != INTEGER_TYPE)
    return type;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (signed_char_type_node))
    return unsignedp ? unsigned_char_type_node : signed_char_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (integer_type_node)) 
    return unsignedp ? unsigned_type_node : integer_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (short_integer_type_node)) 
    return unsignedp ? short_unsigned_type_node : short_integer_type_node;
  if (TYPE_PRECISION (type) == TYPE_PRECISION (long_integer_type_node)) 
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;
  return type;
}

/* Return an integer type with BITS bits of precision,
   that is unsigned if UNSIGNEDP is nonzero, otherwise signed.  */

tree
type_for_size (bits, unsignedp)
     int bits;
     int unsignedp;
{
  if (bits <= TYPE_PRECISION (signed_char_type_node))
    return unsignedp ? unsigned_char_type_node : signed_char_type_node;

  if (bits <= TYPE_PRECISION (short_integer_type_node))
    return unsignedp ? short_unsigned_type_node : short_integer_type_node;

  if (bits <= TYPE_PRECISION (integer_type_node))
    return unsignedp ? unsigned_type_node : integer_type_node;

  if (bits <= TYPE_PRECISION (long_integer_type_node))
    return unsignedp ? long_unsigned_type_node : long_integer_type_node;

  return 0;
}

tree
get_floating_type (mode)
     enum machine_mode mode;
{
  if (mode == SFmode)
    return float_type_node;
  if (mode == DFmode)
    return double_type_node;
  abort ();
}

tree
c_sizeof (type)
     tree type;
{
  enum tree_code code = TREE_CODE (type);

  if (code == FUNCTION_TYPE)
    {
      if (pedantic)
	warning ("sizeof applied to a function type");
      return build_int (1);
    }
  if (code == VOID_TYPE)
    {
      if (pedantic)
	warning ("sizeof applied to a void type");
      return build_int (1);
    }
  /* C++: this is really correct!  */
  if (code == REFERENCE_TYPE)
    type = TREE_TYPE (type);

  return size_in_bytes (type);
}

tree
c_sizeof_nowarn (type)
     tree type;
{
  enum tree_code code = TREE_CODE (type);

  if (code == FUNCTION_TYPE
      || code == VOID_TYPE)
    return build_int (1);
  if (code == REFERENCE_TYPE)
    type = TREE_TYPE (type);

  return size_in_bytes (type);
}

/* Implement the __alignof keyword: Return the minimum required
   alignment of TYPE, measured in bytes.  */

tree
c_alignof (type)
     tree type;
{
  enum tree_code code = TREE_CODE (type);

  if (pedantic)
    warning ("ANSI C does not allow `__alignof'");

  if (code == FUNCTION_TYPE)
    return build_int (FUNCTION_BOUNDARY / BITS_PER_UNIT);

  if (code == VOID_TYPE)
    return build_int (1);
  /* C++: this is really correct!  */
  if (code == REFERENCE_TYPE)
    type = TREE_TYPE (type);

  return build_int (TYPE_ALIGN (type) / BITS_PER_UNIT);
}

/* Perform default promotions for C data used in expressions.
   Arrays and functions are converted to pointers;
   enumeral types or short or char, to int.
   In addition, manifest constants symbols are replaced by their values.  */

tree
default_conversion (exp)
     tree exp;
{
  register tree dt = TREE_TYPE (exp);
  register enum tree_code form = TREE_CODE (dt);

  if (TREE_CODE (exp) == CONST_DECL)
    exp = DECL_INITIAL (exp);
  else
    {
      exp = maybe_convert_decl_to_const (exp);
      dt = TREE_TYPE (exp);
    }

  if (form == ENUMERAL_TYPE
      || (form == INTEGER_TYPE
	  && (TYPE_PRECISION (dt)
	      < TYPE_PRECISION (integer_type_node))))
    {
      /* Traditionally, unsignedness is preserved in default promotions.  */
      if (flag_traditional && TREE_UNSIGNED (dt))
	return convert (unsigned_type_node, exp);
      return convert (integer_type_node, exp);
    }
  if (flag_traditional && dt == float_type_node)
    return convert (double_type_node, exp);
  if (form == VOID_TYPE)
    {
      error ("void value not ignored as it ought to be");
      return error_mark_node;
    }
  if (form == FUNCTION_TYPE)
    {
      if (TREE_CODE (exp) == FUNCTION_DECL)
	TREE_ADDRESSABLE (exp) = 1;
      return build_unary_op (ADDR_EXPR, exp, 0);
    }
  if (form == ARRAY_TYPE)
    {
      register tree adr;
      if (TREE_CODE (exp) == INDIRECT_REF)
	return convert (TYPE_POINTER_TO (TREE_TYPE (dt)),
			TREE_OPERAND (exp, 0));

      if (TREE_CODE (exp) == COMPOUND_EXPR)
	{
	  tree op1 = default_conversion (TREE_OPERAND (exp, 1));
	  return build (COMPOUND_EXPR, TREE_TYPE (op1),
			TREE_OPERAND (exp, 0), op1);
	}

      if (!lvalue_p (exp)
	  && ! (TREE_CODE (exp) == CONSTRUCTOR && TREE_STATIC (exp)))
	{
	  error ("invalid use of non-lvalue array");
	  return error_mark_node;
	}

/* ??? This is not really quite correct
   in that the type of the operand of ADDR_EXPR
   is not the target type of the type of the ADDR_EXPR itself.
   Question is, can this lossage be avoided?  */
      adr = build (ADDR_EXPR, TYPE_POINTER_TO (TREE_TYPE (dt)), exp);
      mark_addressable (exp);
      TREE_LITERAL (adr) = staticp (exp);
      TREE_VOLATILE (adr) = 0;   /* Default would be, same as EXP.  */
      return adr;
    }
  if (form == REFERENCE_TYPE)
    {
      return bash_reference_type (exp);
    }
  return exp;
}

tree
build_x_arrow (datum)
     tree datum;
{
  tree types_memoized = NULL_TREE;
  tree rval = datum;
  tree last_rval = datum;

  if (datum == error_mark_node)
    return error_mark_node;

  while (rval = build_opfncall (COMPONENT_REF, rval))
    {
      if (rval == error_mark_node)
	return error_mark_node;

      if (value_member (TREE_TYPE (rval), types_memoized))
	{
	  error ("circular pointer delegation detected");
	  return error_mark_node;
	}
      else
	{
	  types_memoized = tree_cons (NULL_TREE, TREE_TYPE (rval),
				      types_memoized);
	}
      last_rval = rval;
    }

  if (TREE_CODE (TREE_TYPE (last_rval)) == REFERENCE_TYPE)
    last_rval = bash_reference_type (last_rval);

  if (TREE_CODE (TREE_TYPE (last_rval)) == POINTER_TYPE)
    return build_indirect_ref (last_rval);
  if (types_memoized)
    error ("result of `operator->()' yields non-pointer result");
  else
    error ("base operand of `->' is not a pointer");
  return error_mark_node;
}

/* Make an expression to refer to the COMPONENT field of
   structure or union value DATUM.  COMPONENT is an IDENTIFIER_NODE.

   For C++, COMPONENT may be a TREE_LIST.  This happens when we must
   return a pointer to a method of the current class, but there is not
   yet enough typing information to know which one.  As a special
   case, if there is only one method by that name, it is returned.
   Otherwise we return an expression which other routines will
   have to know how to deal with later.  */
tree
build_m_component_ref (datum, component)
     tree datum, component;
{
  tree type = TREE_TYPE (component);
  tree rval;

  if (datum == error_mark_node
      || component == error_mark_node)
    return error_mark_node;

  if (! TREE_MEMBER_POINTER (type))
    {
      error ("invalid type for method/member pointer");
      return error_mark_node;
    }
  if (! comptypes (TYPE_DOMAIN (type), TREE_TYPE (datum), 0))
    {
      error ("m_component: cannot convert to method/member pointer");
      return error_mark_node;
    }
  /* If the component is a data element (or
     a virtual function), we play games here
     to make things work.  */

  if (TREE_CODE (TREE_TYPE (type)) == FUNCTION_TYPE)
    {
      tree e0, e1, e3;

      if (TREE_VIRTUAL (TREE_TYPE (datum)))
	{
	  tree vtbl_type = build_pointer_type (TREE_TYPE (type));

	  e0 = build (NOP_EXPR, integer_type_node, component);

#ifdef VTABLE_USES_MASK
	  /* If we are willing to limit the number of
	     virtual functions a class may have to some
	     *small* number, then if, for a function address,
	     we are passed some small number, we know that
	     it is a virtual function index, and work from there.  */
	  e1 = build (BIT_AND_EXPR, integer_type_node, e0, vtbl_mask);
#else
	  /* There is a hack here that takes advantage of
	     twos complement arithmetic, and the fact that
	     there are more than one UNITS to the WORD.
	     If the high bit is set for the `function',
	     then we pretend it is a virtual function,
	     and the array indexing will knock this bit
	     out the top, leaving a valid index.  */
#if UNITS_PER_WORD <= 1
	  virtual_functions_lose !;
#endif
	  e1 = build (GT_EXPR, integer_type_node, e0, integer_zero_node);
#endif
	  e3 = build_component_ref (datum, get_vfield_name (TREE_TYPE (datum)), 1);
	  e3 = build_array_ref (build (NOP_EXPR, build_pointer_type (vtbl_type), e3), e0);

	  if (e3 == error_mark_node)
	    abort ();

	  TREE_TYPE (e3) = TREE_TYPE (component);
	  component = build_conditional_expr (e1, component, e3);
	}
      /* Return a pointer to member construct.  This can
	 be distinguished because its TYPE field will
	 have its pointer-to-member bit set.  */
      rval = build (SCOPE_REF, type, datum, component);
    }
  else
    {
      tree e0, e1, e2;
      e0 = build (NOP_EXPR, integer_type_node,
		  build_unary_op (ADDR_EXPR, datum, 0));
      e1 = build (NOP_EXPR, integer_type_node, component);
      e2 = build (PLUS_EXPR, type, e0, e1);
      rval = build_indirect_ref (e2);
    }
  return rval;
}

tree
build_component_ref (datum, component, protect)
     tree datum, component;
     int protect;
{
  tree basetypes = NULL_TREE;
  register tree basename = datum;
  register tree basetype = TREE_TYPE (basename);
  register enum tree_code form = TREE_CODE (basetype);
  register tree field = NULL, fields;
  register tree ref;
  enum visibility_type visibility;
  int can_be_private, can_be_protected, via_public = 1;

  if (form == REFERENCE_TYPE)
    {
      datum = bash_reference_type (datum);
      basename = datum;
      basetype = TREE_TYPE (basename);
      form = TREE_CODE (basetype);
    }

  /* First, see if there is a field or component with name COMPONENT. */
  if (TREE_CODE (component) == TREE_LIST)
    {
      if (TREE_CHAIN (component) == NULL_TREE
	  && TREE_CHAIN (TREE_VALUE (component)) == NULL_TREE)
	{
	  component = TREE_PURPOSE (TREE_VALUE (component));
	  fatal ("bug in build_component_ref!");
	  /* NOTREACHED */
	}
      return build (COMPONENT_REF, TREE_TYPE (component), basename, component);
    }
  if (TREE_CODE (component) == TYPE_EXPR)
    {
      return build_component_type_expr (datum, component, protect);
    }

  if (! IS_AGGR_TYPE_CODE (form))
    {
      if (form != ERROR_MARK)
	error ("request for member `%s' in something not a class, structure or union",
	       IDENTIFIER_POINTER (component));
      return error_mark_node;
    }

  if (TYPE_SIZE (basetype) == 0)
    {
      incomplete_type_error (0, basetype);
      return error_mark_node;
    }

  visibility = visibility_public;
  can_be_private = 0;
  can_be_protected = !! (int)IDENTIFIER_CLASS_VALUE (component);

  /* Look up component name in the structure type definition.  */

  /* Private members can only be accessed by friends of the class
     they are defined in.  Protected members can be accessed by
     classes derived from classes that they are derived in.  */
  while (basetype)
    {
      for (field = TYPE_FIELDS (basetype); field; field = TREE_CHAIN (field))
	{
	  if (DECL_NAME (field) == component)
	    {
	      tree btypes = basetypes, vtypes = DECL_VISIBILITY (field);
	      tree member;
	      enum visibility_type this_v = visibility_default;

	      if (vtypes)
		while (btypes)
		  {
		    member = purpose_member (TREE_VALUE (btypes), vtypes);
		    if (member)
		      {
			this_v = (enum visibility_type)TREE_VALUE (member);
			break;
		      }
		    btypes = TREE_CHAIN (btypes);
		  }
	      if (protect)
		{
		  if (this_v != visibility_default)
		    visibility = this_v;
		  else if (TREE_PRIVATE (field))
		    {
		      visibility = visibility_private;
		      can_be_private =
			(basetype == current_class_type
			 || is_friend (current_function_decl, basetype)
			 || purpose_member (basetype, DECL_VISIBILITY (field)));
		    }
		  else if (TREE_PROTECTED (field))
		    {
		      visibility = visibility_protected;
		      can_be_protected =
			(can_be_protected
			 || is_friend (current_function_decl, basetype)
			 || purpose_member (basetype, DECL_VISIBILITY (field)));
		    }
		}
	      goto found;
	    }
	}
      if (protect)
	{
	  via_public &= TREE_VIA_PUBLIC (basetype);

	  if (TREE_VIA_PROTECTED (basetype))
	    visibility = visibility_protected;
	  else if (basetype != current_class_type && via_public == 0)
	    visibility = visibility_private;
	}
      basetypes = tree_cons (NULL_TREE, basetype, basetypes);
      basetype = TYPE_BASELINK (basetype);
    }

  /* Not found as a data field, look for it as a method.  If found,
     then if this is the only possible one, return it, else
     report ambiguity error.  */
  visibility = visibility_public;
  can_be_private = 0;
  can_be_protected = !! (int)IDENTIFIER_CLASS_VALUE (component);

  basetype = TREE_TYPE (basename);
  basetypes = NULL_TREE;
  fields = NULL_TREE;	/* Current list of possible methods */
  via_public = 1;
  while (basetype)
    {
      for (field = TYPE_FN_FIELDS (basetype); field; field = TREE_CHAIN (field))
	{
	  if (TREE_PURPOSE (field) == component)
	    {
	      tree tmp = build_tree_list (basetype, field);
	      TREE_TYPE (tmp) = unknown_type_node;
	      TREE_VIA_PUBLIC (tmp) = via_public;
	      fields = chainon (fields, tmp);
	    }
	}
      via_public &= TREE_VIA_PUBLIC (basetype);
      basetypes = tree_cons (NULL_TREE, basetype, basetypes);
      basetype = TYPE_BASELINK (basetype);
    }

  if (fields)
    {
      if (TREE_CHAIN (fields) == NULL_TREE
	  && TREE_CHAIN (TREE_VALUE (TREE_VALUE (fields))) == NULL_TREE)
	{
	  tree btypes = basetypes, vtypes;
	  tree member;
	  enum visibility_type this_v = visibility_default;

	  /* Unique, so use this one now.  */
	  basetype = TREE_PURPOSE (fields);
	  field = TREE_VALUE (TREE_VALUE (fields));
	  vtypes = DECL_VISIBILITY (field);
	  if (vtypes)
	    while (btypes)
	      {
		member = purpose_member (TREE_VALUE (btypes), vtypes);
		if (member)
		  {
		    this_v = (enum visibility_type)TREE_VALUE (member);
		    break;
		  }
		btypes = TREE_CHAIN (btypes);
	      }
	  if (protect)
	    {
	      if (this_v != visibility_default)
		visibility = this_v;
	      else if (TREE_PRIVATE (field))
		{
		  visibility = visibility_private;
		  can_be_private =
		    (basetype == current_class_type
		     || is_friend (current_function_decl, basetype)
		     || purpose_member (basetype, DECL_VISIBILITY (field)));
		}
	      else if (TREE_PROTECTED (field))
		{
		  visibility = visibility_protected;
		  can_be_protected =
		    (can_be_protected
		     || is_friend (current_function_decl, basetype)
		     || purpose_member (basetype, DECL_VISIBILITY (field)));
		}
	    }
	  goto found;
	}
      else
	return build (COMPONENT_REF, TREE_TYPE (fields), basename, fields);
    }

  error (form == RECORD_TYPE
	 ? "structure has no member named `%s'"
	 : "union has no member named `%s'",
	 IDENTIFIER_POINTER (component));
  return error_mark_node;

 found:

  if (visibility != visibility_public && VFIELD_NAME_P (DECL_NAME (field)))
    visibility = visibility_public;
  else if (visibility == visibility_private && ! can_be_private)
    {
      error (TREE_PRIVATE (field)
	     ? "component `%s' is private"
	     : "component `%s' is from private base class",
	     IDENTIFIER_POINTER (component));
      return error_mark_node;
    }
  else if (visibility == visibility_protected && ! can_be_protected)
    {
      error (TREE_PROTECTED (field)
	     ? "component `%s' is protected"
	     : "component `%s' is from protected base class",
	     IDENTIFIER_POINTER (component));
      return error_mark_node;
    }

  if (TREE_CODE (TREE_TYPE (field)) == FUNCTION_DECL)
    {
      warning ("methods cannot be converted to function pointers");
      return TREE_TYPE (field);
    }

  if (TREE_STATIC (field))
    {
      tree t = DECL_STATIC_NAME (field);
      ref = IDENTIFIER_GLOBAL_VALUE (t); /* the overloaded name */
      return ref;
    }
  else
    ref = build (COMPONENT_REF, TREE_TYPE (field), basename, field);

  if (TREE_READONLY (basename) || TREE_READONLY (field))
    TREE_READONLY (ref) = 1;
  if (TREE_THIS_VOLATILE (basename) || TREE_VOLATILE (field))
    TREE_THIS_VOLATILE (ref) = 1;

  return ref;
}

/* Given an expression PTR for a pointer, return an expression
   for the value pointed to.
   ERRORSTRING is the name of the operator to appear in error messages.

   This function may need to overload OPERATOR_FNNAME.
   Must also handle REFERENCE_TYPEs for C++.  */

tree
build_x_indirect_ref (ptr, errorstring)
     tree ptr;
     char *errorstring;
{
  tree rval = build_opfncall (INDIRECT_REF, ptr);

  if (rval) return rval;
  return build_indirect_ref (ptr, errorstring);
}

tree
build_indirect_ref (ptr, errorstring)
     tree ptr;
     char *errorstring;
{
  register tree pointer = default_conversion (ptr);
  register tree dt = TREE_TYPE (pointer);

  if (ptr == current_class_decl)
    return C_C_D;

  if (TREE_CODE (dt) == POINTER_TYPE || TREE_CODE (dt) == REFERENCE_TYPE)
    if (TREE_CODE (pointer) == ADDR_EXPR
	&& (TREE_TYPE (TREE_OPERAND (pointer, 0))
	    == TREE_TYPE (dt)))
      return TREE_OPERAND (pointer, 0);
    else
      {
	tree t = TREE_TYPE (dt);
	register tree ref = build (INDIRECT_REF,
				   TYPE_MAIN_VARIANT (t), pointer);

	TREE_READONLY (ref) = TREE_READONLY (t);
	TREE_VOLATILE (ref) = TREE_VOLATILE (t) || TREE_VOLATILE (pointer);
	TREE_THIS_VOLATILE (ref) = TREE_VOLATILE (t);
	return ref;
      }
  else if (TREE_CODE (pointer) != ERROR_MARK)
    error ("invalid type argument of `%s'", errorstring);
  return error_mark_node;
}

/* This handles expressions of the form "a[i]", which denotes
   an array reference.

   This is logically equivalent in C to *(a+i), but we may do it differently.
   If A is a variable or a member, we generate a primitive ARRAY_REF.
   This avoids forcing the array out of registers, and can work on
   arrays that are not lvalues (for example, members of structures returned
   by functions).

   If INDEX is of some user-defined type, it must be converted to
   integer type.  Otherwise, to make a compatible PLUS_EXPR, it
   will inherit the type of the array, which will be some pointer type.  */

tree
build_x_array_ref (array, index)
     tree array, index;
{
  tree rval;

  rval = build_opfncall (ARRAY_REF, array, array);
  if (rval)
    return rval;
  return build_array_ref (array, index);
}

tree
build_array_ref (array, index)
     tree array, index;
{
  tree itype;

  if (index == 0)
    {
      error ("subscript missing in array reference");
      return error_mark_node;
    }

  itype = TREE_TYPE (index);
  if (TREE_CODE (itype) == REFERENCE_TYPE)
    {
      index = bash_reference_type (index);
      itype = TREE_TYPE (index);
    }

  if (IS_AGGR_TYPE (itype))
    if (TREE_HAS_INT_CONVERSION (TYPE_MAIN_VARIANT (itype)))
      index = build_type_conversion (integer_type_node, index, 1);
    else
      {
	error ("type `%s' requires integer conversion for array indexing",
	       TYPE_NAME_STRING (TREE_TYPE (index)));
	return error_mark_node;
      }

  if (TREE_CODE (TREE_TYPE (array)) == ARRAY_TYPE
      && TREE_CODE (array) != INDIRECT_REF)
    {
      index = default_conversion (index);
      if (TREE_CODE (TREE_TYPE (index)) != INTEGER_TYPE)
	{
	  error ("array subscript is not an integer");
	  return error_mark_node;
	}

      /* An array that is indexed by a non-constant
	 cannot be stored in a register; we must be able to do
	 address arithmetic on its address.
	 Likewise an array of elements of variable size.  */
      if (TREE_CODE (index) != INTEGER_CST
	  || TREE_CODE (TYPE_SIZE (TREE_TYPE (TREE_TYPE (array)))) != INTEGER_CST)
	mark_addressable (array);

      if (pedantic && !lvalue_p (array))
	warning ("ANSI C forbids subscripting non-lvalue array");

      return require_complete_type (build (ARRAY_REF,
					   TREE_TYPE (TREE_TYPE (array)),
					   array, index));
    }

  return build_indirect_ref (build_binary_op (PLUS_EXPR, array, index),
			     "array indexing");
}

/* Build a function call to function FUNCTION with parameters PARAMS.
   PARAMS is a list--a chain of TREE_LIST nodes--in which the
   TREE_VALUE of each node is a parameter-expression.
   FUNCTION's data type may be a function type or a pointer-to-function.

   For C++: If FUNCTION's data type is a TREE_LIST, then the tree list
   is the list of possible methods that FUNCTION could conceivably
   be.  If the list of methods comes from a class, then it will be
   a list of lists (where each element is associated with the class
   that produced it), otherwise it will be a simple list (for
   functions overloaded in global scope).

   In the first case, TREE_VALUE (function) is the head of one of those
   lists, and TREE_PURPOSE is the name of the function.

   In the second case, TREE_PURPOSE (function) is the function's
   name directly.

   DECL is the class instance variable, usually CURRENT_CLASS_DECL.  */

tree
build_x_function_call (function, params, decl)
     tree function, params, decl;
{
  tree rval;
  tree cname = current_class_name;
  tree ctype = current_class_type;

  if (TREE_CODE (function) == SCOPE_REF)
    {
      /* This is a call to a function via a member pointer.  */
      ctype = TREE_TYPE (function);
      if (! TREE_MEMBER_POINTER (ctype))
	abort ();
      /* Get the domain of the member pointer.  This will be used
	 to cast our `this' pointer to, if everything else is ok.  */
      ctype = TYPE_DOMAIN (ctype);
      cname = DECL_NAME (TYPE_NAME (ctype));

      if (TREE_OPERAND (function, 0))
	decl = TREE_OPERAND (function, 0);

      if (decl)
	{
	  function = TREE_OPERAND (function, 1);
	  ctype = build_pointer_type (ctype);
	  if (TREE_CODE (TREE_TYPE (decl)) != POINTER_TYPE)
	    {
	      /* @@ Should we hassle with const, etc. here?  */
	      decl = build_unary_op (ADDR_EXPR, decl, 0);
	      TREE_TYPE (decl) = ctype;
	    }
	  else
	    decl = build_c_cast (ctype, decl);
	  if (TREE_CODE (function) == FUNCTION_DECL)
	    {
	      /* Differs from default_conversion by not setting TREE_ADDRESSABLE
		 (because calling an inline function does not mean the function
		 needs to be separately compiled).  */
	      function = build (ADDR_EXPR, build_pointer_type (TREE_TYPE (function)),
				function);
	    }
	  else
	    function = default_conversion (function);

	  if (TREE_CODE (TREE_OPERAND (function, 0)) == TREE_LIST)
	    {
	      /* @@ May be able to perform some type instantiation here.  */
	      sorry ("type instantiation on member pointers");
	      return error_mark_node;
	    }
	  else
	    {
	      return build_function_call (function,
					  tree_cons (NULL_TREE, decl, params));
	    }
	}
      else
	{
	  error ("cannot resolve types outside of class scope (no appropriate value for `this')");
	  return error_mark_node;
	}
    }

  /* Handle methods, friends, and overloaded functions, respectively.  */
  if (TREE_METHOD (function))
    {
      if (TREE_CODE (function) == FUNCTION_DECL)
	function = DECL_ORIGINAL_NAME (function);
      else if  (TREE_CODE (TREE_VALUE (function)) == TREE_LIST)
	function = TREE_PURPOSE (TREE_VALUE (function));
      else
	function = TREE_PURPOSE (function);

      /* this is an abbreviated method call.
         must go through here in case it is a virtual function.
	 @@ Perhaps this could be optimized.  */
      return build_classfn_ref (decl, function, params, 1, 1);
    }
  else if (TREE_CODE (function) == COMPONENT_REF
	   && TREE_TYPE (function) == unknown_type_node)
    {
      function = TREE_PURPOSE (TREE_OPERAND (function, 1));
      return build_classfn_ref (decl, function, params, 1, 1);
    }
  else if (TREE_CODE (function) == TREE_LIST)
    {
      function = TREE_PURPOSE (function);
      return do_actual_overload (function, params, 1, 0);
    }

  {
    tree type = TREE_TYPE (function);
    if (TREE_CODE (type) == REFERENCE_TYPE)
      type = TREE_TYPE (type);

    if (TREE_HAS_CALL_OVERLOADED (TYPE_MAIN_VARIANT (type)))
      return build_opfncall (CALL_EXPR, function, params);
  }

  return build_function_call (function, params);
}

tree
build_function_call (function, params)
     tree function, params;
{
  register tree fntype, fndecl;
  register tree value_type;
  register tree coerced_params;
  tree actualparameterlist ();
  int is_method;
  
  if (TREE_CODE (function) == FUNCTION_DECL)
    {
      fndecl = function;
      is_method = TREE_METHOD (function);
    }
  else
    {
      fndecl = NULL_TREE;
      is_method = 0;
    }

  /* Convert anything with function type to a pointer-to-function.  */
  if (TREE_INLINE (function) && TREE_CODE (function) == FUNCTION_DECL)
    {
      /* Differs from default_conversion by not setting TREE_ADDRESSABLE
	 (because calling an inline function does not mean the function
	 needs to be separately compiled).  */
      function = build (ADDR_EXPR, build_pointer_type (TREE_TYPE (function)),
			function);
    }
  else
    function = default_conversion (function);

  fntype = TREE_TYPE (function);

  if (TREE_CODE (fntype) == ERROR_MARK)
    return error_mark_node;

  if (!(TREE_CODE (fntype) == POINTER_TYPE
	&& TREE_CODE (TREE_TYPE (fntype)) == FUNCTION_TYPE))
    {
      error ("called object is not a function");
      return error_mark_node;
    }

  /* fntype now gets the type of function pointed to.  */
  fntype = TREE_TYPE (fntype);

  /* Convert the parameters to the types declared in the
     function prototype, or apply default promotions.  */

  coerced_params = actualparameterlist (TYPE_ARG_TYPES (fntype), params, fndecl);

  /* Recognize certain built-in functions so we can make tree-codes
     other than CALL_EXPR.  We do this when it enables fold-const.c
     to do something useful.  */

  if (TREE_CODE (function) == ADDR_EXPR
      && TREE_CODE (TREE_OPERAND (function, 0)) == FUNCTION_DECL)
    switch (DECL_FUNCTION_CODE (TREE_OPERAND (function, 0)))
      {
      case BUILT_IN_ABS:
      case BUILT_IN_LABS:
      case BUILT_IN_FABS:
	if (coerced_params == 0)
	  return integer_zero_node;
	return build_unary_op (ABS_EXPR, TREE_VALUE (coerced_params), 0);
      }

  value_type = TREE_TYPE (fntype) ? TREE_TYPE (fntype) : void_type_node;

  if (is_method)
    {
      tree parm = TREE_VALUE (coerced_params);
      tree parmtype = TREE_TYPE (parm);
      if (TREE_CODE (parmtype) != POINTER_TYPE)
	abort ();
      parmtype = TREE_TYPE (parmtype);
      if (TREE_NEEDS_WRAPPER (parmtype))
	{
	  if (fndecl == NULL_TREE || ! WRAPPER_NAME_P (DECL_NAME (fndecl)))
	    {
	      int bytecount = get_arglist_len_in_bytes (coerced_params);

	      params = tree_cons (NULL_TREE, build_int_2 (bytecount, 0),
				  tree_cons (NULL_TREE, function, TREE_CHAIN (coerced_params)));

	      return build_classfn_ref (TREE_VALUE (coerced_params),
					wrapper_name, params, 1, 1);
	    }
	}
    }
  {
    register tree result = 
      build (CALL_EXPR, value_type, function, coerced_params, NULL_TREE);

    TREE_VOLATILE (result) = 1;
    if (value_type == void_type_node)
      return result;
    return require_complete_type (result);
  }
}

/* Convert the actual parameter expressions in the list VALUES
   to the types in the list TYPELIST.
   If parmdecls is exhausted, or when an element has NULL as its type,
   perform the default conversions.

   NAME is an IDENTIFIER_NODE or 0.  It is used only for error messages.

   This is also where warnings about wrong number of args are generated.
   
   Return a list of expressions for the parameters as converted.

   Both VALUES and the returned value are chains of TREE_LIST nodes
   with the elements of the list in the TREE_VALUE slots of those nodes.

   In C++, unspecified trailing parameters can be filled in with their
   default arguments, if such were specified.  Do so here.  */

tree
actualparameterlist (typelist, values, fndecl)
     tree typelist, values, fndecl;
{
  register tree typetail, valtail;
  register tree result = NULL;
  char *called_thing;

  if (fndecl)
    if (TREE_METHOD (fndecl))
      if (TREE_TYPE (DECL_ORIGINAL_NAME (fndecl)))
	called_thing = "constructor";
      else
	called_thing = "method";
    else
      called_thing = "function";

  for (valtail = values, typetail = typelist;
       valtail;
       valtail = TREE_CHAIN (valtail))
    {
      register tree type = typetail ? TREE_VALUE (typetail) : 0;
      register tree val = TREE_VALUE (valtail);
      register tree parm;

      if (type == void_type_node)
	{
	  char buf[OVERLOAD_MAX_LEN];
	  if (fndecl)
	    {
	      hack_function_decl (buf, NULL_TREE, fndecl, 0);
	      error ("too many arguments to %s `%s'",
		     called_thing, buf);
	    }
	  else
	    error ("too many arguments to function");
	  /* In case anybody wants to know if this argument
	     list is valid.  */
	  if (result)
	    TREE_TYPE (result) = error_mark_node;
	  break;
	}

      /* The tree type of the parameter being passed may not yet be
	 known.  In this case, its type is TYPE_UNKNOWN, and will
	 be instantiated by the type given by TYPE.  If TYPE
	 is also NULL, the tree type of VAL is ERROR_MARK_NODE.  */
      if (TREE_TYPE (val) == unknown_type_node)
	if (TREE_CODE (val) == TREE_LIST
	    && TREE_CHAIN (val) == NULL_TREE)
	  {
	    /* Instantiates automatically.  */
	    val = TREE_VALUE (val);
	  }
	else if (type)
	  {
	    if (TREE_CODE (val) == OP_EXPR)
	      val = build_instantiated_decl (type, val);
	    else if (TREE_CODE (val) == TREE_LIST)
	      {
		tree new_val = build_tree_list (NULL_TREE, val);
		TREE_TYPE (new_val) = unknown_type_node;
		instantiate_type (type, new_val);
		if (TREE_TYPE (new_val) != error_mark_node)
		  val = TREE_VALUE (new_val);
	      }
	    else
	      instantiate_type (type, val);
	  }
	else
	  {
	    error ("insufficient type information in parameter list");
	    val = integer_zero_node;
	  }

      if (TREE_CODE (TREE_TYPE (val)) == ARRAY_TYPE
	  || TREE_CODE (TREE_TYPE (val)) == FUNCTION_TYPE)
	val = default_conversion (val);

      val = require_complete_type (val);

      if (type != 0)
	/* Formal parm type is specified by a function prototype.  */
	parm = build_tree_list (0, convert_for_initialization (0, type, val, "argument passing"));
      else if (TREE_CODE (TREE_TYPE (val)) == REAL_TYPE
               && (TYPE_PRECISION (TREE_TYPE (val))
	           < TYPE_PRECISION (double_type_node)))
	/* Convert `float' to `double'.  */
	parm = build_tree_list (NULL_TREE,
				convert (double_type_node, val));
      else
	/* Convert `short' and `char' to full-size `int'.  */
	parm = build_tree_list (NULL_TREE, default_conversion (val));

      result = chainon (result, parm);
      if (typetail)
	typetail = TREE_CHAIN (typetail);
    }

  if (typetail != 0 && TREE_VALUE (typetail) != void_type_node)
    {
      /* See if there are default arguments that can be used */
      if (TREE_PURPOSE (typetail))
	{
	  while (TREE_VALUE (typetail) != void_type_node)
	    {
	      tree type = TREE_VALUE (typetail);
	      tree val = TREE_PURPOSE (typetail);
	      tree parm;

	      parm =
		build_tree_list (0, convert_for_initialization (0, type, val, "default argument"));
	      result = chainon (result, parm);
	      typetail = TREE_CHAIN (typetail);
	    }
	}
      else
	{
	  char buf[OVERLOAD_MAX_LEN];
	  if (fndecl)
	    {
	      hack_function_decl (buf, NULL_TREE, fndecl, 0);
	      error ("too few arguments to %s `%s'",
		     called_thing, buf);
	    }
	  else
	    error ("too few arguments to function");
	  return error_mark_list;
	}
    }

  return result;
}

/* Build a binary-operation expression, after performing default
   conversions on the operands.  CODE is the kind of expression to build.  */

tree
build_x_binary_op (code, arg1, arg2)
     enum tree_code code;
     tree arg1, arg2;
{
  tree rval;

  if (rval = build_opfncall (code, arg1, arg2))
    return rval;
  return build_binary_op (code, arg1, arg2);
}

tree
build_binary_op (code, arg1, arg2)
     enum tree_code code;
     tree arg1, arg2;
{
  arg1 = default_conversion (arg1);
  arg2 = default_conversion (arg2);
  if (IS_AGGR_TYPE (TREE_TYPE (arg1)))
    {
      if (IS_AGGR_TYPE (TREE_TYPE (arg2)))
	{
	  /* Try to convert this to something reasonable.  */
	  if (! build_default_binary_type_conversion (&arg1, &arg2))
	    return error_mark_node;
	}
      else
	{
	  tree type1 = TREE_TYPE (arg1);
	  arg1 = build_type_conversion (TREE_TYPE (arg2), arg1, 1);
	  if (! arg1)
	    {
	      error ("type conversion required for type `%s'",
		     TYPE_NAME_STRING (type1));
	      return error_mark_node;
	    }
	}
    }
  else if (IS_AGGR_TYPE (TREE_TYPE (arg2)))
    {
      tree type2 = TREE_TYPE (arg2);
      arg2 = build_type_conversion (TREE_TYPE (arg1), arg2, 1);
      if (! arg2)
	{
	  error ("type conversion required for type `%s'",
		 TYPE_NAME_STRING (type2));
	  return error_mark_node;
	}
    }
  return build_binary_op_nodefault (code, arg1, arg2);
}

/* Build a binary-operation expression without default conversions.
   CODE is the kind of expression to build.
   This function differs from `build' in several ways:
   the data type of the result is computed and recorded in it,
   warnings are generated if arg data types are invalid,
   special handling for addition and subtraction of pointers is known,
   and some optimization is done (operations on narrow ints
   are done in the narrower type when that gives the same result).
   Constant folding is also done before the result is returned.

   Note that the operands will never have enumeral types
   because either they have just had the default conversions performed
   or they have both just been converted to some other type in which
   the arithmetic is to be done.  */

tree
build_binary_op_nodefault (code, op0, op1)
     enum tree_code code;
     tree op0, op1;
{
  tree dt0 = datatype (op0), dt1 = datatype (op1);

  /* The expression codes of the data types of the arguments tell us
     whether the arguments are integers, floating, pointers, etc.  */
  register enum tree_code code0 = TREE_CODE (dt0);
  register enum tree_code code1 = TREE_CODE (dt1);

  /* Expression code to give to the expression when it is built.
     Normally this is CODE, which is what the caller asked for,
     but in some special cases we change it.  */
  register enum tree_code resultcode = code;

  /* Data type in which the computation is to be performed.
     In the simplest cases this is the common type of the arguments.  */
  register tree result_type = NULL;

  /* Nonzero means operands have already been type-converted
     in whatever way is necessary.
     Zero means they need to be converted to RESULT_TYPE.  */
  int converted = 0;

  /* Nonzero means after finally constructing the expression
     give it this type.  Otherwise, give it type RESULT_TYPE.  */
  tree final_type = 0;

  /* Nonzero if this is an operation like MIN or MAX which can
     safely be computed in short if both args are promoted shorts.
     Also implies COMMON.
     -1 indicates a bitwise operation; this makes a difference
     in the exact conditions for when it is safe to do the operation
     in a narrower mode.  */
  int shorten = 0;

  /* Nonzero if this is a comparison operation;
     if both args are promoted shorts, compare the original shorts.
     Also implies COMMON.  */
  int short_compare = 0;

  /* Nonzero if this is a right-shift operation, which can be computed on the
     original short and then promoted if the operand is a promoted short.  */
  int short_shift = 0;

  /* Nonzero means set RESULT_TYPE to the common type of the args.  */
  int common = 0;

  /* If an error was already reported for one of the arguments,
     avoid reporting another error.  */

  if (code0 == ERROR_MARK || code1 == ERROR_MARK)
    return error_mark_node;

  switch (code)
    {
    case PLUS_EXPR:
      /* Handle the pointer + int case.  */
      if (code0 == POINTER_TYPE && code1 == INTEGER_TYPE)
	return pointer_int_sum (PLUS_EXPR, op0, op1);
      else if (code1 == POINTER_TYPE && code0 == INTEGER_TYPE)
	return pointer_int_sum (PLUS_EXPR, op1, op0);
      else
	common = 1;
      break;

    case MINUS_EXPR:
      /* Subtraction of two similar pointers.
	 We must subtract them as integers, then divide by object size.  */
      if (code0 == POINTER_TYPE && code1 == POINTER_TYPE
	  && comp_target_types (dt0, dt1))
	return pointer_diff (op0, op1);
      /* Handle pointer minus int.  Just like pointer plus int.  */
      else if (code0 == POINTER_TYPE && code1 == INTEGER_TYPE)
	return pointer_int_sum (MINUS_EXPR, op0, op1);
      else
	common = 1;
      break;

    case MULT_EXPR:
      common = 1;
      break;

    case TRUNC_DIV_EXPR:
      if ((code0 == INTEGER_TYPE || code0 == REAL_TYPE)
	  && (code1 == INTEGER_TYPE || code1 == REAL_TYPE))
	{
	  if (!(code0 == INTEGER_TYPE && code1 == INTEGER_TYPE))
	    resultcode = RDIV_EXPR;
	  else
	    shorten = 1;
	  common = 1;
	}
      break;

    case BIT_AND_EXPR:
    case BIT_ANDTC_EXPR:
    case BIT_IOR_EXPR:
    case BIT_XOR_EXPR:
      if (code0 == INTEGER_TYPE && code1 == INTEGER_TYPE)
	shorten = -1;
      /* If one operand is a constant, and the other is a short type
	 that has been converted to an int,
	 really do the work in the short type and then convert the
	 result to int.  If we are lucky, the constant will be 0 or 1
	 in the short type, making the entire operation go away.  */
      if (TREE_CODE (op0) == INTEGER_CST
	  && TREE_CODE (op1) == NOP_EXPR
	  && TYPE_PRECISION (dt1) > TYPE_PRECISION (TREE_TYPE (TREE_OPERAND (op1, 0)))
	  && TREE_UNSIGNED (TREE_TYPE (TREE_OPERAND (op1, 0))))
	{
	  final_type = result_type;
	  op1 = TREE_OPERAND (op1, 0);
	  result_type = TREE_TYPE (op1);
	}
      if (TREE_CODE (op1) == INTEGER_CST
	  && TREE_CODE (op0) == NOP_EXPR
	  && TYPE_PRECISION (dt0) > TYPE_PRECISION (TREE_TYPE (TREE_OPERAND (op0, 0)))
	  && TREE_UNSIGNED (TREE_TYPE (TREE_OPERAND (op0, 0))))
	{
	  final_type = result_type;
	  op0 = TREE_OPERAND (op0, 0);
	  result_type = TREE_TYPE (op0);
	}
      break;

    case TRUNC_MOD_EXPR:
      if (code0 == INTEGER_TYPE && code1 == INTEGER_TYPE)
	shorten = 1;
      break;

    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
    case TRUTH_AND_EXPR:
    case TRUTH_OR_EXPR:
      if ((code0 == INTEGER_TYPE || code0 == POINTER_TYPE || code0 == REAL_TYPE)
	  && (code1 == INTEGER_TYPE || code1 == POINTER_TYPE || code1 == REAL_TYPE))
	{
	  /* Result of these operations is always an int,
	     but that does not mean the operands should be
	     converted to ints!  */
	  result_type = integer_type_node;
	  converted = 1;
	}
      break;

      /* Shift operations: result has same type as first operand.
	 Also set SHORT_SHIFT if shifting rightward.  */

    case RSHIFT_EXPR:
      if (code0 == INTEGER_TYPE && code1 == INTEGER_TYPE)
	{
	  result_type = dt0;
	  if (TREE_CODE (op1) == INTEGER_CST
	      && TREE_INT_CST_LOW (op1) > 0)
	    short_shift = 1;
	}
      break;

    case LSHIFT_EXPR:
      if (code0 == INTEGER_TYPE && code1 == INTEGER_TYPE)
	{
	  result_type = dt0;
	  if (TREE_CODE (op1) == INTEGER_CST
	      && TREE_INT_CST_LOW (op1) < 0)
	    short_shift = 1;
	}
      break;

    case RROTATE_EXPR:
    case LROTATE_EXPR:
      if (code0 == INTEGER_TYPE && code1 == INTEGER_TYPE)
	result_type = dt0;
      break;

    case EQ_EXPR:
    case NE_EXPR:
      /* Result of comparison is always int,
	 but don't convert the args to int!  */
      result_type = integer_type_node;
      converted = 1;
      if ((code0 == INTEGER_TYPE || code0 == REAL_TYPE)
	  && (code1 == INTEGER_TYPE || code1 == REAL_TYPE))
	short_compare = 1;
      else if (code0 == POINTER_TYPE && code1 == POINTER_TYPE)
	{
	  register tree tt0 = TREE_TYPE (dt0);
	  register tree tt2 = TREE_TYPE (dt1);
	  /* Anything compares with void *.  void * compares with anything.
	     Otherwise, the targets must be the same.  */
	  if (comp_target_types (dt0, dt1))
	    ;
	  else if (tt0 == void_type_node)
	    {
	      if (pedantic && TREE_CODE (tt2) == FUNCTION_TYPE)
		warning ("ANSI C forbids comparison of `void *' with function pointer");
	    }
	  else if (tt2 == void_type_node)
	    {
	      if (pedantic && TREE_CODE (tt0) == FUNCTION_TYPE)
		warning ("ANSI C forbids comparison of `void *' with function pointer");
	    }
	  else
	    warning ("comparison of distinct pointer types lacks a cast");
	}
      else if (code0 == POINTER_TYPE && TREE_CODE (op1) == INTEGER_CST
	       && integer_zerop (op1))
	op1 = null_pointer_node;
      else if (code1 == POINTER_TYPE && TREE_CODE (op0) == INTEGER_CST
	       && integer_zerop (op0))
	op0 = null_pointer_node;
      else if (code0 == POINTER_TYPE && code1 == INTEGER_TYPE)
	{
	  if (! flag_traditional)
	    warning ("comparison between pointer and integer");
	  op1 = convert (TREE_TYPE (op0), op1);
	}
      else if (code0 == INTEGER_TYPE && code1 == POINTER_TYPE)
	{
	  if (! flag_traditional)
	    warning ("comparison between pointer and integer");
	  op0 = convert (TREE_TYPE (op1), op0);
	}
      else
	/* If args are not valid, clear out RESULT_TYPE
	   to cause an error message later.  */
	result_type = 0;
      break;

    case MAX_EXPR:
    case MIN_EXPR:
      if ((code0 == INTEGER_TYPE || code0 == REAL_TYPE)
	   && (code1 == INTEGER_TYPE || code1 == REAL_TYPE))
	shorten = 1;
      else if (code0 == POINTER_TYPE && code1 == POINTER_TYPE)
	{
	  if (! comp_target_types (dt0, dt1))
	    warning ("comparison of distinct pointer types lacks a cast");
	  else if (pedantic 
		   && TREE_CODE (TREE_TYPE (dt0)) == VOID_TYPE)
	    warning ("ANSI C forbids ordered comparisons of `void *'");
	  else if (pedantic 
		   && TREE_CODE (TREE_TYPE (dt0)) == FUNCTION_TYPE)
	    warning ("ANSI C forbids ordered comparisons of pointers to functions");
	  result_type = commontype (dt0, dt1);
	}
      else if (code0 == POINTER_TYPE && TREE_CODE (op1) == INTEGER_CST
	       && integer_zerop (op1))
	{
	  result_type = dt0;
	  op1 = null_pointer_node;
	  if (pedantic)
	    warning ("ordered comparison of pointer with integer zero");
	}
      else if (code1 == POINTER_TYPE && TREE_CODE (op0) == INTEGER_CST
	       && integer_zerop (op0))
	{
	  result_type = dt1;
	  op0 = null_pointer_node;
	  if (pedantic)
	    warning ("ordered comparison of pointer with integer zero");
	}
      break;

    case LE_EXPR:
    case GE_EXPR:
    case LT_EXPR:
    case GT_EXPR:
      if ((code0 == INTEGER_TYPE || code0 == REAL_TYPE)
	   && (code1 == INTEGER_TYPE || code1 == REAL_TYPE))
	short_compare = 1;
      else if (code0 == POINTER_TYPE && code1 == POINTER_TYPE)
	{
	  if (! comp_target_types (dt0, dt1, 0))
	    warning ("comparison of distinct pointer types lacks a cast");
	  else if (pedantic 
		   && TREE_CODE (TREE_TYPE (dt0)) == FUNCTION_TYPE)
	    warning ("ANSI C forbids ordered comparisons of pointers to functions");
	  result_type = integer_type_node;
	}
      else if (code0 == POINTER_TYPE && TREE_CODE (op1) == INTEGER_CST
	       && integer_zerop (op1))
	{
	  result_type = integer_type_node;
	  op1 = null_pointer_node;
	  if (! flag_traditional)
	    warning ("ordered comparison of pointer with integer zero");
	}
      else if (code1 == POINTER_TYPE && TREE_CODE (op0) == INTEGER_CST
	       && integer_zerop (op0))
	{
	  result_type = integer_type_node;
	  op0 = null_pointer_node;
	  if (pedantic)
	    warning ("ordered comparison of pointer with integer zero");
	}
      else if (code0 == POINTER_TYPE && code1 == INTEGER_TYPE)
	{
	  if (! flag_traditional)
	    warning ("comparison between pointer and integer");
	  op1 = convert (TREE_TYPE (op0), op1);
	}
      else if (code0 == INTEGER_TYPE && code1 == POINTER_TYPE)
	{
	  if (! flag_traditional)
	    warning ("comparison between pointer and integer");
	  op0 = convert (TREE_TYPE (op1), op0);
	}
      converted = 1;
      break;
    }

  if ((code0 == INTEGER_TYPE || code0 == REAL_TYPE)
      && (code1 == INTEGER_TYPE || code1 == REAL_TYPE))
    {
      if (shorten || common || short_compare)
	result_type = commontype (dt0, dt1);

      /* For certain operations (which identify themselves by shorten != 0)
	 if both args were extended from the same smaller type,
	 do the arithmetic in that type and then extend.

	 shorten !=0 and !=1 indicates a bitwise operation.
	 For them, this optimization is safe only if
	 both args are zero-extended or both are sign-extended.
	 Otherwise, we might change the result.
	 Eg, (short)-1 | (unsigned short)-1 is (int)-1
	 but calculated in (unsigned short) it would be (unsigned short)-1.  */

      if (shorten)
	{
	  int unsigned0, unsigned1;
	  tree arg0 = get_narrower (op0, &unsigned0);
	  tree arg1 = get_narrower (op1, &unsigned1);
	  /* UNS is 1 if the operation to be done is an unsigned one.  */
	  int uns = TREE_UNSIGNED (result_type);
	  tree type;

	  final_type = result_type;

	  /* Handle the case that OP0 does not *contain* a conversion
	     but it *requires* conversion to FINAL_TYPE.  */

	  if (op0 == arg0 && TREE_TYPE (op0) != final_type)
	    unsigned0 = TREE_UNSIGNED (TREE_TYPE (op0));
	  if (op1 == arg1 && TREE_TYPE (op1) != final_type)
	    unsigned1 = TREE_UNSIGNED (TREE_TYPE (op1));

	  /* Now UNSIGNED0 is 1 if ARG0 zero-extends to FINAL_TYPE.  */

	  /* For bitwise operations, signedness of nominal type
	     does not matter.  Consider only how operands were extended.  */
	  if (shorten == -1)
	    uns = unsigned0;

	  /* Note that in all three cases below we refrain from optimizing
	     an unsigned operation on sign-extended args.
	     That would not be valid.  */

	  /* Both args variable: if both extended in same way
	     from same width, do it in that width.
	     Do it unsigned if args were zero-extended.  */
	  if ((TYPE_PRECISION (TREE_TYPE (arg0))
	       < TYPE_PRECISION (result_type))
	      && (TYPE_PRECISION (TREE_TYPE (arg1))
		  == TYPE_PRECISION (TREE_TYPE (arg0)))
	      && unsigned0 == unsigned1
	      && (unsigned0 || !uns))
	    result_type
	      = signed_or_unsigned_type (unsigned0,
					 commontype (TREE_TYPE (arg0), TREE_TYPE (arg1)));
	  else if (TREE_CODE (arg0) == INTEGER_CST
		   && (unsigned1 || !uns)
		   && (TYPE_PRECISION (TREE_TYPE (arg1))
		       < TYPE_PRECISION (result_type))
		   && (type = signed_or_unsigned_type (unsigned1,
						       TREE_TYPE (arg1)),
		       int_fits_type_p (arg0, type)))
	    result_type = type;
	  else if (TREE_CODE (arg1) == INTEGER_CST
		   && (unsigned0 || !uns)
		   && (TYPE_PRECISION (TREE_TYPE (arg0))
		       < TYPE_PRECISION (result_type))
		   && (type = signed_or_unsigned_type (unsigned0,
						       TREE_TYPE (arg0)),
		       int_fits_type_p (arg1, type)))
	    result_type = type;
	}

      /* Shifts can be shortened if shifting right.  */

      if (short_shift)
	{
	  int unsigned_arg;
	  tree arg0 = get_narrower (op0, &unsigned_arg);

	  final_type = result_type;

	  if (TYPE_PRECISION (TREE_TYPE (arg0)) < TYPE_PRECISION (result_type)
	      /* If arg is sign-extended and then unsigned-shifted,
		 we can simulate this with a signed shift in arg's type
		 only if the extended result is at least twice as wide
		 as the arg.  Otherwise, the shift could use up all the
		 ones made by sign-extension and bring in zeros.
		 We can't optimize that case at all, but in most machines
		 it never happens because available widths are 2**N.  */
	      && (!TREE_UNSIGNED (final_type)
		  || unsigned_arg
		  || 2 * TYPE_PRECISION (TREE_TYPE (arg0)) <= TYPE_PRECISION (result_type)))
	    {
	      /* Convert the shift-count to its nominal type.  */
	      if (TREE_TYPE (op1) != result_type)
		op1 = convert (result_type, op1);
	      /* Do an unsigned shift if the operand was zero-extended.  */
	      result_type
		= signed_or_unsigned_type (unsigned_arg,
					   TREE_TYPE (arg0));
	      /* Convert value-to-be-shifted to that type.  */
	      if (TREE_TYPE (op0) != result_type)
		op0 = convert (result_type, op0);
	      converted = 1;
	    }
	}

      /* Comparison operations are shortened too but differently.
	 They identify themselves by setting short_compare = 1.  */

      if (short_compare)
	{
	  /* Don't write &op0, etc., because that would prevent op0
	     from being kept in a register.
	     Instead, make copies of the our local variables and
	     pass the copies by reference, then copy them back afterward.  */
	  tree xop0 = op0, xop1 = op1, xresult_type = result_type;
	  enum tree_code xresultcode = resultcode;
	  tree val 
	    = shorten_compare (&xop0, &xop1, &xresult_type, &xresultcode);
	  if (val != 0)
	    return val;
	  op0 = xop0, op1 = xop1, result_type = xresult_type;
	  resultcode = xresultcode;
	}
    }

  /* At this point, RESULT_TYPE must be nonzero to avoid an error message.
     If CONVERTED is zero, both args will be converted to type RESULT_TYPE.
     Then the expression will be built.
     It will be given type FINAL_TYPE if that is nonzero;
     otherwise, it will be given type RESULT_TYPE.  */

  if (!result_type)
    {
      binary_op_error (code);
      return error_mark_node;
    }

  if (! converted)
    {
      if (TREE_TYPE (op0) != result_type)
	op0 = convert (result_type, op0); 
      if (TREE_TYPE (op1) != result_type)
	op1 = convert (result_type, op1); 
    }

  {
    register tree result = build (resultcode, result_type, op0, op1);
    register tree folded;

    folded = fold (result);
    if (folded == result)
      TREE_LITERAL (folded) = TREE_LITERAL (op0) & TREE_LITERAL (op1);
    if (final_type != 0)
      return convert (final_type, folded);
    return folded;
  }
}

/* Return a tree for the sum or difference (RESULTCODE says which)
   of pointer PTROP and integer INTOP.  */

static tree
pointer_int_sum (resultcode, ptrop, intop)
     enum tree_code resultcode;
     register tree ptrop, intop;
{
  tree size_exp;

  register tree result;
  register tree folded;

  /* The result is a pointer of the same type that is being added.  */

  register tree result_type = datatype (ptrop);

  if (TREE_TYPE (result_type) == void_type_node)
    {
      if (pedantic)
	warning ("pointer of type `void *' used in arithmetic");
      size_exp = integer_one_node;
    }
  else if (TREE_CODE (TREE_TYPE (result_type)) == FUNCTION_TYPE)
    {
      if (pedantic)
	warning ("pointer to a function used in arithmetic");
      size_exp = integer_one_node;
    }
  else
    size_exp = size_in_bytes (TREE_TYPE (result_type));

  /* If what we are about to multiply by the size of the elements
     contains a constant term, apply distributive law
     and multiply that constant term separately.
     This helps produce common subexpressions.  */

  if ((TREE_CODE (intop) == PLUS_EXPR || TREE_CODE (intop) == MINUS_EXPR)
      && ! TREE_LITERAL (intop)
      && TREE_LITERAL (TREE_OPERAND (intop, 1))
      && TREE_LITERAL (size_exp))
    {
      enum tree_code subcode = resultcode;
      if (TREE_CODE (intop) == MINUS_EXPR)
	subcode = (subcode == PLUS_EXPR ? MINUS_EXPR : PLUS_EXPR);
      ptrop = build_binary_op (subcode, ptrop, TREE_OPERAND (intop, 1));
      intop = TREE_OPERAND (intop, 0);
    }

  /* Convert the integer argument to a type the same size as a pointer
     so the multiply won't overflow spuriously.  */

  if (TYPE_PRECISION (TREE_TYPE (intop)) != POINTER_SIZE)
    intop = convert (type_for_size (POINTER_SIZE, 0), intop);

  /* Replace the integer argument
     with a suitable product by the object size.  */

  intop = build_binary_op (MULT_EXPR, intop, size_exp);

  /* Create the sum or difference.  */

  result = build (resultcode, result_type, ptrop, intop);

  folded = fold (result);
  if (folded == result)
    TREE_LITERAL (folded) = TREE_LITERAL (ptrop) & TREE_LITERAL (intop);
  return folded;
}

/* Return a tree for the difference of pointers OP0 and OP1.
   The resulting tree has type int.  */

static tree
pointer_diff (op0, op1)
     register tree op0, op1;
{
  tree dt0 = datatype (op0);
  enum tree_code resultcode;
  register tree result, folded;
  tree restype = type_for_size (POINTER_SIZE, 0);

  if (pedantic)
    {
      if (TREE_CODE (TREE_TYPE (dt0)) == VOID_TYPE)
	warning ("pointer of type `void *' used in subtraction");
      if (TREE_CODE (TREE_TYPE (dt0)) == FUNCTION_TYPE)
	warning ("pointer to a function used in subtraction");
    }

  /* First do the subtraction as integers;
     then drop through to build the divide operator.  */

  op0 = build_binary_op (MINUS_EXPR,
			 convert (restype, op0), convert (restype, op1));
  op1 = ((TREE_TYPE (dt0) == void_type_node
	  || TREE_CODE (TREE_TYPE (dt0)) == FUNCTION_TYPE)
	 ? integer_one_node
	 : size_in_bytes (TREE_TYPE (dt0)));

  /* By altering RESULTCODE, we direct this function to build
     the division operation.  If dividing by a power of 2,
     use floor-division (rounding down) since that is what
     a shift insn does.  Otherwise, since we can't use a shift anyway,
     use whichever kind of rounding this machine does most easily.  */

  if (TREE_CODE (op1) == INTEGER_CST
      && -1 == exact_log2 (TREE_INT_CST_LOW (op1)))
    resultcode = FLOOR_DIV_EXPR;
  else
    resultcode = EASY_DIV_EXPR;

  /* Create the sum or difference.  */

  result = build (resultcode, restype, op0, op1);

  folded = fold (result);
  if (folded == result)
    TREE_LITERAL (folded) = TREE_LITERAL (op0) & TREE_LITERAL (op1);
  return folded;
}

/* Print an error message for invalid operands to arith operation CODE.  */

static void
binary_op_error (code)
     enum tree_code code;
{
  register char *opname;
  switch (code)
    {
    case PLUS_EXPR:
      opname = "+"; break;
    case MINUS_EXPR:
      opname = "-"; break;
    case MULT_EXPR:
      opname = "*"; break;
    case MAX_EXPR:
      opname = "max"; break;
    case MIN_EXPR:
      opname = "min"; break;
    case EQ_EXPR:
      opname = "=="; break;
    case NE_EXPR:
      opname = "!="; break;
    case LE_EXPR:
      opname = "<="; break;
    case GE_EXPR:
      opname = ">="; break;
    case LT_EXPR:
      opname = "<"; break;
    case GT_EXPR:
      opname = ">"; break;
    case LSHIFT_EXPR:
      opname = "<<"; break;
    case RSHIFT_EXPR:
      opname = ">>"; break;
    case TRUNC_MOD_EXPR:
      opname = "%"; break;
    case TRUNC_DIV_EXPR:
      opname = "/"; break;
    case BIT_AND_EXPR:
      opname = "&"; break;
    case BIT_IOR_EXPR:
      opname = "|"; break;
    case TRUTH_ANDIF_EXPR:
      opname = "&&"; break;
    case TRUTH_ORIF_EXPR:
      opname = "||"; break;
    case BIT_XOR_EXPR:
      opname = "^"; break;
    }
  error ("invalid operands to binary %s", opname);
}

/* Subroutine of build_binary_op_nodefault, used for comparison operations.
   See if the operands have both been converted from subword integer types
   and, if so, perhaps change them both back to their original type.

   The arguments of this function are all pointers to local variables
   of build_binary_op_nodefault: OP0_PTR is &OP0, OP1_PTR is &OP1,
   RESTYPE_PTR is &RESULT_TYPE and RESCODE_PTR is &RESULTCODE.

   If this function returns nonzero, it means that the comparison has
   a constant value.  What this function returns is an expression for
   that value.  */

static tree
shorten_compare (op0_ptr, op1_ptr, restype_ptr, rescode_ptr)
     tree *op0_ptr, *op1_ptr;
     tree *restype_ptr;
     enum tree_code *rescode_ptr;
{
  register tree type;
  tree op0 = *op0_ptr;
  tree op1 = *op1_ptr;
  int unsignedp0, unsignedp1;
  int real1, real2;
  tree primop0, primop1;
  enum tree_code code = *rescode_ptr;

  /* Throw away any conversions to wider types
     already present in the operands.  */

  primop0 = get_narrower (op0, &unsignedp0);
  primop1 = get_narrower (op1, &unsignedp1);

  /* If one of the operands must be floated, we cannot optimize.  */
  real1 = TREE_CODE (TREE_TYPE (primop0)) == REAL_TYPE;
  real2 = TREE_CODE (TREE_TYPE (primop1)) == REAL_TYPE;

  /* If first arg is constant, swap the args (changing operation
     so value is preserved), for canonicalization.  */

  if (TREE_LITERAL (primop0))
    {
      register tree tem = primop0;
      register int temi = unsignedp0;
      primop0 = primop1;
      primop1 = tem;
      tem = op0;
      op0 = op1;
      op1 = tem;
      *op0_ptr = op0;
      *op1_ptr = op1;
      unsignedp0 = unsignedp1;
      unsignedp1 = temi;
      temi = real1;
      real1 = real2;
      real2 = temi;

      switch (code)
	{
	case LT_EXPR:
	  code = GT_EXPR;
	  break;
	case GT_EXPR:
	  code = LT_EXPR;
	  break;
	case LE_EXPR:
	  code = GE_EXPR;
	  break;
	case GE_EXPR:
	  code = LE_EXPR;
	  break;
	}
      *rescode_ptr = code;
    }

  /* If comparing an integer against a constant more bits wide,
     maybe we can deduce a value of 1 or 0 independent of the data.
     Or else truncate the constant now
     rather than extend the variable at run time.

     This is only interesting if the constant is the wider arg.
     Also, it is not safe if the constant is unsigned and the
     variable arg is signed, since in this case the variable
     would be sign-extended and then regarded as unsigned.
     Our technique fails in this case because the lowest/highest
     possible unsigned results don't follow naturally from the
     lowest/highest possible values of the variable operand.
     For just EQ_EXPR and NE_EXPR there is another technique that
     could be used: see if the constant can be faithfully represented
     in the other operand's type, by truncating it and reextending it
     and see if that preserves the constant's value.  */

  if (!real1 && !real2
      && TREE_CODE (primop1) == INTEGER_CST
      && TYPE_PRECISION (TREE_TYPE (primop0)) < TYPE_PRECISION (*restype_ptr))
    {
      int min_gt, max_gt, min_lt, max_lt;
      tree maxval, minval;
      /* 1 if comparison is nominally unsigned.  */
      int unsignedp = TREE_UNSIGNED (*restype_ptr);
      tree val;

      type = signed_or_unsigned_type (unsignedp0, TREE_TYPE (primop0));

      maxval = TYPE_MAX_VALUE (type);
      minval = TYPE_MIN_VALUE (type);

      if (unsignedp && !unsignedp0)
	*restype_ptr = signed_type (*restype_ptr);

      if (TREE_TYPE (primop1) != *restype_ptr)
	primop1 = convert (*restype_ptr, primop1);
      if (type != *restype_ptr)
	{
	  minval = convert (*restype_ptr, minval);
	  maxval = convert (*restype_ptr, maxval);
	}

      if (unsignedp && unsignedp0)
	{
	  min_gt = INT_CST_LT_UNSIGNED (primop1, minval);
	  max_gt = INT_CST_LT_UNSIGNED (primop1, maxval);
	  min_lt = INT_CST_LT_UNSIGNED (minval, primop1);
	  max_lt = INT_CST_LT_UNSIGNED (maxval, primop1);
	}
      else
	{
	  min_gt = INT_CST_LT (primop1, minval);
	  max_gt = INT_CST_LT (primop1, maxval);
	  min_lt = INT_CST_LT (minval, primop1);
	  max_lt = INT_CST_LT (maxval, primop1);
	}

      val = 0;
      switch (code)
	{
	case NE_EXPR:
	  if (max_lt || min_gt)
	    val = integer_one_node;
	  break;

	case EQ_EXPR:
	  if (max_lt || min_gt)
	    val = integer_zero_node;
	  break;

	case LT_EXPR:
	  if (max_lt)
	    val = integer_one_node;
	  if (!min_lt)
	    val = integer_zero_node;
	  break;

	case GT_EXPR:
	  if (min_gt)
	    val = integer_one_node;
	  if (!max_gt)
	    val = integer_zero_node;
	  break;

	case LE_EXPR:
	  if (!max_gt)
	    val = integer_one_node;
	  if (min_gt)
	    val = integer_zero_node;
	  break;

	case GE_EXPR:
	  if (!min_lt)
	    val = integer_one_node;
	  if (max_lt)
	    val = integer_zero_node;
	  break;
	}

      /* If primop0 was sign-extended and unsigned comparison specd,
	 we did a signed comparison above using the signed type bounds.
	 But the comparison we output must be unsigned.

	 Also, for inequalities, VAL is no good; but if the signed
	 comparison had *any* fixed result, it follows that the
	 unsigned comparison just tests the sign in reverse
	 (positive values are LE, negative ones GE).
	 So we can generate an unsigned comparison
	 against an extreme value of the signed type.  */

      if (unsignedp && !unsignedp0)
	{
	  if (val != 0)
	    switch (code)
	      {
	      case LT_EXPR:
	      case GE_EXPR:
		primop1 = TYPE_MIN_VALUE (type);
		val = 0;
		break;

	      case LE_EXPR:
	      case GT_EXPR:
		primop1 = TYPE_MAX_VALUE (type);
		val = 0;
		break;
	      }
	  type = unsigned_type (type);
	}

      if (max_lt && !unsignedp0)
	{
	  /* This is the case of (char)x >?< 0x80, which people used to use
	     expecting old C compilers to change the 0x80 into -0x80.  */
	  if (val == integer_zero_node)
	    warning ("comparison is always 0 due to limited range of data type");
	  if (val == integer_one_node)
	    warning ("comparison is always 1 due to limited range of data type");
	}

      if (val != 0)
	{
	  /* Don't forget to evaluate PRIMOP0 if it has side effects.  */
	  if (TREE_VOLATILE (primop0))
	    return build (COMPOUND_EXPR, TREE_TYPE (val), primop0, val);
	  return val;
	}

      /* Value is not predetermined, but do the comparison
	 in the type of the operand that is not constant.
	 TYPE is already properly set.  */
    }
  else if (real1 && real2
	   && TYPE_PRECISION (TREE_TYPE (primop0)) == TYPE_PRECISION (TREE_TYPE (primop1)))
    type = TREE_TYPE (primop0);

  /* If args' natural types are both narrower than nominal type
     and both extend in the same manner, compare them
     in the type of the wider arg.
     Otherwise must actually extend both to the nominal
     common type lest different ways of extending
     alter the result.
     (eg, (short)-1 == (unsigned short)-1  should be 0.)  */

  else if (unsignedp0 == unsignedp1 && real1 == real2
	   && TYPE_PRECISION (TREE_TYPE (primop0)) < TYPE_PRECISION (*restype_ptr)
	   && TYPE_PRECISION (TREE_TYPE (primop1)) < TYPE_PRECISION (*restype_ptr))
    {
      type = commontype (TREE_TYPE (primop0), TREE_TYPE (primop1));
      type = signed_or_unsigned_type (unsignedp0
				      || TREE_UNSIGNED (*restype_ptr),
				      type);
      /* Make sure shorter operand is extended the right way
	 to match the longer operand.  */
      primop0 = convert (signed_or_unsigned_type (unsignedp0, TREE_TYPE (primop0)),
			 primop0);
      primop1 = convert (signed_or_unsigned_type (unsignedp1, TREE_TYPE (primop1)),
			 primop1);
    }
  else
    {
      /* Here we must do the comparison on the nominal type
	 using the args exactly as we received them.  */
      type = *restype_ptr;
      primop0 = op0;
      primop1 = op1;
    }

  *op0_ptr = convert (type, primop0);
  *op1_ptr = convert (type, primop1);

  *restype_ptr = integer_type_node;

  return 0;
}

/* Construct and perhaps optimize a tree representation
   for a unary operation.  CODE, a tree_code, specifies the operation
   and XARG is the operand.  NOCONVERT nonzero suppresses
   the default promotions (such as from short to int).  */

tree
build_x_unary_op (code, xarg, noconvert)
     enum tree_code code;
     tree xarg;
     int noconvert;
{
  tree rval;

  if (rval = build_opfncall (code, xarg))
    return rval;
  return build_unary_op (code, xarg, noconvert);
}

/* C++: Must handle pointers to members.  */
tree
build_unary_op (code, xarg, noconvert)
     enum tree_code code;
     tree xarg;
     int noconvert;
{
  /* No default_conversion here.  It causes trouble for ADDR_EXPR.  */
  register tree arg = xarg;
  register tree argtype = 0;
  register enum tree_code typecode = TREE_CODE (TREE_TYPE (arg));
  char *errstring = NULL;
  tree val;

  if (typecode == ERROR_MARK)
    return error_mark_node;

  if (typecode == REFERENCE_TYPE && code != ADDR_EXPR && ! noconvert)
    {
      arg = bash_reference_type (arg);
      typecode = TREE_CODE (TREE_TYPE (arg));
    }

  if (typecode == ENUMERAL_TYPE)
    typecode = INTEGER_TYPE;

  switch (code)
    {
    case CONVERT_EXPR:
      if (!(typecode == INTEGER_TYPE || typecode == REAL_TYPE
	    || typecode == POINTER_TYPE))
        errstring = "wrong type argument to unary plus";
      /* This is used for unary plus, because a CONVERT_EXPR
	 is enough to prevent anybody from looking inside for
	 associativity, but won't generate any code.
	 Any argument is ok.  */
      break;

    case NEGATE_EXPR:
      if (!(typecode == INTEGER_TYPE || typecode == REAL_TYPE))
        errstring = "wrong type argument to unary minus";
      else if (!noconvert)
	arg = default_conversion (arg);
      break;

    case BIT_NOT_EXPR:
      if (typecode != INTEGER_TYPE)
        errstring = "wrong type argument to bit-complement";
      else if (!noconvert)
	arg = default_conversion (arg);
      break;

    case ABS_EXPR:
      if (!(typecode == INTEGER_TYPE || typecode == REAL_TYPE))
        errstring = "wrong type argument to abs";
      else if (!noconvert)
	arg = default_conversion (arg);
      break;

    case TRUTH_NOT_EXPR:
      if (typecode != INTEGER_TYPE
	  && typecode != REAL_TYPE && typecode != POINTER_TYPE)
	{
	  errstring = "wrong type argument to unary exclamation mark";
	  break;
	}
      arg = truthvalue_conversion (arg);
      if (TREE_CODE (arg) == NE_EXPR)
	{
	  TREE_SET_CODE (arg, EQ_EXPR);
	  return arg;
	}
      if (TREE_CODE (arg) == EQ_EXPR)
	{
	  TREE_SET_CODE (arg, NE_EXPR);
	  return arg;
	}
      if (TREE_CODE (arg) == TRUTH_NOT_EXPR)
	{
	  return TREE_OPERAND (arg, 0);
	}
      break;

    case NOP_EXPR:
      break;
      
    case PREINCREMENT_EXPR:
    case POSTINCREMENT_EXPR:
    case PREDECREMENT_EXPR:
    case POSTDECREMENT_EXPR:
      /* Handle complex lvalues (when permitted)
	 by reduction to simpler cases.  */

      val = unary_complex_lvalue (code, arg);
      if (val != 0)
	return val;

      /* Report invalid types.  */

      if (typecode != POINTER_TYPE
	  && typecode != INTEGER_TYPE && typecode != REAL_TYPE)
	{
	  if (code == PREINCREMENT_EXPR || code == POSTINCREMENT_EXPR)
	    errstring ="wrong type argument to increment";
	  else
	    errstring ="wrong type argument to decrement";
	  break;
	}

      /* Report something read-only.  */

      if (TREE_READONLY (arg))
	readonly_warning (arg, 
			  ((code == PREINCREMENT_EXPR
			    || code == POSTINCREMENT_EXPR)
			   ? "increment" : "decrement"));

      {
	register tree inc;
	tree result_type = TREE_TYPE (arg);

	arg = get_unwidened (arg, 0);
	argtype = TREE_TYPE (arg);

	/* Compute the increment.  */

	if (typecode == POINTER_TYPE)
	  {
	    if (pedantic && (TREE_CODE (argtype) == FUNCTION_TYPE
			     || TREE_CODE (argtype) == VOID_TYPE))
	      warning ("wrong type argument to %s",
		       ((code == PREINCREMENT_EXPR
			 || code == POSTINCREMENT_EXPR)
			? "increment" : "decrement"));
	    inc = c_sizeof_nowarn (TREE_TYPE (argtype));
	  }
	else
	  inc = integer_one_node;

	inc = convert (argtype, inc);

	/* Handle incrementing a cast-expression.  */

	if (!pedantic)
	  switch (TREE_CODE (arg))
	    {
	    case NOP_EXPR:
	    case CONVERT_EXPR:
	    case FLOAT_EXPR:
	    case FIX_TRUNC_EXPR:
	    case FIX_FLOOR_EXPR:
	    case FIX_ROUND_EXPR:
	    case FIX_CEIL_EXPR:
	      {
		tree incremented, modify, value;
		arg = stabilize_reference (arg);
		if (code == PREINCREMENT_EXPR || code == PREDECREMENT_EXPR)
		  value = arg;
		else
		  value = save_expr (arg);
		incremented = build (((code == PREINCREMENT_EXPR
				       || code == POSTINCREMENT_EXPR)
				      ? PLUS_EXPR : MINUS_EXPR),
				     argtype, value, inc);
		modify = build_modify_expr (arg, NOP_EXPR, incremented);
		return build (COMPOUND_EXPR, TREE_TYPE (arg), modify, value);
	      }
	    }

	/* Complain about anything else that is not a true lvalue.  */
	if (!lvalue_or_else (arg, ((code == PREINCREMENT_EXPR
				    || code == POSTINCREMENT_EXPR)
				   ? "increment" : "decrement")))
	  return error_mark_node;

	return convert (result_type, build (code, TREE_TYPE (arg),
					    arg, inc));
      }

    case ADDR_EXPR:
      /* Note that this operation never does default_conversion
	 regardless of NOCONVERT.  */

      if (TREE_CODE (arg) == REFERENCE_EXPR)
	{
	  error ("references are not lvalues");
	  return error_mark_node;
	}
      else if (typecode == REFERENCE_TYPE)
	return build (REFERENCE_EXPR, build_pointer_type (TREE_TYPE (TREE_TYPE (arg))), arg);

      /* Let &* cancel out to simplify resulting code.  */
      if (TREE_CODE (arg) == INDIRECT_REF)
	{
	  /* Keep `default_conversion' from converting if
	     ARG is of REFERENCE_TYPE.  */
	  arg = TREE_OPERAND (arg, 0);
	  if (TREE_CODE (TREE_TYPE (arg)) == REFERENCE_TYPE)
	    arg = build (REFERENCE_EXPR, build_pointer_type (TREE_TYPE (TREE_TYPE (arg))), arg);
	  return arg;
	}

      /* For &x[y], return x+y */
      if (TREE_CODE (arg) == ARRAY_REF)
	{
	  tree rval;

	  mark_addressable (TREE_OPERAND (arg, 0));
	  rval = build_binary_op (PLUS_EXPR, TREE_OPERAND (arg, 0),
				  TREE_OPERAND (arg, 1));
	  if (TREE_MEMBER_POINTER (TREE_TYPE (arg)))
	    {
	      argtype = TREE_TYPE (rval);
	      TREE_TYPE (rval) = build_type_variant (argtype,
						     TREE_READONLY (argtype),
						     TREE_THIS_VOLATILE (argtype),
						     TREE_TYPE (argtype));
	    }
	  return rval;
	}

      /* Uninstantiated types are all functions.  Taking the
	 address of a function is a no-op, so just return the
	 arguemnt.  */
      if (TREE_CODE (arg) == TREE_LIST)
	{
	  warning ("address of (uninstantiated) function ignored");
	  return arg;
	}

      /* For & S::mf, or & S::m, return an expression such that
	 when it is expanded, we will get the desired effect.  */
      if (member_pointer_p (arg))
	{
	  if (TREE_CODE (arg) == COMPONENT_REF)
	    {
	      /* unresolved member reference.  */
	      tree member = TREE_OPERAND (arg, 1);

	      if (TREE_CODE (member) == TREE_LIST)
		{
		  /* overloaded method pointer  */
		  sorry ("overloaded methods");
		  return error_mark_node;
		}
	      else
		{
		  /* member pointer  */
		  sorry (" & class :: member");
		  return error_mark_node;
		}
	    }
	  else if (TREE_CODE (arg) == SCOPE_REF)
	    {
	      /* This will be in the form
		 MEMBER_TYPE (MEMBER_POINTER_TYPE :: MEMBER) */
	      tree member = TREE_OPERAND (arg, 1);
	      arg = TREE_OPERAND (arg, 0);

	      /* If ARG is a POINTER_TYPE, then we have a simple
		 pointer-to-member construct.

		 Otherwise, ARG is a valid expr (we hope),
		 and the value of this expression
		 will compute the actual address of
		 the specific member being referenced.  */
	      if (TREE_CODE (arg) != POINTER_TYPE)
		{
		  if (TREE_CODE (member) == FUNCTION_DECL)
		    return default_conversion (member);
		  if (TREE_CODE (member) == FIELD_DECL)
		    return build_unary_op (ADDR_EXPR, IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (member)), 0);
		  if (TREE_CODE (member) == INTEGER_CST)
		    {
		      abort ();
		    }
		  argtype = TREE_TYPE (member);
		  if (TREE_CODE (argtype) != POINTER_TYPE)
		    abort ();
		  argtype = TREE_TYPE (argtype);
		  return build (NOP_EXPR, build_pointer_type (argtype), member);
		}

	      argtype = arg;
	      if (TREE_CODE (member) == FUNCTION_DECL)
		{
		  arg = build (ADDR_EXPR, argtype, member);
		  TREE_LITERAL (arg) = 1;
		  return arg;
		}
	      else if (TREE_CODE (member) == INTEGER_CST)
		{
		  arg = build (NOP_EXPR, argtype, member);
		  TREE_LITERAL (arg) = 1;
		  return arg;
		}
	      else if (TREE_CODE (member) == FIELD_DECL)
		{
		  /* This is a static class member.  */
		  warning ("not really implemented: pointers to static class members");
		  return build (ADDR_EXPR, argtype,
				IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (member)));
		}
	      else
		{
		  /* what can this be?  */
		  abort ();
		  return build (NOP_EXPR, argtype, member);
		}
	    }
	}

      /* Handle complex lvalues (when permitted)
	 by reduction to simpler cases.  */
      val = unary_complex_lvalue (code, arg);
      if (val != 0)
	return val;

      /* Address of a cast is just a cast of the address
	 of the operand of the cast.  */
      switch (TREE_CODE (arg))
	{
	case NOP_EXPR:
	case CONVERT_EXPR:
	case FLOAT_EXPR:
	case FIX_TRUNC_EXPR:
	case FIX_FLOOR_EXPR:
	case FIX_ROUND_EXPR:
	case FIX_CEIL_EXPR:
	  if (pedantic)
	    warning ("ANSI C forbids the address of a cast expression");
	  return convert (build_pointer_type (TREE_TYPE (arg)),
			  build_unary_op (ADDR_EXPR, TREE_OPERAND (arg, 0), 0));
	}

      /* Allow the address of a constructor if all the elements
	 are constant.  */
      if (TREE_CODE (arg) == CONSTRUCTOR && TREE_LITERAL (arg))
	;
      /* Anything not already handled and not a true memory reference
	 is an error.  */
      else if (typecode != FUNCTION_TYPE && !lvalue_or_else (arg, "unary `&'"))
	return error_mark_node;

      /* Ordinary case; arg is a COMPONENT_REF or a decl.  */
      argtype = TREE_TYPE (arg);
      if (TREE_READONLY (arg) || TREE_THIS_VOLATILE (arg))
	argtype = build_type_variant (argtype,
				      TREE_READONLY (arg),
				      TREE_THIS_VOLATILE (arg), 0);

      argtype = build_pointer_type (argtype);

      mark_addressable (arg);

      {
	tree addr;

	if (TREE_CODE (arg) == COMPONENT_REF)
	  {
	    tree field = TREE_OPERAND (arg, 1);
	    addr = build_unary_op (ADDR_EXPR, TREE_OPERAND (arg, 0), 0);

	    if (TREE_PACKED (field))
	      {
		error ("attempt to take address of bit-field structure member `%s'",
		       IDENTIFIER_POINTER (DECL_NAME (field)));
		return error_mark_node;
	      }

	    if (DECL_OFFSET (field) != 0)
	      {
		tree offset = build_int_2 ((DECL_OFFSET (field)
					    / BITS_PER_UNIT),
					   0);
		TREE_TYPE (offset) = argtype;
		addr = fold (build (PLUS_EXPR, argtype, addr, offset));
	      }
	    else
	      addr = convert (argtype, addr);
	  }
	else
	  addr = build (code, argtype, arg);

	/* Address of a static or external variable or
	   function counts as a constant */
	TREE_LITERAL (addr) = staticp (arg);
	return addr;
      }
    }

  if (!errstring)
    {
      if (argtype == 0)
	argtype = TREE_TYPE (arg);
      return fold (build (code, argtype, arg));
    }

  error (errstring);
  return error_mark_node;
}

/* If CONVERSIONS is a conversion expression or a nested sequence of such,
   convert ARG with the same conversions in the same order
   and return the result.  */

static tree
convert_sequence (conversions, arg)
     tree conversions;
     tree arg;
{
  switch (TREE_CODE (conversions))
    {
    case NOP_EXPR:
    case CONVERT_EXPR:
    case FLOAT_EXPR:
    case FIX_TRUNC_EXPR:
    case FIX_FLOOR_EXPR:
    case FIX_ROUND_EXPR:
    case FIX_CEIL_EXPR:
      return convert (TREE_TYPE (conversions),
		      convert_sequence (TREE_OPERAND (conversions, 0),
					arg));

    default:
      return arg;
    }
}

/* Apply unary lvalue-demanding operator CODE to the expression ARG
   for certain kinds of expressions which are not really lvalues
   but which we can accept as lvalues.

   If ARG is not a kind of expression we can handle, return zero.  */
   
tree
unary_complex_lvalue (code, arg)
     enum tree_code code;
     tree arg;
{
  if (pedantic)
    return 0;

  /* Handle (a, b) used as an "lvalue".  */
  if (TREE_CODE (arg) == COMPOUND_EXPR)
    {
      tree real_result = build_unary_op (code, TREE_OPERAND (arg, 1), 0);
      return build (COMPOUND_EXPR, TREE_TYPE (real_result),
		    TREE_OPERAND (arg, 0), real_result);
    }

  /* Handle (a ? b : c) used as an "lvalue".  */
  if (TREE_CODE (arg) == COND_EXPR)
    return (build_conditional_expr
	    (TREE_OPERAND (arg, 0),
	     build_unary_op (code, TREE_OPERAND (arg, 1), 0),
	     build_unary_op (code, TREE_OPERAND (arg, 2), 0)));

  return 0;
}

/* Warn about storing in something that is `const'.  */

void
readonly_warning (arg, string)
     tree arg;
     char *string;
{
  char buf[80];
  strcpy (buf, string);

  if (TREE_CODE (arg) == COMPONENT_REF)
    {
      if (TREE_READONLY (TREE_OPERAND (arg, 0)))
	readonly_warning (TREE_OPERAND (arg, 0), string);
      else
	{
	  strcat (buf, " of read-only member `%s'");
	  warning (buf, IDENTIFIER_POINTER (DECL_NAME (TREE_OPERAND (arg, 1))));
	}
    }
  else if (TREE_CODE (arg) == VAR_DECL)
    {
      strcat (buf, " of read-only variable `%s'");
      warning (buf, IDENTIFIER_POINTER (DECL_NAME (arg)));
    }
  else
    {
      warning ("%s of read-only location", buf);
    }
}

/* Prepare expr to be an argument of a TRUTH_NOT_EXPR,
   or validate its data type for an `if' or `while' statement or ?..: exp.

   This preparation consists of taking the ordinary
   representation of an expression expr and producing a valid tree
   boolean expression describing whether expr is nonzero.  We could
   simply always do build_binary_op (NE_EXPR, expr, integer_zero_node),
   but we optimize comparisons, &&, ||, and !  */

tree
truthvalue_conversion (expr)
     tree expr;
{
  register enum tree_code form = TREE_CODE (expr);

  if (form == EQ_EXPR && integer_zerop (TREE_OPERAND (expr, 1)))
    return build_unary_op (TRUTH_NOT_EXPR,
			   truthvalue_conversion (TREE_OPERAND (expr, 0)), 0);

  /* A one-bit unsigned bit-field is already acceptable.  */
  if (form == COMPONENT_REF
      && 1 == TREE_INT_CST_LOW (DECL_SIZE (TREE_OPERAND (expr, 1)))
      && 1 == DECL_SIZE_UNIT (TREE_OPERAND (expr, 1))
      && TREE_UNSIGNED (TREE_OPERAND (expr, 1)))
    return expr;

  if (form == TRUTH_ANDIF_EXPR || form == TRUTH_ORIF_EXPR
      || form == TRUTH_AND_EXPR || form == TRUTH_OR_EXPR
      || form == TRUTH_NOT_EXPR
      || form == EQ_EXPR || form == NE_EXPR
      || form == LE_EXPR || form == GE_EXPR
      || form == LT_EXPR || form == GT_EXPR
      || form == ERROR_MARK)
    return expr;

  /* Unary minus has no effect on whether its argument is nonzero.  */
  if (form == NEGATE_EXPR)
    return truthvalue_conversion (TREE_OPERAND (expr, 0));

  /* Sign-extension and zero-extension has no effect.  */
  if (form == NOP_EXPR
      && TREE_CODE (TREE_TYPE (expr)) == INTEGER_TYPE
      && (TYPE_PRECISION (TREE_TYPE (expr))
	  > TYPE_PRECISION (TREE_TYPE (TREE_OPERAND (expr, 0)))))
    return truthvalue_conversion (TREE_OPERAND (expr, 0));

  return build_binary_op (NE_EXPR, expr, integer_zero_node);
}

/* Mark EXP saying that we need to be able to take the
   address of it; it should not be allocated in a register.

   C++: we do not allow `current_class_decl' to be addressable */

void
mark_addressable (exp)
     tree exp;
{
  register tree x = exp;
  while (1)
    switch (TREE_CODE (x))
      {
      case ADDR_EXPR:
      case COMPONENT_REF:
      case ARRAY_REF:
	x = TREE_OPERAND (x, 0);
	break;

      case PARM_DECL:
	if (x == current_class_decl)
	  {
	    error ("address of `this' not available");
	    TREE_ADDRESSABLE (x) = 1; /* so compiler doesn't die later */
	    put_var_into_stack (x);
	    return;
	  }
      case VAR_DECL:
      case CONST_DECL:
      case RESULT_DECL:
	if (TREE_REGDECL (x) && !TREE_ADDRESSABLE (x))
	  warning ("address requested for `%s', which is declared `register'",
		   IDENTIFIER_POINTER (DECL_NAME (x)));
	put_var_into_stack (x);

	/* drops in */
      case FUNCTION_DECL:
	TREE_ADDRESSABLE (x) = 1;

      default:
	return;
    }
}

/* Build and return a conditional expression IFEXP ? OP0 : OP1.  */

tree
build_x_conditional_expr (ifexp, op0, op1)
     tree ifexp, op0, op1;
{
  tree rval;

  if (rval = build_opfncall (COND_EXPR, ifexp, op0, op1))
    return rval;
  return build_conditional_expr (ifexp, op0, op1);
}

tree
build_conditional_expr (ifexp, op0, op1)
     tree ifexp, op0, op1;
{
  register tree type0;
  register tree type1;
  register enum tree_code code0;
  register enum tree_code code1;
  register tree result_type = NULL_TREE;

  if (ifexp == error_mark_node
      || op0 == error_mark_node
      || op1 == error_mark_node)
    return error_mark_node;

  /* If second operand is omitted, it is the same as the first one;
     make sure it is calculated only once.  */
  if (op0 == 0)
    {
      if (pedantic)
	warning ("ANSI C forbids omitting the middle term of a ?: expression");
      ifexp = op0 = save_expr (ifexp);
    }

  ifexp = truthvalue_conversion (ifexp);
  if (TREE_CODE (TREE_TYPE (op0)) != VOID_TYPE)
    op0 = default_conversion (op0);
  if (TREE_CODE (TREE_TYPE (op1)) != VOID_TYPE)
    op1 = default_conversion (op1);

  type0 = TREE_TYPE (op0);
  code0 = TREE_CODE (type0);
  type1 = TREE_TYPE (op1);
  code1 = TREE_CODE (type1);

  if (TREE_CODE (ifexp) == ERROR_MARK
      || code0 == ERROR_MARK || code1 == ERROR_MARK)
    return error_mark_node;
      
  /* Quickly detect the usual case where op0 and op1 have the same type.
     This is the only case that handles structures and unions.  */
  if (type0 == type1)
    result_type = type0;
  else if ((code0 == INTEGER_TYPE || code0 == REAL_TYPE)
           && (code1 == INTEGER_TYPE || code1 == REAL_TYPE))
    {
      result_type = commontype (type0, type1);
    }
  else if (code0 == VOID_TYPE || code1 == VOID_TYPE)
    {
      if (pedantic && (code0 != VOID_TYPE || code1 != VOID_TYPE))
	warning ("ANSI C forbids conditional expr with only one void side");
      result_type = void_type_node;
    }
  else if (code0 == POINTER_TYPE && code1 == POINTER_TYPE)
    {
      if (comptypes (type0, type1, 0))
	result_type = type0;
      else if (TYPE_MAIN_VARIANT (TREE_TYPE (type0)) == void_type_node)
	{
	  if (pedantic && TREE_CODE (type1) == FUNCTION_TYPE)
	    warning ("ANSI C forbids conditional expr between `void *' and function pointer");
	  result_type = qualify_type (type0, type1);
	}
      else if (TYPE_MAIN_VARIANT (TREE_TYPE (type1)) == void_type_node)
	{
	  if (pedantic && TREE_CODE (type0) == FUNCTION_TYPE)
	    warning ("ANSI C forbids conditional expr between `void *' and function pointer");
	  result_type = qualify_type (type1, type0);
	}
      /* C++ */
      else if (comptypes (type1, type0, 0))
	result_type = type1;
      else
	{
	  warning ("pointer type mismatch in conditional expression");
	  result_type = build_pointer_type (void_type_node);
	}
    }
  else if (code0 == POINTER_TYPE
	   && TREE_CODE (op1) == INTEGER_CST)
    {
      if (!integer_zerop (op1))
	warning ("pointer/integer type mismatch in conditional expression");
      if (pedantic && TREE_CODE (type0) == FUNCTION_TYPE)
	warning ("ANSI C forbids conditional expr between 0 and function pointer");
      result_type = type0;
      op1 = null_pointer_node;
    }
  else if (code1 == POINTER_TYPE
	   && TREE_CODE (op0) == INTEGER_CST)
    {
      if (!integer_zerop (op0))
	warning ("pointer/integer type mismatch in conditional expression");
      if (pedantic && TREE_CODE (type1) == FUNCTION_TYPE)
	warning ("ANSI C forbids conditional expr between 0 and function pointer");
      result_type = type1;
      op0 = null_pointer_node;
    }
  /* C++ */
  else if (code0 == REFERENCE_TYPE || code1 == REFERENCE_TYPE)
    {
      tree tt0 = code0 == REFERENCE_TYPE ? TREE_TYPE (type0) : type0;
      tree tt1 = code1 == REFERENCE_TYPE ? TREE_TYPE (type1) : type1;

      if (comptypes (tt0, tt1, 0))
	if (TREE_CODE (type1) == REFERENCE_TYPE
	    || TREE_CODE (type0) != REFERENCE_TYPE)
	  result_type = type1;
	else
	  result_type = type0;
    }

  if (!result_type)
    {
      if (flag_cond_mismatch)
	result_type = void_type_node;
      else
	{
	  error ("type mismatch in conditional expression");
	  return error_mark_node;
	}
    }

  if (result_type != TREE_TYPE (op0))
    op0 = convert (result_type, op0);
  if (result_type != TREE_TYPE (op1))
    op1 = convert (result_type, op1);

#if 0
  if (IS_AGGR_TYPE_CODE (code0))
    {
      result_type = TREE_TYPE (op0);
      if (TREE_LITERAL (ifexp))
	return (integer_zerop (ifexp) ? op1 : op0);

      if (TYPE_MODE (result_type) == BLKmode)
	{
	  register tree tempvar
	    = build_decl (VAR_DECL, NULL_TREE, result_type);
	  register tree xop0 = build_modify_expr (tempvar, NOP_EXPR, op0);
	  register tree xop1 = build_modify_expr (tempvar, NOP_EXPR, op1);
	  register tree result = build (COND_EXPR, result_type,
					ifexp, xop0, xop1);

	  layout_decl (tempvar);
	  /* No way to handle variable-sized objects here.
	     I fear that the entire handling of BLKmode conditional exprs
	     needs to be redone.  */
	  if (! TREE_LITERAL (DECL_SIZE (tempvar)))
	    abort ();
	  DECL_RTL (tempvar)
	    = assign_stack_local (DECL_MODE (tempvar),
				  (TREE_INT_CST_LOW (DECL_SIZE (tempvar))
				   * DECL_SIZE_UNIT (tempvar)
				   + BITS_PER_UNIT - 1)
				  / BITS_PER_UNIT);

	  TREE_VOLATILE (result)
	    = TREE_VOLATILE (ifexp) | TREE_VOLATILE (op0)
	      | TREE_VOLATILE (op1);
	  return build (COMPOUND_EXPR, result_type, result, tempvar);
	}
    }
#endif				/* 0 */

  if (TREE_LITERAL (ifexp))
    return (integer_zerop (ifexp) ? op1 : op0);

  return build (COND_EXPR, result_type, ifexp, op0, op1);
}

/* Given a list of expressions, return a compound expression
   that performs them all and returns the value of the last of them.  */

tree
build_compound_expr (list)
     tree list;
{
  register tree rest;

  if (TREE_CHAIN (list) == 0)
    return TREE_VALUE (list);

  rest = build_compound_expr (TREE_CHAIN (list));

  if (TREE_LITERAL (TREE_VALUE (list)))
    return rest;

  return build (COMPOUND_EXPR, TREE_TYPE (rest), TREE_VALUE (list), rest);
}

/* Build an expression representing a cast to type TYPE of expression EXPR.  */

tree
build_c_cast (type, expr)
     register tree type;
     tree expr;
{
  register tree value;

  if (type == error_mark_node)
    return error_mark_node;

  if (type == TREE_TYPE (expr))
    {
      if (pedantic)
	{
	  if (TREE_CODE (type) == RECORD_TYPE
	      || TREE_CODE (type) == UNION_TYPE)
	    warning ("ANSI C forbids casting nonscalar to the same type");
	}
      return expr;
    }

  if (TREE_TYPE (expr) == unknown_type_node)
    {
      if (TREE_CODE (expr) == OP_EXPR)
	expr = build_instantiated_decl (type, expr);
      else if (TREE_CODE (expr) == TREE_LIST)
	{
	  tree new_expr = build_tree_list (NULL_TREE, expr);
	  TREE_TYPE (new_expr) = unknown_type_node;
	  instantiate_type (type, new_expr);
	  if (TREE_TYPE (new_expr) != error_mark_node)
	    expr = TREE_VALUE (new_expr);
	}
      else
	instantiate_type (type, expr);
      /* Did we lose?  */
      if (TREE_TYPE (expr) == error_mark_node)
	return error_mark_node;
      /* If not, then the FIELD_DECL points to the FUNCTION_DECL
	 that we want.  */
      if (TREE_CODE (expr) == COMPONENT_REF)
	expr = TREE_TYPE (TREE_OPERAND (expr, 1));
    }

  value = convert (TYPE_MAIN_VARIANT (type), default_conversion (expr));
  /* As far as I know, it is not meaningful to cast something
     to a const or volatile type, because those are meaningful
     only for lvalues.
     But if it is meaningful, we must somehow return something
     whose TREE_READONLY or TREE_VOLATILE is set.
     That is not trivial because it is possible that VALUE == EXPR
     or is a shared constant.  */
  return value;
}

/* Build an assignment expression of lvalue LHS from value RHS.

   In C++, if the left hand side of the assignment is a REFERENCE_TYPE,
   that reference becomes deferenced down to it base type. */

/* Build an assignment expression of lvalue LHS from value RHS.
   MODIFYCODE is the code for a binary operator that we use
   to combine the old value of LHS with RHS to get the new value.
   Or else MODIFYCODE is NOP_EXPR meaning do a simple assignment.

   C++: If MODIFYCODE is INIT_EXPR, then leave references unbashed.  */
tree
build_modify_expr (lhs, modifycode, rhs)
     tree lhs, rhs;
     enum tree_code modifycode;
{
  register tree result;
  tree newrhs = rhs;
  tree lhstype = TREE_TYPE (lhs);
  tree olhstype = lhstype;

  /* Types that aren't fully specified cannot be used in assignments.  */
  lhs = require_complete_type (lhs);

  /* Avoid duplicate error messages from operands that had errors.  */
  if (TREE_CODE (lhs) == ERROR_MARK || TREE_CODE (rhs) == ERROR_MARK)
    return error_mark_node;

  /* Handle control structure constructs used as "lvalues".  */

  if (!pedantic)
    switch (TREE_CODE (lhs))
      {
	/* Handle (a, b) used as an "lvalue".  */
      case COMPOUND_EXPR:
	return build (COMPOUND_EXPR, lhstype,
		      TREE_OPERAND (lhs, 0),
		      build_modify_expr (TREE_OPERAND (lhs, 1),
					 modifycode, rhs));

	/* Handle (a ? b : c) used as an "lvalue".  */
      case COND_EXPR:
	rhs = save_expr (rhs);
	return (build_conditional_expr
		(TREE_OPERAND (lhs, 0),
		 build_modify_expr (TREE_OPERAND (lhs, 1), modifycode, rhs),
		 build_modify_expr (TREE_OPERAND (lhs, 2), modifycode, rhs)));
      }

  /* If a binary op has been requested, combine the old LHS value with the RHS
     producing the value we should actually store into the LHS.  */

  if (modifycode == INIT_EXPR)
    ;
  else if (modifycode == NOP_EXPR)
    {
      /* must deal with overloading here.  */
      if (TREE_CODE (lhstype) == REFERENCE_TYPE)
	lhstype = TREE_TYPE (lhstype);
      if (TREE_HAS_ASSIGNMENT (TYPE_MAIN_VARIANT (lhstype)))
	{
	  result = build_opfncall (MODIFY_EXPR, lhs, rhs, NOP_EXPR);
	  if (result == NULL_TREE)
	    return error_mark_node;
	  return result;
	}
      lhstype = olhstype;
    }
  else
    {
      lhs = stabilize_reference (lhs);
      newrhs = build_binary_op (modifycode, lhs, rhs);
    }

  /* Handle a cast used as an "lvalue".
     We have already performed any binary operator using the value as cast.
     Now convert the result to the true type of the lhs and store there;
     then cast the result back to the specified type to be the value
     of the assignment.  */

  if (!pedantic)
    switch (TREE_CODE (lhs))
      {
      case NOP_EXPR:
      case CONVERT_EXPR:
      case FLOAT_EXPR:
      case FIX_TRUNC_EXPR:
      case FIX_FLOOR_EXPR:
      case FIX_ROUND_EXPR:
      case FIX_CEIL_EXPR:
	if (TREE_CODE (TREE_TYPE (newrhs)) == ARRAY_TYPE
	    || TREE_CODE (TREE_TYPE (newrhs)) == FUNCTION_TYPE)
	  newrhs = default_conversion (newrhs);
	{
	  tree inner_lhs = TREE_OPERAND (lhs, 0);
	  tree result = build_modify_expr (inner_lhs, NOP_EXPR,
					   convert (TREE_TYPE (inner_lhs),
						    newrhs));
	  return convert (TREE_TYPE (lhs), result);
	}
      }

  /* C++: handle assignment to static class members that have not
     been fixed up yet.  The field is already valid, as is the base type.  */
  if (TREE_CODE (lhs) == SCOPE_REF)
    {
      tree field = TREE_OPERAND (lhs, 1);
      if (TREE_STATIC (field))
	{
	  lhs = IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (field));
	}
      else 
	abort ();
    }

  /* Now we have handled acceptable kinds of LHS that are not truly lvalues.
     Reject anything strange now.  */

  if (!lvalue_or_else (lhs, "assignment"))
    return error_mark_node;

  /* Warn about storing in something that is `const'.  */

  if (TREE_READONLY (lhs)
      || ((TREE_CODE (lhstype) == RECORD_TYPE
	   || TREE_CODE (lhstype) == UNION_TYPE)
	  && C_TYPE_FIELDS_READONLY (lhstype)))
    readonly_warning (lhs, "assignment");

  /* If storing into a structure or union member,
     it has probably been given type `int'.
     Compute the type that would go with
     the actual amount of storage the member occupies.  */

  if (TREE_CODE (lhs) == COMPONENT_REF
      && (TREE_CODE (lhstype) == INTEGER_TYPE
	  || TREE_CODE (lhstype) == REAL_TYPE
	  || TREE_CODE (lhstype) == ENUMERAL_TYPE))
    lhstype = TREE_TYPE (get_unwidened (lhs, 0));

  /* check to see if there is an assignment to `this' */
  if (lhs == current_class_decl)
    {
      if (current_class_name != current_function_name)
	{
	  warning ("assignment to `this' only allowed in constructors and destructors");
	}
      else
	current_function_just_assigned_this = 1;
    }

  /* The TREE_TYPE of RHS may be TYPE_UNKNOWN.  This can happen
     when the type of RHS is not yet known, i.e. its type
     is inherited from LHS.  */
  if (TREE_TYPE (rhs) == unknown_type_node)
    {
      if (TREE_CODE (rhs) == OP_EXPR)
	rhs = build_instantiated_decl (lhstype, rhs);
      else if (TREE_CODE (rhs) == TREE_LIST)
	{
	  tree new_rhs = build_tree_list (NULL_TREE, rhs);
	  TREE_TYPE (new_rhs) = unknown_type_node;
	  instantiate_type (lhstype, new_rhs);
	  if (TREE_TYPE (new_rhs) != error_mark_node)
	    rhs = TREE_VALUE (new_rhs);
	  else
	    return error_mark_node;
	}
      else
	{
	  instantiate_type (lhstype, rhs);
	  if (TREE_TYPE (rhs) == error_mark_node)
	    return error_mark_node;
	}
      newrhs = rhs;
    }

  if (modifycode != INIT_EXPR)
    {
      /* Make modifycode now either a NOP_EXPR or an INIT_EXPR.  */
      modifycode = NOP_EXPR;
      /* Reference-bashing */
      if (TREE_CODE (lhstype) == REFERENCE_TYPE)
	{
	  lhs = bash_reference_type (lhs);
	  lhstype = TREE_TYPE (lhs);
	  olhstype = lhstype;
	}
      if (TREE_CODE (TREE_TYPE (rhs)) == REFERENCE_TYPE)
	{
	  rhs = bash_reference_type (rhs);
	  newrhs = rhs;
	}
    }

  /* C++: The semantics of C++ differ from those of C when an
     assignment of an aggregate is desired.  Assignment in C++ is
     now defined as memberwise assignment of non-static members
     and base class objects.  This rule applies recursively
     until a member of a built-in type is found.

     Also, we cannot do a bit-wise copy of aggregates which
     contain virtual function table pointers.  Those
     pointer values must be preserved through the copy.
     However, this is handled in expand_expr, and not here.
     This is because much better code can be generated at
     that stage than this one.  */
  if (modifycode != INIT_EXPR
      && TREE_GETS_ASSIGNMENT (lhstype)
      && check_base_type (lhstype, TREE_TYPE (rhs)))
    {
      result = NULL_TREE;

      while (TREE_GETS_ASSIGNMENT (lhstype))
	{
	  /* Perform assignment on each member, depth-first,
	     left-right.  */
	  register tree elt;

	  if (TYPE_BASELINK (lhstype))
	    {
	      tree base_lhs;
	      tree base_rhs, rhstype = TREE_TYPE (rhs);

	      if (lhstype == rhstype)
		{
		  base_lhs = copy_node (lhs);
		  TREE_TYPE (base_lhs) = TYPE_BASELINK (lhstype);
		  base_rhs = copy_node (rhs);
		  TREE_TYPE (base_rhs) = TYPE_BASELINK (rhstype);
		}
	      else
		{
		  while (rhstype && lhstype != rhstype)
		    rhstype = TYPE_BASELINK (rhstype);

		  if (rhstype == NULL_TREE)
		    {
		      error ("illegal assignment between aggregate types");
		      return error_mark_node;
		    }

		  base_lhs = lhs;
		  base_rhs = copy_node (rhs);
		  TREE_TYPE (base_rhs) = rhstype;
		}

	      if (! TYPE_FIELDS (lhstype))
		{
		  lhs = base_lhs;
		  rhs = base_rhs;
		  lhstype = TREE_TYPE (lhs);
		  continue;
		}
	      result = build_modify_expr (base_lhs, modifycode, base_rhs);
	      /* We either get back a compound stmt, or a simple one.  */
	      if (TREE_CODE (result) != TREE_LIST)
		result = build_tree_list (NULL_TREE, result);
	    }
	  for (elt = TYPE_FIELDS (lhstype); elt; elt = TREE_CHAIN (elt))
	    if (! VFIELD_NAME_P (DECL_NAME (elt)))
	      {
		tree elt_lhs = build (COMPONENT_REF, TREE_TYPE (elt), lhs, elt);
		tree elt_rhs = build (COMPONENT_REF, TREE_TYPE (elt), rhs, elt);
		/* It is not always safe to go through `build_modify_expr'
		   when performing element-wise copying.  This is because
		   an element may be of ARRAY_TYPE, which will not
		   be properly copied as a naked element.  */
		if (TREE_GETS_ASSIGNMENT (TREE_TYPE (elt_lhs))
		    || TREE_GETS_INIT_REF (TREE_TYPE (elt_lhs)))
		  elt_lhs = build_modify_expr (elt_lhs, modifycode, elt_rhs);
		else
		  elt_lhs = build (modifycode == NOP_EXPR ? MODIFY_EXPR : INIT_EXPR,
				   void_type_node, elt_lhs, elt_rhs);
		result = tree_cons (NULL_TREE, elt_lhs, result);
	      }
	  return build_compound_expr (result);
	}
    }

  /* If storing in a field that is in actuality a short or narrower than one,
     we must store in the field in its actual type.  */

  if (lhstype != TREE_TYPE (lhs))
    {
      lhs = copy_node (lhs);
      TREE_TYPE (lhs) = lhstype;
    }

  /* Convert new value to destination type.  */

  if (modifycode == INIT_EXPR)
    {
      newrhs = convert_for_initialization (lhs, lhstype, newrhs, "assignment");
      if (lhs == DECL_RESULT (current_function_decl))
	{
	  if (DECL_INITIAL (DECL_RESULT (current_function_decl)))
	    warning ("return value from function receives multiple initializations");
	  DECL_INITIAL (lhs) = newrhs;
	}
    }
  else
    newrhs = convert_for_assignment (lhstype, newrhs, "assignment");

  result = build (modifycode == NOP_EXPR ? MODIFY_EXPR : INIT_EXPR,
		  lhstype, lhs, newrhs);
  TREE_VOLATILE (result) = 1;

  /* If we got the LHS in a different type for storing in,
     convert the result back to the nominal type of LHS
     so that the value we return always has the same type
     as the LHS argument.  */

  if (olhstype == TREE_TYPE (result))
    return result;
  return convert_for_assignment (olhstype, result, "assignment");
}

/* Return 0 if EXP is not a valid lvalue in this language
   even though `lvalue_or_else' would accept it.  */

int
language_lvalue_valid (exp)
     tree exp;
{
  return 1;
}

/* Convert value RHS to type TYPE as preparation for an assignment
   to an lvalue of type TYPE.
   The real work of conversion is done by `convert'.
   The purpose of this function is to generate error messages
   for assignments that are not allowed in C.
   ERRTYPE is a string to use in error messages:
   "assignment", "return", etc.

   C++: attempts to allow `convert' to find conversions involving
   implicit type conversion between aggregate and scalar types
   as per 8.5.6 of C++ manual.  Does not randomly dereference
   pointers to aggregates!  */

static tree
convert_for_assignment (type, rhs, errtype)
     tree type, rhs;
     char *errtype;
{
  register enum tree_code codel = TREE_CODE (type);
  register tree rhstype = datatype (rhs);
  register enum tree_code coder = TREE_CODE (rhstype);

  if (coder == ERROR_MARK)
    return rhs;

  if (TREE_CODE (TREE_TYPE (rhs)) == ARRAY_TYPE
      || TREE_CODE (TREE_TYPE (rhs)) == FUNCTION_TYPE)
    {
      rhs = default_conversion (rhs);
    }
  else if (coder == REFERENCE_TYPE)
    {
      rhs = bash_reference_type (rhs);
      rhstype = TREE_TYPE (rhs);
      coder = TREE_CODE (rhstype);
    }

  rhs = maybe_convert_decl_to_const (rhs);
  if (TREE_LITERAL (rhs))
    rhstype = TREE_TYPE (rhs);

  if (type == rhstype)
    return rhs;

  if (coder == VOID_TYPE)
    {
      error ("void value not ignored as it ought to be");
      return error_mark_node;
    }
  /* Arithmetic types all interconvert, and enum is treated like int.  */
  if ((codel == INTEGER_TYPE || codel == REAL_TYPE || codel == ENUMERAL_TYPE)
       &&
      (coder == INTEGER_TYPE || coder == REAL_TYPE || coder == ENUMERAL_TYPE))
    {
      return convert (type, rhs);
    }
  /* Conversions among pointers */
  else if (codel == POINTER_TYPE && coder == POINTER_TYPE)
    {
      register tree ttl = TREE_TYPE (type);
      register tree ttr = TREE_TYPE (rhstype);
      /* Anything converts to void *.  void * converts to anything.
	 Otherwise, the targets must be the same except that the
	 lhs target may be const or volatile while the rhs target isn't.  */

      if (TREE_MEMBER_POINTER (ttl) != TREE_MEMBER_POINTER (ttr))
	{
	  error ("conversion between member pointer and non-member pointer types");
	  return error_mark_node;
	}

      if (comp_target_types (type, rhstype))
	{
	  if (! TREE_READONLY (ttl) && TREE_READONLY (ttr))
	    warning ("%s of non-const * pointer from const *", errtype);
	  if (! TREE_VOLATILE (ttl) && TREE_VOLATILE (ttr))
	    warning ("%s of non-volatile * pointer from volatile *", errtype);
	}
      else if (!((ttl == void_type_node
		  && !(pedantic && TREE_CODE (ttr) != FUNCTION_TYPE))
		 || (ttr == void_type_node
		     && !(pedantic && TREE_CODE (ttl) != FUNCTION_TYPE))))
	warning ("%s between incompatible pointer types", errtype);
      return convert (type, rhs);
    }
  else if (codel == POINTER_TYPE && coder == INTEGER_TYPE)
    {
      if (! integer_zerop (rhs))
	{
	  warning ("%s of pointer from integer lacks a cast", errtype);
	  return convert (type, rhs);
	}
      return null_pointer_node;
    }
  else if (codel == INTEGER_TYPE && coder == POINTER_TYPE)
    {
      warning ("%s of integer from pointer lacks a cast", errtype);
      return convert (type, rhs);
    }

  /* C++ */
  else if (codel == ERROR_MARK || coder == ERROR_MARK)
    return error_mark_node;

  /* This should no longer happen.  References are initialzed via
     `convert_for_initialization'.  They should otherwise be
     bashed before coming here.  */
  else if (codel == REFERENCE_TYPE)
    abort ();

  else if (TREE_HAS_CONSTRUCTOR (type) || IS_AGGR_TYPE (TREE_TYPE (rhs)))
    {
      return convert (type, rhs);
    }

  error ("incompatible types in %s", errtype);
  return error_mark_node;
}

/* Convert RHS to be of type TYPE.  If EXP is non-zero,
   it is the target of the initialization.
   ERRTYPE is a string to use in error messages.

   Two major differences between the behavior of
   `convert_for_assignment' and `convert_for_initialization'
   are that references are bashed in the former, while
   copied in the latter, and aggregates are assigned in
   the former (operator=) while initialized in the
   latter (X(X&)).  */
static tree
convert_for_initialization (exp, type, rhs, errtype)
     tree exp, type, rhs;
     char *errtype;
{
  register enum tree_code codel = TREE_CODE (type);
  register tree rhstype = datatype (rhs);
  register enum tree_code coder = TREE_CODE (rhstype);

  if (coder == ERROR_MARK)
    return rhs;

  if (codel == REFERENCE_TYPE)
    {
      extern tree convert_to_reference ();
      return convert_to_reference (exp, type, rhs, 0);
    }

  if (TREE_CODE (rhstype) == REFERENCE_TYPE)
    rhstype = TREE_TYPE (rhstype);

  if (TREE_HAS_CONSTRUCTOR (type) || IS_AGGR_TYPE (rhstype))
    {
      if (check_base_type (TYPE_MAIN_VARIANT (type), TYPE_MAIN_VARIANT (rhstype)))
	{
	  if (TREE_GETS_INIT_REF (type))
	    {
	      if (TREE_CODE (rhstype) != REFERENCE_TYPE)
		rhs = convert_to_reference (0, TYPE_REFERENCE_TO (type), rhs, 0);
	      
	      if (exp)
		{
		  exp = copy_node (exp);

		  if (IS_AGGR_TYPE (TREE_TYPE (exp)))
		    TREE_TYPE (exp) = type;
		  else if (IS_AGGR_TYPE (rhstype))
		    TREE_TYPE (exp) = build_reference_type (type);
		  else
		    error ("conversion to aggregate type from non-aggregate type (compiler error)");
		  exp = build_indirect_ref (build_classfn_ref (exp, DECL_NAME (TYPE_NAME (type)),
					    build_tree_list (NULL_TREE, rhs), 1, 1),
					    "X(X&) constructor (compiler error)");
		}
	      else
		exp = get_temp_name (type, NULL_TREE, rhs);
	      return exp;
	    }
	  if (TREE_GETS_ASSIGNMENT (type))
	    warning ("bitwise copy: `%s' has a member with operator=()",
		     TYPE_NAME_STRING (type));

	  if (TREE_CODE (TREE_TYPE (rhs)) == REFERENCE_TYPE)
	    return bash_reference_type (rhs);
	  return rhs;
	}

      return convert (type, rhs);
    }
  if (TREE_GETS_ASSIGNMENT (type))
    warning ("bitwise copy: `%s' has a member with operator=()",
	     TYPE_NAME_STRING (type));

  return convert_for_assignment (type, rhs, errtype);
}

/* Return nonzero if VALUE is a valid constant-valued expression
   for use in initializing a static variable; one that can be an
   element of a "constant" initializer.

   Return 1 if the value is absolute; return 2 if it is relocatable.
   We assume that VALUE has been folded as much as possible;
   therefore, we do not need to check for such things as
   arithmetic-combinations of integers.  */

static int
initializer_constant_valid_p (value)
     tree value;
{
  switch (TREE_CODE (value))
    {
    case CONSTRUCTOR:
      return TREE_STATIC (value);

    case INTEGER_CST:
    case REAL_CST:
    case STRING_CST:
      return 1;

    case ADDR_EXPR:
      return 2;

    case CONVERT_EXPR:
      /* Allow (int) &foo.  */
      if (TREE_CODE (TREE_TYPE (value)) == INTEGER_TYPE
	  && TREE_CODE (TREE_TYPE (TREE_OPERAND (value, 0))) == POINTER_TYPE)
	return initializer_constant_valid_p (TREE_OPERAND (value, 0));
      return 0;

    case NOP_EXPR:
      return initializer_constant_valid_p (TREE_OPERAND (value, 0));

    case PLUS_EXPR:
      {
	int valid0 = initializer_constant_valid_p (TREE_OPERAND (value, 0));
	int valid1 = initializer_constant_valid_p (TREE_OPERAND (value, 1));
	if (valid0 == 1 && valid1 == 2)
	  return 2;
	if (valid0 == 2 && valid1 == 1)
	  return 2;
	return 0;
      }

    case MINUS_EXPR:
      {
	int valid0 = initializer_constant_valid_p (TREE_OPERAND (value, 0));
	int valid1 = initializer_constant_valid_p (TREE_OPERAND (value, 1));
	if (valid0 == 2 && valid1 == 1)
	  return 2;
	return 0;
      }
    }

  return 0;
}

/* Perform appropriate conversions on the initial value of a variable,
   store it in the declaration DECL if it is valid, and print any error
   messages that are appropriate.

   C++: Note that INIT might be a TREE_LIST, which would mean that it is
   a base class initializer for some aggregate type, hopefully compatible
   with DECL.  If INIT is a single element, and DECL is an aggregate
   type, we silently convert INIT into a TREE_LIST, allowing a constructor
   to be called.

   If INIT is a TREE_LIST and there is no constructor, turn INIT
   into a CONSTRUCTOR and use standard initialization techniques.
   Perhaps a warning should be generated?  */

void
store_init_value (decl, init)
     tree decl, init;
{
  register tree value, type;

  /* If variable's type was invalidly declared, just ignore it.  */

  type = TREE_TYPE (decl);
  if (TREE_CODE (type) == ERROR_MARK)
    return;

  /* Take care of C++ business up here.  */
  type = TYPE_MAIN_VARIANT (type);

  /* implicitly tests if IS_AGGR_TYPE.  */
  if (TREE_NEEDS_CONSTRUCTOR (type))
    {
      if (TREE_CODE (init) == TREE_LIST
	  || (TREE_CODE (init) != CONSTRUCTOR
	      && TYPE_MAIN_VARIANT (TREE_TYPE (init)) == type))
	{
#if 0
	  /* Special case: check for X = X(X&).  */
	  if (TREE_HAS_ASSIGN_REF (type))
	    {
	      /* @@ This may be too restrictive.  */
	      tree tmp = build_opfncall (MODIFY_EXPR,
					 decl, init, (tree)NOP_EXPR);
	      if (tmp != error_mark_node)
		{
		  init = build_classfn_ref (decl, DECL_NAME (TYPE_NAME (type)),
					    build_tree_list (NULL_TREE, init), 0, 1);
		  tmp = tree_cons (NULL_TREE, init, build_tree_list (NULL_TREE, tmp));
		  DECL_INITIAL (decl) = build_compound_expr (tmp);
		}
	      else
		{
		  warning ("construct X = X(X&) failed");
		  /* No need to fool around here.  */
		  DECL_INITIAL (decl) = init;
		}
	    }
	  else
#endif
	    {
	      /* No need to fool around here.  */
	      DECL_INITIAL (decl) = init;
	    }
	  return;
	}
      if (TREE_CODE (init) != CONSTRUCTOR)
	{
	  /* If the value going into the aggregate type
	     is not a CONSTRUCTOR, then if it is a TREE_LIST,
	     the TREE_LIST is an argument list for some
	     constructor. Otherwise it is some expression, which
	     acts as an argument to an implicit constructor for
	     that pseudo-argument.  */
	  if (TREE_CODE (init) != TREE_LIST)
	    init = build_tree_list (NULL_TREE, init);
	  DECL_INITIAL (decl) = init;
	  return;
	}
    }
  else if (IS_AGGR_TYPE (type))
    {
      /* @@ This may be wrong, but I do not know what is right.  */
      if (TREE_CODE (init) == TREE_LIST)
	{
	  warning ("constructor syntax used, but no constructor declared for type `%s'",
		   TYPE_NAME_STRING (type));
	  init = build_nt (CONSTRUCTOR, NULL_TREE, nreverse (init));
	}
    }

  /* End of special C++ code.  */

  /* Digest the specified initializer into an expression.  */

  value = digest_init (type, init, 0);

  /* Store the expression if valid; else report error.  */

  if (value == error_mark_node)
    ;
  else if (TREE_STATIC (decl) && ! TREE_LITERAL (value))
    error ("initializer for static variable is not constant");
  else if (TREE_STATIC (decl)
	   && ! initializer_constant_valid_p (value))
    error ("initializer for static variable uses complex arithmetic");
  else
    {
      DECL_INITIAL (decl) = value;
      /* C++: take care of possibly long-lived constants.  */
      if (TREE_LITERAL (value) && (TREE_READONLY (decl) || TREE_READONLY (TREE_TYPE (decl))))
	make_rtx_constant_now (value);

      if (pedantic && TREE_CODE (value) == CONSTRUCTOR)
	{
	  if (! TREE_LITERAL (value))
	    warning ("aggregate initializer is not constant");
	  else if (! TREE_STATIC (value))
	    warning ("aggregate initializer uses complex arithmetic");
	}
    }
}

/* Digest the parser output INIT as an initializer for type TYPE.
   Return a C expression of type TYPE to represent the initial value.

   If TAIL is nonzero, it points to a variable holding a list of elements
   of which INIT is the first.  We update the list stored there by
   removing from the head all the elements that we use.
   Normally this is only one; we use more than one element only if
   TYPE is an aggregate and INIT is not a constructor.  */

tree
digest_init (type, init, tail)
     tree type, init, *tail;
{
  enum tree_code code = TREE_CODE (type);
  tree element = 0;
  tree old_tail_contents;
  /* Nonzero if INIT is a braced grouping, which comes in as a CONSTRUCTOR
     tree node which has no TREE_TYPE.  */
  int raw_constructor
    = TREE_CODE (init) == CONSTRUCTOR && TREE_TYPE (init) == 0;

  /* By default, assume we use one element from a list.
     We correct this later in the sole case where it is not true.  */

  if (tail)
    {
      old_tail_contents = *tail;
      *tail = TREE_CHAIN (*tail);
    }

  if (init == error_mark_node)
    return init;

  if (init && raw_constructor
      && CONSTRUCTOR_ELTS (init) != 0
      && TREE_CHAIN (CONSTRUCTOR_ELTS (init)) == 0)
    element = TREE_VALUE (CONSTRUCTOR_ELTS (init));

  /* Any type can be initialized from an expression of the same type,
     optionally with braces.  */

  if (init && (TREE_TYPE (init) == type
	       || (code == ARRAY_TYPE && TREE_TYPE (init)
		   && comptypes (TREE_TYPE (init), type))))
    {
      if (pedantic && code == ARRAY_TYPE)
	warning ("ANSI C forbids initializing array from array expression");
      return init;
    }

  if (element && (TREE_TYPE (element) == type
		  || (code == ARRAY_TYPE && TREE_TYPE (element)
		      && comptypes (TREE_TYPE (element), type))))
    {
      if (pedantic && code == ARRAY_TYPE)
	warning ("ANSI C forbids initializing array from array expression");
      if (pedantic && (code == RECORD_TYPE || code == UNION_TYPE))
	warning ("single-expression nonscalar initializer has braces");
      return maybe_convert_decl_to_const (element);
    }

  /* Check for initializing a union by its first field.
     Such an initializer must use braces.  */

  if (code == UNION_TYPE)
    {
      tree result;

      if (TYPE_FIELDS (type) == 0)
	{
	  error ("union with no members cannot be initialized");
	  return error_mark_node;
	}
      if (raw_constructor)
	{
	  error ("type mismatch in initialization");
	  return error_mark_node;
	}
      if (element == 0)
	{
	  error ("union initializer requires one element");
	  return error_mark_node;
	}
      /* Take just the first element from within the constructor
	 and it should match the type of the first element.  */
      element = digest_init (TREE_TYPE (TYPE_FIELDS (type)), element, 0);
      result = build (CONSTRUCTOR, type, 0, build_tree_list (0, element));
      TREE_LITERAL (result) = TREE_LITERAL (element);
      TREE_STATIC (result) = (initializer_constant_valid_p (element)
			      && TREE_LITERAL (element));
      return result;
    }

  /* Initialization of an array of chars from a string constant
     optionally enclosed in braces.  */

  if (code == ARRAY_TYPE
      && (TREE_TYPE (type) == char_type_node
	  || TREE_TYPE (type) == signed_char_type_node
	  || TREE_TYPE (type) == unsigned_char_type_node
	  || TREE_TYPE (type) == unsigned_type_node
	  || TREE_TYPE (type) == integer_type_node)
      && ((init && TREE_CODE (init) == STRING_CST)
	  || (element && TREE_CODE (element) == STRING_CST)))
    {
      tree string = element ? element : init;

      if (TREE_TYPE (TREE_TYPE (string)) != char_type_node
	  && TYPE_PRECISION (TREE_TYPE (type)) == BITS_PER_UNIT)
	{
	  error ("char-array initialized from wide string");
	  return error_mark_node;
	}
      if (TREE_TYPE (TREE_TYPE (string)) == char_type_node
	  && TYPE_PRECISION (TREE_TYPE (type)) != BITS_PER_UNIT)
	{
	  error ("int-array initialized from non-wide string");
	  return error_mark_node;
	}

      if (pedantic && TREE_TYPE (type) != char_type_node)
	warning ("ANSI C forbids string initializer except for `char' elements");
      TREE_TYPE (string) = type;
      if (TYPE_DOMAIN (type) != 0
	  && TREE_LITERAL (TYPE_SIZE (type)))
	{
	  register int size
	    = TREE_INT_CST_LOW (TYPE_SIZE (type)) * TYPE_SIZE_UNIT (type);
	  size = (size + BITS_PER_UNIT - 1) / BITS_PER_UNIT;
	  /* Subtract 1 because it's ok to ignore the terminating null char
	     that is counted in the length of the constant.  */
	  if (size < TREE_STRING_LENGTH (string) - 1)
	    warning ("initializer-string for array of chars is too long");
	}
      return string;
    }

  /* Handle scalar types, including conversions.  */

  if (code == INTEGER_TYPE || code == REAL_TYPE || code == POINTER_TYPE
      || code == ENUMERAL_TYPE || code == REFERENCE_TYPE)
    {
      if (raw_constructor)
	{
	  if (element == 0)
	    {
	      error ("initializer for scalar variable requires one element");
	      return error_mark_node;
	    }
	  init = element;
	}

      return convert_for_initialization (0, type, init, "initialization");
    }

  /* Come here only for records and arrays.  */

  if (TYPE_SIZE (type) && ! TREE_LITERAL (TYPE_SIZE (type)))
    {
      error ("variable-sized object may not be initialized");
      return error_mark_node;
    }

  if (code == ARRAY_TYPE || code == RECORD_TYPE)
    {
      tree result = 0;
      if (raw_constructor)
	return process_init_constructor (type, init, 0);
      else if (tail != 0)
	{
	  *tail = old_tail_contents;
	  return process_init_constructor (type, 0, tail);
	}
    }

  error ("invalid initializer");
  return error_mark_node;
}

/* Process a constructor for a variable of type TYPE.
   The constructor elements may be specified either with INIT or with ELTS,
   only one of which should be non-null.

   If INIT is specified, it is a CONSTRUCTOR node which is specifically
   and solely for initializing this datum.

   If ELTS is specified, it is the address of a variable containing
   a list of expressions.  We take as many elements as we need
   from the head of the list and update the list.

   In the resulting constructor, TREE_LITERAL is set if all elts are
   constant, and TREE_STATIC is set if, in addition, all elts are simple enough
   constants that the assembler and linker can compute them.  */

static tree
process_init_constructor (type, init, elts)
     tree type, init, *elts;
{
  register tree tail;
  /* List of the elements of the result constructor,
     in reverse order.  */
  register tree members = NULL;
  tree result;
  int allconstant = 1;
  int allsimple = 1;
  int error = 0;

  /* Make TAIL be the list of elements to use for the initialization,
     no matter how the data was given to us.  */

  if (elts)
    tail = *elts;
  else
    tail = CONSTRUCTOR_ELTS (init);

  /* Gobble as many elements as needed, and make a constructor or initial value
     for each element of this aggregate.  Chain them together in result.
     If there are too few, use 0 for each scalar ultimate component.  */

  if (TREE_CODE (type) == ARRAY_TYPE)
    {
      tree domain = TYPE_DOMAIN (type);
      register long len;
      register int i;

      if (domain)
	len = TREE_INT_CST_LOW (TYPE_MAX_VALUE (domain))
	  - TREE_INT_CST_LOW (TYPE_MIN_VALUE (domain))
	    + 1;
      else
	len = -1;  /* Take as many as there are */

      for (i = 0; (len < 0 || i < len) && tail != 0; i++)
	{
	  register tree next1;

	  if (TREE_VALUE (tail) != 0)
	    {
	      tree tail1 = tail;
	      next1 = digest_init (TREE_TYPE (type),
				   TREE_VALUE (tail), &tail1);
	      tail = tail1;
	    }
	  else
	    {
	      next1 = error_mark_node;
	      tail = TREE_CHAIN (tail);
	    }

	  if (next1 == error_mark_node)
	    error = 1;
	  else if (!TREE_LITERAL (next1))
	    allconstant = 0;
	  else if (! initializer_constant_valid_p (next1))
	    allsimple = 0;
	  members = tree_cons (NULL_TREE, next1, members);
	}
    }
  if (TREE_CODE (type) == RECORD_TYPE)
    {
      register tree field;

      for (field = TYPE_FIELDS (type); field && tail;
	   field = TREE_CHAIN (field))
	{
	  register tree next1;

	  if (TREE_VALUE (tail) != 0)
	    {
	      tree tail1 = tail;
	      next1 = digest_init (TREE_TYPE (field),
				   TREE_VALUE (tail), &tail1);
	      tail = tail1;
	    }
	  else
	    {
	      next1 = error_mark_node;
	      tail = TREE_CHAIN (tail);
	    }

	  if (next1 == error_mark_node)
	    error = 1;
	  else if (!TREE_LITERAL (next1))
	    allconstant = 0;
	  else if (! initializer_constant_valid_p (next1))
	    allsimple = 0;
	  members = tree_cons (field, next1, members);
	}
    }

  /* If arguments were specified as a list, just remove the ones we used.  */
  if (elts)
    *elts = tail;
  /* If arguments were specified as a constructor,
     complain unless we used all the elements of the constructor.  */
  else if (tail)
    warning ("excess elements in aggregate initializer");

  if (error)
    return error_mark_node;

  result = build (CONSTRUCTOR, type, NULL_TREE, nreverse (members));
  if (allconstant) TREE_LITERAL (result) = 1;
  if (allconstant && allsimple) TREE_STATIC (result) = 1;
  return result;
}

/* Expand an ASM statement with operands, handling output operands
   that are not variables or INDIRECT_REFS by transforming such
   cases into cases that expand_asm_operands can handle.

   Arguments are same as for expand_asm_operands.  */

void
c_expand_asm_operands (string, outputs, inputs, vol)
     tree string, outputs, inputs;
     int vol;
{
  int noutputs = list_length (outputs);
  register int i;
  /* o[I] is the place that output number I should be written.  */
  register tree *o = (tree *) alloca (noutputs * sizeof (tree));
  register tree tail;

  /* Record the contents of OUTPUTS before it is modifed.  */
  for (i = 0, tail = outputs; tail; tail = TREE_CHAIN (tail), i++)
    o[i] = TREE_VALUE (tail);

  /* Generate the ASM_OPERANDS insn;
     store into the TREE_VALUEs of OUTPUTS some trees for
     where the values were actually stored.  */
  expand_asm_operands (string, outputs, inputs, vol);

  /* Copy all the intermediate outputs into the specified outputs.  */
  for (i = 0, tail = outputs; tail; tail = TREE_CHAIN (tail), i++)
    if (o[i] != TREE_VALUE (tail))
      expand_expr (build_modify_expr (o[i], NOP_EXPR, TREE_VALUE (tail)),
		   0, VOIDmode, 0);
}

/* Expand a C `return' statement.
   RETVAL is the expression for what to return,
   or a null pointer for `return;' with no value.

   C++: upon seeing a `return', we must call destructors on all
   variables in scope which had constructors called on them.

   The RETURN statement in C++ has initialization semantics.  */

void
c_expand_return (retval)
     tree retval;
{
  tree valtype = TREE_TYPE (TREE_TYPE (current_function_decl));
  tree save_from_destructor = NULL_TREE;

  if (!retval)
    {
      current_function_returns_null = 1;
      if (warn_return_type && valtype != 0 && TREE_CODE (valtype) != VOID_TYPE)
	warning ("`return' with no value, in function returning non-void");
      cleanup_all_aggregates (0, 0);
      expand_null_return ();
    }
  else if (valtype == 0 || TREE_CODE (valtype) == VOID_TYPE)
    {
      current_function_returns_null = 1;
      if (pedantic || TREE_CODE (TREE_TYPE (retval)) != VOID_TYPE)
	warning ("`return' with a value, in function returning void");
      cleanup_all_aggregates (0, 0);
      expand_null_return ();
    }
  else
    {
      register tree t;

      if (TREE_NEEDS_CONSTRUCTOR (valtype))
	{
	  do_aggr_init (DECL_RESULT (current_function_decl), retval);
	  save_from_destructor = DECL_INITIAL (DECL_RESULT (current_function_decl));
	  DECL_INITIAL (DECL_RESULT (current_function_decl)) = NULL_TREE;
	  retval = 0;
	}
      else
	{
	  /* Watch out for constructors, which "return" aggregates
	     via initialization, but which otherwise "return" a pointer.  */
	  if (current_function_name == current_class_name)
	    {
	      if (retval != current_class_decl)
		{
		  error ("return from a constructor: use `this = ...' instead");
		  retval = current_class_decl;
		}
	    }
	  else
	    {
	      retval = convert_for_initialization (DECL_RESULT (current_function_decl),
						   valtype, retval, "return");
	      save_from_destructor = DECL_INITIAL (DECL_RESULT (current_function_decl));
	      DECL_INITIAL (DECL_RESULT (current_function_decl)) = NULL_TREE;
	    }

	  /* Add some useful error checking for C++.  */
	  if (TREE_CODE (valtype) == REFERENCE_TYPE)
	    {
	      if (TREE_CODE (retval) == VAR_DECL
		  && TEMP_NAME_P (DECL_NAME (retval)))
		warning ("reference to non-lvalue returned");
	      else if (TREE_CODE (retval) == ADDR_EXPR
		       && TREE_CODE (TREE_OPERAND (retval, 0)) == VAR_DECL
		       && ! TREE_STATIC (TREE_OPERAND (retval, 0))
		       && IDENTIFIER_LOCAL_VALUE (DECL_NAME (TREE_OPERAND (retval, 0))))
		if (TEMP_NAME_P (DECL_NAME (TREE_OPERAND (retval, 0))))
		  warning ("reference to non-lvalue returned");
		else
		  warning ("reference to local variable `%s' returned",
			   IDENTIFIER_POINTER (DECL_NAME (TREE_OPERAND (retval, 0))));
	    }
	}

      /* Now deal with possible C++ hair:
	 (1) Compute the return value.
	 (2) If there are aggregate values with destructors which
	     must be cleaned up, clean them (taking care
	     not to clobber the return value).
	 (3) If an X(X&) constructor is defined, the return
	     value must be returned via that.  */

      if (any_aggregates_need_cleaning_p (0, save_from_destructor))
	{
	  /* Keep anybody from thinking that our return value
	     is up for grabs (i.e., later inline function expansion).  */
	  emit_use_insn (DECL_RESULT (current_function_decl));
	  if (retval)
	    {
	      if (TREE_GETS_INIT_REF (valtype))
		t = get_temp_name (valtype, error_mark_node, retval);
	      else if (TYPE_MODE (valtype) != BLKmode)
		t = get_temp_regvar (valtype, retval);
	      else
		t = retval;
	    }
	  else
	    t = retval;

	  cleanup_all_aggregates (save_from_destructor);
	  emit_queue ();
	}
      else
	t = retval;

      if (t)
	expand_return (build (INIT_EXPR, void_type_node, DECL_RESULT (current_function_decl), t));
      else
	expand_return (DECL_RESULT (current_function_decl));

      current_function_returns_value = 1;
    }
}

/* Start a C switch statement, testing expression EXP.
   Return EXP if it is valid, an error node otherwise.  */

tree
c_expand_start_case (exp)
     tree exp;
{
  register enum tree_code code = TREE_CODE (TREE_TYPE (exp));
  tree type = TREE_TYPE (exp);

  if (code != INTEGER_TYPE && code != ENUMERAL_TYPE && code != ERROR_MARK)
    {
      error ("switch quantity not an integer");
      exp = error_mark_node;
    }
  else
    {
      tree index;

      exp = default_conversion (exp);
      type = TREE_TYPE (exp);
      index = get_unwidened (exp, 0);
      /* We can't strip a conversion from a signed type to an unsigned,
	 because if we did, int_fits_type_p would do the wrong thing
	 when checking case values for being in range,
	 and it's too hard to do the right thing.  */
      if (TREE_UNSIGNED (TREE_TYPE (exp))
	  == TREE_UNSIGNED (TREE_TYPE (index)))
	exp = index;
    }

  expand_start_case (1, exp, type);

  return exp;
}

/* Return the character string for the name that encodes the
   enumeral value VALUE in the domain TYPE.  */
char *
enum_name_string (value, type)
     tree value;
     tree type;
{
  static char buf[OVERLOAD_MAX_LEN];
  register tree values = TYPE_VALUES (type);
  register int intval = TREE_INT_CST_LOW (value);

  if (TREE_CODE (type) != ENUMERAL_TYPE)
    abort ();
  while (values
	 && TREE_INT_CST_LOW (TREE_VALUE (values)) != intval)
    values = TREE_CHAIN (values);
  if (values == NULL_TREE)
    {
      /* Value must have been cast.  */
      sprintf (buf, "(enum %s)%d",
	       TYPE_NAME_STRING (type), intval);
      return buf;
    }
  return IDENTIFIER_POINTER (TREE_PURPOSE (values));
}

/* Print out a language-specific error message for
   (Pascal) case or (C) switch statements.
   CODE tells what sort of message to print. 
   TYPE is the type of the switch index expression.
   NEW is the new value that we were trying to add.
   OLD is the old value that stopped us from adding it.  */
void
report_case_error (code, type, new, old)
     int code;
     tree type;
     tree new, old;
{
  char buf[OVERLOAD_MAX_LEN];

  if (code == 1)
    {
      if (new)
	error ("case label not within a switch statement");
      else
	error ("default label not within a switch statement");
    }
  else if (code == 2)
    {
      if (new == 0)
	{
	  error ("multiple default labels in one switch");
	  return;
	}
      if (TREE_CODE (new) == RANGE_EXPR)
	if (TREE_CODE (old) == RANGE_EXPR)
	  {
	    if (TREE_CODE (type) == ENUMERAL_TYPE)
	      sprintf (buf, "overlapping ranges [%s..%s], [%s..%s] in case expression",
		       enum_name_string (TREE_OPERAND (new, 0), type),
		       enum_name_string (TREE_OPERAND (new, 1), type),
		       enum_name_string (TREE_OPERAND (old, 0), type),
		       enum_name_string (TREE_OPERAND (old, 1), type));
	    else
	      sprintf (buf, "overlapping ranges [%d..%d], [%d..%d] in case expression",
		       TREE_INT_CST_LOW (TREE_OPERAND (new, 0)),
		       TREE_INT_CST_LOW (TREE_OPERAND (new, 1)),
		       TREE_INT_CST_LOW (TREE_OPERAND (old, 0)),
		       TREE_INT_CST_LOW (TREE_OPERAND (old, 1)));
	    error (buf);
	  }
	else
	  {
	    if (TREE_CODE (type) == ENUMERAL_TYPE)
	      sprintf (buf, "range [%s..%s] includes element `%s' in case expression",
		       enum_name_string (TREE_OPERAND (new, 0), type),
		       enum_name_string (TREE_OPERAND (new, 1), type),
		       enum_name_string (old, type));
	    else
	      sprintf (buf, "range [%d..%d] includes (%d) in case expression",
		       TREE_INT_CST_LOW (TREE_OPERAND (new, 0)),
		       TREE_INT_CST_LOW (TREE_OPERAND (new, 1)),
		       TREE_INT_CST_LOW (old));
	    error (buf);
	  }
      else if (TREE_CODE (old) == RANGE_EXPR)
	{
	  if (TREE_CODE (type) == ENUMERAL_TYPE)
	    sprintf (buf, "range [%s..%s] includes element `%s' in case expression",
		     enum_name_string (TREE_OPERAND (old, 0), type),
		     enum_name_string (TREE_OPERAND (old, 1), type),
		     enum_name_string (new, type));
	  else
	    sprintf (buf, "range [%d..%d] includes (%d) in case expression",
		     TREE_INT_CST_LOW (TREE_OPERAND (old, 0)),
		     TREE_INT_CST_LOW (TREE_OPERAND (old, 1)),
		     TREE_INT_CST_LOW (new));
	  error (buf);
	}
      else
	{
	  if (TREE_CODE (type) == ENUMERAL_TYPE)
	    error ("duplicate label `%s' in switch statement",
		   enum_name_string (new, type));
	  else
	    error ("duplicate label (%d) in switch statement",
		   TREE_INT_CST_LOW (new));
	}
    }
  else if (code == 3)
    {
      if (TREE_CODE (type) == ENUMERAL_TYPE)
	warning ("case value out of range for enum %s",
		 TYPE_NAME_STRING (type));
      else
	warning ("case value out of range");
    }
  else if (code == 4)
    {
      if (TREE_CODE (type) == ENUMERAL_TYPE)
	error ("range values `%s' and `%s' reversed",
	       enum_name_string (new, type),
	       enum_name_string (old, type));
      else
	error ("range values reversed");
    }
}
