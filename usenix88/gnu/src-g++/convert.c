/* Language-depednent node constructors for parse phase of GNU compiler.
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


/* This file contains the low level primitives for the construction
   of type converions.  */

#include "config.h"
#include <stdio.h>
#include "tree.h"
#include "c-tree.h"
#include "flags.h"
#include "overload.h"

/* Subroutines of `convert'.  */

/* Change of width--truncation and extension of integers or reals--
   is represented with NOP_EXPR.  Proper functioning of many things
   assumes that no other conversions can be NOP_EXPRs.

   Conversion between integer and pointer is represented with CONVERT_EXPR.
   Converting integer to real uses FLOAT_EXPR
   and real to integer uses FIX_TRUNC_EXPR.  */

static tree
convert_to_pointer (type, expr)
     tree type, expr;
{
  register tree intype = TREE_TYPE (expr);
  register enum tree_code form = TREE_CODE (intype);
  
  if (integer_zerop (expr))
    {
      if (type == TREE_TYPE (null_pointer_node))
	return null_pointer_node;
      expr = build_int_2 (0, 0);
      TREE_TYPE (expr) = type;
      return expr;
    }

  if (form == POINTER_TYPE)
    return build (NOP_EXPR, type, expr);

  if (form == INTEGER_TYPE || form == ENUMERAL_TYPE)
    {
      if (type_precision (intype) == POINTER_SIZE)
	return build (CONVERT_EXPR, type, expr);
      return convert_to_pointer (type,
				 convert (type_for_size (POINTER_SIZE, 0),
					  expr));
    }

  if (form == REFERENCE_TYPE)
    {
      expr = bash_reference_type (expr);
      if (TREE_TYPE (expr) != type)
	return convert (type, expr);
      return expr;
    }

  if (TREE_HAS_TYPE_CONVERSION (intype))
    {
      /* If we cannot convert to the specific pointer type,
	 try to convert to the type `void *'.  */
      tree rval;
      rval = build_type_conversion (type, expr, 1);
      if (rval) return rval;
      rval = build_type_conversion (ptr_type_node, expr, 1);
      if (rval) return rval;
    }

  error ("cannot convert to a pointer type");

  return null_pointer_node;
}

/* We are passing something to a function which requires a reference.
   The type we are interested in is in TYPE. The initial
   value we have to begin with is in ARG.  */
