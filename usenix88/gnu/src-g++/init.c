/* Handle initialization things in C++.
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


/* High-level class interface. */

#include "config.h"
#include "tree.h"
#include "c-tree.h"
#include "overload.h"
#include "obstack.h"
#include "flags.h"

#define NULL 0

/* In C++, structures with well-defined constructors are initialized by
   those constructors, unasked.  CURRENT_BASE_INIT_LIST
   holds a list of stmts for a BASE_INIT term in the grammar.
   This list has one element for each base class which must be
   initialized.  The list elements are [basename, init], with
   type basetype.  This allows the possibly anachronistic form
   (assuming d : a, b, c) "d (int a) : c(a+5), b (a-4), a (a+3)"
   where each successive term can be handed down the constructor
   line.  Perhaps this was not intended.  */
tree current_base_init_list, current_member_init_list;

void init_init ();
void finish_base_init ();
void do_aggr_init ();
void do_member_init ();
void do_virtual_init ();

static tree do_vec_init ();
void expand_vec_delete ();
tree build_vec_delete ();

/* Cache _builtin_new and _builtin_delete exprs.  */
static tree BIN, BIVN, BID, BIVD;
static tree USR_NEW, USR_VNEW;
static tree build_user_new ();
static tree minus_one;

/* Set up local variable for this file.  MUST BE CALLED AFTER
   INIT_DECL_PROCESSING.  */
void init_init ()
{
  BIN = default_conversion (lookup_name (get_identifier ("__builtin_new")));
  BIVN = default_conversion (lookup_name (get_identifier ("__builtin_vec_new")));
  BID = default_conversion (lookup_name (get_identifier ("__builtin_delete")));
  BIVD = default_conversion (lookup_name (get_identifier ("__builtin_vec_delete")));
  USR_NEW = get_identifier ("__user_new");
  USR_VNEW = get_identifier ("__user_vec_new");

  minus_one = build_int_2 (-1, -1);
}

/* When a stmt has been parsed, this function is called.

   Currently, this function only does something within a
   constructor's scope: if a stmt has just assigned to this,
   and we are in a derived class, we call `finish_base_init'.  */
void
finish_stmt ()
{
#ifdef __GNUC__
  /* workaround "incomplete in scope ending here" error message.  */
  struct nesting { int x, y; };
#endif

  extern struct nesting *cond_stack, *loop_stack, *case_stack;

  if (current_function_name != current_class_name
      || current_function_assigns_this
      || ! current_function_just_assigned_this)
    return;
  if (cond_stack || loop_stack || case_stack)
    return;
  finish_base_init ();
  current_function_assigns_this = 1;
}

/* Perform whatever initialization have yet to be done on the
   base class of the class variable.  These actions are in
   the global variable CURRENT_BASE_INIT_LIST.  Such an
   action could be NULL_TREE, meaning that the user has explicitly
   called the base class constructor with no arguments.

   If there is a need for a call to a constructor, we
   must surround that call with a pushlevel/poplevel pair,
   since we are technically at the PARM level of scope.  */
   
void
finish_base_init ()
{
  tree basetype = TYPE_BASELINK (current_class_type);
  tree member, decl, name;
  tree type, result;
  char *filename;
  int line;

  filename = DECL_SOURCE_FILE (current_class_decl);
  line = DECL_SOURCE_LINE (current_class_decl);

  if (current_base_init_list)
    {
      tree init = TREE_VALUE (current_base_init_list);

      /* Only doing single inheritance so far.  */
      if (TREE_CHAIN (current_base_init_list))
	abort ();

      if (! basetype)
	error ("type `%s' does not have a base class",
	       IDENTIFIER_POINTER (current_class_name));
      else if (! TREE_NEEDS_CONSTRUCTOR (basetype))
	error ("base class hierarchy does not have any constructors defined");
      else
	{
	  /* The base initialization list goes up to the first
	     base class which can actually use it.  */
	  while (! TREE_HAS_CONSTRUCTOR (basetype))
	    basetype = TYPE_BASELINK (basetype);

	  TREE_TYPE (C_C_D) = basetype;
	  do_aggr_init (C_C_D, init, 0);
	  TREE_TYPE (C_C_D) = current_class_type;
	}
    }
  else if (basetype && TREE_NEEDS_CONSTRUCTOR (basetype))
    {
      TREE_TYPE (C_C_D) = basetype;
      do_aggr_init (C_C_D, NULL_TREE, 0);
      TREE_TYPE (C_C_D) = current_class_type;
    }

  /* If this type has a constructor, then that constructor will
     clobber the virtual function table.  Fix it here.  */
  if (TREE_VIRTUAL (current_class_type))
    do_virtual_init (C_C_D);

  /* Members we through do_member_init.  We initialize all the members
     needing initialization that did not get it so far.  */
  while (current_member_init_list)
    {
      tree name = TREE_PURPOSE (current_member_init_list);
      tree init = TREE_VALUE (current_member_init_list);
      tree field = IDENTIFIER_CLASS_VALUE (name);
      decl = build_component_ref (C_C_D, name, 1);

      type = TREE_TYPE (field);
      if (TREE_HAS_CONSTRUCTOR (type))
	{
	  do_aggr_init (decl, init);
	  DECL_INITIAL (field) = error_mark_node;
	}
      else if (TREE_NEEDS_CONSTRUCTOR (type))
	{
	  sorry ("initializing members which cannot take initializer directly");
	  DECL_INITIAL (field) = error_mark_node;
	}
      else
	{
	  if (init == NULL_TREE)
	    {
	      error ("types without constructors must have complete initializers");
	      init = error_mark_node;
	    }
	  else if (TREE_CHAIN (init))
	    {
	      warning ("initializer list treated as compound expression");
	      init = build_compound_expr (init);
	    }
	  else
	    init = TREE_VALUE (init);

	  if (TREE_VIRTUAL (TREE_TYPE (decl)))
	    do_virtual_init (decl);

	  expand_expr_stmt (build_modify_expr (decl, INIT_EXPR, init));
	}
      current_member_init_list = TREE_CHAIN (current_member_init_list);
    }

  for (member = TYPE_FIELDS (current_class_type); member; member = TREE_CHAIN (member))
    {
      decl = IDENTIFIER_CLASS_VALUE (DECL_NAME (member));

      /* Member had explicit initializer.  */
      if (DECL_INITIAL (decl))
	{
	  if (DECL_INITIAL (decl) != error_mark_node)
	    abort ();
	  DECL_INITIAL (decl) = 0;
	  continue;
	}
      if (TREE_STATIC (member))
	continue;

      type = TREE_TYPE (member);

      /* Implicitly tests if IS_AGGR_TYPE (type).  */
      if (type == error_mark_node)
	continue;

      if (TREE_NEEDS_CONSTRUCTOR (type))
	{
	  do_aggr_init (build_component_ref (C_C_D, DECL_NAME (member), 0),
			NULL_TREE);
	}
      else if (TREE_VIRTUAL (type))
	{
	  do_virtual_init (build_component_ref (C_C_D, DECL_NAME (member), 0));
	}
    }
}

/* This code sets up the virtual function table appropriate for
   the aggregate value DECL.  */
void
do_virtual_init (decl)
     tree decl;
{
  tree vtbl, vtbl_ptr;
  tree rval;

  vtbl = lookup_name (get_vtable_name (TREE_TYPE (decl)));
  if (vtbl == NULL_TREE) abort ();

  vtbl_ptr = build_component_ref (decl, get_vfield_name (TREE_TYPE (decl)), 0);
  rval = build_modify_expr (vtbl_ptr, NOP_EXPR,
			    build (ADDR_EXPR, TYPE_POINTER_TO (TREE_TYPE (vtbl)), vtbl));

  /* virtual init is called after the constructor for the base type.  */
  expand_expr_stmt (rval);
}

/* If NAME is a viable field name for the aggregate DECL,
   and PARMS is a viable parameter list, then expand an _EXPR
   which describes this initialization.

   Note that we do not need to chase through the class's base classes
   to look for NAME, because if it's in that list, it will be handled
   by the constructor for that base class.

   We do not yet have a fixed-point finder to instantiate types
   being fed to overloaded constructors.  If there is a unique
   constructor, then argument types can be got from that one.

   If INIT is non-NULL, then it the initialization should
   be placed in `current_base_init_list', where it will be processed
   by `finish_base_init'.  */
