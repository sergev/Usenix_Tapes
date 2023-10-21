/* Process declarations and variables for C compiler.
   Copyright (C) 1988 Free Software Foundation, Inc.

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

static tree lookup_tag ();
static tree lookup_tag_reverse ();
static tree lookup_name_current_level ();
static char *redeclaration_error_message ();

/* a node which has tree code ERROR_MARK, and whose type is itself.
   All erroneous expressions are replaced with this node.  All functions
   that accept nodes as arguments should avoid generating error messages
   if this node is one of the arguments, since it is undesirable to get
   multiple error messages from one error in the input.  */

tree error_mark_node;

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

/* Function type `void (void *, void *, int)' and similar ones */

tree void_ftype_ptr_ptr_int, int_ftype_ptr_ptr_int, void_ftype_ptr_int_int;

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
tree current_function_decl;

/* Set to 0 at beginning of a function definition, set to 1 if
   a return statement that specifies a return value is seen.  */

int current_function_returns_value;

/* Set to 0 at beginning of a function definition, set to 1 if
   a return statement with no argument is seen.  */

int current_function_returns_null;

/* Set to nonzero by `grokdeclarator' for a function
   whose return type is defaulted, if warnings for this are desired.  */

static int warn_about_return_type;

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

  /* This warning is turned off because it causes warnings for
     declarations like `extern struct foo *x'.  */
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
#endif /* 0 */

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
	IDENTIFIER_LOCAL_VALUE (DECL_NAME (link)) = 0;
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
   In that case, the TYPE_SIZE will be zero.  */

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
    }
}

/* Handle when a new declaration NEW has the same name as an old one OLD
   in the same binding contour.  Prints an error message if appropriate.

   If safely possible, alter OLD to look like NEW, and return 1.
   Otherwise, return 0.  */

