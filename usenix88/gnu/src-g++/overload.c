/* Manage function and varaible name overloading.
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


#include "config.h"
#include "tree.h"
#include "c-tree.h"
#include "overload.h"
#include "rtl.h"

#define NULL 0

static char *build_overload_name ();

/* Change the name of a function definition so that it may be
   overloaded. NAME is the name of the function to overload,
   PARMS is the parameter list (which determines what name the
   final function obtains).  */
tree
do_decl_overload (name, parms)
     char *name;
     tree parms;
{
  int tmp;
  char tname[OVERLOAD_MAX_LEN];

  name = (char *)strcpy (tname, name);
  tmp = strlen (name);
  name[tmp++] = '_';
  if (parms && TREE_VALUE (parms) != void_type_node)
    {
      char *name_end;
      name_end = build_overload_name (parms, name+tmp, &tname[OVERLOAD_MAX_LEN]);
      name_end[-1] = '\0';	/* cancel trailing '_' */
    }
  else
    {
      sprintf (name + tmp, "VOID");
    }
  return get_identifier (name);
}

/* Build an overload name for the type expression TYPE.  CTYPE is
   the class we are building this for.  */
tree
do_typename_overload (type)
     tree type;
{
  int tmp;
  char tname[OVERLOAD_MAX_LEN], *name_end;

  sprintf (tname, OPERATOR_TYPENAME_FORMAT);
  tname[sizeof (OPERATOR_TYPENAME_FORMAT) - 1] = '_';
  name_end = build_overload_name (type,
				  tname + sizeof (OPERATOR_TYPENAME_FORMAT),
				  &tname[OVERLOAD_MAX_LEN]);
  name_end[-1] = '\0';
  return get_identifier (tname);
}

/* A function has just been referenced that needs to be overloaded.
   The name of this function comes through NAME.  The name depends
   on PARMS.

   Note that this function must handle simple `C' promotions,
   as well as variable numbers of arguments (...), and
   default arguments to boot.

   If the overloading is successful, we return a treenode which
   contains the call to the function.

   Note that the DECL_RTL of FUNCTION must be made to agree with this
   function's new name.  */

tree
do_actual_overload (fnname, parms, complain, final_cp)
     tree fnname, parms;
     int complain;
     struct candidate *final_cp;
{
  /* must check for overloading here */
  tree decl, functions, function, parm;
  tree parmtypes;
  int tmp;
  char name[OVERLOAD_MAX_LEN];
  int length;
  int parmlength = list_length (parms);

  struct candidate *candidates, *cp;
  int rank_for_overload ();

  if (final_cp)
    {
      final_cp->evil = 0;
      final_cp->user = 0;
      final_cp->b_or_d = 0;
      final_cp->easy = 0;
    }

  strcpy (name, IDENTIFIER_POINTER (fnname));
  tmp = IDENTIFIER_LENGTH (fnname);
  name[tmp++] = '_';
  parmtypes = NULL_TREE;
  for (parm = parms; parm; parm = TREE_CHAIN (parm))
    {
      register tree val = TREE_VALUE (parm);

      if (val == error_mark_node)
	return error_mark_node;
      parmtypes = chainon (parmtypes, build_tree_list (NULL_TREE, TREE_TYPE (val)));
    }
  if (parmtypes && TREE_VALUE (parmtypes) != void_type_node)
    {
      char *name_end;
      name_end = build_overload_name (parmtypes, name+tmp, &name[OVERLOAD_MAX_LEN]);
      name_end[-1] = '\0';	/* cancel trailing '_' */
    }
  else
    {
      sprintf (name + tmp, "VOID");
    }

  /* implicitly_declare() is going to pretend that the "undeclared"
     (really overloaded) function is declared in the local scope.
     We know better. Kill this wrong definition, and let the real
     one come through (if one exists).  */
  if (IDENTIFIER_LOCAL_VALUE (fnname))
    abort ();
  
  decl = (tree) get_identifier (name);

  /* Now check to see whether or not we can win.
     Note that if we are called from `build_classfn_ref',
     then we cannot have a mis-match, because we would have
     already found such a winning case.  */

