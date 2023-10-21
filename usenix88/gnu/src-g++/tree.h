/* Front-end tree definitions for GNU compiler.
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


/* codes of tree nodes */

#define DEFTREECODE(SYM, STRING, TYPE, NARGS)   SYM,
#define DEFTREECODE_X(SYM, STRING, TYPE, NARGS, ERR_NAME) SYM,
#define DEFTREECODE_XY(SYM, STRING, TYPE, NARGS, NAME, ASG_NAME) SYM,

enum tree_code {
#include "tree.def"
};

#undef DEFTREECODE
#undef DEFTREECODE_X
#undef DEFTREECODE_XY

/* Indexed by enum tree_code, contains a character which is
   `e' for an expression, `r' for a reference, `c' for a constant,
   `d' for a decl, `t' for a type, `s' for a statement,
   and `x' for anything else (TREE_LIST, IDENTIFIER, etc).  */

extern char *tree_code_type[];

/* Number of argument-words in each kind of tree-node.  */

extern int tree_code_length[];

/* Get the definition of `enum machine_mode' */

#ifndef HAVE_MACHINE_MODES
#define DEF_MACHMODE(SYM, NAME, TYPE, SIZE, UNIT)  SYM,

enum machine_mode {
#include "machmode.def"
};

#undef DEF_MACHMODE

#define HAVE_MACHINE_MODES

#endif /* not HAVE_MACHINE_MODES */

#ifndef NUM_MACHINE_MODES
#define NUM_MACHINE_MODES (int) MAX_MACHINE_MODE
#endif

/* Codes that identify the various built in functions
   so that expand_call can identify them quickly.  */

enum built_in_function
{
  NOT_BUILT_IN,
  BUILT_IN_ALLOCA,
  BUILT_IN_ABS,
  BUILT_IN_FABS,
  BUILT_IN_LABS,
  BUILT_IN_FFS,
  BUILT_IN_DIV,
  BUILT_IN_LDIV,
  BUILT_IN_FFLOOR,
  BUILT_IN_FCEIL,
  BUILT_IN_FMOD,
  BUILT_IN_FREM,
  BUILT_IN_MEMCPY,
  BUILT_IN_MEMCMP,
  BUILT_IN_MEMSET,
  BUILT_IN_FSQRT,
  BUILT_IN_GETEXP,
  BUILT_IN_GETMAN,

  /* C++ extensions */
  BUILT_IN_NEW,
  BUILT_IN_VEC_NEW,
  BUILT_IN_DELETE,
  BUILT_IN_VEC_DELETE,
};

/* The definition of tree nodes fills the next several pages.  */

/* A tree node can represent a data type, a variable, an expression
   or a statement.  Each node has a TREE_CODE which says what kind of
   thing it represents.  Some common codes are:
   INTEGER_TYPE -- represents a type of integers.
   ARRAY_TYPE -- represents a type of pointer.
   VAR_DECL -- represents a declared variable.
   INTEGER_CST -- represents a constant integer value.
   PLUS_EXPR -- represents a sum (an expression).

   As for the contents of a tree node: there are some fields
   that all nodes share.  Each TREE_CODE has various special-purpose
   fields as well.  The fields of a node are never accessed directly,
   always through accessor macros.  */

/* This type is used everywhere to refer to a tree node.  */

typedef union tree_node *tree;

#define NULL_TREE (tree) NULL

/* Every kind of tree node starts with this structure,
   so all nodes have these fields.

   See the accessor macros, defined below, for documentation of the fields.  */

struct tree_common
{
  int uid;
  union tree_node *chain;
  union tree_node *type;
  enum tree_code code : 8;

  unsigned external_attr : 1;
  unsigned public_attr : 1;
  unsigned static_attr : 1;
  unsigned volatile_attr : 1;
  unsigned packed_attr : 1;
  unsigned readonly_attr : 1;
  unsigned literal_attr : 1;
  unsigned nonlocal_attr : 1;
  unsigned permanent_attr : 1;
  unsigned addressable_attr : 1;
  unsigned regdecl_attr : 1;
  unsigned this_vol_attr : 1;
  unsigned unsigned_attr : 1;
  unsigned asm_written_attr: 1;

  unsigned overloaded_attr : 1;
  unsigned private_attr : 1;
  unsigned protected_attr : 1;
  unsigned virtual_attr : 1;
  unsigned has_constructor_attr : 1;
  unsigned has_destructor_attr : 1;
  unsigned method_attr : 1;
  unsigned member_pointer_attr : 1;
  unsigned inline_attr : 1;
  unsigned everused_attr : 1;
};

/* Define accessors for the fields that all tree nodes have
   (though some fields are not used for all kinds of nodes).  */

/* The unique id of a tree node distinguishes it from all other nodes
   in the same compiler run.  */
#define TREE_UID(NODE) ((NODE)->common.uid)

/* The tree-code says what kind of node it is.
   Codes are defined in tree.def.  */
#define TREE_CODE(NODE) ((NODE)->common.code)
#define TREE_SET_CODE(NODE, VALUE) ((NODE)->common.code = (VALUE))

/* In all nodes that are expressions, this is the data type of the expression.
   In POINTER_TYPE nodes, this is the type that the pointer points to.
   In ARRAY_TYPE nodes, this is the type of the elements.

   In IDENTIFIER_NODE nodes, if the identifier names an aggregate typedef,
   this is the TYPE_DECL that it names.  */
#define TREE_TYPE(NODE) ((NODE)->common.type)