static int
duplicate_decls (new, old)
     register tree new, old;
{
  int types_match = comptypes (TREE_TYPE (new), TREE_TYPE (old));

  /* If this decl has linkage, and the old one does too, maybe no error.  */
  if (TREE_CODE (old) != TREE_CODE (new))
    error_with_decl (new, "`%s' redeclared as different kind of symbol");
  else
    {
      if (!types_match)
	error_with_decl (new, "conflicting types for `%s'");
      else
	{
	  char *errmsg = redeclaration_error_message (new, old);
	  if (errmsg)
	    error_with_decl (new, errmsg);
	}
    }

  if (TREE_CODE (old) == TREE_CODE (new))
    {
      /* Copy all the DECL_... slots specified in the new decl
	 except for any that we copy here from the old type.  */

      if (types_match)
	{
	  tree oldtype = TREE_TYPE (old);
	  /* Merge the data types specified in the two decls.  */
	  TREE_TYPE (new)
	    = TREE_TYPE (old) = commontype (TREE_TYPE (new), TREE_TYPE (old));

	  /* Lay the type out, unless already done.  */
	  if (oldtype != TREE_TYPE (new))
	    {
	      if (TREE_TYPE (new) != error_mark_node)
		layout_type (TREE_TYPE (new));
	      if (TREE_CODE (new) != FUNCTION_DECL
		  && TREE_CODE (new) != TYPE_DECL
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
	  TREE_PUBLIC (old) = TREE_PUBLIC (new);
	}
      /* If either decl says `inline', this fn is inline,
	 unless its definition was passed already.  */
      if (TREE_INLINE (new) && DECL_INITIAL (old) == 0)
	TREE_INLINE (old) = 1;

      bcopy ((char *) new + sizeof (struct tree_common),
	     (char *) old + sizeof (struct tree_common),
	     sizeof (struct tree_decl) - sizeof (struct tree_common));
      return 1;
    }

  /* New decl is completely inconsistent with the old one =>
     tell caller to replace the old one.  */
  return 0;
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
      if (t && duplicate_decls (x, t))
	return t;

      /* If declaring a type as a typedef, and the type has no known
	 typedef name, install this TYPE_DECL as its typedef name.  */
      if (TREE_CODE (x) == TYPE_DECL)
	if (TYPE_NAME (TREE_TYPE (x)) == 0
	    || TREE_CODE (TYPE_NAME (TREE_TYPE (x))) != TYPE_DECL)
	  TYPE_NAME (TREE_TYPE (x)) = x;

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
	}

      /* Keep count of variables in this level with incomplete type.  */
      if (TYPE_SIZE (TREE_TYPE (x)) == 0)
	++current_binding_level->n_incomplete;
    }

  /* Put decls on list in reverse order.
     We will reverse them later if necessary.  */
  TREE_CHAIN (x) = current_binding_level->names;
  current_binding_level->names = x;

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
    return "redefinition of `%s'";
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
   in the cases in which this is used.  */

static tree
lookup_tag_reverse (type)
     tree type;
{
  register struct binding_level *level;

  for (level = current_binding_level; level; level = level->level_chain)
    {
      register tree tail;
      for (tail = level->tags; tail; tail = TREE_CHAIN (tail))
	{
	  if (TREE_VALUE (tail) == type)
	    return TREE_PURPOSE (tail);
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
  pushlevel (0);	/* make the binding_level structure for global names */
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
  pushdecl (build_decl (TYPE_DECL, get_identifier ("unsigned int"),
			unsigned_type_node));

  /* `unsigned int' is the type for sizeof.  */
  sizetype = unsigned_type_node;
  TREE_TYPE (TYPE_SIZE (integer_type_node)) = sizetype;
  TREE_TYPE (TYPE_SIZE (char_type_node)) = sizetype;
  TREE_TYPE (TYPE_SIZE (unsigned_type_node)) = sizetype;

  error_mark_node = make_node (ERROR_MARK);
  TREE_TYPE (error_mark_node) = error_mark_node;

  short_integer_type_node = make_signed_type (BITS_PER_UNIT * min (UNITS_PER_WORD / 2, 2));
  pushdecl (build_decl (TYPE_DECL, get_identifier ("short int"),
			short_integer_type_node));

  long_integer_type_node = make_signed_type (BITS_PER_WORD);
  pushdecl (build_decl (TYPE_DECL, get_identifier ("long int"),
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
  layout_type (void_type_node);	/* Uses integer_zero_node */

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
   Otherwise, it is an error.  */

void
shadow_tag (declspecs)
     tree declspecs;
{
  register tree link;

  for (link = declspecs; link; link = TREE_CHAIN (link))
    {
      register tree value = TREE_VALUE (link);
      register enum tree_code code = TREE_CODE (value);
      if ((code == RECORD_TYPE || code == UNION_TYPE || code == ENUMERAL_TYPE)
	  && TYPE_SIZE (value) != 0)
	{
	  register tree name = lookup_tag_reverse (value);
	  register tree t = lookup_tag (code, name, current_binding_level, 1);
	  if (t == 0)
	    {
	      t = make_node (code);
	      pushtag (name, t);
	      return;
	    }
	  if (name != 0 || code == ENUMERAL_TYPE)
	    return;
	}
    }
  warning ("empty declaration");
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
  register tree decl = grokdeclarator (declarator, declspecs,
				       NORMAL, initialized);
  register tree tem;
  int init_written = initialized;

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
	if (TYPE_SIZE (TREE_TYPE (decl)) != 0)
	  ;			/* A complete type is ok.  */
	else if (TREE_CODE (TREE_TYPE (decl)) != ARRAY_TYPE)
	  {
	    error ("variable `%s' has initializer but incomplete type",
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	    initialized = 0;
	  }
	else if (TYPE_SIZE (TREE_TYPE (TREE_TYPE (decl))) == 0)
	  {
	    error ("elements of array `%s' have incomplete type",
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	    initialized = 0;
	  }
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
    }

  /* Add this decl to the current binding level.  */
  tem = pushdecl (decl);

  if (init_written)
    {
      /* When parsing and digesting the initializer,
	 use temporary storage.  Do this even if we will ignore the value.  */
      if (current_binding_level == global_binding_level
	  && debug_no_temp_inits)
	temporary_allocation ();
    }

  return tem;
}

/* Finish processing of a declaration;
   install its line number and initial value.
   If the length of an array type is not known before,
   it must be determined now, from the initial value, or it is an error.  */

void
finish_decl (decl, init, asmspec)
     tree decl, init;
     tree asmspec;
{
  register tree type = TREE_TYPE (decl);
  int init_written = init != 0;

  /* If `start_decl' didn't like having an initialization, ignore it now.  */

  if (init != 0 && DECL_INITIAL (decl) == 0)
    init = 0;

  if (init)
    {
      if (TREE_CODE (decl) != TYPE_DECL)
	store_init_value (decl, init);
      else
	{
	  /* typedef foo = bar; store the type of bar as the type of foo.  */
	  TREE_TYPE (decl) = TREE_TYPE (init);
	  DECL_INITIAL (decl) = init = 0;
	}
    }

  /* For top-level declaration, the initial value was read in
     the temporary obstack.  MAXINDEX, rtl, etc. to be made below
     must go in the permanent obstack; but don't discard the
     temporary data yet.  */

  if (debug_no_temp_inits && init_written
      && current_binding_level == global_binding_level)
    end_temporary_allocation ();

  /* Deduce size of array from initialization, if not already known */

  if (TREE_CODE (type) == ARRAY_TYPE
      && TYPE_DOMAIN (type) == 0
      && TREE_CODE (decl) != TYPE_DECL)
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
	}
      else if (!TREE_EXTERNAL (decl) && DECL_SIZE (decl) == 0)
	{
	  /* An automatic variable with an incomplete type:
	     that is an error.  */
	  error_with_decl (decl, "storage size of `%s' isn't known");
	  TREE_TYPE (decl) = error_mark_node;
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
      if (TYPE_SIZE (TREE_TYPE (decl)) != 0
	  || TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE)
	if (current_binding_level != global_binding_level)
	  expand_decl (decl);
    }

  /* Resume permanent allocation, if not within a function.  */
  if (debug_no_temp_inits && init_written
      && current_binding_level == global_binding_level)
    permanent_allocation ();
}

/* Given a parsed parameter declaration,
   decode it into a PARM_DECL and push that on the current binding level.  */

void
push_parm_decl (parm)
     tree parm;
{
  register tree decl = grokdeclarator (TREE_VALUE (parm), TREE_PURPOSE (parm),
				       PARM, 0);

  /* Add this decl to the current binding level.  */
  finish_decl (pushdecl (decl), NULL_TREE, NULL_TREE);
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
   and `extern' are intrepreted.  */

static tree
grokdeclarator (declarator, declspecs, decl_context, initialized)
     tree declspecs;
     tree declarator;
     enum decl_context decl_context;
     int initialized;
{
  int specbits = 0;
  tree spec;
  tree type = NULL_TREE;
  int longlong = 0;
  int constp;
  int volatilep;
  int inlinep;
  int explicit_int = 0;
  int explicit_char = 0;
  char *name;
  tree typedef_type = 0;
  int funcdef_flag = 0;
  enum tree_code innermost_code = ERROR_MARK;

  if (decl_context == FUNCDEF)
    funcdef_flag = 1, decl_context = NORMAL;

  /* Look inside a declarator for the name being declared
     and get it as a string, for an error message.  */
  {
    register tree decl = declarator;
    name = 0;

    while (decl)
      switch (TREE_CODE (decl))
	{
	case ARRAY_REF:
	case INDIRECT_REF:
	case CALL_EXPR:
	  innermost_code = TREE_CODE (decl);
	  decl = TREE_OPERAND (decl, 0);
	  break;

	case IDENTIFIER_NODE:
	  name = IDENTIFIER_POINTER (decl);
	  decl = 0;
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

  if (decl_context == NORMAL
      && current_binding_level->level_chain == global_binding_level)
    decl_context = PARM;

  /* Look through the decl specs and record which ones appear.
     Some typespecs are defined as built-in typenames.
     Others, the ones that are modifiers of other types,
     are represented by bits in SPECBITS: set the bits for
     the modifiers that appear.  Storage class keywords are also in SPECBITS.

     If there is a typedef name or a type, store the type in TYPE.
     This includes builtin typedefs such as `int'.

     Set EXPLICIT_INT if the type is `int' or `char' and did not
     come from a user typedef.

     Set LONGLONG if `long' is mentioned twice.  */

  for (spec = declspecs; spec; spec = TREE_CHAIN (spec))
    {
      register int i;
      register tree id = TREE_VALUE (spec);

      if (id == ridpointers[(int) RID_INT])
	explicit_int = 1;
      if (id == ridpointers[(int) RID_CHAR])
	explicit_char = 1;

      if (TREE_CODE (id) == IDENTIFIER_NODE)
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
      if (funcdef_flag && warn_return_type
	  && ! (specbits & ((1 << (int) RID_LONG) | (1 << (int) RID_SHORT)
			    | (1 << (int) RID_SIGNED) | (1 << (int) RID_UNSIGNED))))
	warn_about_return_type = 1;
      explicit_int = 1;
      type = integer_type_node;
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
  type = TYPE_MAIN_VARIANT (type);

  /* Warn if two storage classes are given. Default to `auto'.  */

  {
    int nclasses = 0;

    if (specbits & 1 << (int) RID_AUTO) nclasses++;
    if (specbits & 1 << (int) RID_STATIC) nclasses++;
    if (specbits & 1 << (int) RID_EXTERN) nclasses++;
    if (specbits & 1 << (int) RID_REGISTER) nclasses++;
    if (specbits & 1 << (int) RID_TYPEDEF) nclasses++;

    /* Warn about storage classes that are invalid for certain
       kinds of declarations (parameters, typenames, etc.).  */

    if (nclasses > 1)
      error ("multiple storage classes in declaration of `%s'", name);
    else if (decl_context != NORMAL && nclasses > 0)
      {
	if (decl_context == PARM && specbits & 1 << (int) RID_REGISTER)
	  ;
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
			   | (1 << (int) RID_EXTERN));
	  }
      }
    else if (current_binding_level == global_binding_level)
      {
	if (specbits & (1 << (int) RID_AUTO))
	  error ("top-level declaration of `%s' specifies `auto'", name);
	if (specbits & (1 << (int) RID_REGISTER))
	  error ("top-level declaration of `%s' specifies `register'", name);
      }
  }

  /* Now figure out the structure of the declarator proper.
     Descend through it, creating more complex types, until we reach
     the declared identifier (or NULL_TREE, in an absolute declarator).  */

  while (declarator && TREE_CODE (declarator) != IDENTIFIER_NODE)
    {
      if (type == error_mark_node)
	{
	  declarator = TREE_OPERAND (declarator, 0);
	  continue;
	}

      /* Each level of DECLARATOR is either an ARRAY_REF (for ...[..]),
	 an INDIRECT_REF (for *...),
	 a CALL_EXPR (for ...(...)),
	 an identifier (for the name being declared)
	 or a null pointer (for the place in an absolute declarator
	 where the name was omitted).
	 For the last two cases, we have just exited the loop.

	 At this point, TYPE is the type of elements of an array,
	 or for a function to return, or for a pointer to point to.
	 After this sequence of ifs, TYPE is the type of the
	 array or function or pointer, and DECLARATOR has had its
	 outermost layer removed.  */

      if (TREE_CODE (declarator) == ARRAY_REF)
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
      else if (TREE_CODE (declarator) == CALL_EXPR)
	{
	  /* Declaring a function type.
	     Make sure we have a valid type for the function to return.  */
	  if (type == error_mark_node)
	    continue;

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

	  /* Construct the function type and go to the next
	     inner layer of declarator.  */

	  type =
	    build_function_type (type,
				 grokparms (TREE_OPERAND (declarator, 1),
					    funcdef_flag));
	  declarator = TREE_OPERAND (declarator, 0);
	}
      else if (TREE_CODE (declarator) == INDIRECT_REF)
	{
	  /* Merge any constancy or volatility into the target type
	     for the pointer.  */

	  if (constp || volatilep)
	    type = build_type_variant (type, constp, volatilep);
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
	}
      else
	abort ();

/*      layout_type (type);  */

      /* @@ Should perhaps replace the following code by changes in
       * @@ stor_layout.c. */
      if (TREE_CODE (type) == FUNCTION_DECL)
	{
	  /* A function variable in C should be Pmode rather than EPmode
	     because it has just the address of a function, no static chain.*/
	  TYPE_MODE (type) = Pmode;
	}
    }

  /* Now TYPE has the actual type.  */

  /* If this is declaring a typedef name, return a TYPE_DECL.  */

  if (specbits & (1 << (int) RID_TYPEDEF))
    {
      /* Note that the grammar rejects storage classes
	 in typenames, fields or parameters */
      if (constp || volatilep)
	type = build_type_variant (type, constp, volatilep);
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
	 in typenames, fields or parameters */
      if (constp || volatilep)
	type = build_type_variant (type, constp, volatilep);
      return type;
    }

  /* `void' at top level (not within pointer)
     is allowed only in typedefs or type names.
     We don't complain about parms either, but that is because
     a better error message can be made later.  */

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
	/* A parameter declared as an array of T is really a pointer to T.
	   One declared as a function is really a pointer to a function.  */

	if (TREE_CODE (type) == ARRAY_TYPE)
	  {
	    /* Transfer const-ness of array into that of type pointed to.  */
	    type = build_pointer_type
		    (build_type_variant (TREE_TYPE (type), constp, volatilep));
	    volatilep = constp = 0;
	  }
	else if (TREE_CODE (type) == FUNCTION_TYPE)
	  type = build_pointer_type (type);
	else if (TYPE_SIZE (type) == 0)
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
      }
    else if (decl_context == FIELD)
      {
	/* Structure field.  It may not be a function.  */

	if (TREE_CODE (type) == FUNCTION_TYPE)
	  {
	    error ("field `%s' declared as a function",
		   IDENTIFIER_POINTER (declarator));
	    type = build_pointer_type (type);
	  }
	else if (TYPE_SIZE (type) == 0)
	  {
	    error ("field `%s' has incomplete type",
		   IDENTIFIER_POINTER (declarator));
	    type = error_mark_node;
	  }
	decl = build_decl (FIELD_DECL, declarator, type);
      }
    else if (TREE_CODE (type) == FUNCTION_TYPE)
      {
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
	decl = build_decl (FUNCTION_DECL, declarator, type);
	TREE_EXTERNAL (decl) = 1;
	/* Record presence of `static'.  */
	TREE_PUBLIC (decl) = !(specbits & (1 << (int) RID_STATIC));
	/* Record presence of `inline', if it is reasonable.  */
	if (inlinep)
	  {
	    tree last = tree_last (TYPE_ARG_TYPES (type));

	    if (! strcmp (IDENTIFIER_POINTER (declarator), "main"))
	      warning ("cannot inline function `main'");
	    else if (last && TREE_VALUE (last) != void_type_node)
	      error ("inline declaration ignored for function with `...'");
	    else
	      /* Assume that otherwise the function can be inlined.  */
	      TREE_INLINE (decl) = 1;
	  }
      }
    else
      {
	/* It's a variable.  */

	decl = build_decl (VAR_DECL, declarator, type);

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

    return decl;
  }
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

/* Decode the list of parameter types for a function type.
   Given the list of things declared inside the parens,
   return a list of types.

   The arg is either a list of identifiers or a list of types.
   If the former, store it in `last_function_parms' and return 0.
   If the latter, return it; `last_function_parms' was already set
   (in `get_parm_types') so do not change it.

   FUNCDEF_FLAG is nonzero for a function definition, 0 for
   a mere declaration.  A nonempty identifier-list gets an error message
   when FUNCDEF_FLAG is zero.  */

static tree
grokparms (first_parm, funcdef_flag)
     tree first_parm;
     int funcdef_flag;
{
  if (first_parm != 0
      && TREE_CODE (TREE_VALUE (first_parm)) == IDENTIFIER_NODE)
    {
      if (! funcdef_flag)
	warning ("parameter names (without types) in function declaration");

      last_function_parms = first_parm;
      return 0;
    }
  else
    {
      return first_parm;
    }
}


/* Return a list of the types of the parameters just parsed.
   Also store in `last_function_parms' a chain of the decls of those parms.
   Both this chain and the return value must be in the proper order.
   Also store in `last_function_parm_tags' a chain of the struct and union
   tags declared among the parms.

   VOID_AT_END nonzero means append `void' to the end of the value-list.
   Zero means the parmlist ended with an ellipsis so don't append `void'.  */

tree
get_parm_types (void_at_end)
     int void_at_end;
{
  register tree decl;
  register tree types = 0;
  tree link;
  int erred = 0;

  last_function_parm_tags = gettags ();
  last_function_parms = nreverse (getdecls ());

  /* Just `void' (and no ellipsis) is special.  There are really no parms.  */
  if (void_at_end && last_function_parms != 0
      && TREE_CHAIN (last_function_parms) == 0
      && TREE_TYPE (last_function_parms) == void_type_node)
    {
      last_function_parms = NULL_TREE;
      storedecls (NULL_TREE);
      return tree_cons (NULL_TREE, void_type_node, NULL_TREE);
    }

  storedecls (last_function_parms);

  for (decl = last_function_parms; decl; decl = TREE_CHAIN (decl))
    {
      /* Since there is a prototype,
	 args are passed in their declared types.  */
      DECL_ARG_TYPE (decl) = TREE_TYPE (decl);

      types = tree_cons (NULL_TREE, TREE_TYPE (decl), types);
      if (TREE_VALUE (types) == void_type_node && ! erred)
	{
	  error ("`void' in parameter list must be the entire list");
	  erred = 1;
	}
    }

  if (void_at_end)
    return nreverse (tree_cons (NULL_TREE, void_type_node, types));

  return nreverse (types);
}

/* Get the struct, enum or union (CODE says which) with tag NAME.
   Define the tag as a forward-reference if it is not defined.  */

tree
xref_tag (code, name)
     tree name;
{
  /* If a cross reference is requested, look up the type
     already defined for this tag and return it.  */

  register tree ref = lookup_tag (code, name, current_binding_level, 0);
  if (ref) return ref;

  /* If no such tag is yet defined, create a forward-reference node
     and record it as the "definition".
     When a real declaration of this type is found,
     the forward-reference will be altered into a real type.  */

  ref = make_node (code);
  pushtag (name, ref);
  return ref;
}

/* Make sure that the tag NAME is defined *in the current binding level*
   at least as a forward reference.
   CODE says which kind of tag NAME ought to be.  */

tree
start_struct (code, name)
     enum tree_code code;
     tree name;
{
  /* If there is already a tag defined at this binding level
     (as a forward reference), just return it.  */

  register tree ref = 0;

  if (name != 0)
    ref = lookup_tag (code, name, current_binding_level, 1);
  if (ref && TREE_CODE (ref) == code)
    {
      if (TYPE_FIELDS (ref))
	error ((code == UNION_TYPE ? "redefinition of `union %s'"
		: "redefinition of `struct %s'"),
	       IDENTIFIER_POINTER (name));

      return ref;
    }

  /* Otherwise create a forward-reference just so the tag is in scope.  */

  ref = make_node (code);
  pushtag (name, ref);
  return ref;
}

/* Process the specs, declarator (NULL if omitted) and width (NULL if omitted)
   of a structure component, returning a FIELD_DECL node.
   WIDTH is non-NULL for bit fields only, and is an INTEGER_CST node.

   This is done during the parsing of the struct declaration.
   The FIELD_DECL nodes are chained together and the lot of them
   are ultimately passed to `build_struct' to make the RECORD_TYPE node.  */

tree
grokfield (filename, line, declarator, declspecs, width)
     char *filename;
     int line;
     tree declarator, declspecs, width;
{
  register tree value = grokdeclarator (declarator, declspecs, FIELD, 0);

  finish_decl (value, NULL, NULL);
  DECL_INITIAL (value) = width;

  return value;
}

/* Fill in the fields of a RECORD_TYPE or UNION_TYPE node, T.
   FIELDLIST is a chain of FIELD_DECL nodes for the fields.  */

tree
finish_struct (t, fieldlist)
     register tree t, fieldlist;
{
  register tree x;
  int old;
  int round_up_size = 1;

  /* If this type was previously laid out as a forward reference,
     make sure we lay it out again.  */

  TYPE_SIZE (t) = 0;

  old = suspend_momentary ();

  if (fieldlist == 0 && pedantic)
    warning ((TREE_CODE (t) == UNION_TYPE ? "union has no members"
	      : "structure has no members"));

  /* Install struct as DECL_CONTEXT of each field decl.
     Also process specified field sizes.
     Set DECL_SIZE_UNIT to the specified size, or 0 if none specified.
     The specified size is found in the DECL_INITIAL.
     Store 0 there, except for ": 0" fields (so we can find them
     and delete them, below).  */

  for (x = fieldlist; x; x = TREE_CHAIN (x))
    {
      DECL_CONTEXT (x) = t;
      DECL_SIZE_UNIT (x) = 0;

      /* If any field is const, the structure type is pseudo-const.  */
      if (TREE_READONLY (x))
	C_TYPE_FIELDS_READONLY (t) = 1;
      else
	{
	  /* A field that is pseudo-const makes the structure likewise.  */
	  tree t1 = TREE_TYPE (x);
	  while (TREE_CODE (t1) == ARRAY_TYPE)
	    t1 = TREE_TYPE (t1);
	  if ((TREE_CODE (t1) == RECORD_TYPE || TREE_CODE (t1) == UNION_TYPE)
	      && C_TYPE_FIELDS_READONLY (t1))
	    C_TYPE_FIELDS_READONLY (t) = 1;
	}

      /* Detect invalid bit-field size.  */
      if (DECL_INITIAL (x) && TREE_CODE (DECL_INITIAL (x)) != INTEGER_CST)
	{
	  error_with_decl (x, "bit-field `%s' width not an integer constant");
	  DECL_INITIAL (x) = NULL;
	}

      /* Detect invalid bit-field type.  */
      if (DECL_INITIAL (x)
	  && TREE_CODE (TREE_TYPE (x)) != INTEGER_TYPE
	  && TREE_CODE (TREE_TYPE (x)) != ENUMERAL_TYPE)
	{
	  error_with_decl (x, "bit-field `%s' has invalid type");
	  DECL_INITIAL (x) = NULL;
	}
      if (DECL_INITIAL (x) && pedantic
	  && TREE_TYPE (x) != integer_type_node
	  && TREE_TYPE (x) != unsigned_type_node)
	warning_with_decl (x, "bit-field `%s' type invalid in ANSI C");

      /* Detect and ignore out of range field width.  */
      if (DECL_INITIAL (x))
	{
	  register int width = TREE_INT_CST_LOW (DECL_INITIAL (x));

	  if (width < 0)
	    {
	      DECL_INITIAL (x) = NULL;
	      warning_with_decl (x, "negative width in bit-field `%s'");
	    }
	  else if (width > TYPE_PRECISION (TREE_TYPE (x)))
	    {
	      DECL_INITIAL (x) = NULL;
	      warning_with_decl (x, "width of `%s' exceeds its type");
	    }
	}

      /* Process valid field width.  */
      if (DECL_INITIAL (x))
	{
	  register int width = TREE_INT_CST_LOW (DECL_INITIAL (x));

	  if (width == 0)
	    {
	      /* field size 0 => mark following field as "aligned" */
	      if (TREE_CHAIN (x))
		DECL_ALIGN (TREE_CHAIN (x)) = EMPTY_FIELD_BOUNDARY;
	      /* field of size 0 at the end => round up the size.  */
	      else
		round_up_size = EMPTY_FIELD_BOUNDARY;
	    }
	  else
	    {
	      DECL_INITIAL (x) = NULL;
	      DECL_SIZE_UNIT (x) = width;
	      TREE_PACKED (x) = 1;
	      /* Traditionally a bit field is unsigned
		 even if declared signed.  */
	      if (flag_traditional
		  && TREE_CODE (TREE_TYPE (x)) == INTEGER_TYPE)
		TREE_TYPE (x) = unsigned_type_node;
	    }
	}
      else
	/* Non-bit-fields are aligned for their type.  */
	DECL_ALIGN (x) = TYPE_ALIGN (TREE_TYPE (x));
    }

  /* Now DECL_INITIAL is null on all members except for zero-width bit-fields.
     And they have already done their work.  */

  /* Delete all zero-width bit-fields from the front of the fieldlist */
  while (fieldlist
	 && DECL_INITIAL (fieldlist))
    fieldlist = TREE_CHAIN (fieldlist);
  /* Delete all such members from the rest of the fieldlist */
  for (x = fieldlist; x;)
    {
      if (TREE_CHAIN (x) && DECL_INITIAL (TREE_CHAIN (x)))
	TREE_CHAIN (x) = TREE_CHAIN (TREE_CHAIN (x));
      else x = TREE_CHAIN (x);
    }

  /* Delete all duplicate fields from the fieldlist */
  for (x = fieldlist; x && TREE_CHAIN (x);)
    /* Anonymous fields aren't duplicates.  */
    if (DECL_NAME (TREE_CHAIN (x)) == 0)
      x = TREE_CHAIN (x);
    else
      {
	register tree y = fieldlist;
	  
	while (1)
	  {
	    if (DECL_NAME (y) == DECL_NAME (TREE_CHAIN (x)))
	      break;
	    if (y == x)
	      break;
	    y = TREE_CHAIN (y);
	  }
	if (DECL_NAME (y) == DECL_NAME (TREE_CHAIN (x)))
	  {
	    error_with_decl (TREE_CHAIN (x), "duplicate member `%s'");
	    TREE_CHAIN (x) = TREE_CHAIN (TREE_CHAIN (x));
	  }
	else x = TREE_CHAIN (x);
      }

  /* Now we have the final fieldlist.  Record it,
     then lay out the structure or union (including the fields).  */

  TYPE_FIELDS (t) = fieldlist;

  /* If there's a :0 field at the end, round the size to the
     EMPTY_FIELD_BOUNDARY.  */
  TYPE_ALIGN (t) = round_up_size;
  
  layout_type (t);

  /* Promote each bit-field's type to int if it is narrower than that.  */
  for (x = fieldlist; x; x = TREE_CHAIN (x))
    if (TREE_PACKED (x)
	&& TREE_CODE (TREE_TYPE (x)) == INTEGER_TYPE
	&& (TREE_INT_CST_LOW (DECL_SIZE (x)) * DECL_SIZE_UNIT (x)
	    < TYPE_PRECISION (integer_type_node)))
      TREE_TYPE (x) = integer_type_node;

  /* If this structure or union completes the type of any previous
     variable declaration, lay it out and output its rtl.  */

  if (current_binding_level->n_incomplete != 0)
    {
      tree decl;
      for (decl = current_binding_level->names; decl; decl = TREE_CHAIN (decl))
	if (TREE_TYPE (decl) == t
	    && TREE_CODE (decl) != TYPE_DECL)
	  {
	    int toplevel = global_binding_level == current_binding_level;
	    layout_decl (decl);
	    rest_of_decl_compilation (decl, 0, toplevel, 0);
	    if (! toplevel)
	      expand_decl (decl);
	    --current_binding_level->n_incomplete;
	  }
    }

  resume_momentary (old);

  return t;
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
   yyparse to report a parse error.  */

int
start_function (declspecs, declarator)
     tree declarator, declspecs;
{
  tree decl1, old_decl;

  current_function_returns_value = 0;  /* Assume, until we see it does. */
  current_function_returns_null = 0;
  warn_about_return_type = 0;

  decl1 = grokdeclarator (declarator, declspecs, FUNCDEF, 1);

  /* If the declarator is not suitable for a function definition,
     cause a syntax error.  */
  if (decl1 == 0)
    return 0;

  current_function_decl = decl1;

  announce_function (current_function_decl);

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

  pushlevel (0);
  current_binding_level->parm_flag = 1;

  make_function_rtl (current_function_decl);

  /* Allocate further tree nodes temporarily during compilation
     of this function only.  */
  temporary_allocation ();

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
	    error_with_decl (parm, "parameter name omitted");
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
		  error ("argument decls of `%s' don't match prototype",
			 IDENTIFIER_POINTER (DECL_NAME (fndecl)));
		  break;
		}
	      /* Type for passing arg must be consistent
		 with that declared for the arg.  */
	      if (! comptypes (DECL_ARG_TYPE (parm), TREE_VALUE (type)))
		error ("argument `%s' doesn't match function prototype",
		       IDENTIFIER_POINTER (DECL_NAME (parm)));
	    }
	}
    }

  /* Now store the final chain of decls for the arguments
     as the decl-chain of the current lexical scope.
     Put the enumerators in as well, at the front so that
     DECL_ARGUMENTS is not modified.  */

  storedecls (chainon (nonparms, DECL_ARGUMENTS (fndecl)));

  /* Initialize the RTL code for the function.  */

  expand_function_start (fndecl);
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

/*  TREE_READONLY (fndecl) = 1;
    This caused &foo to be of type ptr-to-const-function
    which then got a warning when stored in a ptr-to-function variable.  */

  poplevel (1, 0, 1);

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
}