void
do_member_init (exp, name, init)
     tree exp, name, init;
{
  extern tree ptr_type_node;	/* should be in tree.h */

  tree basetype, field;
  tree method_name, fntype, function;
  tree parm;
  tree rval, type;
  tree actual_name;

  if (exp == NULL_TREE)
    return;			/* complain about this later */

  type = TYPE_MAIN_VARIANT (TREE_TYPE (exp));
  basetype = TYPE_BASELINK (type);
  if (init)
    {
      /* The grammar should not allow fields which have names
	 that are TYPENAMEs.  Therefore, if the field has
	 a non-NULL TREE_TYPE, we may assume that this is an
	 attempt to initialize a base class member of the current
	 type.  Otherwise, it is an attempt to initialize a
	 member field.  */

      if (init == void_type_node)
	init = NULL_TREE;

      if (name == NULL_TREE || TREE_TYPE (name))
	{
	  tree base_init;

	  if (name == NULL_TREE)
	    if (basetype)
	      name = DECL_NAME (TYPE_NAME (basetype));
	    else
	      {
		error ("no base class to initialize");
		return;
	      }
	  else
	    {
	      basetype = TREE_TYPE (TREE_TYPE (name));
	      if (! check_base_type (basetype, type))
		{
		  error ("type `%s' is not a base type for `%s'",
			 IDENTIFIER_POINTER (name),
			 TYPE_NAME_STRING (type));
		  return;
		}
	    }

	  if (purpose_member (name, current_base_init_list))
	    {
	      error ("base class `%s' already initialized",
		     IDENTIFIER_POINTER (name));
	      return;
	    }

	  base_init = build_tree_list (name, init);
	  TREE_TYPE (base_init) = basetype;
	  current_base_init_list = chainon (current_base_init_list, base_init);
	}
      else
	{
	  tree member_init;

	  field = get_field (type, name);
	  if (field == NULL_TREE)
	    {
	      error ("class `%s' does not have any field named `%s'",
		     TYPE_NAME_STRING (basetype),
		     IDENTIFIER_POINTER (name));
	      return;
	    }

	  if (purpose_member (name, current_member_init_list))
	    {
	      error ("field `%s' already initialized",
		     IDENTIFIER_POINTER (name));
	      return;
	    }

	  member_init = build_tree_list (name, init);
	  TREE_TYPE (member_init) = TREE_TYPE (field);
	  current_member_init_list = chainon (current_member_init_list, member_init);
	}
      return;
    }
  else if (name == NULL_TREE)
    {
      error ("compiler error in `do_member_init'");
      return;
    }

  basetype = type;
  field = get_field (basetype, name);
  if (field)
    {
      /* now see if there is a constructor for this type
	 which will take these args. */

      if (TREE_TYPE (field) == error_mark_node)
	rval = error_mark_node;
      if (TREE_HAS_CONSTRUCTOR (TREE_TYPE (field)))
	{
	  tree parmtypes, fndecl;

	  if (TREE_CODE (exp) == VAR_DECL
	      || TREE_CODE (exp) == PARM_DECL)
	    {
	      /* just know that we've seen something for this node */
	      DECL_INITIAL (exp) = error_mark_node;
	      TREE_EVERUSED (exp) = 1;
	    }
	  type = TYPE_MAIN_VARIANT (TREE_TYPE (field));
	  actual_name = DECL_NAME (TYPE_NAME (type));
	  parm = build_component_ref (exp, name, 0);

	  /* Now get to the constructor.  */
	  field = TREE_VALUE (get_fnfields (type, actual_name));
	  if (TREE_HAS_DESTRUCTOR (type))
	    field = TREE_CHAIN (field);

	  /* If the field is unique, we can use the parameter
	     types to guide possible type instantiation.  */
	  if (TREE_CHAIN (field) == NULL_TREE)
	    {
	      fndecl = TREE_TYPE (field);
	      parmtypes = TREE_CHAIN (TYPE_ARG_TYPES (TREE_TYPE (fndecl)));
	    }
	  else
	    {
	      parmtypes = NULL_TREE;
	      fndecl = NULL_TREE;
	    }

	  init = actualparameterlist (parmtypes, NULL_TREE, fndecl);
	  if (init == NULL_TREE || TREE_TYPE (init) != error_mark_node)
	    rval = build_classfn_ref (NULL_TREE, actual_name, init, 1, 1);
	  else
	    return;

	  if (rval != error_mark_node)
	    {
	      /* Now, fill in the first parm with our guy */
	      TREE_VALUE (TREE_OPERAND (rval, 1))
		= build_unary_op (ADDR_EXPR, parm, 0);
	      TREE_TYPE (rval) = ptr_type_node;
	      TREE_VOLATILE (rval) = 1;
	    }
	}
      else if (TREE_NEEDS_CONSTRUCTOR (TREE_TYPE (field)))
	{
	  parm = build_component_ref (exp, name, 0);
	  do_aggr_init (parm, NULL_TREE);
	  rval = error_mark_node;
	}

      /* Now initialize the member.  It does not have to
	 be of aggregate type to receive initialization.  */
      if (rval != error_mark_node)
	expand_expr_stmt (rval);
    }
  else
    {
      error ("class `%s' does not have any field named `%s'",
	     TYPE_NAME_STRING (basetype),
	     IDENTIFIER_POINTER (name));
    }
}

/* This is like `do_member_init', only it stores one aggregate
   value into another.  It is called by mark_aggr_birth.

   INIT comes in two flavors: it is either a value which
   is to be stored in DECL, or it is a parameter list
   to go to a constructor, which will operate on DECL.

   If INIT resolves to a CALL_EXPR which happens to return
   something of the type we are looking for, then we know
   that we can safely use that call to perform the
   initialization.

   The virtual function table pointer cannot be set up here, because
   we do not really know its type.

   This never calls operator=().  */