/* Nodes are chained together for many purposes.
   Types are chained together to record them for being output to the debugger
   (see the function `chain_type').
   Decls in the same scope are chained together to record the contents
   of the scope.
   Statement nodes for successive statements used to be chained together.
   Often lists of things are represented by TREE_LIST nodes thaht
   are chained together.  */

#define TREE_CHAIN(NODE) ((NODE)->common.chain)

/* Define many boolean fields that all tree nodes have.  */

/* In a VAR_DECL or FUNCTION_DECL,
   nonzero means external reference:
   do not allocate storage, and refer to a definition elsewhere.  */
#define TREE_EXTERNAL(NODE) ((NODE)->common.external_attr)

/* In a VAR_DECL, nonzero means allocate static storage.
   In a FUNCTION_DECL, currently nonzero if function has been defined.  */
#define TREE_STATIC(NODE) ((NODE)->common.static_attr)

/* In a VAR_DECL or FUNCTION_DECL,
   nonzero means name is to be accessible from outside this module.  */
#define TREE_PUBLIC(NODE) ((NODE)->common.public_attr)

/* In VAR_DECL nodes, nonzero means address of this is needed.
   So it cannot be in a register.
   In a FUNCTION_DECL, nonzero means its address is needed.
   So it must be compiled even if it is an inline function.
   In CONSTRUCTOR nodes, it means are all constants suitable
   for output as assembly-language initializers.
   In LABEL_DECL nodes, it means a goto for this label has been seen 
   from a place outside all binding contours that restore stack levels,
   or that an error message about jumping into such a binding contour
   has been printed for this label.  */
#define TREE_ADDRESSABLE(NODE) ((NODE)->common.addressable_attr)

/* In VAR_DECL nodes, nonzero means declared `register'.  */
#define TREE_REGDECL(NODE) ((NODE)->common.regdecl_attr)

/* In any expression, nonzero means it has side effects or reevaluation
   of the whole expression could produce a different value.
   This is set if any subexpression is a function call, a side effect
   or a reference to a volatile variable.
   In a ..._DECL, this is set only of the declaration said `volatile'.
   In a ..._TYPE, nonzero means the type is volatile-qualified.  */
#define TREE_VOLATILE(NODE) ((NODE)->common.volatile_attr)

/* Nonzero means this expression is volatile in the C sense:
   its address should be of type `volatile WHATEVER *'.
   If this bit is set, so is `volatile_attr'.  */
#define TREE_THIS_VOLATILE(NODE) ((NODE)->common.this_vol_attr)

/* In a VAR_DECL, PARM_DECL or FIELD_DECL, or any kind of ..._REF node,
   nonzero means it may not be the lhs of an assignment.
   In a ..._TYPE node, means this type is const-qualified.  */
#define TREE_READONLY(NODE) ((NODE)->common.readonly_attr)

/* Nonzero in a FIELD_DECL means it is a bit-field; it may occupy
   less than a storage unit, and its address may not be taken, etc.
   This controls layout of the containing record.
   In a LABEL_DECL, nonzero means label was defined inside a binding
   contour that restored a stack level and which is now exited.

   C++: Nonzero in a FUNCTION_DECL means that it was defined as an inline
   function before being used.  If nobody needs its address, it will
   not be written.  If somebody does need it, this bit must be reset.  */
#define TREE_PACKED(NODE) ((NODE)->common.packed_attr)

/* Value of expression is constant.
   Always appears in all ..._CST nodes.
   May also appear in an arithmetic expression, an ADDR_EXPR or a CONSTRUCTOR
   if the value is constant.  */
#define TREE_LITERAL(NODE) ((NODE)->common.literal_attr)

/* Nonzero in a ..._DECL means this variable is ref'd from a nested function.
   Cannot happen in C because it does not allow nested functions, as of now.
   For VAR_DECL nodes, PARM_DECL nodes, and
   maybe FUNCTION_DECL or LABEL_DECL nodes.

   C++: set for VAR_DECL nodes that exist via the class instance variable
   `this'.  */
#define TREE_NONLOCAL(NODE) ((NODE)->common.nonlocal_attr)

/* Nonzero means permanent node;
   node will continue to exist for the entire compiler run.
   Otherwise it will be recycled at the end of the function.  */
#define TREE_PERMANENT(NODE) ((NODE)->common.permanent_attr)

/* In INTEGER_TYPE or ENUMERAL_TYPE nodes, means an unsigned type.
   In FIELD_DECL nodes, means an unsigned bit field.  */
#define TREE_UNSIGNED(NODE) ((NODE)->common.unsigned_attr)

/* Nonzero in a VAR_DECL means assembler code has been written.
   Nonzero in a FUNCTION_DECL means that the function has been compiled.
   This is interesting in an inline function, since it might not need
   to be compiled separately.  */
#define TREE_ASM_WRITTEN(NODE) ((NODE)->common.asm_written_attr)

/* Nonzero in a FUNCTION_DECL means this function can be substituted
   where it is called.  */
#define TREE_INLINE(NODE) ((NODE)->common.inline_attr)

/* C++ extensions */
/* Nonzero in IDENTIFIER_NODE and FUNCTION_DECL means that
   this name is overloaded, and should be lookuped up
   in a non-standard way.  */
#define TREE_OVERLOADED(NODE) ((NODE)->common.overloaded_attr)

/* Nonzero in FUNCTION_DECL means that this node is
   really a method belonging to a class.  Could permit
   this to use non-standard calling sequences, but probably
   not a real win any more.  */
#define TREE_METHOD(NODE) ((NODE)->common.method_attr)

/* Nonzero in FIELD_DECL means that the underlying FUNCTION_DECL
   is really an operator.  */
