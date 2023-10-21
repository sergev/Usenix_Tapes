/* Process declarations and variables for C compiler.
   Copyright (C) 1988 Free Software Foundation, Inc.
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


/* Process declarations and symbol lookup for C front end.
   Also constructs types; the standard scalar types at initialization,
   and structure, union, array and enum types when they are declared.  */

/* ??? not all decl nodes are given the most useful possible
   line numbers.  For example, the CONST_DECLs for enum values.  */

#include "config.h"
#include "tree.h"
#include "flags.h"
#include "c-tree.h"
#include "overload.h"
#include "parse.h"

/* In grokdeclarator, distinguish syntactic contexts of declarators.  */
enum decl_context
{ NORMAL,			/* Ordinary declaration */
  FUNCDEF,			/* Function definition */
  PARM,				/* Declaration of parm before function body */
  FIELD,			/* Declaration inside struct or union */
  TYPENAME};			/* Typename (inside cast or sizeof)  */

#define NULL 0
#define min(X,Y) ((X) < (Y) ? (X) : (Y))

static tree grokparms (), grokdeclarator ();
tree pushdecl ();
tree make_index_type ();
static void builtin_function ();
/* static */ void grokclassfn ();
/* static */ tree grokopexpr (), grokoptypename ();

static tree lookup_tag ();
static tree lookup_tag_reverse ();
static tree lookup_name_current_level ();
static char *redeclaration_error_message ();
static int parmlist_is_exprlist ();

/* a node which has tree code ERROR_MARK, and whose type is itself.
   All erroneous expressions are replaced with this node.  All functions
   that accept nodes as arguments should avoid generating error messages
   if this node is one of the arguments, since it is undesirable to get
   multiple error messages from one error in the input.  */

tree error_mark_node;

/* Erroneous argument lists can use this *IFF* they do not modify it.  */
tree error_mark_list;

/* INTEGER_TYPE and REAL_TYPE nodes for the standard data types */

tree short_integer_type_node;
tree integer_type_node;
tree long_integer_type_node;

tree short_unsigned_type_node;
tree unsigned_type_node;
tree long_unsigned_type_node;

tree unsigned_char_type_node;
tree signed_char_type_node;
tree char_type_node;

tree float_type_node;
tree double_type_node;
tree long_double_type_node;

/* a VOID_TYPE node.  */

tree void_type_node;

/* A node for type `void *'.  */

tree ptr_type_node;

/* A node for type `char *'.  */

tree string_type_node;

/* Type `char[256]' or something like it.
   Used when an array of char is needed and the size is irrelevant.  */

tree char_array_type_node;

/* Type `int[256]' or something like it.
   Used when an array of int needed and the size is irrelevant.  */

tree int_array_type_node;

/* type `int ()' -- used for implicit declaration of functions.  */

tree default_function_type;

/* function types `double (double)' and `double (double, double)', etc.  */

tree double_ftype_double, double_ftype_double_double;
tree int_ftype_int, long_ftype_long;

/* Function type `void (void *, void *, int)' and similar ones.  */

tree void_ftype_ptr_ptr_int, int_ftype_ptr_ptr_int, void_ftype_ptr_int_int;

/* C++ extensions */
tree class_type_node, record_type_node, union_type_node, enum_type_node;
tree unknown_type_node;

/* Function type `void * (int)', 'void (void *)' */
tree ptr_ftype_int, void_ftype_ptr;
tree ptr_ftype_ptr_int_int_ptr, void_ftype_ptr_int_int_ptr_int_int;

/* Used for virtual function tables.  */
tree vtbl_mask;

/* Array type `(void *)[]' */
tree vtbl_type_node;

/* Used to help generate temporary names which are unique within
   a function.  Reset to 0 by start_function.  */

static int temp_name_counter = 0;

/* -- end of C++ */

/* Two expressions that are constants with value zero.
   The first is of type `int', the second of type `void *'.  */

tree integer_zero_node;
tree null_pointer_node;

/* A node for the integer constant 1.  */

tree integer_one_node;

/* An identifier whose name is <value>.  This is used as the "name"
   of the RESULT_DECLs for values of functions.  */

tree value_identifier;

/* While defining an enum type, this is 1 plus the last enumerator
   constant value.  */

static tree enum_next_value;

/* Parsing a function declarator leaves a list of parameter names
   or a chain or parameter decls here.  */

static tree last_function_parms;

/* Parsing a function declarator leaves here a chain of structure
   and enum types declared in the parmlist.  */

static tree last_function_parm_tags;

/* After parsing the declarator that starts a function definition,
   `start_function' puts here the list of parameter names or chain of decls.
   `store_parm_decls' finds it here.  */

static tree current_function_parms;

/* Similar, for last_function_parm_tags.  */
static tree current_function_parm_tags;

/* A list (chain of TREE_LIST nodes) of all LABEL_STMTs in the function
   that have names.  Here so we can clear out their names' definitions
   at the end of the function.  */

static tree named_labels;

/* The FUNCTION_DECL for the function currently being compiled,
   or 0 if between functions.  */
tree current_function_decl, current_function_name;

/* Set to 0 at beginning of a function definition, set to 1 if
   a return statement that specifies a return value is seen.  */

int current_function_returns_value;

/* Set to 0 at beginning of a function definition, set to 1 if
   a return statement with no argument is seen.  */

int current_function_returns_null;

/* Set to nonzero by `grokdeclarator' for a function
   whose return type is defaulted, if warnings for this are desired.  */

static int warn_about_return_type;

/* Set to 0 at beginning of a constructor, set to 1
   if that function does an allocation before referencing its
   instance variable.  */
int current_function_assigns_this;
int current_function_just_assigned_this;

/* For each binding contour we allocate a binding_level structure
 * which records the names defined in that contour.
 * Contours include:
 *  0) the global one
 *  1) one for each function definition,
 *     where internal declarations of the parameters appear.
 *  2) one for each compound statement,
 *     to record its declarations.
 *
 * The current meaning of a name can be found by searching the levels from
 * the current one out to the global one.
 */

/* Note that the information in the `names' component of the global contour
   is duplicated in the IDENTIFIER_GLOBAL_VALUEs of all identifiers.  */

struct binding_level
  {
    /* A chain of _DECL nodes for all variables, constants, functions,
       and typedef types.  These are in the reverse of the order supplied.
     */
    tree names;

    /* A list of structure, union and enum definitions,
     * for looking up tag names.
     * It is a chain of TREE_LIST nodes, each of whose TREE_PURPOSE is a name,
     * or NULL_TREE; and whose TREE_VALUE is a RECORD_TYPE, UNION_TYPE,
     * or ENUMERAL_TYPE node.
     *
     * C++: the TREE_VALUE nodes can be simple types for component_bindings.
     *
     */
    tree tags;

    /* For each level, a list of shadowed outer-level local definitions
       to be restored when this level is popped.
       Each link is a TREE_LIST whose TREE_PURPOSE is an identifier and
       whose TREE_VALUE is its old definition (a kind of ..._DECL node).  */
    tree shadowed;

    /* For each level (except not the global one),
       a chain of LET_STMT nodes for all the levels
       that were entered and exited one level down.  */
    tree blocks;

    /* The binding level which this one is contained in (inherits from).  */
    struct binding_level *level_chain;

    /* Nonzero for the level that holds the parameters of a function.  */
    char parm_flag;

    /* Nonzero if this level "doesn't exist" for tags.  */
    char tag_transparent;

    /* Number of decls in `names' that have incomplete 
       structure or union types.  */
    int n_incomplete;
  };

#define NULL_BINDING_LEVEL (struct binding_level *) NULL
  
/* The binding level currently in effect.  */

static struct binding_level *current_binding_level;

/* A chain of binding_level structures awaiting reuse.  */

static struct binding_level *free_binding_level;

/* The outermost binding level, for names of file scope.
   This is created when the compiler is started and exists
   through the entire run.  */

static struct binding_level *global_binding_level;

/* Binding level structures are initialized by copying this one.  */

static struct binding_level clear_binding_level
  = {NULL, NULL, NULL, NULL, NULL, 0, 0, 0};

/* Create a new `struct binding_level'.  */

static
struct binding_level *
make_binding_level ()
{
  /* NOSTRICT */
  return (struct binding_level *) xmalloc (sizeof (struct binding_level));
}

/* Enter a new binding level.
   If TAG_TRANSPARENT is nonzero, do so only for the name space of variables,
   not for that of tags.  */

void
pushlevel (tag_transparent)
     int tag_transparent;
{
  register struct binding_level *newlevel = NULL_BINDING_LEVEL;

  /* If this is the top level of a function,
     just make sure that NAMED_LABELS is 0.
     They should have been set to 0 at the end of the previous function.  */

  if (current_binding_level == global_binding_level)
    {
      if (named_labels)
	abort ();
    }

  /* Reuse or create a struct for this binding level.  */

  if (free_binding_level)
    {
      newlevel = free_binding_level;
      free_binding_level = free_binding_level->level_chain;
    }
  else
    {
      newlevel = make_binding_level ();
    }

  /* Add this level to the front of the chain (stack) of levels that
     are active.  */

  *newlevel = clear_binding_level;
  newlevel->level_chain = current_binding_level;
  current_binding_level = newlevel;
  newlevel->tag_transparent = tag_transparent;
}

/* Exit a binding level.
   Pop the level off, and restore the state of the identifier-decl mappings
   that were in effect when this level was entered.

   If KEEP is nonzero, this level had explicit declarations, so
   and create a "block" (a LET_STMT node) for the level
   to record its declarations and subblocks for symbol table output.

   If FUNCTIONBODY is nonzero, this level is the body of a function,
   so create a block as if KEEP were set and also clear out all
   label names.

   If REVERSE is nonzero, reverse the order of decls before putting
   them into the LET_STMT.  */

void
poplevel (keep, reverse, functionbody)
     int keep;
     int reverse;
     int functionbody;
{
  register tree link;
  /* The chain of decls was accumulated in reverse order.
     Put it into forward order, just for cleanliness.  */
  tree decls;
  tree tags = current_binding_level->tags;
  tree subblocks = current_binding_level->blocks;
  tree block = 0;

#if 0
  /* Warn about incomplete structure types in this level.  */
  for (link = tags; link; link = TREE_CHAIN (link))
    if (TYPE_SIZE (TREE_VALUE (link)) == 0)
      {
	tree type = TREE_VALUE (link);
	char *errmsg;
	switch (TREE_CODE (type))
	  {
	  case RECORD_TYPE:
	    errmsg = "`struct %s' incomplete in scope ending here";
	    break;
	  case UNION_TYPE:
	    errmsg = "`union %s' incomplete in scope ending here";
	    break;
	  case ENUMERAL_TYPE:
	    errmsg = "`enum %s' incomplete in scope ending here";
	    break;
	  }
	if (TREE_CODE (TYPE_NAME (type)) == IDENTIFIER_NODE)
	  error (errmsg, IDENTIFIER_POINTER (TYPE_NAME (type)));
	else
	  /* If this type has a typedef-name, the TYPE_NAME is a TYPE_DECL.  */
	  error (errmsg, IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (type))));
      }
#endif

  /* Get the decls in the order they were written.
     Usually current_binding_level->names is in reverse order.
     But parameter decls were previously put in forward order.  */

  if (reverse)
    current_binding_level->names
      = decls = nreverse (current_binding_level->names);
  else
    decls = current_binding_level->names;

  /* If there were any declarations or structure tags in that level,
     or if this level is a function body,
     create a LET_STMT to record them for the life of this function.  */

  if (keep || functionbody)
    block = build_let (0, 0, keep ? decls : 0,
		       subblocks, 0, keep ? tags : 0);

  /* In each subblock, record that this is its superior.  */

  for (link = subblocks; link; link = TREE_CHAIN (link))
    STMT_SUPERCONTEXT (link) = block;

  /* Clear out the meanings of the local variables of this level;
     also record in each decl which block it belongs to.  */

  for (link = decls; link; link = TREE_CHAIN (link))
    {
      if (DECL_NAME (link) != 0)
	{
	  if (keep && ! TREE_EVERUSED (link))
	    {
	      if (TREE_CODE (link) == VAR_DECL)
		warning ("variable `%s' never used",
			 IDENTIFIER_POINTER (DECL_NAME (link)));
	      else if (TREE_CODE (link) == PARM_DECL)
		{
		  if (THIS_NAME_P (DECL_NAME (link)))
		    {
#ifdef WARNING_ABOUT_CCD
		      warning ("class instance variable unused");
#endif
		    }
		  else
		    if (! ANON_PARMNAME_P (DECL_NAME (link)))
		      warning ("parameter `%s' never used",
			       IDENTIFIER_POINTER (DECL_NAME (link)));
		}
	    }
	  IDENTIFIER_LOCAL_VALUE (DECL_NAME (link)) = 0;
	}
      DECL_CONTEXT (link) = block;
    }

  /* Restore all name-meanings of the outer levels
     that were shadowed by this level.  */

  for (link = current_binding_level->shadowed; link; link = TREE_CHAIN (link))
    IDENTIFIER_LOCAL_VALUE (TREE_PURPOSE (link)) = TREE_VALUE (link);

  /* If the level being exited is the top level of a function,
     check over all the labels.  */

  if (functionbody)
    {
      /* Clear out the definitions of all label names,
	 since their scopes end here.  */

      for (link = named_labels; link; link = TREE_CHAIN (link))
	{
	  if (DECL_SOURCE_LINE (TREE_VALUE (link)) == 0)
	    {
	      error ("label `%s' used somewhere above but not defined",
		     IDENTIFIER_POINTER (DECL_NAME (TREE_VALUE (link))));
	      /* Avoid crashing later.  */
	      define_label (input_filename, 1, DECL_NAME (TREE_VALUE (link)));
	    }
	  IDENTIFIER_LABEL_VALUE (DECL_NAME (TREE_VALUE (link))) = 0;
	}

      named_labels = 0;
    }

  /* Pop the current level, and free the structure for reuse.  */

  {
    register struct binding_level *level = current_binding_level;
    current_binding_level = current_binding_level->level_chain;

    level->level_chain = free_binding_level;
    free_binding_level = level;
  }

  if (functionbody)
    {
      DECL_INITIAL (current_function_decl) = block;
      /* If this is the top level block of a function,
	 the vars are the function's parameters.
	 Don't leave them in the LET_STMT because they are
	 found in the FUNCTION_DECL instead.  */
      STMT_VARS (block) = 0;
    }
  else if (block)
    current_binding_level->blocks
      = chainon (current_binding_level->blocks, block);
  /* If we did not make a block for the level just exited,
     any blocks made for inner levels
     (since they cannot be recorded as subblocks in that level)
     must be carried forward so they will later become subblocks
     of something else.  */
  else if (subblocks)
    current_binding_level->blocks
      = chainon (current_binding_level->blocks, subblocks);
    
}

/* Push a definition of struct, union or enum tag "name".
   "type" should be the type node.
   We assume that the tag "name" is not already defined.

   Note that the definition may really be just a forward reference.
   In that case, the TYPE_SIZE will be zero.

   C++ gratuitously puts all these tags in the name space. */

void
pushtag (name, type)
     tree name, type;
{
  register struct binding_level *b = current_binding_level;
  while (b->tag_transparent) b = b->level_chain;

  if (name)
    {
      /* Record the identifier as the type's name if it has none.  */

      if (TYPE_NAME (type) == 0)
	TYPE_NAME (type) = name;

      b->tags = tree_cons (name, type, b->tags);

      /* do C++ gratuitous typedefing.  Note that we put the
	 TYPE_DECL in the TREE_TYPE of the IDENTIFIER_NODE.  */
      if (TREE_TYPE (name) == NULL_TREE
	  || TREE_TYPE (name) != TYPE_NAME (type))
	{
	  register tree t = pushdecl (build_decl (TYPE_DECL, name, type));
	  if (TREE_TYPE (name))
	    error ("tag `%s' redefined",
		   IDENTIFIER_POINTER (name));
	  TYPE_NAME (type) = t;
	  TREE_TYPE (name) = t;
	}
    }
}

/* This routine is called from the last rule in yyparse ().
   Its job is to create all the code needed to initialize and
   destroy the global aggregates.  We do the destruction
   first, since that way we only need to reverse the decls once.  */

void
finish_file ()
{
  char buf[OVERLOAD_MAX_LEN], *p;
  tree fnname;
  tree vars = getaggrs ();
  tree void_list = build_tree_list (NULL_TREE, void_type_node);
  int needs_cleaning = 0, needs_messing_up = 0;

  if (current_binding_level != global_binding_level)
    abort ();

  if (vars == NULL_TREE)
    return;

  /* See if we really need the hassle.  */
  while (vars && needs_cleaning == 0)
    {
      tree decl = TREE_PURPOSE (vars);
      tree type = TREE_TYPE (decl);
      if (TREE_NEEDS_DESTRUCTOR (type))
	{
	  needs_cleaning = 1;
	  needs_messing_up = 1;
	  break;
	}
      else
	needs_messing_up += TREE_NEEDS_CONSTRUCTOR (type);
      vars = TREE_CHAIN (vars);
    }

  sprintf (buf, "_GLOBAL_$D$%s", input_filename);

  for (p = buf+11; *p; p++)
    if (! ((*p >= '0' && *p <= '9')
	   || (*p >= 'A' && *p <= 'Z')
	   || (*p >= 'a' && *p < 'z')))
      *p = '_';

  if (needs_cleaning == 0)
    goto mess_up;

  /* Otherwise, GDB can get confused, because in only knows
     about source for LINENO-1 lines.  */
  lineno -= 1;

  fnname = get_identifier (buf);
  start_function (void_list, build_nt (CALL_EXPR, fnname, void_list), 0);
  store_parm_decls ();
  pushlevel ();
  clear_last_expr ();
  push_momentary ();
  expand_start_bindings (0);

  /* These must be done in backward order to destroy,
     in which they happen to be!  */
  while (vars)
    {
      tree decl = TREE_PURPOSE (vars);
      tree type = TREE_TYPE (decl);

      if (TREE_NEEDS_DESTRUCTOR (type))
	expand_delete (type, decl, integer_zero_node);
      vars = TREE_CHAIN (vars);
    }

  expand_end_bindings (1, 1);
  poplevel (1, 0, 1);
  pop_momentary ();
  finish_function ("destruction phase", 1);
  /* if it needed cleaning, then it will need messing up: drop through  */

mess_up:
  /* Must do this while we think we are at the top level.  */
  vars = nreverse (getaggrs ());
  buf[9] = 'I';
  fnname = get_identifier (buf);
  start_function (void_list, build_nt (CALL_EXPR, fnname, void_list), 0);
  store_parm_decls ();
  pushlevel (0);
  clear_last_expr ();
  push_momentary ();
  expand_start_bindings (0);

  while (vars)
    {
      tree decl = TREE_PURPOSE (vars);
      tree init = TREE_VALUE (vars);

      if (TREE_NEEDS_CONSTRUCTOR (TREE_TYPE (decl)))
	{
	  if (init)
	    {
	      if (TREE_CODE (init) == VAR_DECL)
		{
		  /* This behavior results when there are
		     multiple declarations of an aggregate,
		     the last of which defines it.  */
		  if (DECL_RTL (init) == DECL_RTL (decl))
		    {
		      if (DECL_INITIAL (decl) == error_mark_node)
			init = DECL_INITIAL (init);
		      else
			abort ();
		    }
		  else if (TREE_TYPE (decl) == TREE_TYPE (init))
		    {
#if 0
		      abort ();
#else
		      /* point to real decl's rtl anyway.  */
		      DECL_RTL (init) = DECL_RTL (decl);
		      if (DECL_INITIAL (decl) == error_mark_node)
			init = DECL_INITIAL (init);
		      else
			abort ();
#endif
		    }
		}
	    }
	  /* Set these global variables so that GDB at least puts
	     us near the declaration which required the initialization.  */
	  input_filename = DECL_SOURCE_FILE (decl);
	  lineno = DECL_SOURCE_LINE (decl);
	  emit_note (input_filename, lineno);
	  do_aggr_init (decl, init);
	}
      vars = TREE_CHAIN (vars);
    }

  expand_end_bindings (0, 1);
  poplevel (1, 0, 1);
  pop_momentary ();
  finish_function ("initialization phase", 1);
}

/* Handle when a new declaration NEW has the same name as an old one OLD
   in the same binding contour.  Prints an error message if appropriate.

   If safely possible, alter OLD to look like NEW, and return 1.
   Otherwise, return 0.  */