  if (IDENTIFIER_CLASS_VALUE (decl))
    if (TREE_CODE (IDENTIFIER_CLASS_VALUE (decl)) != TREE_LIST)
      return build_function_call (IDENTIFIER_CLASS_VALUE (decl), parms);
  if (IDENTIFIER_GLOBAL_VALUE (decl))
    if (TREE_CODE (IDENTIFIER_GLOBAL_VALUE (decl)) != TREE_LIST)
      return build_function_call (IDENTIFIER_GLOBAL_VALUE (decl), parms);

  /* @@ Kludge for now.  */
  /* @@ Also need to check visibility.  */
  if (IDENTIFIER_CLASS_VALUE (fnname) && IDENTIFIER_GLOBAL_VALUE (fnname))
    {
      tree class_decl = IDENTIFIER_CLASS_VALUE (fnname);
      tree global_decl = IDENTIFIER_GLOBAL_VALUE (fnname);
      if (TREE_CODE (global_decl) == TREE_LIST && TREE_CHAIN (global_decl) == 0)
	global_decl = TREE_VALUE (global_decl);
      if (class_decl != global_decl)
	{
	  /* Don't really know what to do here.  */
	  warning ("overloads conflict in global and class space");
	}
      /* Otherwise it is safe to use the IDENTIFIER_CLASS_VALUE.  */
      else if (TREE_CODE (class_decl) == FUNCTION_DECL)
	return build_function_call (class_decl, parms);
    }

  if (IDENTIFIER_CLASS_VALUE (fnname))
    functions = IDENTIFIER_CLASS_VALUE (fnname);
  else functions = IDENTIFIER_GLOBAL_VALUE (fnname);

  if (functions == NULL_TREE)
    {
      if (complain)
	error ("only member functions apply");
      if (final_cp)
	final_cp->evil = 1;
      return error_mark_node;
    }

  if (TREE_CODE (functions) == FUNCTION_DECL)
    if (final_cp)
      {
	/* We are just curious whether this is a viable alternative or not.  */
	compute_conversion_costs (functions, parms, final_cp, parmlength);
	return functions;
      }
    else
      return build_function_call (functions, parms);

  if (TREE_CODE (TREE_VALUE (functions)) == TREE_LIST)
    {
      register tree inner;
      length = 0;
      for (inner = functions; inner; inner = TREE_CHAIN (inner))
	length += list_length (TREE_VALUE (TREE_VALUE (inner)));
    }
  else length = list_length (functions);

  candidates = (struct candidate *)alloca (length * sizeof (struct candidate));

  cp = candidates;

  if (TREE_CODE (TREE_VALUE (functions)) == TREE_LIST)
    {
      /* OUTER is a list of method lists.  The PURPOSE and VALUE fields
	 are the owning class and the method list, respectively.  */
      register tree outer = functions;

      while (outer)
	{
	  /* INNER is the chain of FIELD_DECLS.  */
	  register tree inner = TREE_VALUE (TREE_VALUE (outer));
	  while (inner)
	    {
	      function = TREE_TYPE (inner);
	      compute_conversion_costs (function, parms, cp, parmlength);
	      if (cp->evil == 0)
		{
		  if (cp->user == 0 && cp->b_or_d == 0
		      && cp->easy <= 1)
		    if (final_cp)
		      {
			final_cp->easy = cp->easy;
			return function;
		      }
		    else return build_function_call (function, parms);
		  cp++;
		}
	      inner = TREE_CHAIN (inner);
	    }
	  outer = TREE_CHAIN (outer);
	}
    }
  else
    {
      /* OUTER is the list of FUNCTION_DECLS, in a TREE_LIST.  */
      register tree outer = functions;
      while (outer)
	{
	  function = TREE_VALUE (outer);
	  compute_conversion_costs (function, parms, cp, parmlength);
	  if (cp->evil == 0)
	    {
	      if (cp->user == 0 && cp->b_or_d == 0
		  && cp->easy <= 1)
		if (final_cp)
		  {
		    final_cp->easy = cp->easy;
		    return function;
		  }
		else return build_function_call (function, parms);
	      cp++;
	    }
	  outer = TREE_CHAIN (outer);
	}
    }

