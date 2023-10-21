/* Expands front end tree to back end RTL for GNU C-Compiler
   Copyright (C) 1987,1988 Free Software Foundation, Inc.
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


/* This file handles the generation of rtl code from tree structure
   above the level of expressions, using subroutines in exp*.c and emit-rtl.c.
   It also creates the rtl expressions for parameters and auto variables
   and has full responsibility for allocating stack slots.

   The functions whose names start with `expand_' are called by the
   parser to generate RTL instructions for various kinds of constructs.

   Some control and binding constructs require calling several such
   functions at different times.  For example, a simple if-then
   is expanded by calling `expand_start_cond' (with the condition-expression
   as argument) before parsing the then-clause and calling `expand_end_cond'
   after parsing the then-clause.

   `expand_start_function' is called at the beginning of a function,
   before the function body is parsed, and `expand_end_function' is
   called after parsing the body.

   Call `assign_stack_local' to allocate a stack slot for a local variable.
   This is usually done during the RTL generation for the function body,
   but it can also be done in the reload pass when a pseudo-register does
   not get a hard register.

   Call `put_var_into_stack' when you learn, belatedly, that a variable
   previously given a pseudo-register must in fact go in the stack.
   This function changes the DECL_RTL to be a stack slot instead of a reg
   then scans all the RTL instructions so far generated to correct them.  */

#include "config.h"

#include <stdio.h>

#include "rtl.h"
#include "tree.h"
#include "flags.h"
#include "insn-flags.h"
#include "insn-config.h"
#include "expr.h"
#include "regs.h"

#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

/* Nonzero if function being compiled pops its args on return.
   May affect compilation of return insn or of function epilogue.  */

int current_function_pops_args;

/* If function's args have a fixed size, this is that size, in bytes.
   Otherwise, it is -1.
   May affect compilation of return insn or of function epilogue.  */

int current_function_args_size;

/* # bytes the prologue should push and pretend that the caller pushed them.
   The prologue must do this, but only if parms can be passed in registers.  */

int current_function_pretend_args_size;

/* Label that will go on function epilogue.
   Jumping to this label serves as a "return" instruction
   on machines which require execution of the epilogue on all returns.  */

rtx return_label;

/* The FUNCTION_DECL node for the function being compiled.  */

static tree this_function;

/* Offset to end of allocated area of stack frame.
   If stack grows down, this is the address of the last stack slot allocated.
   If stack grows up, this is the address for the next slot.  */
static int frame_offset;

/* Nonzero if a stack slot has been generated whose address is not
   actually valid.  It means that the generated rtl must all be scanned
   to detect and correct the invalid addresses where they occur.  */
static int invalid_stack_slot;

/* Label to jump back to for tail recursion, or 0 if we have
   not yet needed one for this function.  */
static rtx tail_recursion_label;

/* Place after which to insert the tail_recursion_label if we need one.  */
static rtx tail_recursion_reentry;

/* Each time we expand an expression-statement,
   record the expr's type and its RTL value here.  */

static tree last_expr_type;
static rtx last_expr_value;

static void fixup_gotos ();
static int tail_recursion_args ();
void fixup_stack_slots ();
static rtx fixup_stack_1 ();
static rtx fixup_memory_subreg ();
static void fixup_var_refs ();
static rtx fixup_var_refs_1 ();
static rtx parm_stack_loc ();
static void optimize_bit_field ();
void do_jump_if_equal ();

/* C++ things.  */
void expand_aggr_birth ();
void cleanup_temporary_aggregates ();
void cleanup_aggregates ();
void cleanup_all_aggregates ();

/* Stack of control and binding constructs we are currently inside.

   These constructs begin when you call `expand_start_WHATEVER'
   and end when you call `expand_end_WHATEVER'.  This stack records
   info about how the construct began that tells the end-function
   what to do.  It also may provide information about the construct
   to alter the behavior of other constructs within the body.
   For example, they may affect the behavior of C `break' and `continue'.

   Each construct gets one `struct nesting' object.
   All of these objects are chained through the `all' field.
   `nesting_stack' points to the first object (innermost construct).
   The position of an entry on `nesting_stack' is in its `depth' field.

   Each type of construct has its own individual stack.
   For example, loops have `loop_stack'.  Each object points to the
   next object of the same type through the `next' field.

   Some constructs are visible to `break' exit-statements and others
   are not.  Which constructs are visible depends on the language.
   Therefore, the data structure allows each construct to be visible
   or not, according to the args given when the construct is started.
   The construct is visible if the `exit_label' field is non-null.
   In that case, the value should be a CODE_LABEL rtx.

   C++: it is frequently necessary to create temporary variables
   which can be used for type conversion operations, providing
   default arguments to functions, etc.  This impacts the nesting
   structure in two ways.

   The first impact is that what used to be simple nesting structures
   now may need to pop extra contours that are needed in the presence
   of conditional expressions.  For example, in the expression

	if (fn1 () && fn2 ()) fn3 ();

   if the call to fn2 requires that a temporary aggregate be created,
   then it must be destroyed *only if it was in fact created*.  Earlier
   versions of GNU CC contained a bug where it would create these
   temporaries at tree build time, in binding countours which did
   not know anything about lazy conditional evaluation.  Similarly,
   if the call to fn3 requires the creation of a temporary, then it
   too must be destroyed only if it was created.  An equivalent statement

	if (fn1 () && ({ fn2 (); })) { fn3 (); }

   illustrates the conceptual binding contours.

   The second (minor) issue is determining whether to allow the
   user any access to these temporaries.  Currently, they
   only live on a nesting contour, which is invisible to GDB/DBX.  */

struct nesting
{
  struct nesting *all;
  struct nesting *next;
  int depth;
  rtx exit_label;
  union
    {
      /* For conds (if-then and if-then-else statements).  */
      struct
	{
	  /* Label on the else-part, if any, else 0.  */
	  rtx else_label;
	  /* Label at the end of the whole construct.  */
	  rtx after_label;
	} cond;
      /* For loops.  */
      struct
	{
	  /* Label at the top of the loop; place to loop back to.  */
	  rtx start_label;
	  /* Label at the end of the whole construct.  */
	  rtx end_label;
	  /* Label for `continue' statement to jump to;
	     this is in front of the stepper of the loop.  */
	  rtx continue_label;
	} loop;
      /* For variable binding contours.  */
      struct
	{
	  /* Nonzero => value to restore stack to on exit.  */
	  rtx stack_level;
	  /* The NOTE that starts this contour.
	     Used by expand_goto to check whether the destination
	     is within each contour or not.  */
	  rtx first_insn;
	  /* Innermost containing binding contour that has a stack level.  */
	  struct nesting *innermost_stack_block;
	  /* Chain of labels defined inside this binding contour.
	     Only for contours that have stack levels.  */
	  struct label_chain *label_chain;

	  /* C++: nonzero if this binding contour should
	     be cleaned up at the next control point.  */
	  int temporary;

	  /* C++: For each binding countour, a list of all aggregate values
	     which have had constructors called on them.  When they
	     go out of scope, these _DECLs must be destroyed by whatever
	     destructors they use.  These are in the reverse order of the
	     order supplied.

	     The TREE_PURPOSE field holds the _DECL node, the TREE_VALUE
	     field holds the head of the insn chain for initializing
	     this aggregate.  */

	  tree aggregates;
	} block;
      /* For switch (C) or case (Pascal) statements,
	 and also for dummies (see `expand_start_case_dummy').  */
      struct
	{
	  /* The insn after which the case dispatch should finally
	     be emitted.  Zero for a dummy.  */
	  rtx start;
	  /* A list of the case-values and their labels.
	     A chain of TREE_LIST nodes with the value to test for
	     (a constant node) in the TREE_PURPOSE and the
	     label (a LABEL_DECL) in the TREE_VALUE.  */
	  tree case_list;
	  /* The expression to be dispatched on.  */
	  tree index_expr;
	  /* Type that INDEX_EXPR should be converted to.  */
	  tree nominal_type;

	  /* C++: look for default label quickly.  */
	  short has_default;
	  /* C++: number of range exprs in case statement.  */
	  short num_ranges;
	} case_stmt;
    } data;
};

/* Chain of all pending binding contours.  */
struct nesting *block_stack;

/* Chain of all pending binding contours that restore stack levels.  */
struct nesting *stack_block_stack;

/* Chain of all pending conditional statements.  */
struct nesting *cond_stack;

/* Chain of all pending loops.  */
struct nesting *loop_stack;

/* Chain of all pending case or switch statements.  */
struct nesting *case_stack;

/* Separate chain including all of the above,
   chained through the `all' field.  */
struct nesting *nesting_stack;

/* Number of entries on nesting_stack now.  */
int nesting_depth;

/* Pop one of the sub-stacks, such as `loop_stack' or `cond_stack';
   and pop off `nesting_stack' down to the same level.  */

#define POPSTACK(STACK)					\
do { int initial_depth = nesting_stack->depth;		\
     do { struct nesting *this = STACK;			\
	  STACK = this->next;				\
	  nesting_stack = this->all;			\
	  nesting_depth = this->depth;			\
	  free (this); }				\
     while (nesting_depth > initial_depth); } while (0)

/* C++: List of aggregates which are defined in global scope.
   This serves the purpose of having a global block_stack.  */
tree global_aggregates;

/* Return the rtx-label that corresponds to a LABEL_DECL,
   creating it if necessary.  */

static rtx
label_rtx (label)
     tree label;
{
  if (TREE_CODE (label) != LABEL_DECL)
    abort ();

  if (DECL_RTL (label))
    return DECL_RTL (label);

  return DECL_RTL (label) = gen_label_rtx ();
}

/* Add an unconditional jump to LABEL as the next sequential instruction.  */

void
emit_jump (label)
     rtx label;
{
  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();
  emit_jump_insn (gen_jump (label));
  emit_barrier ();
}

/* Handle goto statements and the labels that they can go to.  */

/* In some cases it is impossible to generate code for a forward goto 
   until the label definition is seen.  This happens when it may be necessary
   for the goto to reset the stack pointer: we don't yet know how to do that.
   So expand_goto puts an entry on this fixup list.
   Each time a binding contour that resets the stack is exited,
   we check each fixup.
   If the target label has now been defined, we can insert the proper code.  */

struct goto_fixup
{
  /* Points to following fixup.  */
  struct goto_fixup *next;
  /* Points to the insn before the jump insn.
     If more code must be inserted, it goes after this insn.  */
  rtx before_jump;
  /* The LABEL_DECL that this jump is jumping to.  */
  tree target;
  /* The outermost stack level that should be restored for this jump.
     Each time a binding contour that resets the stack is exited,
     if the target label is *not* yet defined, this slot is updated.  */
  rtx stack_level;
};

static struct goto_fixup *goto_fixup_chain;

/* Within any binding contour that must restore a stack level,
   all labels are recorded with a chain of these structures.  */

struct label_chain
{
  /* Points to following fixup.  */
  struct label_chain *next;
  tree label;
};

/* Specify the location in the RTL code of a label BODY,
   which is a LABEL_DECL tree node.

   This is used for the kind of label that the user can jump to with a
   goto statement, and for alternatives of a switch or case statement.
   RTL labels generated for loops and conditionals don't go through here;
   they are generated directly at the RTL level, by other functions below.

   Note that this has nothing to do with defining label *names*.
   Languages vary in how they do that and what that even means.  */

void
expand_label (body)
     tree body;
{
  struct label_chain *p;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();
  emit_label (label_rtx (body));

  if (stack_block_stack)
    {
      p = (struct label_chain *) oballoc (sizeof (struct label_chain));
      p->next = stack_block_stack->data.block.label_chain;
      stack_block_stack->data.block.label_chain = p;
      p->label = body;
    }
}

/* Generate RTL code for a `goto' statement with target label BODY.
   BODY should be a LABEL_DECL tree node that was or will later be
   defined with `expand_label'.  */

void
expand_goto (body)
     tree body;
{
  struct nesting *block;
  rtx stack_level = 0;
  rtx label = label_rtx (body);

  if (GET_CODE (label) != CODE_LABEL)
    abort ();

  /* If label has already been defined, we can tell now
     whether and how we must alter the stack level.  */

  if (DECL_SOURCE_LINE (body) != 0)
    {
      /* Find the outermost pending block that contains the label.
	 (Check containment by comparing insn-uids.)
	 Then restore the outermost stack level within that block.  */
      for (block = block_stack; block; block = block->next)
	{
	  if (INSN_UID (block->data.block.first_insn) < INSN_UID (label))
	    break;
	  if (block->data.block.stack_level != 0)
	    stack_level = block->data.block.stack_level;
	}

      if (stack_level)
	emit_move_insn (stack_pointer_rtx, stack_level);

      if (TREE_PACKED (body))
	error ("goto \"%s\" invalidly jumps into binding contour",
	       IDENTIFIER_POINTER (DECL_NAME (body)));
    }
  /* Label not yet defined: may need to put this goto
     on the fixup list.  */
  else
    {
      /* Does any containing block have a stack level?
	 If not, no fixup is needed, and that is the normal case
	 (the only case, for standard C).

	 For C++, we must also take care of temporary aggregates.  */
      for (block = block_stack; block; block = block->next)
	if (block->data.block.stack_level != 0
	    || block->data.block.aggregates != 0)
	  break;

      if (block)
	{
	  /* Ok, a fixup is needed.  Add a fixup to the list of such.  */
	  struct goto_fixup *fixup
	    = (struct goto_fixup *) oballoc (sizeof (struct goto_fixup));
	  /* In case an old stack level is restored, make sure that comes
	     after any pending stack adjust.  */
	  cleanup_temporary_aggregates ();
	  do_pending_stack_adjust ();
	  fixup->before_jump = get_last_insn ();
	  fixup->target = body;
	  fixup->stack_level = 0;
	  fixup->next = goto_fixup_chain;
	  goto_fixup_chain = fixup;
	}
      else
	/* No fixup needed.  Record that the label is the target
	   of at least one goto that has no fixup.  */
	TREE_ADDRESSABLE (body) = 1;
    }

  emit_jump (label);
}