static int
duplicate_decls (new, old)
     register tree new, old;
{
  int types_match;

  if (TREE_CODE (new) == FUNCTION_DECL && TREE_CODE (old) == FUNCTION_DECL)
    {
      tree p1 = TYPE_ARG_TYPES (TREE_TYPE (new));
      tree p2 = TYPE_ARG_TYPES (TREE_TYPE (old));
      /* Here we must take care of the case where new default
	 parameters are specified.  Also, warn if an old
	 declaration becomes ambiguous because default
	 parameters may cause the two to be ambiguous.  */
      if (comptypes (TREE_TYPE (TREE_TYPE (new)), TREE_TYPE (TREE_TYPE (old)), 1))
	{
	  types_match = compparms (p1, p2, 1);
	  /* C++: copy friendlist *before* we get smooshed.  */
	  if (DECL_FRIENDLIST (old) && !DECL_FRIENDLIST (new))
	    DECL_FRIENDLIST (new) = DECL_FRIENDLIST (old);
	}
      else types_match = 0;
    }
  else
    types_match = comptypes (TREE_TYPE (new), TREE_TYPE (old), 1);

  /* If this decl has linkage, and the old one does too, maybe no error.  */
  if (TREE_CODE (old) != TREE_CODE (new))
    {
      error_with_decl (new, "`%s' redeclared as different kind of symbol");


      /* New decl is completely inconsistent with the old one =>
	 tell caller to replace the old one.  */
      return 0;
    }

  if (!types_match)
    {
      if (TREE_CODE (new) == FUNCTION_DECL)
	{
	  /* `hack_function_decl' returns a pointer to static storage.  */
	  char oldbuf[OVERLOAD_MAX_LEN];
	  char newbuf[OVERLOAD_MAX_LEN];
	  hack_function_decl (oldbuf, 0, old, 1);
	  hack_function_decl (newbuf, 0, new, 1);
	  yylineerror (DECL_SOURCE_LINE (new),
		       "conflicting types for functions:\n\t(old)\t`%s'\n\t(new)\t`%s'",
		       oldbuf, newbuf);
	}
      else if (TREE_CODE (old) == TYPE_DECL && TYPE_SIZE (TREE_TYPE (old)) == 0)
	{
	  /* This happens when there is a forward reference to `old',
	     and it is later filled in by the type definition of `new'.
	     Just forget that we ever saw `old'.  Treat it exactly
	     like `new' from now on.  */
	  TREE_TYPE (old) = TREE_TYPE (new);
	  DECL_SIZE (old) = DECL_SIZE (new);
	  DECL_SIZE_UNIT (old) = DECL_SIZE_UNIT (new);
	  return 1;
	}
      else
	error_with_decl (new, "conflicting types for `%s'");
    }
  else
    {
      char *errmsg = redeclaration_error_message (new, old);
      if (errmsg)
	error_with_decl (new, errmsg);
    }

  /* Deal with C++: must preserve virtual function table size.  */
  if (TREE_CODE (old) == TYPE_DECL)
    {
      DECL_VSIZE (new) = DECL_VSIZE (old);
      DECL_FRIEND_CLASSES (new) = DECL_FRIEND_CLASSES (old);
    }

  /* Copy all the DECL_... slots specified in the new decl
     except for any that we copy here from the old type.  */

  if (types_match)
    {
      /* Automatically handles default parameters.  */
      tree oldtype = TREE_TYPE (old);
      /* Merge the data types specified in the two decls.  */
      TREE_TYPE (new)
	= TREE_TYPE (old) = commontype (TREE_TYPE (new), TREE_TYPE (old));

      /* Lay the type out, unless already done.  */
      if (TREE_CODE (new) != FUNCTION_DECL
	  && oldtype != TREE_TYPE (new))
	{
	  layout_type (TREE_TYPE (new));
	  if (TREE_CODE (new) != TYPE_DECL
	      && TREE_CODE (new) != CONST_DECL)
	    layout_decl (new);
	}
      else
	{
	  /* Since the type is OLD's, make OLD's size go with.  */
	  DECL_SIZE (new) = DECL_SIZE (old);
	  DECL_SIZE_UNIT (new) = DECL_SIZE_UNIT (old);
	}

      /* Merge the initialization information.  */
      if (DECL_INITIAL (new) == 0)
	DECL_INITIAL (new) = DECL_INITIAL (old);
      /* Keep the old rtl since we can safely use it.  */
      DECL_RTL (new) = DECL_RTL (old);
    }
  /* If cannot merge, then use the new type
     and discard the old rtl.  */
  else
    TREE_TYPE (old) = TREE_TYPE (new);

  /* Merge the storage class information.  */
  if (TREE_EXTERNAL (new))
    {
      TREE_STATIC (new) = TREE_STATIC (old);
      TREE_PUBLIC (new) = TREE_PUBLIC (old);
      TREE_EXTERNAL (new) = TREE_EXTERNAL (old);
    }
  else
    {
      TREE_STATIC (old) = TREE_STATIC (new);
      TREE_EXTERNAL (old) = 0;
      TREE_PUBLIC (old) = TREE_INLINE (old) ? 0 : TREE_PUBLIC (new);
    }
  if (TREE_INLINE (new))
    if (DECL_INITIAL (old) == 0)
      {
	/* Only makes sense to pretend it is inline if it
	   has not been written out yet.  */
#if 0
	warning_with_decl (old, "`%s' redeclared as `inline'");
#endif
	TREE_PUBLIC (old) = 0;
	TREE_INLINE (old) = 1;
      }
    else if (TREE_PUBLIC (old) == 1)
      {
	error_with_decl (old, "`%s' redeclared with non-public visibility");
      }

  TREE_ADDRESSABLE (new) = TREE_ADDRESSABLE (old);
  TREE_ASM_WRITTEN (new) = TREE_ASM_WRITTEN (old);

  bcopy ((char *) new + sizeof (struct tree_common),
	 (char *) old + sizeof (struct tree_common),
	 sizeof (struct tree_decl) - sizeof (struct tree_common));
  return 1;
}

/* Record a decl-node X as belonging to the current lexical scope.
   Check for errors (such as an incompatible declaration for the same
   name already seen in the same scope).

   Returns either X or an old decl for the same name.
   If an old decl is returned, it may have been smashed
   to agree with what X says.  */

tree
pushdecl (x)
     tree x;
{
  register tree t;
  register tree name = DECL_NAME (x);

  if (name)
    {
      t = lookup_name_current_level (name);
      if (t)
	{
	  if (TREE_CODE (t) != TREE_CODE (x))
	    {
	      if (TREE_CODE (t) == TYPE_DECL)
		warning ("type declaration of %s shadowed",
			 IDENTIFIER_POINTER (name));
	      else if (TREE_CODE (x) == TYPE_DECL)
		warning ("type declaration of %s shadows previous declaration",
			 IDENTIFIER_POINTER (name));
	      else if (duplicate_decls (x, t))
		return t;
	    }
	  else if (duplicate_decls (x, t))
	    return t;
	}

      /* If declaring a type as a typedef, and the type has no known
	 typedef name, install this TYPE_DECL as its typedef name.

	 C++: If it had an anonymous aggregate or enum name,
	 give it a `better' one.  */
      if (TREE_CODE (x) == TYPE_DECL)
	{
	  if (TYPE_NAME (TREE_TYPE (x)) == 0
	      || TREE_CODE (TYPE_NAME (TREE_TYPE (x))) != TYPE_DECL)
	    TYPE_NAME (TREE_TYPE (x)) = x;
	  else if (ANON_AGGRNAME_P (DECL_NAME (TYPE_NAME (TREE_TYPE (x)))))
	    {
	      /* do gratuitous C++ typedefing, and make sure that
		 we access this type either through TREE_TYPE field
		 or via the tags list.  */
	      TYPE_NAME (TREE_TYPE (x)) = x;
	      pushtag (DECL_NAME (x), TREE_TYPE (x));
	    }
	}

      /* This name is new in its binding level.
	 Install the new declaration and return it.  */
      if (current_binding_level == global_binding_level
	  /* In PCC-compatibility mode, extern decls
	     take effect at top level no matter where they are.  */
	  || (flag_traditional && TREE_EXTERNAL (x)
	      && lookup_name (name) == 0))
	{
	  /* Install a global value.  */
	  
	  IDENTIFIER_GLOBAL_VALUE (name) = x;

	  if (IDENTIFIER_IMPLICIT_DECL (name) != 0
	      /* If this real decl matches the implicit, don't complain.  */
	      && ! (TREE_CODE (x) == FUNCTION_DECL
		    && TREE_TYPE (TREE_TYPE (x)) == integer_type_node))
	    warning ("`%s' was previously implicitly declared to return `int'",
		     IDENTIFIER_POINTER (name));
	}
      else
	{
	  /* Here to install a non-global value.
	     First warn if shadowing an argument.  */
	  if (IDENTIFIER_LOCAL_VALUE (name)
	      && TREE_CODE (IDENTIFIER_LOCAL_VALUE (name)) == PARM_DECL
	      && current_binding_level->level_chain->parm_flag)
	    warning ("shadowing parameter `%s' with a local variable",
		     IDENTIFIER_POINTER (name));
	  /* If storing a local value, there may already be one (inherited).
	     If so, record it for restoration when this binding level ends.  */
	  if (IDENTIFIER_LOCAL_VALUE (name))
	    current_binding_level->shadowed
	      = tree_cons (name, IDENTIFIER_LOCAL_VALUE (name),
			   current_binding_level->shadowed);
	  IDENTIFIER_LOCAL_VALUE (name) = x;

	  /* Keep count of variables in this level with incomplete type.  */
	  if (TREE_CODE (x) != TYPE_DECL && TYPE_SIZE (TREE_TYPE (x)) == 0)
	    ++current_binding_level->n_incomplete;
	}
    }

  /* Put decls on list in reverse order.
     We will reverse them later if necessary.  */
  TREE_CHAIN (x) = current_binding_level->names;
  current_binding_level->names = x;

  return x;
}

/* Like pushdecl, only it places X in GLOBAL_BINDING_LEVEL,
   if appropriate.  */
tree
pushdecl_top_level (x)
     tree x;
{
  register tree t;
  register tree name = DECL_NAME (x);

  if (name)
    {
      t = IDENTIFIER_GLOBAL_VALUE (name);
      if (t)
	{
	  if (TREE_CODE (t) != TREE_CODE (x))
	    {
	      if (TREE_CODE (t) == TYPE_DECL)
		warning ("type declaration of %s shadowed",
			 IDENTIFIER_POINTER (name));
	      else if (TREE_CODE (x) == TYPE_DECL)
		warning ("type declaration of %s shadows previous declaration",
			 IDENTIFIER_POINTER (name));
	      else if (duplicate_decls (x, t))
		return t;
	    }
	  else if (duplicate_decls (x, t))
	    return t;
	}

      /* If declaring a type as a typedef, and the type has no known
	 typedef name, install this TYPE_DECL as its typedef name.  */
      if (TREE_CODE (x) == TYPE_DECL)
	if (TYPE_NAME (TREE_TYPE (x)) == 0
	    || TREE_CODE (TYPE_NAME (TREE_TYPE (x))) != TYPE_DECL)
	  TYPE_NAME (TREE_TYPE (x)) = x;

      /* This name is new in its binding level.
	 Install the new declaration and return it.  */

      IDENTIFIER_GLOBAL_VALUE (name) = x;

      /* Keep count of variables in this level with incomplete type.  */
      if (TYPE_SIZE (TREE_TYPE (x)) == 0)
	{
	  /* Don't really know what to do about this.  */
	  abort ();
	  ++global_binding_level->n_incomplete;
	}
    }

  /* Put decls on list in reverse order.
     We will reverse them later if necessary.  */
  TREE_CHAIN (x) = global_binding_level->names;
  global_binding_level->names = x;

  return x;
}

/* Generate an implicit declaration for identifier FUNCTIONID
   as a function of type int ().  Print a warning if appropriate.  */

tree
implicitly_declare (functionid)
     tree functionid;
{
  register tree decl;
  int force_perm = 0;

  if (current_binding_level == global_binding_level
      || flag_traditional)
    /* An implicit declaration not inside a function?
       It can happen with invalid input in an initializer.
       A suitable error message will happen later,
       but we must not put a temporary node in a global value!
       If -traditional, ALL implicit decls must be permanent.  */
    force_perm = 1;

  if (force_perm)
    end_temporary_allocation ();

  decl = build_decl (FUNCTION_DECL, functionid, default_function_type);

  TREE_EXTERNAL (decl) = 1;
  TREE_PUBLIC (decl) = 1;

  /* ANSI standard says implicit declarations are in the innermost block.
     So we record the decl in the standard fashion.
     If flag_traditional is set, pushdecl does it top-level.  */
  pushdecl (decl);
  rest_of_decl_compilation (decl, 0, 0, 0);

  if (warn_implicit
      /* Only one warning per identifier.  */
      && IDENTIFIER_IMPLICIT_DECL (functionid) == 0)
    warning ("implicit declaration of function `%s'",
	     IDENTIFIER_POINTER (functionid));

  IDENTIFIER_IMPLICIT_DECL (functionid) = decl;

  if (force_perm)
    temporary_allocation ();

  return decl;
}

/* Return zero if the declaration NEW is valid
   when the declaration OLD (assumed to be for the same name)
   has already been seen.
   Otherwise return an error message format string with a %s
   where the identifier should go.  */

static char *
redeclaration_error_message (new, old)
     tree new, old;
{
  if (TREE_CODE (new) == TYPE_DECL)
    {
      /* Because C++ can put things into name space for free,
	 constructs like "typedef struct foo { ... } foo"
	 would look like an erroneous redeclaration.  */
      if (TREE_TYPE (old) == TREE_TYPE (new))
	return 0;
      else
	return "redefinition of `%s'";
    }
  else if (TREE_CODE (new) == FUNCTION_DECL)
    {
      /* Declarations of functions can insist on internal linkage
	 but they can't be inconsistent with internal linkage,
	 so there can be no error on that account.
	 However defining the same name twice is no good.  */
      if (DECL_INITIAL (old) != 0 && DECL_INITIAL (new) != 0)
	return "redefinition of `%s'";
      return 0;
    }
  else if (current_binding_level == global_binding_level)
    {
      /* Objects declared at top level:  */
      /* If at least one is a reference, it's ok.  */
      if (TREE_EXTERNAL (new) || TREE_EXTERNAL (old))
	return 0;
      /* Reject two definitions.  */
      if (DECL_INITIAL (old) != 0 && DECL_INITIAL (new) != 0)
	return "redefinition of `%s'";
      /* Now we have two tentative defs, or one tentative and one real def.  */
      /* Insist that the linkage match.  */
      if (TREE_PUBLIC (old) != TREE_PUBLIC (new))
	return "conflicting declarations of `%s'";
      return 0;
    }
  else
    {
      /* Objects declared with block scope:  */
      /* Reject two definitions, and reject a definition
	 together with an external reference.  */
      if (!(TREE_EXTERNAL (new) && TREE_EXTERNAL (old)))
	return "redeclaration of `%s'";
      return 0;
    }
}

/* Get the LABEL_DECL corresponding to identifier ID as a label.
   Create one if none exists so far for the current function.
   This function is called for both label definitions and label references.  */

tree
lookup_label (id)
     tree id;
{
  register tree decl = IDENTIFIER_LABEL_VALUE (id);

  if (decl != 0)
    return decl;

  decl = build_decl (LABEL_DECL, id, NULL_TREE);
  DECL_MODE (decl) = VOIDmode;
  /* Mark that the label's definition has not been seen.  */
  DECL_SOURCE_LINE (decl) = 0;

  IDENTIFIER_LABEL_VALUE (id) = decl;

  named_labels
    = tree_cons (NULL_TREE, decl, named_labels);

  return decl;
}

/* Define a label, specifying the location in the source file.
   Return the LABEL_DECL node for the label, if the definition is valid.
   Otherwise return 0.  */

tree
define_label (filename, line, name)
     char *filename;
     int line;
     tree name;
{
  tree decl = lookup_label (name);
  if (DECL_SOURCE_LINE (decl) != 0)
    {
      error_with_decl (decl, "duplicate label `%s'");
      return 0;
    }
  else
    {
      /* Mark label as having been defined.  */
      DECL_SOURCE_FILE (decl) = filename;
      DECL_SOURCE_LINE (decl) = line;
      return decl;
    }
}

/* Return the list of declarations of the current level.
   Note that this list is in reverse order unless/until
   you nreverse it; and when you do nreverse it, you must
   store the result back using `storedecls' or you will lose.  */

tree
getdecls ()
{
  return current_binding_level->names;
}

/* Return the list of type-tags (for structs, etc) of the current level.  */

tree
gettags ()
{
  return current_binding_level->tags;
}

/* Store the list of declarations of the current level.
   This is done for the parameter declarations of a function being defined,
   after they are modified in the light of any missing parameters.  */

static void
storedecls (decls)
     tree decls;
{
  current_binding_level->names = decls;
}

/* Similarly, store the list of tags of the current level.  */

static void
storetags (tags)
     tree tags;
{
  current_binding_level->tags = tags;
}

/* Given NAME, an IDENTIFIER_NODE,
   return the structure (or union or enum) definition for that name.
   Searches binding levels from BINDING_LEVEL up to the global level.
   If THISLEVEL_ONLY is nonzero, searches only the specified context
   (but skips any tag-transparent contexts to find one that is
   meaningful for tags).
   FORM says which kind of type the caller wants;
   it is RECORD_TYPE or UNION_TYPE or ENUMERAL_TYPE.
   If the wrong kind of type is found, an error is reported.  */

static tree
lookup_tag (form, name, binding_level, thislevel_only)
     enum tree_code form;
     struct binding_level *binding_level;
     tree name;
     int thislevel_only;
{
  register struct binding_level *level;

  for (level = binding_level; level; level = level->level_chain)
    {
      register tree tail;
      for (tail = level->tags; tail; tail = TREE_CHAIN (tail))
	{
	  if (TREE_PURPOSE (tail) == name)
	    {
	      if (TREE_CODE (TREE_VALUE (tail)) != form)
		{
		  /* Definition isn't the kind we were looking for.  */
		  error ("`%s' defined as wrong kind of tag",
			 IDENTIFIER_POINTER (name));
		}
	      return TREE_VALUE (tail);
	    }
	}
      if (thislevel_only && ! level->tag_transparent)
	return NULL_TREE;
    }
  return NULL_TREE;
}

/* Given a type, find the tag that was defined for it and return the tag name.
   Otherwise return 0.  However, the value can never be 0
   in the cases in which this is used.

   C++: If NAME is non-zero, this is the new name to install.  This is
   done when replacing anonymous tags with real tag names.  */

static tree
lookup_tag_reverse (type, name)
     tree type;
     tree name;
{
  register struct binding_level *level;

  for (level = current_binding_level; level; level = level->level_chain)
    {
      register tree tail;
      for (tail = level->tags; tail; tail = TREE_CHAIN (tail))
	{
	  if (TREE_VALUE (tail) == type)
	    {
	      if (name)
		TREE_PURPOSE (tail) = name;
	      return TREE_PURPOSE (tail);
	    }
	}
    }
  return NULL_TREE;
}

/* Look up NAME in the current binding level and its superiors
   in the namespace of variables, functions and typedefs.
   Return a ..._DECL node of some kind representing its definition,
   or return 0 if it is undefined.  */

tree
lookup_name (name)
     tree name;
{
  register tree val;
  if (current_binding_level != global_binding_level
      && IDENTIFIER_LOCAL_VALUE (name))
    val = IDENTIFIER_LOCAL_VALUE (name);
  /* In C++ class fields are somewhere between local and global scope.  */
  else if (IDENTIFIER_CLASS_VALUE (name))
    val = IDENTIFIER_CLASS_VALUE (name);
  else
    val = IDENTIFIER_GLOBAL_VALUE (name);
  if (val && TREE_TYPE (val) == error_mark_node)
    return error_mark_node;
  return val;
}

/* Similar to `lookup_name' but look only at current binding level.  */

static tree
lookup_name_current_level (name)
     tree name;
{
  register tree t;

  if (current_binding_level == global_binding_level)
    return IDENTIFIER_GLOBAL_VALUE (name);

  if (IDENTIFIER_LOCAL_VALUE (name) == 0)
    return 0;

  for (t = current_binding_level->names; t; t = TREE_CHAIN (t))
    if (DECL_NAME (t) == name)
      break;

  return t;
}

/* Create the predefined scalar types of C,
   and some nodes representing standard constants (0, 1, (void *)0).
   Initialize the global binding level.
   Make definitions for built-in primitive functions.  */

void
init_decl_processing ()
{
  register tree endlink;

  current_function_decl = NULL;
  named_labels = NULL;
  current_binding_level = NULL_BINDING_LEVEL;
  free_binding_level = NULL_BINDING_LEVEL;
  pushlevel (0);	/* make the binding_level structure for global names.  */
  global_binding_level = current_binding_level;

  value_identifier = get_identifier ("<value>");

  /* Define `int' and `char' first so that dbx will output them first.  */

#ifdef INT_TYPE_SIZE
  integer_type_node = make_signed_type (INT_TYPE_SIZE);
#else
  integer_type_node = make_signed_type (BITS_PER_WORD);
#endif
  pushdecl (build_decl (TYPE_DECL, ridpointers[(int) RID_INT],
			integer_type_node));

  /* Define `char', which is like either `signed char' or `unsigned char'
     but not the same as either.  */

  char_type_node =
    (flag_signed_char
     ? make_signed_type (BITS_PER_UNIT)
     : make_unsigned_type (BITS_PER_UNIT));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("char"),
			char_type_node));

#ifdef INT_TYPE_SIZE
  unsigned_type_node = make_unsigned_type (INT_TYPE_SIZE);
#else
  unsigned_type_node = make_unsigned_type (BITS_PER_WORD);
#endif
  IDENTIFIER_GLOBAL_VALUE (ridpointers[(int) RID_UNSIGNED])
    = pushdecl (build_decl (TYPE_DECL, get_identifier ("unsigned int"),
			    unsigned_type_node));

  /* `unsigned int' is the type for sizeof.  */
  sizetype = unsigned_type_node;
  TREE_TYPE (TYPE_SIZE (integer_type_node)) = sizetype;
  TREE_TYPE (TYPE_SIZE (char_type_node)) = sizetype;
  TREE_TYPE (TYPE_SIZE (unsigned_type_node)) = sizetype;

  error_mark_node = make_node (ERROR_MARK);
  TREE_TYPE (error_mark_node) = error_mark_node;
  error_mark_list = build_tree_list (error_mark_node, error_mark_node);
  TREE_TYPE (error_mark_list) = error_mark_node;

  short_integer_type_node = make_signed_type (BITS_PER_UNIT * min (UNITS_PER_WORD / 2, 2));
  IDENTIFIER_GLOBAL_VALUE (ridpointers[(int) RID_SHORT])
    = pushdecl (build_decl (TYPE_DECL, get_identifier ("short int"),
			short_integer_type_node));

  long_integer_type_node = make_signed_type (BITS_PER_WORD);
  IDENTIFIER_GLOBAL_VALUE (ridpointers[(int) RID_LONG])
    = pushdecl (build_decl (TYPE_DECL, get_identifier ("long int"),
			long_integer_type_node));

  short_unsigned_type_node = make_unsigned_type (BITS_PER_UNIT * min (UNITS_PER_WORD / 2, 2));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("short unsigned int"),
			short_unsigned_type_node));

  long_unsigned_type_node = make_unsigned_type (BITS_PER_WORD);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long unsigned int"),
			long_unsigned_type_node));

  /* Define both `signed char' and `unsigned char'.  */
  signed_char_type_node = make_signed_type (BITS_PER_UNIT);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("signed char"),
			signed_char_type_node));

  unsigned_char_type_node = make_unsigned_type (BITS_PER_UNIT);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("unsigned char"),
			unsigned_char_type_node));

  float_type_node = make_node (REAL_TYPE);
  TYPE_PRECISION (float_type_node) = BITS_PER_WORD;
  pushdecl (build_decl (TYPE_DECL, ridpointers[(int) RID_FLOAT],
			float_type_node));
  layout_type (float_type_node);

  double_type_node = make_node (REAL_TYPE);
  TYPE_PRECISION (double_type_node) = 2 * BITS_PER_WORD;
  pushdecl (build_decl (TYPE_DECL, ridpointers[(int) RID_DOUBLE],
			double_type_node));
  layout_type (double_type_node);

  long_double_type_node = make_node (REAL_TYPE);
  TYPE_PRECISION (long_double_type_node) = 2 * BITS_PER_WORD;
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long double"),
			long_double_type_node));
  layout_type (long_double_type_node);

  integer_zero_node = build_int_2 (0, 0);
  TREE_TYPE (integer_zero_node) = integer_type_node;
  integer_one_node = build_int_2 (1, 0);
  TREE_TYPE (integer_one_node) = integer_type_node;

  void_type_node = make_node (VOID_TYPE);
  pushdecl (build_decl (TYPE_DECL,
			ridpointers[(int) RID_VOID], void_type_node));
  layout_type (void_type_node);	/* Uses integer_zero_node.  */

  null_pointer_node = build_int_2 (0, 0);
  TREE_TYPE (null_pointer_node) = build_pointer_type (void_type_node);
  layout_type (TREE_TYPE (null_pointer_node));

  string_type_node = build_pointer_type (char_type_node);

  /* make a type for arrays of 256 characters.
     256 is picked randomly because we have a type for integers from 0 to 255.
     With luck nothing will ever really depend on the length of this
     array type.  */
  char_array_type_node
    = build_array_type (char_type_node, unsigned_char_type_node);
  /* Likewise for arrays of ints.  */
  int_array_type_node
    = build_array_type (integer_type_node, unsigned_char_type_node);

  default_function_type
    = build_function_type (integer_type_node, NULL_TREE);

  ptr_type_node = build_pointer_type (void_type_node);
  endlink = tree_cons (NULL_TREE, void_type_node, NULL_TREE);

  double_ftype_double
    = build_function_type (double_type_node,
			   tree_cons (NULL_TREE, double_type_node, endlink));

  double_ftype_double_double
    = build_function_type (double_type_node,
			   tree_cons (NULL_TREE, double_type_node,
				      tree_cons (NULL_TREE,
						 double_type_node, endlink)));

  int_ftype_int
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, integer_type_node, endlink));

  long_ftype_long
    = build_function_type (long_integer_type_node,
			   tree_cons (NULL_TREE,
				      long_integer_type_node, endlink));

  void_ftype_ptr_ptr_int
    = build_function_type (void_type_node,
			   tree_cons (NULL_TREE, ptr_type_node,
				      tree_cons (NULL_TREE, ptr_type_node,
						 tree_cons (NULL_TREE,
							    integer_type_node,
							    endlink))));

  int_ftype_ptr_ptr_int
    = build_function_type (integer_type_node,
			   tree_cons (NULL_TREE, ptr_type_node,
				      tree_cons (NULL_TREE, ptr_type_node,
						 tree_cons (NULL_TREE,
							    integer_type_node,
							    endlink))));

  void_ftype_ptr_int_int
    = build_function_type (void_type_node,
			   tree_cons (NULL_TREE, ptr_type_node,
				      tree_cons (NULL_TREE, integer_type_node,
						 tree_cons (NULL_TREE,
							    integer_type_node,
							    endlink))));

  ptr_ftype_int
    = build_function_type (ptr_type_node,
			   tree_cons (NULL_TREE,
				      integer_type_node,
				      endlink));

  ptr_ftype_ptr_int_int_ptr
    = build_function_type (ptr_type_node,
	   tree_cons (NULL_TREE, ptr_type_node,
	      tree_cons (NULL_TREE, integer_type_node,
		 tree_cons (NULL_TREE, integer_type_node,
		    tree_cons (NULL_TREE, ptr_type_node,
			       endlink)))));

  void_ftype_ptr
    = build_function_type (void_type_node,
			   tree_cons (NULL_TREE,
				      ptr_type_node,
				      endlink));
  void_ftype_ptr_int_int_ptr_int_int
    = build_function_type (void_type_node,
	   tree_cons (NULL_TREE, ptr_type_node,
	      tree_cons (NULL_TREE, integer_type_node,
		 tree_cons (NULL_TREE, integer_type_node,
		    tree_cons (NULL_TREE, ptr_type_node,
		       tree_cons (NULL_TREE, integer_type_node,
			  tree_cons (NULL_TREE, integer_type_node,
				     endlink)))))));

