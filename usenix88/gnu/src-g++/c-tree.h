/* Definitions for C parsing and type checking.
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

/* Nonzero means reject anything that ANSI standard C forbids.  */
extern int pedantic;

/* C++ extensions */
/* Zero means prototype weakly, as in ANSI C (no args means nothing).  */
extern int strict_prototype;
/* Non-zero means that if a label exists, and no other identifier
   applies, use the value of the label.  */
extern int flag_labels_ok;

/* In a RECORD_TYPE or UNION_TYPE, nonzero if any component is read-only.  */
#define C_TYPE_FIELDS_READONLY(type) TYPE_SEP_UNIT (type)

/* in typecheck.c */
extern tree build_x_conditional_expr ();
extern tree build_x_unary_op (), build_x_binary_op ();
extern tree build_x_function_call ();
extern tree build_x_indirect_ref (), build_x_array_ref ();
extern tree build_x_modify_expr (), build_x_modify_op_expr ();

extern tree build_component_ref (), build_m_component_ref ();
extern tree build_component_type_expr ();
extern tree build_conditional_expr(), build_compound_expr();
extern tree build_x_arrow ();

extern tree build_unary_op(), build_binary_op(), build_function_call();
extern tree build_binary_op_nodefault ();
extern tree build_indirect_ref(), build_array_ref(), build_c_cast();
extern tree build_modify_expr();
extern tree c_sizeof (), c_alignof ();
extern void store_init_value ();
extern tree digest_init ();
extern tree c_expand_start_case ();
extern tree default_conversion ();
extern tree build_return_stmt ();
extern tree actualparameterlist ();

/* in tree.c */
extern tree build_let ();

/* C++ extensions */
/* in decl.c */
extern tree lookup_some_tag ();
extern void hack_incomplete_structures ();

/* The largest size a virtual function table can be.
   Must be a (power of 2).  */
#define VINDEX_MAX ((unsigned)128)
/* This is the integer ~ (vindex_max - 1).  */
extern tree vtbl_mask;

/* Array type `(void *)[]' */
extern tree vtbl_type_node;

/* in overload.c */
extern tree current_class_name;
extern tree current_class_type;

extern tree hack_identifier (), hack_operator (), hack_wrapper ();
extern tree convert_to_aggr ();
extern tree build_x_new (), build_x_delete ();
extern tree build_new (), build_vec_new (), build_delete (), build_vec_delete ();
extern tree make_destructor_name ();
extern tree build_classfn_ref ();
extern tree do_actual_overload ();
extern tree start_method (), start_type_method ();
extern void finish_method ();

extern tree get_field (), get_fnfields ();

void pushclass (), popclass (), pushclasstype();
extern tree build_operator_fnname (), build_opfncall (), build_type_conversion ();
extern tree build_wrapper ();

/* One non-C++ declaration... */
/* Points to the FUNCTION_DECL of the function whose body we are reading. */
extern tree current_function_decl;

/* Points to the name of that function. May not be the DECL_NAME
   of CURRENT_FUNCTION_DECL due to overloading */
extern tree current_function_name;

/* See comments in `overload.c' */
# define IS_AGGR_TYPE(t) \
  (TREE_CODE (t) == RECORD_TYPE \
   || TREE_CODE (t) == UNION_TYPE)

# define IS_AGGR_TYPE_CODE(t) \
  (t == RECORD_TYPE \
   || t == UNION_TYPE)

extern tree do_decl_overload (), do_typename_overload ();
extern tree build_destructor_call ();
extern tree current_class_name, current_class_type, current_class_decl, C_C_D;
extern tree current_vtable_decl;

/* in init.c  */
extern tree purpose_member (), value_member ();
extern void check_base_init ();
extern void do_member_init ();
extern tree global_base_init_list;
extern tree current_base_init_list, current_member_init_list;
extern tree get_vtable_name (), get_vfield_name ();
extern tree build_member_call (), build_member_ref ();

extern int current_function_assigns_this;
extern int current_function_just_assigned_this;

/* Cannot use '$' up front, because this confuses gdb.
   Note that any format of this kind *must* make the
   format for `this' lexicgraphically less than any other
   parameter name, i.e. "$this" is less than anything else can be.

   Note that all forms in which the '$' is significant are long enough
   for direct indexing.  */

#define THIS_NAME "$this"
#define VPTR_NAME "$vptr"
#define DESTRUCTOR_NAME_FORMAT "~%s"
#define DESTRUCTOR_DECL_FORMAT "_$_%s"
#define WRAPPER_NAME_FORMAT "()%s"
#define WRAPPER_DECL_FORMAT "_WRAPPER_"
#define ANTI_WRAPPER_NAME_FORMAT "~()%s"
#define ANTI_WRAPPER_DECL_FORMAT "_~WRAPPER_"
#define AUTO_DELETE_NAME "__delete$me__"
#define AUTO_TEMP_NAME "_$tmp_"
#define AUTO_TEMP_FORMAT "_$tmp_%d"
#define OPERATOR_ASSIGN_FORMAT "op$assign_%s"
#define OPERATOR_FORMAT "op$%s"
#define VTABLE_NAME_FORMAT "_vt$%s"
#define VFIELD_NAME "_vptr$"
#define VFIELD_NAME_FORMAT "_vptr$%s"
#define STATIC_NAME_FORMAT "_%s$%s"
#define OPERATOR_TYPENAME_FORMAT "type$"

