/* YACC parser for C++ syntax.
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


/*  This grammar is based on the GNU CC grammar.  */

%{
#include "config.h"
#include "tree.h"
#include "parse.h"
#include "c-tree.h"
#include "flags.h"

/* C++ extensions */
extern tree ridpointers[];	/* need this up here */

#include <stdio.h>

/* Cause the `yydebug' variable to be defined.  */
#define YYDEBUG
%}

%start program

%union {long itype; tree ttype; enum tree_code code; char *cptr; }

/* All identifiers that are not reserved words
   and are not declared typedefs in the current block */
%token IDENTIFIER

/* All identifiers that are declared typedefs in the current block.
   In some contexts, they are treated just like IDENTIFIER,
   but they can also serve as typespecs in declarations.  */
%token TYPENAME

/* Reserved words that specify storage class.
   yylval contains an IDENTIFIER_NODE which indicates which one.  */
%token SCSPEC

/* Reserved words that specify type.
   yylval contains an IDENTIFIER_NODE which indicates which one.  */
%token TYPESPEC

/* Reserved words that qualify type: "const" or "volatile".
   yylval contains an IDENTIFIER_NODE which indicates which one.  */
%token TYPE_QUAL

/* Character or numeric constants.
   yylval is the node for the constant.  */
%token CONSTANT

/* String constants in raw form.
   yylval is a STRING_CST node.  */
%token STRING

/* "...", used for functions with variable arglists.  */
%token ELLIPSIS

/* the reserved words */
%token SIZEOF ENUM /* STRUCT UNION */ IF ELSE WHILE DO FOR SWITCH CASE DEFAULT
%token BREAK CONTINUE RETURN GOTO ASM TYPEOF ALIGNOF

/* the reserved words... C++ extensions */
%token <ttype> AGGR
%token DELETE NEW OVERLOAD PRIVATE PUBLIC PROTECTED THIS OPERATOR
%token <itype> SCOPE

/* Define the operator tokens and their precedences.
   The value is an integer because, if used, it is the tree code
   to use in the expression made from the operator.  */

%left EMPTY			/* used to resolve s/r with epsilon */

%left ELSE			/* used to resolve classic `else' s/r conflict */

%left IDENTIFIER TYPENAME SCSPEC TYPESPEC TYPE_QUAL ENUM AGGR

%left ','

%right <code> ASSIGN '='
%right <code> '?' ':' RANGE
%left <code> OROR
%left <code> ANDAND
%left <code> '|'
%left <code> '^'
%left <code> '&'
%left <code> MIN_MAX
%left <code> EQCOMPARE
%left <code> ARITHCOMPARE
%left <code> LSHIFT RSHIFT
%left <code> '+' '-'
%left <code> '*' '/' '%'
%right <code> UNARY PLUSPLUS MINUSMINUS
%left HYPERUNARY
%left <code> POINTSAT '.' '(' '['

%right SCOPE			/* C++ extension */
%nonassoc NEW DELETE

%type <code> unop

%type <ttype> identifier IDENTIFIER TYPENAME CONSTANT expr nonnull_exprlist exprlist
%type <ttype> expr_no_commas primary string STRING
%type <ttype> typed_declspecs reserved_declspecs
%type <ttype> typed_typespecs reserved_typespecquals
%type <ttype> declmods typespecqual_reserved
%type <ttype> typespec SCSPEC TYPESPEC TYPE_QUAL nonempty_type_quals maybe_type_qual
%type <itype> initdecls notype_initdecls initdcl notype_initdcl	/* C++ modification */
%type <ttype> init initlist maybeasm
%type <ttype> asm_operands asm_operand

%type <ttype> declarator notype_declarator after_type_declarator

%type <ttype> structsp opt.component_decl_list component_decl_list component_decl components component_declarator
%type <ttype> enumlist enumerator
%type <ttype> typename absdcl absdcl1 type_quals
%type <ttype> xexpr parmlist parms parm bad_parm

/* C++ extensions */
%token <ttype> TYPENAME_COLON TYPENAME_SCOPE TYPENAME_ELLIPSIS
%token <ttype> PRE_PARSED_FUNCTION_DECL
%type <ttype> fn.def2 dummy_decl x_typespec x_typed_declspecs
%type <ttype> class_head opt.init base_class_list base_class_visibility_list
%type <ttype> after_type_declarator_no_typename
%type <ttype> component_declarator0 scoped_declarator
%type <ttype> forhead.1 identifier_or_opname operator_name
%type <ttype> new object
%type <itype> LC forhead.2 initdcl0 notype_initdcl0 wrapper

%{
/* the declaration found for the last IDENTIFIER token read in.
   yylex must look this up to detect typedefs, which get token type TYPENAME,
   so it is left around in case the identifier is not a typedef but is
   used in a context which makes it a reference to a variable.  */
tree lastiddecl;

tree make_pointer_declarator (), make_reference_declarator ();

tree combine_strings ();
void reinit_parse_for_function ();
void reinit_parse_for_method ();

/* list of types and structure classes of the current declaration */
tree current_declspecs;

char *input_filename;		/* file being read */
char *main_input_filename;	/* top-level source file */
%}

%%
program: /* empty */
	| extdefs
		{ finish_file (); }
	;

/* the reason for the strange actions in this rule
 is so that notype_initdecls when reached via datadef
 can find a valid list of type and sc specs in $0. */

extdefs:
	  {$<ttype>$ = NULL_TREE; } extdef
	| extdefs {$<ttype>$ = NULL_TREE; } extdef
	;

extdef:
	  fndef
		{ if (pending_inlines) do_pending_inlines (); }
	| datadef
		{ if (pending_inlines) do_pending_inlines (); }
	| overloaddef
	;

overloaddef:
	  OVERLOAD ov_identifiers ';'

ov_identifiers: IDENTIFIER
		{ declare_overloaded ($1); }
	| ov_identifiers ',' IDENTIFIER
		{ declare_overloaded ($3); }
	;
	  
dummy_decl: /* empty */
		{ $$ = NULL_TREE; }
	;

datadef:
	  dummy_decl notype_initdecls ';'
		{ if (pedantic)
		    error ("ANSI C forbids data definition lacking type or storage class");
  		  else
  		    warning ("data definition lacks type or storage class"); }
	| declmods notype_initdecls ';'
		{}
	| typed_declspecs initdecls ';'
		{}
        | declmods ';'
	  { error ("empty declaration"); }
	| typed_declspecs ';'
	  { shadow_tag ($1); }
	| error ';'
	| error '}'
	| ';'
	;

fndef:
	  fn.def1 base_init compstmt
		{ finish_function (input_filename, @2.first_line); }
	| fn.def1 base_init error compstmt
		{ finish_function (input_filename, @2.first_line); }
	| fn.def1 xdecls compstmt
		{ finish_function (input_filename, @2.first_line); }
	| typed_declspecs declarator error
		{}
	| declmods notype_declarator error
		{}
	| dummy_decl notype_declarator error
		{}
	;

fn.def1:
	  typed_declspecs declarator
		{ if (! start_function ($1, $2, 0))
		    YYFAIL;
		  reinit_parse_for_function (); }
	| declmods notype_declarator
		{ if (! start_function ($1, $2, 0))
		    YYFAIL;
		  reinit_parse_for_function (); }
	| dummy_decl notype_declarator
		{ if (! start_function (NULL_TREE, $2, 0))
		    YYFAIL;
		  reinit_parse_for_function (); }
	| error '{'
		{ start_function (NULL_TREE, get_identifier ("?function"), 0); }
	| PRE_PARSED_FUNCTION_DECL
		{ start_function (NULL_TREE, $1, 1);
		  reinit_parse_for_function (); }
	;

/* more C++ complexity */
fn.def2:
	  typed_declspecs '(' parmlist ')'
		{
		  tree decl = build_nt (CALL_EXPR, TREE_VALUE ($1), $3);
		  $$ = start_method (TREE_CHAIN ($1), decl);
		  if (! $$)
		    YYFAIL;
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  reinit_parse_for_method (yychar); }
	| typed_declspecs notype_declarator
		{ $$ = start_method ($1, $2);
		  if (! $$)
		    YYFAIL;
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  reinit_parse_for_method (yychar); }
	| declmods '(' parmlist ')'
		{
		  tree decl = build_nt (CALL_EXPR, TREE_VALUE ($1), $3);
		  $$ = start_method (TREE_CHAIN ($1), decl);
		  if (! $$)
		    YYFAIL;
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  reinit_parse_for_method (yychar); }
	| declmods notype_declarator
		{ $$ = start_method ($1, $2);
		  if (! $$)
		    YYFAIL;
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  reinit_parse_for_method (yychar); }
	| dummy_decl notype_declarator
		{ $$ = start_method (NULL_TREE, $2);
		  if (! $$)
		    YYFAIL;
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  reinit_parse_for_method (yychar); }
	;