#define TREE_OPERATOR(NODE) ((NODE)->common.method_attr)

/* Nonzero for FIELD_DECLs means that this field is private,
   and can only be accessed within the scope of the class
   which defines it (or its friends). */
#define TREE_PRIVATE(NODE) ((NODE)->common.private_attr)

/* Nonzero for FIELD_DECLs means that this field is protected,
   and can only be accessed within the scope of the class
   which defines it, its friends, or if there is a path in
   the type hierarchy from the current class scope to
   the one that defines it. */
#define TREE_PROTECTED(NODE) ((NODE)->common.protected_attr)

/* Nonzero for FIELD_DECLs and _TYPEs means that the argument
   is virtual.  For a FIELD_DECL, this means that while a direct
   function call can be made in certain cases, this function
   must generally be accessed via a virtual function table.
   For _TYPEs, it means that this type defines or uses a
   virtual function table for some of its methods.  */
#define TREE_VIRTUAL(NODE) ((NODE)->common.virtual_attr)

/* Nonzero for _TYPE means that the _TYPE defines
   at least one constructor.

   When appearing in an INDIRECT_REF, it means that the tree structure
   underneath is actually a call to a constructor.  This is needed
   when the constructor must initialize local storage (which can
   be automatically destroyed), rather than allowing it to allocate
   space from the heap.

   Overloaded for FUNCTION_DECLS: nonzero means that the constructor
   is known to return a non-zero `this' unchanged.  */
#define TREE_HAS_CONSTRUCTOR(NODE) ((NODE)->common.has_constructor_attr)
#define TREE_RETURNS_FIRST_ARG(NODE) ((NODE)->common.has_constructor_attr)

/* Nonzero for _TYPE means that the _TYPE defines
   a destructor.  This implicitly means that a constructor
   is also defined.  */
#define TREE_HAS_DESTRUCTOR(NODE) ((NODE)->common.has_destructor_attr)

/* Nonzero for _TYPE means that the _TYPE either has
   a special meaning for the assignment operator ("operator="),
   or one of its fields (or base members) has a special meaning
   defined.  */
#define TREE_HAS_ASSIGNMENT(NODE) ((NODE)->type.type_flags.has_assignment_attr)
#define TREE_GETS_ASSIGNMENT(NODE) ((NODE)->type.type_flags.gets_assignment_attr)

/* Nonzero for _TYPE means that operator new and delete are defined,
   respectively.  */
#define TREE_GETS_NEW(NODE) ((NODE)->type.type_flags.gets_new_attr)
#define TREE_GETS_DELETE(NODE) ((NODE)->type.type_flags.gets_delete_attr)

/* Nonzero for POINTER_TYPE nodes means that this POINTER_TYPE
   is of type pointer to member.  In this case, the TREE_TYPE
   field still refers to the type pointed to, but the TYPE_DOMAIN
   field tells what type we may point from.  */
#define TREE_MEMBER_POINTER(NODE) ((NODE)->common.member_pointer_attr)

/* Nonzero for TREE_LIST or _TYPE node means that the path to the
   base class is via a `public' declaration, which preserves public
   fields from the base class as public.
   OVERLOADED.  */
#define TREE_VIA_PUBLIC(NODE) ((NODE)->common.external_attr)

/* Nonzero for TREE_LIST or _TYPE node means that the path to the
   base class is via a `protected' declaration, which preserves
   protected fields from the base class as protected.
   OVERLOADED.  */
#define TREE_VIA_PROTECTED(NODE) ((NODE)->common.public_attr)

/* Nonzero for _TYPE node means that creating an object of this type
   will involve a call to a constructor.  */
#define TREE_NEEDS_CONSTRUCTOR(NODE) ((NODE)->type.type_flags.needs_ctor_attr)

/* Nonzero for _TYPE node means that destroying an object of this type
   will involve a call to a destructor.  */
#define TREE_NEEDS_DESTRUCTOR(NODE) ((NODE)->type.type_flags.needs_dtor_attr)

/* Nonzero means that this _TYPE node defines ways of converting
   itself to other types.  */
#define TREE_HAS_TYPE_CONVERSION(NODE) ((NODE)->type.type_flags.has_type_conversion_attr)

/* Nonzero means that this _TYPE node can convert itself to an
   INTEGER_TYPE.  */
#define TREE_HAS_INT_CONVERSION(NODE) ((NODE)->type.type_flags.has_int_conversion_attr)

/* Nonzero means that this _TYPE node can convert itself to an
   REAL_TYPE.  */
#define TREE_HAS_REAL_CONVERSION(NODE) ((NODE)->type.type_flags.has_float_conversion_attr)

/* Nonzero means that this _TYPE node overloads operator=(X&).  */
#define TREE_HAS_ASSIGN_REF(NODE) ((NODE)->type.type_flags.has_assign_ref_attr)
#define TREE_GETS_ASSIGN_REF(NODE) ((NODE)->type.type_flags.gets_assign_ref_attr)

/* Nonzero means that this _TYPE node has an X(X&) constructor.  */
#define TREE_GETS_INIT_REF(NODE) ((NODE)->type.type_flags.gets_init_ref_attr)

/* Nonzero means that this _TYPE node has an X(X ...) constructor.  */
#define TREE_GETS_INIT_AGGR(NODE) ((NODE)->type.type_flags.gets_init_aggr_attr)

/* Nonzero means that this _TYPE node overloads the method call
   operator.  In this case, all method calls go through `operator->()(...).  */