/* When exiting a binding contour, process all pending gotos requiring fixups.
   STACK_LEVEL is the rtx for the stack level to restore on exit from
   this contour.  FIRST_INSN is the insn that begain this contour.
   Gotos that jump out of this contour must restore the
   stack level before actually jumping.

   Also print an error message if any fixup describes a jump into this
   contour from before the beginning of the contour.  */

static void
fixup_gotos (stack_level, first_insn)
     rtx stack_level;
     rtx first_insn;
{
  register struct goto_fixup *f;

  for (f = goto_fixup_chain; f; f = f->next)
    {
      /* Test for a fixup that is inactive because it is already handled.  */
      if (f->before_jump == 0)
	;
      /* Has this fixup's target label been defined?
	 If so, we can finalize it.  */
      else if (DECL_SOURCE_LINE (f->target) != 0)
	{
	  /* If this fixup jumped into this contour from before the beginning
	     of this contour, report an error.  */
	  if (INSN_UID (first_insn) > INSN_UID (f->before_jump)
	      && ! TREE_ADDRESSABLE (f->target))
	    {
	      error_with_file_and_line (DECL_SOURCE_FILE (f->target),
					DECL_SOURCE_LINE (f->target),
					"label \"%s\" was used \
before containing binding contour",
					IDENTIFIER_POINTER (DECL_NAME (f->target)));
	      /* Prevent multiple errors for one label.  */
	      TREE_ADDRESSABLE (f->target) = 1;
	    }

	  /* Restore stack level for the biggest contour that this
	     jump jumps out of.  */
	  if (f->stack_level)
	    emit_insn_after (gen_move_insn (stack_pointer_rtx, f->stack_level),
			     f->before_jump);
	  f->before_jump = 0;
	}
      /* Label has still not appeared.  If we are exiting a block with
	 a stack level to restore, mark this stack level as needing
	 restoration when the fixup is later finalized.  */
      else if (stack_level)
	f->stack_level = stack_level;
    }
}

/* Generate RTL for an asm statement (explicit assembler code).
   BODY is a STRING_CST node containing the assembler code text.  */

void
expand_asm (body)
     tree body;
{
  emit_insn (gen_rtx (ASM_INPUT, VOIDmode,
		      TREE_STRING_POINTER (body)));
  last_expr_type = 0;
}

/* Generate RTL for an asm statement with arguments.
   STRING is the instruction template.
   OUTPUTS is a list of output arguments (lvalues); INPUTS a list of inputs.
   Each output or input has an expression in the TREE_VALUE and
   a constraint-string in the TREE_PURPOSE.

   Not all kinds of lvalue that may appear in OUTPUTS can be stored directly.
   Some elements of OUTPUTS may be replaced with trees representing temporary
   values.  The caller should copy those temporary values to the originally
   specified lvalues.

   VOL nonzero means the insn is volatile; don't optimize it.  */

void
expand_asm_operands (string, outputs, inputs, vol)
     tree string, outputs, inputs;
     int vol;
{
  rtvec argvec, constraints;
  rtx body;
  int ninputs = list_length (inputs);
  int noutputs = list_length (outputs);
  int numargs = 0;
  tree tail;
  int i;

  if (ninputs + noutputs > MAX_RECOG_OPERANDS)
    {
      error ("more than %d operands in `asm'", MAX_RECOG_OPERANDS);
      return;
    }

  for (i = 0, tail = outputs; tail; tail = TREE_CHAIN (tail), i++)
    {
      tree val = TREE_VALUE (tail);

      /* If an output operand is not a variable or indirect ref,
	 create a SAVE_EXPR which is a pseudo-reg
	 to act as an intermediate temporary.
	 Make the asm insn write into that, then copy it to
	 the real output operand.  */

      if (TREE_CODE (val) != VAR_DECL
	  && TREE_CODE (val) != PARM_DECL
	  && TREE_CODE (val) != INDIRECT_REF)
	TREE_VALUE (tail) = build (SAVE_EXPR, TREE_TYPE (val), val,
				   gen_reg_rtx (TYPE_MODE (TREE_TYPE (val))));
    }

  /* Make vectors for the expression-rtx and constraint strings.  */

  argvec = rtvec_alloc (ninputs);
  constraints = rtvec_alloc (ninputs);

  body = gen_rtx (ASM_OPERANDS, VOIDmode,
		  TREE_STRING_POINTER (string), "", 0, argvec, constraints);
  body->volatil = vol;

  /* Eval the inputs and put them into ARGVEC.
     Put their constraints into ASM_INPUTs and store in CONSTRAINTS.  */

  i = 0;
  for (tail = inputs; tail; tail = TREE_CHAIN (tail))
    {
      XVECEXP (body, 3, i)      /* argvec */
	= expand_expr (TREE_VALUE (tail), 0, VOIDmode, 0);
      XVECEXP (body, 4, i)      /* constraints */
	= gen_rtx (ASM_INPUT, TYPE_MODE (TREE_TYPE (TREE_VALUE (tail))),
		   TREE_STRING_POINTER (TREE_PURPOSE (tail)));
      i++;
    }

  /* Now, for each output, construct an rtx
     (set OUTPUT (asm_operands INSN OUTPUTNUMBER OUTPUTCONSTRAINT
			       ARGVEC CONSTRAINTS))
     If there is more than one, put them inside a PARALLEL.  */

  if (noutputs == 1)
    {
      tree val = TREE_VALUE (outputs);

      XSTR (body, 1) = TREE_STRING_POINTER (TREE_PURPOSE (outputs));
      emit_insn (gen_rtx (SET, VOIDmode,
			  expand_expr (val, 0, VOIDmode, 0),
			  body));
    }
  else
    {
      body = gen_rtx (PARALLEL, VOIDmode, rtvec_alloc (noutputs));

      for (i = 0, tail = outputs; tail; tail = TREE_CHAIN (tail), i++)
	{
	  tree val = TREE_VALUE (tail);

	  XVECEXP (body, 0, i)
	    = gen_rtx (SET, VOIDmode,
		       expand_expr (val, 0, VOIDmode, 0),
		       gen_rtx (ASM_OPERANDS, VOIDmode,
				TREE_STRING_POINTER (string),
				TREE_STRING_POINTER (TREE_PURPOSE (tail)),
				i, argvec, constraints));
	  SET_SRC (XVECEXP (body, 0, i))->volatil = vol;
	}

      emit_insn (body);
    }
  last_expr_type = 0;
}

/* Nonzero if within a ({...}) grouping, in which case we must
   always compute a value for each expr-stmt in case it is the last one.  */

int expr_stmts_for_value;

/* Generate RTL to evaluate the expression EXP
   and remember it in case this is the VALUE in a ({... VALUE; }) constr.  */

void
expand_expr_stmt (exp)
     tree exp;
{
  /* C++: do not let temporaries accumulate on stack.  */
  rtx real_last_expr_value;
  static int delete_temps = 1;

  last_expr_type = TREE_TYPE (exp);
  real_last_expr_value = expand_expr (exp, expr_stmts_for_value ? 0 : const0_rtx,
				      VOIDmode, 0);
  emit_queue ();

  /* C++: destroy temporaries which were created in this expr_stmt.  */
  if (block_stack != 0 && delete_temps != 0)
    {
      tree temps, prev_temps;
      /* Do not get caught in infinite recursion if
	 expand_*_delete calls expand_expr_stmt.  */
      delete_temps = 0;

      temps = block_stack->data.block.aggregates;
      prev_temps = NULL_TREE;
      while (temps)
	{
	  tree decl = TREE_PURPOSE (temps);
	  if (temp_name_p (DECL_NAME (decl)))
	    {
	      if (TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE)
		expand_vec_delete (TYPE_NELTS (TREE_TYPE (decl)),
				   decl, integer_zero_node);
	      else
		expand_delete (TREE_TYPE (decl), decl, integer_zero_node);
	      if (prev_temps)
		TREE_CHAIN (prev_temps) = TREE_CHAIN (temps);
	      else
		block_stack->data.block.aggregates = TREE_CHAIN (temps);
	    }
	  else
	    prev_temps = temps;
	  temps = TREE_CHAIN (temps);
	}
      delete_temps = 1;
    }
  /* Keep expand_*_delete from setting last_expr_value.  */
  last_expr_value = real_last_expr_value;
}

/* Clear out the memory of the last expression evaluated.  */

void
clear_last_expr ()
{
  last_expr_type = 0;
}

/* Return a tree node that refers to the last expression evaluated.
   The nodes of that expression have been freed by now, so we cannot use them.
   But we don't want to do that anyway; the expression has already been
   evaluated and now we just want to use the value.  So generate a SAVE_EXPR
   with the proper type and RTL value.

   If the last statement was not an expression,
   return something with type `void'.  */

tree
get_last_expr ()
{
  tree t;

  if (last_expr_type == 0)
    {
      last_expr_type = void_type_node;
      last_expr_value = const0_rtx;
    }
  t = build (RTL_EXPR, last_expr_type, NULL, NULL);
  RTL_EXPR_RTL (t) = last_expr_value;
  RTL_EXPR_SEQUENCE (t) = gen_sequence ();
  return t;
}

void
expand_start_stmt_expr ()
{
  extern int emit_to_sequence;
  expr_stmts_for_value++;
  emit_to_sequence++;
}

void
expand_end_stmt_expr ()
{
  extern int emit_to_sequence;
  expr_stmts_for_value--;
  emit_to_sequence--;
}

/* Generate RTL for the start of an if-then.  COND is the expression
   whose truth should be tested.

   If EXITFLAG is nonzero, this conditional is visible to
   `exit_something'.  */

void
expand_start_cond (cond, exitflag)
     tree cond;
     int exitflag;
{
  struct nesting *thiscond
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  /* Make an entry on cond_stack for the cond we are entering.  */

  thiscond->next = cond_stack;
  thiscond->all = nesting_stack;
  thiscond->depth = ++nesting_depth;
  thiscond->data.cond.after_label = 0;
  thiscond->data.cond.else_label = gen_label_rtx ();
  thiscond->exit_label = exitflag ? thiscond->data.cond.else_label : 0;
  cond_stack = thiscond;
  nesting_stack = thiscond;

  do_jump (cond, thiscond->data.cond.else_label, NULL);
}

/* Generate RTL for the end of an if-then with no else-clause.
   Pop the record for it off of cond_stack.  */

void
expand_end_cond ()
{
  struct nesting *thiscond = cond_stack;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();
  emit_label (thiscond->data.cond.else_label);

  POPSTACK (cond_stack);
  last_expr_type = 0;
}

/* Generate RTL between the then-clause and the else-clause
   of an if-then-else.  */

void
expand_start_else ()
{
  cond_stack->data.cond.after_label = gen_label_rtx ();
  if (cond_stack->exit_label != 0)
    cond_stack->exit_label = cond_stack->data.cond.after_label;
  emit_jump (cond_stack->data.cond.after_label);
  if (cond_stack->data.cond.else_label)
    emit_label (cond_stack->data.cond.else_label);
}

/* Generate RTL for the end of an if-then-else.
   Pop the record for it off of cond_stack.  */

void
expand_end_else ()
{
  struct nesting *thiscond = cond_stack;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();
  /* Note: a syntax error can cause this to be called
     without first calling `expand_start_else'.  */
  if (thiscond->data.cond.after_label)
    emit_label (thiscond->data.cond.after_label);

  POPSTACK (cond_stack);
  last_expr_type = 0;
}

/* Generate RTL for the start of a loop.  EXIT_FLAG is nonzero if this
   loop should be exited by `exit_something'.  This is a loop for which
   `expand_continue' will jump to the top of the loop.

   Make an entry on loop_stack to record the labels associated with
   this loop.  */

void
expand_start_loop (exit_flag)
     int exit_flag;
{
  register struct nesting *thisloop
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  /* Make an entry on loop_stack for the loop we are entering.  */

  thisloop->next = loop_stack;
  thisloop->all = nesting_stack;
  thisloop->depth = ++nesting_depth;
  thisloop->data.loop.start_label = gen_label_rtx ();
  thisloop->data.loop.end_label = gen_label_rtx ();
  thisloop->data.loop.continue_label = thisloop->data.loop.start_label;
  thisloop->exit_label = exit_flag ? thisloop->data.loop.end_label : 0;
  loop_stack = thisloop;
  nesting_stack = thisloop;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();
  emit_queue ();
  emit_note (0, NOTE_INSN_LOOP_BEG);
  emit_label (thisloop->data.loop.start_label);
}

/* Like expand_start_loop but for a loop where the continuation point
   (for expand_continue_loop) will be specified explicitly.  */

void
expand_start_loop_continue_elsewhere (exit_flag)
     int exit_flag;
{
  expand_start_loop (exit_flag);
  loop_stack->data.loop.continue_label = gen_label_rtx ();
}

/* Specify the continuation point for a loop started with
   expand_start_loop_continue_elsewhere.
   Use this at the point in the code to which a continue statement
   should jump.  */

void
expand_loop_continue_here ()
{
  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();
  emit_label (loop_stack->data.loop.continue_label);
}

/* Finish a loop.  Generate a jump back to the top and the loop-exit label.
   Pop the block off of loop_stack.  */