base_init:
	  ':' .set_base_init member_init_list
		{
		  poplevel (0, 0, 0);
		  setup_vtbl_ptr ();
		}
	| ':' error
		{
		  poplevel (0, 0, 0);
		  setup_vtbl_ptr ();
		}
	;

.set_base_init:
	/* empty */
		{
		  /* @@ current_function_name or original_function_name? */
		  char *errmsg = NULL;

		  if (current_class_name == NULL_TREE)
		    errmsg = "base initializers not allowed here";
		  else if (current_function_name != current_class_name)
		    errmsg = "only constructors take base initializers";

		  store_parm_decls ();
		  pushlevel (0);
		  if (errmsg)
		    {
		      error (errmsg);
		      yyerrstatus = 3;
		      YYFAIL;
		    }
		}
	;

member_init_list:
	  /* empty */
	| member_init
	| member_init_list ',' member_init
	;

member_init:
	  '(' exprlist ')'
		{
#if 0
		  warning ("old style base class initialization; use `%s (...)'",
			   IDENTIFIER_POINTER (current_class_name));
#endif
		  if ($2 == NULL_TREE)
		    $2 = void_type_node;
		  do_member_init (C_C_D, NULL_TREE, $2);
		}
	| identifier '(' exprlist ')'
		{
		  if ($3 == NULL_TREE)
		    $3 = void_type_node;
		  do_member_init (C_C_D, $1, $3);
		}
	;

identifier:
	  IDENTIFIER
	| TYPENAME
	;

identifier_or_opname:
	  IDENTIFIER
	| TYPENAME
	| operator_name
		{ $$ = hack_operator ($1); }
	| wrapper IDENTIFIER
		{ $$ = hack_wrapper ($1, NULL_TREE, $2); }
	| wrapper TYPENAME
		{ $$ = hack_wrapper ($1, NULL_TREE, $2); }
	| wrapper operator_name
		{ $$ = hack_wrapper ($1, NULL_TREE, $2); }
	| wrapper TYPENAME_SCOPE IDENTIFIER
		{ $$ = hack_wrapper ($1, $2, $3); }
	| wrapper TYPENAME_SCOPE operator_name
		{ $$ = hack_wrapper ($1, $2, $3); }
	;

wrapper:  '(' ')'
		{ $$ = 0; }
	| '~' '(' ')'
		{ $$ = 1; }
	;

unop:     '-'
		{ $$ = NEGATE_EXPR; }
	| '+'
		{ $$ = CONVERT_EXPR; }
	| PLUSPLUS
		{ $$ = PREINCREMENT_EXPR; }
	| MINUSMINUS
		{ $$ = PREDECREMENT_EXPR; }
	| '!'
		{ $$ = TRUTH_NOT_EXPR; }
	;

expr:	nonnull_exprlist
		{ $$ = build_compound_expr ($1); }
	;

exprlist:
	  /* empty */
		{ $$ = NULL_TREE; }
	| nonnull_exprlist
	;

nonnull_exprlist:
	  expr_no_commas
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| nonnull_exprlist ',' expr_no_commas
		{ chainon ($1, build_tree_list (NULL_TREE, $3)); }
	| nonnull_exprlist ',' error
		{ chainon ($1, build_tree_list (NULL_TREE, error_mark_node)); }
	;

expr_no_commas:
	  primary %prec UNARY
		{
		  if (TREE_CODE ($1) == TYPE_EXPR)
		    $$ = build_component_type_expr (C_C_D, $1, 1);
		  else
		    $$ = $1;
		}
	| '*' expr_no_commas   %prec UNARY
		{ $$ = build_x_indirect_ref ($2, "unary *"); }
	| '&' expr_no_commas   %prec UNARY
		{ $$ = build_x_unary_op (ADDR_EXPR, $2); }
	| '~' expr_no_commas   %prec UNARY
		{ $$ = build_x_unary_op (BIT_NOT_EXPR, $2); }
	| unop expr_no_commas  %prec UNARY
		{ $$ = build_x_unary_op ($1, $2, 0); }
	| '(' typename ')' expr_no_commas  %prec UNARY
		{ tree type = groktypename ($2);
		  $$ = build_c_cast (type, $4); }
	| '(' typename ')' '{' initlist maybecomma '}'  %prec UNARY
		{ tree type = groktypename ($2);
		  if (pedantic)
		    warning ("ANSI C forbids constructor-expressions");
		  $$ = digest_init (type, build_nt (CONSTRUCTOR, NULL_TREE, nreverse ($5)), 0);
		  if (TREE_CODE (type) == ARRAY_TYPE && TYPE_SIZE (type) == 0)
		    {
		      int failure = complete_array_type (type, $$, 1);
		      if (failure)
			abort ();
		    }
		}
	| SIZEOF expr_no_commas  %prec UNARY
		{ if (TREE_CODE ($2) == COMPONENT_REF
		      && TREE_PACKED (TREE_OPERAND ($2, 1)))
		    error ("sizeof applied to a bit-field");
		  $$ = c_sizeof (TREE_TYPE ($2)); }
	| SIZEOF '(' typename ')'  %prec HYPERUNARY
		{ $$ = c_sizeof (groktypename ($3)); }
	| ALIGNOF expr_no_commas  %prec UNARY
		{ if (TREE_CODE ($2) == COMPONENT_REF
		      && TREE_PACKED (TREE_OPERAND ($2, 1)))
		    error ("__alignof applied to a bit-field");
		  $$ = c_alignof (TREE_TYPE ($2)); }
	| ALIGNOF '(' typename ')'  %prec HYPERUNARY
		{ $$ = c_alignof (groktypename ($3)); }

	/* C++ extension.  */
	| DELETE expr_no_commas  %prec UNARY
		{ tree expr = bash_reference_type ($2);
		  tree type = TREE_TYPE (expr);

		  if (TREE_CODE (type) != POINTER_TYPE)
		    {
		      error ("non-pointer type to `delete'");
		      $$ = error_mark_node;
		    }
		  else if (TREE_GETS_DELETE (TREE_TYPE (type)))
		    $$ = build_opfncall (DELETE_EXPR, current_class_decl, expr);
		  else
		    {
		      $$ = build_delete (type, expr, integer_one_node);
		    }
		}
	| DELETE '[' expr ']' expr_no_commas  %prec UNARY
		{
		  tree maxindex = build_binary_op (MINUS_EXPR, $3, integer_one_node);
		  tree exp = bash_reference_type ($5);

		  if (yychar == YYEMPTY)
		    yychar = YYLEX;

		  if (yychar == ';')
		    {
		      /* This is an optimization that allows vector
			 delete to be called as an inline function.  */
		      expand_vec_delete (maxindex, exp, integer_one_node);
		      $$ = error_mark_node;
		    }
		  else
		    {
		      $$ = build_vec_delete (maxindex, exp, integer_one_node, integer_zero_node);
		    }
		}

	| expr_no_commas '+' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '-' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '*' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '/' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '%' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas LSHIFT expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas RSHIFT expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas ARITHCOMPARE expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas EQCOMPARE expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas MIN_MAX expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '&' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '|' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas '^' expr_no_commas
		{ $$ = build_x_binary_op ($2, $1, $3); }
	| expr_no_commas ANDAND expr_no_commas
		{ $$ = build_x_binary_op (TRUTH_ANDIF_EXPR, $1, $3); }
	| expr_no_commas OROR expr_no_commas
		{ $$ = build_x_binary_op (TRUTH_ORIF_EXPR, $1, $3); }
	| expr_no_commas '?' xexpr ':' expr_no_commas
		{ $$ = build_x_conditional_expr ($1, $3, $5); }
	| expr_no_commas '=' expr_no_commas
		{ $$ = build_modify_expr ($1, NOP_EXPR, $3); }
	| expr_no_commas ASSIGN expr_no_commas
		{ register tree rval;
		  if (rval = build_opfncall (MODIFY_EXPR, $1, $3, $2))
		    $$ = rval;
		  else
		    $$ = build_modify_expr ($1, $2, $3); }
	;