  if (cp - candidates)
    {
      tree rval = error_mark_node;

      /* Rank from worst to best.  Then cp will point to best one.
	 Private fields have their bits flipped.  For unsigned
	 numbers, this should make them look very large.
	 If the best alternate has a (signed) negative value,
	 then all we ever saw were private members.  */
      if (cp - candidates > 1)
	{
	  qsort (candidates,	/* char *base */
		 cp - candidates, /* int nel */
		 sizeof (struct candidate), /* int width */
		 rank_for_overload); /* int (*compar)() */

	  if (cp[-1].user && cp[-1].b_or_d)
	    {
	      /* If the best one requires a type conversion, and
		 we do not store worse ones, and there is more than
		 one, then there are mutiple assignments possible
		 for this function.  Error.  */
	      error ("type conversion ambiguous");
	    }
	  else
	    rval = cp[-1].function;
	}
      else if (cp[-1].evil > 1)
	error ("type conversion ambiguous");
      else
	rval = cp[-1].function;

      if (final_cp)
	{
	  final_cp->evil = cp[-1].evil;
	  final_cp->user = cp[-1].user;
	  final_cp->b_or_d = cp[-1].b_or_d;
	  final_cp->easy = cp[-1].easy;
	  return rval;
	}

      return build_function_call (rval, parms);
    }
  else if (complain)
    {
      tree name;
      char *err_name;
      /* Initialize name for error reporting.  */
      if (TREE_CODE (functions) == TREE_LIST)
	name = TREE_PURPOSE (functions);
      else
	name = DECL_ORIGINAL_NAME (functions);

      if (OPERATOR_NAME_P (name))
	{
	  err_name = (char *)alloca (OVERLOAD_MAX_LEN);
	  sprintf (err_name, "operator %s",
		   TREE_CODE (functions) == TREE_LIST
		   ? OPERATOR_NAME_STRING (TREE_VALUE (functions))
		   : OPERATOR_NAME_STRING (functions));
	}
      else
	err_name = IDENTIFIER_POINTER (name);

      report_type_mismatch (cp, parms, "function", err_name);
    }
  if (final_cp) final_cp->evil = 1;
  return error_mark_node;
}

/* BASETYPE is a class type. Return the list of fields with name
   NAME, or NULL_TREE is no such field exists.  */
tree
get_field (basetype, name)
     tree basetype, name;
{
  register tree baselink;
  register enum tree_code form;
  register tree field;

  /* First, see if there is a field or component with name NAME. */
  form = TREE_CODE (basetype);
  if (IS_AGGR_TYPE_CODE (form))
    {
      while (basetype)
	{
	  for (field = TYPE_FIELDS (basetype);
	       field; field = TREE_CHAIN (field))
	    {
	      if (DECL_NAME (field) == name)
		return field;
	    }
	  basetype = TYPE_BASELINK (basetype);
	}
      return NULL_TREE;
    }
  abort();
}

/* BASETYPE is a class type. Return the list of fields with name
   NAME, or NULL_TREE is no such field exists.  */
tree
get_fnfields (basetype, name)
     tree basetype, name;
{
  register tree baselink;
  register enum tree_code form;
  register tree field;

  /* First, see if there is a field or component with name NAME. */
  form = TREE_CODE (basetype);
  if (IS_AGGR_TYPE_CODE (form))
    {
      while (basetype)
	{
	  for (field = TYPE_FN_FIELDS (basetype);
	       field; field = TREE_CHAIN (field))
	    {
	      if (TREE_PURPOSE (field) == name)
		return field;
	    }

	  if (name == DECL_NAME (TYPE_NAME (basetype)))
	    return NULL_TREE;

	  basetype = TYPE_BASELINK (basetype);
	}
      return NULL_TREE;
    }
  abort();
}