void
expand_end_loop ()
{
  register struct nesting *thisloop = loop_stack;
  register rtx insn = get_last_insn ();
  register rtx start_label = loop_stack->data.loop.start_label;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();

  /* If optimizing, perhaps reorder the loop.  If the loop
     starts with a conditional exit, roll that to the end
     where it will optimize together with the jump back.  */
  if (optimize
      &&
      ! (GET_CODE (insn) == JUMP_INSN
	 && GET_CODE (PATTERN (insn)) == SET
	 && SET_DEST (PATTERN (insn)) == pc_rtx
	 && GET_CODE (SET_SRC (PATTERN (insn))) == IF_THEN_ELSE))
    {
      /* Scan insns from the top of the loop looking for a qualified
	 conditional exit.  */
      for (insn = loop_stack->data.loop.start_label; insn; insn= NEXT_INSN (insn))
	if (GET_CODE (insn) == JUMP_INSN && GET_CODE (PATTERN (insn)) == SET
	    && SET_DEST (PATTERN (insn)) == pc_rtx
	    && GET_CODE (SET_SRC (PATTERN (insn))) == IF_THEN_ELSE
	    &&
	    ((GET_CODE (XEXP (SET_SRC (PATTERN (insn)), 1)) == LABEL_REF
	      && (XEXP (XEXP (SET_SRC (PATTERN (insn)), 1), 0)
		  == loop_stack->data.loop.end_label))
	     ||
	     (GET_CODE (XEXP (SET_SRC (PATTERN (insn)), 2)) == LABEL_REF
	      && (XEXP (XEXP (SET_SRC (PATTERN (insn)), 2), 0)
		  == loop_stack->data.loop.end_label))))
	  break;
      if (insn != 0)
	{
	  /* We found one.  Move everything from there up
	     to the end of the loop, and add a jump into the loop
	     to jump to there.  */
	  register rtx newstart_label = gen_label_rtx ();

	  emit_label_after (newstart_label, PREV_INSN (start_label));
	  reorder_insns (start_label, insn, get_last_insn ());
	  emit_jump_insn_after (gen_jump (start_label), PREV_INSN (newstart_label));
	  emit_barrier_after (PREV_INSN (newstart_label));
	  start_label = newstart_label;
	}
    }

  emit_jump (start_label);
  emit_note (0, NOTE_INSN_LOOP_END);
  emit_label (loop_stack->data.loop.end_label);

  POPSTACK (loop_stack);

  last_expr_type = 0;
}

/* Generate a jump to the current loop's continue-point.
   This is usually the top of the loop, but may be specified
   explicitly elsewhere.  If not currently inside a loop,
   return 0 and do nothing; caller will print an error message.  */

int
expand_continue_loop ()
{
  last_expr_type = 0;
  if (loop_stack == 0)
    return 0;
  emit_jump (loop_stack->data.loop.continue_label);
  return 1;
}

/* Generate a jump to exit the current loop.  If not currently inside a loop,
   return 0 and do nothing; caller will print an error message.  */

int
expand_exit_loop ()
{
  last_expr_type = 0;
  if (loop_stack == 0)
    return 0;
  emit_jump (loop_stack->data.loop.end_label);
  return 1;
}

/* Generate a conditional jump to exit the current loop if COND
   evaluates to zero.  If not currently inside a loop,
   return 0 and do nothing; caller will print an error message.  */

int
expand_exit_loop_if_false (cond)
     tree cond;
{
  last_expr_type = 0;
  if (loop_stack == 0)
    return 0;
  do_jump (cond, loop_stack->data.loop.end_label, NULL);
  return 1;
}

/* Generate a jump to exit the current loop, conditional, binding contour
   or case statement.  Not all such constructs are visible to this function,
   only those started with EXIT_FLAG nonzero.  Individual languages use
   the EXIT_FLAG parameter to control which kinds of constructs you can
   exit this way.

   If not currently inside anything that can be exited,
   return 0 and do nothing; caller will print an error message.  */

int
expand_exit_something ()
{
  struct nesting *n, *b = block_stack;
  last_expr_type = 0;
  for (n = nesting_stack; n; n = n->all)
    {
      if (n == b)
	{
	  if (b->data.block.aggregates)
	    error ("jump target is outside scope binding contour with aggregate values needing destructors");
	  b = b->next;
	}

      if (n->exit_label != 0)
	{
	  emit_jump (n->exit_label);
	  return 1;
	}
    }
  return 0;
}

/* Generate RTL to return from the current function, with no value.
   (That is, we do not do anything about returning any value.)  */

void
expand_null_return ()
{
  clear_pending_stack_adjust ();
#ifdef FUNCTION_EPILOGUE
  emit_jump (return_label);
#else
  emit_jump_insn (gen_return ());
  emit_barrier ();
#endif
  last_expr_type = 0;
}

/* Generate RTL to evaluate the expression RETVAL and return it
   from the current function.  */

void
expand_return (retval)
     tree retval;
{
  register rtx val = 0;
  register rtx op0;
  register tree retval_rhs;

  if (TREE_CODE (retval) == RESULT_DECL)
    retval_rhs = retval;
  else if ((TREE_CODE (retval) == MODIFY_EXPR || TREE_CODE (retval) == INIT_EXPR)
	   && TREE_CODE (TREE_OPERAND (retval, 0)) == RESULT_DECL)
    retval_rhs = TREE_OPERAND (retval, 1);
  else retval_rhs = NULL_TREE;

  /* For tail-recursive call to current function,
     just jump back to the beginning.
     It's unsafe if any auto variable in this function
     has its address taken; for simplicity,
     require stack frame to be empty.  */
  if (optimize && retval_rhs
      && frame_offset == 0
      && TREE_CODE (retval_rhs) == CALL_EXPR
      && TREE_CODE (TREE_OPERAND (retval_rhs, 0)) == ADDR_EXPR
      && TREE_OPERAND (TREE_OPERAND (retval_rhs, 0), 0) == this_function
      /* Finish checking validity, and if valid emit code
	 to set the argument variables for the new call.  */
      && tail_recursion_args (TREE_OPERAND (retval_rhs, 1),
			      DECL_ARGUMENTS (this_function)))
    {
      ;
      if (tail_recursion_label == 0)
	{
	  tail_recursion_label = gen_label_rtx ();
	  emit_label_after (tail_recursion_label,
			    tail_recursion_reentry);
	}
      emit_jump (tail_recursion_label);
      emit_barrier ();
      return;
    }
#ifndef FUNCTION_EPILOGUE
  /* If this is  return x == y;  then generate
     if (x == y) return 1; else return 0;
     if we can do it with explicit return insns.  */
  if (retval_rhs)
    switch (TREE_CODE (retval_rhs))
      {
      case EQ_EXPR:
      case NE_EXPR:
      case GT_EXPR:
      case GE_EXPR:
      case LT_EXPR:
      case LE_EXPR:
      case TRUTH_ANDIF_EXPR:
      case TRUTH_ORIF_EXPR:
      case TRUTH_NOT_EXPR:
	op0 = gen_label_rtx ();
	val = DECL_RTL (DECL_RESULT (this_function));
	jumpifnot (retval_rhs, op0);
	emit_move_insn (val, const1_rtx);
	emit_insn (gen_rtx (USE, VOIDmode, val));
	expand_null_return ();
	emit_label (op0);
	emit_move_insn (val, const0_rtx);
	emit_insn (gen_rtx (USE, VOIDmode, val));
	expand_null_return ();
	return;
      }
#endif
  val = expand_expr (retval, 0, VOIDmode, 0);
  emit_queue ();

  if (retval_rhs && GET_CODE (val) == REG)
    emit_insn (gen_rtx (USE, VOIDmode, val));

  expand_null_return ();
}

/* Return 1 if the end of the generated RTX is not a barrier.
   This means code already compiled can drop through.  */

int
drop_through_at_end_p ()
{
  rtx insn = get_last_insn ();
  while (insn && GET_CODE (insn) == NOTE)
    insn = PREV_INSN (insn);
  return insn && GET_CODE (insn) != BARRIER;
}

/* Emit code to alter this function's formal parms for a tail-recursive call.
   ACTUALS is a list of actual parameter expressions (chain of TREE_LISTs).
   FORMALS is the chain of decls of formals.
   Return 1 if this can be done;
   otherwise return 0 and do not emit any code.  */

static int
tail_recursion_args (actuals, formals)
     tree actuals, formals;
{
  register tree a = actuals, f = formals;
  register int i;
  register rtx *argvec;

  /* Check that number and types of actuals are compatible
     with the formals.  This is not always true in valid C code.
     Also check that no formal needs to be addressable
     and that all formals are scalars.  */

  /* Also count the args.  */

  for (a = actuals, f = formals, i = 0; a && f; a = TREE_CHAIN (a), f = TREE_CHAIN (f), i++)
    {
      if (TREE_TYPE (TREE_VALUE (a)) != TREE_TYPE (f))
	return 0;
      if (GET_CODE (DECL_RTL (f)) != REG || DECL_MODE (f) == BLKmode)
	return 0;
    }
  if (a != 0 || f != 0)
    return 0;

  /* Compute all the actuals.  */

  argvec = (rtx *) alloca (i * sizeof (rtx));

  for (a = actuals, i = 0; a; a = TREE_CHAIN (a), i++)
    argvec[i] = expand_expr (TREE_VALUE (a), 0, VOIDmode, 0);

  /* Find which actual values refer to current values of previous formals.
     Copy each of them now, before any formal is changed.  */

  for (a = actuals, i = 0; a; a = TREE_CHAIN (a), i++)
    {
      int copy = 0;
      register int j;
      for (f = formals, j = 0; j < i; f = TREE_CHAIN (f), j++)
	if (reg_mentioned_p (DECL_RTL (f), argvec[i]))
	  { copy = 1; break; }
      if (copy)
	argvec[i] = copy_to_reg (argvec[i]);
    }

  /* Store the values of the actuals into the formals.  */

  for (f = formals, a = actuals, i = 0; f;
       f = TREE_CHAIN (f), a = TREE_CHAIN (a), i++)
    {
      if (DECL_MODE (f) == GET_MODE (argvec[i]))
	emit_move_insn (DECL_RTL (f), argvec[i]);
      else
	convert_move (DECL_RTL (f), argvec[i],
		      TREE_UNSIGNED (TREE_TYPE (TREE_VALUE (a))));
    }

  return 1;
}

/* Generate the RTL code for entering a binding contour.
   The variables are declared one by one, by calls to `expand_decl'.

   EXIT_FLAG is nonzero if this construct should be visible to
   `exit_something'.  */

void
expand_start_bindings (exit_flag)
     int exit_flag;
{
  struct nesting *thisblock
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  rtx note = emit_note (0, NOTE_INSN_BLOCK_BEG);

  /* Make an entry on block_stack for the block we are entering.  */

  thisblock->next = block_stack;
  thisblock->all = nesting_stack;
  thisblock->depth = ++nesting_depth;
  thisblock->data.block.stack_level = 0;
  thisblock->data.block.label_chain = 0;
  thisblock->data.block.innermost_stack_block = stack_block_stack;
  thisblock->data.block.first_insn = note;
  thisblock->data.block.aggregates = 0;
  thisblock->data.block.temporary = 0;
  thisblock->exit_label = exit_flag ? gen_label_rtx () : 0;
  block_stack = thisblock;
  nesting_stack = thisblock;
}

/* Generate RTL code to terminate a binding contour.
   GET_DECLS_P is non-zero if we should call `getdecls'
   for the variables bound in this contour.
   MARK_ENDs is nonzero if we should put a note at the beginning
   and end of this binding contour.  */

void
expand_end_bindings (call_getdecls_p, mark_ends)
     int call_getdecls_p;
     int mark_ends;
{
  tree vars, getdecls ();
  register struct nesting *thisblock = block_stack;
  register tree decl;

  /* This may add some last-minute decls to the binding contour.  */
  cleanup_aggregates ();

  if (call_getdecls_p)
    vars = getdecls ();
  else
    vars = NULL_TREE;

  /* Mark the beginning and end of the scope if requested.  */

  if (mark_ends)
    emit_note (0, NOTE_INSN_BLOCK_END);
  else
    /* Get rid of the beginning-mark if we don't make an end-mark.  */
    NOTE_LINE_NUMBER (thisblock->data.block.first_insn) = NOTE_INSN_DELETED;

  if (thisblock->exit_label)
    {
      do_pending_stack_adjust ();
      emit_label (thisblock->exit_label);
    }

  /* Restore stack level in effect before the block
     (only if variable-size objects allocated).  */

  if (thisblock->data.block.stack_level != 0
      || thisblock->data.block.aggregates != 0)
    {
      struct label_chain *chain;

      do_pending_stack_adjust ();
      if (thisblock->data.block.stack_level != 0)
	emit_move_insn (stack_pointer_rtx, thisblock->data.block.stack_level);

      /* Any labels in this block are no longer valid to go to.
	 Mark them to cause an error message.  */
      for (chain = thisblock->data.block.label_chain; chain; chain = chain->next)
	{
	  TREE_PACKED (chain->label) = 1;
	  /* If any goto without a fixup came to this label,
	     that must be an error, because gotos without fixups
	     come from outside all saved stack-levels.  */
	  if (TREE_ADDRESSABLE (chain->label))
	    error_with_file_and_line (DECL_SOURCE_FILE (chain->label),
				      DECL_SOURCE_LINE (chain->label),
				      "label \"%s\" was used \
before containing binding contour",
				      IDENTIFIER_POINTER (DECL_NAME (chain->label)));
	}

      /* Any gotos out of this block must also restore the stack level.
	 Also report any gotos with fixups that came to labels in this level.  */
      fixup_gotos (thisblock->data.block.stack_level,
		   thisblock->data.block.first_insn);
    }

  /* If doing stupid register allocation, make sure lives of all
     register variables declared here extend thru end of scope.  */

  if (obey_regdecls)
    for (decl = vars; decl; decl = TREE_CHAIN (decl))
      {
	if (TREE_CODE (decl) == VAR_DECL
	    && DECL_RTL (decl) != 0
	    && GET_CODE (DECL_RTL (decl)) == REG)
	  emit_insn (gen_rtx (USE, VOIDmode, DECL_RTL (decl)));
      }

  /* Restore block_stack level for containing block.  */

  stack_block_stack = thisblock->data.block.innermost_stack_block;
  POPSTACK (block_stack);
}

/* Generate code to call destructors on all temporary aggregates
   which have been initialized in this and all consecutive
   block_stacks which contain temporary aggregates.  */
void
cleanup_temporary_aggregates ()
{
  if (block_stack)
    while (block_stack == nesting_stack
	   && block_stack->data.block.temporary)
      {
	expand_end_bindings (0, 0);
      }
}