#ifdef VTABLE_USES_MASK
  /* This is primarily for virtual function definition.  We
     declare an array of `void *', which can later be
     converted to the appropriate function pointer type.
     To do pointers to members, we need a mask which can
     distinguish an index value into a virtual function table
     from an address.  */
  vtbl_mask = build_int_2 (~(VINDEX_MAX - 1), -1);
#endif

  vtbl_type_node
    = build_array_type (ptr_type_node, NULL_TREE);
  layout_type (vtbl_type_node);
  vtbl_type_node = build_type_variant (vtbl_type_node, 1, 0, 0);

  builtin_function ("__builtin_alloca",
		    build_function_type (ptr_type_node,
					 tree_cons (NULL_TREE,
						    integer_type_node,
						    endlink)),
		    BUILT_IN_ALLOCA);

  builtin_function ("__builtin_abs", int_ftype_int, BUILT_IN_ABS);
  builtin_function ("__builtin_fabs", double_ftype_double, BUILT_IN_FABS);
  builtin_function ("__builtin_labs", long_ftype_long, BUILT_IN_LABS);
  builtin_function ("__builtin_ffs", int_ftype_int, BUILT_IN_FFS);
/*  builtin_function ("__builtin_div", default_ftype, BUILT_IN_DIV);
  builtin_function ("__builtin_ldiv", default_ftype, BUILT_IN_LDIV); */
  builtin_function ("__builtin_ffloor", double_ftype_double, BUILT_IN_FFLOOR);
  builtin_function ("__builtin_fceil", double_ftype_double, BUILT_IN_FCEIL);
  builtin_function ("__builtin_fmod", double_ftype_double_double, BUILT_IN_FMOD);
  builtin_function ("__builtin_frem", double_ftype_double_double, BUILT_IN_FREM);
  builtin_function ("__builtin_memcpy", void_ftype_ptr_ptr_int, BUILT_IN_MEMCPY);
  builtin_function ("__builtin_memcmp", int_ftype_ptr_ptr_int, BUILT_IN_MEMCMP);
  builtin_function ("__builtin_memset", void_ftype_ptr_int_int, BUILT_IN_MEMSET);
  builtin_function ("__builtin_fsqrt", double_ftype_double, BUILT_IN_FSQRT);
  builtin_function ("__builtin_getexp", double_ftype_double, BUILT_IN_GETEXP);
  builtin_function ("__builtin_getman", double_ftype_double, BUILT_IN_GETMAN);

  /* C++ extensions */

  unknown_type_node = make_node (UNKNOWN_TYPE);
  pushdecl (build_decl (TYPE_DECL,
			get_identifier ("unknown type"),
			void_type_node));
  layout_type (unknown_type_node);

  /* Define these now, but use 0 as their DECL_FUNCTION_CODE.  This
     will install them in the global binding level, but cause them
     to be expanded normally.  */
  builtin_function ("__builtin_new", ptr_ftype_int, 0);
  builtin_function ("__builtin_vec_new", ptr_ftype_ptr_int_int_ptr, 0);
  builtin_function ("__builtin_delete", void_ftype_ptr, 0);
  builtin_function ("__builtin_vec_delete", void_ftype_ptr_int_int_ptr_int_int, 0);
}

/* Make a definition for a builtin function named NAME and whose data type
   is TYPE.  TYPE should be a function type with argument types.
   FUNCTION_CODE tells later passes how to compile calls to this function.
   See tree.h for its possible values.  */

static void
builtin_function (name, type, function_code)
     char *name;
     tree type;
     enum built_in_function function_code;
{
  tree decl = build_decl (FUNCTION_DECL, get_identifier (name), type);
  TREE_EXTERNAL (decl) = 1;
  TREE_PUBLIC (decl) = 1;
  make_function_rtl (decl);
  pushdecl (decl);
  DECL_SET_FUNCTION_CODE (decl, function_code);
}

/* Called when a declaration is seen that contains no names to declare.
   If its type is a reference to a structure, union or enum inherited
   from a containing scope, shadow that tag name for the current scope
   with a forward reference.
   If its type defines a new named structure or union
   or defines an enum, it is valid but we need not do anything here.
   Otherwise, it is an error.

   C++: may have to grok the declspecs to learn about static,
   complain for anonymous unions.  */

void
shadow_tag (declspecs)
     tree declspecs;
{
  register tree link;
  register tree value;
  register enum tree_code code;
  register tree t = NULL_TREE;

  for (link = declspecs; link; link = TREE_CHAIN (link))
    {
      value = TREE_VALUE (link);
      code = TREE_CODE (value);
      if ((IS_AGGR_TYPE_CODE (code) || code == ENUMERAL_TYPE)
	  && TYPE_SIZE (value) != 0)
	{
	  register tree name = TYPE_NAME (value);

	  if (name == NULL_TREE)
	    name = lookup_tag_reverse (value, NULL_TREE);

	  if (name && TREE_CODE (name) == TYPE_DECL)
	    name = DECL_NAME (name);

	  t = lookup_tag (code, name, current_binding_level, 1);

	  if (t == 0)
	    {
	      t = make_node (code);
	      pushtag (name, t);
	      break;
	    }

	  if (name != 0 || code == ENUMERAL_TYPE)
	    break;
	}
    }

  /* @@ What would it mean for t to be NULL_TREE?  */
  if (t == NULL_TREE)
    return;

  if (code == UNION_TYPE
      && ANON_AGGRNAME_P (DECL_NAME (TYPE_NAME (t)))
      && TYPE_FIELDS (t))
    {
      tree decl = grokdeclarator (get_identifier ("anonymous union"), declspecs, NORMAL, 0);
      finish_anon_union (decl);
    }
}

/* Decode a "typename", such as "int **", returning a ..._TYPE node.  */

tree
groktypename (typename)
     tree typename;
{
  if (TREE_CODE (typename) != TREE_LIST)
    return typename;
  return grokdeclarator (TREE_VALUE (typename),
			 TREE_PURPOSE (typename),
			 TYPENAME, 0);
}

/* Decode a declarator in an ordinary declaration or data definition.
   This is called as soon as the type information and variable name
   have been parsed, before parsing the initializer if any.
   Here we create the ..._DECL node, fill in its type,
   and put it on the list of decls for the current context.
   The ..._DECL node is returned as the value.

   Exception: for arrays where the length is not specified,
   the type is left null, to be filled in by `finish_decl'.

   Function definitions do not come here; they go to start_function
   instead.  However, external and forward declarations of functions
   do go through here.  Structure field declarations are done by
   grokfield and not through here.  */

/* Set this nonzero to debug not using the temporary obstack
   to parse initializers.  */
int debug_no_temp_inits = 1;