void
do_aggr_init (exp, init)
     tree exp, init;
{
  tree type = TREE_TYPE (exp);
  tree init_type = NULL_TREE;
  tree rval;
  tree member;

  if (exp == error_mark_node || type == error_mark_node)
    return;

  if (TREE_CODE (exp) == VAR_DECL
      || TREE_CODE (exp) == PARM_DECL)
    {
      /* just know that we've seen something for this node */
      TREE_EVERUSED (exp) = 1;
    }

  /* Use a function returning the desired type to initialize EXP for us.
     If the function is a constructor, and its first argument is
     NULL_TREE, know that it was meant for us--just slide exp on
     in and expand the constructor.  Constructors now come
     pre-dereferenced (INDIRECT_REF is their top CODE).  */
  if (init)
    {
      if (TREE_CODE (init) == TREE_LIST)
	init_type = TREE_TYPE (TREE_VALUE (init));
      else
	init_type = TREE_TYPE (init);

      if (TREE_CODE (init_type) == REFERENCE_TYPE)
	init_type = TREE_TYPE (init_type);

      if (! TREE_GETS_INIT_REF (type)
	  && TREE_CODE (init) == CALL_EXPR && init_type == type)
	{
	  /* A CALL_EXPR is a legitmate form of initialization, so
	     we should not print this warning message.  */
#if 0
	  if (TREE_GETS_ASSIGNMENT (type))
	    warning ("bitwise copy: `%s' has a member with operator=()",
		     TYPE_NAME_STRING (type));
#endif
	  store_expr (init, DECL_RTL (exp), 0);
	  if (exp == DECL_RESULT (current_function_decl))
	    {
	      if (DECL_INITIAL (DECL_RESULT (current_function_decl)))
		warning ("return value from function receives multiple initializations");
	      DECL_INITIAL (exp) = init;
	    }
	  return;
	}

      /* This is not a typo.  If `init' is a call to a constructor,
	 then we mark it as such with TREE_HAS_CONSTRUCTOR.  */
      if (TREE_HAS_CONSTRUCTOR (init))
	{
	  tree tmp = TREE_OPERAND (TREE_OPERAND (init, 0), 1);
	  if (tmp == NULL_TREE
	      || (TREE_CODE (TREE_VALUE (tmp)) == NOP_EXPR
		  && (TREE_OPERAND (TREE_VALUE (tmp), 0) == integer_zero_node)))
	    {
	      /* In order for this to work for RESULT_DECLs, if their
		 type has a constructor, then they must be BLKmode
		 so that they will be meaningfully addressable.  */
	      init = TREE_OPERAND (init, 0);
	      exp = build_unary_op (ADDR_EXPR, exp, 0);

	      if (tmp == NULL_TREE)
		{
		  TREE_OPERAND (init, 1) = build_tree_list (NULL_TREE, exp);
		  fatal ("empty parm list");
		}
	      else
		TREE_VALUE (TREE_OPERAND (init, 1)) = exp;
	      TREE_HAS_CONSTRUCTOR (init) = 0;
	      expand_expr_stmt (init);
	      return;
	    }
	}

      /* Handle this case: when calling a constructor: xyzzy foo(bar);
	 which really means:  xyzzy foo = bar; Ugh!  */

      if (! TREE_NEEDS_CONSTRUCTOR (type))
	{
	  if (TREE_CODE (init) == TREE_LIST)
	    if (TREE_CHAIN (init) == NULL_TREE)
	      init = TREE_VALUE (init);
	    else
	      {
		warning ("initializer list being treated as compound expression");
		init = build_compound_expr (init);
	      }
	  expand_expr_stmt (build_modify_expr (exp, INIT_EXPR, init));
	  return;
	}
    }

  if (TREE_HAS_CONSTRUCTOR (type))
    {
      /* The following strategy fails for a case in the OOPS code:
         @@ If the initializer is not a list, first try to call a constructor
	 @@ with that as an argument.  If that fails, try to build an
	 @@ assignment using `='.

	 It fails because there may not be a constructor which takes
	 its own type as the first (or only parameter), but which does
	 take other types via a conversion.  So, if the thing initializing
	 the expression is a unit element of type X, first try X(X&),
	 followed by initialization by X.  If neither of these work
	 out, then look hard.  */

      tree parms = (init == NULL_TREE || TREE_CODE (init) == TREE_LIST)
	? init : build_tree_list (NULL_TREE, init);

      if (parms) init = TREE_VALUE (parms);

      if (parms && TREE_CHAIN (parms) == NULL_TREE
	  && init_type == type
	  && ! TREE_GETS_INIT_REF (type))
	{
	  rval = build_modify_expr (exp, INIT_EXPR, init);
	  expand_expr_stmt (rval);
	  return;
	}
      rval = build_classfn_ref (exp, DECL_NAME (TYPE_NAME (type)), parms, 0,
				parms == NULL_TREE || TREE_CHAIN (parms));
      if (rval != error_mark_node)
	{
	  /* p. 222: if the base class assigns to `this', then that
	     value is used in the derived class.  */
	  if (exp == C_C_D && TREE_TYPE (C_C_D) != current_class_type)
	    if (TREE_RETURNS_FIRST_ARG (TREE_OPERAND (TREE_OPERAND (rval, 0), 0)) == 0)
	      {
		TREE_TYPE (rval) = TREE_TYPE (current_class_decl);
		rval = build_modify_expr (current_class_decl, INIT_EXPR, rval);
	      }
	}
      else
	{
	  char *err_name = TYPE_NAME_STRING (type);

	  /* We have already tried reasonable forms of assignment via constructors
	     and failed.  Type conversions were also tried.  Make an
	     attempt via operator = (which may or may not be overloaded).  */

	  if (parms && TREE_CHAIN (parms) == NULL_TREE)
	    {
	      rval = build_opfncall (MODIFY_EXPR, exp, init, NOP_EXPR);
	      if (rval == NULL_TREE)
		{
		  if (comptypes (type, init_type, 0))
		    {
		      warning ("attempts to call constructor for `%s' failed; using bitwise copy", TYPE_NAME_STRING (type));
		      rval = build_modify_expr (exp, INIT_EXPR, init);
		    }
		  else
		    {
		      error ("bad argument to constructor `%s'", err_name);
		      rval = error_mark_node;
		    }
		}
	    }
	  else if (init)
	    error ("bad argument list to constructor `%s'", err_name);
	  else
	    error ("no default constructor defined for type `%s'", err_name);
	}
      expand_expr_stmt (rval);
    }
  else if (TREE_CODE (type) == ARRAY_TYPE)
    {
      if (TREE_NEEDS_CONSTRUCTOR (type))
	do_vec_init (exp, exp, TYPE_NELTS (type), init);
    }
  else
    {
      tree basetype = TYPE_BASELINK (type);

      if (init && init_type == type)
	{
	  if (TREE_CODE (init) == TREE_LIST
	      && TREE_CHAIN (init) == NULL_TREE)
	    init = TREE_VALUE (init);
	  rval = build_modify_expr (exp, INIT_EXPR, init);
	  expand_expr_stmt (rval);

	  if (TREE_VIRTUAL (type)
	      && TREE_CODE (init) != VAR_DECL
	      && TREE_CODE (init) != PARM_DECL
	      && TREE_CODE (init) != RESULT_DECL)
	    do_virtual_init (exp);
	  return;
	}

      while (basetype && ! TREE_HAS_CONSTRUCTOR (basetype))
	basetype = TYPE_BASELINK (basetype);

      if (basetype)
	{
	  /* It may need a constructor but not have a base type.
	     This happens when one of its members needs a constructor.  */
	  TREE_TYPE (exp) = basetype;
	  do_aggr_init (exp, init);
	  TREE_TYPE (exp) = type;
	}

      if (TREE_VIRTUAL (type))
	do_virtual_init (exp);

      /* Members we through do_member_init.  We initialize all the members
	 needing initialization that did not get it so far.  */
      for (member = TYPE_FIELDS (type); member; member = TREE_CHAIN (member))
	{
	  if (TREE_STATIC (member))
	    continue;

	  type = TREE_TYPE (member);

	  if (type == error_mark_node)
	    continue;

	  /* Implicitly tests if IS_AGGR_TYPE (type).  */
	  if (TREE_NEEDS_CONSTRUCTOR (type))
	    do_member_init (exp, DECL_NAME (member), NULL_TREE);
	}
    }
}

/* Report an error if NAME is not the name of a user-defined,
   aggregate type.

   Must handle CLASS_TYPE, since it may be called before
   the type is fully processed.  */
int
is_aggr_typedef_or_else (name)
     tree name;
{
  tree type = TREE_TYPE (name);

  if (type == NULL_TREE || TREE_CODE (type) != TYPE_DECL)
    {
      error ("`%s' fails to be an aggregate typedef",
	     IDENTIFIER_POINTER (name));
      return 0;
    }
  type = TREE_TYPE (type);
  if (! IS_AGGR_TYPE (type) && TREE_CODE (type) != CLASS_TYPE)
    {
      fatal ("type `%s' is of non-aggregate type",
	     IDENTIFIER_POINTER (name));
      return 0;
    }
  return 1;
}

/* Return the name of the virtual function table (as an IDENTIFIER_NODE)
   for the given TYPE.  */
tree
get_vtable_name (type)
     tree type;
{
  char buf[OVERLOAD_MAX_LEN];
  if (! IS_AGGR_TYPE (type))
    abort ();

  sprintf (buf, VTABLE_NAME_FORMAT, TYPE_NAME_STRING (type));
  return get_identifier (buf);
}

/* Return the name of the virtual function pointer field
   (as an IDENTIFIER_NODE) for the given TYPE.  Note that
   this may have to look back through base types to find the
   ultimate field name.  (For single inheritance, these could
   all be the same name.  Who knows for multiple inheritance).  */
tree
get_vfield_name (type)
     tree type;
{
  char buf[OVERLOAD_MAX_LEN];
  if (! IS_AGGR_TYPE (type))
    abort ();

  while (TYPE_BASELINK (type) && TREE_VIRTUAL (TYPE_BASELINK (type)))
    type = TYPE_BASELINK (type);
  sprintf (buf, VFIELD_NAME_FORMAT, TYPE_NAME_STRING (type));
  return get_identifier (buf);
}

/* This code could just as well go in `class.c', but is placed here for
   modularity.  */

/* For an expression of the form CNAME :: NAME (PARMLIST), build
   the appropriate function call.  */