/* Move all temporary aggregates from the current block_stack
   to the containing block_stack, where they are assumed to
   have been created.  If anything can cause a temporary to
   be created, but not expanded for more than one level of
   block_stacks, then this code will have to change.  */
void
move_temporary_aggregates_up ()
{
  tree temps, prev_temps;
  struct nesting *outer = block_stack->next;

  temps = block_stack->data.block.aggregates;
  prev_temps = NULL_TREE;
  while (temps)
    {
      tree decl = TREE_PURPOSE (temps);
      if (temp_name_p (DECL_NAME (decl)))
	{
	  outer->data.block.aggregates
	    = tree_cons (decl, NULL_TREE, outer->data.block.aggregates);
	  if (prev_temps)
	    TREE_CHAIN (prev_temps) = TREE_CHAIN (temps);
	  else
	    block_stack->data.block.aggregates = TREE_CHAIN (temps);
	}
      else
	prev_temps = temps;
      temps = TREE_CHAIN (temps);
    }
}

/* Generate RTL for the automatic variable declaration DECL.
   (Other kinds of declarations are simply ignored.)

   C++: if `nesting_stack' is not `block_stack' (i.e. we are
   not simply in a binding contour), then make this be the case.
   When expand_end_WHATEVER is called, this assymmetry will be
   corrected by a call to expand_end_x_bindings ().

   CALL_CTOR is non-zero if we should call a constructor
   in order to initialize DECL. */

void
expand_decl (decl, call_ctor)
     register tree decl;
     int call_ctor;
{
  struct nesting *thisblock = block_stack;
  tree type = TREE_TYPE (decl);

  /* C++: if we are in some sort of control stack's scope C
     create a binding contour B so that we might remove the
     temporary when we leave C's scope (by popping B).  */
  if (nesting_stack != thisblock)
    {
      expand_start_bindings (0);
      thisblock = block_stack;
      thisblock->data.block.temporary = 1;
    }

  /* External function declarations are supposed to have been
     handled in assemble_variable.  Verify this.  */
  if (TREE_CODE (decl) == FUNCTION_DECL)
    {
      if (DECL_RTL (decl) == 0)
	abort ();
      return;
    }

  /* Aside from that, only automatic variables need any expansion done.
     Static and external variables were handled by `assemble_variable'
     (called from finish_decl).  TYPE_DECL and CONST_DECL require nothing;
     PARM_DECLs are handled in `assign_parms'.  */

  if (TREE_CODE (decl) != VAR_DECL)
    return;
  if (TREE_STATIC (decl) || TREE_EXTERNAL (decl))
    return;

  /* Create the RTL representation for the variable.  */

  if (type == error_mark_node)
    DECL_RTL (decl) = gen_rtx (MEM, BLKmode, const0_rtx);
  else if (DECL_MODE (decl) != BLKmode
	   /* If -ffloat-store, don't put explicit float vars
	      into regs.  */
	   && !(flag_float_store
		&& TREE_CODE (type) == REAL_TYPE)
	   && ! TREE_VOLATILE (decl)
	   && ! TREE_ADDRESSABLE (decl)
	   && (TREE_REGDECL (decl) || ! obey_regdecls))
    {
      /* Automatic variable that can go in a register.  */
      DECL_RTL (decl) = gen_reg_rtx (DECL_MODE (decl));
      if (TREE_CODE (type) == POINTER_TYPE)
	mark_reg_pointer (DECL_RTL (decl));
      DECL_RTL (decl)->volatil = 1;
    }
  else if (DECL_SIZE (decl) == 0)
    /* Variable with incomplete type.  */
    /* Error message was already done; now avoid a crash.  */
    DECL_RTL (decl) = assign_stack_local (DECL_MODE (decl), 0);
  else if (TREE_LITERAL (DECL_SIZE (decl)))
    {
      /* Variable of fixed size that goes on the stack.  */
      DECL_RTL (decl)
	= assign_stack_local (DECL_MODE (decl),
			      (TREE_INT_CST_LOW (DECL_SIZE (decl))
			       * DECL_SIZE_UNIT (decl)
			       + BITS_PER_UNIT - 1)
			      / BITS_PER_UNIT);
      /* If this is a memory ref that contains aggregate components,
	 mark it as such for cse and loop optimize.  */
      DECL_RTL (decl)->in_struct
	= (TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE
	   || TREE_CODE (TREE_TYPE (decl)) == RECORD_TYPE
	   || TREE_CODE (TREE_TYPE (decl)) == UNION_TYPE);
    }
  else
    /* Dynamic-size object: must push space on the stack.  */
    {
      rtx address, size;

      frame_pointer_needed = 1;

      /* Record the stack pointer on entry to block, if have
	 not already done so.  */
      if (thisblock->data.block.stack_level == 0)
	{
	  do_pending_stack_adjust ();
	  thisblock->data.block.stack_level
	    = copy_to_reg (stack_pointer_rtx);
	  stack_block_stack = thisblock;
	}

      /* Compute the variable's size, in bytes.  */
      size = expand_expr (convert_units (DECL_SIZE (decl),
					 DECL_SIZE_UNIT (decl),
					 BITS_PER_UNIT),
			  0, VOIDmode, 0);

      /* Round it up to this machine's required stack boundary.  */
#ifdef STACK_BOUNDARY
      /* Avoid extra code if we can prove it's a multiple already.  */
      if (DECL_SIZE_UNIT (decl) % STACK_BOUNDARY)
	size = round_push (size);
#endif

      /* Make space on the stack, and get an rtx for the address of it.  */
#ifdef STACK_GROWS_DOWNWARD
      anti_adjust_stack (size);
#endif
      address = copy_to_reg (stack_pointer_rtx);
#ifndef STACK_GROWS_DOWNWARD
      anti_adjust_stack (size);
#endif

      /* Reference the variable indirect through that rtx.  */
      DECL_RTL (decl) = gen_rtx (MEM, DECL_MODE (decl), address);
    }

  if (TREE_VOLATILE (decl))
    DECL_RTL (decl)->volatil = 1;
  if (TREE_READONLY (decl))
    DECL_RTL (decl)->unchanging = 1;

  /* If doing stupid register allocation, make sure life of any
     register variable starts here, at the start of its scope.  */

  if (obey_regdecls
      && TREE_CODE (decl) == VAR_DECL
      && DECL_RTL (decl) != 0
      && GET_CODE (DECL_RTL (decl)) == REG)
    emit_insn (gen_rtx (USE, VOIDmode, DECL_RTL (decl)));

  /* Compute and store the initial value now.  */
  if (TREE_CODE (type) == RECORD_TYPE || TREE_CODE (type) == UNION_TYPE
      || (TREE_CODE (type) == ARRAY_TYPE &&
	  (TREE_CODE (TREE_TYPE (type)) == RECORD_TYPE
	   || TREE_CODE (TREE_TYPE (type)) == UNION_TYPE)))
    expand_aggr_birth (decl, DECL_INITIAL (decl), call_ctor);

  else if (DECL_INITIAL (decl) && TREE_CODE (DECL_INITIAL (decl)) != TREE_LIST)
    {
      emit_note (DECL_SOURCE_FILE (decl), DECL_SOURCE_LINE (decl));
      expand_assignment (decl, DECL_INITIAL (decl), 0, 0);
      emit_queue ();
    }
}

/* Enter a case (Pascal) or switch (C) statement.
   Push a block onto case_stack and nesting_stack
   to accumulate the case-labels that are seen
   and to record the labels generated for the statement.

   EXIT_FLAG is nonzero if `exit_something' should exit this case stmt.
   Otherwise, this construct is transparent for `exit_something'.

   EXPR is the index-expression to be dispatched on.
   TYPE is its nominal type.  We could simply convert EXPR to this type,
   but instead we take short cuts.  */

void
expand_start_case (exit_flag, expr, type)
     int exit_flag;
     tree expr;
     tree type;
{
  register struct nesting *thiscase
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  /* Make an entry on case_stack for the case we are entering.  */

  thiscase->next = case_stack;
  thiscase->all = nesting_stack;
  thiscase->depth = ++nesting_depth;
  thiscase->exit_label = exit_flag ? gen_label_rtx () : 0;
  thiscase->data.case_stmt.case_list = 0;
  thiscase->data.case_stmt.index_expr = expr;
  thiscase->data.case_stmt.nominal_type = type;
  thiscase->data.case_stmt.has_default = 0;
  thiscase->data.case_stmt.num_ranges = 0;
  case_stack = thiscase;
  nesting_stack = thiscase;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();

  thiscase->data.case_stmt.start = get_last_insn ();
}

/* Start a "dummy case statement" within which case labels are invalid
   and are not connected to any larger real case statement.
   This can be used if you don't want to let a case statement jump
   into the middle of certain kinds of constructs.  */

void
expand_start_case_dummy ()
{
  register struct nesting *thiscase
    = (struct nesting *) xmalloc (sizeof (struct nesting));

  /* Make an entry on case_stack for the dummy.  */

  thiscase->next = case_stack;
  thiscase->all = nesting_stack;
  thiscase->depth = ++nesting_depth;
  thiscase->exit_label = 0;
  thiscase->data.case_stmt.case_list = 0;
  thiscase->data.case_stmt.start = 0;
  thiscase->data.case_stmt.nominal_type = 0;
  thiscase->data.case_stmt.has_default = 0;
  thiscase->data.case_stmt.num_ranges = 0;
  case_stack = thiscase;
  nesting_stack = thiscase;
}

/* End a dummy case statement.  */

void
expand_end_case_dummy ()
{
  POPSTACK (case_stack);
}

/* Accumulate one case or default label inside a case or switch statement.
   VALUE is the value of the case (a null pointer, for a default label).

   If not currently inside a case or switch statement, return 1 and do
   nothing.  The caller will print a language-specific error message.
   If VALUE is a duplicate, return 2 and do nothing.
   If VALUE is out of range, return 3 and do nothing.
   Return 0 on success.
   Return -1 in the face of inherited error.

   C++: extended to handle range statements, should they ever
   be adopted.  */

void
pushcase (value, label)
     register tree value;
     register tree label;
{
  register tree l;
  tree index_type;
  tree nominal_type;

  /* Fail if not inside a real case statement.  */
  if (! (case_stack && case_stack->data.case_stmt.start))
    {
      report_case_error (1);
      return;
    }

  index_type = TREE_TYPE (case_stack->data.case_stmt.index_expr);
  nominal_type = case_stack->data.case_stmt.nominal_type;

  /* If the index is erroneous, avoid more problems: pretend to succeed.  */
  if (index_type == error_mark_node)
    return;

  /* Convert VALUE to the type in which the comparisons are nominally done.  */
  if (value != 0)
    value = convert (nominal_type, value);

  /* Fail if this value is out of range for the actual type of the index
     (which may be narrower than NOMINAL_TYPE).  */
  if (value != 0 && ! int_fits_type_p (value, index_type))
    {
      report_case_error (3, index_type, value);
      return;
    }

  /* Fail if this is a duplicate entry.  */
  if (value == 0)
    {
      if (case_stack->data.case_stmt.has_default)
	{
	  report_case_error (2, index_type, NULL_TREE, NULL_TREE);
	  return;
	}
      case_stack->data.case_stmt.has_default = 1;
    }
  else if (case_stack->data.case_stmt.num_ranges)
    {
      for (l = case_stack->data.case_stmt.case_list; l; l = TREE_CHAIN (l))
	{
	  tree elem = TREE_PURPOSE (l);
	  if (! elem)
	    continue;

	  if (TREE_CODE (elem) == INTEGER_CST)
	    {
	      if ((TREE_INT_CST_LOW (value)
		   == TREE_INT_CST_LOW (elem))
		  && (TREE_INT_CST_HIGH (value)
		      == TREE_INT_CST_HIGH (elem)))
		{
		  report_case_error (2, index_type, value, elem);
		  return;
		}
	    }
	  else if (TREE_CODE (elem) == RANGE_EXPR)
	    {
	      if ((TREE_INT_CST_HIGH (value)
		   >= TREE_INT_CST_HIGH (TREE_OPERAND (elem, 0)))
		  && (TREE_INT_CST_HIGH (value)
		      <= TREE_INT_CST_HIGH (TREE_OPERAND (elem, 1)))
		  && (TREE_INT_CST_LOW (value)
		      >= TREE_INT_CST_LOW (TREE_OPERAND (elem, 0)))
		  && (TREE_INT_CST_LOW (value)
		      <= TREE_INT_CST_LOW (TREE_OPERAND (elem, 1))))
		{
		  report_case_error (2, index_type, value, elem);
		  return;
		}
	    }
	  else abort ();
	}
    }
  else
    {
      for (l = case_stack->data.case_stmt.case_list; l; l = TREE_CHAIN (l))
	{
	  if (TREE_PURPOSE (l)
	      && (TREE_INT_CST_LOW (value)
		  == TREE_INT_CST_LOW (TREE_PURPOSE (l)))
	      && (TREE_INT_CST_HIGH (value)
		  == TREE_INT_CST_HIGH (TREE_PURPOSE (l))))
	    {
	      report_case_error (2, index_type, value, TREE_PURPOSE (l));
	      return;
	    }
	}
    }

  /* Add this label to the list, and succeed.
     Copy VALUE so it is temporary rather than momentary.  */
  case_stack->data.case_stmt.case_list
    = tree_cons (value ? copy_node (value) : 0, label,
	         case_stack->data.case_stmt.case_list);
  expand_label (label);
}