/* Top-level interface to explicit overload requests. Allow NAME
   to be overloaded. Error if NAME is already declared for the current
   scope. Warning if function is redundanly overloaded. */

void
declare_overloaded (name)
     tree name;
{
  if (is_overloaded (name))
    warning ("function `%s' already declared overloaded",
	     IDENTIFIER_POINTER (name));
  else if (IDENTIFIER_GLOBAL_VALUE (name))
    error ("overloading function `%s' that is already defined",
	   IDENTIFIER_POINTER (name));
  else
    {
      TREE_OVERLOADED (name) = 1;
      IDENTIFIER_GLOBAL_VALUE (name) = build_tree_list (name, NULL_TREE);
      TREE_TYPE (IDENTIFIER_GLOBAL_VALUE (name)) = unknown_type_node;
    }
}

/* Check to see if NAME is overloaded. For first approximation,
   check to see if its TREE_OVERLOADED is set.  This is used on
   IDENTIFIER nodes.  */
int
is_overloaded (name)
     tree name;
{
  /* @@ */
  return (TREE_OVERLOADED (name)
	  && ! IDENTIFIER_CLASS_VALUE (name)
	  && ! IDENTIFIER_LOCAL_VALUE (name));
}

/* Given a list of parameters in PARMS, and a buffer in TEXT, of
   length LEN bytes, create an unambiguous overload string. Should
   distinguish any type that C (or C++) can distinguish. I.e.,
   pointers to functions are treated correctly.

   Any default conversions must take place before this function
   is called.  */

static char *
build_overload_name (parms, text, text_end)
     tree parms;
     char *text, *text_end;
{
  extern char *mode_name[];	/* rtl.h */
  char *textp = text;
  int just_one;
  tree parm;

  if (parms)
    {
      if (just_one = (TREE_CODE (parms) != TREE_LIST)) {
	parm = parms;
	goto only_one;
      }

      while (parms) {
	if (text_end - text < 4)
	  fatal ("Out of string space in build_overload_name!");
	parm = TREE_VALUE (parms);

      only_one:

	switch (TREE_CODE (parm))
	  {
	  case REFERENCE_TYPE:
	    *textp++ = 'R';
	    goto more;
	  case ARRAY_TYPE:
#ifdef PARM_CAN_BE_ARRAY_TYPE
	    *textp++ = 'A';
	    goto more;
#else
	    *textp++ = 'P';
	    goto more;
#endif
	  case POINTER_TYPE:
	    *textp++ = 'P';
	    if (TREE_TYPE (parm) == void_type_node)
	      {
		*textp++ = 'V';
		break;
	      }
	  more:
	    /* ignore trailing '_' */
	    textp = build_overload_name (TREE_TYPE (parm), textp, text_end) - 1;
	    break;

	  case FUNCTION_TYPE:
	    /* @@ It may be possible to pass a function type in
	       which is not preceded by a 'P'.  */
	    *textp++ = 'F';
	    textp = build_overload_name (TREE_TYPE (parm), textp, text_end) - 1;
	    *textp++ = '$';
	    /* keep extra '_' for arg visibility */
	    textp = build_overload_name (TYPE_ARG_TYPES (parm), textp, text_end);
	    break;

	  case INTEGER_TYPE:
	    if (TREE_UNSIGNED (parm))
	      {
		if (TYPE_MAIN_VARIANT (parm) == long_unsigned_type_node)
		  *textp++ = 'l';
		*textp++ = 'u';
	      }
	    else if (TYPE_MAIN_VARIANT (parm) == long_integer_type_node)
	      *textp++ = 'l';
	    strcpy (textp, mode_name[(int) TYPE_MODE (parm)]);
	    textp += strlen (textp);
	    break;

	  case REAL_TYPE:
	    if (TYPE_MAIN_VARIANT (parm) == long_double_type_node)
	      *textp++ = 'l';
	    strcpy (textp, mode_name[(int) TYPE_MODE (parm)]);
	    textp += strlen (textp);
	    break;

	  case COMPLEX_TYPE:
	    *textp++ = 'c';
	    strcpy (textp, mode_name[(int) TYPE_MODE (parm)]);
	    textp += strlen (textp);
	    break;

	  case BOOLEAN_TYPE:
	    *textp++ = 'b';
	    strcpy (textp, mode_name[(int) TYPE_MODE (parm)]);
	    textp += strlen (textp);
	    break;

	  case ERROR_MARK:	/* not right, but nothing is anyway */
	  case VOID_TYPE:
	    goto done;		/* end of parameter list */

	    /* have to do these */
	  case UNION_TYPE:
	    *textp++ = 'U';
	    goto common;
	  case ENUMERAL_TYPE:
	    *textp++ = 'E';
	    goto common;
	  case RECORD_TYPE:
	  case CLASS_TYPE:
	    *textp++ = 'S';
	    goto common;
	  common:
	    parm = TYPE_NAME (parm);
	    if (TREE_CODE (parm) == TYPE_DECL)
	      parm = DECL_NAME (parm);
	    if (TREE_CODE (parm) != IDENTIFIER_NODE)
	      abort ();
	    strcpy (textp, IDENTIFIER_POINTER (parm));
	    textp += IDENTIFIER_LENGTH (parm);
	    break;

	  case UNKNOWN_TYPE:
	    /* This will take some work.  */
	    *textp++ = '?';
	    break;

	    /* these are for PASCAL */
	  case CHAR_TYPE:
	  case FILE_TYPE:
	  case SET_TYPE:
	  case STRING_TYPE:
	    really_sorry ("PASCAL types");
	  case IDENTIFIER_NODE:
	    fatal("Identifier to overload?");

	  default:
	    abort();
	  }

	*textp++ = '_';
	if (just_one) break;
	parms = TREE_CHAIN (parms);
      }
    }
  done:
  *textp = '\0';		/* hassle from single type parameter */
  return textp;
}