#define TREE_HAS_METHOD_CALL_OVERLOADED(NODE) ((NODE)->type.type_flags.has_method_call_overloaded)
#define TREE_HAS_WRAPPER(NODE) ((NODE)->type.type_flags.has_wrapper)
#define TREE_NEEDS_WRAPPER(NODE) ((NODE)->type.type_flags.needs_wrapper)

/* Nonzero  means that this _TYPE node overloads operator().  */
#define TREE_HAS_CALL_OVERLOADED(NODE) ((NODE)->type.type_flags.has_call_overloaded)

/* Nonzero  means that this _TYPE node overloads operator[].  */
#define TREE_HAS_ARRAY_REF_OVERLOADED(NODE) ((NODE)->type.type_flags.has_array_ref_overloaded)

/* Nonzero for VAR_DECL and PARM_DECL nodes, means that there is
   a reference, somewhere, to this _DECL during its lifetime.  */
#define TREE_EVERUSED(NODE) ((NODE)->common.everused_attr)

/* Nonzero for TREE_LIST node means that this list of things
   is a list of parameters, as opposed to a list of expressions.  */
#define TREE_PARMLIST(NODE) ((NODE)->common.regdecl_attr) /* overloaded! */

/* Nonzero for FIELD_DECL node means that this FIELD_DECL is
   a member of an anonymous union construct.  The name of the
   union is .  */
#define TREE_ANON_UNION_ELEM(NODE) ((NODE)->common.regdecl_attr) /* overloaded! */

/* Nonzero for FUNCTION_DECL means that this constructor may perform an
   assignment to `this', and therefore cannot be trusted.  Otherwise,
   we can treat the type that this function returns as a static type.  */
#define TREE_ASSIGNS_THIS(NODE) ((NODE)->common.private_attr) /* overloaded! */

/* Define additional fields and accessors for nodes representing constants.  */

/* In an INTEGER_CST node.  These two together make a 64 bit integer.
   If the data type is signed, the value is sign-extended to 64 bits
   even though not all of them may really be in use.
   In an unsigned constant shorter than 64 bits, the extra bits are 0.  */
#define TREE_INT_CST_LOW(NODE) ((NODE)->int_cst.int_cst_low)
#define TREE_INT_CST_HIGH(NODE) ((NODE)->int_cst.int_cst_high)

#define INT_CST_LT(A, B)  \
(TREE_INT_CST_HIGH (A) < TREE_INT_CST_HIGH (B)			\
 || (TREE_INT_CST_HIGH (A) == TREE_INT_CST_HIGH (B)		\
     && ((unsigned) TREE_INT_CST_LOW (A) < (unsigned) TREE_INT_CST_LOW (B))))

#define INT_CST_LT_UNSIGNED(A, B)  \
((unsigned) TREE_INT_CST_HIGH (A) < (unsigned) TREE_INT_CST_HIGH (B)	  \
 || ((unsigned) TREE_INT_CST_HIGH (A) == (unsigned) TREE_INT_CST_HIGH (B) \
     && ((unsigned) TREE_INT_CST_LOW (A) < (unsigned) TREE_INT_CST_LOW (B))))

struct tree_int_cst
{
  char common[sizeof (struct tree_common)];
  long int_cst_low;
  long int_cst_high;
};

/* In REAL_CST, STRING_CST, COMPLEX_CST nodes, and CONSTRUCTOR nodes,
   and generally in all kinds of constants that could
   be given labels (rather than being immediate).  */

#define TREE_CST_RTL(NODE) ((NODE)->real_cst.rtl)

/* In a REAL_CST node.  */
#define TREE_REAL_CST(NODE) ((NODE)->real_cst.real_cst)

struct tree_real_cst
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  double real_cst;
};

/* In a STRING_CST */
#define TREE_STRING_LENGTH(NODE) ((NODE)->string.length)
#define TREE_STRING_POINTER(NODE) ((NODE)->string.pointer)

struct tree_string
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  int length;
  char *pointer;
};

/* In a COMPLEX_CST node.  */
#define TREE_REALPART(NODE) ((NODE)->complex.real)
#define TREE_IMAGPART(NODE) ((NODE)->complex.imag)

struct tree_complex
{
  char common[sizeof (struct tree_common)];
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  union tree_node *real;
  union tree_node *imag;
};

/* Define fields and accessors for some special-purpose tree nodes.  */

#define IDENTIFIER_LENGTH(NODE) ((NODE)->identifier.length)
#define IDENTIFIER_POINTER(NODE) ((NODE)->identifier.pointer)
#define IDENTIFIER_GLOBAL_VALUE(NODE) ((NODE)->identifier.global_value)
#define IDENTIFIER_CLASS_VALUE(NODE) ((NODE)->identifier.class_value)
#define IDENTIFIER_LOCAL_VALUE(NODE) ((NODE)->identifier.local_value)
#define IDENTIFIER_LABEL_VALUE(NODE) ((NODE)->identifier.label_value)
#define IDENTIFIER_IMPLICIT_DECL(NODE) ((NODE)->identifier.implicit_decl)

struct tree_identifier
{
  char common[sizeof (struct tree_common)];
  int length;
  char *pointer;
  union tree_node *global_value;
  union tree_node *class_value;
  union tree_node *local_value;
  union tree_node *label_value;
  union tree_node *implicit_decl;
};

/* In a TREE_LIST node.  */
#define TREE_PURPOSE(NODE) ((NODE)->list.purpose)
#define TREE_VALUE(NODE) ((NODE)->list.value)

struct tree_list
{
  char common[sizeof (struct tree_common)];
  union tree_node *purpose;
  union tree_node *value;
};

/* Define fields and accessors for some nodes that represent expressions.  */