void
pushcase_range (value1, value2, label)
     register tree value1, value2;
     register tree label;
{
  register tree l;
  tree index_type;
  tree value;

  /* Fail if not inside a real case statement.  */
  if (! (case_stack && case_stack->data.case_stmt.start))
    {
      report_case_error (1);
      return;
    }

  index_type = TREE_TYPE (case_stack->data.case_stmt.index_expr);

  /* If the index is erroneous, avoid more problems: pretend to succeed.  */
  if (index_type == error_mark_node)
    return;

  /* Make VALUEs unsigned iff the case index expression is.  */
  if (value1 != 0
      && (TREE_UNSIGNED (TREE_TYPE (value1))
	  != TREE_UNSIGNED (index_type)))
    value1 = convert (signed_or_unsigned_type (TREE_UNSIGNED (index_type),
					       TREE_TYPE (value1)),
		     value1);
  if (value2 != 0
      && (TREE_UNSIGNED (TREE_TYPE (value2))
	  != TREE_UNSIGNED (index_type)))
    value2 = convert (signed_or_unsigned_type (TREE_UNSIGNED (index_type),
					       TREE_TYPE (value2)),
		     value2);

  /* Fail if these values are out of range.  */
  if (value1 != 0 && ! int_fits_type_p (value1, index_type))
    {
      report_case_error (3, index_type, value1);
      return;
    }
  if (value2 != 0 && ! int_fits_type_p (value2, index_type))
    {
      report_case_error (3, index_type, value2);
    }

  if (INT_CST_LT (value2, value1))
    {
      report_case_error (4, index_type, value1, value2);
    }

  if (TREE_INT_CST_LOW (value1) == TREE_INT_CST_LOW (value2)
      && TREE_INT_CST_HIGH (value1) == TREE_INT_CST_HIGH (value2))
    {
      pushcase (value1, label);
      return;
    }

  /* Fail if this is a duplicate entry.  For range expressions,
     we make error messages somewhat more helpful.  */
  value = build_nt (RANGE_EXPR, value1, value2);

  for (l = case_stack->data.case_stmt.case_list; l; l = TREE_CHAIN (l))
    {
      tree elem = TREE_PURPOSE (l);

      if (! elem)
	continue;

      if (TREE_CODE (elem) == INTEGER_CST)
	{
	  if ((TREE_INT_CST_HIGH (value1)
	       <= TREE_INT_CST_HIGH (elem))
	      && (TREE_INT_CST_HIGH (value2)
		  >= TREE_INT_CST_HIGH (elem))
	      && (TREE_INT_CST_LOW (value1)
		  <= TREE_INT_CST_LOW (elem))
	      && (TREE_INT_CST_LOW (value2)
		  >= TREE_INT_CST_LOW (elem)))
	    {
	      report_case_error (2, index_type, value, elem);
	      return;
	    }
	}
      else if (TREE_CODE (elem) == RANGE_EXPR)
	{
	  tree r1, r2;
	  tree v1, v2;
	  if (INT_CST_LT (value1, TREE_OPERAND (elem, 0)))
	    {
	      r1 = value;
	      r2 = elem;
	    }
	  else
	    {
	      r1 = elem;
	      r2 = value;
	    }
	  v1 = TREE_OPERAND (r1, 1);
	  v2 = TREE_OPERAND (r2, 0);
	  if (INT_CST_LT (v2, v1))
	    {
	      report_case_error (2, index_type, value, elem);
	      return;
	    }
	}
    }

  /* Add this label to the list, and succeed.
     Copy VALUE so it is temporary rather than momentary.  */
  case_stack->data.case_stmt.case_list
    = tree_cons (value ? copy_node (value) : 0, label,
	         case_stack->data.case_stmt.case_list);
  expand_label (label);

  case_stack->data.case_stmt.num_ranges++;

  return;
}

/* Terminate a case (Pascal) or switch (C) statement
   in which CASE_INDEX is the expression to be tested.
   Generate the code to test it and jump to the right place.  */

void
expand_end_case ()
{
  tree minval, maxval, range;
  rtx default_label = 0;
  register tree elt;
  register tree c;
  int count;
  rtx index;
  rtx table_label = gen_label_rtx ();
  int ncases;
  rtx *labelvec;
  register int i;
  rtx before_case;
  register struct nesting *thiscase = case_stack;
  tree index_expr = thiscase->data.case_stmt.index_expr;

  cleanup_temporary_aggregates ();
  do_pending_stack_adjust ();

  /* This happens for various reasons including invalid data type.  */
  if (index_expr != error_mark_node)
    {
      /* Warn about duplicate tags, etc.
	 C++ does extra work in this function.  */
      /* Get upper and lower bounds of case values.
	 Also convert all the case values to the index expr's data type.  */
      /* Count the elements and track the largest and
	 smallest of them
	 (treating them as signed even if they are not).  */
      if (thiscase->data.case_stmt.has_default == 0)
	pushcase (0, build_decl (LABEL_DECL, NULL_TREE, NULL_TREE));

      before_case = get_last_insn ();

      /* Get upper and lower bounds of case values.
	 Also convert all the case values to the index expr's data type.  */
      count = 0;
      for (c = thiscase->data.case_stmt.case_list; c; c = TREE_CHAIN (c))
	if (elt = TREE_PURPOSE (c))
	  {
	    /* Note that in Pascal it will be possible
	       to have a RANGE_EXPR here as long as both
	       ends of the range are constant.
	       It will be necessary to extend this function
	       to handle them.  */
	    if (TREE_CODE (elt) != INTEGER_CST)
	      abort ();

	    TREE_PURPOSE (c) = elt = convert (TREE_TYPE (index_expr), elt);

	    /* Count the elements and track the largest and
	       smallest of them
	       (treating them as signed even if they are not).  */
	    if (count++ == 0)
	      {
		minval = maxval = elt;
	      }
	    else
	      {
		if (INT_CST_LT (elt, minval))
		  minval = elt;
		if (INT_CST_LT (maxval, elt))
		  maxval = elt;
	      }
	  }
	else
	  default_label = label_rtx (TREE_VALUE (c));

      if (default_label == 0)
	abort ();

      /* Compute span of values.  */
      if (count != 0)
	range = combine (MINUS_EXPR, maxval, minval);

      if (count == 0 || TREE_CODE (TREE_TYPE (index_expr)) == ERROR_MARK)
	{
	  expand_expr (index_expr, const0_rtx, VOIDmode, 0);
	  emit_queue ();
	  emit_jump (default_label);
	}
      /* If range of values is much bigger than number of values,
	 make a sequence of conditional branches instead of a dispatch.
	 If the switch-index is a constant, do it this way
	 because we can optimize it.  */
      else if (TREE_INT_CST_HIGH (range) != 0
#ifdef HAVE_casesi
	       || count < 4
#else
	       /* If machine does not have a case insn that compares the
		  bounds, this means extra overhead for dispatch tables
		  which raises the threshold for using them.  */
	       || count < 5
#endif
	       || (unsigned) (TREE_INT_CST_LOW (range)) > 10 * count
	       || TREE_CODE (index_expr) == INTEGER_CST)
	{
	  index = expand_expr (index_expr, 0, VOIDmode, 0);
	  emit_queue ();

	  index = protect_from_queue (index, 0);
	  if (GET_CODE (index) == MEM)
	    index = copy_to_reg (index);
	  cleanup_temporary_aggregates ();
	  do_pending_stack_adjust ();

	  for (c = thiscase->data.case_stmt.case_list; c; c = TREE_CHAIN (c))
	    {
	      elt = TREE_PURPOSE (c);
	      if (elt && TREE_VALUE (c))
		do_jump_if_equal (expand_expr (elt, 0, VOIDmode, 0), index,
				  label_rtx (TREE_VALUE (c)));
	    }

	  emit_jump (default_label);
	}
      else
	{
#ifdef HAVE_casesi
	  if (TYPE_MODE (TREE_TYPE (index_expr)) == DImode)
	    {
	      index_expr = convert (integer_type_node,
				    build (MINUS_EXPR, TREE_TYPE (index_expr),
					   index_expr, minval));
	      minval = integer_zero_node;
	    }
	  else if (TYPE_MODE (TREE_TYPE (index_expr)) != SImode)
	    index_expr = convert (integer_type_node, index_expr);
	  index = expand_expr (index_expr, 0, VOIDmode, 0);
	  emit_queue ();
	  index = protect_from_queue (index, 0);
	  cleanup_temporary_aggregates ();
	  do_pending_stack_adjust ();

	  emit_jump_insn (gen_casesi (index, expand_expr (minval, 0, VOIDmode, 0),
				      expand_expr (range, 0, VOIDmode, 0),
				      table_label, default_label));
#else
#ifdef HAVE_tablejump
	  index_expr = convert (integer_type_node,
				build (MINUS_EXPR, TREE_TYPE (index_expr),
				       index_expr, minval));
	  index = expand_expr (index_expr, 0, VOIDmode, 0);
	  emit_queue ();
	  index = protect_from_queue (index, 0);
	  cleanup_temporary_aggregates ();
	  do_pending_stack_adjust ();

	  do_tablejump (index,
			gen_rtx (CONST_INT, VOIDmode, TREE_INT_CST_LOW (range)),
			table_label, default_label);
#else
	  lossage;
#endif				/* not HAVE_tablejump */
#endif				/* not HAVE_casesi */

	  /* Get table of labels to jump to, in order of case index.  */

	  ncases = TREE_INT_CST_LOW (range) + 1;
	  labelvec = (rtx *) alloca (ncases * sizeof (rtx));
	  bzero (labelvec, ncases * sizeof (rtx));

	  for (c = thiscase->data.case_stmt.case_list; c; c = TREE_CHAIN (c))
	    if (TREE_VALUE (c) && (elt = TREE_PURPOSE (c)))
	      {
		register int i
		  = TREE_INT_CST_LOW (elt) - TREE_INT_CST_LOW (minval);
		labelvec[i]
		  = gen_rtx (LABEL_REF, Pmode, label_rtx (TREE_VALUE (c)));
	      }

	  /* Fill in the gaps with the default.  */
	  for (i = 0; i < ncases; i++)
	    if (labelvec[i] == 0)
	      labelvec[i] = gen_rtx (LABEL_REF, Pmode, default_label);

	  /* Output the table */
	  emit_label (table_label);

#ifdef CASE_VECTOR_PC_RELATIVE
	  emit_jump_insn (gen_rtx (ADDR_DIFF_VEC, CASE_VECTOR_MODE,
				   gen_rtx (LABEL_REF, Pmode, table_label),
				   gen_rtvec_v (ncases, labelvec)));
#else
	  emit_jump_insn (gen_rtx (ADDR_VEC, CASE_VECTOR_MODE,
				   gen_rtvec_v (ncases, labelvec)));
#endif
	  /* If the case insn drops through the table,
	     after the table we must jump to the default-label.
	     Otherwise record no drop-through after the table.  */
#ifdef CASE_DROPS_THROUGH
	  emit_jump (default_label);
#else
	  emit_barrier ();
#endif
	}

      reorder_insns (NEXT_INSN (before_case), get_last_insn (),
		     thiscase->data.case_stmt.start);
    }
  if (thiscase->exit_label)
    emit_label (thiscase->exit_label);

  POPSTACK (case_stack);
}

/* Generate code to jump to LABEL if OP1 and OP2 are equal.  */
/* ??? This may need an UNSIGNEDP argument to work properly ??? */

void
do_jump_if_equal (op1, op2, label)
     rtx op1, op2, label;
{
  if (GET_CODE (op1) == CONST_INT
      && GET_CODE (op2) == CONST_INT)
    {
      if (INTVAL (op1) == INTVAL (op2))
	emit_jump (label);
    }
  else
    {
      emit_cmp_insn (op1, op2, 0, 0);
      emit_jump_insn (gen_beq (label));
    }
}

/* Allocate fixed slots in the stack frame of the current function.  */

/* Return size needed for stack frame based on slots so far allocated.  */

int
get_frame_size ()
{
#ifdef FRAME_GROWS_DOWNWARD
  return -frame_offset;
#else
  return frame_offset;
#endif
}

/* Allocate a stack slot of SIZE bytes and return a MEM rtx for it
   with machine mode MODE.  */

rtx
assign_stack_local (mode, size)
     enum machine_mode mode;
     int size;
{
  register rtx x, addr;

  frame_pointer_needed = 1;

  /* Make each stack slot a multiple of the main allocation unit.  */
  size = (((size + (BIGGEST_ALIGNMENT / BITS_PER_UNIT) - 1)
	   / (BIGGEST_ALIGNMENT / BITS_PER_UNIT))
	  * (BIGGEST_ALIGNMENT / BITS_PER_UNIT));

#ifdef FRAME_GROWS_DOWNWARD
  frame_offset -= size;
#endif
  addr = gen_rtx (PLUS, Pmode, frame_pointer_rtx,
		  gen_rtx (CONST_INT, VOIDmode, frame_offset));
#ifndef FRAME_GROWS_DOWNWARD
  frame_offset += size;
#endif

  if (! memory_address_p (mode, addr))
    invalid_stack_slot = 1;

  x = gen_rtx (MEM, mode, addr);

  return x;
}

/* Retroactively move an auto variable from a register to a stack slot.
   This is done when an address-reference to the variable is seen.  */

void
put_var_into_stack (decl)
     tree decl;
{
  register rtx reg = DECL_RTL (decl);
  register rtx new;

  /* No need to do anything if decl has no rtx yet
     since in that case caller is setting TREE_ADDRESSABLE
     and a stack slot will be assigned when the rtl is made.  */
  if (reg == 0)
    return;
  if (GET_CODE (reg) != REG)
    return;

  new = parm_stack_loc (reg);
  if (new == 0)
    new = assign_stack_local (GET_MODE (reg), GET_MODE_SIZE (GET_MODE (reg)));

  /* If this is a memory ref that contains aggregate components,
     mark it as such for cse and loop optimize.  */
  reg->in_struct
    = (TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE
       || TREE_CODE (TREE_TYPE (decl)) == RECORD_TYPE
       || TREE_CODE (TREE_TYPE (decl)) == UNION_TYPE);

  XEXP (reg, 0) = XEXP (new, 0);
  PUT_CODE (reg, MEM);
  /* `volatil' bit means one thing for MEMs, another entirely for REGs.  */
  reg->volatil = 0;

  fixup_var_refs (reg);
}