primary:
	IDENTIFIER
		{ $$ = lastiddecl;
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  if (!$$)
		    {
		      if (yychar == '(')
			{
			  $$ = implicitly_declare ($1);
			}
		      else
			{
			  if (IDENTIFIER_GLOBAL_VALUE ($1) != error_mark_node)
			    error ("undeclared variable `%s' (first use here)",
				   IDENTIFIER_POINTER ($1));
			  $$ = error_mark_node;
			  /* Prevent repeated error messages.  */
			  IDENTIFIER_GLOBAL_VALUE ($1) = error_mark_node;
			}
		    }
		  if (TREE_CODE ($$) == CONST_DECL)
		    $$ = DECL_INITIAL ($$);
		  else $$ = hack_identifier ($$, $1, yychar);
		}
	| operator_name
		{ $$ = hack_operator ($1); }
	| CONSTANT
	| string
		{ $$ = combine_strings ($1); }
	| '(' expr ')'
		{ $$ = $2; }
	| '(' error ')'
		{ $$ = error_mark_node; }
	| '('
		{ if (current_function_decl == 0)
		    {
		      error ("braced-group within expression allowed only inside a function");
		      YYFAIL;
		    }
		  expand_start_stmt_expr (); }
	  compstmt ')'
		{ if (pedantic)
		    warning ("ANSI C forbids braced-groups within expressions");
		  $$ = get_last_expr ();
		  expand_end_stmt_expr (); }
	| primary '(' exprlist ')'
		{ $$ = build_x_function_call ($1, $3, current_class_decl); }
	| primary '[' expr ']'
		{
		  tree type = TREE_TYPE ($1);
		  if (TREE_CODE (type) == REFERENCE_TYPE)
		    type = TREE_TYPE (type);
		  if (TREE_HAS_ARRAY_REF_OVERLOADED (TYPE_MAIN_VARIANT (type)))
		    $$ = build_opfncall (ARRAY_REF, $1, $3);
		  else
		    $$ = build_array_ref ($1, $3);
		}
	| object identifier_or_opname  %prec UNARY
		{ $$ = build_component_ref ($1, $2, 1); }
	| object scoped_declarator identifier_or_opname %prec UNARY
		{
		  /* This is a strange way of casting the object to
		     a parent type.  */
		  if (is_aggr_typedef_or_else ($2))
		    {
		      tree type = TREE_TYPE (TREE_TYPE ($2));

		      if (check_base_type (type, TREE_TYPE ($1)))
			{
			  tree decl;

			  $1 = copy_node ($1);
			  TREE_TYPE ($1) = type;
			  $$ = build_component_ref ($1, $3, 1);
			}
		      else
			{
			  error ("type `%s' is not derived from type `%s'",
				 TYPE_NAME_STRING (TREE_TYPE ($1)),
				 TYPE_NAME_STRING (type));
			  $$ = error_mark_node;
			}
		    }
		  else $$ = error_mark_node;
		}
	| object '*' primary  %prec UNARY
		{ $$ = build_m_component_ref ($1, $3); }
	| primary PLUSPLUS
		{ $$ = build_x_unary_op (POSTINCREMENT_EXPR, $1, 0); }
	| primary MINUSMINUS
		{ $$ = build_x_unary_op (POSTDECREMENT_EXPR, $1, 0); }

	/* C++ extensions */
	| THIS
		{ if (current_class_decl)
		    {
#ifdef WARNING_ABOUT_CCD
		      TREE_EVERUSED (current_class_decl) = 1;
#endif
		      $$ = current_class_decl;
		    }
		  else
		    {
		      error ("request for `this' not in a class, struct, or union");
		      $$ = error_mark_node;
		    }
		}
	| dummy_decl TYPE_QUAL '(' exprlist ')'
		{
		  tree type;
		  tree id = $2;
		  int i;

		  /* This is a C cast in C++'s `functional' notation.  */
		  if ($4 == error_mark_node)
		    {
		      $$ = error_mark_node;
		      break;
		    }
		  if ($4 == NULL_TREE)
		    {
		      error ("cannot cast null list to type `%s'",
			     IDENTIFIER_POINTER (TYPE_NAME ($2)));
		      $$ = error_mark_node;
		      break;
		    }
		  for (i = (int) RID_INT; i < (int) RID_MAX; i++)
		    if (ridpointers[i] == id)
		      {
			switch ((enum rid) i)
			  {
			  case RID_INT:
			    type = integer_type_node;
			    break;
			  case RID_CHAR:
			    type = char_type_node;
			    break;
			  case RID_FLOAT:
			    type = float_type_node;
			    break;
			  case RID_DOUBLE:
			    type = double_type_node;
			    break;
			  case RID_UNSIGNED:
			    type = unsigned_type_node;
			    break;
			  case RID_SHORT:
			    type = short_integer_type_node;
			    break;
			  case RID_LONG:
			    type = long_integer_type_node;
			    break;
			  case RID_SIGNED:
			    type = integer_type_node;
			    break;
			  default:
			    error ("invalid cast type `%s'",
				   IDENTIFIER_POINTER (id));
			    type = error_mark_node;
			  };
			break;
		      }
		  if (type == error_mark_node)
		    $$ = error_mark_node;
		  else
		    $$ = build_c_cast (type, build_compound_expr ($4));
		}
	| x_typespec '(' exprlist ')'
		{
		  /* This is either a call to a constructor,
		     or a C cast in C++'s `functional' notation.  */
		  tree tem, type;

		  if ($3 == error_mark_node)
		    {
		      $$ = error_mark_node;
		      break;
		    }
		  if (TREE_CODE ($1) == IDENTIFIER_NODE)
		    {
		      if (! TREE_TYPE ($1))
			{
			  type = lookup_name ($1);
			  if (!type || TREE_CODE (type) != TYPE_DECL)
			    {
			      error ("`%s' fails to be a typedef or built-in type",
				     IDENTIFIER_POINTER ($1));
			      $$ = error_mark_node;
			      break;
			    }
			  type = TREE_TYPE (type);
			}
		      else if (! is_aggr_typedef_or_else ($1))
			{
			  $$ = error_mark_node;
			  break;
			}
		      else
			type = TREE_TYPE (TREE_TYPE ($1));
		    }
		  else type = $1;
		  if (! IS_AGGR_TYPE (type))
		    {
		      /* this must build a C cast */
		      if ($3 == NULL_TREE)
			{
			  error ("cannot cast null list to type `%s'",
				 IDENTIFIER_POINTER ($1));
			  $$ = error_mark_node;
			  break;
			}
		      $$ = build_c_cast (type, build_compound_expr ($3));
		    }
		  else
		    {
		      /* Call to a consructor.  If this expression
			 is actually used, for example,

			 	return X (arg1, arg2, ...);

			 then the slot being initialized will be filled in.  */

		      $$ = build_classfn_ref (NULL_TREE, $1, $3, 1, 1);
		      if ($$ != error_mark_node)
			{
			  $$ = build (INDIRECT_REF, type, $$);
			  TREE_HAS_CONSTRUCTOR ($$) = 1;
			}
		    }
		}
	| SCOPE IDENTIFIER
		{
		  $$ = IDENTIFIER_GLOBAL_VALUE ($2);
		  if (yychar == YYEMPTY)
		    yychar = YYLEX;
		  if (! $$)
		    {
		      if (yychar == '(')
			{
			  $$ = implicitly_declare ($2);
			}
		      else
			{
			  if (IDENTIFIER_GLOBAL_VALUE ($2) != error_mark_node)
			    error ("undeclared variable `%s' (first use here)",
				   IDENTIFIER_POINTER ($2));
			  $$ = error_mark_node;
			  /* Prevent repeated error messages.  */
			  IDENTIFIER_GLOBAL_VALUE ($2) = error_mark_node;
			}
		    }
		  else if (TREE_CODE ($$) == CONST_DECL)
		    $$ = DECL_INITIAL ($$);
		}
	| scoped_declarator identifier_or_opname  %prec HYPERUNARY
		{ $$ = build_member_ref ($1, $2, 0); }
	| scoped_declarator identifier_or_opname '(' exprlist ')'
		{ $$ = build_member_call ($1, $2, $4, 0); }
	| scoped_declarator '~' identifier
		{ $$ = build_member_ref ($1, $3, 1); }
	| scoped_declarator '~' identifier '(' exprlist ')'
		{ $$ = build_member_call ($1, $3, $5, 1); }

	| object identifier_or_opname '(' exprlist ')'
		{ $$ = build_classfn_ref ($1, $2, $4, 1, 2); }
	| object scoped_declarator identifier_or_opname '(' exprlist ')'
		{
		  /* This is how virtual function calls are avoided.  */
		  if (is_aggr_typedef_or_else ($2))
		    {
		      tree type = TREE_TYPE (TREE_TYPE ($2));

		      if (check_base_type (type, TREE_TYPE ($1)))
			{
			  tree decl;
			  if (TREE_CODE ($1) == INDIRECT_REF)
			    {
			      decl = copy_node (TREE_OPERAND ($1, 0));
			      TREE_TYPE (decl) = TYPE_POINTER_TO (type);
			    }
			  else
			    {
			      decl = build (ADDR_EXPR, TYPE_POINTER_TO (type), $1);
			    }
			  $$ = build_classfn_ref (decl, $3, $5, 1, 3);
			}
		      else
			{
			  error ("type `%s' is not derived from type `%s'",
				 TYPE_NAME_STRING (TREE_TYPE ($1)),
				 TYPE_NAME_STRING (type));
			  $$ = error_mark_node;
			}
		    }
		  else $$ = error_mark_node;
		}

	| new typename %prec '='
		{ $$ = build_new ($1, $2, NULL_TREE); }
	| new x_typespec '(' exprlist ')'
		{ $$ = build_new ($1, $2, $4); }
	| new typename '=' init %prec '='
		{ $$ = build_new ($1, $2, $4); }
	| new '(' typename ')'
		{ $$ = build_new ($1, $3, NULL_TREE); }
	;