static tree
build_up_reference (type, arg)
     tree type, arg;
{
  tree rval;
  int constant_flag = 0;
  int volatile_flag = TREE_VOLATILE (arg);

  if (TREE_CODE (type) != REFERENCE_TYPE)
    abort ();

  if (TREE_MEMBER_POINTER (TREE_TYPE (arg)))
    {
      sorry ("references to member pointers.");
      return error_mark_node;
    }

  switch (TREE_CODE (arg))
    {

    case INDIRECT_REF:
      /* This is a call to a constructor which did not know what it was
	 initializing until now: it needs to initialize a temporary.  */
      if (TREE_HAS_CONSTRUCTOR (arg))
	{
	  tree tmp = get_temp_name (TREE_TYPE (arg), error_mark_node, NULL_TREE);
	  tree init = TREE_OPERAND (arg, 0);

	  /* This should not have been filled in.  */
	  if (TREE_VALUE (TREE_OPERAND (init, 1)) == NULL_TREE
	      || TREE_CODE (TREE_VALUE (TREE_OPERAND (init, 1))) != NOP_EXPR
	      || TREE_OPERAND (TREE_VALUE (TREE_OPERAND (init, 1)), 0) != integer_zero_node)
	    abort ();
	  TREE_VALUE (TREE_OPERAND (init, 1)) = build (ADDR_EXPR, type, tmp);
	  TREE_HAS_CONSTRUCTOR (arg) = 0;
	  return build (NOP_EXPR, type, init);
	}
      /* Let &* cancel out to simplify resulting code.  */
      return build (NOP_EXPR, type, TREE_OPERAND (arg, 0));

      /* Get this out of a register if we happened to be in one by accident.
	 Also, build up references to non-lvalues it we must.  */
      /* For &x[y], return (&) x+y */
    case ARRAY_REF:
      mark_addressable (TREE_OPERAND (arg, 0));
      rval = build_binary_op (PLUS_EXPR, TREE_OPERAND (arg, 0),
			      TREE_OPERAND (arg, 1));
      TREE_TYPE (rval) = type;
      return rval;

    case SCOPE_REF:
      /* Could be a reference to a static member.  */
      {
	tree field = TREE_OPERAND (arg, 1);
	if (TREE_STATIC (field))
	  {
	    rval = IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (field));
	    return build (ADDR_EXPR, type, rval);
	  }
	else
	  {
	    /* we should have farmed out member pointers above.  */
	    abort ();
	  }
      }
      break;

    case COMPONENT_REF:
      {
	tree field = TREE_OPERAND (arg, 1);
	rval = build_unary_op (ADDR_EXPR, TREE_OPERAND (arg, 0), 0);

	if (TREE_PACKED (field))
	  {
	    error ("attempt to make a reference to bit-field structure member `%s'",
		   IDENTIFIER_POINTER (DECL_NAME (field)));
	    return error_mark_node;
	  }

	if (DECL_OFFSET (field) != 0)
	  {
	    rval = build (PLUS_EXPR, type, rval,
			  build_int_2 ((DECL_OFFSET (field)
					/ BITS_PER_UNIT),
				       0));
	    rval = fold (rval);
	  }
	else
	  rval = build (NOP_EXPR, type, rval);
	TREE_LITERAL (rval) = staticp (arg);
	TREE_VOLATILE (rval) = volatile_flag;
	return rval;
      }

      /* Anything not already handled and not a true memory reference
	 needs to have a reference built up.  Do so silently for
	 things like integers and return values from function,
	 but complain if we need a reference to something declared
	 as `register'.  */

    case PARM_DECL:
      if (arg == current_class_decl)
	{
	  error ("address of `this' not available");
	  TREE_ADDRESSABLE (arg) = 1; /* so compiler doesn't die later */
	  put_var_into_stack (arg);
	  break;
	}
    case VAR_DECL:
    case CONST_DECL:
    case RESULT_DECL:
      if (TREE_REGDECL (arg) && !TREE_ADDRESSABLE (arg))
	warning ("address needed to build reference for `%s', which is declared `register'",
		 IDENTIFIER_POINTER (DECL_NAME (arg)));
      else if (staticp (arg))
	constant_flag = 1;

      TREE_ADDRESSABLE (arg) = 1;
      put_var_into_stack (arg);
      break;

    case COMPOUND_EXPR:
      {
	tree real_reference = build_up_reference (type, TREE_OPERAND (arg, 1));
	return build (COMPOUND_EXPR, type, TREE_OPERAND (arg, 0), real_reference);
      }

    case COND_EXPR:
      return build (COND_EXPR, type,
		    TREE_OPERAND (arg, 0),
		    build_up_reference (type, TREE_OPERAND (arg, 1)),
		    build_up_reference (type, TREE_OPERAND (arg, 2)));

    default:
      break;
    }

  if (TREE_ADDRESSABLE (arg) == 0)
    {
      tree tmp;

      tmp = get_temp_name (TREE_TYPE (arg), error_mark_node, arg);
      arg = build (ADDR_EXPR, type, tmp);
    }
  else
    {
      arg = build (ADDR_EXPR, type, arg);
    }
  return arg;
}

/* For C++: Only need to do one-level references, but cannot
   get tripped up on signed/unsigned differences.  */
tree
convert_to_reference (decl, reftype, expr, strict)
     tree decl;
     tree reftype, expr;
     int strict;
{
  register tree type = TYPE_MAIN_VARIANT (TREE_TYPE (reftype));
  register tree intype = TREE_TYPE (expr);
  register enum tree_code form = TREE_CODE (intype);

  if (TREE_CODE (reftype) != REFERENCE_TYPE)
    abort ();

  if (form == REFERENCE_TYPE)
    intype = TREE_TYPE (intype);
  intype = TYPE_MAIN_VARIANT (intype);

  /* @@ Probably need to have a check for X(X&) here.  */

  if (TREE_HAS_TYPE_CONVERSION (intype))
    {
      tree rval = build_type_conversion (reftype, expr, 1);
      if (rval)
	return rval;
    }

  /* @@ Perhaps this should try to go through a constructor first
     @@ for proper initialization, but I am not sure when that
     @@ is needed or desirable.  */

  if (int_size_in_bytes (type) <= int_size_in_bytes (intype)
      || (strict < 0 && comptypes (type, intype, strict)))
    {
      if (form == REFERENCE_TYPE)
	{
	  if (decl)
	    {
	      expand_expr (build (INIT_EXPR, void_type_node, decl, expr), 0, VOIDmode, 0);
	      return build (NOP_EXPR, reftype, decl);
	    }
	  expr = copy_node (expr);
	  TREE_TYPE (expr) = reftype;
	  return expr;
	}
      else
	{
	  return build_up_reference (reftype, expr);
	}
    }

  /* Definitely need to go through a constructor here.  */
  if (TREE_HAS_CONSTRUCTOR (type))
    {
      tree rval = get_temp_name (type, NULL_TREE, expr);
      return build_up_reference (reftype, rval);
    }

  error ("cannot convert to a reference type");

  return error_mark_node;
}