tree
build_member_call (cname, name, parmlist, dtor)
     tree cname, name, parmlist;
     int dtor;
{
  tree type, t;

  if (! is_aggr_typedef_or_else (cname))
    return error_mark_node;

  /* An operator we did not like.  */
  if (name == NULL_TREE)
    return error_mark_node;

  if (dtor)
    {
      if (! TREE_HAS_DESTRUCTOR (TREE_TYPE (TREE_TYPE (cname))))
	{
	  error ("type `%s' does not have a destructor",
		 IDENTIFIER_POINTER (cname));
	}
      else
	{
	  error ("destructor specification error");
	}
      return error_mark_node;
    }

  if (! current_class_decl)
    {
      error ("object missing for `%s::%s'",
	     IDENTIFIER_POINTER (cname),
	     IDENTIFIER_POINTER (name));
      return error_mark_node;
    }

  type = TREE_TYPE (TREE_TYPE (cname));
  t = get_fnfields (type, name);
  if (t)
    {
      tree decl;
      /* This cannot overload `operator ()' because the
	 base type is not aggregate.  */

      if (cname == current_class_name)
	decl = current_class_decl;
      else if (comptypes (type, current_class_type, 0))
	{
	  decl = copy_node (current_class_decl);
	  TREE_TYPE (decl) = TYPE_POINTER_TO (DECL_CONTEXT (TREE_VALUE (t)));
	}
      else
	{
	  error ("incompatible type conversion in member function call");
	  return error_mark_node;
	}
      return do_actual_overload (name, tree_cons (NULL_TREE, decl, parmlist), 1, 0);
    }
  else
    {
      error ("no method `%s::%s'", IDENTIFIER_POINTER (cname),
	     IDENTIFIER_POINTER (name));
      return error_mark_node;
    }
}

/* Build a reference to a member of an aggregate.  This is not a
   C++ `&', but really something which can have its address taken,
   and then act as a pointer to member, for example CNAME :: FIELD
   can have its address taken by saying & CNAME :: FIELD.  */
tree
build_member_ref (cname, name, dtor)
     tree cname, name;
     int dtor;
{
  tree type, t, offset;

  if (cname == NULL_TREE)
    {
      /* this is using the `::name' method of accessing global functions.  */
      return build_nt (SCOPE_REF, NULL_TREE, name);
    }
  if (! is_aggr_typedef_or_else (cname))
    {
      return error_mark_node;
    }
  type = TREE_TYPE (TREE_TYPE (cname));

  t = get_fnfields (type, name);
  if (t)
    {
      if (TREE_CHAIN (TREE_VALUE (t)) == NULL_TREE || dtor)
	{
	  /* unique functions are handled easily.  */
	  tree pftype;
	  t = TREE_VALUE (t);
	unique:
	  pftype = build_pointer_type (TREE_TYPE (TREE_TYPE (t)));
	  if (TREE_VIRTUAL (t)
	      || (flag_all_virtual && TREE_TYPE (DECL_NAME (t)) == NULL_TREE))
	    {
	      type = build_type_variant (pftype, 1, 0, type);
	      offset = DECL_VINDEX (t);
	    }
	  else
	    {
	      type = build_type_variant (pftype,
					 TREE_READONLY (t), TREE_VOLATILE (t),
					 type);
	      offset = TREE_TYPE (t);
	    }
	  return build (SCOPE_REF, TYPE_DOMAIN (type), type, offset);
	}
      else
	{
	  /* overloaded functions may need more work.  */
	  if (cname == name)
	    {
	      if (! TREE_HAS_CONSTRUCTOR (type))
		{
		  error ("class `%s' does not have a constructor",
			 IDENTIFIER_POINTER (cname));
		  return error_mark_node;
		}
	      t = TREE_CHAIN (TREE_VALUE (t));
	      if (TREE_CHAIN (t) == NULL_TREE)
		{
		  goto unique;
		}
	    }

	  type = build_type_variant (TYPE_POINTER_TO (type),
				     0, 0, type);
	  return build (COMPONENT_REF, unknown_type_node, TYPE_DOMAIN (type), t);
	}
    }
  else
    {
      t = get_field (type, name);
      if (t == NULL_TREE)
	{
	  error ("field `%s' is not a member of type `%s'",
		 IDENTIFIER_POINTER (name),
		 IDENTIFIER_POINTER (cname));
	  return error_mark_node;
	}

      /* In constructors and destructors, the form `cname::fname' is
	 equivalent to `this->fname', except, of course, for static
	 members, which get their value from a special place.  */
      if (type == current_class_type
	  && current_function_name == current_class_name)
	{
	  if (TREE_STATIC (t))
	    return IDENTIFIER_GLOBAL_VALUE (DECL_STATIC_NAME (t));
	  else
	    return build (COMPONENT_REF, TREE_TYPE (t), C_C_D, t);
	}

      /* This could be a pointer to a member.  */
      type = build_type_variant (build_pointer_type (TREE_TYPE (t)),
				 TREE_READONLY (t),
				 TREE_VOLATILE (t),
				 type);
      if (TREE_STATIC (t))
	{
	  return build (SCOPE_REF, TREE_TYPE (t), type, t);
	}
      else
	{
	  /* @@ What is the correct machine-independent way to do this?  */
	  offset = build_int_2 (DECL_OFFSET (t) / DECL_SIZE_UNIT (t), 0);
	  return build (SCOPE_REF, TREE_TYPE (t), type, offset);
	}
    }
}

/* If a SCOPE_REF made it through to here, then it did
   not have its address taken.  In that case, we attempt
   to expand it as though it were a normal COMPONENT_REF.
   If field is static, return the value of the static field,
   otherwise, if base type is derived from `current_class_type',
   make foo::bar really `this->foo::bar'.  */

tree
resolve_scope_ref (exp)
     tree exp;
{
  tree basetype = TREE_TYPE (exp);
  tree field = TREE_OPERAND (exp, 1);

  /* This happens when we use `::name'.  Just return `name'.  */
  if (basetype == NULL_TREE)
    return field;

  if (TREE_CODE (field) == FUNCTION_DECL)
    return build (ADDR_EXPR, build_pointer_type (TREE_TYPE (field)), field);
  else if (TREE_CODE (field) == TREE_LIST)
    return field;

  if (TREE_STATIC (field))
    {
      tree t = DECL_STATIC_NAME (field);
      return IDENTIFIER_GLOBAL_VALUE (t);
    }
  if (! current_class_type)
    {
      error ("class member reference `%s' does not have a `this' pointer",
	     TYPE_NAME_STRING (basetype));
      return error_mark_node;
    }
  if (! check_base_type (basetype, current_class_type))
    {
      error ("class `%s' not in current class hierarchy",
	     TYPE_NAME_STRING (basetype));
      return error_mark_node;
    }
  return build (COMPONENT_REF, TREE_TYPE (field), basetype, field);
}

int member_pointer_p (exp)
     tree exp;
{
  if (TREE_MEMBER_POINTER (TREE_TYPE (exp)))
    return 1;
  if (TREE_CODE (exp) == NOP_EXPR)
    return member_pointer_p (TREE_OPERAND (exp, 0));
  else if (TREE_CODE (exp) == CONVERT_EXPR)
    return member_pointer_p (TREE_OPERAND (exp, 0));
  else if (TREE_CODE (exp) == COMPOUND_EXPR)
    return member_pointer_p (TREE_OPERAND (exp, 1));
  else if (TREE_CODE (exp) == SCOPE_REF)
    return TREE_OPERAND (exp, 0)
      ? TREE_MEMBER_POINTER (TREE_OPERAND (exp, 0)) : 0;
  else return 0;
}

tree maybe_convert_decl_to_const (decl)
     tree decl;
{
  if (! flag_const_is_variable
      && TREE_CODE (decl) == VAR_DECL
      && (TREE_READONLY (decl) || (TREE_READONLY (TREE_TYPE (decl))
				   && fatal ("constant type but non-constant decl")))
      && DECL_INITIAL (decl))
    return DECL_INITIAL (decl);
  return decl;
}

/* Some simple list processing predicates.  */

/* Check whether TYPE is derived from PARENT.  */
int
check_base_type (parent, type)
     register tree parent, type;
{
  while (type)
    {
      if (type == parent)
	return 1;
      type = TYPE_BASELINK (type);
    }
  return 0;
}

/* Return list element whose TREE_VALUE is  ELEM.
   Return 0 if ELEM is not in LIST.  */
tree
value_member (elem, list)
     tree elem, list;
{
  while (list)
    {
      if (elem == TREE_VALUE (list))
	return list;
      list = TREE_CHAIN (list);
    }
  return NULL_TREE;
}

/* Return list element whose TREE_PURPOSE is  ELEM.
   Return 0 if ELEM is not in LIST.  */
tree
purpose_member (elem, list)
     tree elem, list;
{
  while (list)
    {
      if (elem == TREE_PURPOSE (list))
	return list;
      list = TREE_CHAIN (list);
    }
  return NULL_TREE;
}

/* Friend handling routines.  */