new:	  NEW
		{ $$ = NULL_TREE; }
	| NEW '{' nonnull_exprlist '}'
		{ $$ = $3; }
	;

/* Produces a STRING_CST with perhaps more STRING_CSTs chained onto it.  */
string:
	  STRING
	| string STRING
		{ $$ = chainon ($1, $2); }
	;

xdecls:
	  /* empty */
		{
		do_xdecls:
		  store_parm_decls ();
		  setup_vtbl_ptr ();
		}
	| decls
		{ warning ("Old style parameter specification frowned upon");
		  goto do_xdecls; }
	;

object:	  primary '.'
		{
		  if ($1 == error_mark_node)
		    $$ = error_mark_node;
		  else
		    {
		      tree type = TREE_TYPE ($1);

		      if (IS_AGGR_TYPE (type)
			  || (TREE_CODE (type) == REFERENCE_TYPE
			      && IS_AGGR_TYPE (TREE_TYPE (type))))
			$$ = $1;
		      else
			{
			  error ("object in '.' expression is not of aggregate type");
			  $$ = error_mark_node;
			}
		    }
		}
	| primary POINTSAT
		{
		  $$ = build_x_arrow ($1);
		}
	;

decls:
	  decl
	| errstmt
	| decls decl
	| decl errstmt
	;

decl:
	  typed_declspecs initdecls ';'
		{ resume_momentary ($2); }
	| declmods notype_initdecls ';'
		{ resume_momentary ($2); }
	| typed_declspecs ';'
		{ shadow_tag ($1); }
	| declmods ';'
		{ warning ("empty declaration"); }
	;

/* Any kind of declarator (thus, all declarators allowed
   after an explicit typespec).  */

declarator:
	  after_type_declarator
	| notype_declarator
	;

/* Declspecs which contain at least one type specifier or typedef name.
   (Just `const' or `volatile' is not enough.)
   A typedef'd name following these is taken as a name to be declared.  */

typed_declspecs:
	  x_typespec
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| declmods typespec
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	| x_typespec reserved_declspecs
		{ $$ = tree_cons (NULL_TREE, $1, $2); }
	| declmods typespec reserved_declspecs
		{ $$ = tree_cons (NULL_TREE, $2, chainon ($3, $1)); }
	;

reserved_declspecs:  /* empty
		{ $$ = NULL_TREE; } */
	  typespecqual_reserved
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| SCSPEC
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| reserved_declspecs typespecqual_reserved
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	| reserved_declspecs SCSPEC
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	;

/* List of just storage classes and type modifiers.
   A declaration can start with just this, but then it cannot be used
   to redeclare a typedef-name.  */

declmods:
	  dummy_decl TYPE_QUAL
		{ $$ = build_tree_list (NULL_TREE, $2); }
	| dummy_decl SCSPEC
		{ $$ = build_tree_list (NULL_TREE, $2); }
	| declmods TYPE_QUAL
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	| declmods SCSPEC
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	;


/* Used instead of declspecs where storage classes are not allowed
   (that is, for typenames and structure components).

   C++ can takes storage classes for structure components.
   Don't accept a typedef-name if anything but a modifier precedes it.  */

typed_typespecs:
	  x_typespec
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| nonempty_type_quals typespec
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	| x_typespec reserved_typespecquals
		{ $$ = tree_cons (NULL_TREE, $1, $2); }
	| nonempty_type_quals typespec reserved_typespecquals
		{ $$ = tree_cons (NULL_TREE, $2, chainon ($3, $1)); }
	;

x_typed_declspecs:
	  dummy_decl SCSPEC
		{ $$ = build_tree_list (NULL_TREE, $2); }
	| dummy_decl SCSPEC typed_declspecs
		{ $$ = tree_cons (NULL_TREE, $2, $3); }
	;

reserved_typespecquals:
	  typespecqual_reserved
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| reserved_typespecquals typespecqual_reserved
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	;

/* A typespec (but not a type qualifier).
   Once we have seen one of these in a declaration,
   if a typedef name appears then it is being redeclared.  */

typespec: TYPESPEC
	| structsp
	| TYPENAME
	| TYPEOF '(' expr ')'
		{ $$ = TREE_TYPE ($3);
		  if (pedantic)
		    warning ("ANSI C forbids `typeof'") }
	| TYPEOF '(' typename ')'
		{ $$ = groktypename ($3);
		  if (pedantic)
		    warning ("ANSI C forbids `typeof'") }
	;

/* A typespec that is a reserved word, or a type qualifier.  */

typespecqual_reserved: TYPESPEC
	| TYPE_QUAL
	| structsp
	;

x_typespec:
	  dummy_decl TYPESPEC
		{ $$ = $2; }
	| dummy_decl structsp
		{ $$ = $2; }
	| dummy_decl TYPENAME
		{ $$ = $2; }
	| dummy_decl TYPEOF '(' expr ')'
		{ $$ = TREE_TYPE ($4);
		  if (pedantic)
		    warning ("ANSI C forbids `typeof'") }
	| dummy_decl TYPEOF '(' typename ')'
		{ $$ = groktypename ($4);
		  if (pedantic)
		    warning ("ANSI C forbids `typeof'") }
	;

initdecls:
	  initdcl0
	| initdecls ',' initdcl
	;

notype_initdecls:
	  notype_initdcl0
	| notype_initdecls ',' initdcl
	;

maybeasm:
	  /* empty */
		{ $$ = NULL_TREE; }
	| ASM '(' string ')'
		{ if (TREE_CHAIN ($3)) $3 = combine_strings ($3);
		  $$ = $3;
		  if (pedantic)
		    warning ("ANSI C forbids use of `asm' keyword");
		}
	;

initdcl0:
	  declarator maybeasm '='
		{ current_declspecs = $<ttype>0;
		  $<itype>3 = suspend_momentary ();
		  $<ttype>$ = start_decl ($1, current_declspecs, 1); }
	  init
/* Note how the declaration of the variable is in effect while its init is parsed! */
		{ finish_decl ($<ttype>4, $5, $2);
		  $$ = $<itype>3; }
	| declarator maybeasm
		{ tree d;
		  current_declspecs = $<ttype>0;
		  $$ = suspend_momentary ();
		  d = start_decl ($1, current_declspecs, 0);
		  finish_decl (d, NULL_TREE, $2);
		}
	;

initdcl:
	  declarator maybeasm '='
		{ $<ttype>$ = start_decl ($1, current_declspecs, 1); }
	  init
/* Note how the declaration of the variable is in effect while its init is parsed! */
		{ finish_decl ($<ttype>4, $5, $2); }
	| declarator maybeasm
		{ tree d = start_decl ($1, current_declspecs, 0);
		  finish_decl (d, NULL_TREE, $2);
		}
	;

notype_initdcl0:
	  notype_declarator maybeasm '='
		{ current_declspecs = $<ttype>0;
		  $<itype>3 = suspend_momentary ();
		  $<ttype>$ = start_decl ($1, current_declspecs, 1); }
	  init
/* Note how the declaration of the variable is in effect while its init is parsed! */
		{ finish_decl ($<ttype>4, $5, $2);
		  $$ = $<itype>3; }
	| notype_declarator maybeasm
		{ tree d;
		  current_declspecs = $<ttype>0;
		  $$ = suspend_momentary ();
		  d = start_decl ($1, current_declspecs, 0);
		  finish_decl (d, NULL_TREE, $2);
		}
	;

notype_initdcl:
	  notype_declarator maybeasm '='
		{ $<ttype>$ = start_decl ($1, current_declspecs, 1); }
	  init
/* Note how the declaration of the variable is in effect while its init is parsed! */
		{ finish_decl ($<ttype>4, $5, $2);
		  expand_decl ($<ttype>4, 1); }
	| notype_declarator maybeasm
		{ tree d = start_decl ($1, current_declspecs, 0);
		  finish_decl (d, NULL_TREE, $2);
		}
	;

init:
	  expr_no_commas %prec '='
	| '{' initlist '}'
		{ $$ = build_nt (CONSTRUCTOR, NULL_TREE, nreverse ($2)); }
	| '{' initlist ',' '}'
		{ $$ = build_nt (CONSTRUCTOR, NULL_TREE, nreverse ($2)); }
	| error
		{ $$ = NULL_TREE; }
	;

/* This chain is built in reverse order,
   and put in forward order where initlist is used.  */
initlist:
	  init
		{ $$ = build_tree_list (NULL_TREE, $1); }
	| initlist ',' init
		{ $$ = tree_cons (NULL_TREE, $3, $1); }
	;