/* In a SAVE_EXPR node.  */
#define SAVE_EXPR_RTL(NODE) (*(struct rtx_def **) &(NODE)->exp.operands[1])

/* In a RTL_EXPR node.  */
#define RTL_EXPR_SEQUENCE(NODE) (*(struct rtx_def **) &(NODE)->exp.operands[0])
#define RTL_EXPR_RTL(NODE) (*(struct rtx_def **) &(NODE)->exp.operands[1])

/* In a CALL_EXPR node.  */
#define CALL_EXPR_RTL(NODE) (*(struct rtx_def **) &(NODE)->exp.operands[2])

/* In a CONSTRUCTOR node.  */
#define CONSTRUCTOR_ELTS(NODE) TREE_OPERAND (NODE, 1)

/* In expression and reference nodes.  */
#define TREE_OPERAND(NODE, I) ((NODE)->exp.operands[I])

struct tree_exp
{
  char common[sizeof (struct tree_common)];
  union tree_node *operands[1];
};

/* Define fields and accessors for nodes representing data types.  */

/* See tree.def for documentation of the use of these fields.  */

#define TYPE_SIZE(NODE) ((NODE)->type.size)
#define TYPE_SIZE_UNIT(NODE) ((NODE)->type.size_unit)
#define TYPE_MODE(NODE) ((NODE)->type.mode)
#define TYPE_ALIGN(NODE) ((NODE)->type.align)
#define TYPE_VALUES(NODE) ((NODE)->type.values)
#define TYPE_DOMAIN(NODE) ((NODE)->type.values)
#define TYPE_FIELDS(NODE) ((NODE)->type.values)
#define TYPE_FN_FIELDS(NODE) ((NODE)->type.max)
#define TYPE_ARG_TYPES(NODE) ((NODE)->type.values)
#define TYPE_SEP(NODE) ((NODE)->type.sep)
#define TYPE_SEP_UNIT(NODE) ((NODE)->type.sep_unit)
#define TYPE_POINTER_TO(NODE) ((NODE)->type.pointer_to)
#define TYPE_REFERENCE_TO(NODE) ((NODE)->type.reference_to)
#define TYPE_MIN_VALUE(NODE) ((NODE)->type.sep)
#define TYPE_MAX_VALUE(NODE) ((NODE)->type.max)
#define TYPE_PRECISION(NODE) ((NODE)->type.sep_unit)
#define TYPE_PARSE_INFO(NODE) ((NODE)->type.parse_info)
#define TYPE_SYMTAB_ADDRESS(NODE) ((NODE)->type.symtab_address)
#define TYPE_NAME(NODE) ((NODE)->type.name)
#define TYPE_NEXT_VARIANT(NODE) ((NODE)->type.next_variant)
#define TYPE_MAIN_VARIANT(NODE) ((NODE)->type.main_variant)

#define TYPE_BASELINK(NODE) ((NODE)->type.baselink)
#define TYPE_NELTS(NODE) ((NODE)->type.baselink)
#define TYPE_VIRTUALS(NODE) ((NODE)->type.name->decl.arguments)
#define TYPE_VSIZE(NODE) ((NODE)->type.name->decl.offset)
#define TYPE_TAGS(NODE) ((NODE)->type.tags)

/* Make things look a bit cleaner.  */
#define TYPE_NAME_STRING(NODE) (IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (NODE))))

struct tree_type
{
  char common[sizeof (struct tree_common)];
  union tree_node *values;
  union tree_node *sep;
  union tree_node *size;

  unsigned char size_unit;
  unsigned char align;
  unsigned char sep_unit;
  enum machine_mode mode : 8;

  /* This must fill out to 4 bytes.  */
  struct
    {
      unsigned needs_ctor_attr : 1;
      unsigned needs_dtor_attr : 1;
      unsigned has_type_conversion_attr : 1;
      unsigned has_int_conversion_attr : 1;
      unsigned has_float_conversion_attr : 1;
      unsigned gets_init_ref_attr : 1;
      unsigned gets_new_attr : 1;
      unsigned gets_delete_attr : 1;

      unsigned has_assignment_attr : 1;
      unsigned gets_assignment_attr : 1;
      unsigned has_assign_ref_attr : 1;
      unsigned gets_assign_ref_attr : 1;
      unsigned has_wrapper : 1;
      unsigned needs_wrapper : 1;
      unsigned has_method_call_overloaded : 1;
      unsigned returns_known_arg : 1;

      unsigned known_arg : 4;
      unsigned gets_init_aggr_attr : 1;
      unsigned has_call_overloaded : 1;
      unsigned has_array_ref_overloaded : 1;
      unsigned dummy9 : 9;
    } type_flags;

  union tree_node *pointer_to;
  union tree_node *reference_to; /* C++ */
  int parse_info;
  int symtab_address;
  union tree_node *name;
  union tree_node *max;
  union tree_node *next_variant;
  union tree_node *main_variant;

  /* C++ vagaries */
  union tree_node *baselink;
  union tree_node *tags;
};

/* Define fields and accessors for nodes representing declared names.  */