/* Given a tree_code CODE, and some arguments (at least one),
   attempt to use an overloaded operator on the arguments.

   For unary operators, only the first argument need be checked.
   For binary operators, both arguments may need to be checked.

   Member functions can convert class references to class pointers,
   for one-level deep indirection.  More than that is not supported.
   Operators [](), ()(), and ->() must be member functions.

   We call function call building calls with nonzero complain if
   they are our only hope.  This is true when we see a vanilla operator
   applied to something of aggregate type.  If this fails, we are free to
   return `error_mark_node', because we will have reported the error.

   Operators NEW and delete overload in funny ways: operator new takes
   a single `size' parameter, and operator delete takes a pointer to the
   storage being deleted.  When overloading these operators, success is
   assumed.  If there is a failure, report an error message and return
   `error_mark_node'.  */

/* NOSTRICT */
tree
build_opfncall (code, xarg1, xarg2, arg3)
     enum tree_code code;
     tree xarg1, xarg2;
     tree arg3;
{
  extern int tree_code_length[];
  tree rval;
  tree arg1, arg2;
  tree type1, type2, fnname;
  tree fields1 = 0, parms = 0;
  tree global_fn;
  int try_second;
  int binary_is_unary;
  int complain;

  if (xarg1 == error_mark_node)
    return error_mark_node;

  if (code == COND_EXPR)
    {
      if (xarg2 == error_mark_node || arg3 == error_mark_node)
	return error_mark_node;
    }
  if (code == COMPONENT_REF)
    if (TREE_CODE (TREE_TYPE (xarg1)) == POINTER_TYPE)
      return NULL_TREE;

  /* Some tree codes have length > 1, but we really only want to
     overload them if their first argument has a user defined type.  */
  switch (code)
    {
    case PREINCREMENT_EXPR:
      code = POSTINCREMENT_EXPR;
      binary_is_unary = 1;
      try_second = 0;
      break;

    case POSTDECREMENT_EXPR:
      code = PREDECREMENT_EXPR;
      binary_is_unary = 1;
      try_second = 0;
      break;

    case PREDECREMENT_EXPR:
    case POSTINCREMENT_EXPR:
    case COMPONENT_REF:
      binary_is_unary = 1;
      try_second = 0;
      break;

    case CALL_EXPR:
    case ARRAY_REF:
    case METHOD_REF:
      binary_is_unary = 0;
      try_second = 0;
      break;

    case NEW_EXPR:
      rval = build_classfn_ref (build (NOP_EXPR, xarg1, integer_zero_node),
				get_identifier ("op$new_expr"),
				build_tree_list (NULL_TREE, xarg2), 1, 0);
      if (rval == error_mark_node)
	error ("could not call `operator new' (compiler error)");
      TREE_TYPE (rval) = xarg1;
      return rval;
      break;

    case DELETE_EXPR:
      if (xarg1 == NULL_TREE)
	xarg1 = build (NOP_EXPR, TREE_TYPE (xarg2), integer_zero_node);
      rval = build_classfn_ref (xarg1,
				get_identifier ("op$delete_expr"),
				build_tree_list (NULL_TREE, xarg2), 1, 0);
      if (rval == error_mark_node)
	error ("could not call `operator delete' (compiler error)");
      TREE_TYPE (rval) = void_type_node;
      return rval;
      break;

    default:
      binary_is_unary = 0;
      try_second = tree_code_length [(int) code] == 2;
      if (xarg2 == error_mark_node)
	return error_mark_node;
      break;
    }

  if (try_second && xarg2 == error_mark_node)
    return error_mark_node;

  /* First, see if we can work with the first argument */
  type1 = TREE_TYPE (xarg1);

  if (type1 == unknown_type_node
      || (try_second && TREE_TYPE (xarg2) == unknown_type_node))
    {
      /* This will not be implemented in the forseeable future.  */
      return 0;
    }

  /* What ever it was, we do not know how to deal with it.  */
  if (type1 == NULL_TREE)
    return 0;

  if (TREE_CODE (type1) == REFERENCE_TYPE)
    {
      arg1 = bash_reference_type (xarg1);
      type1 = TREE_TYPE (arg1);
    }
  else
    {
      arg1 = xarg1;
    }

  if (!IS_AGGR_TYPE (type1))
    {
      /* Try to fail. First, fail if unary */
      if (! try_second)
	return 0;
      /* Second, see if second argument is non-aggregate. */
      type2 = TREE_TYPE (xarg2);
      if (TREE_CODE (type2) == REFERENCE_TYPE)
	{
	  arg2 = bash_reference_type (xarg2);
	  type2 = TREE_TYPE (arg2);
	}
      else
	{
	  arg2 = xarg2;
	}

      if (!IS_AGGR_TYPE (type2))
	return 0;
      try_second = 0;
    }
  if (try_second)
    {
      /* First arg may succeed; see whether second should.  */
      type2 = TREE_TYPE (xarg2);
      if (TREE_CODE (type2) == REFERENCE_TYPE)
	{
	  arg2 = bash_reference_type (xarg2);
	  type2 = TREE_TYPE (arg2);
	}
      else
	{
	  arg2 = xarg2;
	}

      if (! IS_AGGR_TYPE (type2))
	try_second = 0;
    }

  if (code == MODIFY_EXPR)
    fnname = build_operator_fnname (build_nt (OP_EXPR, '=', arg3), 0, 2);
  else
    fnname = build_operator_fnname (build_nt (OP_EXPR, 0, (tree)code), 0, 1);

  global_fn = IDENTIFIER_GLOBAL_VALUE (fnname);

  /* This is the last point where we will accept failure.  This
     may be too eager if we wish an overloaded operator not to match,
     but would rather a normal operator be called on a type-converted
     argument.  */

  if (IS_AGGR_TYPE (type1))
    fields1 = get_fnfields (type1, fnname);

  if (fields1 == NULL_TREE && global_fn == NULL_TREE)
    return 0;

  /* If RVAL winds up being `error_mark_node', we will return
     that... There is no way that normal semantics of these
     operators will succeed.  */

  /* This argument may be an uncommited SCOPE_REF.  This is
     the case for example when dealing with static class members
     which are referenced from their class name rather than
     from a class instance.  */
  if (TREE_CODE (xarg1) == SCOPE_REF
      && TREE_CODE (TREE_OPERAND (xarg1, 1)) == FIELD_DECL)
    {
      xarg1 = IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (TREE_OPERAND (xarg1, 1)));
    }
  if (try_second && xarg2 && TREE_CODE (xarg2) == SCOPE_REF
      && TREE_CODE (TREE_OPERAND (xarg2, 1)) == FIELD_DECL)
    {
      xarg2 = IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (TREE_OPERAND (xarg2, 1)));
    }

  if (global_fn)
    complain = 4;
  else
    complain = 1;

  if (code == CALL_EXPR)
    {
      /* This can only be a member function.  */
      return build_classfn_ref (xarg1, fnname, xarg2, 1, 1);
    }
  else if (tree_code_length[(int) code] == 1 || binary_is_unary)
    {
      parms = NULL_TREE;
      rval = build_classfn_ref (xarg1, fnname, NULL_TREE, 1, complain);
    }
  else if (code == COND_EXPR)
    {
      parms = tree_cons (0, xarg2, build_tree_list (0, arg3));
      rval = build_classfn_ref (xarg1, fnname, parms, 1, complain);
    }
  else if (code == METHOD_REF)
    {
      /* must be a member function.  */
      parms = tree_cons (NULL_TREE, xarg2, arg3);
      return build_classfn_ref (xarg1, fnname, parms, 1, 1);
    }
  else if (fields1)
    {
      parms = build_tree_list (NULL_TREE, xarg2);
      rval = build_classfn_ref (xarg1, fnname, parms, 4, complain);
    }
  else
    {
      parms = tree_cons (NULL_TREE, xarg1,
			 build_tree_list (NULL_TREE, xarg2));
      rval = do_actual_overload (fnname, parms, 1, 0);
    }

  return rval;
}