/* Tell if this function specified by FUNCTION_DECL
   can be a friend of type TYPE.
   Return the type of visibility that this friend
   has for the given type, or NO_VISIBILITY if
   this function cannot be a friend.  */
int
is_friend (decl, type)
     tree decl, type;
{
  tree typedecl = TYPE_NAME (type);
  tree ctype = NULL_TREE;
  tree list;
  tree name;

  if (TREE_METHOD (decl))
    {
      tree fntype = TREE_TYPE (decl);
      list = DECL_FRIEND_CLASSES (typedecl);
      ctype = TREE_TYPE (TREE_VALUE (TYPE_ARG_TYPES (fntype)));
      while (list)
	{
	  if (ctype == TREE_VALUE (list))
	    return 1;
	  list = TREE_CHAIN (list);
	}
    }

  list = DECL_FRIENDLIST (typedecl);
  name = DECL_ORIGINAL_NAME (decl);
  while (list)
    {
      if (name == TREE_PURPOSE (list))
	{
	  tree friends = TREE_VALUE (list);
	  while (friends)
	    {
	      if (ctype == TREE_PURPOSE (friends))
		return 1;
	      if (decl == TREE_VALUE (friends))
		return 1;
	      friends = TREE_CHAIN (friends);
	    }
	  return 0;
	}
      list = TREE_CHAIN (list);
    }
  return 0;
}

/* Add a new friend to the friends of the aggregate type TYPE.
   DECL is the FUNCTION_DECL of the friend being added.  */
void
add_friend (type, decl)
     tree type, decl;
{
  tree typedecl = TYPE_NAME (type);
  tree list = DECL_FRIENDLIST (typedecl);
  tree name = DECL_ORIGINAL_NAME (decl);

  while (list)
    {
      if (name == TREE_PURPOSE (list))
	{
	  tree friends = TREE_VALUE (list);
	  while (friends)
	    {
	      if (decl == TREE_VALUE (friends))
		{
		  char buf[OVERLOAD_MAX_LEN];
		  warning ("`%s' is already a friend of class `%s'",
			   hack_function_decl (buf, NULL_TREE, decl, 0),
			   IDENTIFIER_POINTER (DECL_NAME (typedecl)));
		  return;
		}
	      friends = TREE_CHAIN (friends);
	    }
	  TREE_VALUE (list) = tree_cons (error_mark_node, decl,
					 TREE_VALUE (list));
	  return;
	}
      list = TREE_CHAIN (list);
    }
  DECL_FRIENDLIST (typedecl)
    = tree_cons (DECL_ORIGINAL_NAME (decl),
		 build_tree_list (error_mark_node, decl),
		 DECL_FRIENDLIST (typedecl));
  if (! strncmp (IDENTIFIER_POINTER (DECL_NAME (decl)),
		 "op$modify_expr", 11))
    {
      tree parmtypes = TYPE_ARG_TYPES (TREE_TYPE (TREE_TYPE (decl)));
      TREE_HAS_ASSIGNMENT (TREE_TYPE (typedecl)) = 1;
      TREE_GETS_ASSIGNMENT (TREE_TYPE (typedecl)) = 1;
      if (parmtypes && TREE_CHAIN (parmtypes))
	{
	  tree parmtype = TREE_VALUE (TREE_CHAIN (parmtypes));
	  if (TREE_CODE (parmtype) == REFERENCE_TYPE
	      && TREE_TYPE (parmtypes) == TREE_TYPE (typedecl))
	    {
	      TREE_HAS_ASSIGN_REF (TREE_TYPE (typedecl)) = 1;
	      TREE_GETS_ASSIGN_REF (TREE_TYPE (typedecl)) = 1;
	    }
	}
    }
}

/* Declare that every member function NAME in FRIEND_TYPE
   (which may be NULL_TREE) is a friend of type TYPE.  */
void
add_friends (type, name, friend_type)
     tree type, name, friend_type;
{
  tree typedecl = TYPE_NAME (type);
  tree list = DECL_FRIENDLIST (typedecl);

  while (list)
    {
      if (name == TREE_PURPOSE (list))
	{
	  tree friends = TREE_VALUE (list);
	  while (friends && TREE_PURPOSE (friends) != friend_type)
	    friends = TREE_CHAIN (friends);
	  if (friends)
	    if (friend_type)
	      warning ("method `%s::%s is already a friend of class",
		       TYPE_NAME_STRING (friend_type),
		       IDENTIFIER_POINTER (name));
	    else
	      warning ("function `%s' is already a friend of class `%s'",
		       IDENTIFIER_POINTER (name),
		       IDENTIFIER_POINTER (DECL_NAME (typedecl)));
	  else
	    TREE_VALUE (list) = tree_cons (friend_type, NULL_TREE,
					   TREE_VALUE (list));
	  return;
	}
      list = TREE_CHAIN (list);
    }
  DECL_FRIENDLIST (typedecl) =
    tree_cons (name,
	       build_tree_list (friend_type, NULL_TREE),
	       DECL_FRIENDLIST (typedecl));
  if (! strncmp (name, "op$modify_expr", 11))
    {
      TREE_HAS_ASSIGNMENT (TREE_TYPE (typedecl)) = 1;
      TREE_GETS_ASSIGNMENT (TREE_TYPE (typedecl)) = 1;
      sorry ("declaring \"friend operator =\" will not find \"operator = (X&)\" if it exists");
    }
}

/* Set up a cross reference so that functions with name NAME and
   type FRIEND_TYPE know that they are friends of TYPE.  */
void
add_friend_xref (type, name, friend_type)
     tree type, name, friend_type;
{
  tree decl = TYPE_NAME (type);
  tree friend_decl = TYPE_NAME (friend_type);
  tree t = tree_cons (NULL_TREE, friend_type,
		      DECL_UNDEFINED_FRIENDS (decl));

  DECL_UNDEFINED_FRIENDS (decl) = t;
  SET_DECL_WAITING_FRIENDS (friend_decl, tree_cons (type, t, DECL_WAITING_FRIENDS (friend_decl)));
  TREE_TYPE (DECL_WAITING_FRIENDS (friend_decl)) = name;
}

/* Make FRIEND_TYPE a friend class to TYPE.  If FRIEND_TYPE has already
   been defined, we make all of its member functions friends of
   TYPE.  If not, we make it a pending friend, which can later be added
   when its definition is seen.  If a type is defined, then its TYPE_DECL's
   DECL_UNDEFINED_FRIENDS contains a (possibly empty) list of friend
   classes that are not defined.  If a type has not yet been defined,
   then the DECL_WAITING_FRIENDS contains a list of types
   waiting to make it their friend.  Note that these two can both
   be in use at the same time!  */
void
make_friend_class (type, friend_type)
     tree type, friend_type;
{
  tree typedecl = TYPE_NAME (type);
  tree classes;

  if (type == error_mark_node
      || friend_type == error_mark_node)
    return;

  if (type == friend_type)
    {
      warning ("class `%s' is implicitly friends with itself",
	       TYPE_NAME_STRING (type));
      return;
    }

  classes = DECL_FRIEND_CLASSES (typedecl);
  while (classes && TREE_VALUE (classes) != friend_type)
    classes = TREE_CHAIN (classes);
  if (classes)
    warning ("class `%s' is already friends with class `%s'",
	     TYPE_NAME_STRING (type), TYPE_NAME_STRING (type));
  else
    {
      DECL_FRIEND_CLASSES (typedecl)
	= tree_cons (NULL_TREE, friend_type,
		     DECL_FRIEND_CLASSES (typedecl));
    }
}

/* Main friend processor.  This is large, and for modularity purposes,
   has been removed from grokdeclarator.  It returns `void_type_node'
   to indicate that something happened, though a FIELD_DECL is
   not returned.  */
