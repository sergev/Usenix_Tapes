/* Extra definitions that belong to GNU C++.
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


/* this is how large a name can grow to */
#define OVERLOAD_MAX_LEN 1024

enum overload_flags { NO_SPECIAL = 0, DESTRUCTOR, OPERATOR, OP_TYPENAME, WRAPPER, ANTI_WRAPPER };

extern void instantiate_type ();
extern tree default_conversion (), pushdecl (), pushdecl_top_level ();
extern tree make_instance_name (), do_decl_overload ();
extern tree build_instantiated_decl ();
extern tree build_vtbl_ref ();
extern tree make_anon_parm_name ();
extern int resolves_to_fixed_type_p ();

extern tree do_friend ();
extern void grokclassfn ();

extern tree current_class_decl, C_C_D;	/* PARM_DECL: the class instance variable */

/* The following two can be derived from the previous one */
extern tree current_class_name;	/* IDENTIFIER_NODE: name of current class */
extern tree current_class_type;	/* _TYPE: the type of the current class */

/* The following structure is used when comparing various alternatives
   for overloading.  The unsigned quantity `strikes.i' is used
   for fast comparison of two possibilities.  This number is an
   aggregate of four constituents:

     EVIL: if this is non-zero, then the candidate should not be considered
     USER: if this is non-zero, then a user-defined type conversion is needed
     B_OR_D: if this is non-zero, then use a base pointer instead of the
             type of the pointer we started with.
     EASY: if this is non-zero, then we have a builtin conversion
           (such as int to long, int to float, etc) to do.

   If two candidates require user-defined type conversions, and the
   type conversions are not identical, then an ambiguity error
   is reported.

   If two candidates agree on user-defined type conversions,
   and one uses pointers of strictly higher type (derived where
   another uses base), then that alternative is silently chosen.

   If two candidates have a non-monotonic derived/base pointer
   relationship, and/or a non-monotonic easy conversion relationship,
   then a warning is emitted to show which paths are possible, and
   which one is being chosen.

   For example:

   int i;
   double x;

   overload f;
   int f (int, int);
   double f (double, double);

   f (i, x);	// draws a warning

   struct B
   {
     f (int);
   } *bb;
   struct D : B
   {
     f (double);
   } *dd;

   dd->f (x);	// exact match
   dd->f (i);	// draws warning

   Note that this technique really only works for 255 arguments.  Perhaps
   this is not enough.  */

struct candidate
{
  tree function;		/* A FUNCTION_DECL */

  unsigned char evil;		/* ~0 if this will never convert.  */
  unsigned char user;		/* ~0 if at least one user-defined type conv.  */
  unsigned short b_or_d;	/* count number of derived->base conv.  */
  unsigned short easy;		/* count number of builtin type conv.  */

  tree arg;			/* an _EXPR node that is first parm to function */
  union
    {
      tree field;		/* If no evil strikes, the FIELD_DECL of
				   the function (if a member function).  */
      int bad_arg;		/* the index of the first bad argument:
				   0 if no bad arguements
				   > 0 is first bad argument
				   -1 if extra actual arguments
				   -2 if too few actual arguments.  */
    } u;
};
int rank_for_overload ();

/* Anatomy of a DECL_FRIENDLIST (which is a TREE_LIST):
   purpose = friend name (IDENTIFIER_NODE);
   value = TREE_LIST of FUNCTION_DECLS;
   chain, type = EMPTY;  */
#define FRIEND_NAME(LIST) (TREE_PURPOSE (LIST))
#define FRIEND_DECLS(LIST) (TREE_VALUE (LIST))