tree
start_decl (declarator, declspecs, initialized)
     tree declspecs, declarator;
     int initialized;
{
  register tree decl, type;
  register tree tem;
  int init_written = initialized;

  decl = grokdeclarator (declarator, declspecs,
			 NORMAL, initialized);
  if (! decl) return decl;
  if (TREE_CODE (decl) == SCOPE_REF)
    {
      error ("bad scope request");
      return NULL_TREE;
    }

  type = TREE_TYPE (decl);

  if (initialized)
    /* Is it valid for this decl to have an initializer at all?
       If not, set INITIALIZED to zero, which will indirectly
       tell `finish_decl' to ignore the initializer once it is parsed.  */
    switch (TREE_CODE (decl))
      {
      case TYPE_DECL:
	/* typedef foo = bar  means give foo the same type as bar.
	   We haven't parsed bar yet, so `finish_decl' will fix that up.
	   Any other case of an initialization in a TYPE_DECL is an error.  */
	if (pedantic || list_length (declspecs) > 1)
	  {
	    error ("typedef `%s' is initialized",
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	    initialized = 0;
	  }
	break;

      case FUNCTION_DECL:
	error ("function `%s' is initialized like a variable",
	       IDENTIFIER_POINTER (DECL_NAME (decl)));
	initialized = 0;
	break;

      default:
	/* Don't allow initializations for incomplete types
	   except for arrays which might be completed by the initialization.  */
	if (TYPE_SIZE (type) != 0)
	  ;			/* A complete type is ok.  */
	else if (TREE_CODE (type) != ARRAY_TYPE)
	  {
	    error ("variable `%s' has initializer but incomplete type",
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	    initialized = 0;
	  }
	else if (TYPE_SIZE (TREE_TYPE (type)) == 0)
	  {
	    error ("elements of array `%s' have incomplete type",
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	    initialized = 0;
	  }
      }

  if (! (initialized || TREE_CODE (decl) == TYPE_DECL
	 || ! IS_AGGR_TYPE (type) || TREE_EXTERNAL (decl)))
    {
      tree basetype = type;
      if (TYPE_SIZE (type) == 0)
	{
	  error ("aggregate `%s' has incomplete type and cannot be initialized",
		 IDENTIFIER_POINTER (DECL_NAME (decl)));
	  /* Change the type so that assemble_variable will give
	     DECL an rtl we can live with: (mem (const_int 0)).  */
	  TREE_TYPE (decl) = error_mark_node;
	  type = error_mark_node;
	}
      else do
	{
	  if (TREE_NEEDS_CONSTRUCTOR (basetype))
	    {
	      initialized = 2;
	      break;
	    }
	  basetype = TYPE_BASELINK (basetype);
	} while (basetype);
    }

  if (initialized)
    {
      if (current_binding_level != global_binding_level
	  && TREE_EXTERNAL (decl))
	warning ("declaration of `%s' has `extern' and is initialized",
		 IDENTIFIER_POINTER (DECL_NAME (decl)));
      TREE_EXTERNAL (decl) = 0;
      if (current_binding_level == global_binding_level)
	TREE_STATIC (decl) = 1;

      /* Tell `pushdecl' this is an initialized decl
	 even though we don't yet have the initializer expression.
	 Also tell `finish_decl' it may store the real initializer.  */
      DECL_INITIAL (decl) = error_mark_node;
      /* Add this decl to the current binding level.  */
      tem = pushdecl (decl);

      /* When parsing and digesting the initializer,
	 use temporary storage.

	 C++: but only if we won't need that initializer
	 back later!  */
      if ((TREE_CODE (decl) == VAR_DECL
	   && IS_AGGR_TYPE (type)
	   && TREE_NEEDS_CONSTRUCTOR (type)
#if 0
	   && set_temp_name (decl)
#endif
	   )
	  || (TREE_CODE (decl) == CONST_DECL))
	;
      else if (init_written
	       && current_binding_level == global_binding_level
	       && (flag_const_is_variable
		   || TREE_CODE (decl) != VAR_DECL
		   || ! TREE_READONLY (decl))
	       && debug_no_temp_inits)
	temporary_allocation ();
    }
  else
    tem = pushdecl (decl);

  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      if (TREE_OVERLOADED (decl))
	{
	  /* @@ Also done in start_function.  */
	  tree fn_name = DECL_ORIGINAL_NAME (decl);
	  tree glob = IDENTIFIER_GLOBAL_VALUE (fn_name);
	  TREE_OVERLOADED (tem) = 1;

	  if (glob && TREE_CODE (glob) == FUNCTION_DECL)
	    glob = build_tree_list (NULL_TREE, glob);

	  if (glob && TREE_VALUE (glob) == NULL_TREE)
	    TREE_VALUE (glob) = tem;
	  else if (value_member (tem, glob) == 0)
	    {
	      IDENTIFIER_GLOBAL_VALUE (fn_name)
		= tree_cons (fn_name, tem, glob);
	      TREE_TYPE (IDENTIFIER_GLOBAL_VALUE (fn_name)) = unknown_type_node;
	    }
	}
    }

  return tem;
}

/* Finish processing of a declaration;
   install its line number and initial value.
   If the length of an array type is not known before,
   it must be determined now, from the initial value, or it is an error.

   For C++, `finish_decl' must be fairly evasive:  it must keep initializers
   for aggregates that have constructors alive on the permanent obstack,
   so that the global initializing functions can be written at the end.

   INIT0 holds the value of an initializer that should be allowed to escape
   the normal rules.

   For functions that take defualt parameters, DECL points to its
   "maximal" instantiation.  finish_decl must then also declared its
   subsequently lower and lower forms of instantiation, checking for
   ambiguity as it goes.  This can be sped up later.  */

void
finish_decl (decl, init, asmspec)
     tree decl, init;
     tree asmspec;
{
  register tree type;
  tree init0 = 0;

  /* If this is 0, then we did not change obstacks.  */
  if (! decl) return;

  /* If the type of the thing we are declaring either has
     a constructor, or has a virtual function table pointer,
     AND its initialization was accepted by `start_decl',
     then we stayed on the permanent obstack through the
     declaration, otherwise, changed obstacks as GCC would.  */

  type = TREE_TYPE (decl);

  /* Take care of TYPE_DECLs up front.  */
  if (TREE_CODE (decl) == TYPE_DECL)
    {
      if (init && DECL_INITIAL (decl))
	{
	  /* typedef foo = bar; store the type of bar as the type of foo.  */
	  TREE_TYPE (decl) = type = TREE_TYPE (init);
	  DECL_INITIAL (decl) = init = 0;
	}
      goto finish_end;
    }

  if (TREE_NEEDS_CONSTRUCTOR (type)
      && (TREE_READONLY (decl) || TREE_READONLY (type)))
    {
      readonly_warning (decl, "initialization");
      TREE_READONLY (decl) = 0;
      type = TYPE_MAIN_VARIANT (type);
      TREE_TYPE (decl) = type;
    }

  if (decl == init)
    {
      /* This happens when an aggregate is assigned from
	 a constructor "long-distance".  I.e., by
	 "xyzzy magic = xyzzy (...)" rather than "xyzzy magic (...)".  */
      fatal ("long distance");
      init0 = DECL_INITIAL (init);
      init = 0;
      DECL_INITIAL (decl) = error_mark_node;
    }

  if (TREE_CODE (decl) == FIELD_DECL || TREE_CODE (decl) == FRIEND_DECL)
    {
      if (init)
	{
	  /* We allow initializers to become parameters to base initializers.  */
	  if (TREE_STATIC (decl))
	    sorry ("initialization of static class members");
	  if (! TREE_PERMANENT (init))
	    abort ();
	  DECL_INITIAL (decl) = init;
	}
      if (TREE_STATIC (decl))
	{
	  char buf[OVERLOAD_MAX_LEN];
	  tree name, static_decl;

	  sprintf (buf, STATIC_NAME_FORMAT, IDENTIFIER_POINTER (current_class_name),
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	  name = get_identifier (buf);
	  SET_DECL_STATIC_NAME (decl, name);
	  static_decl = build_decl (VAR_DECL, name, TREE_TYPE (decl));
	  TREE_STATIC (static_decl) = 1;
	  TREE_PUBLIC (static_decl) = 1;
	  DECL_SOURCE_LINE (static_decl) = DECL_SOURCE_LINE (decl);
	  DECL_SOURCE_FILE (static_decl) = DECL_SOURCE_FILE (decl);
	  DECL_INITIAL (static_decl) = error_mark_node;
	  static_decl = pushdecl_top_level (static_decl);
	  /* we must let `layout_union' or `layout_record' do
	     the rest of the work.  */
	  DECL_INITIAL (static_decl) = NULL_TREE;
	}
      DECL_ASM_NAME (decl) = asmspec;
      if (asmspec)
	{
	  /* This must override the asm specifier which was placed
	     by grokclassfn.  Lay this out fresh.

	     @@ Should emit an error if this redefines an asm-specified
	     @@ name, or if we have already used the function's name.  */
	  DECL_RTL (TREE_TYPE (decl)) = 0;
	  rest_of_decl_compilation (TREE_TYPE (decl), asmspec, 0, 0);
	}

      goto finish_end;
    }

  /* If `start_decl' didn't like having an initialization, ignore it now.  */

  if (init != 0 && DECL_INITIAL (decl) == 0)
    {
      init = 0;
      init0 = 0;
    }
  else if (TREE_CODE (type) == REFERENCE_TYPE)
    {
      if (! init)
	{
	  if (! init0)
	    warning ("variable declared as reference not initialized");
	}
      else
	{
	  int is_reference = TREE_CODE (TREE_TYPE (init)) == REFERENCE_TYPE;
	  if (is_reference)
	    {
	      tree tmp = bash_reference_type (init);
	      if (! comptypes (TREE_TYPE (type), TREE_TYPE (tmp), 0))
		{
		  error ("assignment between dissimilar reference types");
		  init = 0;
		}
	    }
	  else if (! comptypes (TREE_TYPE (type), TREE_TYPE (init), 0))
	    {
	      error ("illegal type conversion for reference");
	      init = 0;
	    }

	  if (init)
	    {
	      /* In the case of initialization, it is permissable
		 to assign one reference to another.  */
	      if (is_reference)
		DECL_INITIAL (decl) = init;
	      else if (lvalue_p (init))
		{
		  DECL_INITIAL (decl) = build_unary_op (ADDR_EXPR, init, 0);
		  TREE_TYPE (DECL_INITIAL (decl)) = type;
		}
	      else
		get_temp_name (TREE_TYPE (type), decl, init);
	      init = 0;
	    }
	  init0 = 0;
	}
    }
  else if (TREE_NEEDS_CONSTRUCTOR (type))
    {
#if 0
      set_temp_name (NULL_TREE);
#endif
      if (current_binding_level == global_binding_level && init)
	{
	  /* We must hide the initializer so that DECL_INITIAL
	     holds nothing that `output_addressed_constants'
	     is not willing to assemble.  */
	  init0 = init;
	  init = 0;
	}
    }

  if (TREE_CODE (decl) == CONST_DECL)
    {
      /* This will keep us from needing to worry about our obstacks.  */
	 
      DECL_INITIAL (decl) = init;
      if (init == 0)
	error ("init == 0 in finish_decl (compiler error)");
      else
	{
	  /* STRING_CST, REAL_CST, and other literal constants
	     need to have their rtl live on the permanent obstack.  */
	  if (TREE_LITERAL (decl) = TREE_LITERAL (init))
	    make_rtx_constant_now (init);
	}
      init = 0;
    }
  else if (init)
    {
      store_init_value (decl, init);
    }

  /* For top-level declaration, the initial value was read in
     the temporary obstack.  MAXINDEX, rtl, etc. to be made below
     must go in the permanent obstack; but don't discard the
     temporary data yet.  */

  if (debug_no_temp_inits
      && current_binding_level == global_binding_level
      && init
      && ! TREE_NEEDS_CONSTRUCTOR (type)
      && (flag_const_is_variable
	  || TREE_CODE (decl) != VAR_DECL
	  || ! TREE_READONLY (decl)))
    end_temporary_allocation ();

  /* Deduce size of array from initialization, if not already known.  */

  if (TREE_CODE (type) == ARRAY_TYPE
      && TYPE_DOMAIN (type) == 0)
    {
      int do_default = ! ((!pedantic && TREE_STATIC (decl))
			  || TREE_EXTERNAL (decl));
      int failure
	= complete_array_type (type, DECL_INITIAL (decl), do_default);

      if (failure == 1)
	error_with_decl (decl, "initializer fails to determine size of `%s'");

      if (failure == 2)
	{
	  if (do_default)
	    error_with_decl (decl, "array size missing in `%s'");
	  else if (!pedantic && TREE_STATIC (decl))
	    TREE_EXTERNAL (decl) = 1;
	}

      if (pedantic && TYPE_DOMAIN (type) != 0
	  && integer_zerop (TYPE_MAX_VALUE (TYPE_DOMAIN (type))))
	error_with_decl (decl, "zero-size array `%s'");

      if (TREE_CODE (decl) != TYPE_DECL)
	layout_decl (decl, 0);
    }

  if (TREE_CODE (decl) == VAR_DECL)
    {
      if (TREE_STATIC (decl) && DECL_SIZE (decl) == 0)
	{
	  /* A static variable with an incomplete type:
	     that is an error if it is initialized or `static'.
	     Otherwise, let it through, but if it is not `extern'
	     then it may cause an error message later.  */
	  if (! (TREE_PUBLIC (decl) && DECL_INITIAL (decl) == 0))
	    error_with_decl (decl, "storage size of `%s' isn't known");
	  else if (!TREE_EXTERNAL (decl) && DECL_SIZE (decl) == 0)
	    {
	      /* An automatic variable with an incomplete type:
		 that is an error.  */
	      error_with_decl (decl, "storage size of `%s' isn't known");
	      TREE_TYPE (decl) = error_mark_node;
	    }
	}

      if ((TREE_EXTERNAL (decl) || TREE_STATIC (decl))
	  && DECL_SIZE (decl) != 0 && ! TREE_LITERAL (DECL_SIZE (decl)))
	error_with_decl (decl, "storage size of `%s' isn't constant");
    }

  /* Output the assembler code and/or RTL code for variables and functions,
     unless the type is an undefined structure or union.
     If not, it will get done when the type is completed.  */

  if (TREE_CODE (decl) == VAR_DECL || TREE_CODE (decl) == FUNCTION_DECL)
    {
      rest_of_decl_compilation (decl, asmspec,
				current_binding_level == global_binding_level,
				0);

      /* C++: Handle overloaded functions with defualt parameters.  */
      /* Implicitly tests if TREE_CODE (decl) == FUNCTION_DECL.  */
      if (TREE_OVERLOADED (decl))
	{
	  tree parmtypes = TYPE_ARG_TYPES (type);
	  tree prev = NULL_TREE;
	  char *original_name = IDENTIFIER_POINTER (DECL_ORIGINAL_NAME (decl));

	  while (parmtypes && TREE_VALUE (parmtypes) != void_type_node)
	    {
	      if (TREE_PURPOSE (parmtypes))
		{
		  tree fnname, fndecl;
		  tree *argp = prev
		    ? & TREE_CHAIN (prev)
		      : & TYPE_ARG_TYPES (type);

		  *argp = NULL_TREE;
		  fnname = do_decl_overload (original_name, TYPE_ARG_TYPES (type));
		  *argp = parmtypes;
		  fndecl = build_decl (FUNCTION_DECL, fnname, type);
		  DECL_ORIGINAL_NAME (fndecl) = DECL_ORIGINAL_NAME (decl);
		  fndecl = pushdecl (fndecl);
		  DECL_RTL (fndecl) = DECL_RTL (decl);
		}
	      prev = parmtypes;
	      parmtypes = TREE_CHAIN (parmtypes);
	    }
	}

      if (current_binding_level != global_binding_level)
	{
	  if (IS_AGGR_TYPE (type))
	    {
	      if (DECL_INITIAL (decl) == error_mark_node)
		DECL_INITIAL (decl) = init0;
	      else if (init0)
		error ("naked init0 (compiler error)");
	    }
	  if (TYPE_SIZE (TREE_TYPE (decl)) != 0
	      || TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE)
	    expand_decl (decl, 1);
	}
      else if (TREE_NEEDS_CONSTRUCTOR (type))
	expand_aggr_birth (decl, init0, 1);
    }

 finish_end:
  /* Resume permanent allocation, if not within a function.  */
  if (debug_no_temp_inits && init
      && current_binding_level == global_binding_level)
    permanent_allocation ();
}

/* Make TYPE a complete type based on INITIAL_VALUE.
   Return 0 if successful, 1 if INITIAL_VALUE can't be decyphered,
   2 if there was no information (in which case assume 1 if DO_DEFAULT).  */

int
complete_array_type (type, initial_value, do_default)
     tree type;
     tree initial_value;
     int do_default;
{
  register tree maxindex = NULL_TREE;
  int value = 0;

  if (initial_value)
    {
      /* Note MAXINDEX  is really the maximum index,
	 one less than the size.  */
      if (TREE_CODE (initial_value) == STRING_CST)
	maxindex = build_int_2 (TREE_STRING_LENGTH (initial_value) - 1, 0);
      else if (TREE_CODE (initial_value) == CONSTRUCTOR)
	{
	  register int nelts
	    = list_length (CONSTRUCTOR_ELTS (initial_value));
	  maxindex = build_int_2 (nelts - 1, 0);
	}
      else
	{
	  /* Make an error message unless that happened already.  */
	  if (initial_value != error_mark_node)
	    value = 1;

	  /* Prevent further error messages.  */
	  maxindex = build_int_2 (1, 0);
	}
    }

  if (!maxindex)
    {
      if (do_default)
	maxindex = build_int_2 (1, 0);
      value = 2;
    }

  if (maxindex)
    {
      TYPE_DOMAIN (type) = make_index_type (maxindex);
      if (!TREE_TYPE (maxindex))
	TREE_TYPE (maxindex) = TYPE_DOMAIN (type);
    }

  /* Lay out the type now that we can get the real answer.  */

  layout_type (type);

  return value;
}

/* Given declspecs and a declarator,
   determine the name and type of the object declared.
   DECLSPECS is a chain of tree_list nodes whose value fields
    are the storage classes and type specifiers.

   DECL_CONTEXT says which syntactic context this declaration is in:
     NORMAL for most contexts.  Make a VAR_DECL or FUNCTION_DECL or TYPE_DECL.
     FUNCDEF for a function definition.  Like NORMAL but a few different
      error messages in each case.  Return value may be zero meaning
      this definition is too screwy to try to parse.
     PARM for a parameter declaration (either within a function prototype
      or before a function body).  Make a PARM_DECL, or return void_type_node.
     TYPENAME if for a typename (in a cast or sizeof).
      Don't make a DECL node; just return the type.
     FIELD for a struct or union field; make a FIELD_DECL.
   INITIALIZED is 1 if the decl has an initializer.

   In the TYPENAME case, DECLARATOR is really an absolute declarator.
   It may also be so in the PARM case, for a prototype where the
   argument type is specified but not the name.

   This function is where the complicated C meanings of `static'
   and `extern' are intrepreted.

   For C++, if there is any monkey business to do, the function which
   calls this one must do it, i.e., prepending instance variables,
   renaming overloaded function names, etc.

   Note that for this C++, it is an error to define a method within a class
   which does not belong to that class.

   Constant declarations which are initialized to manifest constants
   are returned as CONST_DECLS.  The caller must deal with this.

   Execpt in the case where SCOPE_REFs are implicitly known (such as
   methods within a class being redundantly qualified),
   declarations which involve SCOPE_REFs are returned as SCOPE_REFs
   (class_name::decl_name).  The caller must also deal with this.

   If a constructor or destructor is seen, and the context is FIELD,
   then the type gains the attribtue TREE_HAS_x.  If such a declaration
   is erroneous, NULL_TREE is returned.  */

static tree
grokdeclarator (declarator, declspecs, decl_context, initialized)
     tree declspecs;
     tree declarator;
     enum decl_context decl_context;
     int initialized;
{
  int specbits = 0;
  int nclasses = 0;
  tree spec;
  tree type = NULL_TREE;
  int longlong = 0;
  int constp;
  int volatilep;
  int virtualp, friendp, inlinep;
  int explicit_int = 0;
  int explicit_char = 0;
  char *name;
  tree typedef_type = 0;
  int funcdef_flag = 0;
  enum tree_code innermost_code = ERROR_MARK;
  /* C++: do not warn if constructor, destructor, op operator typename.  */
  int really_warn_return_type = warn_return_type;

  tree oldcname = current_class_name, cname = NULL_TREE, dname = NULL_TREE;
  tree ctype;
  enum overload_flags flags = NO_SPECIAL;

  if (decl_context == FUNCDEF)
    funcdef_flag = 1, decl_context = NORMAL;

  /* Look inside a declarator for the name being declared
     and get it as a string, for an error message.  */
  {
    register tree decl = declarator;
    name = 0;

    /* If we see something of the form `aggr_type xyzzy (a, b, c)'
       it is either an old-style function declaration or a call to
       a constructor.  The following conditional makes recognizes this
       case as being a call to a constructor.  Too bad if it is not.  */
    {
      tree type;

      if (decl && declspecs
	  && TREE_CODE (decl) == CALL_EXPR
	  && TREE_OPERAND (decl, 0)
	  && TREE_CODE (TREE_OPERAND (decl, 0)) == IDENTIFIER_NODE)
	{
	  type = TREE_TYPE (TREE_VALUE (declspecs));

	  if (type && IS_AGGR_TYPE (TREE_TYPE (type))
	      && parmlist_is_exprlist (TREE_OPERAND (decl, 1)))
	    {
	      if (decl_context == FIELD)
		{
		  /* That was an initializer list.  */
		  sorry ("initializer lists for field declarations");
		  decl = TREE_OPERAND (decl, 0);
		  declarator = decl;
		}
	      else
		{
		  tree d = start_decl (TREE_OPERAND (decl, 0), declspecs, 1);
		  finish_decl (d, TREE_OPERAND (decl, 1), NULL_TREE);
		  return 0;
		}
	    }

	  if (parmlist_is_random (TREE_OPERAND (decl, 1)))
	    {
	      error ("bad parameter list specification for function `%s'",
		     IDENTIFIER_POINTER (TREE_OPERAND (decl, 0)));
	      return 0;
	    }
	}
    }

    while (decl)
      switch (TREE_CODE (decl))
	{
	case WRAPPER_EXPR:	/* for C++ wrappers.  */
	  if (flags != NO_SPECIAL)
	    abort ();
	  flags = WRAPPER;
	  decl = TREE_OPERAND (decl, 0);
	  break;

	case ANTI_WRAPPER_EXPR:	/* for C++ wrappers.  */
	  if (flags != NO_SPECIAL)
	    abort ();
	  flags = ANTI_WRAPPER;
	  decl = TREE_OPERAND (decl, 0);
	  break;

	case BIT_NOT_EXPR:	/* for C++ destructors!  */
	  if (flags != NO_SPECIAL)
	    abort ();
	  flags = DESTRUCTOR;
	  really_warn_return_type = 0;
	  decl = TREE_OPERAND (decl, 0);
	  break;

	case ADDR_EXPR:		/* C++ reference declaration */
	  /* fall through */
	case ARRAY_REF:
	case INDIRECT_REF:
	  innermost_code = TREE_CODE (decl);
	  decl = TREE_OPERAND (decl, 0);
	  break;

	case CALL_EXPR:
	  innermost_code = TREE_CODE (decl);
	  decl = TREE_OPERAND (decl, 0);
	  if (decl == oldcname)
	    really_warn_return_type = 0;
	  break;

	case IDENTIFIER_NODE:
	  dname = decl;
	  name = IDENTIFIER_POINTER (decl);
	  decl = 0;
	  break;

	case OP_EXPR:
	  /* C++ operators: if these are member functions, then
	     they overload the same way normal methods do.  However,
	     if they are declared outside of a classes scope, then
	     they are implicitly treated like `friends', i.e.,
	     they do not take any unseen arguments.  */
	  if (flags != NO_SPECIAL)
	    abort ();
	  flags = OPERATOR;
	  name = "operator name";
	  decl = 0;
	  break;

	case TYPE_EXPR:
	  if (flags != NO_SPECIAL)
	    abort ();
	  flags = OP_TYPENAME;
	  name = "operator <typename>";
	  /* Go to the absdcl.  */
	  decl = TREE_OPERAND (decl, 1);
	  really_warn_return_type = 0;
	  break;

	  /* C++ extension */
	case SCOPE_REF:
	  cname = TREE_OPERAND (decl, 0);
	  if (! is_aggr_typedef_or_else (cname))
	    {
	      TREE_OPERAND (decl, 0) = 0;
	      cname = 0;
	    }
	  decl = TREE_OPERAND (decl, 1);
	  if (cname == decl)
	    really_warn_return_type = 0;
	  break;

	default:
	  abort ();
	}
    if (name == 0)
      name = "type name";
  }

  /* A function definition's declarator must have the form of
     a function declarator.  */

  if (funcdef_flag && innermost_code != CALL_EXPR)
    return 0;

  /* Anything declared one level down from the top level
     must be one of the parameters of a function
     (because the body is at least two levels down).  */

  /* This heuristic cannot be applied to C++ nodes! Fixed, however,
     by not allowing C++ class definitions to specify their parameters
     with xdecls (must be spec.d in the parmlist).

     Since we now wait to push a class scope until we are sure that
     we are in a legitimate method context, we must set oldcname
     explicitly (since current_class_name is not yet alive).  */
  if (decl_context == NORMAL)
    if (current_binding_level->level_chain == global_binding_level)
      decl_context = PARM;
    else if (cname && ! oldcname)
      oldcname = cname;

  /* Look through the decl specs and record which ones appear.
     Some typespecs are defined as built-in typenames.
     Others, the ones that are modifiers of other types,
     are represented by bits in SPECBITS: set the bits for
     the modifiers that appear.  Storage class keywords are also in SPECBITS.

     If there is a typedef name or a type, store the type in TYPE.
     This includes builtin typedefs such as `int'.

     Set EXPLICIT_INT if the type is `int' or `char' and did not
     come from a user typedef.

     Set LONGLONG if `long' is mentioned twice.

     For C++, constructors and destructors have their own fast treatment.  */

  for (spec = declspecs; spec; spec = TREE_CHAIN (spec))
    {
      register int i;
      register tree id = TREE_VALUE (spec);

      /* Certain parse errors slip through.  For example,
	 `int class;' is not caught by the parser. Try
	 weakly to recover here.  */
      if (TREE_CODE (spec) != TREE_LIST)
	return 0;

      if (TREE_CODE (id) == IDENTIFIER_NODE)
	{
	  if (id == ridpointers[(int) RID_INT])
	    {
	      explicit_int = 1;
	      type = TREE_TYPE (IDENTIFIER_GLOBAL_VALUE (id));
	      goto found;
	    }
	  if (id == ridpointers[(int) RID_CHAR])
	    {
	      explicit_char = 1;
	      type = TREE_TYPE (IDENTIFIER_GLOBAL_VALUE (id));
	      goto found;
	    }
	  /* C++ aggregate types.  */
	  if (TREE_TYPE (id))
	    {
	      /* normalize so that `class foo bar' becomes `struct foo bar'.  */
	      type = TREE_TYPE (TREE_TYPE (id));
	      if (type != TYPE_MAIN_VARIANT (type)
		  && TREE_READONLY (type) == 0
		  && TREE_VOLATILE (type) == 0)
		type = TYPE_MAIN_VARIANT (type);
	      goto found;
	    }

	  for (i = (int) RID_FIRST_MODIFIER; i < (int) RID_MAX; i++)
	    {
	      if (ridpointers[i] == id)
		{
		  if (i == (int) RID_LONG && specbits & (1<<i))
		    longlong = 1;
		  if (specbits & (1 << i))
		    warning ("duplicate `%s'", IDENTIFIER_POINTER (id));
		  specbits |= 1 << i;
		  goto found;
		}
	    }
	}
      if (type)
	error ("two or more data types in declaration of `%s'", name);
      else if (TREE_CODE (id) == IDENTIFIER_NODE)
	{
	  register tree t = lookup_name (id);
	  if (!t || TREE_CODE (t) != TYPE_DECL)
	    error ("`%s' fails to be a typedef or built in type",
		   IDENTIFIER_POINTER (id));
	  else type = TREE_TYPE (t);
	}
      else if (TREE_CODE (id) != ERROR_MARK)
	type = id;

    found: {}
    }

  typedef_type = type;

  /* No type at all: default to `int', and set EXPLICIT_INT
     because it was not a user-defined typedef.  */

  if (type == 0)
    {
      /* Save warning until we know what is really going on.  */
      explicit_int = -1;
      type = integer_type_node;
      if (funcdef_flag && really_warn_return_type
	  && ! (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
			    | (1 << (int) RID_SIGNED) | (1 << (int) RID_UNSIGNED))))
	warn_about_return_type = 1;
    }

  /* Now process the modifiers that were specified
     and check for invalid combinations.  */

  /* Long double is a special combination.  */

  if ((specbits & 1 << (int) RID_LONG) && type == double_type_node)
    {
      specbits &= ~ (1 << (int) RID_LONG);
      type = long_double_type_node;
    }

  /* Check all other uses of type modifiers.  */

  if (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
		  | (1 << (int) RID_UNSIGNED) | (1 << (int) RID_SIGNED)))
    {
      if (!explicit_int && !explicit_char && !pedantic)
	error ("long, short, signed or unsigned used invalidly for `%s'", name);
      else if ((specbits & 1 << (int) RID_LONG) && (specbits & 1 << (int) RID_SHORT))
	error ("long and short specified together for `%s'", name);
      else if (((specbits & 1 << (int) RID_LONG) || (specbits & 1 << (int) RID_SHORT))
	       && explicit_char)
	error ("long or short specified with char for `%s'", name);
      else if ((specbits & 1 << (int) RID_SIGNED) && (specbits & 1 << (int) RID_UNSIGNED))
	error ("signed and unsigned given together for `%s'", name);
      else
	{
	  if (specbits & 1 << (int) RID_UNSIGNED)
	    {
	      if (specbits & 1 << (int) RID_LONG)
		type = long_unsigned_type_node;
	      else if (specbits & 1 << (int) RID_SHORT)
		type = short_unsigned_type_node;
	      else if (type == char_type_node)
		type = unsigned_char_type_node;
	      else
		type = unsigned_type_node;
	    }
	  else if ((specbits & 1 << (int) RID_SIGNED)
		   && type == char_type_node)
	    type = signed_char_type_node;
	  else if (specbits & 1 << (int) RID_LONG)
	    type = long_integer_type_node;
	  else if (specbits & 1 << (int) RID_SHORT)
	    type = short_integer_type_node;
	}
    }

  /* Set CONSTP if this declaration is `const', whether by
     explicit specification or via a typedef.
     Likewise for VOLATILEP.  */

  constp = !! (specbits & 1 << (int) RID_CONST) + TREE_READONLY (type);
  volatilep = !! (specbits & 1 << (int) RID_VOLATILE) + TREE_VOLATILE (type);
  inlinep = !! (specbits & (1 << (int) RID_INLINE));
  if (constp > 1)
    warning ("duplicate `const'");
  if (volatilep > 1)
    warning ("duplicate `volatile'");
  virtualp = specbits & (1 << (int) RID_VIRTUAL);
  friendp = specbits & (1 << (int) RID_FRIEND);
  specbits &= ~ ((1 << (int) RID_VIRTUAL) | (1 << (int) RID_FRIEND));

  if (! TREE_MEMBER_POINTER (type))
    type = TYPE_MAIN_VARIANT (type);

  /* Warn if two storage classes are given. Default to `auto'.  */

  if (specbits)
    {
      if (specbits & 1 << (int) RID_AUTO) nclasses++;
      if (specbits & ((1 << (int) RID_STATIC)|(1 << (int) RID_INLINE)))
	nclasses++;
      if (specbits & 1 << (int) RID_EXTERN) nclasses++;
      if (specbits & 1 << (int) RID_REGISTER) nclasses++;
      if (specbits & 1 << (int) RID_TYPEDEF) nclasses++;
    }

  /* Warn about storage classes that are invalid for certain
     kinds of declarations (parameters, typenames, etc.).  */

  if (nclasses > 1)
    error ("multiple storage classes in declaration of `%s'", name);
  else if (decl_context != NORMAL && nclasses > 0)
    {
      if (decl_context == PARM && (specbits & (1 << (int) RID_REGISTER)|(1 << (int) RID_AUTO)))
	;
      else if (decl_context == FIELD
	       && (specbits & ((1 << (int) RID_STATIC)|(1 << (int) RID_INLINE))))
	;			/* C++ allows static class elements */
      else if (decl_context == FIELD && (specbits == (1 << (int) RID_TYPEDEF)))
	{
	  /* A typedef which was made in a class's scope.  */
	  tree loc_typedecl;
	  /* keep `grokdeclarator' from thinking we are in PARM context.  */
	  pushlevel (0);
	  loc_typedecl = start_decl (declarator, declspecs, initialized);
	  poplevel (0, 0, 0);
	  if (TREE_CODE (TREE_TYPE (loc_typedecl)) == ENUMERAL_TYPE)
	    {
	      tree ref = lookup_tag (ENUMERAL_TYPE, DECL_NAME (loc_typedecl), current_binding_level, 0);
	      if (! ref)
		pushtag (DECL_NAME (loc_typedecl), TREE_TYPE (loc_typedecl));
	    }
	  return pushdecl (loc_typedecl);
	}
      else
	{
	  error ((decl_context == FIELD
		  ? "storage class specified for structure field `%s'"
		  : (decl_context == PARM
		     ? "storage class specified for parameter `%s'"
		     : "storage class specified for typename")),
		 name);
	  specbits &= ~ ((1 << (int) RID_TYPEDEF) | (1 << (int) RID_REGISTER)
			 | (1 << (int) RID_AUTO) | (1 << (int) RID_STATIC)
			 | (1 << (int) RID_EXTERN) | (1 << (int) RID_INLINE));
	}
    }
    else if (current_binding_level == global_binding_level)
      {
	if (specbits & (1 << (int) RID_AUTO))
	  error ("top-level declaration of `%s' specifies `auto'", name);
	if (specbits & (1 << (int) RID_REGISTER))
	  error ("top-level declaration of `%s' specifies `register'", name);
      }

  if (oldcname)
    ctype = TREE_TYPE (TREE_TYPE (oldcname));

  /* Now figure out the structure of the declarator proper.
     Descend through it, creating more complex types, until we reach
     the declared identifier (or NULL_TREE, in an absolute declarator).  */

  while (declarator)
    {
      /* Each level of DECLARATOR is either an ARRAY_REF (for ...[..]),
	 an INDIRECT_REF (for *...),
	 a CALL_EXPR (for ...(...)),
	 an identifier (for the name being declared)
	 or a null pointer (for the place in an absolute declarator
	 where the name was omitted).
	 For the last two cases, we have just exited the loop.

	 For C++ it could also be
	 a SCOPE_REF (for class :: ...). In this case, we pick off
	 the name being declared, and process class to see if it makes sense.
	 an ADDR_EXPR (for &...),
	 a BIT_NOT_EXPR (for destructors)
	 a TYPE_EXPR (for operator typenames)
	 a WRAPPER_EXPR (for wrappers)
	 an ANTI_WRAPPER_EXPR (for averting wrappers)

	 At this point, TYPE is the type of elements of an array,
	 or for a function to return, or for a pointer to point to.
	 After this sequence of ifs, TYPE is the type of the
	 array or function or pointer, and DECLARATOR has had its
	 outermost layer removed.  */

      if (type == error_mark_node
	  && TREE_CODE (declarator) != IDENTIFIER_NODE
	  && TREE_CODE (declarator) != OP_EXPR)
	{
	  declarator = TREE_OPERAND (declarator, 0);
	  continue;
	}
      else switch (TREE_CODE (declarator))
	{
	case IDENTIFIER_NODE:
	  goto loop_end;

	case ARRAY_REF:
	  {
	    register tree itype = NULL_TREE;
	    register tree size = TREE_OPERAND (declarator, 1);

	    declarator = TREE_OPERAND (declarator, 0);

	    /* Check for some types that there cannot be arrays of.  */

	    if (type == void_type_node)
	      {
		error ("declaration of `%s' as array of voids", name);
		type = error_mark_node;
	      }

	    if (TREE_CODE (type) == FUNCTION_TYPE)
	      {
		error ("declaration of `%s' as array of functions", name);
		type = error_mark_node;
	      }

	    if (size == error_mark_node)
	      type = error_mark_node;

	    if (type == error_mark_node)
	      continue;

	    if (declarator != 0 && TREE_CODE (declarator) != IDENTIFIER_NODE
		&& size == 0)
	      {
		if (TREE_CODE (declarator) != INDIRECT_REF
		    || pedantic)
		  warning ("declaration of `%s' invalidly omits array size", name);
		size = integer_one_node;
	      }

	    /* If size was specified, set ITYPE to a range-type for that size.
	       Otherwise, ITYPE remains null.  finish_decl may figure it out
	       from an initial value.  */

	    if (size)
	      {
		if (TREE_CODE (TREE_TYPE (size)) != INTEGER_TYPE
		    && TREE_CODE (TREE_TYPE (size)) != ENUMERAL_TYPE)
		  {
		    error ("size of array `%s' has non-integer type", name);
		    size = integer_one_node;
		  }
		else size = maybe_convert_decl_to_const (size);

		if (pedantic && integer_zerop (size))
		  warning ("ANSI C forbids zero-size array `%s'", name);
		if (INT_CST_LT (size, integer_zero_node))
		  {
		    error ("size of array `%s' is negative", name);
		    size = integer_one_node;
		  }
		if (TREE_LITERAL (size))
		  itype = make_index_type (build_int_2 (TREE_INT_CST_LOW (size) - 1, 0));
		else
		  {
		    if (pedantic)
		      warning ("ANSI C forbids variable-size array `%s'", name);
		    itype = build_binary_op (MINUS_EXPR, size, integer_one_node);
		    itype = make_index_type (itype);
		  }
	      }

	    /* Build the array type itself.  */

	    type = build_array_type (type, itype);
	  }
	  break;

	case CALL_EXPR:
	  {
	    tree parms;

	    /* Declaring a function type.
	       Make sure we have a valid type for the function to return.  */
	    /* Is this an error?  Should they be merged into TYPE here?  */
	    if (constp || volatilep)
	      warning ("function declared to return const or volatile result");
	    constp = 0;
	    volatilep = 0;

	    /* Warn about some types functions can't return.  */

	    if (TREE_CODE (type) == FUNCTION_TYPE)
	      {
		error ("`%s' declared as function returning a function", name);
		type = integer_type_node;
	      }
	    if (TREE_CODE (type) == ARRAY_TYPE)
	      {
		error ("`%s' declared as function returning an array", name);
		type = integer_type_node;
	      }

	    if (oldcname && oldcname == dname)
	      {
		/* We are within a class's scope. If our declarator name
		   is the same as the class name, and we are defining
		   a function, then it is a constructor/destructor, and
		   therefore returns a void type.  */

		if (flags != WRAPPER && flags != ANTI_WRAPPER)
		  if ((virtualp || friendp)
		      && flags != DESTRUCTOR)
		    {
		      error ("constructors cannot be declared virtual or friend");
		      virtualp = 0;
		      friendp = 0;
		    }
		  else if (specbits & ~((1 << (int) RID_INLINE)|(1 << (int) RID_VOID)|(1 << (int) RID_STATIC)))
		    error ("Return value type specifier for `%s' ignored",
			   flags == DESTRUCTOR ? "destructor" : "constructor");

		if (flags == DESTRUCTOR)
		  {
		    type = void_type_node;
		    if (decl_context == FIELD)
		      {
			if (cname && cname != oldcname)
			  {
			    error ("destructor for alien class `%s' cannot be a member",
				   IDENTIFIER_POINTER (cname));
			    return NULL_TREE;
			  }
			if (TREE_HAS_DESTRUCTOR (ctype))
			  {
			    error ("class `%s' already has destructor defined",
				   IDENTIFIER_POINTER (oldcname));
			  }
			TREE_HAS_DESTRUCTOR (ctype) = 1;
		      }
		  }
		else if (flags == WRAPPER || flags == ANTI_WRAPPER)
		  {
		    if (decl_context == FIELD)
		      {
			if (cname && cname != oldcname)
			  {
			    error ("wrapper for alien class `%s' cannot be member",
				   IDENTIFIER_POINTER (cname));
			    return NULL_TREE;
			  }
			TREE_HAS_WRAPPER (ctype) = 1;
		      }
		  }
		else
		  {
		    type = build_pointer_type (ctype);
		    if (decl_context == FIELD)
		      {
			if (cname && cname != oldcname)
			  {
			    error ("constructor for alien class `%s' cannot be member",
				   IDENTIFIER_POINTER (cname));
			    return NULL_TREE;
			  }
			TREE_HAS_CONSTRUCTOR (ctype) = 1;
		      }
		  }
	      }
	    else if (friendp && virtualp)
	      {
		/* Cannot be both friend and virtual.  */
		error ("virtual functions cannot be friends");
		specbits ^= (1 << (int) RID_FRIEND);
		friendp = 0;
	      }

	    /* Construct the function type and go to the next
	       inner layer of declarator.  */

	    parms = grokparms (TREE_OPERAND (declarator, 1), funcdef_flag);
	    declarator = TREE_OPERAND (declarator, 0);

	    if (declarator)
	      {
		/* Get past destructors, wrappers, etc.
		   We know we have one because FLAGS will be non-zero. */
		if (TREE_CODE (declarator) == BIT_NOT_EXPR)
		  declarator = TREE_OPERAND (declarator, 0);
		else if (TREE_CODE (declarator) == WRAPPER_EXPR)
		  declarator = wrapper_name;
		else if (TREE_CODE (declarator) == ANTI_WRAPPER_EXPR)
		  declarator = anti_wrapper_name;

		if (TREE_CODE (declarator) == SCOPE_REF)
		  {
		    tree t = TREE_OPERAND (declarator, 0);
		    declarator = TREE_OPERAND (declarator, 1);
		    if (t)
		      {
			t = TREE_TYPE (TREE_TYPE (t));
			if (TREE_CODE (declarator) == INDIRECT_REF)
			  {
			    parms = tree_cons (NULL_TREE, build_pointer_type (t), parms);
			    declarator = TREE_OPERAND (declarator, 0);
			    type = build_function_type (type, parms);
			    type = build_pointer_type (type);
			    type = build_type_variant (type, constp, volatilep, t);
			    constp = 0;
			    volatilep = 0;
			    continue;
			  }
		      }
		  }
		if (TREE_CODE (declarator) == OP_EXPR)
		  {
		    /* Now we know the number of parameters,
		       so build the real operator fnname.  */
		    declarator = build_operator_fnname (declarator, parms, friendp ? 0 : oldcname != NULL_TREE);
		    name = IDENTIFIER_POINTER (declarator);
		  }
		else if (TREE_CODE (declarator) == TYPE_EXPR)
		  {
		    tree oldtype = type;
		    /* Here's a trick: if this is a type conversion function,
		       then the return type of the function (though not
		       specified explicitly) is the name of the function.

		       @@ I do not remember what this code is for, or
		       @@ if it is even reachable anymore.  */

		    abort ();
		    type =
		      grokdeclarator (TREE_OPERAND (declarator, 1),
				      build_tree_list (NULL_TREE,
						       TREE_OPERAND (declarator, 0)),
				      TYPENAME, 0);
		    declarator = do_typename_overload (type);

		    TREE_TYPE (declarator) = type;

		    if (explicit_int >= 0 && oldtype != type)
		      error ("return type declaration for type conversion operator ignored",
			     IDENTIFIER_POINTER (declarator));
		  }
	      }
	    type = build_function_type (type, parms);
	  }
	  break;

	case INDIRECT_REF:
	  /* Merge any constancy or volatility into the target type
	     for the pointer.  */

	  if (constp || volatilep)
	    type = build_type_variant (type, constp, volatilep, 0);
	  constp = 0;
	  volatilep = 0;

	  type = build_pointer_type (type);

	  /* Process a list of type modifier keywords
	     (such as const or volatile) that were given inside the `*'.  */

	  if (TREE_TYPE (declarator))
	    {
	      register tree typemodlist;
	      int erred = 0;
	      for (typemodlist = TREE_TYPE (declarator); typemodlist;
		   typemodlist = TREE_CHAIN (typemodlist))
		{
		  if (TREE_VALUE (typemodlist) == ridpointers[(int) RID_CONST])
		    constp++;
		  else if (TREE_VALUE (typemodlist) == ridpointers[(int) RID_VOLATILE])
		    volatilep++;
		  else if (!erred)
		    {
		      erred = 1;
		      error ("invalid type modifier within pointer declarator");
		    }
		}
	      if (constp > 1)
		warning ("duplicate `const'");
	      if (volatilep > 1)
		warning ("duplicate `volatile'");
	    }
	  declarator = TREE_OPERAND (declarator, 0);
	  break;

	case ADDR_EXPR:
	  /* Merge any constancy or volatility into the target type
	     for the pointer.  */

	  if (constp || volatilep)
	    type = build_type_variant (type, constp, volatilep, 0);
	  constp = 0;
	  volatilep = 0;

	  type = build_reference_type (type);

	  /* Process a list of type modifier keywords
	     (such as const or volatile) that were given inside the `*'.  */

	  if (TREE_TYPE (declarator))
	    {
	      register tree typemodlist;
	      int erred = 0;
	      for (typemodlist = TREE_TYPE (declarator); typemodlist;
		   typemodlist = TREE_CHAIN (typemodlist))
		{
		  if (TREE_VALUE (typemodlist) == ridpointers[(int) RID_CONST])
		    constp++;
		  else if (TREE_VALUE (typemodlist) == ridpointers[(int) RID_VOLATILE])
		    volatilep++;
		  else if (!erred)
		    {
		      erred = 1;
		      error ("invalid type modifier within pointer declarator");
		    }
		}
	      if (constp > 1)
		warning ("duplicate `const'");
	      if (volatilep > 1)
		warning ("duplicate `volatile'");
	    }
	  declarator = TREE_OPERAND (declarator, 0);
	  break;

	case SCOPE_REF:
	  {
	    /* This could be a simple scope ref, or it could lead
	       through a `*' for a pointer to member construct.
	       Note that the grammar thoughfully insures that
	       the 0th TREE_OPERAND of `declarator' will be
	       an aggregate typedef.   `qtype' is the qualified type. */
	    tree qname = TREE_OPERAND (declarator, 0);
	    tree sname = TREE_OPERAND (declarator, 1);
	    tree qtype;

	    if (qname == NULL_TREE)
	      {
		/* If we were given a non-aggregate typedef,
		   then we cleared this out.  Just keep going
		   as though it wasn't there.  */
		declarator = sname;
		continue;
	      }

	    qtype = TREE_TYPE (TREE_TYPE (qname));
	    if (TREE_CODE (sname) == INDIRECT_REF)
	      {
		type = build_pointer_type (type);
		type = build_type_variant (type, constp, volatilep, qtype);
		constp = 0;
		volatilep = 0;
		declarator = TREE_OPERAND (sname, 0);
		continue;
	      }
	    else if (TREE_CODE (sname) == ADDR_EXPR)
	      {
		sorry ("references to members not implemented yet");
		declarator = TREE_OPERAND (sname, 0);
		continue;
	      }
	    else
	      {
		/* This is the `standard' use of the scoping operator:
		   basetype :: member .  This is reasonable only if the
		   scope is the name of a base class of the type we
		   are deriving from.  */
		if (cname != oldcname)
		  ctype = TREE_TYPE (TREE_TYPE (cname));
		if (check_base_type (qtype, ctype))
		  if (TREE_CODE (sname) == TYPE_EXPR)
		    {
		      /* This also comes out of order: the grammar does not
			 build these exprs in the way that is most
			 useful for grokdeclarator to handle.  Fix
			 SCOPE_REF in place.  */
		      sname = grokoptypename (sname, 1);
		      type = TREE_TYPE (TREE_OPERAND (sname, 0));
		      TREE_OPERAND (declarator, 1) = TREE_OPERAND (sname, 0);
		      TREE_OPERAND (sname, 0) = declarator;
		      declarator = sname;
		      continue;
		    }
		  else if (decl_context == FIELD)
		    {
		      if (TYPE_SIZE (qtype))
			{
			  tree t;
			  if (TREE_CODE (sname) == CALL_EXPR
			      && (t = get_fnfields (qtype, TREE_OPERAND (sname, 0))))
			    {
			      t = TREE_VALUE (t);
			      while (t)
				{
				  if (TREE_TYPE (TREE_TYPE (t)) == type)
				    break;
				  t = TYPE_BASELINK (t);
				}
			    }
			  else if (TREE_CODE (sname) == IDENTIFIER_NODE)
			    {
			      t = get_field (qtype, sname);
			      if (! t)
				t = get_fnfields (qtype, sname);
			    }
			  else
			    abort ();

			  if (friendp)
			    return do_friend (qname, sname, qtype, flags);

			  if (t)
			    return build_decl (FIELD_DECL, type,
					       build_nt (SCOPE_REF, qtype, t));

			  if (flags == OP_TYPENAME)
			    error ("type conversion is not a member of structure `%s'",
				   IDENTIFIER_POINTER (qname));
			  else
			    error ("field `%s' is not a member of structure `%s'",
				   IDENTIFIER_POINTER (sname),
				   IDENTIFIER_POINTER (qname));
			}
		      else
			sorry ("structure `%s' not yet defined",
			       IDENTIFIER_POINTER (qname));
		    }
		  else
		    error ("invalid scope request `%s::%s'",
			   IDENTIFIER_POINTER (qname),
			   TREE_CODE (sname) == IDENTIFIER_NODE
			   ? IDENTIFIER_POINTER (sname)
			   : "(type conversion)");
		else
		  error ("invalid scope request: aggregate `%s' is not derived from `%s'",
			 IDENTIFIER_POINTER (qname),
			 IDENTIFIER_POINTER (cname));
		declarator = sname;
	      }
	  }
	  break;

	case OP_EXPR:
	  declarator = grokopexpr (declarator, 1);
	  if (declarator == NULL_TREE)
	    return NULL_TREE;
	  name = IDENTIFIER_POINTER (declarator);
	  break;

	case BIT_NOT_EXPR:
	  declarator = TREE_OPERAND (declarator, 0);
	  break;

	case TYPE_EXPR:
	  declarator = grokoptypename (declarator, 1);
	  type = TREE_TYPE (TREE_OPERAND (declarator, 0));
	  break;

	case WRAPPER_EXPR:
	case ANTI_WRAPPER_EXPR:
	  error ("wrapper encountered");
	  declarator = TREE_OPERAND (declarator, 0);
	  break;

	default:
	  abort ();
	}

      /* layout_type (type); */

      /* @@ Should perhaps replace the following code by changes in
       * @@ stor_layout.c. */
      if (TREE_CODE (type) == FUNCTION_DECL)
	{
	  /* A function variable in C should be Pmode rather than EPmode
	     because it has just the address of a function, no static chain.*/
	  TYPE_MODE (type) = Pmode;
	}
    }

 loop_end:

  /* Now TYPE has the actual type.  */

  /* If this is declaring a typedef name, return a TYPE_DECL.  */

  if (specbits & (1 << (int) RID_TYPEDEF))
    {
      /* Note that the grammar rejects storage classes
	 in typenames, fields or parameters.  */
      if (constp || volatilep)
	type = build_type_variant (type, constp, volatilep, 0);

      /* If the user declares "struct {...} foo" then `foo' will have
	 an anonymous name.  Fill that name in now.  Nothing can
	 refer to it, so nothing needs know about the name change.
	 The TYPE_NAME field was filled in by build_struct_xref.  */
      if (IS_AGGR_TYPE (type)
	  && ANON_AGGRNAME_P (DECL_NAME (TYPE_NAME (type))))
	{
	  /* replace the anonymous name with the real name everywhere.  */
	  lookup_tag_reverse (type, declarator);
	  DECL_NAME (TYPE_NAME (type)) = declarator;
	}

      return build_decl (TYPE_DECL, declarator, type);
    }

  /* Detect the case of an array type of unspecified size
     which came, as such, direct from a typedef name.
     We must copy the type, so that each identifier gets
     a distinct type, so that each identifier's size can be
     controlled separately by its own initializer.  */

  if (type == typedef_type && TREE_CODE (type) == ARRAY_TYPE
      && TYPE_DOMAIN (type) == 0)
    {
      type = build_array_type (TREE_TYPE (type), TYPE_DOMAIN (type));
    }

  /* If this is a type name (such as, in a cast or sizeof),
     compute the type and return it now.  */

  if (decl_context == TYPENAME)
    {
      /* Note that the grammar rejects storage classes
	 in typenames, fields or parameters.  */
      if (constp || volatilep)
	type = build_type_variant (type, constp, volatilep, 0);

      /* Special case: "friend class foo" looks like a TYPENAME context.  */
      if (friendp)
	{
	  /* A friendly class?  */
	  make_friend_class (current_class_type, type);
	  return void_type_node;
	}

      return type;
    }

  /* `void' at top level (not within pointer)
     is allowed only in typedefs or type names.
     We don't complain about parms either, but that is because
     a better error message can be made later.

     For C++, we will have methods which may return void.
     To keep from emitting spurious error messages, supress
     the following code with a check that type is not FUNCITON_TYPE */

  if (type == void_type_node && decl_context != PARM)
    {
      error ("variable or field `%s' declared void",
	     IDENTIFIER_POINTER (declarator));
      type = integer_type_node;
    }

  /* Now create the decl, which may be a VAR_DECL, a PARM_DECL
     or a FUNCTION_DECL, depending on DECL_CONTEXT and TYPE.  */

  {
    register tree decl;

    if (decl_context == PARM)
      {
	if (cname && ! TREE_MEMBER_POINTER (type))
	  error ("cannot use `::' in parameter declaration");

	/* A parameter declared as an array of T is really a pointer to T.
	   One declared as a function is really a pointer to a function.  */

	if (TREE_CODE (type) == ARRAY_TYPE)
	  {
	    /* Transfer const-ness of array into that of type pointed to.  */
	    type = build_pointer_type
		    (build_type_variant (TREE_TYPE (type), constp, volatilep, 0));
	    volatilep = constp = 0;
	  }
	else if (TYPE_SIZE (type) == 0
		 && type != current_class_type)
	  {
	    if (declarator)
	      error ("parameter `%s' has incomplete type",
		     IDENTIFIER_POINTER (declarator));
	    else
	      incomplete_type_error (0, type);
	    type = error_mark_node;
	  }

	decl = build_decl (PARM_DECL, declarator, type);

	/* Compute the type actually passed in the parmlist,
	   for the case where there is no prototype.
	   (For example, shorts and chars are passed as ints.)
	   When there is a prototype, this is overridden later.  */

	DECL_ARG_TYPE (decl) = type;
	if (type == float_type_node)
	  DECL_ARG_TYPE (decl) = double_type_node;
	else if (TREE_CODE (type) == INTEGER_TYPE
		 && TYPE_PRECISION (type) < TYPE_PRECISION (integer_type_node))
	  DECL_ARG_TYPE (decl) = integer_type_node;

	if (virtualp)
	  {
	    error ("virtual non function `%s'", IDENTIFIER_POINTER (declarator));
	    virtualp = 0;
	  }
      }
    else if (decl_context == FIELD)
      {
	if (TREE_CODE (type) == FUNCTION_TYPE)
	  {
	    type = build_decl (FUNCTION_DECL, declarator, type);
	    DECL_ARGUMENTS (type) = last_function_parms;
	    TREE_INLINE (type) = inlinep;
	  }
	else if (TYPE_SIZE (type) == 0)
	  {
	    error ("field `%s' has incomplete type",
		   IDENTIFIER_POINTER (declarator));
	    type = error_mark_node;
	  }

	if (friendp)
	  {
	    /* Friends are treated specially.  */
	    if (cname == current_class_name)
	      warning ("member functions are implicitly friends of their class");
	    else
	      return do_friend (cname, declarator, type, flags);
	  }

	/* Structure field.  It may not be a function, except for C++ */
	decl = NULL_TREE;

	if (TREE_CODE (type) == FUNCTION_DECL)
	  {
	    tree tmp;

	    /* Function gets the ugly name, field gets the nice one.
	       This call may change the type of the function ! */
	    grokclassfn (current_class_name, type, flags, 0);

	    /* Now install the declaration of this function so that
	       others may find it (esp. its DECL_FRIENDLIST).
	       Pretend we are at top level, we will get true
	       reference later, perhaps.  */
	    type = pushdecl (type);
	    rest_of_decl_compilation (type, NULL_TREE, 1, 0);

	    /* If this declaration supersedes the declaration of
	       a method declared virtual in the base class, then
	       mark this field as being virtual as well.  */
	    tmp = TYPE_BASELINK (ctype);
	    if (tmp && (TREE_VIRTUAL (tmp)
			|| (flag_all_virtual
			    && declarator != current_class_name)))
	      {
		/* Destructors do not match by name.  */
		if (flags == DESTRUCTOR)
		  {
		    while (tmp && TREE_NEEDS_DESTRUCTOR (tmp)
			   && ! TREE_HAS_DESTRUCTOR (tmp))
		      tmp = TYPE_BASELINK (tmp);
		    /* Found a dtor... See if it is virtual.  */
		    if (tmp && TREE_HAS_DESTRUCTOR (tmp))
		      {
			tmp = TREE_VALUE (TYPE_FN_FIELDS (tmp));
			if (TREE_VIRTUAL (tmp))
			  {
			    virtualp = 1;
			    decl = build_decl (FIELD_DECL, declarator, type);
			    SET_DECL_VINDEX (decl, DECL_VINDEX (tmp));
			    DECL_VOFFSET (decl) = TREE_TYPE (tmp);
			  }
		      }
		  }
		else if ((tmp = get_fnfields (tmp, declarator))
			 && TREE_VIRTUAL (TREE_VALUE (tmp)))
		  {
		    tree dtypes, btypes;
		    int matched;
		    tree tmp1 = TREE_VALUE (tmp);

		    dtypes = TYPE_ARG_TYPES (TREE_TYPE (type));
		    while (tmp1)
		      {
			btypes = TYPE_ARG_TYPES (TREE_TYPE (TREE_TYPE (tmp1)));
			if (compparms (TREE_CHAIN (btypes), TREE_CHAIN (dtypes), 1))
			  break;
			tmp1 = TREE_CHAIN (tmp1);
		      }
		    if (! tmp1)
		      {
			tree tmp2 = NULL_TREE;
			char buf[OVERLOAD_MAX_LEN];

			tmp1 = TREE_VALUE (tmp);
			while (tmp1)
			  {
			    btypes = TYPE_ARG_TYPES (TREE_TYPE (TREE_TYPE (tmp1)));
			    if (compparms (TREE_CHAIN (btypes), TREE_CHAIN (dtypes), 0))
			      {
				tmp2 = TREE_CHAIN (tmp1);
				break;
			      }
			    tmp1 = TREE_CHAIN (tmp1);
			  }
			while (tmp2)
			  {
			    btypes = TYPE_ARG_TYPES (TREE_TYPE (TREE_TYPE (tmp2)));
			    if (compparms (TREE_CHAIN (btypes), TREE_CHAIN (dtypes), 0))
			      break;
			    tmp2 = TREE_CHAIN (tmp2);
			  }
			if (tmp1 == NULL_TREE || tmp2)
			  {
			    hack_function_decl (buf, cname, type, 0);
			    if (tmp1 == NULL_TREE)
			      error ("type of virtual function `%s' does not match any in base class", buf);
			    else
			      error ("type of virtual function `%s' is ambiguously related to functions in base class", buf);
			  }
		      }
		    else if (TREE_VIRTUAL (tmp1)
			     || (flag_all_virtual && declarator != current_class_name))
		      {
			virtualp = 1;
			decl = build_decl (FIELD_DECL, declarator, type);
			SET_DECL_VINDEX (decl, DECL_VINDEX (tmp1));
			DECL_VOFFSET (decl) = TREE_TYPE (tmp1);
		      }
		  }
	      }
	  }
	else
	  {
	    if (virtualp)
	      {
		error ("virtual non function declaration for field `%s'",
		       IDENTIFIER_POINTER (declarator));
		virtualp = 0;
	      }
	  }
	if (decl == NULL_TREE)
	  decl = build_decl (FIELD_DECL, declarator, type);
	    /* C++ allows static class members.  */
	if (specbits & (1 << (int) RID_STATIC))
	  if (TREE_CODE (type) == FUNCTION_DECL)
	    {
	      warning ("function declared static in class");
	      TREE_PUBLIC (type) = 0;
	    }
	  else
	    TREE_STATIC (decl) = 1;
      }
    else if (TREE_CODE (type) == FUNCTION_TYPE)
      {
	int was_overloaded = 0;
	tree original_name = declarator;

	if (! declarator) return NULL_TREE;

	if (specbits & ((1 << (int) RID_AUTO) | (1 << (int) RID_REGISTER)))
	  error ("invalid storage class for function `%s'",
		 IDENTIFIER_POINTER (declarator));
	/* Function declaration not at top level.
	   Storage classes other than `extern' are not allowed
	   and `extern' makes no difference.  */
	if (current_binding_level != global_binding_level
	    && (specbits & ((1 << (int) RID_STATIC) | (1 << (int) RID_INLINE)))
	    && pedantic)
	  warning ("invalid storage class for function `%s'",
		   IDENTIFIER_POINTER (declarator));

	if (! cname)
	  {
	    if (virtualp)
	      {
		error ("virtual non-class function `%s'",
		       IDENTIFIER_POINTER (declarator));
		virtualp = 0;
	      }
	    if (is_overloaded (declarator))
	      {
		/* Plain overloading: will not be grok'd by grokclassfn.  */
		declarator = do_decl_overload (name, TYPE_ARG_TYPES (type));
		was_overloaded = 1;
	      }
	  }
	decl = build_decl (FUNCTION_DECL, declarator, type);
	TREE_EXTERNAL (decl) = 1;
	/* Record presence of `static'.  In C++, `inline' is like `static'.  */
	TREE_PUBLIC (decl) = !(specbits & ((1 << (int) RID_STATIC) | (1 << (int) RID_INLINE)));
	/* Record presence of `inline', if it is reasonable.  */
	if (inlinep)
	  {
	    tree last = tree_last (TYPE_ARG_TYPES (type));

	    if (! was_overloaded
		&& ! cname
		&& ! strcmp (IDENTIFIER_POINTER (original_name), "main"))
	      warning ("cannot inline function `main'");
	    else if (last && TREE_VALUE (last) != void_type_node)
	      error ("inline declaration ignored for function with `...'");
	    else
	      {
		/* Assume that otherwise the function can be inlined.  */
		TREE_INLINE (decl) = 1;
	      }
	  }
	if (was_overloaded)
	  {
	    TREE_OVERLOADED (decl) = 1;
	    DECL_ORIGINAL_NAME (decl) = original_name;
	  }

	/* must do this because start_function won't.  */
	if (cname)
	  {
	    /* This is a top level definition of a method.  */
	    grokclassfn (cname, decl, flags, 1);
	    decl = build_nt (SCOPE_REF, cname, decl);
	  }
      }
    else
      {
	/* It's a variable.  */
	enum tree_code code;

	if (!flag_const_is_variable && constp
	    && TYPE_MODE (type) != BLKmode
	    && ! IS_AGGR_TYPE (type)
	    && initialized
	    && (specbits & (1 << (int) RID_STATIC)))
	  code = CONST_DECL;
	else
	  code = VAR_DECL;

	decl = build_decl (code, declarator, type);

	if (inlinep)
	  warning_with_decl (decl, "variable `%s' declared `inline'");

	/* An uninitialized decl with `extern' is a reference.  */
	TREE_EXTERNAL (decl)
	  = !initialized && (specbits & (1 << (int) RID_EXTERN));
	/* At top level, either `static' or no s.c. makes a definition
	   (perhaps tentative), and absence of `static' makes it public.  */
	if (current_binding_level == global_binding_level)
	  {
	    TREE_PUBLIC (decl) = !(specbits & (1 << (int) RID_STATIC));
	    TREE_STATIC (decl) = ! TREE_EXTERNAL (decl);
	  }
	/* Not at top level, only `static' makes a static definition.  */
	else
	  {
	    TREE_STATIC (decl) = (specbits & (1 << (int) RID_STATIC)) != 0;
	    /* `extern' with initialization is invalid if not at top level.  */
	    if ((specbits & (1 << (int) RID_EXTERN)) && initialized)
	      error ("`%s' has both `extern' and initializer", name);
	  }
      }

    /* Record `register' declaration for warnings on &
       and in case doing stupid register allocation.  */

    if (specbits & (1 << (int) RID_REGISTER))
      TREE_REGDECL (decl) = 1;

    /* Record constancy and volatility.  */

    if (constp)
      TREE_READONLY (decl) = 1;
    if (volatilep)
      {
	TREE_VOLATILE (decl) = 1;
	TREE_THIS_VOLATILE (decl) = 1;
      }
    if (virtualp)
      TREE_VIRTUAL (decl) = 1;
    if (friendp && (TREE_CODE (decl) != FIELD_DECL))
      error ("illegal friend declaration");

    return decl;
  }
}

/* Classes overload their constituent function names automatically.
   When a function name is declared in a record structure,
   its name is changed to it overloaded name. Note that methods
   within classes receive as their first argument, a pointer
   to their instance variable. The type of this variable
   is unique up to class type, so the actual name of the function
   does not need to be changed. Since constructors and
   destructors can conflict, we place a leading '$' for destructors.

   CNAME is the name of the class we are grokking for.

   FUNCTION is a FUNCTION_DECL.  It was created by `grokdeclarator',
   but we may return something else, namely, the function decl
   which already exists within the class definition we are
   grokking for.

   FN is a FUNCTION_TYPE.  Because we are now hashing these things
   (and hence saving store), it is important that this function
   not side-effect the function which may already be hashed.

   FLAGS contains bits saying what's special about today's
   arguments.  1 == DESTRUCTOR.  2 == OPERATOR.

   For C++, if this function is really a class method, then we need to
   add the class instance variable (this) up front.  If it is a
   destructor, then we must also add the `auto-delete' field.
   There is some hair associated with the fact that we must
   "declare" this variable in the manner consistent with the
   way the rest of the arguements were declared.  */

void
grokclassfn (cname, function, flags, complain)
     tree cname, function;
     enum overload_flags flags;
{
  tree fn_name = DECL_NAME (function);
  tree fn = TREE_TYPE (function);
  tree vname, type, parm, arg_types;
  tree ctype = TREE_TYPE (TREE_TYPE (cname));
  char *err_name;

  /* Let parameter passing mechanisms know that this
     is a method.  */
  TREE_METHOD (function) = 1;

  /* Initialize name for error reporting.  */
  if (OPERATOR_NAME_P (fn_name))
    {
      err_name = (char *)alloca (OVERLOAD_MAX_LEN);
      sprintf (err_name, "operator %s", OPERATOR_NAME_STRING (function));
    }
  else
    err_name = IDENTIFIER_POINTER (fn_name);

  type = build_pointer_type (ctype);

#if 0
  /* This code disabled because we currently
     need it for a dirty hack.  */
  if (DECL_NAME (function) != cname)
    {
      /* Only c'tors and d'tors are allowed to assign to `this'.
	 We may also want to pretend it is volatile.  */
	 type = build_type_variant (type, 1, 1, 0);
    }
#endif

  /* Need the instance variable up front */
  arg_types = tree_cons (NULL_TREE, type, TYPE_ARG_TYPES (fn));
  vname = get_identifier (THIS_NAME);

  if (! last_function_parms
      || TREE_CODE (last_function_parms) == PARM_DECL)
    {
      /* If there are no last_function_parms, pretend that there
	 were, and that the function was prototyped.  */
      parm = build_decl (PARM_DECL, vname, type);
      DECL_ARG_TYPE (parm) = type;
      TREE_CHAIN (parm) = last_function_parms;
      if (! DECL_ARGUMENTS (function)
	  || DECL_ARGUMENTS (function) == last_function_parms)
	DECL_ARGUMENTS (function) = parm;
      else
	{
	  tree alt_parm = copy_node (parm);
	  fatal ("DECL_ARGUMENTS and last_function_parms are probably out of synch");
	  TREE_CHAIN (parm) = DECL_ARGUMENTS (function);
	  DECL_ARGUMENTS (function) = parm;
	}
      last_function_parms = parm;
    }
  else
    {
      /* Pretend that these were declared using old-style specs.
         Make a spot in the decls;  grokdecl will know what to do.  */
      parm = start_decl (vname, build_tree_list (NULL_TREE, type), 0);
      if (TREE_CODE (parm) != PARM_DECL)
	abort ();
      finish_decl (parm, NULL_TREE, NULL_TREE);
      parm = build_tree_list (type, vname);
      TREE_CHAIN (parm) = last_function_parms;
      last_function_parms = parm;
    }

  if (flags == DESTRUCTOR)
    {
      tree const_integer_type = build_type_variant (integer_type_node, 1, 0, 0);
      char name[OVERLOAD_MAX_LEN];
      sprintf (name, DESTRUCTOR_DECL_FORMAT, IDENTIFIER_POINTER (cname));
      DECL_NAME (function) = get_identifier (name);

      /* we know that destructors only have one argument here
	 so just use TREE_CHAIN field instead of chainon.  */
      TREE_CHAIN (arg_types) =
	tree_cons (NULL_TREE, integer_type_node,
		   build_tree_list (NULL_TREE, void_type_node));
      vname = get_identifier (AUTO_DELETE_NAME);
      parm = build_decl (PARM_DECL, vname, const_integer_type);
      TREE_EVERUSED (parm) = 1;
      TREE_READONLY (parm) = 1;
      DECL_ARG_TYPE (parm) = const_integer_type;
      /* This is the same chain as DECL_ARGUMENTS (fndecl).  */
      TREE_CHAIN (last_function_parms) = parm;
    }
  else
    {
      DECL_NAME (function) = do_decl_overload (IDENTIFIER_POINTER (fn_name),
					       arg_types);
      if (flags == OP_TYPENAME)
	TREE_TYPE (DECL_NAME (function)) = TREE_TYPE (fn_name);

      if (WRAPPER_NAME_P (fn_name) || ANTI_WRAPPER_NAME_P (fn_name))
	DECL_ORIGINAL_NAME (function) = cname;
    }

  fn = build_function_type (TREE_TYPE (fn), arg_types);
  TREE_TYPE (function) = fn;

  /* now, the sanity check: report error if this function is not
     really a member of the class it is supposed to belong to.  */
  if (complain)
    {
      tree field, fields;
      int any_friends = 0;

      fields = TYPE_FN_FIELDS (ctype);
      while (fields)
	{
	  if (fn_name == TREE_PURPOSE (fields))
	    {
	      any_friends = !! TREE_TYPE (fields);
	      field = TREE_VALUE (fields);
	      while (field)
		{
		  if (DECL_NAME (function) == DECL_NAME (TREE_TYPE (field)))
		    return;
		  field = TREE_CHAIN (field);
		}
	      break;		/* loser */
	    }
	  fields = TREE_CHAIN (fields);
	}
      if (fields)
	if (any_friends)
	  error ("argument list for `%s' does not match any methods or friends of class", err_name);
	else
	  error ("argument list for `%s' does not match any in class", err_name);
      else
	error ("no method named `%s' in class", err_name);

      /* If we did not find the method in the class, add it to
	 avoid spurious errors.  */
      add_method (ctype, fields, function);
    }
}

/* The precedence rules of this grammar (or any other deterministic LALR
   grammar, for that matter), place the CALL_EXPR somewhere where we
   may not want it.  The solution is to grab the first CALL_EXPR we see,
   pretend that that is the one that belongs to the parameter list of
   the type conversion function, and leave everything else alone.
   We pull it out in place.

   CALL_REQUIRED is non-zero if we should complain if a CALL_EXPR
   does not appear in DECL.  */
tree
grokoptypename (decl, call_required)
     tree decl;
     int call_required;
{
  tree tmp, last;
  tree parms;

  if (TREE_CODE (decl) != TYPE_EXPR)
    abort ();

  tmp = TREE_OPERAND (decl, 1);
  last = NULL_TREE;

  while (tmp)
    {
      switch (TREE_CODE (tmp))
	{
	case CALL_EXPR:
	  if (last)
	    TREE_OPERAND (last, 0) = TREE_OPERAND (tmp, 0);
	  else
	    TREE_OPERAND (decl, 1) = TREE_OPERAND (tmp, 0);
	  if (TREE_OPERAND (tmp, 1)
	      && TREE_VALUE (TREE_OPERAND (tmp, 1)) != void_type_node)
	    {
	      error ("operator <typename> requires empty parameter list");
	      TREE_OPERAND (tmp, 1) = build_tree_list (NULL_TREE, void_type_node);
	    }
	  last = grokdeclarator (TREE_OPERAND (decl, 1),
				 TREE_OPERAND (decl, 0),
				 TYPENAME, 0);
	  TREE_OPERAND (tmp, 0) = do_typename_overload (last);
	  TREE_TYPE (TREE_OPERAND (tmp, 0)) = last;
	  return tmp;

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

  if (call_required)
    error ("operator <typename> construct requires parameter list");

  last = grokdeclarator (TREE_OPERAND (decl, 1),
			TREE_OPERAND (decl, 0),
			TYPENAME, 0);
  tmp = build_nt (CALL_EXPR, do_typename_overload (last),
		  build_tree_list (NULL_TREE, void_type_node), 0);
  TREE_TYPE (TREE_OPERAND (tmp, 0)) = last;
  return tmp;
}

/* Given an encoding for an operator name (see parse.y, the rules
   for making an `operator_name' in the variable DECLARATOR,
   return the name of the operator prefix as an IDENTIFIER_NODE.
   If REPORT_AMBIGUOUS is non-zero, an error message
   is reported, and a default arity of the operator is
   returned.  Otherwise, return the operator under an OP_EXPR,
   for later evaluation when type information will enable
   proper instantiation.  */
tree
grokopexpr (declarator, report_ambiguous)
     tree declarator;
     int report_ambiguous;
{
  switch ((int)TREE_OPERAND (declarator, 0))
    {
    case '+':
    case '&':
    case '*':
    case '-':
      if (report_ambiguous)
	{
	  error ("operator '%c' ambiguous, (default binary)",
		 (int)TREE_OPERAND (declarator, 0));
	  declarator = build_operator_fnname (declarator, NULL_TREE, 2);
	}
      else
	{
	  /* do something intellegent.  */
	  return build (OP_EXPR, unknown_type_node, declarator);
	}
      break;
    default:
      declarator = build_operator_fnname (declarator, NULL_TREE, -1);
      break;
    }
  return declarator;
}

/* Create a type of integers to be the TYPE_DOMAIN of an ARRAY_TYPE.
   MAXVAL should be the maximum value in the domain
   (one less than the length of the array).  */

tree
make_index_type (maxval)
     tree maxval;
{
  register tree itype = make_node (INTEGER_TYPE);
  int maxint = TREE_INT_CST_LOW (maxval);
  TYPE_PRECISION (itype) = BITS_PER_WORD;
  TYPE_MIN_VALUE (itype) = build_int_2 (0, 0);
  TREE_TYPE (TYPE_MIN_VALUE (itype)) = itype;
  TYPE_MAX_VALUE (itype) = maxval;
  TREE_TYPE (maxval) = itype;
  TYPE_MODE (itype) = SImode;
  TYPE_SIZE (itype) = TYPE_SIZE (long_integer_type_node);
  TYPE_SIZE_UNIT (itype) = TYPE_SIZE_UNIT (long_integer_type_node);
  TYPE_ALIGN (itype) = TYPE_ALIGN (long_integer_type_node);
  return type_hash_canon (maxint > 0 ? maxint : - maxint, itype);
}

/* Tell if a parmlist/exprlist looks like an exprlist or a parmlist.
   An empty exprlist is a parmlist.  An exprlist which
   contains only identifiers at the global level
   is a parmlist.  Otherwise, it is an exprlist. */
static int
parmlist_is_exprlist (exprs)
     tree exprs;
{
  tree decl;

  if (exprs == NULL_TREE || TREE_PARMLIST (exprs))
    return 0;

  if (current_binding_level == global_binding_level)
    {
      /* At the global level, if these are all identifiers,
	 then it is a parmlist.  */
      while (exprs)
	{
	  if (TREE_CODE (TREE_VALUE (exprs)) != IDENTIFIER_NODE)
	    return 1;
	  exprs = TREE_CHAIN (exprs);
	}
      return 0;
    }
  return 1;
}

/* Make sure that the this list of PARMS has a chance of being
   grokked by `grokparms'.

   @@ This is really weak, but the grammar does not allow us
   @@ to easily reject things that this has to catch as syntax errors.  */
static int
parmlist_is_random (parms)
     tree parms;
{
  if (parms == NULL_TREE)
    return 0;

  if (TREE_CODE (parms) != TREE_LIST)
    return 1;

  while (parms)
    {
      if (TREE_VALUE (parms) == void_type_node)
	return 0;

      if (TREE_CODE (TREE_VALUE (parms)) != TREE_LIST)
	return 1;
      parms = TREE_CHAIN (parms);
    }
  return 0;
}

/* Decode the list of parameter types for a function type.
   Given the list of things declared inside the parens,
   return a list of types.

   The list we receive can have three kinds of elements:
   an IDENTIFIER_NODE for names given without types,
   a TREE_LIST node for arguments given as typespecs or names with typespecs,
   or void_type_node, to mark the end of an argument list
   when additional arguments are not permitted (... was not used).

   FUNCDEF_FLAG is nonzero for a function definition, 0 for
   a mere declaration.  A nonempty identifier-list gets an error message
   when FUNCDEF_FLAG is zero.

   If all elements of the input list contain types,
   we return a list of the types.
   If all elements contain no type (except perhaps a void_type_node
   at the end), we return a null list.
   If some have types and some do not, it is an error, and we
   return a null list.

   Also set last_function_parms to either
   a list of names (IDENTIFIER_NODEs) or a chain of PARM_DECLs.
   A list of names is converted to a chain of PARM_DECLs
   by store_parm_decls so that ultimately it is always a chain of decls.

   Note that in C++, paramters can take default values.  These default
   values are in the TREE_PURPOSE field of the TREE_LIST.  It is
   an error to specify default values which are followed by parameters
   that have no defualt values, or an ELLIPSES.  For simplicities sake,
   only parameters which are specified with their types can take on
   default values.  */

static tree
grokparms (first_parm, funcdef_flag)
     tree first_parm;
     int funcdef_flag;
{
  tree result = NULL_TREE;
  tree names = NULL_TREE;
  tree decls = NULL_TREE;

  if (first_parm == 0
      || TREE_CODE (TREE_VALUE (first_parm)) == IDENTIFIER_NODE)
    {
      if (! funcdef_flag && first_parm != 0)
	{
	  warning ("parameter names (without types) in function declaration");
	  return 0;
	}
      last_function_parms = first_parm;
      return 0;
    }
  else
    {
      /* Types were specified.  This is a list of declarators
	 each represented as a TREE_LIST node.  */
      register tree parm;
      int any_init = 0, any_error = 0, saw_void = 0;

      for (parm = first_parm;
	   parm != NULL_TREE;
	   parm = TREE_CHAIN (parm))
	{
	  register tree decl = TREE_VALUE (parm);
	  tree init = TREE_PURPOSE (parm);

	  /* @@ weak defense agains error.  */
	  if (decl == error_mark_node)
	    continue;

	  if (decl != void_type_node)
	    decl = grokdeclarator (TREE_VALUE (decl),
				   TREE_PURPOSE (decl),
				   PARM, 0);
	  if (! decl) continue;
	  if (decl == void_type_node)
	    {
	      result = chainon (result, build_tree_list (0, void_type_node));
	      saw_void = 1;
	      break;
	    }

	  /* Since there is a prototype, args are passed in their own types.  */
	  DECL_ARG_TYPE (decl) = TREE_TYPE (decl);
	  if (!any_error)
	    {
	      if (init)
		{
		  /* If init contains expressions which have addresses
		     (such as a string), then we must save their
		     rtx's on the permanent obstack.  This is done
		     by calling make_rtx_constant_now on the
		     expression here, at its first occurance.  If this
		     expression is referenced later, the rtl created here
		     will be the rtl that is returned.  */
		  make_rtx_constant_now (init);
		  any_init++;
		}
	      else if (any_init)
		{
		  error ("all trailing parameters must have default arguments");
		  any_error = 1;
		}
	    }
	  else
	    init = NULL_TREE;

	  decls = chainon (decls, decl);
	  result = chainon (result, build_tree_list (init, TREE_TYPE (decl)));
	}
      if (any_init && ! saw_void)
	{
	  error ("default arguments cannot precede ELLIPSES");
	  any_error = 1;
	}
    }

  last_function_parms = decls;
  return result;
}

/* These memoizing functions keep track of special properties which
   a class may have.  `grok_ctor_properties' notices whether a class
   has a constructor of the for X(X&), and also complains
   if the class has a constructor of the form X(X).
   `grok_op_properties' takes notice of the various forms of
   operator= which are defined, as well as what sorts of type conversion
   may apply.  Both functions take a FUNCTION_DECL as an argument.  */
static void
grok_ctor_properties (decl)
     tree decl;
{
  tree parmtypes = TREE_CHAIN (TYPE_ARG_TYPES (TREE_TYPE (decl)));
  tree parmtype = parmtypes ? TREE_VALUE (parmtypes) : void_type_node;

  if (TREE_CODE (parmtype) == REFERENCE_TYPE
      && TYPE_MAIN_VARIANT (TREE_TYPE (parmtype)) == current_class_type)
    if (TREE_CHAIN (parmtypes) == NULL_TREE
	|| TREE_VALUE (TREE_CHAIN (parmtypes)) == void_type_node
	|| TREE_PURPOSE (TREE_CHAIN (parmtypes)))
      TREE_GETS_INIT_REF (current_class_type) = 1;
    else
      TREE_GETS_INIT_AGGR (current_class_type) = 1;
  else if (TYPE_MAIN_VARIANT (parmtype) == current_class_type)
    {
      TREE_GETS_INIT_AGGR (current_class_type) = 1;
      if (TREE_CHAIN (parmtypes) != NULL_TREE
	  && TREE_VALUE (TREE_CHAIN (parmtypes)) == void_type_node)
	error ("invalid constructor; you probably meant `%s (%s&)'",
	       TYPE_NAME_STRING (current_class_type),
	       TYPE_NAME_STRING (current_class_type));
    }
}

static void
grok_op_properties (decl)
     tree decl;
{
  if (! strncmp (IDENTIFIER_POINTER (DECL_NAME (decl)), "op$modify_expr", 11))
    {
      tree parmtypes = TREE_CHAIN (TYPE_ARG_TYPES (TREE_TYPE (decl)));
      tree parmtype = parmtypes ? TREE_VALUE (parmtypes) : void_type_node;

      if (TREE_CODE (parmtype) == REFERENCE_TYPE
	  && TREE_TYPE (parmtype) == current_class_type)
	{
	  TREE_HAS_ASSIGN_REF (current_class_type) = 1;
	  TREE_GETS_ASSIGN_REF (current_class_type) = 1;
	}
    }
#if 0
  else if (! strncmp (IDENTIFIER_POINTER (DECL_NAME (decl)), "op$call_expr", 8))
    TREE_HAS_CALL_OVERLOADED (current_class_type) = 1;
  else if (! strncmp (IDENTIFIER_POINTER (DECL_NAME (decl)), "op$array_ref", 9))
    TREE_HAS_ARRAY_REF_OVERLOADED (current_class_type) = 1;
#endif
}

/* Process the specs, declarator (NULL if omitted) and width (NULL if omitted)
   of a structure component, returning a FIELD_DECL node.
   WIDTH is non-NULL for bit fields only, and is an INTEGER_CST node.

   This is done during the parsing of the struct declaration.
   The FIELD_DECL nodes are chained together and the lot of them
   are ultimately passed to `build_struct' to make the RECORD_TYPE node.

   C++: It is possible that the value returned will be a VOID_TYPE_NODE.
   This can happen, for example, when a friend declaration creates multiple
   friends.

   DO NOT MAKE ANY CHANGES TO THIS CODE WITHOUT MAKING CORRESPONDING
   CHANGES TO CODE IN `start_method'.  */

tree
grokfield (declarator, declspecs, width, init, asmspec)
     tree declarator, declspecs, width, init, asmspec;
{
  register tree value;

  value = grokdeclarator (declarator, declspecs, FIELD, 0);
  if (! value) return NULL_TREE; /* friends went bad.  */

  if (value == void_type_node || TREE_CODE (value) == FRIEND_DECL)
    {
      /* Friends.  */
      if (TREE_CODE (value) == FRIEND_DECL && OPERATOR_NAME_P (DECL_NAME (value)))
	{
	  TREE_OPERATOR (value) = 1;
	  grok_op_properties (TREE_TYPE (value));
	}
      return void_type_node;
    }
  DECL_ASM_NAME (value) = NULL_TREE;
  finish_decl (value, init, asmspec);
  if (TREE_CODE (TREE_TYPE (value)) != FUNCTION_DECL)
    {
      if (TREE_CODE (value) == TYPE_DECL)
	/* Look like a friend.  It does not matter that we are not.  */
	return void_type_node;

      if (TREE_STATIC (value))
	{
	  if (width)
	    {
	      error ("static field `%s' cannot be a bitfield",
		     IDENTIFIER_POINTER (DECL_NAME (value)));
	      return NULL_TREE;
	    }
	}
      /* detect invalid field size.  */
      else if (width)
	{
	  if (TREE_CODE (width) != INTEGER_CST)
	    {
	      error_with_decl (value, "structure field `%s' width not an integer constant");
	      DECL_INITIAL (value) = NULL;
	    }
	  else
	    {
	      DECL_INITIAL (value) = width;
	      TREE_PACKED (value) = 1;
	    }
	}
      else
	{
	  DECL_INITIAL (value) = init;
	}
    }
  else
    {
      if (OPERATOR_NAME_P (DECL_NAME (value)))
	{
	  TREE_OPERATOR (value) = 1;
	  grok_op_properties (TREE_TYPE (value));
	}
      else
	{
	  tree type = TREE_TYPE (DECL_NAME (value));
	  if (type && TREE_CODE (type) == TYPE_DECL
	      && TREE_TYPE (type) == current_class_type)
	    grok_ctor_properties (TREE_TYPE (value));
	}
    }
  return value;
}

/* Like GROKFIELD, except that the declarator has been
   buried in DECLSPECS.  Find the declarator, and
   return something that looks like it came from
   GROKFIELD.  */
tree
groktypefield (declspecs, parmlist)
     tree declspecs;
     tree parmlist;
{
  tree spec = declspecs;
  tree prev = NULL_TREE;

  tree type_id = NULL_TREE;
  tree quals = NULL_TREE;
  tree lengths = NULL_TREE;
  tree decl = NULL_TREE;

  while (spec)
    {
      register int i;
      register tree id = TREE_VALUE (spec);

      if (TREE_CODE (spec) != TREE_LIST)
	{
	  /* Certain parse errors slip through.  For example,
	     `int class ();' is not caught by the parser. Try
	     weakly to recover here.  */
	  return NULL_TREE;
	}

      if (TREE_CODE (id) == TYPE_DECL
	  || (TREE_CODE (id) == IDENTIFIER_NODE
	      && TREE_TYPE (id)))
	{
	  /* We have a constructor/destructor or
	     conversion operator.  Use it.  */
	  if (prev)
	    TREE_CHAIN (prev) = TREE_CHAIN (spec);
	  else
	    {
	      declspecs = TREE_CHAIN (spec);
	    }
	  type_id = id;
	  goto found;
	}
      prev = spec;
      spec = TREE_CHAIN (spec);
    }

  /* Nope, we have a conversion operator to a scalar type.  */
  spec = declspecs;
  while (spec)
    {
      tree id = TREE_VALUE (spec);

      if (TREE_CODE (id) == IDENTIFIER_NODE)
	{
	  if (id == ridpointers[(int)RID_INT]
	      || id == ridpointers[(int)RID_DOUBLE]
	      || id == ridpointers[(int)RID_FLOAT])
	    {
	      if (type_id)
		error ("extra `%s' ignored",
		       IDENTIFIER_POINTER (id));
	      else
		type_id = id;
	    }
	  else if (id == ridpointers[(int)RID_LONG]
		   || id == ridpointers[(int)RID_SHORT]
		   || id == ridpointers[(int)RID_CHAR])
	    {
	      lengths = tree_cons (NULL_TREE, id, lengths);
	    }
	  else if (id == ridpointers[(int)RID_VOID])
	    {
	      if (type_id)
		error ("spurious `void' type ignored");
	      else
		error ("conversion to `void' type illegal");
	    }
	  else if (id == ridpointers[(int)RID_AUTO]
		   || id == ridpointers[(int)RID_REGISTER]
		   || id == ridpointers[(int)RID_TYPEDEF]
		   || id == ridpointers[(int)RID_CONST]
		   || id == ridpointers[(int)RID_VOLATILE])
	    {
	      error ("type specifier `%s' used invalidly",
		     IDENTIFIER_POINTER (id));
	    }
	  else if (id == ridpointers[(int)RID_FRIEND]
		   || id == ridpointers[(int)RID_VIRTUAL]
		   || id == ridpointers[(int)RID_INLINE]
		   || id == ridpointers[(int)RID_UNSIGNED]
		   || id == ridpointers[(int)RID_SIGNED]
		   || id == ridpointers[(int)RID_STATIC]
		   || id == ridpointers[(int)RID_EXTERN])
	    {
	      quals = tree_cons (NULL_TREE, id, quals);
	    }
	  else
	    abort ();
	}
      else
	abort ();
      spec = TREE_CHAIN (spec);
    }

  if (type_id)
    {
      declspecs = chainon (lengths, quals);
    }
  else if (lengths)
    {
      if (TREE_CHAIN (lengths))
	error ("multiple length specifiers");
      type_id = ridpointers[(int)RID_INT];
      declspecs = chainon (lengths, quals);
    }
  else if (quals)
    {
      error ("no type given, defaulting to `operator int ...'");
      type_id = ridpointers[(int)RID_INT];
      declspecs = quals;
    }
 found:
  decl = grokdeclarator (build_nt (CALL_EXPR, type_id, parmlist, NULL_TREE),
			 declspecs, FIELD, 0);
  if (decl == NULL_TREE)
    return NULL_TREE;
  if (decl == void_type_node || TREE_CODE (decl) == FRIEND_DECL)
    {
      /* bunch of friends.  */
      return decl;
    }
  DECL_ASM_NAME (decl) = NULL_TREE;
  finish_decl (decl, NULL_TREE, NULL_TREE);

  /* We now get constructors coming through here.  They do
     not get their TREE_OPERATOR bit set.  */
  if (TREE_TYPE (type_id) != 0
      && TREE_CODE (TREE_TYPE (type_id)) == TYPE_DECL
      && IS_AGGR_TYPE (TREE_TYPE (TREE_TYPE (type_id))))
    {
      grok_ctor_properties (TREE_TYPE (decl));
    }
  else
    {
      TREE_OPERATOR (decl) = 1;
      grok_op_properties (TREE_TYPE (decl));
    }

  return decl;
}

/* Get the struct, enum or union (CODE says which) with tag NAME.
   Define the tag as a forward-reference if it is not defined.

   C++: If a class derivation is given, process it here, and report
   an error if multiple derivation declarations are not identical.  */
tree
xref_tag (code_type_node, name, basetype_info)
     tree code_type_node;
     tree name, basetype_info;
{
  enum tree_code code = TREE_CODE (code_type_node);
  enum tree_code actual_code = code == CLASS_TYPE ? RECORD_TYPE : code;

  /* If a cross reference is requested, look up the type
     already defined for this tag and return it.  */
  register tree ref = lookup_tag (actual_code, name, current_binding_level, 0);

  if (! ref)
    {
      /* If no such tag is yet defined, create a forward-reference node
	 and record it as the "definition".
	 When a real declaration of this type is found,
	 the forward-reference will be altered into a real type.  */

      ref = make_node (actual_code);
      pushtag (name, ref);

      /* Lay this out now for ease later.  */
      if (IS_AGGR_TYPE_CODE (actual_code))
	build_pointer_type (ref);
    }

  if (basetype_info)
    {
      /* The base of a derived struct is public.  */
      int via_public = (code != CLASS_TYPE
			|| TREE_PURPOSE (basetype_info) == (tree)visibility_public);
      tree basetype = TREE_TYPE (TREE_VALUE (basetype_info));
      if (basetype && TREE_CODE (basetype) == TYPE_DECL)
	{
	  basetype = TREE_TYPE (basetype);
	}
      if (!basetype || !(IS_AGGR_TYPE (basetype)))
	{
	  error ("base type `%s' fails to be a type",
		 IDENTIFIER_POINTER (basetype_info));
	}
      else if (TYPE_SIZE (basetype) == 0)
	{
	  error ("base class `%s' has incomplete type",
		 TYPE_NAME_STRING (basetype));
	}
      else
	{
	  TREE_VIA_PUBLIC (ref) = via_public;

	  if (TYPE_BASELINK (ref))
	    {
	      tree tt1 = basetype, tt2 = TYPE_BASELINK (ref);
	      while (tt1 || tt2)
		{
		  if (tt1 != tt2)
		    {
		      error ("redeclaration of derivation chain of type `%s'",
			     IDENTIFIER_POINTER (name));
		      break;
		    }
		  tt1 = TYPE_BASELINK (tt1);
		  tt2 = TYPE_BASELINK (tt2);
		}
	    }
	  TYPE_BASELINK (ref) = basetype;

	  TREE_GETS_ASSIGNMENT (ref) = TREE_GETS_ASSIGNMENT (basetype);
	  TREE_HAS_METHOD_CALL_OVERLOADED (ref) = TREE_HAS_METHOD_CALL_OVERLOADED (basetype);
	  TREE_GETS_NEW (ref) = TREE_GETS_NEW (basetype);
	  TREE_GETS_DELETE (ref) = TREE_GETS_DELETE (basetype);
	}
    }

  if (code == CLASS_TYPE && !TYPE_NEXT_VARIANT (ref))
    {
      tree cref = copy_node (ref);
      TYPE_MAIN_VARIANT (cref) = ref;
      TREE_TYPE (cref) = class_type_node;
      TYPE_NEXT_VARIANT (ref) = cref;
      return cref;
    }
  else if (code == RECORD_TYPE)
    TREE_TYPE (ref) = record_type_node;
  else if (code == UNION_TYPE)
    TREE_TYPE (ref) = union_type_node;

  return ref;
}

/* Finish off the processing of a RECORD_TYPE structure.
   Only the static members need to be written out here.
   Anonymous structures do not exist.  */
void
finish_record (elems)
     tree elems;
{
  tree decl, init, asmspec;

  while (elems)
    {
      decl = TREE_VALUE (elems);
      asmspec = TREE_PURPOSE (elems);
      finish_decl (decl, NULL_TREE, asmspec);
      elems = TREE_CHAIN (elems);
    }
}

/* Finish off the processing of a UNION_TYPE structure.
   If there are static members, then all members are
   static, and must be laid out together.  If the
   union is an anonymous union, we arrage for that
   as well.  PUBLICP is nonzero if this union is
   not declared static.  */
void
finish_anon_union (anon_union_decl)
     tree anon_union_decl;
{
  tree type = TREE_TYPE (anon_union_decl);
  tree field, decl;
  tree elems;
  int public_p = TREE_PUBLIC (anon_union_decl);
  int static_p = TREE_STATIC (anon_union_decl);
  int external_p = TREE_EXTERNAL (anon_union_decl);

  if ((field = TYPE_FIELDS (type)) == NULL_TREE)
    return;

  decl = build_decl (VAR_DECL, DECL_NAME (field), TREE_TYPE (field));

  /* tell `pushdecl' that this is not tentative.  */
  DECL_INITIAL (decl) = error_mark_node;
  TREE_PUBLIC (decl) = public_p;
  TREE_STATIC (decl) = static_p;
  TREE_EXTERNAL (decl) = external_p;
  decl = pushdecl (decl);
  DECL_INITIAL (decl) = NULL_TREE;
  elems = build_tree_list (DECL_ASM_NAME (field), decl);
  TREE_TYPE (elems) = type;

  while (field = TREE_CHAIN (field))
    {
      decl = build_decl (VAR_DECL, DECL_NAME (field), TREE_TYPE (field));
      /* tell `pushdecl' that this is not tentative.  */
      DECL_INITIAL (decl) = error_mark_node;
      TREE_PUBLIC (decl) = public_p;
      TREE_STATIC (decl) = static_p;
      TREE_EXTERNAL (decl) = external_p;
      decl = pushdecl (decl);
      DECL_INITIAL (decl) = NULL_TREE;
      if (static_p)
	elems = tree_cons (DECL_ASM_NAME (field), decl, elems);
      TREE_TYPE (elems) = type;
    }
  if (static_p)
    rest_of_anon_union_compilation (anon_union_decl, elems, current_binding_level == global_binding_level, 1);
  else
    {
      /* lay it out on the stack by hand.  */
      struct rtx_def *anon_rtl = DECL_RTL (get_temp_name (type, NULL_TREE, NULL_TREE));
      for (field = TYPE_FIELDS (type); field; field = TREE_CHAIN (field))
	{
	  DECL_RTL (IDENTIFIER_LOCAL_VALUE (DECL_NAME (field))) = anon_rtl;
	}
    }
}

/* Begin compiling the definition of an enumeration type.
   NAME is its name (or null if anonymous).
   Returns the type object, as yet incomplete.
   Also records info about it so that build_enumerator
   may be used to declare the individual values as they are read.  */

tree
start_enum (name)
     tree name;
{
  register tree enumtype = 0;

  /* If this is the real definition for a previous forward reference,
     fill in the contents in the same object that used to be the
     forward reference.  */

  if (name != 0)
    enumtype = lookup_tag (ENUMERAL_TYPE, name, current_binding_level, 1);

  if (enumtype == 0 || TREE_CODE (enumtype) != ENUMERAL_TYPE)
    {
      enumtype = make_node (ENUMERAL_TYPE);
      pushtag (name, enumtype);
    }

  if (TYPE_VALUES (enumtype) != 0)
    {
      /* This enum is a named one that has been declared already.  */
      error ("redeclaration of `enum %s'", IDENTIFIER_POINTER (name));

      /* Completely replace its old definition.
	 The old enumerators remain defined, however.  */
      TYPE_VALUES (enumtype) = 0;
    }

  /* Initially, set up this enum as like `int'
     so that we can create the enumerators' declarations and values.
     Later on, the precision of the type may be changed and
     it may be layed out again.  */

  TYPE_PRECISION (enumtype) = TYPE_PRECISION (integer_type_node);
  TYPE_SIZE (enumtype) = 0;
  fixup_unsigned_type (enumtype);

  enum_next_value = integer_zero_node;

  return enumtype;
}

/* After processing and defining all the values of an enumeration type,
   install their decls in the enumeration type and finish it off.
   ENUMTYPE is the type object and VALUES a list of name-value pairs.
   Returns ENUMTYPE.  */

tree
finish_enum (enumtype, values)
     register tree enumtype, values;
{
  register tree pair = values;
  register long maxvalue = 0;
  register int i;

  TYPE_VALUES (enumtype) = values;

  /* Calculate the maximum value of any enumerator in this type.  */

  for (pair = values; pair; pair = TREE_CHAIN (pair))
    {
      int value = TREE_INT_CST_LOW (TREE_VALUE (pair));
      if (value > maxvalue)
	maxvalue = value;
    }

#if 0
  /* Determine the precision this type needs, lay it out, and define it.  */

  for (i = maxvalue; i; i >>= 1)
    TYPE_PRECISION (enumtype)++;

  if (!TYPE_PRECISION (enumtype))
    TYPE_PRECISION (enumtype) = 1;

  /* Cancel the laying out previously done for the enum type,
     so that fixup_unsigned_type will do it over.  */
  TYPE_SIZE (enumtype) = 0;

  fixup_unsigned_type (enumtype);
#endif

  TREE_INT_CST_LOW (TYPE_MAX_VALUE (enumtype)) =  maxvalue;

  return enumtype;
}

/* Build and install a CONST_DECL for one value of the
   current enumeration type (one that was begun with start_enum).
   Return a tree-list containing the name and its value.
   Assignment of sequential values by default is handled here.  */

tree
build_enumerator (name, value)
     tree name, value;
{
  register tree decl;

  /* Validate and default VALUE.  */

  if (value != 0 && TREE_CODE (value) != INTEGER_CST)
    {
      error ("enumerator value for `%s' not integer constant",
	     IDENTIFIER_POINTER (name));
      value = 0;
    }

  /* Default based on previous value.  */
  if (value == 0)
    value = enum_next_value;

  /* Set basis for default for next value.  */
  enum_next_value = build_binary_op_nodefault (PLUS_EXPR, value,
					       integer_one_node);

  /* Now create a declaration for the enum value name.  */

  decl = build_decl (CONST_DECL, name, integer_type_node);
  DECL_INITIAL (decl) = value;
  TREE_TYPE (value) = integer_type_node;
  pushdecl (decl);

  return build_tree_list (name, value);
}

/* Create the FUNCTION_DECL for a function definition.
   LINE1 is the line number that the definition absolutely begins on.
   LINE2 is the line number that the name of the function appears on.
   DECLSPECS and DECLARATOR are the parts of the declaration;
   they describe the function's name and the type it returns,
   but twisted together in a fashion that parallels the syntax of C.

   This function creates a binding context for the function body
   as well as setting up the FUNCTION_DECL in current_function_decl.

   Returns 1 on success.  If the DECLARATOR is not suitable for a function
   (it defines a datum instead), we return 0, which tells
   yyparse to report a parse error.

   For C++, we must first check whether that datum makes any sense.
   For example, "class A local_a(1,2);" means that variable local
   a is an aggregate of type A, which should have a constructor
   applied to it with the argument list [1, 2].

   @@ There is currently no way to retrieve the storage
   @@ allocated to FUNCTION (or all of its parms) if we return
   @@ something we had previously.  */

int
start_function (declspecs, declarator, pre_parsed_p)
     tree declarator, declspecs;
     int pre_parsed_p;
{
  tree decl1, old_decl;
  tree ctype = NULL_TREE;

  current_function_returns_value = 0;  /* Assume, until we see it does. */
  current_function_returns_null = 0;
  warn_about_return_type = 0;
  current_function_assigns_this = 0;
  current_function_just_assigned_this = 0;
  temp_name_counter = 0;

  if (pre_parsed_p)
    {
      /* We must waste one node's worth of space to satisfy `pushdecl'.  */
      decl1 = copy_node (declarator);
      last_function_parms = DECL_ARGUMENTS (decl1);
      last_function_parm_tags = 0;
      if (TREE_METHOD (decl1))
	{
	  tree t = TREE_VALUE (TYPE_ARG_TYPES (TREE_TYPE (decl1)));
	  if (TREE_CODE (t) != POINTER_TYPE || ! IS_AGGR_TYPE (TREE_TYPE (t)))
	    abort ();
	  ctype = TREE_TYPE (t);
	}
    }
  else
    {
      decl1 = grokdeclarator (declarator, declspecs, FUNCDEF, 1);

      /* If the declarator is not suitable for a function definition,
	 cause a syntax error.  */
      if (decl1 == 0) return 0;

      if (TREE_CODE (decl1) == SCOPE_REF)
	{
	  tree t;

	  if (TREE_CODE (TREE_OPERAND (decl1, 1)) != FUNCTION_DECL)
	    return 0;

	  t = TREE_TYPE (TREE_TYPE (TREE_OPERAND (decl1, 0)));
	  ctype = t;
	  decl1 = TREE_OPERAND (decl1, 1);
	}
    }

  current_function_decl = decl1;
  current_function_name = DECL_ORIGINAL_NAME (decl1);

  announce_function (ctype ? DECL_NAME (TYPE_NAME (ctype)) : 0,
		     current_function_decl, last_function_parms);

  if (TYPE_SIZE (TREE_TYPE (decl1)) == 0)
    {
      error ("return-type is an incomplete type");
      /* Make it return void instead.  */
      TREE_TYPE (decl1)
	= build_function_type (void_type_node,
			       TYPE_ARG_TYPES (TREE_TYPE (decl1)));
    }

  if (warn_about_return_type)
    warning ("return-type defaults to `int'");

  /* Save the parm names or decls from this function's declarator
     where store_parm_decls will find them.  */
  current_function_parms = last_function_parms;
  current_function_parm_tags = last_function_parm_tags;

  /* Make the init_value nonzero so pushdecl knows this is not tentative.
     error_mark_node is replaced below (in poplevel) with the LET_STMT.  */
  DECL_INITIAL (current_function_decl) = error_mark_node;

  /* If this definition isn't a prototype and we had a prototype declaration
     before, copy the arg type info from that prototype.  */
  old_decl = lookup_name_current_level (DECL_NAME (current_function_decl));
  if (old_decl != 0
      && TREE_TYPE (TREE_TYPE (current_function_decl)) == TREE_TYPE (TREE_TYPE (old_decl))
      && TYPE_ARG_TYPES (TREE_TYPE (current_function_decl)) == 0)
    TREE_TYPE (current_function_decl) = TREE_TYPE (old_decl);

  /* This is a definition, not a reference.  */
  TREE_EXTERNAL (current_function_decl) = 0;
  /* This function exists in static storage.
     (This does not mean `static' in the C sense!)  */
  TREE_STATIC (current_function_decl) = 1;

  /* Record the decl so that the function name is defined.
     If we already have a decl for this name, and it is a FUNCTION_DECL,
     use the old decl.  */

  current_function_decl = pushdecl (current_function_decl);

  if (TREE_CODE (current_function_decl) == FUNCTION_DECL)
    {
      if (TREE_OVERLOADED (current_function_decl))
	{
	  /* @@ Also done in start_decl.  */
	  tree glob = IDENTIFIER_GLOBAL_VALUE (current_function_name);

	  if (glob && TREE_CODE (glob) == FUNCTION_DECL)
	    glob = tree_cons (NULL_TREE, glob);

	  if (glob && TREE_VALUE (glob) == NULL_TREE)
	    TREE_VALUE (glob) = current_function_decl;
	  else if (value_member (current_function_decl, glob) == 0)
	    {
	      IDENTIFIER_GLOBAL_VALUE (current_function_name)
		= tree_cons (current_function_name, current_function_decl, glob);
	      TREE_TYPE (IDENTIFIER_GLOBAL_VALUE (current_function_name))
		= unknown_type_node;
	    }
	}
    }

  pushlevel (0);
  current_binding_level->parm_flag = 1;

  make_function_rtl (current_function_decl);

  /* Allocate further tree nodes temporarily during compilation
     of this function only.  */
  temporary_allocation ();

  if (ctype)
    pushclass (ctype, 1);

  DECL_RESULT (current_function_decl)
    = build_decl (RESULT_DECL, value_identifier,
		  TREE_TYPE (TREE_TYPE (current_function_decl)));

  /* Make the FUNCTION_DECL's contents appear in a local tree dump
     and make the FUNCTION_DECL itself not appear in the permanent dump.  */

  TREE_PERMANENT (current_function_decl) = 0;

  return 1;
}

/* Store the parameter declarations into the current function declaration.
   This is called after parsing the parameter declarations, before
   digesting the body of the function.  */

void
store_parm_decls ()
{
  register tree fndecl = current_function_decl;
  register tree parm;

  /* This is either a chain of PARM_DECLs (if a prototype was used)
     or a list of IDENTIFIER_NODEs (for an old-fashioned C definition).  */
  tree specparms = current_function_parms;

  /* This is a list of types declared among parms in a prototype.  */
  tree parmtags = current_function_parm_tags;

  /* This is a chain of PARM_DECLs from old-style parm declarations.  */
  register tree parmdecls = getdecls ();

  /* This is a chain of any other decls that came in among the parm
     declarations.  If a parm is declared with  enum {foo, bar} x;
     then CONST_DECLs for foo and bar are put here.  */
  tree nonparms = 0;

  if (current_binding_level == global_binding_level)
    {
      fatal ("parse errors have confused me too much");
    }

  if (specparms != 0 && TREE_CODE (specparms) == PARM_DECL)
    {
      /* This case is when the function was defined with an ANSI prototype.
	 The parms already have decls, so we need not do anything here
	 except record them as in effect
	 and complain if any redundant old-style parm decls were written.  */

      register tree next;

      if (parmdecls != 0)
	error_with_decl (fndecl,
			 "parm types given both in parmlist and separately");

      for (parm = nreverse (specparms); parm; parm = next)
	{
	  next = TREE_CHAIN (parm);
	  if (DECL_NAME (parm) == 0)
	    {
#if 0
	      error_with_decl (parm, "parameter name omitted");
#else
	      /* for C++, this is not an error.  */
	      DECL_NAME (parm) = make_anon_parm_name ();
	      pushdecl (parm);
#endif
	    }
	  else if (TREE_TYPE (parm) == void_type_node)
	    error_with_decl (parm, "parameter `%s' declared void");
	  else
	    pushdecl (parm);
	}

      /* Get the decls in their original chain order
	 and record in the function.  */
      DECL_ARGUMENTS (fndecl) = getdecls ();

      storetags (chainon (parmtags, gettags ()));
    }
  else
    {
      /* SPECPARMS is an identifier list--a chain of TREE_LIST nodes
	 each with a parm name as the TREE_VALUE.

	 PARMDECLS is a list of declarations for parameters.
	 Warning! It can also contain CONST_DECLs which are not parameters
	 but are names of enumerators of any enum types
	 declared among the parameters.

	 First match each formal parameter name with its declaration.
	 Associate decls with the names and store the decls
	 into the TREE_PURPOSE slots.  */

      for (parm = specparms; parm; parm = TREE_CHAIN (parm))
	{
	  register tree tail, found = NULL;

	  if (TREE_VALUE (parm) == 0)
	    {
	      error_with_decl (fndecl, "parameter name missing from parameter list");
	      TREE_PURPOSE (parm) = 0;
	      continue;
	    }

	  /* See if any of the parmdecls specifies this parm by name.
	     Ignore any enumerator decls.  */
	  for (tail = parmdecls; tail; tail = TREE_CHAIN (tail))
	    if (DECL_NAME (tail) == TREE_VALUE (parm)
		&& TREE_CODE (tail) == PARM_DECL)
	      {
		found = tail;
		break;
	      }

	  /* If declaration already marked, we have a duplicate name.
	     Complain, and don't use this decl twice.   */
	  if (found && DECL_CONTEXT (found) != 0)
	    {
	      error_with_decl (found, "multiple parameters named `%s'");
	      found = 0;
	    }

	  /* If the declaration says "void", complain and ignore it.  */
	  if (found && TREE_TYPE (found) == void_type_node)
	    {
	      error_with_decl (found, "parameter `%s' declared void");
	      TREE_TYPE (found) = integer_type_node;
	      DECL_ARG_TYPE (found) = integer_type_node;
	      layout_decl (found, 0);
	    }

	  /* If no declaration found, default to int.  */
	  if (!found)
	    {
	      found = build_decl (PARM_DECL, TREE_VALUE (parm),
				  integer_type_node);
	      DECL_ARG_TYPE (found) = TREE_TYPE (found);
	      DECL_SOURCE_LINE (found) = DECL_SOURCE_LINE (fndecl);
	      DECL_SOURCE_FILE (found) = DECL_SOURCE_FILE (fndecl);
	      pushdecl (found);
	    }

	  TREE_PURPOSE (parm) = found;

	  /* Mark this decl as "already found" -- see test, above.
	     It is safe to clobber DECL_CONTEXT temporarily
	     because the final values are not stored until
	     the `poplevel' in `finish_function'.  */
	  DECL_CONTEXT (found) = error_mark_node;
	}

      /* Complain about declarations not matched with any names.
	 Put any enumerator constants onto the list NONPARMS.  */

      nonparms = 0;
      for (parm = parmdecls; parm; )
	{
	  tree next = TREE_CHAIN (parm);
	  TREE_CHAIN (parm) = 0;

	  if (TREE_CODE (parm) != PARM_DECL)
	    nonparms = chainon (nonparms, parm);

	  else if (DECL_CONTEXT (parm) == 0)
	    {
	      error_with_decl (parm,
			       "declaration for parameter `%s' but no such parameter");
	      /* Pretend the parameter was not missing.
		 This gets us to a standard state and minimizes
		 further error messages.  */
	      specparms
		= chainon (specparms,
			   tree_cons (parm, NULL_TREE, NULL_TREE));
	    }

	  parm = next;
	}

      /* Chain the declarations together in the order of the list of names.  */
      /* Store that chain in the function decl, replacing the list of names.  */
      parm = specparms;
      DECL_ARGUMENTS (fndecl) = 0;
      {
	register tree last;
	for (last = 0; parm; parm = TREE_CHAIN (parm))
	  if (TREE_PURPOSE (parm))
	    {
	      if (last == 0)
		DECL_ARGUMENTS (fndecl) = TREE_PURPOSE (parm);
	      else
		TREE_CHAIN (last) = TREE_PURPOSE (parm);
	      last = TREE_PURPOSE (parm);
	      TREE_CHAIN (last) = 0;
	      DECL_CONTEXT (last) = 0;
	    }
      }

      /* If there was a previous prototype,
	 set the DECL_ARG_TYPE of each argument according to
	 the type previously specified, and report any mismatches.  */

      if (TYPE_ARG_TYPES (TREE_TYPE (fndecl)))
	{
	  register tree type;
	  for (parm = DECL_ARGUMENTS (fndecl),
	       type = TYPE_ARG_TYPES (TREE_TYPE (fndecl));
	       parm || (type && TREE_VALUE (type) != void_type_node);
	       parm = TREE_CHAIN (parm), type = TREE_CHAIN (type))
	    {
	      if (parm == 0 || type == 0
		  || TREE_VALUE (type) == void_type_node)
		{
		  error_with_decl (fndecl, "argument decls of `%s' don't match prototype");
		  break;
		}
	      /* Type for passing arg must be consistent
		 with that declared for the arg.  */
	      if (! comptypes (DECL_ARG_TYPE (parm), TREE_VALUE (type), 1))
		error_with_decl (fndecl, "argument `%s' doesn't match function prototype");
	    }
	}
    }

  /* Now store the final chain of decls for the arguments
     as the decl-chain of the current lexical scope.
     Put the enumerators in as well, at the front so that
     DECL_ARGUMENTS is not modified.  */

  storedecls (chainon (nonparms, DECL_ARGUMENTS (fndecl)));

  if (current_class_type)
    {
      current_class_decl = DECL_ARGUMENTS (fndecl);
      if (TREE_CODE (TREE_TYPE (current_class_decl)) == POINTER_TYPE)
	C_C_D = build (INDIRECT_REF, current_class_type, current_class_decl);
      else C_C_D = current_class_decl;
    }

  /* Initialize the RTL code for the function.  */

  expand_function_start (fndecl);
}

void
setup_vtbl_ptr ()
{
  if (! flag_this_is_variable
      && current_class_type
      && TREE_VIRTUAL (current_class_type))
    {
      tree vfield = build_component_ref (C_C_D,
					 get_vfield_name (current_class_type), 0);
      current_vtable_decl = DECL_VTBL_PTR (TYPE_NAME (current_class_type));
      DECL_RTL (current_vtable_decl) = 0;
      DECL_INITIAL (current_vtable_decl) = error_mark_node;
      finish_decl (current_vtable_decl, vfield, 0);
      current_vtable_decl = build_indirect_ref (current_vtable_decl, "vtbl decl (compiler error)");
    }
  else
    current_vtable_decl = NULL_TREE;
}

/* Finish up a function declaration and compile that function
   all the way to assembler language output.  The free the storage
   for the function definition.

   This is called after parsing the body of the function definition.
   STMTS is the chain of statements that makes up the function body.  */

void
finish_function (filename, line)
     char *filename;
     int line;
{
  register tree fndecl = current_function_decl;
  tree rval;
  struct rtx_def *head, *last_parm_insn, *mark;

  register tree link;

/*  TREE_READONLY (fndecl) = 1;
    This caused &foo to be of type ptr-to-const-function
    which then got a warning when stored in a ptr-to-function variable.  */

  /* Clean house because we will need to reorder insns here.  */
  do_pending_stack_adjust ();

  if (DESTRUCTOR_NAME_P (DECL_NAME (fndecl)))
    {
      tree cond;
      tree thenclause;
      tree member, expr, exprstmt = NULL_TREE;
      tree basetype = TYPE_BASELINK (current_class_type);
      tree auto_delete_node = lookup_name (get_identifier (AUTO_DELETE_NAME));

      if (basetype && TREE_NEEDS_DESTRUCTOR (basetype))
	{
	   cond = build_binary_op (NE_EXPR, current_class_decl, integer_zero_node);
	   thenclause = build_delete (basetype, C_C_D, auto_delete_node);
        }
      else
	{
	   cond = build_binary_op (NE_EXPR, auto_delete_node, integer_zero_node);
	   thenclause = build_x_delete ();
        }

      for (member = TYPE_FIELDS (current_class_type); member; member = TREE_CHAIN (member))
	{
	  if (TREE_STATIC (member))
	    continue;
	  if (TREE_NEEDS_DESTRUCTOR (TREE_TYPE (member)))
	    {
	      expr = build_delete (TREE_TYPE (member), build_component_ref (C_C_D, DECL_NAME (member), 0), integer_zero_node);
	      if (expr != error_mark_node)
		exprstmt = tree_cons (NULL_TREE, expr, exprstmt);
	    }
	}

      /* These initializations might go inline.  Protect
	 the binding level of the parms.  */
      pushlevel (0);

      if (exprstmt)
	expand_expr_stmt (build_compound_expr (exprstmt));

      expand_start_cond (cond, 0);
      expand_expr_stmt (thenclause);
      expand_end_cond ();

      poplevel (0, 0, 1);

      /* Dont execute destructor code if `this' is NULL.  */
      mark = (struct rtx_def *)get_last_insn ();
      last_parm_insn = (struct rtx_def *)get_first_nonparm_insn ();
      if (last_parm_insn == 0) last_parm_insn = mark;
      else last_parm_insn = (struct rtx_def *)prev_insn (last_parm_insn);

      cond = build_binary_op (NE_EXPR, current_class_decl, integer_zero_node);
      expand_start_cond (cond, 0);
      reorder_insns (next_insn (mark), get_last_insn (), last_parm_insn);
      expand_end_cond ();
    }
  else if (current_function_assigns_this)
    {
      TREE_ASSIGNS_THIS (fndecl) = 1;
      c_expand_return (current_class_decl);
      current_function_assigns_this = 0;
      current_function_just_assigned_this = 0;
    }
  else if (current_class_name == current_function_name)
    {
      tree cond, thenclause;

      TREE_RETURNS_FIRST_ARG (fndecl) = 1;

      cond = build_binary_op (EQ_EXPR, current_class_decl, integer_zero_node);
      thenclause = build_modify_expr (current_class_decl, NOP_EXPR,
				      build_x_new (current_class_name, void_type_node));
      /* must keep the first insn safe.  */
      head = (struct rtx_def *)get_insns ();

      /* this note will come up to the top with us.  */
      mark = (struct rtx_def *)get_last_insn ();

      /* This is where the real function begins.  If there were no
	 insns in this function body, then the last_parm_insn
	 is also the last insn.  */
      last_parm_insn = (struct rtx_def *)get_first_nonparm_insn ();
      if (last_parm_insn == 0) last_parm_insn = mark;
      else last_parm_insn = (struct rtx_def *)prev_insn (last_parm_insn);

      /* These initializations might go inline.  Protect
	 the binding level of the parms.  */
      pushlevel (0);

      expand_start_cond (cond, 0);
      expand_expr_stmt (thenclause);
      expand_end_cond ();

      /* Make `finish_base_init' set up virtual function table pointer.  */
      finish_base_init ();

      if (head == last_parm_insn)
	reorder_insns (next_insn (mark), get_last_insn (), head);
      else
	reorder_insns (next_insn (mark), get_last_insn (), last_parm_insn);

      c_expand_return (current_class_decl);

      poplevel (0, 0, 1);

      current_function_assigns_this = 0;
      current_function_just_assigned_this = 0;
    }

  poplevel (1, 0, 1);

  /* reset scope for C++: if we were in the scope of a class,
     then when we finish this function, we are not longer so. */
  if (current_class_name)
    {
      popclass (1);
    }

  /* Must mark the RESULT_DECL as being in this function.  */

  DECL_CONTEXT (DECL_RESULT (fndecl)) = DECL_INITIAL (fndecl);

  /* Generate rtl for function exit.  */
  expand_function_end ();

  if (warn_return_type)
    /* So we can tell if jump_optimize sets it to 1.  */
    current_function_returns_null = 0;

  /* Run the optimizers and output the assembler code for this function.  */
  rest_of_compilation (fndecl);

  if (warn_return_type && current_function_returns_null
      && TREE_TYPE (TREE_TYPE (fndecl)) != void_type_node)
    /* If this function returns non-void and control can drop through,
       complain.  */
    warning ("control reaches end of non-void function");
  /* With just -W, complain only if function returns both with
     and without a value.  */
  else if (extra_warnings
      && current_function_returns_value && current_function_returns_null)
    warning ("this function may return with or without a value");

  /* Free all the tree nodes making up this function.  */
  /* Switch back to allocating nodes permanently
     until we start another function.  */
  permanent_allocation ();

  if (DECL_SAVED_INSNS (fndecl) == 0)
    {
      /* Stop pointing to the local nodes about to be freed.  */
      /* But DECL_INITIAL must remain nonzero so we know this
	 was an actual function definition.  */
      DECL_INITIAL (fndecl) = error_mark_node;
      DECL_ARGUMENTS (fndecl) = 0;
      DECL_RESULT (fndecl) = 0;
    }

  /* Let the error reporting routines know that we're outside a function.  */
  current_function_decl = NULL;
  current_function_name = NULL;
  clear_anon_parm_name ();
}

/* Create the FUNCTION_DECL for a function definition.
   LINE1 is the line number that the definition absolutely begins on.
   LINE2 is the line number that the name of the function appears on.
   DECLSPECS and DECLARATOR are the parts of the declaration;
   they describe the function's name and the type it returns,
   but twisted together in a fashion that parallels the syntax of C.

   This function creates a binding context for the function body
   as well as setting up the FUNCTION_DECL in current_function_decl.

   Returns a FIELD_DECL on success.  If the DECLARATOR is not suitable
   for a function (it defines a datum instead), we return 0, which tells
   yyparse to report a parse error.

   Came here with a `.pushlevel' .

   DO NOT MAKE ANY CHANGES TO THIS CODE WITHOUT MAKING CORRESPONDING
   CHANGES TO CODE IN `grokfield'.  */
tree
start_method (declspecs, declarator)
     tree declarator, declspecs;
{
  tree type, decl1;
  decl1 = grokdeclarator (declarator, declspecs, FIELD, 0);

  /* Something too ugly to handle.  */
  if (decl1 == 0)
    return 0;

  /* Pass friends back.  */
  if (decl1 == void_type_node)
    return void_type_node;

  if (TREE_CODE (TREE_TYPE (decl1)) == FUNCTION_DECL)
    {
      current_function_decl = TREE_TYPE (decl1);
      current_function_name = DECL_ORIGINAL_NAME (decl1);
    }
  else
    {
      /* Not a function, tell parser to report parse error.  */
      return 0;
    }

  finish_decl (decl1, NULL, NULL);

  {
    tree type = TREE_TYPE (current_function_name);

    if (type && TREE_CODE (type) == TYPE_DECL
	&& TREE_TYPE (type) == current_class_type)
      grok_ctor_properties (current_function_decl);
    else if (OPERATOR_NAME_P (current_function_name))
      {
	TREE_OPERATOR (decl1) = 1;
	grok_op_properties (current_function_decl);
      }
  }

  /* Make a place for the parms */
  pushlevel (0);
  current_binding_level->parm_flag = 1;
  
  return decl1;
}

/* Finish up a function declaration and compile that function
   all the way to assembler language output.  The free the storage
   for the function definition.

   FINISH_METHOD must return something that looks as though it
   came from GROKFIELD (since we are defining a method, after all).

   This is called after parsing the body of the function definition.
   STMTS is the chain of statements that makes up the function body.

   DECL is the FIELD_DECL that `start_method' provided.  */

void
finish_method (decl)
     tree decl;
{
  register tree fndecl = current_function_decl;
  int old_uid;
  tree old_initial;

  register tree link;

  old_initial = DECL_INITIAL (fndecl);

  /* Undo the level for the parms (from start_method) */
  poplevel (0, 0, 0);

  DECL_INITIAL (fndecl) = old_initial;
  /* Let the error reporting routines know that we're outside a function.  */
  current_function_decl = NULL;
  current_function_name = NULL;
}

#if 0
static tree init_temp_name;

/* @@ Need to rethink this.  */
int
set_temp_name (decl)
     tree decl;
{
  if (decl)
    {
      tree type = TREE_TYPE (decl);
      if (TREE_NEEDS_CONSTRUCTOR (type))
	{
	  init_temp_name = decl;
	}
      else
	init_temp_name = NULL_TREE;
    }
  else
    init_temp_name = NULL_TREE;
}
#endif

/* Hand off a unique name which can be used for variable we don't really
   want to know about anyway, for example, the anonymous variables which
   are needed to make references work. Declare this thing so we can use it.
   The variable created will be of type TYPE.

   If REFERENCE is `error_mark_node', then INIT should be directly in
   DECL.  This currently happens only when we are creating aggregates
   whose constructor has already been figured out.  */

tree
get_temp_name (type, reference, init)
     tree type, reference, init;
{
  static char buf[OVERLOAD_MAX_LEN];
  tree decl;

#if 0
  if (init_temp_name)
    {
      DECL_INITIAL (init_temp_name) = init;
      return init_temp_name;
    }
#endif

  sprintf (buf, AUTO_TEMP_FORMAT, temp_name_counter++);
  decl = pushdecl (build_decl (VAR_DECL, get_identifier (buf), type));

  if (current_binding_level == global_binding_level)
    {
      TREE_STATIC (decl) = 1;
      /* lay this variable out now.  Otherwise `output_addressed_constants'
	 gets confused by its initializer.  */
      rest_of_decl_compilation (decl, 0, 1, 1);
    }
  else
    {
      mark_addressable (decl);
    }

  TREE_EVERUSED (decl) = 1;

  if (reference != error_mark_node)
    {
      if (init)
	store_init_value (decl, init);

#if 0
      expand_decl (decl, 1);
#endif
      TREE_HAS_CONSTRUCTOR (decl) = 0;

      if (reference)
	{
	  tree addr = build (ADDR_EXPR, build_reference_type (TREE_TYPE (decl)), decl);
	  DECL_INITIAL (reference) = addr;
	}
    }
  else
    {
      DECL_INITIAL (decl) = init;
      TREE_HAS_CONSTRUCTOR (decl) = 1;
#if 0
      expand_decl (decl, 0);
#endif
    }

  return decl;
}

tree
get_temp_regvar (type, init)
     tree type, init;
{
  static char buf[OVERLOAD_MAX_LEN] = { '_' };
  tree decl;

  sprintf (buf+1, AUTO_TEMP_FORMAT, temp_name_counter++);
  decl = pushdecl (build_decl (VAR_DECL, get_identifier (buf), type));
  TREE_EVERUSED (decl) = 1;
  TREE_REGDECL (decl) = 1;

  if (init)
    store_init_value (decl, init);

  /* We can expand these without fear, since they cannot need
     constructors or destructors.  */
  expand_decl (decl, 0);
  return decl;
}

/* Make the macro TEMP_NAME_P available to units which do not
   include c-tree.h.  */
int
temp_name_p (decl)
     tree decl;
{
  return TEMP_NAME_P (decl);
}

/* Called when a new struct TYPE is defined.
   If this structure or union completes the type of any previous
   variable declaration, lay it out and output its rtl.  */

void
hack_incomplete_structures (type)
     tree type;
{
  if (current_binding_level->n_incomplete != 0)
    {
      tree decl;
      for (decl = current_binding_level->names; decl; decl = TREE_CHAIN (decl))
	if (TREE_TYPE (decl) == type
	    && TREE_CODE (decl) != TYPE_DECL)
	  {
	    int toplevel = global_binding_level == current_binding_level;
	    layout_decl (decl);
	    rest_of_decl_compilation (decl, 0, toplevel, 0);
	    if (! toplevel)
	      expand_decl (decl, 1);
	    --current_binding_level->n_incomplete;
	  }
    }
  if (current_binding_level->n_incomplete)
    error ("%d incomplete type declarations missed on this binding level (compiler error)",
	   current_binding_level->n_incomplete);
}