#define DECL_VOFFSET(NODE) ((NODE)->decl.voffset)
#define DECL_VOFFSET_UNIT(NODE) ((NODE)->decl.voffset_unit)
#define DECL_OFFSET(NODE) ((NODE)->decl.offset)
#define DECL_FUNCTION_CODE(NODE) ((enum built_in_function) (NODE)->decl.offset)
#define DECL_SET_FUNCTION_CODE(NODE,VAL) ((NODE)->decl.offset = (int) (VAL))
#define DECL_NAME(NODE) ((NODE)->decl.name)
#define DECL_ORIGINAL_NAME(NODE) ((NODE)->decl.original_name)
#define DECL_ASM_NAME(NODE) ((NODE)->decl.original_name)
#define DECL_CONTEXT(NODE) ((NODE)->decl.context)
#define DECL_ARGUMENTS(NODE) ((NODE)->decl.arguments)
#define DECL_ARG_TYPE(NODE) ((NODE)->decl.arguments)
#define DECL_RESULT(NODE) ((NODE)->decl.result)
#define DECL_INITIAL(NODE) ((NODE)->decl.initial)
#define DECL_SOURCE_FILE(NODE) ((NODE)->decl.filename)
#define DECL_SOURCE_LINE(NODE) ((NODE)->decl.linenum)
#define DECL_SIZE(NODE) ((NODE)->decl.size)
#define DECL_SIZE_UNIT(NODE) ((NODE)->decl.size_unit)
#define DECL_ALIGN(NODE) ((NODE)->decl.align)
#define DECL_MODE(NODE) ((NODE)->decl.mode)
#define DECL_RTL(NODE) ((NODE)->decl.rtl)
#define DECL_BLOCK_SYMTAB_ADDRESS(NODE) ((NODE)->decl.block_symtab_address)
#define DECL_SYMTAB_INDEX(NODE) ((NODE)->decl.block_symtab_address)
#define DECL_SAVED_INSNS(NODE) ((NODE)->decl.saved_insns)
#define DECL_FRAME_SIZE(NODE) ((NODE)->decl.frame_size)

/* C++: all of these are overloaded!  These apply only to FUNCTION_DECLs.  */
#define DECL_OPERATOR_NAME(NODE) ((char *)(NODE)->decl.voffset)
#define SET_DECL_OPERATOR_NAME(NODE,VAL) ((char *)((NODE)->decl.voffset = (tree)VAL))
#define OPERATOR_NAME_STRING(NODE) (DECL_OPERATOR_NAME (NODE) ? DECL_OPERATOR_NAME (NODE) : (SET_DECL_OPERATOR_NAME ((NODE), operator_name_string (DECL_ORIGINAL_NAME (NODE)))))

/* C++: all of these are overloaded!  These apply only to TYPE_DECLs.  */
#define DECL_FRIENDLIST(NODE) ((NODE)->decl.voffset)
#define DECL_UNDEFINED_FRIENDS(NODE) ((NODE)->decl.context)
#define DECL_WAITING_FRIENDS(NODE) ((tree)(NODE)->decl.rtl)
#define SET_DECL_WAITING_FRIENDS(NODE,VALUE) ((NODE)->decl.rtl=(struct rtx_def*)VALUE)
#define DECL_FRIEND_CLASSES(NODE) ((NODE)->decl.result)
#define DECL_VIRTUALS(NODE) ((NODE)->decl.arguments)
#define DECL_VSIZE(NODE) ((NODE)->decl.offset)
#define DECL_THIS_PTR(NODE) ((tree)(NODE)->decl.frame_size)
#define SET_DECL_THIS_PTR(NODE,VALUE) ((NODE)->decl.frame_size=(int)VALUE)
#define DECL_VTBL_PTR(NODE) ((tree)(NODE)->decl.saved_insns)
#define SET_DECL_VTBL_PTR(NODE,VALUE) ((NODE)->decl.saved_insns=(struct rtx_def*)VALUE)

/* C++: all of these are overloaded! These apply only to FIELD_DECLs.  */
#define DECL_STATIC_NAME(NODE) ((tree)(NODE)->decl.offset)
#define SET_DECL_STATIC_NAME(NODE,VAL) ((NODE)->decl.offset = (int)VAL)
#define DECL_VPARENT(NODE) ((NODE)->decl.arguments)
#define DECL_VISIBILITY(NODE) ((NODE)->decl.result)
#define DECL_VINDEX(NODE) ((tree)(NODE)->decl.offset)
#define SET_DECL_VINDEX(NODE,VAL) ((NODE)->decl.offset = (int)VAL)
#define DECL_BASELINK(NODE) ((tree)(NODE)->decl.frame_size)
#define SET_DECL_BASELINK(NODE,VAL) ((NODE)->decl.frame_size = (int)VAL)

struct tree_decl
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *size;
  enum machine_mode mode;
  unsigned char size_unit;
  unsigned char align;
  unsigned char voffset_unit;
  union tree_node *name;
  union tree_node *original_name;
  union tree_node *context;
  int offset;
  union tree_node *voffset;
  union tree_node *arguments;
  union tree_node *result;
  union tree_node *initial;
  struct rtx_def *rtl;	/* acts as link to register transfer language
				   (rtl) info */
  int frame_size;		/* For FUNCTION_DECLs: size of stack frame */
  struct rtx_def *saved_insns;	/* For FUNCTION_DECLs: points to insn that
				   constitutes its definition on the
				   permanent obstack.  */
  int block_symtab_address;
};

/* Define fields and accessors for nodes representing statements.
   These are now obsolete for C, except for LET_STMT, which is used
   to record the structure of binding contours (and the names declared
   in each contour) for the sake of outputting debugging info.
   Perhaps they will be used once again for other languages.  */

/* For LABEL_STMT, GOTO_STMT, RETURN_STMT, LOOP_STMT,
   COMPOUND_STMT, ASM_STMT.  */
#define STMT_SOURCE_LINE(NODE) ((NODE)->stmt.linenum)
#define STMT_SOURCE_FILE(NODE) ((NODE)->stmt.filename)
#define STMT_BODY(NODE) ((NODE)->stmt.body)

struct tree_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *body;
};

/* For IF_STMT.  */