structsp:
	  ENUM identifier '{'
		{ $<itype>3 = suspend_momentary ();
		  $$ = start_enum ($2); }
	  enumlist maybecomma '}'
		{ $$ = finish_enum ($<ttype>4, $5);
		  resume_momentary ($<itype>1); }
	| ENUM '{'
		{ $<itype>2 = suspend_momentary ();
		  $$ = start_enum (make_anon_name ()); }
	  enumlist maybecomma '}'
		{ $$ = finish_enum ($<ttype>3, $4);
		  resume_momentary ($<itype>1); }
	| ENUM identifier
		{ $$ = xref_tag (enum_type_node, $2, NULL_TREE); }

	/* C++ extensions, merged with C to avoid shift/reduce conflicts */
	| class_head LC opt.component_decl_list '}'
		{ $$ = finish_struct ($1, $3, 0);
		  resume_momentary ($2); }
	| class_head LC opt.component_decl_list '}' ';'
		{ extern FILE *finput;
		  $$ = finish_struct ($1, $3, 1);
		  resume_momentary ($2);
		  ungetc (';', finput); }
	| class_head
		{ $$ = $1; }
	;

maybecomma:
	  /* empty */
	| ','
	;

class_head:
	  AGGR  %prec EMPTY
		{ $$ = xref_tag ($1, make_anon_name (), NULL_TREE); }
	| AGGR identifier  %prec EMPTY
		{ $$ = xref_tag ($1, $2, NULL_TREE); }
	| AGGR IDENTIFIER ':' base_class_list %prec EMPTY
		{ $$ = xref_tag ($1, $2, $4); }
	| AGGR TYPENAME_COLON base_class_list %prec EMPTY
		{ $$ = xref_tag ($1, $2, $3); }
	;

base_class_list:
	  identifier
		{ if (! is_aggr_typedef_or_else ($1))
		    $$ = NULL_TREE;
		  else $$ = build_tree_list (visibility_default, $1); }
	| base_class_visibility_list identifier
		{ if (! is_aggr_typedef_or_else ($2))
		    $$ = NULL_TREE;
		  else $$ = build_tree_list ($1, $2); }
	| base_class_list ',' identifier
		{ if (! is_aggr_typedef_or_else ($3))
		    $$ = NULL_TREE;
		  else $$ = chainon ($1, build_tree_list (visibility_default, $3)); }
	| base_class_list ',' base_class_visibility_list identifier
		{ if (! is_aggr_typedef_or_else ($4))
		    $$ = NULL_TREE;
		  else $$ = chainon ($1, build_tree_list ($3, $4)); }
	;

base_class_visibility_list:
	  PUBLIC
		{ $$ = (tree)visibility_public; }
	| PRIVATE
		{ $$ = (tree)visibility_private; }
	| SCSPEC
		{ if ($1 != ridpointers[(int)RID_VIRTUAL])
		    sorry ("non-virtual visibility");
		  $$ = (tree)visibility_default_virtual; }
	| base_class_visibility_list PUBLIC
		{ if ($1 == (tree)visibility_private)
		    error ("base class cannot be public and private");
		  else if ($1 == (tree)visibility_default_virtual)
		    $$ = (tree)visibility_public_virtual; }
	| base_class_visibility_list PRIVATE
		{ if ($1 == (tree)visibility_public)
		    error ("base class cannot be private and public");
		  else if ($1 == (tree)visibility_default_virtual)
		    $$ = (tree)visibility_private_virtual; }
	| base_class_visibility_list SCSPEC
		{ if ($2 != ridpointers[(int)RID_VIRTUAL])
		    sorry ("non-virtual visibility");
		  if ($1 == (tree)visibility_public)
		    $$ = (tree)visibility_public_virtual;
		  else if ($1 == (tree)visibility_private)
		    $$ = (tree)visibility_private_virtual; }
	;

LC: '{'
		{ pushclass ($<ttype>0, 0);
		  $$ = suspend_momentary (); }

opt.component_decl_list:
	/* empty */
		{ $$ = NULL_TREE; }
	| component_decl_list
		{ $$ = build_tree_list ((tree)visibility_default, $1); }
	| opt.component_decl_list PUBLIC ':' component_decl_list
		{ $$ = chainon ($1, build_tree_list ((tree)visibility_public, $4)); }
	| opt.component_decl_list PRIVATE ':' component_decl_list
		{ $$ = chainon ($1, build_tree_list ((tree)visibility_private, $4)); }
	| opt.component_decl_list PROTECTED ':' component_decl_list
		{ $$ = chainon ($1, build_tree_list ((tree)visibility_protected, $4)); }
	| opt.component_decl_list PUBLIC ':'
	| opt.component_decl_list PRIVATE ':'
	| opt.component_decl_list PROTECTED ':'
	;

component_decl_list:
	  component_decl
	| component_decl_list component_decl
		{ $$ = $2 ? chainon ($1, $2) : $1; }
	| component_decl_list ';'
		{ if (pedantic) 
		    warning ("extra semicolon in struct or union specified"); }
	;

component_decl:
	  typed_declspecs components ';'
		{
		do_components:
		  if ($2 == void_type_node)
		    {
		      /* We just got some friends.
			 They have been recorded elsewhere.  */
		      $$ = NULL_TREE;
		    }
		  else if ($2 == NULL_TREE)
		    {
		      tree t = groktypename (build_tree_list ($1, NULL_TREE));
		      if (t == NULL_TREE)
			{
			  yylineerror (@1.first_line,
				       "error in component specification");
			  $$ = NULL_TREE;
			}
		      else if (TREE_CODE (t) == UNION_TYPE)
			{
			  tree name = DECL_NAME (TYPE_NAME (t));
			  /* handle anonymous unions */
			  if (TYPE_FN_FIELDS (t))
			    sorry ("methods in anonymous unions");
			  $$ = TYPE_FIELDS (t);
			  TYPE_FIELDS (t) = NULL_TREE;
			  TYPE_NAME (t) = NULL_TREE;
			  t = $$;
			  for (; t; t = TREE_CHAIN (t))
			    TREE_ANON_UNION_ELEM (t) = 1;
			  IDENTIFIER_GLOBAL_VALUE (name) = NULL_TREE;
			  TREE_TYPE (name) = NULL_TREE;
			}
		      else
			$$ = NULL_TREE;
		    }
		  else
		    $$ = $2;
		}
	| typed_declspecs '(' parmlist ')' ';'
		{
		do_type_component:
		  $$ = groktypefield ($1, $3);
		}
	| declmods components ';'
		{ goto do_components; }
	| declmods '(' parmlist ')' ';'
		{ goto do_type_component; }
	| ':' expr_no_commas ';'
		{ $$ = grokfield (NULL_TREE, NULL_TREE, $2, NULL_TREE, NULL_TREE); }
	| error
		{ $$ = NULL_TREE; }

	/* C++: handle constructors, destructors and inline functions */
	/* note that INLINE is like a TYPESPEC */
	| fn.def2 ':' /* base_init compstmt */
		{ $$ = $1; finish_method ($1); }
	| fn.def2 '{' /* xdecls compstmt */
		{ $$ = $1; finish_method ($1); }
	| dummy_decl notype_declarator ';'
		{ $$ = grokfield ($2, NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE); }
/*
	| dummy_decl TYPENAME '(' parmlist ')' ';'
		{
		  tree decl = build_nt (CALL_EXPR, $2, $4);
		  $$ = grokfield (decl, NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE);
		}
*/
	;

components:
	  /* empty: possibly anonymous */
		{ $$ = NULL_TREE; }
	| component_declarator0
	| components ',' component_declarator
		{
		  /* In this context, void_type_node encodes
		     friends.  They have been recorded elsewhere.  */
		  if ($1 == void_type_node)
		    $$ = $3;
		  else
		    $$ = chainon ($1, $3);
		}
	;

component_declarator0:
	  declarator maybeasm opt.init
		{ current_declspecs = $<ttype>0;
		  $$ = grokfield ($1, current_declspecs, NULL_TREE, $3, $2); }
	| IDENTIFIER ':' expr_no_commas
		{ current_declspecs = $<ttype>0;
		  $$ = grokfield ($1, current_declspecs, $3, NULL_TREE, NULL_TREE); }
	| TYPENAME_COLON expr_no_commas
		{ current_declspecs = $<ttype>0;
		  $$ = grokfield ($1, current_declspecs, $2, NULL_TREE, NULL_TREE); }
	;

component_declarator:
	  declarator maybeasm opt.init
		{ $$ = grokfield ($1, current_declspecs, NULL_TREE, $3, $2); }
	| IDENTIFIER ':' expr_no_commas
		{ $$ = grokfield ($1, current_declspecs, $3, NULL_TREE, NULL_TREE); }
	| TYPENAME_COLON expr_no_commas
		{ $$ = grokfield ($1, current_declspecs, $2, NULL_TREE, NULL_TREE); }
	| ':' expr_no_commas
		{ $$ = grokfield (NULL_TREE, NULL_TREE, $2, NULL_TREE, NULL_TREE); }
	;