/* This function takes an identifier, ID, and attempts to figure out what
   it means. There are a number of possible scenarios, presented in increasing
   order of hair:

   1) not in a class's scope
   2) in class's scope, member name of the class's method
   3) in class's scope, but not a member name of the class
   4) in class's scope, member name of a class's variable

   NAME is $1 from the bison rule. It is an IDENTIFIER_NODE.
   VALUE is $$ from the bison rule. It is the value returned by lookup_name ($1)
   yychar is the pending input character (suitably encoded :-).

   As a last ditch, try to look up the name as a label and return that
   address.  */

tree
hack_identifier (value, name, yychar)
     tree value, name;
{
  if (value == error_mark_node)
    {
      if (current_class_name)
	{
	  tree fields = get_fnfields (current_class_type, name);
	  if (fields)
	    {
	      fields = TREE_VALUE (fields);
	      if (TREE_CHAIN (fields) == NULL_TREE)
		{
		  warning ("Methods cannot be converted to function pointers");
		  return TREE_TYPE (fields);
		}
	      else
		{
		  error ("ambiguous request for method pointer");
		  return error_mark_node;
		}
	    }
	}
      if (flag_labels_ok && IDENTIFIER_LABEL_VALUE (name))
	{
	  return IDENTIFIER_LABEL_VALUE (name);
	}
      return error_mark_node;
    }

  if (TREE_NONLOCAL (value))
    {
      if (TREE_CODE (value) ==
#if 1
	  FIELD_DECL
#else
	  VAR_DECL
#endif
	  )
	{
	  if (yychar == '(')
	    if (! TREE_HAS_CALL_OVERLOADED (TREE_TYPE (value))
		&& TREE_CODE (TREE_TYPE (value)) != POINTER_TYPE
		&& TREE_CODE (TREE_TYPE (TREE_TYPE (value))) != FUNCTION_TYPE)
	    {
	      error ("component `%s' is not a method",
		     IDENTIFIER_POINTER (name));
	      return error_mark_node;
	    }
#ifdef WARNING_ABOUT_CCD
	  TREE_EVERUSED (current_class_decl) = 1;
#endif
	  return build_component_ref (C_C_D, name, 1);
	}

      /* if (TREE_CODE (value) == TREE_LIST
	 || TREE_CODE (value) == FUNCTION_DECL) */
	{
	  /* This is going to be a method.  */
	  if (yychar == '(')
	    {
#ifdef WARNING_ABOUT_CCD
	      TREE_EVERUSED (current_class_decl) = 1;
#endif
	      return value;
	    }
	}
    }

  TREE_EVERUSED (value) = 1;
  return value;
}