tree
do_friend (cname, declarator, type, flags)
     tree cname, declarator, type;
     enum overload_flags flags;
{
  if (cname)
    {
      /* A method friend.  */
      if (TREE_CODE (type) == FUNCTION_DECL)
	{
	  grokclassfn (cname, type, flags, 1);
	  if (TREE_TYPE (type) != error_mark_node)
	    {
	      tree decl = pushdecl (type);
	      rest_of_decl_compilation (decl, NULL_TREE, 1, 0);
	      add_friend (current_class_type, decl);
	      return build_decl (FRIEND_DECL, DECL_ORIGINAL_NAME (decl), decl);
	    }
	}
      else
	{
	  /* Possibly a bunch of method friends.  */

	  /* Get the class they belong to.  */
	  tree ctype = TREE_TYPE (TREE_TYPE (cname));

	  /* This class is defined, use its methods now.  */
	  if (TYPE_SIZE (ctype))
	    {
	      tree fields = get_fnfields (ctype, declarator);
	      if (fields)
		add_friends (current_class_type, declarator, type);
	      else
		error ("method `%s' is not a member of class `%s'",
		       IDENTIFIER_POINTER (declarator),
		       IDENTIFIER_POINTER (cname));
	    }
	  else
	    add_friend_xref (current_class_type, declarator, type);
	}
      return void_type_node;
    }
  /* A global friend.
     @@ or possibly a friend from a base class ?!?  */
  if (TREE_CODE (type) == FUNCTION_DECL)
    {
      tree decl = type;
      tree glob = IDENTIFIER_GLOBAL_VALUE (declarator);

      /* Friends must all go through the overload machinery,
	 even though they may not technically be overloaded.

	 Note that because classes all wind up being top-level
	 in their scope, their friend wind up in top-level scope as well.  */
      DECL_NAME (decl) =
	do_decl_overload (IDENTIFIER_POINTER (declarator),
			  TYPE_ARG_TYPES (TREE_TYPE (type)));
      decl = pushdecl (decl);
      rest_of_decl_compilation (decl, NULL_TREE, 1, 0);
      add_friend (current_class_type, decl);
      TREE_OVERLOADED (decl) = 1;
      TREE_OVERLOADED (declarator) = 1;

      if (glob)
	{
	  if (TREE_CODE (glob) == FUNCTION_DECL)
	    glob = build_tree_list (NULL_TREE, glob);
	  if (TREE_VALUE (glob) == NULL_TREE)
	    TREE_VALUE (glob) = decl;
	  else if (value_member (decl, glob) == 0)
	    {
	      tree tmp = tree_cons (declarator, decl, glob);
	      IDENTIFIER_GLOBAL_VALUE (declarator) = tmp;
	      TREE_TYPE (tmp) = unknown_type_node;
	    }
	}
      else
	{
	  tree tmp = tree_cons (declarator, decl, glob);
	  IDENTIFIER_GLOBAL_VALUE (declarator) = tmp;
	  TREE_TYPE (tmp) = unknown_type_node;
	}
      return build_decl (FRIEND_DECL, DECL_ORIGINAL_NAME (decl), decl);
    }
  else
    {
      /* @@ Should be able to ingest later definitions of this function
	 before use.  */
      tree decl = IDENTIFIER_GLOBAL_VALUE (declarator);
      if (decl == NULL_TREE)
	{
	  warning ("implicitly declaring `%s' as struct",
		   IDENTIFIER_POINTER (declarator));
	  decl = xref_tag (record_type_node, declarator, NULL_TREE);
	  decl = TYPE_NAME (decl);
	}

      /* Allow abbreviated declarations of overloaded functions,
	 but not if those functions are really class names.  */
      if (TREE_CODE (decl) == TREE_LIST && TREE_TYPE (TREE_PURPOSE (decl)))
	{
	  warning ("`friend %s' archaic, use `friend class %s' instead",
		   IDENTIFIER_POINTER (declarator),
		   IDENTIFIER_POINTER (declarator));
	  decl = TREE_TYPE (TREE_PURPOSE (decl));
	}

      if (TREE_CODE (decl) == TREE_LIST)
	add_friends (current_class_type, TREE_PURPOSE (decl), NULL_TREE);
      else
	make_friend_class (current_class_type, TREE_TYPE (decl));

    }
  return void_type_node;
}

/* TYPE has now been defined.  It may, however, have a number of things
   waiting make make it their friend.  We resolve these references
   here.  */
void
embrace_waiting_friends (type)
     tree type;
{
  tree waiters = DECL_WAITING_FRIENDS (TYPE_NAME (type));
  while (waiters)
    {
      tree waiter = TREE_PURPOSE (waiters);
      tree waiter_prev = TREE_VALUE (waiters);

      if (TREE_TYPE (waiters))
	{
	  tree field = get_fnfields (type, TREE_TYPE (waiters));
	  if (field)
	    add_friends (waiter, TREE_TYPE (waiters));
	  else
	    yylineerror (DECL_SOURCE_LINE (TYPE_NAME (waiter)),
			 "no method `%s' defined in class `%s'",
			 IDENTIFIER_POINTER (TREE_TYPE (waiters)),
			 TYPE_NAME_STRING (type));
	}
      else
	{
	  make_friend_class (type, waiter);
	}
      if (TREE_CHAIN (waiter_prev))
	{
	  TREE_CHAIN (waiter_prev) = TREE_CHAIN (TREE_CHAIN (waiter_prev));
	}
      else
	{
	  DECL_UNDEFINED_FRIENDS (TYPE_NAME (waiter)) = NULL_TREE;
	}
      waiters = TREE_CHAIN (waiters);
    }
}

void
add_method (type, fields, function)
     tree type, fields, function;
{
  tree decl = build_decl (FIELD_DECL,
			  DECL_ORIGINAL_NAME (function), function);
  if (fields)
    {
      TREE_CHAIN (decl) = TREE_VALUE (fields);
      TREE_VALUE (fields) = decl;
    }
  else
    {
      if (TREE_HAS_CONSTRUCTOR (type))
	{
	  TREE_CHAIN (TYPE_FN_FIELDS (type))
	    = tree_cons (DECL_ORIGINAL_NAME (function),
			 decl, TREE_CHAIN (TYPE_FN_FIELDS (type)));
	}
      else
	{
	  TYPE_FN_FIELDS (type) = tree_cons (DECL_ORIGINAL_NAME (function),
					     decl, TYPE_FN_FIELDS (type));
	}
    }
}

/* Generate a C++ "new" expression. DECL is either a TREE_LIST
   (which needs to go through some sort of groktypename) or it
   is the name of the class we are newing. INIT is an initialization value.
   It is either an EXPRLIST, an EXPR_NO_COMMAS, or something in braces.
   If INIT is void_type_node, it means do *not* call a constructor
   for this instance.

   For types with constructors, the data returned is initialized
   by the approriate constructor.

   Whether the type has a constructor or not, if it has a pointer
   to a virtual function table, then that pointer is set up
   here.

   Unless I am mistaken, a call to new () will return initialized
   data regardless of whether the constructor itself is private or
   not.

   Parameter USER_PARMS, if non-null, is passed as a second parameter
   to a function called "__user_new".  This function should behave in
   a similar fashion to "__builtin_new", except that it may allocate
   storage in a more user-defined way.  */

tree
build_x_new (decl, init)
     tree decl;
{
  if (TREE_GETS_NEW (current_class_type))
    return build_opfncall (NEW_EXPR, TREE_TYPE (current_class_decl),
			   size_in_bytes (current_class_type));
  return build_new (NULL_TREE, decl, void_type_node);
}