/* We are using a reference VAL for its value. Bash that reference all the
   way down to its lowest form. */
tree
bash_reference_type (val)
     tree val;
{
  /* REFERENCE_EXPR's do not get bashed.

     Note that the type of a REFERENCE_EXPR is the same sort of
     type of the object underneath.  We use its type, however, since
     it may not agree perfectly with the type underneath.  This
     happens when we do a cast through a REFERENCE_EXPR.  */
  if (TREE_CODE (val) == REFERENCE_EXPR)
    {
      val = build (NOP_EXPR, build_pointer_type (TREE_TYPE (TREE_TYPE (val))), val);
    }
  else if (TREE_CODE (TREE_TYPE (val)) == REFERENCE_TYPE)
    {
      tree dt = TREE_TYPE (TREE_TYPE (val));

      /* This can happen if we cast to a reference type.  */
      if (TREE_CODE (val) == ADDR_EXPR)
	{
	  val = TREE_OPERAND (val, 0);
	  if (dt != TREE_TYPE (val))
	    {
	      val = copy_node (val);
	      TREE_TYPE (val) = dt;
	    }
	  return val;
	}

      if (! flag_const_is_variable
	  && TREE_READONLY (TREE_TYPE (val)))
	{
	  val = build_indirect_ref (val, "reference bashing (compiler error)");
	  if (TREE_CODE (val) == VAR_DECL
	      && (TREE_READONLY (val) || (TREE_READONLY (TREE_TYPE (val))
					  && fatal ("constant type but non-constant reference decl")))
	      && DECL_INITIAL (val))
	    return DECL_INITIAL (val);
	}

      val = build (INDIRECT_REF, TYPE_MAIN_VARIANT (dt), val);

      TREE_THIS_VOLATILE (val) = TREE_VOLATILE (dt);
      TREE_READONLY (val) = TREE_READONLY (dt);
    }
  return val;
}

/* The result of this is always supposed to be a newly created tree node
   not in use in any existing structure.  */