/* #define STMT_SOURCE_LINE(NODE) */
/* #define STMT_SOURCE_FILE(NODE) */
#define STMT_COND(NODE) ((NODE)->if_stmt.cond)
#define STMT_THEN(NODE) ((NODE)->if_stmt.thenpart)
#define STMT_ELSE(NODE) ((NODE)->if_stmt.elsepart)

struct tree_if_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *cond, *thenpart, *elsepart;
};

/* For LET_STMT and WITH_STMT.  */

/* #define STMT_SOURCE_LINE(NODE) */
/* #define STMT_SOURCE_FILE(NODE) */
/* #define STMT_BODY(NODE) */
#define STMT_VARS(NODE) ((NODE)->bind_stmt.vars)
#define STMT_SUPERCONTEXT(NODE) ((NODE)->bind_stmt.supercontext)
#define STMT_BIND_SIZE(NODE) ((NODE)->bind_stmt.bind_size)
#define STMT_TYPE_TAGS(NODE) ((NODE)->bind_stmt.type_tags)

struct tree_bind_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *body, *vars, *supercontext, *bind_size, *type_tags;
};

/* For CASE_STMT.  */

#define STMT_CASE_INDEX(NODE) ((NODE)->case_stmt.index)
#define STMT_CASE_LIST(NODE) ((NODE)->case_stmt.case_list)

struct tree_case_stmt
{
  char common[sizeof (struct tree_common)];
  char *filename;
  int linenum;
  union tree_node *index, *case_list;
};

/* Define the overall contents of a tree node.
   It may be any of the structures declared above
   for various types of node.  */

union tree_node
{
  struct tree_common common;
  struct tree_int_cst int_cst;
  struct tree_real_cst real_cst;
  struct tree_string string;
  struct tree_complex complex;
  struct tree_identifier identifier;
  struct tree_decl decl;
  struct tree_type type;
  struct tree_list list;
  struct tree_exp exp;
  struct tree_stmt stmt;
  struct tree_if_stmt if_stmt;
  struct tree_bind_stmt bind_stmt;
  struct tree_case_stmt case_stmt;
};

extern char *oballoc ();
extern char *permalloc ();

/* Lowest level primitive for allocating a node.
   The TREE_CODE is the only argument.  Contents are initialized
   to zero except for a few of the common fields.  */

extern tree make_node ();

/* Make a copy of a node, with all the same contents except
   for TREE_UID and TREE_PERMANENT.  (The copy is permanent
   iff nodes being made now are permanent.)  */

extern tree copy_node ();

/* Return the (unique) IDENTIFIER_NODE node for a given name.
   The name is supplied as a char *.  */

extern tree get_identifier ();

/* Construct various types of nodes.  */

extern tree build_int_2 ();
extern tree build_real ();
extern tree build_real_from_string ();
extern tree build_real_from_int_cst ();
extern tree build_complex ();
extern tree build_string ();
extern tree build ();
extern tree build_nt ();
extern tree build_tree_list ();
extern tree build_decl ();
extern tree build_let ();

/* Construct various nodes representing data types.  */

extern tree make_signed_type ();
extern tree make_unsigned_type ();
extern void fixup_unsigned_type ();
extern tree build_pointer_type ();
extern tree build_reference_type ();
extern tree build_array_type ();
extern tree build_function_type ();

/* Construct expressions, performing type checking.  */

extern tree build_binary_op ();
extern tree build_indirect_ref ();
extern tree build_unary_op ();

/* Given a type node TYPE, and CONSTP and VOLATILEP, return a type
   for the same kind of data as TYPE describes.
   Variants point to the "main variant" (which has neither CONST nor VOLATILE)
   via TYPE_MAIN_VARIANT, and it points to a chain of other variants
   so that duplicate variants are never made.
   Only main variants should ever appear as types of expressions.  */

extern tree build_type_variant ();

/* Given a ..._TYPE node, calculate the TYPE_SIZE, TYPE_SIZE_UNIT,
   TYPE_ALIGN and TYPE_MODE fields.
   If called more than once on one node, does nothing except
   for the first time.  */

extern void layout_type ();

/* Given a hashcode and a ..._TYPE node (for which the hashcode was made),
   return a canonicalized ..._TYPE node, so that duplicates are not made.
   How the hash code is computed is up to the caller, as long as any two
   callers that could hash identical-looking type nodes agree.  */

extern tree type_hash_canon ();

/* Given a VAR_DECL, PARM_DECL, RESULT_DECL or FIELD_DECL node,
   calculates the DECL_SIZE, DECL_SIZE_UNIT, DECL_ALIGN and DECL_MODE
   fields.  Call this only once for any given decl node.

   Second argument is the boundary that this field can be assumed to
   be starting at (in bits).  Zero means it can be assumed aligned
   on any boundary that may be needed.  */

extern void layout_decl ();

/* Fold constants as much as possible in an expression.
   Returns the simplified expression.
   Acts only on the top level of the expression;
   if the argument itself cannot be simplified, its
   subexpressions are not changed.  */

extern tree fold ();

/* combine (tree_code, exp1, exp2) where EXP1 and EXP2 are constants
   returns a constant expression for the result of performing
   the operation specified by TREE_CODE on EXP1 and EXP2.  */

extern tree combine ();

extern tree convert ();
extern tree convert_units ();
extern tree size_in_bytes ();
extern tree genop ();
extern tree build_int ();

/* Type for sizes of data-type.  */

extern tree sizetype;

/* Concatenate two lists (chains of TREE_LIST nodes) X and Y
   by making the last node in X point to Y.
   Returns X, except if X is 0 returns Y.  */