tree
hack_operator (op)
     tree op;
{
  if (op == NULL_TREE)
    return get_identifier ("<missing operator>");

  if (TREE_CODE (op) != TYPE_EXPR)
    return grokopexpr (op, 0);

  return op;
}

/* NONWRAPPER is nonzero if this call is not to be wrapped.
   TYPE is the type that the wrapper belongs to (in case
   it should be non-virtual.)
   DECL is the function will will be (not be) wrapped.  */
tree
hack_wrapper (nonwrapper, type, decl)
     int nonwrapper;
     tree type, decl;
{
  if (type == NULL_TREE || is_aggr_typedef_or_else (type))
    {
      if (type)
	type = TREE_TYPE (TREE_TYPE (type));

      if (nonwrapper)
	return build_nt (ANTI_WRAPPER_EXPR, type, decl);
      else
	return build_nt (WRAPPER_EXPR, type, decl);
    }
  return error_mark_node;
}

/* Return an IDENTIFIER which can be used as a name for
   anonymous structs and unions.  */
tree
make_anon_name ()
{
  static int cnt = 0;
  char buf[32];

  sprintf (buf, ANON_AGGRNAME_FORMAT, cnt++);
  return get_identifier (buf);
}

/* Given an object DATUM, and a type conversion operator COMPONENT
   build a call to the conversion operator, if a call is requested,
   or return the address (as a pointer to member function) if one is not.

   PROTECT says whether we apply C++ scoping rules or not.  */