static tree
convert_to_integer (type, expr)
     tree type, expr;
{
  register tree intype = TREE_TYPE (expr);
  register enum tree_code form = TREE_CODE (intype);

  if (form == REFERENCE_TYPE)
    {
      expr = bash_reference_type (expr);
      intype = TREE_TYPE (expr);
      form = TREE_CODE (intype);
    }

  if (form == POINTER_TYPE)
    {
      if (integer_zerop (expr))
	expr = integer_zero_node;
      else
	expr = fold (build (CONVERT_EXPR,
			    type_for_size (POINTER_SIZE, 0), expr));
      intype = TREE_TYPE (expr);
      form = TREE_CODE (intype);
      if (intype == type)
	return expr;
    }

  if (form == INTEGER_TYPE || form == ENUMERAL_TYPE)
    {
      register int outprec = TYPE_PRECISION (type);
      register int inprec = TYPE_PRECISION (intype);
      register enum tree_code ex_form = TREE_CODE (expr);

      if (outprec >= inprec)
	return build (NOP_EXPR, type, expr);

/* Here detect when we can distribute the truncation down past some arithmetic.
   For example, if adding two longs and converting to an int,
   we can equally well convert both to ints and then add.
   For the operations handled here, such truncation distribution
   is always safe.
   It is desirable in these cases:
   1) when truncating down to full-word from a larger size
   2) when truncating takes no work.
   3) when at least one operand of the arithmetic has been extended
   (as by C's default conversions).  In this case we need two conversions
   if we do the arithmetic as already requested, so we might as well
   truncate both and then combine.  Perhaps that way we need only one.

   Note that in general we cannot do the arithmetic in a type
   shorter than the desired result of conversion, even if the operands
   are both extended from a shorter type, because they might overflow
   if combined in that type.  The exceptions to this--the times when
   two narrow values can be combined in their narrow type even to
   make a wider result--are handled by "shorten" in build_binary_op.  */

      switch (ex_form)
	{
	case RSHIFT_EXPR:
	  /* We can pass truncation down through right shifting
	     when the shift count is a negative constant.  */
	  if (TREE_CODE (TREE_OPERAND (expr, 1)) != INTEGER_CST
	      || TREE_INT_CST_LOW (TREE_OPERAND (expr, 1)) > 0)
	    break;
	  goto trunc1;

	case LSHIFT_EXPR:
	  /* We can pass truncation down through left shifting
	     when the shift count is a positive constant.  */
	  if (TREE_CODE (TREE_OPERAND (expr, 1)) != INTEGER_CST
	      || TREE_INT_CST_LOW (TREE_OPERAND (expr, 1)) < 0)
	    break;
	  /* In this case, shifting is like multiplication.  */

	case PLUS_EXPR:
	case MINUS_EXPR:
	case MULT_EXPR:
	case MAX_EXPR:
	case MIN_EXPR:
	case BIT_AND_EXPR:
	case BIT_IOR_EXPR:
	case BIT_XOR_EXPR:
	case BIT_ANDTC_EXPR:
	trunc1:
	  {
	    tree arg0 = get_unwidened (TREE_OPERAND (expr, 0), type);
	    tree arg1 = get_unwidened (TREE_OPERAND (expr, 1), type);

	    if (outprec >= BITS_PER_WORD
		|| TRULY_NOOP_TRUNCATION (outprec, inprec)
		|| inprec > TYPE_PRECISION (TREE_TYPE (arg0))
		|| inprec > TYPE_PRECISION (TREE_TYPE (arg1)))
	      {
		/* Do the arithmetic in type TYPEX,
		   then convert result to TYPE.  */
		register tree typex = type;

		/* Can't do arithmetic in enumeral types
		   so use an integer type that will hold the values.  */
		if (TREE_CODE (typex) == ENUMERAL_TYPE)
		  typex = type_for_size (TYPE_PRECISION (typex),
					 TREE_UNSIGNED (typex));

		/* But now perhaps TYPEX is as wide as INPREC.
		   In that case, do nothing special here.
		   (Otherwise would recurse infinitely in convert.  */
		if (TYPE_PRECISION (typex) != inprec)
		  {
		    /* Don't do unsigned arithmetic where signed was wanted,
		       or vice versa.  */
		    typex = (TREE_UNSIGNED (TREE_TYPE (expr))
			     ? unsigned_type (typex) : signed_type (typex));
		    return convert (type,
				    build_binary_op_nodefault (ex_form,
							       convert (typex, arg0),
							       convert (typex, arg1)));
		  }
	      }
	  }
	  break;

	case EQ_EXPR:
	case NE_EXPR:
	case GT_EXPR:
	case GE_EXPR:
	case LT_EXPR:
	case LE_EXPR:
	case TRUTH_AND_EXPR:
	case TRUTH_OR_EXPR:
	case TRUTH_NOT_EXPR:
	  /* If we want result of comparison converted to a byte,
	     we can just regard it as a byte, since it is 0 or 1.  */
	  TREE_TYPE (expr) = type;
	  return expr;

	case NEGATE_EXPR:
	case BIT_NOT_EXPR:
	case ABS_EXPR:
	  {
	    register tree typex = type;

	    /* Can't do arithmetic in enumeral types
	       so use an integer type that will hold the values.  */
	    if (TREE_CODE (typex) == ENUMERAL_TYPE)
	      typex = type_for_size (TYPE_PRECISION (typex),
				     TREE_UNSIGNED (typex));

	    /* But now perhaps TYPEX is as wide as INPREC.
	       In that case, do nothing special here.
	       (Otherwise would recurse infinitely in convert.  */
	    if (TYPE_PRECISION (typex) != inprec)
	      {
		/* Don't do unsigned arithmetic where signed was wanted,
		   or vice versa.  */
		typex = (TREE_UNSIGNED (TREE_TYPE (expr))
			 ? unsigned_type (typex) : signed_type (typex));
		return convert (type,
				build_unary_op (ex_form,
						convert (typex, TREE_OPERAND (expr, 0)),
						1));
	      }
	  }

	case NOP_EXPR:
	  /* If truncating after truncating, might as well do all at once.
	     If truncating after extending, we may get rid of wasted work.  */
	  return convert (type, get_unwidened (TREE_OPERAND (expr, 0), type));
	}

      return build (NOP_EXPR, type, expr);
    }

  if (form == REAL_TYPE)
    return build (FIX_TRUNC_EXPR, type, expr);

  if (TREE_HAS_TYPE_CONVERSION (intype))
    {
      tree rval;
      rval = build_type_conversion (type, expr, 1);
      if (rval) return rval;
      if (TYPE_PRECISION (type) < TYPE_PRECISION (integer_type_node))
	{
	  rval = build_type_conversion (integer_type_node, expr, 1);
	  if (rval) return rval;
	}
    }

  error ("aggregate value used where an integer was expected");

  {
    register tree tem = build_int_2 (0, 0);
    TREE_TYPE (tem) = type;
    return tem;
  }
}