/* We chain the enumerators in reverse order.
   Because of the way enums are built, the order is
   insignificant.  Take advantage of this fact.  */

enumlist:
	  enumerator
	| enumlist ',' enumerator
		{ TREE_CHAIN ($3) = $1; $$ = $3; }
	;


enumerator:
	  identifier
		{ $$ = build_enumerator ($1, NULL_TREE); }
	| identifier '=' expr_no_commas
		{ $$ = build_enumerator ($1, $3); }
	;

typename:
	  typed_typespecs absdcl
		{ $$ = build_tree_list ($1, $2); }
	| nonempty_type_quals absdcl
		{ $$ = build_tree_list ($1, $2); }
	;

absdcl:   /* an abstract declarator */
	/* empty */ %prec EMPTY
		{ $$ = NULL_TREE; }
	| absdcl1  %prec EMPTY
	;

nonempty_type_quals:
	  dummy_decl TYPE_QUAL
		{ $$ = build_tree_list (NULL_TREE, $2); }
	| nonempty_type_quals TYPE_QUAL
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	;

type_quals:
	  /* empty */
		{ $$ = NULL_TREE; }
	| type_quals TYPE_QUAL
		{ $$ = tree_cons (NULL_TREE, $2, $1); }
	;

/* These rules must follow the rules for function declarations
   and component declarations.  That way, longer rules are prefered.  */

/* A declarator that is allowed only after an explicit typespec.  */
/* may all be followed by prec '.' */
after_type_declarator:
	  after_type_declarator '(' nonnull_exprlist ')'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| after_type_declarator '(' parmlist ')'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| after_type_declarator '(' error ')'
		{ $$ = build_nt (CALL_EXPR, $1, NULL_TREE); }
	| after_type_declarator '[' expr ']'
		{ $$ = build_nt (ARRAY_REF, $1, $3); }
	| after_type_declarator '[' ']'
		{ $$ = build_nt (ARRAY_REF, $1, NULL_TREE); }
	| '(' dummy_decl after_type_declarator_no_typename ')'
		{ $$ = $3; }
	| '(' '*' type_quals after_type_declarator ')'
		{ $$ = make_pointer_declarator ($3, $4); }
	| '(' '&' type_quals after_type_declarator ')'
		{ $$ = make_reference_declarator ($3, $4); }
	| '*' type_quals after_type_declarator  %prec UNARY
		{ $$ = make_pointer_declarator ($2, $3); }
	| '&' type_quals after_type_declarator  %prec UNARY
		{ $$ = make_reference_declarator ($2, $3); }
	| TYPENAME
	;

after_type_declarator_no_typename:
	  after_type_declarator_no_typename '(' nonnull_exprlist ')'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| after_type_declarator_no_typename '(' parmlist ')'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| after_type_declarator_no_typename '(' error ')'
		{ $$ = build_nt (CALL_EXPR, $1, NULL_TREE); }
	| after_type_declarator_no_typename '[' expr ']'
		{ $$ = build_nt (ARRAY_REF, $1, $3); }
	| after_type_declarator_no_typename '[' ']'
		{ $$ = build_nt (ARRAY_REF, $1, NULL_TREE); }
	| '(' dummy_decl after_type_declarator_no_typename ')'
		{ $$ = $3; }
	| '*' type_quals after_type_declarator  %prec UNARY
		{ $$ = make_pointer_declarator ($2, $3); }
	| '&' type_quals after_type_declarator  %prec UNARY
		{ $$ = make_reference_declarator ($2, $3); }
	;

/* A declarator allowed whether or not there has been
   an explicit typespec.  These cannot redeclare a typedef-name.  */

notype_declarator:
	  notype_declarator '(' nonnull_exprlist ')'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| notype_declarator '(' parmlist ')'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| notype_declarator '(' error ')'
		{ $$ = build_nt (CALL_EXPR, $1, NULL_TREE); }
	| '(' notype_declarator ')'
		{ $$ = $2; }
	| '*' type_quals notype_declarator  %prec UNARY
		{ $$ = make_pointer_declarator ($2, $3); }
	| '&' type_quals notype_declarator  %prec UNARY
		{ $$ = make_reference_declarator ($2, $3); }
	| notype_declarator '[' expr ']'
		{ $$ = build_nt (ARRAY_REF, $1, $3); }
	| notype_declarator '[' ']'
		{ $$ = build_nt (ARRAY_REF, $1, NULL_TREE); }
	| IDENTIFIER

	/* C++ extensions.  */
	| operator_name
	| '~' TYPENAME
		{ $$ = build_nt (BIT_NOT_EXPR, $2); }
	| '~' IDENTIFIER
		{ $$ = build_nt (BIT_NOT_EXPR, $2); }
	| '(' parmlist ')' TYPENAME
		{
		  if ($2 && (TREE_CHAIN ($2) || TREE_VALUE ($2) != void_type_node))
		    error ("junk in wrapper declaration");
		  $$ = build_nt (WRAPPER_EXPR, $4);
		}
	| '(' parmlist ')' IDENTIFIER
		{
		  if ($2 && (TREE_CHAIN ($2) || TREE_VALUE ($2) != void_type_node))
		    error ("junk in wrapper declaration");
		  $$ = build_nt (WRAPPER_EXPR, $4);
		}
	| '~' '(' ')' TYPENAME
		{ $$ = build_nt (ANTI_WRAPPER_EXPR, $4); }
	| '~' '(' ')' IDENTIFIER
		{ $$ = build_nt (ANTI_WRAPPER_EXPR, $4); }
	| scoped_declarator type_quals notype_declarator  %prec '('
		{ $$ = build_nt (SCOPE_REF, $1, $3); }
	| scoped_declarator type_quals TYPENAME
		{ $$ = build_nt (SCOPE_REF, $1, $3); }
	;

scoped_declarator:
	  TYPENAME_SCOPE
	| IDENTIFIER SCOPE
	;

absdcl1:  /* a nonempty abstract declarator */
	  '(' absdcl1 ')'
		{ $$ = $2; }
	  /* `(typedef)1' is `int'.  */
	| '*' type_quals absdcl1  %prec EMPTY
		{ $$ = make_pointer_declarator ($2, $3); }
	| '*' type_quals  %prec EMPTY
		{ $$ = make_pointer_declarator ($2, NULL_TREE); }
	| '&' type_quals absdcl1 %prec EMPTY
		{ $$ = make_reference_declarator ($2, $3); }
	| '&' type_quals %prec EMPTY
		{ $$ = make_reference_declarator ($2, NULL_TREE); }
	| absdcl1 '(' parmlist ')'  %prec '.'
		{ $$ = build_nt (CALL_EXPR, $1, $3); }
	| absdcl1 '[' expr ']'  %prec '.'
		{ $$ = build_nt (ARRAY_REF, $1, $3); }
	| absdcl1 '[' ']'  %prec '.'
		{ $$ = build_nt (ARRAY_REF, $1, NULL_TREE); }
	| '(' parmlist ')'  %prec '.'
		{ $$ = build_nt (CALL_EXPR, NULL_TREE, $2); }
	| '[' expr ']'  %prec '.'
		{ $$ = build_nt (ARRAY_REF, NULL_TREE, $2); }
	| '[' ']'  %prec '.'
		{ $$ = build_nt (ARRAY_REF, NULL_TREE, NULL_TREE); }
	| scoped_declarator type_quals absdcl1  %prec EMPTY
		{ $$ = build_nt (SCOPE_REF, $1, $3); }
	| scoped_declarator type_quals %prec EMPTY
		{ $$ = build_nt (SCOPE_REF, $1, 0); }
	;

/* at least one statement, the first of which parses without error.  */
/* stmts is used only after decls, so an invalid first statement
   is actually regarded as an invalid decl and part of the decls.  */

stmts:
	  stmt
	| stmts stmt
	;

xstmts:
	/* empty */
	| stmts
	;

errstmt:  error ';'
	;

/* build the LET_STMT node before parsing its contents,
  so that any LET_STMTs within the context can have their display pointers
  set up to point at this one.  */

.pushlevel:  /* empty */
		{ pushlevel (0);
		  clear_last_expr ();
		  push_momentary ();
		  expand_start_bindings (0);
		}
	;

compstmt: '{' '}'
	| '{' .pushlevel stmts '}'
		{ expand_end_bindings (1, 1);
		  poplevel (1, 1, 0);
		  pop_momentary (); }
	| '{' .pushlevel error '}'
		{ expand_end_bindings (1, 0);
		  poplevel (0, 0, 0);
		  pop_momentary (); }
	;

simple_if:
	  IF '(' expr ')'
		{ emit_note (input_filename, @1.first_line);
		  expand_start_cond (truthvalue_conversion ($3), 0); }
	  stmt
	;