static void
fixup_var_refs (var)
     rtx var;
{
  register rtx insn;

  /* Yes.  Must scan all insns for stack-refs that exceed the limit.  */
  for (insn = get_insns (); insn; )
    {
      rtx next = NEXT_INSN (insn);
      if (GET_CODE (insn) == INSN || GET_CODE (insn) == CALL_INSN
	  || GET_CODE (insn) == JUMP_INSN)
	{
	  /* The insn to load VAR from a home in the arglist
	     is now a no-op.  When we see it, just delete it.  */
	  if (GET_CODE (PATTERN (insn)) == SET
	      && SET_DEST (PATTERN (insn)) == var
	      && rtx_equal_p (SET_SRC (PATTERN (insn)), var))
	    next = delete_insn (insn);
	  else
	    fixup_var_refs_1 (var, PATTERN (insn), insn);
	}
      insn = next;
    }
}

static rtx
fixup_var_refs_1 (var, x, insn)
     register rtx var;
     register rtx x;
     rtx insn;
{
  register int i;
  RTX_CODE code = GET_CODE (x);
  register char *fmt;
  register rtx tem;

  switch (code)
    {
    case MEM:
      if (var == x)
	{
	  x = fixup_stack_1 (x, insn);
	  tem = gen_reg_rtx (GET_MODE (x));
	  emit_insn_before (gen_move_insn (tem, x), insn);
	  return tem;
	}
      break;

    case REG:
    case CC0:
    case PC:
    case CONST_INT:
    case CONST:
    case SYMBOL_REF:
    case LABEL_REF:
    case CONST_DOUBLE:
      return x;

    case SIGN_EXTRACT:
    case ZERO_EXTRACT:
      /* Note that in some cases those types of expressions are altered
	 by optimize_bit_field, and do not survive to get here.  */
    case SUBREG:
      tem = x;
      while (GET_CODE (tem) == SUBREG || GET_CODE (tem) == SIGN_EXTRACT
	     || GET_CODE (tem) == ZERO_EXTRACT)
	tem = XEXP (tem, 0);
      if (tem == var)
	{
	  x = fixup_stack_1 (x, insn);
	  tem = gen_reg_rtx (GET_MODE (x));
	  emit_insn_before (gen_move_insn (tem, x), insn);
	  return tem;
	}
      break;

    case SET:
      /* First do special simplification of bit-field references.  */
      if (GET_CODE (SET_DEST (x)) == SIGN_EXTRACT
	  || GET_CODE (SET_DEST (x)) == ZERO_EXTRACT)
	optimize_bit_field (x, insn, 0);
      if (GET_CODE (SET_SRC (x)) == SIGN_EXTRACT
	  || GET_CODE (SET_SRC (x)) == ZERO_EXTRACT)
	optimize_bit_field (x, insn, 0);

      {
	rtx dest = SET_DEST (x);
	rtx src = SET_SRC (x);
	rtx outerdest = dest;
	rtx outersrc = src;
	int strictflag = GET_CODE (dest) == STRICT_LOW_PART;

	while (GET_CODE (dest) == SUBREG || GET_CODE (dest) == STRICT_LOW_PART
	       || GET_CODE (dest) == SIGN_EXTRACT
	       || GET_CODE (dest) == ZERO_EXTRACT)
	  dest = XEXP (dest, 0);
	while (GET_CODE (src) == SUBREG
	       || GET_CODE (src) == SIGN_EXTRACT
	       || GET_CODE (src) == ZERO_EXTRACT)
	  src = XEXP (src, 0);

	/* If VAR does not appear at the top level of the SET
	   just scan the lower levels of the tree.  */

        if (src != var && dest != var)
	  break;

	/* Clean up (SUBREG:SI (MEM:mode ...) 0)
	   that may appear inside a SIGN_EXTRACT or ZERO_EXTRACT.
	   This was legitimate when the MEM was a REG.  */

	if ((GET_CODE (outerdest) == SIGN_EXTRACT
	     || GET_CODE (outerdest) == ZERO_EXTRACT)
	    && GET_CODE (XEXP (outerdest, 0)) == SUBREG
	    && SUBREG_REG (XEXP (outerdest, 0)) == var)
	  XEXP (outerdest, 0) = fixup_memory_subreg (XEXP (outerdest, 0));

	if ((GET_CODE (outersrc) == SIGN_EXTRACT
	     || GET_CODE (outersrc) == ZERO_EXTRACT)
	    && GET_CODE (XEXP (outersrc, 0)) == SUBREG
	    && SUBREG_REG (XEXP (outersrc, 0)) == var)
	  XEXP (outersrc, 0) = fixup_memory_subreg (XEXP (outersrc, 0));

	/* Make sure a MEM inside a SIGN_EXTRACT has QImode
	   since that's what bit-field insns want.  */

	if ((GET_CODE (outerdest) == SIGN_EXTRACT
	     || GET_CODE (outerdest) == ZERO_EXTRACT)
	    && GET_CODE (XEXP (outerdest, 0)) == MEM
	    && GET_MODE (XEXP (outerdest, 0)) != QImode)
	  {
	    XEXP (outerdest, 0) = copy_rtx (XEXP (outerdest, 0));
	    PUT_MODE (XEXP (outerdest, 0), QImode);
	  }

	if ((GET_CODE (outersrc) == SIGN_EXTRACT
	     || GET_CODE (outersrc) == ZERO_EXTRACT)
	    && GET_CODE (XEXP (outersrc, 0)) == MEM
	    && GET_MODE (XEXP (outersrc, 0)) != QImode)
	  {
	    XEXP (outersrc, 0) = copy_rtx (XEXP (outersrc, 0));
	    PUT_MODE (XEXP (outersrc, 0), QImode);
	  }

	/* STRICT_LOW_PART is a no-op on memory references
	   and it can cause combinations to be unrecognizable,
	   so eliminate it.  */

	if (dest == var && GET_CODE (SET_DEST (x)) == STRICT_LOW_PART)
	  SET_DEST (x) = XEXP (SET_DEST (x), 0);

	/* An insn to copy VAR into or out of a register
	   must be left alone, to avoid an infinite loop here.
	   But do fix up the address of VAR's stack slot if nec.  */

	if (GET_CODE (SET_SRC (x)) == REG || GET_CODE (SET_DEST (x)) == REG)
	  return fixup_stack_1 (x, insn);

	if ((GET_CODE (SET_SRC (x)) == SUBREG
	     && GET_CODE (SUBREG_REG (SET_SRC (x))) == REG)
	    || (GET_CODE (SET_DEST (x)) == SUBREG
		&& GET_CODE (SUBREG_REG (SET_DEST (x))) == REG))
	  return fixup_stack_1 (x, insn);

	/* Otherwise, storing into VAR must be handled specially
	   by storing into a temporary and copying that into VAR
	   with a new insn after this one.  */

	if (dest == var)
	  {
	    rtx temp;
	    rtx fixeddest;
	    tem = SET_DEST (x);
	    if (GET_CODE (tem) == STRICT_LOW_PART)
	      tem = XEXP (tem, 0);
	    temp = gen_reg_rtx (GET_MODE (tem));
	    fixeddest = fixup_stack_1 (SET_DEST (x), insn);
	    emit_insn_after (gen_move_insn (fixeddest, temp), insn);
	    SET_DEST (x) = temp;
	  }
      }
    }

  /* Nothing special about this RTX; fix its operands.  */

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	XEXP (x, i) = fixup_var_refs_1 (var, XEXP (x, i), insn);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < XVECLEN (x, i); j++)
	    XVECEXP (x, i, j)
	      = fixup_var_refs_1 (var, XVECEXP (x, i, j), insn);
	}
    }
  return x;
}

/* Given X, an rtx of the form (SUBREG:m1 (MEM:m2 addr)),
   return an rtx (MEM:m1 newaddr) which is equivalent.  */

static rtx
fixup_memory_subreg (x)
     rtx x;
{
  int offset = SUBREG_WORD (x) * UNITS_PER_WORD;
  rtx addr = XEXP (SUBREG_REG (x), 0);
  enum machine_mode mode = GET_MODE (SUBREG_REG (x));

#ifdef BYTES_BIG_ENDIAN
  offset += (MIN (UNITS_PER_WORD, GET_MODE_SIZE (mode))
	     - MIN (UNITS_PER_WORD, GET_MODE_SIZE (GET_MODE (x))));
#endif
  return change_address (SUBREG_REG (x), mode,
			 plus_constant (addr, offset));
}

#if 0
/* Fix up any references to stack slots that are invalid memory addresses
   because they exceed the maximum range of a displacement.  */

void
fixup_stack_slots ()
{
  register rtx insn;

  /* Did we generate a stack slot that is out of range
     or otherwise has an invalid address?  */
  if (invalid_stack_slot)
    {
      /* Yes.  Must scan all insns for stack-refs that exceed the limit.  */
      for (insn = get_insns (); insn; insn = NEXT_INSN (insn))
	if (GET_CODE (insn) == INSN || GET_CODE (insn) == CALL_INSN
	    || GET_CODE (insn) == JUMP_INSN)
	  fixup_stack_1 (PATTERN (insn), insn);
    }
}
#endif

/* For each memory ref within X, if it refers to a stack slot
   with an out of range displacement, put the address in a temp register
   (emitting new insns before INSN to load these registers)
   and alter the memory ref to use that register.
   Replace each such MEM rtx with a copy, to avoid clobberage.  */

static rtx
fixup_stack_1 (x, insn)
     rtx x;
     rtx insn;
{
  register int i;
  register RTX_CODE code = GET_CODE (x);
  register char *fmt;

  if (code == MEM)
    {
      register rtx ad = XEXP (x, 0);
      /* If we have address of a stack slot but it's not valid
	 (displacement is too large), compute the sum in a register.  */
      if (GET_CODE (ad) == PLUS
	  && XEXP (ad, 0) == frame_pointer_rtx
	  && GET_CODE (XEXP (ad, 1)) == CONST_INT)
	{
	  rtx temp;
	  if (memory_address_p (GET_MODE (x), ad))
	    return x;
	  temp = gen_reg_rtx (GET_MODE (ad));
	  emit_insn_before (gen_move_insn (temp, ad), insn);
	  return change_address (x, VOIDmode, temp);
	}
      return x;
    }

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	XEXP (x, i) = fixup_stack_1 (XEXP (x, i), insn);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < XVECLEN (x, i); j++)
	    XVECEXP (x, i, j) = fixup_stack_1 (XVECEXP (x, i, j), insn);
	}
    }
  return x;
}

/* Optimization: a bit-field instruction whose field
   happens to be a byte or halfword in memory
   can be changed to a move instruction.

   We call here when INSN is an insn to examine or store into a bit-field.
   BODY is the SET-rtx to be altered.

   EQUIV_MEM is the table `reg_equiv_mem' if that is available; else 0.
   (Currently this is called only from stmt.c, and EQUIV_MEM is always 0.)  */

static void
optimize_bit_field (body, insn, equiv_mem)
     rtx body;
     rtx insn;
     rtx *equiv_mem;
{
  register rtx bitfield;
  int destflag;

  if (GET_CODE (SET_DEST (body)) == SIGN_EXTRACT
      || GET_CODE (SET_DEST (body)) == ZERO_EXTRACT)
    bitfield = SET_DEST (body), destflag = 1;
  else
    bitfield = SET_SRC (body), destflag = 0;

  /* First check that the field being stored has constant size and position
     and is in fact a byte or halfword suitably aligned.  */

  if (GET_CODE (XEXP (bitfield, 1)) == CONST_INT
      && GET_CODE (XEXP (bitfield, 2)) == CONST_INT
      && (INTVAL (XEXP (bitfield, 1)) == GET_MODE_BITSIZE (QImode)
	  || INTVAL (XEXP (bitfield, 1)) == GET_MODE_BITSIZE (HImode))
      && INTVAL (XEXP (bitfield, 2)) % INTVAL (XEXP (bitfield, 1)) == 0)
    {
      register rtx memref = 0;

      /* Now check that the contanting word is memory, not a register,
	 and that it is safe to change the machine mode and to
	 add something to the address.  */

      if (GET_CODE (XEXP (bitfield, 0)) == MEM)
	memref = XEXP (bitfield, 0);
      else if (GET_CODE (XEXP (bitfield, 0)) == REG
	       && equiv_mem != 0
	       && (memref = equiv_mem[REGNO (XEXP (bitfield, 0))]) != 0)
	;
      else if (GET_CODE (XEXP (bitfield, 0)) == SUBREG
	       && GET_CODE (SUBREG_REG (XEXP (bitfield, 0))) == MEM)
	memref = SUBREG_REG (XEXP (bitfield, 0));
      else if (GET_CODE (XEXP (bitfield, 0)) == SUBREG
	       && equiv_mem != 0
	       && GET_CODE (SUBREG_REG (XEXP (bitfield, 0))) == REG
	       && (memref = equiv_mem[REGNO (SUBREG_REG (XEXP (bitfield, 0)))]) != 0)
	;

      if (memref
	  && ! mode_dependent_address_p (XEXP (memref, 0))
	  && offsetable_address_p (GET_MODE (bitfield), XEXP (memref, 0)))
	{
	  /* Now adjust the address, first for any subreg'ing
	     that we are now getting rid of,
	     and then for which byte of the word is wanted.  */

	  register int offset
	    = INTVAL (XEXP (bitfield, 2)) / GET_MODE_BITSIZE (QImode);
	  if (GET_CODE (XEXP (bitfield, 0)) == SUBREG)
	    {
	      offset += SUBREG_WORD (XEXP (bitfield, 0)) * UNITS_PER_WORD;
#ifdef BYTES_BIG_ENDIAN
	      offset -= (MIN (UNITS_PER_WORD,
			      GET_MODE_SIZE (GET_MODE (XEXP (bitfield, 0))))
			 - MIN (UNITS_PER_WORD,
				GET_MODE_SIZE (GET_MODE (memref))));
#endif
	    }
	  memref = gen_rtx (MEM,
			    (INTVAL (XEXP (bitfield, 1)) == GET_MODE_BITSIZE (QImode)
			     ? QImode : HImode),
			    XEXP (memref, 0));

	  /* Store this memory reference where
	     we found the bit field reference.  */

	  if (destflag)
	    {
	      SET_DEST (body)
		= adj_offsetable_operand (memref, offset);
	      if (! CONSTANT_ADDRESS_P (SET_SRC (body)))
		{
		  rtx src = SET_SRC (body);
		  while (GET_CODE (src) == SUBREG
			 && SUBREG_WORD (src) == 0)
		    src = SUBREG_REG (src);
		  if (GET_MODE (src) != GET_MODE (memref))
		    src = gen_rtx (SUBREG, GET_MODE (memref),
				   SET_SRC (body), 0);
		  SET_SRC (body) = src;
		}
	      else if (GET_MODE (SET_SRC (body)) != VOIDmode
		       && GET_MODE (SET_SRC (body)) != GET_MODE (memref))
		/* This shouldn't happen because anything that didn't have
		   one of these modes should have got converted explicitly
		   and then referenced through a subreg.
		   This is so because the original bit-field was
		   handled by agg_mode and so its tree structure had
		   the same mode that memref now has.  */
		abort ();
	    }
	  else
	    {
	      rtx newreg = gen_reg_rtx (GET_MODE (SET_DEST (body)));
	      emit_insn_before (gen_extend_insn (newreg, adj_offsetable_operand (memref, offset),
						 GET_MODE (SET_DEST (body)),
						 GET_MODE (memref),
						 GET_CODE (SET_SRC (body)) == ZERO_EXTRACT),
				insn);
	      SET_SRC (body) = newreg;
	    }

	  /* Cause the insn to be re-recognized.  */

	  INSN_CODE (insn) = -1;
	}
    }
}