extern tree chainon ();

/* Make a new TREE_LIST node from specified PURPOSE, VALUE and CHAIN.  */

extern tree tree_cons ();

/* Return the last tree node in a chain.  */

extern tree tree_last ();

/* Reverse the order of elements in a chain, and return the new head.  */

extern tree nreverse ();

/* Returns the length of a chain of nodes
   (number of chain pointers to follow before reaching a null pointer).  */

extern int list_length ();

/* integer_zerop (tree x) is nonzero if X is an integer constant of value 0 */

extern int integer_zerop ();

/* integer_onep (tree x) is nonzero if X is an integer constant of value 1 */

extern int integer_onep ();

/* integer_all_onesp (tree x) is nonzero if X is an integer constant
   all of whose significant bits are 1.  */

extern int integer_all_onesp ();

/* type_unsigned_p (tree x) is nonzero if the type X is an unsigned type
   (all of its possible values are >= 0).
   If X is a pointer type, the value is 1.
   If X is a real type, the value is 0.  */

extern int type_unsigned_p ();

/* staticp (tree x) is nonzero if X is a reference to data allocated
   at a fixed address in memory.  */

extern int staticp ();

/* Gets an error if argument X is not an lvalue.
   Also returns 1 if X is an lvalue, 0 if not.  */

extern int lvalue_or_else ();

/* save_expr (EXP) returns an expression equivalent to EXP
   but it can be used multiple times within context CTX
   and only evaluate EXP once.  */

extern tree save_expr ();

/* stabilize_reference (EXP) returns an reference equivalent to EXP
   but it can be used multiple times
   and only evaluate the subexpressions once.  */

extern tree stabilize_reference ();

/* Return EXP, stripped of any conversions to wider types
   in such a way that the result of converting to type FOR_TYPE
   is the same as if EXP were converted to FOR_TYPE.
   If FOR_TYPE is 0, it signifies EXP's type.  */

extern tree get_unwidened ();

/* Return OP or a simpler expression for a narrower value
   which can be sign-extended or zero-extended to give back OP.
   Store in *UNSIGNEDP_PTR either 1 if the value should be zero-extended
   or 0 if the value should be sign-extended.  */

extern tree get_narrower ();

/* Given PRECISION and UNSIGNEDP, return a suitable type-tree
   for an integer type with at least that precision.
   The definition of this resides in language-specific code
   as the repertoir of available types may vary.  */

extern tree type_for_size ();

/* Given an integer type T, return a type like T but unsigned.
   If T is unsigned, the value is T.
   The definition of this resides in language-specific code
   as the repertoir of available types may vary.  */

extern tree unsigned_type ();

/* Given an integer type T, return a type like T but signed.
   If T is signed, the value is T.
   The definition of this resides in language-specific code
   as the repertoir of available types may vary.  */

extern tree signed_type ();

/* Return the floating type node for a given floating machine mode.  */

extern tree get_floating_type ();

/* Given the FUNCTION_DECL for the current function,
   return zero if it is ok for this function to be inline.
   Otherwise return a warning message with a single %s
   for the function's name.  */

extern char *function_cannot_inline_p ();

/* Declare commonly used variables for tree structure.  */

/* An integer constant with value 0 */
extern tree integer_zero_node;

/* An integer constant with value 1 */
extern tree integer_one_node;

/* A constant of type pointer-to-int and value 0 */
extern tree null_pointer_node;

/* A node of type ERROR_MARK.  */
extern tree error_mark_node;
/* A node that is a list (length 1) of error_mark_nodes.  */
extern tree error_mark_list;

/* The type node for the void type.  */
extern tree void_type_node;

/* The type node for the ordinary (signed) integer type.  */
extern tree integer_type_node;

/* The type node for the unsigned integer type.  */
extern tree unsigned_type_node;

/* The type node for the ordinary character type.  */
extern tree char_type_node;

/* C++ extensions: keep these around */
extern tree ptr_type_node;
extern tree class_type_node, record_type_node, union_type_node, enum_type_node;
extern tree unknown_type_node;

extern tree get_temp_name (), get_temp_regvar ();
extern tree build_classfn_ref ();
extern tree build_type_conversion ();
extern tree maybe_convert_decl_to_const ();

/* in init.c */
extern tree build_up_reference (), bash_reference_type ();
extern tree resolve_scope_ref ();

/* in lex.c  */
extern char *operator_name_string ();
/* -- end of C++ */

/* Points to the name of the input file from which the current input
   being parsed originally came (before it went into cpp).  */
extern char *input_filename;
extern char *main_input_filename;

/* Nonzero for -pedantic switch: warn about anything
   that standard C forbids.  */
extern int pedantic;

/* In stmt.c */

extern tree get_last_expr ();
extern void expand_expr_stmt(), clear_last_expr();
extern void expand_label(), expand_goto(), expand_asm();
extern void expand_start_cond(), expand_end_cond();
extern void expand_start_else(), expand_end_else();
extern void expand_start_loop(), expand_start_loop_continue_elsewhere();
extern void expand_loop_continue_here();
extern void expand_end_loop();
extern int expand_continue_loop();
extern int expand_exit_loop(), expand_exit_loop_if_false();
extern int expand_exit_something();

extern void expand_start_delayed_expr ();
extern tree expand_end_delayed_expr ();
extern void expand_emit_delayed_expr ();

extern void expand_null_return(), expand_return();
extern void expand_start_bindings(), expand_end_bindings();
extern void expand_start_case(), expand_end_case();
extern void pushcase(), pushcase_range ();
extern void expand_start_function(), expand_end_function();