static tree
convert_to_real (type, expr)
     tree type, expr;
{
  register enum tree_code form = TREE_CODE (TREE_TYPE (expr));
  extern int flag_float_store;

  if (form == REFERENCE_TYPE)
    {
      expr = bash_reference_type (expr);
      form = TREE_CODE (TREE_TYPE (expr));
    }

  if (form == REAL_TYPE)
    return build (flag_float_store ? CONVERT_EXPR : NOP_EXPR,
		  type, expr);

  if (form == INTEGER_TYPE || form == ENUMERAL_TYPE)
    return build (FLOAT_EXPR, type, expr);

  if (form == POINTER_TYPE)
    error ("pointer value used where a float was expected");
  else
    {
      /* C++: check to see if we can convert this aggregate type
	 into the required scalar type.  */

      if (TREE_HAS_TYPE_CONVERSION (TREE_TYPE (expr)))
	{
	  tree rval;
	  rval = build_type_conversion (type, expr, 1);
	  if (rval) return rval;
	  if (TYPE_PRECISION (type) < TYPE_PRECISION (double_type_node))
	    {
	      rval = build_type_conversion (double_type_node, expr, 1);
	      if (rval) return rval;
	    }
	}

      else
	error ("aggregate value used where a float was expected");
    }

  {
    register tree tem = make_node (REAL_CST);
    TREE_TYPE (tem) = type;
    TREE_REAL_CST (tem) = 0;
    return tem;
  }
}

/* See if there is a constructor of type TYPE which will convert
   EXPR.  The reference manual seems to suggest (8.5.6) that we need
   not worry about finding constructors for base classes, then converting
   to the derived class.

   MSGP is a pointer to a message that would be an appropriate error
   string.  If MSGP is NULL, then we are not interested in reporting
   errors.  */
tree
convert_to_aggr (type, expr, msgp, protect)
     tree type, expr;
     char **msgp;
{
  tree basetype = TYPE_MAIN_VARIANT (type);
  tree name = DECL_NAME (TYPE_NAME (basetype));
  tree field;
  tree function, fntype, parmtypes, parmlist, result;
  tree method_name;
  enum visibility_type visibility;
  int can_be_private, can_be_protected;

  if (! TREE_HAS_CONSTRUCTOR (basetype))
    {
      if (msgp)
	*msgp = "type `%s' does not have a constructor";
      return error_mark_node;
    }

  /* The type of the first argument will be filled in inside the loop.  */
  parmlist = tree_cons (NULL_TREE, integer_zero_node,
			build_tree_list (NULL_TREE, expr));

  visibility = visibility_public;
  can_be_private = 0;
  can_be_protected = IDENTIFIER_CLASS_VALUE (name) || name == current_class_name;

  parmtypes = tree_cons (NULL_TREE, TYPE_POINTER_TO (basetype),
			 build_tree_list (NULL_TREE, TREE_TYPE (expr)));
  method_name = do_decl_overload (IDENTIFIER_POINTER (name), parmtypes);

  /* constructors are up front.  */
  field = TREE_VALUE (TYPE_FN_FIELDS (basetype));
  if (TREE_HAS_DESTRUCTOR (basetype))
    field = TREE_CHAIN (field);

  while (field)
    {
      if (DECL_NAME (TREE_TYPE (field)) == method_name)
	{
	  function = TREE_TYPE (field);
	  if (protect)
	    {
	      if (TREE_PRIVATE (field))
		{
		  can_be_private =
		    (basetype == current_class_type
		     || is_friend (current_function_decl, basetype)
		     || purpose_member (basetype, DECL_VISIBILITY (field)));
		  if (! can_be_private)
		    goto found;
		}
	      else if (TREE_PROTECTED (field))
		{
		  if (! can_be_protected)
		    goto found;
		}
	    }
	  goto found_and_ok;
	}
      field = TREE_CHAIN (field);
    }

  /* No exact conversion was found.  See if an approximate
     one will do.  */
  field = TREE_VALUE (TYPE_FN_FIELDS (basetype));
  if (TREE_HAS_DESTRUCTOR (basetype))
    field = TREE_CHAIN (field);

  {
    int saw_private = 0;
    int saw_protected = 0;
    struct candidate *candidates =
      (struct candidate *) alloca (list_length (field) * sizeof (struct candidate));
    struct candidate *cp = candidates;

    while (field)
      {
	function = TREE_TYPE (field);
	compute_conversion_costs (function, parmlist, cp, 2);
	if (cp->evil == 0)
	  {
	    cp->u.field = field;
	    if (protect)
	      {
		if (TREE_PRIVATE (field))
		  visibility = visibility_private;
		else if (TREE_PROTECTED (field))
		  visibility = visibility_protected;
		else
		  visibility = visibility_public;
	      }
	    else
	      visibility = visibility_public;

	    if (visibility == visibility_private
		? (basetype == current_class_type
		   || is_friend (cp->function, basetype)
		   || purpose_member (basetype, DECL_VISIBILITY (field)))
		: visibility == visibility_protected
		? (can_be_protected
		   || purpose_member (basetype, DECL_VISIBILITY (field)))
		: 1)
	      {
		if (cp->user == 0 && cp->b_or_d == 0
		    && cp->easy <= 1)
		  {
		    goto found_and_ok;
		  }
		cp++;
	      }
	    else
	      {
		if (visibility == visibility_private)
		  saw_private = 1;
		else
		  saw_protected = 1;
	      }
	  }
	field = TREE_CHAIN (field);
      }
    if (cp - candidates)
      {
	/* Rank from worst to best.  Then cp will point to best one.
	   Private fields have their bits flipped.  For unsigned
	   numbers, this should make them look very large.
	   If the best alternate has a (signed) negative value,
	   then all we ever saw were private members.  */
	if (cp - candidates > 1)
	  qsort (candidates,	/* char *base */
		 cp - candidates, /* int nel */
		 sizeof (struct candidate), /* int width */
		 rank_for_overload); /* int (*compar)() */

	--cp;
	if (cp->evil > 1)
	  {
	    if (msgp)
	      *msgp = "ambiguous type conversion possible for `%s'";
	    return error_mark_node;
	  }

	function = cp->function;
	field = cp->u.field;
	goto found_and_ok;
      }
    else if (msgp)
      {
	if (saw_private)
	  if (saw_protected)
	    *msgp = "only private and protected conversions apply";
	  else
	    *msgp = "only private conversions apply";
	else if (saw_protected)
	  *msgp = "only protected conversions apply";
      }
    return error_mark_node;
  }
  /* NOTREACHED */

 not_found:
  if (msgp) *msgp = "no appropriate conversion to type `%s'";
  return error_mark_node;
 found:
  if (visibility == visibility_private)
    if (! can_be_private)
      {
	if (msgp)
	  *msgp = TREE_PRIVATE (field)
	    ? "conversion to type `%s' is private"
	    : "conversion to type `%s' is from private base class";
	return error_mark_node;
      }
  if (visibility == visibility_protected)
    if (! can_be_protected)
      {
	if (msgp)
	  *msgp = TREE_PRIVATE (field)
	    ? "conversion to type `%s' is protected"
	    : "conversion to type `%s' is from protected base class";
	return error_mark_node;
      }
  function = TREE_TYPE (field);
 found_and_ok:

  /* It will convert, but we don't do anything about it yet.  */
  if (msgp == 0)
    return NULL_TREE;

  fntype = TREE_TYPE (function);
  if (TREE_INLINE (function) && TREE_CODE (function) == FUNCTION_DECL)
    function = build (ADDR_EXPR, build_pointer_type (fntype), function);
  else
    function = default_conversion (function);

  result = build_nt (CALL_EXPR, function,
		     actualparameterlist (TYPE_ARG_TYPES (fntype), parmlist, NULL_TREE),
		     NULL_TREE);
  TREE_TYPE (result) = TREE_TYPE (fntype);
  return result;
}