tree
build_new (user_parms, decl, init)
     tree user_parms;
     tree decl, init;
{
  extern tree require_complete_type ();	/* typecheck.c */
  extern tree ptr_ftype_int;
  tree type, size, rval, build_int_2 ();
  tree init1 = NULL_TREE;
  int has_call = 0, has_array = 0, arr_size = 1;
  int bytes;

  if (decl == error_mark_node)
    return error_mark_node;

  if (TREE_CODE (decl) == TREE_LIST)
    {
      tree typespecs = TREE_PURPOSE (decl);
      tree absdcl = TREE_VALUE (decl);
      tree last_absdcl = NULL_TREE;
      tree nelts = integer_one_node;

      if (absdcl && TREE_CODE (absdcl) == CALL_EXPR)
	{
	  /* probably meant to be a call */
	  has_call = 1;
	  init1 = TREE_OPERAND (absdcl, 1);
	  absdcl = TREE_OPERAND (absdcl, 0);
	  TREE_VALUE (decl) = absdcl;
	}
      while (absdcl && TREE_CODE (absdcl) == INDIRECT_REF)
	{
	  last_absdcl = absdcl;
	  absdcl = TREE_OPERAND (absdcl, 0);
	}
      while (absdcl && TREE_CODE (absdcl) == ARRAY_REF)
	{
	  /* probably meant to be a vec new */
	  tree this_nelts;

	  has_array = 1;
	  this_nelts = TREE_OPERAND (absdcl, 1);
	  absdcl = TREE_OPERAND (absdcl, 0);
	  if (this_nelts == NULL_TREE)
	    {
	      error ("new of array type fails to specify size");
	    }
	  else if (this_nelts == integer_zero_node)
	    {
	      warning ("zero size array reserves no space");
	      nelts = integer_zero_node;
	    }
	  else
	    {
	      nelts = build_binary_op (MULT_EXPR, nelts, this_nelts);
	    }
	}

      if (last_absdcl)
	TREE_OPERAND (last_absdcl, 0) = absdcl;
      else
	TREE_VALUE (decl) = absdcl;

      type = groktypename (decl);
      if (! type || type == error_mark_node)
	return error_mark_node;

      type = TYPE_MAIN_VARIANT (type);

      if (TYPE_SIZE (type) == 0)
	{
	  incomplete_type_error (0, type);
	  return error_mark_node;
	}

      size = size_in_bytes (type);
      if (has_array)
	size = fold (build_binary_op (MULT_EXPR, size, nelts));

      if (has_call)
	init = init1;

      if (user_parms)
	{
	  rval = build_user_new (type, size, user_parms);
	  if (rval == error_mark_node)
	    return error_mark_node;
	}
      else rval = NULL_TREE;

      if (has_array == 0 && TREE_HAS_CONSTRUCTOR (type))
	return build_classfn_ref (rval, DECL_NAME (TYPE_NAME (type)), init, 0, 1);
      else
	{
	  if (! user_parms)
	    {
	      if (has_array == 0 && TREE_GETS_NEW (type))
		rval = build_opfncall (NEW_EXPR, TYPE_POINTER_TO (type), size);
	      else
		rval = build (CALL_EXPR, build_pointer_type (type),
			      BIN, build_tree_list (NULL_TREE, size), 0);
	    }
	  if (TREE_NEEDS_CONSTRUCTOR (type))
	    {
	      rval = save_expr (rval);
	      if (has_array)
		do_vec_init (decl, rval,
			     build_binary_op (MINUS_EXPR, nelts, integer_one_node),
			     init);
	      else
		do_aggr_init (build_indirect_ref (rval, "member initialization"), init);
	    }
	  else if (has_call)
	    {
	      error ("no constructor for this type");
	      rval = error_mark_node;
	    }
	}
      return rval;
    }

  if (TREE_CODE (decl) == IDENTIFIER_NODE)
    {
      decl = TREE_TYPE (decl);
      type = TREE_TYPE (decl);
    }
  else if (TREE_CODE (decl) == TYPE_DECL)
    type = TREE_TYPE (decl);
  else
    {
      type = decl;
      decl = TYPE_NAME (type);
    }
      
  decl = require_complete_type (decl);

  if (decl == error_mark_node)
    return error_mark_node;

  size = size_in_bytes (type);

  if (user_parms)
    {
      rval = build_user_new (type, size, user_parms);
      if (rval == error_mark_node)
	return error_mark_node;
    }
  else rval = NULL_TREE;

  if (TREE_HAS_CONSTRUCTOR (type) && init != void_type_node)
    {
      if (init == NULL_TREE || TREE_CODE (init) == TREE_LIST)
	return build_classfn_ref (rval, DECL_NAME (decl), init, 0, 1);
      else
	{
	  error ("constructors take parameter lists");
	  return error_mark_node;
	}
    }

  if (user_parms)
    {
      rval = build_user_new (type, size, user_parms);
      if (rval == error_mark_node)
	return error_mark_node;
    }
  else
    {
      if (TREE_GETS_NEW (type))
	rval = build_opfncall (NEW_EXPR, TYPE_POINTER_TO (type), size);
      else
	rval = build (CALL_EXPR, build_pointer_type (type),
		      BIN, build_tree_list (NULL_TREE, size), 0);
    }

  if (init != void_type_node)
    {
      if (init && TREE_CODE (init) == TREE_LIST)
	{
	  error ("type `%s' must have constructor to take parameter list",
		 TYPE_NAME_STRING (type));
	  return error_mark_node;
	}
      if (TREE_NEEDS_CONSTRUCTOR (type))
	{
	  rval = save_expr (rval);
	  do_aggr_init (build_indirect_ref (rval, "member initialization"), init);
	}
      else if (init)
	rval = build_modify_expr (build_indirect_ref (rval, "member initialization"), NOP_EXPR, init);
    }
  return rval;
}

static tree
build_user_new (type, size, user_parms)
     tree type;
     tree size;
     tree user_parms;
{
  tree user_new = lookup_name (USR_NEW);
  tree rval;

  if (user_new == NULL_TREE)
    {
      error ("no declaration of \"__user_new\"");
      return error_mark_node;
    }
  rval = build_x_function_call (user_new, tree_cons (NULL_TREE, size, user_parms));
  if (rval == error_mark_node)
    return error_mark_node;
  TREE_TYPE (rval) = build_pointer_type (type);
  return rval;
}

/* `do_vec_init' performs initialization of a vector of aggregate
   types.

   DECL is passed only for error reporting, and provides line number
   and source file name information.
   BASE is the space where the vector will be.
   NELTS is the number of elements in the vector.
   INIT is the (possibly NULL) initializer.  */

static tree
do_vec_init (decl, base, maxindex, init)
     tree decl, base, maxindex, init;
{
  tree rval;
  tree iterator;
  tree type = TREE_TYPE (TREE_TYPE (base));

  maxindex = convert (integer_type_node, maxindex);
  if (maxindex == error_mark_node)
    return error_mark_node;

  iterator = get_temp_regvar (integer_type_node, maxindex);

  base = get_temp_regvar (TYPE_POINTER_TO (type), default_conversion (base));
  expand_start_loop_continue_elsewhere (1);
  do_aggr_init (build (INDIRECT_REF, type, base), init);
  expand_assignment (base,
		     build (PLUS_EXPR, TYPE_POINTER_TO (type),
			    base, size_in_bytes (type)), 0, 0);
  expand_loop_continue_here ();
  expand_exit_loop_if_false (build (NE_EXPR, integer_type_node,
				    build (PREDECREMENT_EXPR, integer_type_node, iterator, integer_one_node), minus_one));
  expand_end_loop ();
}

tree
build_x_delete ()
{
  if (TREE_GETS_DELETE (current_class_type))
    return build_opfncall (DELETE_EXPR, current_class_decl, current_class_decl);
  return build_function_call (BID, build_tree_list (NULL_TREE, current_class_decl));
}

void
expand_delete (type, addr, auto_delete)
     tree type, addr;
     int auto_delete;
{
  extern struct rtx_def *const0_rtx;
  tree rval;

  if (TREE_CODE (type) == ARRAY_TYPE)
    {
      expand_vec_delete (TYPE_NELTS (type), addr, auto_delete);
      return;
    }

  rval = build_delete (type, addr, auto_delete);
  if (rval == error_mark_node)
    return;
  expand_expr_stmt (rval);
}

/* Generate a call to a destructor. TYPE is the type to cast ADDR to.
   ADDR is an expression which yields the store to be destroyed.
   AUTO_DELETE is nonzero if a call to DELETE should be made or not.  */