#define THIS_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[0] == '$' \
			      && IDENTIFIER_POINTER (ID_NODE)[1] == 't')
#define VPTR_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[0] == '$' \
			      && IDENTIFIER_POINTER (ID_NODE)[1] == 'v')
#define DESTRUCTOR_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[1] == '$')

#define WRAPPER_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[1] == 'W' \
				 && IDENTIFIER_POINTER (ID_NODE)[0] == '_')
#define ANTI_WRAPPER_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[1] == '~')

#define OPERATOR_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[2] == '$' \
  && IDENTIFIER_POINTER (ID_NODE)[1])

#define VTABLE_NAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[3] == '$' \
  && IDENTIFIER_POINTER (ID_NODE)[2] \
  && IDENTIFIER_POINTER (ID_NODE)[1])

#define OPERATOR_TYPENAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[4] == '$' \
  && IDENTIFIER_POINTER (ID_NODE)[3] \
  && IDENTIFIER_POINTER (ID_NODE)[2] \
  && IDENTIFIER_POINTER (ID_NODE)[1])

#define TEMP_NAME_P(ID_NODE) (!strncmp (IDENTIFIER_POINTER (ID_NODE), AUTO_TEMP_NAME, sizeof (AUTO_TEMP_NAME)-1))
#define VFIELD_NAME_P(ID_NODE) (!strncmp (IDENTIFIER_POINTER (ID_NODE), VFIELD_NAME, sizeof(VFIELD_NAME)-1))

/* For anonymous aggregate types, we need some sort of name to
   hold on to.  In practice, this should not appear, but it should
   not be harmful if it does.  */
#define ANON_AGGRNAME_FORMAT "$_%d"
#define ANON_AGGRNAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[0] == '$')

#define ANON_PARMNAME_FORMAT "_%d"
#define ANON_PARMNAME_P(ID_NODE) (IDENTIFIER_POINTER (ID_NODE)[0] == '_' \
				  && IDENTIFIER_POINTER (ID_NODE)[1] <= '9')

enum visibility_type {
  visibility_default,
  visibility_public,
  visibility_private,
  visibility_protected,
  visibility_default_virtual,
  visibility_public_virtual,
  visibility_private_virtual,
};

/* -- end of C++ */

/* Given two integer or real types, return the type for their sum.
   Given two compatible ANSI C types, returns the merged type.  */

extern tree commontype ();

/* in decl.c */
extern tree build_label ();

extern int start_function ();
extern void finish_function ();
extern tree get_parm_types ();
extern void store_parm_decls ();

extern void pushlevel(), poplevel();

extern tree grokopexpr ();
extern tree groktypename(), lookup_name();

extern tree lookup_label(), define_label();

extern tree implicitly_declare(), getdecls(), gettags (), getaggrs ();

extern tree start_decl();
extern void finish_decl();

extern tree start_struct(), finish_struct(), xref_tag();
extern void finish_anon_union();
extern tree grokfield(), groktypefield ();

extern tree start_enum(), finish_enum();
extern tree build_enumerator();

extern tree make_index_type ();

extern tree double_type_node, long_double_type_node, float_type_node;
extern tree unsigned_char_type_node, signed_char_type_node;

extern tree short_integer_type_node, short_unsigned_type_node;
extern tree long_integer_type_node, long_unsigned_type_node;
extern tree unsigned_type_node;
extern tree string_type_node, char_array_type_node, int_array_type_node;

extern int current_function_returns_value;
extern int current_function_returns_null;

extern void yyerror();
extern int lineno;

extern tree ridpointers[];

/* Points to the FUNCTION_DECL of the function whose body we are reading. */
extern tree current_function_decl;

/* Things for handling inline functions.  */

struct pending_inline
{
  struct pending_inline *next;	/* pointer to next in chain */
  int lineno;			/* line number we got the text from */
  char *filename;		/* name of file we were processing */
  tree fndecl;			/* FUNCTION_DECL that brought us here */
  int token;			/* token we were scanning */
  int token_value;		/* value of token we were scanning (YYSTYPE) */

  char *buf;			/* pointer to character stream */
  int len;			/* length of stream */
};

/* in method.c */
extern tree wrapper_name, anti_wrapper_name;
extern struct pending_inline *pending_inlines;
extern struct pending_inline *hack_inline_prefix ();
extern char *hack_function_decl ();