/* Create an expression whose value is that of EXPR,
   converted to type TYPE.  The TREE_TYPE of the value
   is always TYPE.  This function implements all reasonable
   conversions; callers should filter out those that are
   not permitted by the language being compiled.  */

tree
convert (type, expr)
     tree type, expr;
{
  register tree e = expr;
  register enum tree_code code = TREE_CODE (type);

  if (type == TREE_TYPE (expr) || TREE_CODE (expr) == ERROR_MARK)
    return expr;
  if (TREE_CODE (TREE_TYPE (expr)) == ERROR_MARK)
    return error_mark_node;
  if (TREE_CODE (TREE_TYPE (expr)) == VOID_TYPE)
    {
      error ("void value not ignored as it ought to be");
      return error_mark_node;
    }
  if (code == VOID_TYPE)
    return build (CONVERT_EXPR, type, e);
#if 0
  /* This is incorrect.  A truncation can't be stripped this way.
     Extensions will be stripped by the use of get_unwidened.  */
  if (TREE_CODE (expr) == NOP_EXPR)
    return convert (type, TREE_OPERAND (expr, 0));
#endif
  if (code == INTEGER_TYPE || code == ENUMERAL_TYPE)
    return fold (convert_to_integer (type, e));
  if (code == POINTER_TYPE)
    return fold (convert_to_pointer (type, e));
  if (code == REAL_TYPE)
    return fold (convert_to_real (type, e));

  /* C++ */
  if (code == REFERENCE_TYPE)
    return fold (convert_to_reference (0, type, e, -1));

  /* New C++ semantics:  since assignment is now based on
     memberwise copying,  if the rhs type is derived from the
     lhs type, then we may still do a conversion.  */
  if (IS_AGGR_TYPE_CODE (code))
    {
      tree dtype = TREE_TYPE (expr);

      if (TREE_CODE (dtype) == REFERENCE_TYPE)
	{
	  expr = bash_reference_type (expr);
	  dtype = TREE_TYPE (expr);
	}
      dtype = TYPE_MAIN_VARIANT (dtype);

      /* Conversion between aggregate types.  New C++ semantics allow
	 objects of derived type to be cast to objects of base type.
	 Old semantics only allowed this bwteen pointers.

	 There may e some ambiguity between using a constructor
	 vs. using a type conversion operator when both apply.  */

      if (IS_AGGR_TYPE (dtype))
	{
	  tree conversion = TREE_HAS_TYPE_CONVERSION (dtype)
	    ? build_type_conversion (type, expr, 1) : NULL_TREE;

	  if (TREE_HAS_CONSTRUCTOR (type))
	    {
	      tree rval = build_classfn_ref (NULL_TREE, DECL_NAME (TYPE_NAME (type)), build_tree_list (NULL_TREE, expr), 0, 0);
	      if (rval != error_mark_node)
		{
		  tree tmp;
		  if (conversion)
		    {
		      error ("both constructor and type conversion operator apply");
		      return error_mark_node;
		    }
		  /* call to constructor successful.  */
		  tmp = get_temp_name (type, error_mark_node, NULL_TREE);
		  TREE_VALUE (TREE_OPERAND (rval, 1)) = tmp;
		  return rval;
		}
	      /* type conversion successful.  */
	      if (conversion)
		return conversion;
	    }

	  /* Type conversion applies.  */
	  if (conversion)
	    return conversion;

	  /* now try normal C++ assignment semantics.  */
	  if (check_base_type (type, dtype))
	    {
	      if (TREE_VIRTUAL (type))
		warning ("assignment to virtual aggregate type");
	      expr = build (NOP_EXPR, type, expr);
	      return expr;
	    }
	  error ("conversion between incompatible aggregate types requested");
	  return error_mark_node;
	}
      /* conversion from non-aggregate to aggregate type requires constructor.  */
      else if (TREE_HAS_CONSTRUCTOR (type))
	return get_temp_name (type, NULL_TREE, expr);
    }

  error ("conversion to non-scalar type requested");
  return error_mark_node;
}