/* 1 + last pseudo register number used for loading a copy
   of a parameter of this function.  */

static int max_parm_reg;

/* Vector indexed by REGNO, containing location on stack in which
   to put the parm which is nominally in pseudo register REGNO,
   if we discover that that parm must go in the stack.  */
static rtx *parm_reg_stack_loc;

/* Last insn of those whose job was to put parms into their nominal homes.  */
static rtx last_parm_insn;

int
max_parm_reg_num ()
{
  return max_parm_reg;
}

/* Return the first insn following those generated by `assign_parms'.  */

rtx
get_first_nonparm_insn ()
{
  if (last_parm_insn)
    return NEXT_INSN (last_parm_insn);
  return get_insns ();
}

/* Get the stack home of a REG rtx that is one of this function's parameters.
   This is called rather than assign a new stack slot as a local.
   Return 0 if there is no existing stack home suitable for such use.  */

static rtx
parm_stack_loc (reg)
     rtx reg;
{
  if (REGNO (reg) < max_parm_reg)
    return parm_reg_stack_loc[REGNO (reg)];
  return 0;
}

/* Assign RTL expressions to the function's parameters.
   This may involve copying them into registers and using
   those registers as the RTL for them.  */

static void
assign_parms (fndecl)
     tree fndecl;
{
  register tree parm;
  register rtx entry_parm;
  register rtx stack_parm;
  register CUMULATIVE_ARGS args_so_far;
  enum machine_mode passed_mode, nominal_mode;
  /* Total space needed so far for args on the stack,
     given as a constant and a tree-expression.  */
  struct args_size stack_args_size;

  int nparmregs
    = list_length (DECL_ARGUMENTS (fndecl)) + FIRST_PSEUDO_REGISTER;

  /* Nonzero if function takes extra anonymous args.
     This means the last named arg must be on the stack
     right before the anonymous ones.  */
  int vararg
    = (TYPE_ARG_TYPES (TREE_TYPE (fndecl)) != 0
      && (TREE_VALUE (tree_last (TYPE_ARG_TYPES (TREE_TYPE (fndecl))))
	  != void_type_node));

  stack_args_size.constant = 0;
  stack_args_size.var = 0;

  parm_reg_stack_loc = (rtx *) oballoc (nparmregs * sizeof (rtx));
  bzero (parm_reg_stack_loc, nparmregs * sizeof (rtx));

  INIT_CUMULATIVE_ARGS (args_so_far, TREE_TYPE (fndecl));

  for (parm = DECL_ARGUMENTS (fndecl); parm; parm = TREE_CHAIN (parm))
    {
      int aggregate
	= (TREE_CODE (TREE_TYPE (parm)) == ARRAY_TYPE
	   || TREE_CODE (TREE_TYPE (parm)) == RECORD_TYPE
	   || TREE_CODE (TREE_TYPE (parm)) == UNION_TYPE);
      struct args_size stack_offset;
      rtx stack_offset_rtx;

      /* Get this parm's offset as an rtx.  */
      stack_offset = stack_args_size;
      stack_offset.constant += FIRST_PARM_OFFSET;
      stack_offset_rtx = ARGS_SIZE_RTX (stack_offset);

      DECL_OFFSET (parm) = -1;

      if (TREE_TYPE (parm) == error_mark_node)
	{
	  DECL_RTL (parm) = gen_rtx (MEM, BLKmode, const0_rtx);
	  continue;
	}

      /* Find mode of arg as it is passed, and mode of arg
	 as it should be during execution of this function.  */
      passed_mode = TYPE_MODE (DECL_ARG_TYPE (parm));
      nominal_mode = TYPE_MODE (TREE_TYPE (parm));

      /* Determine parm's home in the stack,
	 in case it arrives in the stack or we should pretend it did.  */
      stack_parm
	= gen_rtx (MEM, passed_mode,
		   memory_address (passed_mode,
				   gen_rtx (PLUS, Pmode,
					    arg_pointer_rtx, stack_offset_rtx)));

      /* If this is a memory ref that contains aggregate components,
	 mark it as such for cse and loop optimize.  */
      stack_parm->in_struct = aggregate;

      /* Let machine desc say which reg (if any) the parm arrives in.
	 0 means it arrives on the stack.  */
      entry_parm = 0;
      /* Variable-size args, and args following such, are never in regs.  */
      if (TREE_CODE (TYPE_SIZE (TREE_TYPE (parm))) == INTEGER_CST
	  || stack_offset.var != 0)
	{
#ifdef FUNCTION_INCOMING_ARG
	  entry_parm
	    = FUNCTION_INCOMING_ARG (args_so_far, passed_mode,
				     DECL_ARG_TYPE (parm), 1);
#else
	  entry_parm
	    = FUNCTION_ARG (args_so_far, passed_mode, DECL_ARG_TYPE (parm), 1);
#endif
	}
      /* If this parm was passed part in regs and part in memory,
	 pretend it arrived entirely in memory
	 by pushing the register-part onto the stack.

	 In the special case of a DImode or DFmode that is split,
	 we could put it together in a pseudoreg directly,
	 but for now that's not worth bothering with.  */

      /* If this is the last named arg and anonymous args follow,
	 likewise pretend this arg arrived on the stack
	 so varargs can find the anonymous args following it.  */
      {
	int nregs = 0;
	int i;
#ifdef FUNCTION_ARG_PARTIAL_NREGS
	nregs = FUNCTION_ARG_PARTIAL_NREGS (args_so_far, passed_mode,
					    DECL_ARG_TYPE (parm), 1);
#endif
	if (TREE_CHAIN (parm) == 0 && vararg && entry_parm != 0)
	  nregs = GET_MODE_SIZE (GET_MODE (entry_parm)) / UNITS_PER_WORD;

	if (nregs > 0)
	  current_function_pretend_args_size
	    = (((nregs * UNITS_PER_WORD) + (PARM_BOUNDARY / BITS_PER_UNIT) - 1)
	       / (PARM_BOUNDARY / BITS_PER_UNIT)
	       * (PARM_BOUNDARY / BITS_PER_UNIT));

	i = nregs;
	while (--i >= 0)
	  emit_move_insn (gen_rtx (MEM, SImode,
				   plus_constant (XEXP (stack_parm, 0),
						  i * GET_MODE_SIZE (SImode))),
			  gen_rtx (REG, SImode, REGNO (entry_parm) + i));
	if (nregs > 0)
	  entry_parm = stack_parm;
      }

      if (entry_parm == 0)
	entry_parm = stack_parm;

      /* Now ENTRY_PARM refers to the stack iff this parm uses stack space.
	 In that case, count its size in STACK_ARGS_SIZE.  */

      if (entry_parm == stack_parm)
	{
	  tree sizetree = size_in_bytes (DECL_ARG_TYPE (parm));
	  /* Round the size up to multiple of PARM_BOUNDARY bits.  */
	  tree s1 = convert_units (sizetree, BITS_PER_UNIT, PARM_BOUNDARY);
	  tree s2 = convert_units (s1, PARM_BOUNDARY, BITS_PER_UNIT);
	  /* Add it in.  */
	  ADD_PARM_SIZE (stack_args_size, s2);
	}

      /* Now set STACK_PARM to the place in the stack
	 where this parameter should live during execution,
	 if we discover that it must live in the stack during execution.
	 To make debuggers happier on big-endian machines, we store
	 the value in the last bytes of the space available.  */

      if (nominal_mode != BLKmode && nominal_mode != passed_mode)
	{
#ifdef BYTES_BIG_ENDIAN
	  stack_offset.constant
	    += GET_MODE_SIZE (passed_mode)
	      - GET_MODE_SIZE (nominal_mode);
	  stack_offset_rtx = ARGS_SIZE_RTX (stack_offset);
#endif

	  stack_parm
	    = gen_rtx (MEM, nominal_mode,
		       memory_address (nominal_mode,
				       gen_rtx (PLUS, Pmode,
						arg_pointer_rtx,
						stack_offset_rtx)));

	  /* If this is a memory ref that contains aggregate components,
	     mark it as such for cse and loop optimize.  */
	  stack_parm->in_struct = aggregate;
	}

      /* If the parm arrived in a reg, we don't have an argument
	 stack slot for it, so don't think that we do.
	 If it needs to be addressable, it will get put into the stack
	 like an ordinary automatic variable.  */

      if (GET_CODE (entry_parm) == REG)
	stack_parm = 0;
      else
	DECL_OFFSET (parm) = stack_offset.constant * BITS_PER_UNIT;

#if 0 /* Not needed now that `memory_address' is used above.  */
      /* If either STACK_PARM or ENTRY_PARM is a MEM whose address
	 is not really valid, arrange to correct all references later.  */

      if ((GET_CODE (entry_parm) == MEM
	   && ! memory_address_p (passed_mode, XEXP (entry_parm, 0)))
	  || (stack_parm != entry_parm  && stack_parm != 0
	      && GET_CODE (stack_parm) == MEM
	      && ! memory_address_p (nominal_mode, XEXP (stack_parm, 0))))
	invalid_stack_slot = 1;
#endif /* 0 */

      /* ENTRY_PARM is an RTX for the parameter as it arrives,
	 in the mode in which it arrives.

	 Now output code if necessary to convert it to
	 the type in which this function declares it,
	 and store a reference to that value in DECL_RTL.
	 This reference may be the same as STACK_PARM,
	 or a similar stack reference in a different mode,
	 or may be a pseudo register.  */

      if (nominal_mode == BLKmode)
	{
	  /* If a BLKmode arrives in registers, copy it to a stack slot.  */
	  if (stack_parm == 0)
	    {
	      stack_parm
		= assign_stack_local (GET_MODE (entry_parm),
				      int_size_in_bytes (TREE_TYPE (parm)));

	      move_block_from_reg (REGNO (entry_parm), stack_parm,
				   int_size_in_bytes (TREE_TYPE (parm))
				   / UNITS_PER_WORD);
	    }
	  DECL_RTL (parm) = stack_parm;

	}
      else if (! ((obey_regdecls && ! TREE_REGDECL (parm))
		  /* If -ffloat-store specified, don't put explicit
		     float variables into registers.  */
		  || (flag_float_store
		      && TREE_CODE (TREE_TYPE (parm)) == REAL_TYPE)))
	{
	  /* Store the parm in a pseudoregister during the function.  */
	  register rtx parmreg = gen_reg_rtx (nominal_mode);

	  parmreg->volatil = 1;
	  DECL_RTL (parm) = parmreg;

	  /* Copy the value into the register.  */
	  if (GET_MODE (parmreg) != GET_MODE (entry_parm))
	    convert_move (parmreg, entry_parm, 0);
	  else
	    emit_move_insn (parmreg, entry_parm);

	  /* In any case, record the parm's desired stack location
	     in case we later discover it must live in the stack.  */
	  if (REGNO (parmreg) >= nparmregs)
	    {
	      rtx *new;
	      nparmregs = REGNO (parmreg) + 5;
	      new = (rtx *) oballoc (nparmregs * sizeof (rtx));
	      bcopy (parm_reg_stack_loc, new, nparmregs * sizeof (rtx));
	      parm_reg_stack_loc = new;
	    }
	  parm_reg_stack_loc[REGNO (parmreg)] = stack_parm;

	  /* Mark the register as eliminable if we did no conversion
	     and it was copied from memory at a fixed offset.  */
	  if (nominal_mode == passed_mode
	      && GET_CODE (entry_parm) == MEM
	      && stack_offset.var == 0)
	    REG_NOTES (get_last_insn ()) = gen_rtx (EXPR_LIST, REG_EQUIV,
						    entry_parm, 0);

	  /* For pointer data type, suggest pointer register.  */
	  if (TREE_CODE (TREE_TYPE (parm)) == POINTER_TYPE)
	    mark_reg_pointer (parmreg);
	}
      else
	{
	  /* Value must be stored in the stack slot STACK_PARM
	     during function execution.  */

	  if (passed_mode != nominal_mode)
	    /* Conversion is required.  */
	    entry_parm = convert_to_mode (nominal_mode, entry_parm, 0);

	  if (entry_parm != stack_parm)
	    {
	      if (stack_parm == 0)
		stack_parm = assign_stack_local (GET_MODE (entry_parm),
						 GET_MODE_SIZE (GET_MODE (entry_parm)));
	      emit_move_insn (stack_parm, entry_parm);
	    }

	  DECL_RTL (parm) = stack_parm;
	  frame_pointer_needed = 1;
	}
      
      if (TREE_VOLATILE (parm))
	DECL_RTL (parm)->volatil = 1;
      if (TREE_READONLY (parm))
	DECL_RTL (parm)->unchanging = 1;

      /* Update info on where next arg arrives in registers.  */

      FUNCTION_ARG_ADVANCE (args_so_far, passed_mode, DECL_ARG_TYPE (parm), 1);
    }
  max_parm_reg = max_reg_num ();
  last_parm_insn = get_last_insn ();

  current_function_args_size = stack_args_size.constant;
}