tree
build_component_type_expr (datum, component, protect)
     tree datum;
     tree component;
     int protect;
{
  tree tmp, last;
  tree name;

  if (datum == NULL_TREE)
    {
      error ("object required for operator <typename>");
      return error_mark_node;
    }
  if (! IS_AGGR_TYPE (TREE_TYPE (datum)))
    {
      error ("object for operator <typename> is not of aggregate type (compiler error)");
      return error_mark_node;
    }

  if (TREE_CODE (component) != TYPE_EXPR)
    abort ();

  tmp = TREE_OPERAND (component, 1);
  last = NULL_TREE;

  while (tmp)
    {
      switch (TREE_CODE (tmp))
	{
	case CALL_EXPR:
	  if (last)
	    TREE_OPERAND (last, 0) = TREE_OPERAND (tmp, 0);
	  else
	    TREE_OPERAND (component, 1) = TREE_OPERAND (tmp, 0);
	  if (TREE_OPERAND (tmp, 1)
	      && TREE_VALUE (TREE_OPERAND (tmp, 1)) != void_type_node)
	    {
	      error ("operator <typename> requires empty parameter list");
	      TREE_OPERAND (tmp, 1) = NULL_TREE;
	    }
	  last = groktypename (build_tree_list (TREE_OPERAND (component, 0),
						TREE_OPERAND (component, 1)));
	  name = do_typename_overload (last);
	  TREE_TYPE (name) = last;
	  return build_classfn_ref (datum, name, NULL_TREE, protect, 1);

	case INDIRECT_REF:
	case ADDR_EXPR:
	case ARRAY_REF:
	  break;

	case SCOPE_REF:
	  error ("invalid scope request in operator <typename>");
	  break;

	default:
	  abort ();
	}
      last = tmp;
      tmp = TREE_OPERAND (tmp, 0);
    }

  last = groktypename (build_tree_list (TREE_OPERAND (component, 0), TREE_OPERAND (component, 1)));
  name = do_typename_overload (last);
  TREE_TYPE (name) = last;
  return build_component_ref (datum, name, protect);
}