/* Convert an aggregate EXPR to type TYPE.  If a conversion
   exists, return the attempted conversion.  This may
   return ERROR_MARK_NODE if the conversion is not
   allowed (references private members, etc).
   If no conversion exists, NULL_TREE is returned.

   If FOR_REAL is non-zero, really try to make this conversion
   work  This may involve allowing `build_classfn_ref' to irrevocably
   create a temporary variable.

   TYPE may be a reference type, in which case we first look
   for something that will convert to a reference type.  If
   that fails, we will try to look for something of the
   reference's target type, and then return a reference to that.  */
tree
build_type_conversion (type, expr, for_real)
     tree type, expr;
     int for_real;
{
  /* C++: check to see if we can convert this aggregate type
     into the required scalar type.  */
  tree type_default;
  tree typename = do_typename_overload (type);
  tree basetype = TREE_TYPE (expr), save_basetype;
  tree rval;

  if (TREE_CODE (basetype) == REFERENCE_TYPE)
    basetype = TREE_TYPE (basetype);

  basetype = TYPE_MAIN_VARIANT (basetype);
  save_basetype = basetype;

  while (basetype && TREE_HAS_TYPE_CONVERSION (basetype))
    {
      if (get_fnfields (basetype, typename))
	{
	  if (for_real == 0 && ! lvalue_p (expr))
	    expr = build (NOP_EXPR, TYPE_POINTER_TO (basetype), integer_zero_node);
	  rval = build_classfn_ref (expr, typename, NULL_TREE, 1, for_real);
	  if (rval == error_mark_node && for_real == 0)
	    return 0;
	  return rval;
	}
      basetype = TYPE_BASELINK (basetype);
    }

  if (TREE_CODE (type) == REFERENCE_TYPE)
    {
      type = TYPE_MAIN_VARIANT (TREE_TYPE (type));
      typename = do_typename_overload (type);
      basetype = save_basetype;
      /* May need to build a temporary for this.  */
      while (basetype && TREE_HAS_TYPE_CONVERSION (basetype))
	{
	  if (get_fnfields (basetype, typename))
	    {
	      if (for_real == 0 && ! lvalue_p (expr))
		expr = build (NOP_EXPR, TYPE_POINTER_TO (basetype), integer_zero_node);
	      rval = build_classfn_ref (expr, typename, NULL_TREE, 1, for_real);
	      if (rval == error_mark_node && for_real == 0)
		return 0;
	      if (rval == error_mark_node)
		return rval;
	      if (IS_AGGR_TYPE (type))
		{
		  rval = get_temp_name (type, NULL_TREE, rval);
		  return build (ADDR_EXPR, TYPE_REFERENCE_TO (type), rval);
		}
	      return rval;
	    }
	  basetype = TYPE_BASELINK (basetype);
	}
      /* No free conversions for reference types, right?.  */
      return NULL_TREE;
    }

  /* No perfect match found, try default.  */
  if (TREE_CODE (type) == POINTER_TYPE)
    type_default = ptr_type_node;
  else
    {
      extern tree default_conversion ();
      tree tmp = default_conversion (build (NOP_EXPR, type, integer_zero_node));
      if (tmp == error_mark_node)
	return NULL_TREE;
      type_default = TREE_TYPE (tmp);
    }

  if (type_default != type)
    {
      type = type_default;
      typename = do_typename_overload (type);
      basetype = save_basetype;

      while (basetype && TREE_HAS_TYPE_CONVERSION (basetype))
	{
	  if (get_fnfields (basetype, typename))
	    {
	      if (for_real == 0 && ! lvalue_p (expr))
		expr = build (NOP_EXPR, TYPE_POINTER_TO (basetype), integer_zero_node);
	      rval = build_classfn_ref (expr, typename, NULL_TREE, 1, for_real);
	      if (rval == error_mark_node && for_real == 0)
		return 0;
	      return rval;
	    }
	  basetype = TYPE_BASELINK (basetype);
	}
    }

  /* Now, try C promotions...

     void * -> int
     int -> float, void *
     float -> int

     @@ Is this really needed?  */

  basetype = save_basetype;
  if (TREE_CODE (type) == POINTER_TYPE || TREE_CODE (type) == REAL_TYPE)
    type = integer_type_node;
  else if (TREE_CODE (type) == INTEGER_TYPE)
    if (TREE_HAS_REAL_CONVERSION (basetype))
      type = double_type_node;
    else
      type = ptr_type_node;
  else return NULL_TREE;

  typename = do_typename_overload (type);
  while (basetype && TREE_HAS_TYPE_CONVERSION (basetype))
    {
      if (get_fnfields (basetype, typename))
	{
	  if (for_real == 0 && ! lvalue_p (expr))
	    expr = build (NOP_EXPR, TYPE_POINTER_TO (basetype), integer_zero_node);
	  rval = build_classfn_ref (expr, typename, NULL_TREE, 1, for_real);
	  if (rval == error_mark_node && for_real == 0)
	    return 0;
	  return rval;
	}
      basetype = TYPE_BASELINK (basetype);
    }
  return NULL_TREE;
}