/* Allocation of space for returned structure values.
   During the rtl generation pass, `get_structure_value_addr'
   is called from time to time to request the address of a block in our
   stack frame in which called functions will store the structures
   they are returning.  The same space is used for all of these blocks.  

   We allocate these blocks like stack locals.  We keep reusing
   the same block until a bigger one is needed.  */

/* Length in bytes of largest structure value returned by
   any function called so far in this function.  */
static int max_structure_value_size;

/* An rtx for the addr we are currently using for structure values.
   This is typically (PLUS (REG:SI stackptr) (CONST_INT...)).  */
static rtx structure_value;

rtx
get_structure_value_addr (sizex)
     rtx sizex;
{
  register int size;
  if (GET_CODE (sizex) != CONST_INT)
    abort ();
  size = INTVAL (sizex);

  /* Round up to a multiple of the main allocation unit.  */
  size = (((size + (BIGGEST_ALIGNMENT / BITS_PER_UNIT) - 1)
	   / (BIGGEST_ALIGNMENT / BITS_PER_UNIT))
	  * (BIGGEST_ALIGNMENT / BITS_PER_UNIT));

  /* If this size is bigger than space we know to use,
     get a bigger piece of space.  */
  if (size > max_structure_value_size)
    {
      max_structure_value_size = size;
      structure_value = assign_stack_local (BLKmode, size);
      if (GET_CODE (structure_value) == MEM)
	structure_value = XEXP (structure_value, 0);
    }

  return structure_value;
}

/* Walk the tree of LET_STMTs describing the binding levels within a function
   and warn about uninitialized variables.
   This is done after calling flow_analysis and before global_alloc
   clobbers the pseudo-regs to hard regs.  */

void
uninitialized_vars_warning (block)
     tree block;
{
  register tree decl, sub;
  for (decl = STMT_VARS (block); decl; decl = TREE_CHAIN (decl))
    {
      if (TREE_CODE (decl) == VAR_DECL
	  /* These warnings are unreliable for and aggregates
	     because assigning the fields one by one can fail to convince
	     flow.c that the entire aggregate was initialized.
	     Unions are troublesome because members may be shorter.  */
	  && TREE_CODE (TREE_TYPE (decl)) != RECORD_TYPE
	  && TREE_CODE (TREE_TYPE (decl)) != UNION_TYPE
	  && TREE_CODE (TREE_TYPE (decl)) != ARRAY_TYPE
	  && GET_CODE (DECL_RTL (decl)) == REG
	  && regno_uninitialized (REGNO (DECL_RTL (decl))))
	warning_with_decl (decl,
			   "variable `%s' used uninitialized in this function");
      if (TREE_CODE (decl) == VAR_DECL
	  && GET_CODE (DECL_RTL (decl)) == REG
	  && regno_clobbered_at_setjmp (REGNO (DECL_RTL (decl))))
	warning_with_decl (decl,
			   "variable `%s' may be clobbered by `longjmp'");
    }
  for (sub = STMT_BODY (block); sub; sub = TREE_CHAIN (sub))
    uninitialized_vars_warning (sub);
}

/* Generate RTL for the start of the function FUNC (a FUNCTION_DECL tree node)
   and initialize static variables for generating RTL for the statements
   of the function.  */

void
expand_function_start (subr)
     tree subr;
{
  register int i;
  tree tem;

  this_function = subr;
  cse_not_expected = ! optimize;

  /* We have not yet found a reason why a frame pointer cannot
     be omitted for this function in particular, but maybe we know
     a priori that it is required.
     `flag_omit_frame_pointer' has its main effect here.  */
  frame_pointer_needed = FRAME_POINTER_REQUIRED || ! flag_omit_frame_pointer;

  /* No gotos have been expanded yet.  */
  goto_fixup_chain = 0;

  /* No invalid stack slots have been made yet.  */
  invalid_stack_slot = 0;

  /* Initialize the RTL mechanism.  */
  init_emit (write_symbols);

  /* Initialize the queue of pending postincrement and postdecrements,
     and some other info in expr.c.  */
  init_expr ();

  init_const_rtx_hash_table ();

  /* Decide whether function should try to pop its args on return.  */

  current_function_pops_args = RETURN_POPS_ARGS (TREE_TYPE (subr));

  /* Make the label for return statements to jump to, if this machine
     does not have a one-instruction return.  */
#ifdef FUNCTION_EPILOGUE
  return_label = gen_label_rtx ();
#else
  return_label = 0;
#endif

  /* No space assigned yet for structure values.  */
  max_structure_value_size = 0;
  structure_value = 0;

  /* We are not currently within any block, conditional, loop or case.  */
  block_stack = 0;
  loop_stack = 0;
  case_stack = 0;
  cond_stack = 0;
  nesting_stack = 0;
  nesting_depth = 0;

  /* We have not yet needed to make a label to jump to for tail-recursion.  */
  tail_recursion_label = 0;

  /* No stack slots allocated yet.  */
  frame_offset = STARTING_FRAME_OFFSET;

  init_pending_stack_adjust ();
  clear_current_args_size ();

  /* Prevent ever trying to delete the first instruction of a function.
     Also tell final how to output a linenum before the function prologue.  */
  emit_note (DECL_SOURCE_FILE (subr), DECL_SOURCE_LINE (subr));
  /* Make sure first insn is a note even if we don't want linenums.
     This makes sure the first insn will never be deleted.
     Also, final expects a note to appear there.  */
  emit_note (0, NOTE_INSN_DELETED);

  /* Initialize rtx for parameters and local variables.
     In some cases this requires emitting insns.  */

  assign_parms (subr);

  /* If doing stupid allocation, mark parms as born here.  */

  if (obey_regdecls)
    for (i = FIRST_PSEUDO_REGISTER; i < max_parm_reg; i++)
      emit_insn (gen_rtx (USE, VOIDmode, regno_reg_rtx[i]));

  /* After the parm initializations is where the tail-recursion label
     should go, if we end up needing one.  */
  tail_recursion_reentry = get_last_insn ();

  /* Initialize rtx used to return the value.  */

  if (DECL_MODE (DECL_RESULT (subr)) == BLKmode)
    {
      /* Returning something that won't go in a register.  */
      register rtx value_address;

      /* Expect to be passed the address of a place to store the value.  */
      value_address = gen_reg_rtx (Pmode);
      emit_move_insn (value_address, struct_value_incoming_rtx);
      DECL_RTL (DECL_RESULT (subr))
	= gen_rtx (MEM, DECL_MODE (DECL_RESULT (subr)),
		   value_address);
    }
  else
#ifdef FUNCTION_OUTGOING_VALUE
    DECL_RTL (DECL_RESULT (subr))
      = FUNCTION_OUTGOING_VALUE (TREE_TYPE (DECL_RESULT (subr)), subr);
#else
    DECL_RTL (DECL_RESULT (subr))
      = FUNCTION_VALUE (TREE_TYPE (DECL_RESULT (subr)), subr);
#endif
}

/* Generate RTL for the end of the current function.  */

void
expand_function_end ()
{
  register int i;

  /* Fix up any gotos that jumped out to the outermost
     binding level of the function.  */
  fixup_gotos (0, get_insns ());

  /* If doing stupid register allocation,
     mark register parms as dying here.  */

  if (obey_regdecls)
    for (i = FIRST_PSEUDO_REGISTER; i < max_parm_reg; i++)
      emit_insn (gen_rtx (USE, VOIDmode, regno_reg_rtx[i]));

  clear_pending_stack_adjust ();
  do_pending_stack_adjust ();

  /* Mark the end of the function body.
     If control reaches this insn, the function can drop through
     without returning a value.  */
  emit_note (0, NOTE_INSN_FUNCTION_END);

  /* If we require a true epilogue,
     put here the label that return statements jump to.
     If there will be no epilogue, write a return instruction.  */
#ifdef FUNCTION_EPILOGUE
  emit_label (return_label);
#else
  emit_jump_insn (gen_return ());
#endif
}

/* Handle C++ scoping rules.  The rules are this: when an aggregate
   on the stack comes into scope, if it uses any virtual functions,
   its virtual function table must be initialized.  Also, any constructors
   that may be called on it, or on its base members must also be called.
   This function marks the point in the insn list where these calls
   must be made.

   CALL_CTOR is non-zero if a constructor should be called to
   suitably deposit INIT into DECL.  */
void
expand_aggr_birth (decl, init, call_ctor)
     tree decl;
     tree init;
{
  extern tree current_function_decl;

  tree type = TREE_TYPE (decl);
  int top_level = current_function_decl == NULL_TREE;

  /* External aggregates are not initialized here.  */
  if (TREE_EXTERNAL (decl))
    return;

  if (top_level == 0)
    {
      emit_note (DECL_SOURCE_FILE (decl), DECL_SOURCE_LINE (decl));
      if (TREE_NEEDS_CONSTRUCTOR (type))
	{
	  if (call_ctor)
	    do_aggr_init (decl, init);
	  else
	    {
	      if (init && comptypes (TREE_TYPE (decl), TREE_TYPE (init), 0))
		expand_assignment (decl, DECL_INITIAL (decl), 1, 0);
	    }

	  /* If there is no initializer, then make it
	     possible to distinguish whether this
	     aggregate is subsequently used.  */
	  if (init == NULL_TREE)
	    TREE_EVERUSED (decl) = 0;
	}
      else if (init)
	{
	  if (TREE_CODE (decl) != VAR_DECL)
	    abort ();

	  if (TREE_STATIC (decl))
	    abort ();

	  /* We should not have an initializer for an object of variable size.  */
	  if (DECL_VOFFSET (decl)
	      || DECL_SIZE (decl) == 0
	      || !TREE_LITERAL (DECL_SIZE (decl)))
	    abort ();

	  do_aggr_init (decl, init);
	}
      else if (TREE_VIRTUAL (type))
	do_virtual_init (decl);

      emit_queue ();
    }

  /* There is no need to hold on to aggregates that do not
     have destructors if we are not at the top level.  */
  if (top_level)
    {
      if (TREE_NEEDS_CONSTRUCTOR (type)
	  || TREE_NEEDS_DESTRUCTOR (type))
	global_aggregates = tree_cons (decl, init, global_aggregates);
    }
  else if (TREE_NEEDS_DESTRUCTOR (type)
#if 0
	   && call_ctor
#endif
	   )
    block_stack->data.block.aggregates
      = tree_cons (decl, init, block_stack->data.block.aggregates);
}

/* When an aggregate goes out of scope, all currently active
   aggregates in scope must have their destructors called.  Note that
   Since these aggregates are put on in reverse order, the destructors
   are calling the the proper order without needing to reverse
   them.  This is only called from within function scope.  */
void
cleanup_aggregates ()
{
  struct nesting *block = block_stack;
  tree vars = block->data.block.aggregates;
  block->data.block.aggregates = 0;

  while (vars)
    {
      tree decl = TREE_PURPOSE (vars);
      if (TREE_EVERUSED (decl) == 0)
	{
	  warning ("aggregate `%s' initialized, but never used",
		   IDENTIFIER_POINTER (DECL_NAME (decl)));
	  /* Muzzle further error messages in case this type does not
	     rank a destructor.  */
	  TREE_EVERUSED (decl) = 1;
	}
      if (TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE)
	{
	  expand_vec_delete (TYPE_NELTS (TREE_TYPE (decl)),
			     decl, integer_zero_node);
	}
      else
	expand_delete (TREE_TYPE (decl), decl, integer_zero_node);
      vars = TREE_CHAIN (vars);
    }
}

/* Same as above, except handle the case of a return statement
   which kill all aggregates in the current and all higher scopes
   up to the edge of the function body.

   BUT_THIS_ONE is one aggregate we cannot delete: it is being returned
   by this function.  */
void
cleanup_all_aggregates (but_this_one)
     tree but_this_one;
{
  struct nesting *block = block_stack;

  while (block)
    {
      tree vars = block->data.block.aggregates;

      while (vars)
	{
	  tree decl = TREE_PURPOSE (vars);
	  if (decl != but_this_one)
	    {
	      if (TREE_EVERUSED (decl) == 0)
		warning ("aggregate `%s' initialized, but never used on this execution path",
			 IDENTIFIER_POINTER (DECL_NAME (decl)));
	      if (TREE_CODE (TREE_TYPE (decl)) == ARRAY_TYPE)
		{
		  expand_vec_delete (TYPE_NELTS (TREE_TYPE (decl)),
				     decl, integer_zero_node);
		}
	      else
		expand_delete (TREE_TYPE (decl), decl, integer_zero_node);
	    }
	  vars = TREE_CHAIN (vars);
	}
      block = block->next;
    }
}

/* A predicate to say whether any aggregate values are in
   scope at the present time.  THIS_BLOCK_ONLY nonzero means only for
   the current binding level, otherwise must check all binding levels
   of the current function.  BUT_THIS_ONE is something we are returning.
   It does not need cleaning.  */
int
any_aggregates_need_cleaning_p (this_block_only, but_this_one)
     int this_block_only;
     tree but_this_one;
{
  register struct nesting *block = block_stack;

  while (block)
    {
      tree vars = block->data.block.aggregates;

      if (vars &&
	  (TREE_PURPOSE (vars) != but_this_one || TREE_CHAIN (vars)))
	return 1;
      if (this_block_only)
	return 0;
      block = block->next;
    }
  return 0;
}

/* Return a list of the aggregates defined at the current
   block level.  If we are outside of a function, this is
   `global_aggregates'.  */
tree
getaggrs ()
{
  if (block_stack)
    return block_stack->data.block.aggregates;
  return global_aggregates;
}