stmt:
	  compstmt
		{ finish_stmt (); }
	| decl
		{ finish_stmt (); }
	| expr ';'
		{ emit_note (input_filename, @1.first_line);
		  expand_expr_stmt ($1);
		  clear_momentary ();
		  finish_stmt (); }
	| simple_if ELSE
		{ expand_start_else (); }
	  stmt
		{ expand_end_else ();
		  finish_stmt (); }
	| simple_if %prec EMPTY
		{ expand_end_cond ();
		  finish_stmt (); }
	| WHILE
		{ emit_note (input_filename, @1.first_line);
		  expand_start_loop (1); }
	  '(' expr ')'
		{ expand_exit_loop_if_false (truthvalue_conversion ($4)); }
	  stmt
		{ expand_end_loop ();
		  finish_stmt (); }
	| DO
		{ emit_note (input_filename, @1.first_line);
		  expand_start_loop_continue_elsewhere (1); }
	  stmt WHILE
		{ expand_loop_continue_here (); }
	  '(' expr ')' ';'
		{ emit_note (input_filename, @7.first_line);
		  expand_exit_loop_if_false (truthvalue_conversion ($7));
		  expand_end_loop ();
		  clear_momentary ();
		  finish_stmt (); }
	| forhead.1
		{ emit_note (input_filename, @1.first_line);
		  if ($1) expand_expr_stmt ($1);
		  expand_start_loop_continue_elsewhere (1); }
	  xexpr ';'
		{ emit_note (input_filename, @3.first_line);
		  if ($3) expand_exit_loop_if_false (truthvalue_conversion ($3)); }
	  xexpr ')'
		/* Don't let the tree nodes for $6 be discarded
		   by clear_momentary during the parsing of the next stmt.  */
		{ push_momentary (); }
	  stmt
		{ emit_note (input_filename, @6.first_line);
		  expand_loop_continue_here ();
		  if ($6) expand_expr_stmt ($6);
		  pop_momentary ();
		  expand_end_loop ();
		  finish_stmt (); }
	| forhead.2
		{ emit_note (input_filename, @1.first_line);
		  expand_start_loop_continue_elsewhere (1); }
	  xexpr ';'
		{ emit_note (input_filename, @3.first_line);
		  if ($3) expand_exit_loop_if_false (truthvalue_conversion ($3)); }
	  xexpr ')'
		/* Don't let the tree nodes for $6 be discarded
		   by clear_momentary during the parsing of the next stmt.  */
		{ push_momentary (); }
	  stmt
		{ emit_note (input_filename, @6.first_line);
		  expand_loop_continue_here ();
		  if ($6) expand_expr_stmt ($6);
		  pop_momentary ();
		  expand_end_loop ();
		  if ($1)
		    {
		      register keep = $1 > 0;
		      if (keep) expand_end_bindings (0, keep);
		      poplevel (keep, 1, 0);
		      pop_momentary ();
		    }
		  finish_stmt ();
		}
	| SWITCH '(' expr ')'
		{ emit_note (input_filename, @1.first_line);
		  c_expand_start_case ($3);
		  /* Don't let the tree nodes for $3 be discarded by
		     clear_momentary during the parsing of the next stmt.  */
		  push_momentary (); }
	  stmt
		{ expand_end_case ();
		  pop_momentary ();
		  finish_stmt (); }
	| CASE expr ':'
		{ register tree value = fold (maybe_convert_decl_to_const ($2));
		  register tree label
		    = build_decl (LABEL_DECL, NULL_TREE, NULL_TREE);

		  if (TREE_CODE (value) != INTEGER_CST
		      && value != error_mark_node)
		    {
		      error ("case label does not reduce to an integer constant");
		      value = error_mark_node;
		    }
		  else
		    /* Promote char or short to int.  */
		    value = default_conversion (value);
		  if (value != error_mark_node)
		    pushcase (value, label);
		}
	  stmt
	| CASE expr RANGE expr ':'
		{ register tree value1 = fold (maybe_convert_decl_to_const ($2));
		  register tree value2 = fold (maybe_convert_decl_to_const ($4));
		  register tree label
		    = build_decl (LABEL_DECL, NULL_TREE, NULL_TREE);

		  if (pedantic)
		    {
		      error ("ANSI C does not allow range expressions in switch statement");
		      value1 = error_mark_node;
		      value2 = error_mark_node;
		    }
		  if (TREE_CODE (value1) != INTEGER_CST
		      && value1 != error_mark_node)
		    {
		      error ("case label does not reduce to an integer constant");
		      value1 = error_mark_node;
		    }
		  if (TREE_CODE (value2) != INTEGER_CST
		      && value2 != error_mark_node)
		    {
		      error ("case label does not reduce to an integer constant");
		      value2 = error_mark_node;
		    }
		  if (value1 != error_mark_node
		      && value2 != error_mark_node)
		    pushcase_range (value1, value2, label);
		}
	  stmt
	| DEFAULT ':'
		{
		  register tree label
		    = build_decl (LABEL_DECL, NULL_TREE, NULL_TREE);
		  pushcase (NULL_TREE, label);
		}
	  stmt
	| BREAK ';'
		{ emit_note (input_filename, @1.first_line);
		  if ( ! expand_exit_something ())
		    error ("break statement not within loop or switch"); }
	| CONTINUE ';'	
		{ emit_note (input_filename, @1.first_line);
		  if (! expand_continue_loop ())
		    error ("continue statement not within a loop"); }
	| RETURN ';'
		{ emit_note (input_filename, @1.first_line);
		  c_expand_return (NULL_TREE); }
	| RETURN expr ';'
		{ emit_note (input_filename, @1.first_line);
		  c_expand_return ($2);
		  finish_stmt ();
		}
	| ASM maybe_type_qual '(' string ')' ';'
		{ if (pedantic)
		    warning ("ANSI C forbids use of `asm' keyword");
		  emit_note (input_filename, @1.first_line);
		  if (TREE_CHAIN ($4)) $4 = combine_strings ($4);
		  expand_asm ($4);
		  finish_stmt ();
		}
	/* This is the case with just output operands.  */
	| ASM maybe_type_qual '(' string ':' asm_operands ')' ';'
		{ if (pedantic)
		    warning ("ANSI C forbids use of `asm' keyword");
		  emit_note (input_filename, @1.first_line);
		  if (TREE_CHAIN ($4)) $4 = combine_strings ($4);
		  c_expand_asm_operands ($4, $6, NULL_TREE,
					 $2 == ridpointers[(int)RID_VOLATILE]);
		  finish_stmt ();
		}
	/* This is the case with input operands as well.  */
	| ASM maybe_type_qual '(' string ':' asm_operands ':' asm_operands ')' ';'
		{ if (pedantic)
		    warning ("ANSI C forbids use of `asm' keyword");
		  emit_note (input_filename, @1.first_line);
		  if (TREE_CHAIN ($4)) $4 = combine_strings ($4);
		  c_expand_asm_operands ($4, $6, $8,
					 $2 == ridpointers[(int)RID_VOLATILE]);
		  finish_stmt ();
		}
	| GOTO identifier ';'
		{ tree decl;
		  emit_note (input_filename, @1.first_line);
		  decl = lookup_label ($2);
		  expand_goto (decl); }
	| IDENTIFIER ':'
		{ tree label = define_label (input_filename, @1.first_line, $1);
		  if (label)
		    expand_label (label); }
	  stmt
		{ finish_stmt (); }
	| TYPENAME_COLON
		{ tree label = define_label (input_filename, @1.first_line, $1);
		  if (label)
		    expand_label (label); }
	  stmt
		{ finish_stmt (); }
	| ';'
		{ finish_stmt (); }
	;

forhead.1:
	  FOR '(' ';'
		{ $$ = NULL_TREE; }
	| FOR '(' expr ';'
		{ $$ = $3; }
	| FOR '(' '{' '}'
		{ $$ = NULL_TREE; }
	;

forhead.2:
	  FOR '(' decl
		{ $$ = 0; }
	| FOR '(' '{' .pushlevel stmts '}'
		{ $$ = 1; }
	| FOR '(' '{' .pushlevel error '}'
		{ $$ = -1; }
	;

maybe_type_qual:
	/* empty */
	| TYPE_QUAL
	;

xexpr:
	/* empty */
		{ $$ = NULL_TREE; }
	| expr
	| error
		{ $$ = NULL_TREE; }
	;

/* These are the operands other than the first string and comma
   in  asm ("addextend %2,%1", "=dm" (x), "0" (y), "g" (*x))  */
asm_operands: asm_operand
	| asm_operands ',' asm_operand
		{ $$ = chainon ($1, $3); }
	;

asm_operand:
	| STRING '(' expr ')'
		{ $$ = build_tree_list ($1, $3); }
	;