/* Must convert two aggregate types to non-aggregate type.
   Attempts to find a non-ambiguous, "best" type conversion.

   Return 1 on success, 0 on failure.

   @@ What are the real semantics of this supposed to be??? */
int
build_default_binary_type_conversion (arg1, arg2)
     tree *arg1, *arg2;
{
  tree type1 = TREE_TYPE (*arg1);
  tree type2 = TREE_TYPE (*arg2);
  char *name1 = TYPE_NAME_STRING (type1);
  char *name2 = TYPE_NAME_STRING (type2);

  if (! TREE_HAS_TYPE_CONVERSION (type1))
    {
      if (! TREE_HAS_TYPE_CONVERSION (type2))
	error ("type conversion required for types `%s' and `%s'",
	       name1, name2);
      else
	error ("type conversion required for type `%s'", name1);
      return 0;
    }
  else if (! TREE_HAS_TYPE_CONVERSION (type2))
    {
      error ("type conversion required for type `%s'", name2);
      return 0;
    }

  if (TREE_HAS_INT_CONVERSION (type1) && TREE_HAS_REAL_CONVERSION (type1))
    {
      warning ("ambiguous type conversion for type `%s', defaulting to int", name1);
    }
  if (TREE_HAS_INT_CONVERSION (type1))
    {
      *arg1 = build_type_conversion (integer_type_node, *arg1, 1);
      *arg2 = build_type_conversion (integer_type_node, *arg2, 1);
    }
  else if (TREE_HAS_REAL_CONVERSION (type1))
    {
      *arg1 = build_type_conversion (double_type_node, *arg1, 1);
      *arg2 = build_type_conversion (double_type_node, *arg2, 1);
    }
  else
    {
      *arg1 = build_type_conversion (ptr_type_node, *arg1, 1);
      *arg2 = build_type_conversion (ptr_type_node, *arg2, 1);
    }
  if (*arg1 == 0)
    {
      if (*arg2 == 0 && type1 != type2)
	error ("default type conversion for types `%s' and `%s' failed",
	       name1, name2);
      else
	error ("default type conversion for type `%s' failed", name1);
      return 0;
    }
  else if (*arg2 == 0)
    {
      error ("default type conversion for type `%s' failed", name2);
      return 0;
    }
  return 1;
}