tree
build_delete (type, addr, auto_delete)
     tree type, addr;
     tree auto_delete;
{
  tree function, fntype;
  tree name, parms;
  tree member;
  tree expr, exprstmt = NULL_TREE;
  tree ref;
  int ptr;

  if (type == error_mark_node)
    return type;

  type = TYPE_MAIN_VARIANT (type);

  if (TREE_CODE (type) == POINTER_TYPE)
    {
      type = TREE_TYPE (type);
      if (! IS_AGGR_TYPE (type))
	{
	  if (auto_delete == integer_zero_node)
	    error ("non-aggregate type to build delete with auto_delete == 0 (compiler error)");

	  return build (CALL_EXPR, void_type_node,
			BID, build_tree_list (NULL_TREE, addr), 0);
	}
      ref = build_indirect_ref (addr, "member destruction");
      ptr = 1;
    }
  else if (TREE_CODE (type) == ARRAY_TYPE)
    {
      return build_vec_delete (TYPE_NELTS (type), addr, auto_delete, integer_zero_node);
    }
  else
    {
      ref = addr;
      addr = build (ADDR_EXPR, build_pointer_type (type), addr);
      ptr = 0;
    }

  if (! IS_AGGR_TYPE (type))
    {
      error ("non-aggregate type to build_delete (compiler error)");
      return error_mark_node;
    }

  parms = build_tree_list (NULL_TREE, addr);

  if (! TREE_NEEDS_DESTRUCTOR (type))
    if (auto_delete == integer_zero_node)
      return build (NOP_EXPR, void_type_node, integer_zero_node);
    else
      return build (CALL_EXPR, void_type_node, BID, parms, 0);

  /* Below, we will reverse the order in which these calls are made.
     If we have a destructor, then that destructor will take care
     of the base class; otherwise, we must do that here.  */
  if (TREE_HAS_DESTRUCTOR (type))
    {
      tree fields = TYPE_FN_FIELDS (type);
      tree field = TREE_VALUE (fields);

      /* Once we are in a destructor, do not try going through
	 the virtual function table to find the next destructor.  */
      if (TREE_VIRTUAL (field)
	  && auto_delete != integer_zero_node
	  && (ptr == 1 || ! resolves_to_fixed_type_p (addr)))
	{
	  tree vtbl = build_indirect_ref (build_component_ref (ref, get_vfield_name (type), 0), "member destruction");
	  function = build_array_ref (vtbl, DECL_VINDEX (field));
	  if (function == error_mark_node)
	    return error_mark_node;
	  TREE_TYPE (function) = build_pointer_type (TREE_TYPE (TREE_TYPE (field)));
	  TREE_CHAIN (parms) = build_tree_list (NULL_TREE, auto_delete);
	  expr = build_function_call (function, parms);
	  if (ptr)
	    {
	      /* Handle the case where a virtual destructor is
		 being called on an item that is 0.

		 @@ Does this really need to be done?  */
	      tree ifexp = build_binary_op (NE_EXPR, addr, integer_zero_node);
	      expr = build (COND_EXPR, void_type_node,
			    ifexp, expr,
			    build (NOP_EXPR, void_type_node, integer_zero_node));
	    }
	}
      else
	{
	  function = TREE_TYPE (field);
	  if (DECL_INITIAL (function) == void_type_node)
	    if (auto_delete == integer_zero_node)
	      expr = build (NOP_EXPR, void_type_node, integer_zero_node);
	    else
	      expr = build (CALL_EXPR, void_type_node, BID, parms, 0);
	  else
	    {
	      TREE_CHAIN (parms) = build_tree_list (NULL_TREE, auto_delete);
	      expr = build_function_call (function, parms);
	    }
	}
      return expr;
    }
  else
    {
      tree basetype = TYPE_BASELINK (type);
      if (basetype && TREE_NEEDS_DESTRUCTOR (basetype))
	{
	  /* Change suggested by Peter Moore.  */
	  expr = build_delete (build_pointer_type (basetype), addr, auto_delete);
	  if (expr != error_mark_node)
	    exprstmt = build_tree_list (NULL_TREE, expr);
	}

      for (member = TYPE_FIELDS (type); member; member = TREE_CHAIN (member))
	{
	  if (TREE_STATIC (member))
	    continue;
	  if (TREE_NEEDS_DESTRUCTOR (TREE_TYPE (member)))
	    {
	      expr = build_delete (TREE_TYPE (member),
				   build_component_ref (ref, DECL_NAME (member), 0),
				   integer_zero_node);
	      if (expr != error_mark_node)
		exprstmt = tree_cons (NULL_TREE, expr, exprstmt);
	    }
	}
      if (exprstmt)
	return build_compound_expr (exprstmt);
      else
	{
	  error ("build delete does nothing (compiler error)");
	  return error_mark_node;
	}
    }
}

/* Expand a C++ vector delete expression.
   NELTS is the number of elements to be deleted.
   BASE is the expression that should yield the store to be deleted.  */
void
expand_vec_delete (maxindex, base, auto_delete)
     tree maxindex, base;
     tree auto_delete;
{
  tree ptype = TREE_TYPE (base);
  tree type;
  tree rval;
  tree iterator;
  tree size_exp;

  if (TREE_CODE (ptype) == POINTER_TYPE)
    {
      if (maxindex == 0)
	{
	  error ("must specify size for non array type");
	  return;
	}
      warning ("vector delete applied to non-array type");
      maxindex = convert (integer_type_node, maxindex);
      if (maxindex == error_mark_node)
	return;
    }
  else if (TREE_CODE (ptype) == ARRAY_TYPE)
    {
      tree amaxindex = TYPE_NELTS (ptype);

      maxindex = convert (integer_type_node, maxindex);
      if (maxindex == error_mark_node)
	return;

      maxindex = fold (maxindex);
      if (amaxindex != 0
	  && (TREE_CODE (maxindex) == INTEGER_CST || TREE_CODE (amaxindex) == INTEGER_CST)
	  && ! tree_int_cst_equal (maxindex, amaxindex))
	warning ("argument to vector delete disagrees with declared type of array");
      base = default_conversion (base);
      ptype = TREE_TYPE (base);
    }
  else
    {
      error ("type to vector delete is neither pointer or array type");
      return;
    }
  type = TREE_TYPE (ptype);

  if (! IS_AGGR_TYPE (type) || ! TREE_NEEDS_DESTRUCTOR (type))
    {
      if (auto_delete != integer_zero_node)
	expand_expr_stmt (build (CALL_EXPR, void_type_node,
				 BID, build_tree_list (NULL_TREE, base), 0));
      return;
    }

  iterator = get_temp_regvar (integer_type_node, maxindex);
  size_exp = size_in_bytes (type);

  base = get_temp_regvar (ptype,
			  build (PLUS_EXPR, ptype, base,
				 build (MULT_EXPR, integer_type_node, size_exp,
					build (PLUS_EXPR, integer_type_node, maxindex, integer_one_node))));
  expand_start_loop_continue_elsewhere (1);
  expand_assignment (base, build (MINUS_EXPR, ptype, base, size_exp), 0, 0);
  expand_delete (ptype, base, auto_delete);
  expand_loop_continue_here ();
  expand_exit_loop_if_false (build (NE_EXPR, integer_type_node,
				    build (PREDECREMENT_EXPR, integer_type_node,
					   iterator, integer_one_node),
				    minus_one));
  expand_end_loop ();
  if (auto_delete != integer_zero_node)
    expand_expr_stmt (build (CALL_EXPR, void_type_node,
			     BID, build_tree_list (NULL_TREE, base), 0));
}

/* Build a C++ vector delete expression.
   NELTS is the number of elements to be deleted.
   BASE is the expression that should yield the store to be deleted.

   This generates a call to the function BIVD, which passes the
   destructor across the vector.  This is needed only when
   expanding such an action is unacceptable.  */
tree
build_vec_delete (maxindex, base, auto_delete_vec, auto_delete)
     tree maxindex, base;
     tree auto_delete_vec, auto_delete;
{
  tree ptype = TREE_TYPE (base);
  tree type;

  maxindex = fold (convert (integer_type_node, maxindex));

  if (maxindex == error_mark_node)
    return;

  if (TREE_CODE (ptype) == POINTER_TYPE)
    {
      if (maxindex == 0)
	{
	  error ("must specify size for non array type");
	  return;
	}
      warning ("vector delete applied to non-array type");
    }
  else if (TREE_CODE (ptype) == ARRAY_TYPE)
    {
      tree amaxindex = TYPE_NELTS (ptype);

      if ((TREE_CODE (maxindex) == INTEGER_CST
	   || (amaxindex && TREE_CODE (amaxindex) == INTEGER_CST))
	  && ! tree_int_cst_equal (maxindex, amaxindex))
	warning ("argument to vector delete disagrees with declared type of array");
      base = default_conversion (base);
      ptype = TREE_TYPE (base);
    }
  else
    {
      error ("type to vector delete is neither pointer or array type");
      return;
    }
  type = TREE_TYPE (ptype);

  if (TREE_HAS_DESTRUCTOR (type))
    {
      tree parms;
      tree dtor = TREE_TYPE (TREE_VALUE (TYPE_FN_FIELDS (type)));

      parms = tree_cons (NULL_TREE, base,
		 tree_cons (NULL_TREE, maxindex,
		    tree_cons (NULL_TREE, c_sizeof (type)),
		       tree_cons (NULL_TREE, default_conversion (dtor),
			  tree_cons (NULL_TREE, auto_delete_vec,
			     build_tree_list (NULL_TREE, auto_delete)))));

      return build (CALL_EXPR, void_type_node, BIVD, parms, 0);
    }
  if (auto_delete_vec != integer_zero_node)
    return build (CALL_EXPR, void_type_node,
		  BID, build_tree_list (NULL_TREE, base));
  return build (NOP_EXPR, void_type_node, integer_zero_node);
}