/* This is what appears inside the parens in a function declarator.
   Its value is represented in the format that grokdeclarator expects.

   In C++, declaring a function with no parameters
   means that that function takes *no* parameters.  */
parmlist:  /* empty */
		{
		  if (strict_prototype)
		    {
		      $$ = build_tree_list (NULL_TREE, void_type_node);
		      TREE_PARMLIST($$) = 1;
		    }
		  else $$ = NULL_TREE;
		}
	| parms
  		{
		  $$ = chainon ($1, build_tree_list (NULL_TREE,
						     void_type_node));
		  TREE_PARMLIST ($$) = 1;
		}
	| parms ',' ELLIPSIS
		{
		  $$ = $1;
		  TREE_PARMLIST ($$) = 1;
		}
	/* C++ allows an ellipsis without a separating ',' */
	| parms ELLIPSIS
		{
		  $$ = $1;
		  TREE_PARMLIST ($$) = 1;
		}
	| ELLIPSIS
		{
		  warning ("lazy prototyping frowned upon");
		  $$ = NULL_TREE;
		}
	| TYPENAME_ELLIPSIS
		{
		  $$ = $1;
		  TREE_PARMLIST ($$) = 1;
		}
	| parms TYPENAME_ELLIPSIS
		{
		  $$ = $1;
		  TREE_PARMLIST ($$) = 1;
		}
	;

/* A nonempty list of parameter declarations or type names.  */
parms:	
	  parm opt.init
		{ $$ = build_tree_list ($2, $1); }
	| parms ',' parm opt.init
		{ $$ = chainon ($1, build_tree_list ($4, $3)); }
	| parms ',' bad_parm opt.init
		{ $$ = chainon ($1, build_tree_list ($4, $3)); }
	;

/* A single parameter declaration or parameter type name,
   as found in a parmlist.  */
parm:
	  typed_declspecs notype_declarator
		{ $$ = build_tree_list ($1, $2)	; }
	| typed_declspecs absdcl
		{ $$ = build_tree_list ($1, $2); }
	| declmods notype_declarator
		{ $$ = build_tree_list ($1, $2)	; }
	| declmods absdcl
		{ $$ = build_tree_list ($1, $2); }
	;

bad_parm:
	  dummy_decl notype_declarator
		{
		  warning ("type specifier omitted for parameter");
		  $$ = build_tree_list (TREE_PURPOSE (TREE_VALUE ($<ttype>-1)), $2);
		}
	| dummy_decl absdcl
		{
		  warning ("type specifier omitted for parameter");
		  $$ = build_tree_list (TREE_PURPOSE (TREE_VALUE ($<ttype>-1)), $2);
		}
	;

	/* C++ extension: allow for initialization */
opt.init:
	  /* empty */
		{ $$ = NULL_TREE; }
	| '=' init
		{ $$ = $2; }
	;

operator_name:
	  OPERATOR '*'
		{ $$ = build_nt (OP_EXPR, (tree)'*', 0); }
	| OPERATOR '/'
		{ $$ = build_nt (OP_EXPR, 0, TRUNC_DIV_EXPR); }
	| OPERATOR '%'
		{ $$ = build_nt (OP_EXPR, 0, TRUNC_MOD_EXPR); }
	| OPERATOR '+'
		{ $$ = build_nt (OP_EXPR, (tree)'+', 0); }
	| OPERATOR '-'
		{ $$ = build_nt (OP_EXPR, (tree)'-', 0); }
	| OPERATOR '&'
		{ $$ = build_nt (OP_EXPR, (tree)'&', 0); }
	| OPERATOR '|'
		{ $$ = build_nt (OP_EXPR, 0, BIT_IOR_EXPR); }
	| OPERATOR '^'
		{ $$ = build_nt (OP_EXPR, 0, BIT_XOR_EXPR); }
	| OPERATOR '~'
		{ $$ = build_nt (OP_EXPR, 0, BIT_NOT_EXPR); }
	| OPERATOR ARITHCOMPARE
		{ $$ = build_nt (OP_EXPR, 0, $2); }
	| OPERATOR EQCOMPARE
		{ $$ = build_nt (OP_EXPR, 0, $2); }
	| OPERATOR ASSIGN
		{ $$ = build_nt (OP_EXPR, (tree)ASSIGN, $2); }
	| OPERATOR '='
		{
		  $$ = build_nt (OP_EXPR, (tree)ASSIGN, NOP_EXPR);
		  if (current_class_type)
		    {
		      TREE_HAS_ASSIGNMENT (current_class_type) = 1;
		      TREE_GETS_ASSIGNMENT (current_class_type) = 1;
		    }
		}
	| OPERATOR LSHIFT
		{ $$ = build_nt (OP_EXPR, 0, $2); }
	| OPERATOR RSHIFT
		{ $$ = build_nt (OP_EXPR, 0, $2); }
	| OPERATOR PLUSPLUS
		{ $$ = build_nt (OP_EXPR, 0, POSTINCREMENT_EXPR); }
	| OPERATOR MINUSMINUS
		{ $$ = build_nt (OP_EXPR, 0, PREDECREMENT_EXPR); }
	| OPERATOR ANDAND
		{ $$ = build_nt (OP_EXPR, 0, TRUTH_ANDIF_EXPR); }
	| OPERATOR OROR
		{ $$ = build_nt (OP_EXPR, 0, TRUTH_ORIF_EXPR); }
	| OPERATOR '!'
		{ $$ = build_nt (OP_EXPR, 0, TRUTH_NOT_EXPR); }
	| OPERATOR '?' ':'
		{ $$ = build_nt (OP_EXPR, 0, COND_EXPR); }
	| OPERATOR MIN_MAX
		{ $$ = build_nt (OP_EXPR, 0, $2); }
	| OPERATOR POINTSAT  %prec EMPTY
		{ $$ = build_nt (OP_EXPR, 0, COMPONENT_REF); }
	| OPERATOR POINTSAT '(' ')'
		{
		  $$ = build_nt (OP_EXPR, 0, METHOD_REF);
		  if (current_class_type)
		    TREE_HAS_METHOD_CALL_OVERLOADED (current_class_type) = 1;
		}
	| OPERATOR '(' ')'
		{ $$ = build_nt (OP_EXPR, 0, CALL_EXPR);
		  if (current_class_type)
		    TREE_HAS_CALL_OVERLOADED (current_class_type) = 1;
		}
	| OPERATOR '[' ']'
		{ $$ = build_nt (OP_EXPR, 0, ARRAY_REF);
		  if (current_class_type)
		    TREE_HAS_ARRAY_REF_OVERLOADED (current_class_type) = 1;
		}
	| OPERATOR NEW
		{
		  $$ = build_nt (OP_EXPR, 0, NEW_EXPR);
		  if (current_class_type)
		    TREE_GETS_NEW (current_class_type) = 1;
		}
	| OPERATOR DELETE
		{
		  $$ = build_nt (OP_EXPR, 0, DELETE_EXPR);
		  if (current_class_type)
		    TREE_GETS_DELETE (current_class_type) = 1;
		}

	/* These should do `groktypename' and set up TREE_HAS_X_CONVERSION
	   here, rather than doing it in class.c .  */
	| OPERATOR typed_typespecs absdcl
		{ $$ = build_nt (TYPE_EXPR, $2, $3); }
	| OPERATOR error
		{ $$ = NULL_TREE; }
	;

%%
db_yyerror (s, yyps, yychar)
     char *s;
     short *yyps;
     int yychar;
{
  FILE *yyout;
  char buf[1024];
  int st;

  yyerror (s);
  printf ("State is %d, input token number is %d.\n", *yyps, yychar);

  if (*yyps < 1) fatal ("Cannot start from here");
  else if ((yyout = fopen ("/u15/tiemann/g++/parse.output", "r")) == NULL)
    fatal ("Cannot open file parse.output");
  else
    {
      printf ("That is to say,\n\n");
      while (fgets(buf, sizeof (buf)-1, yyout))
	{
	  if (buf[0] != 's') continue;
	  st = atoi (buf+6);
	  if (st != *yyps) continue;
	  printf ("%s", buf);
	  while (fgets (buf, sizeof (buf)-1, yyout))
	    {
	      if (buf[0] == 's') break;
	      printf ("%s", buf);
	    }
	  break;
	}
      printf ("With the token %s\n", yytname[YYTRANSLATE (yychar)]);
      fclose (yyout);
    }
}

void
yyerror ()
{
  /* We can't print string and character constants well
     because the token_buffer contains the result of processing escapes.  */
  extern char *token_buffer;
  /* We can't print string and character constants well
     because the token_buffer contains the result of processing escapes.  */
  if (token_buffer[0] == 0)
    error ("parse error at end of input");
  else if (token_buffer[0] == '"')
    error ("parse error at string constant");
  else if (token_buffer[0] == '\'')
    error ("parse error at character constant");
  else
    error ("parse error at `%s'", token_buffer);
}
