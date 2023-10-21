/* Data flow analysis for GNU compiler.
   Copyright (C) 1987, 1988 Free Software Foundation, Inc.

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


/* This file contains the data flow analysis pass of the compiler.
   It computes data flow information
   which tells combine_instructions which insns to consider combining
   and controls register allocation.

   Additional data flow information that is too bulky to record
   is generated during the analysis, and is used at that time to
   create autoincrement and autodecrement addressing.

   The first step is dividing the function into basic blocks.
   find_basic_blocks does this.  Then life_analysis determines
   where each register is live and where it is dead.

   ** find_basic_blocks **

   find_basic_blocks divides the current function's rtl
   into basic blocks.  It records the beginnings and ends of the
   basic blocks in the vectors basic_block_head and basic_block_end,
   and the number of blocks in n_basic_blocks.

   find_basic_blocks also finds any unreachable loops
   and deletes them.

   ** life_analysis **

   life_analysis is called immediately after find_basic_blocks.
   It uses the basic block information to determine where each
   hard or pseudo register is live.

   ** live-register info **

   The information about where each register is live is in two parts:
   the REG_NOTES of insns, and the vector basic_block_live_at_start.

   basic_block_live_at_start has an element for each basic block,
   and the element is a bit-vector with a bit for each hard or pseudo
   register.  The bit is 1 if the register is live at the beginning
   of the basic block.

   To each insn's REG_NOTES is added an element for each register
   that is live before the insn or set by the insn, but is dead
   after the insn.

   To determine which registers are live after any insn, one can
   start from the beginning of the basic block and scan insns, noting
   which registers are set by each insn and which die there.

   ** Other actions of life_analysis **

   life_analysis sets up the LOG_LINKS fields of insns because the
   information needed to do so is readily available.

   life_analysis deletes insns whose only effect is to store a value
   that is never used.

   life_analysis notices cases where a reference to a register as
   a memory address can be combined with a preceding or following
   incrementation or decrementation of the register.  The separate
   instruction to increment or decrement is deleted and the address
   is changed to a POST_INC or similar rtx.

   Each time an incrementing or decrementing address is created,
   a REG_INC element is added to the insn's REG_NOTES list.

   life_analysis fills in certain vectors containing information about
   register usage: reg_n_refs, reg_n_deaths, reg_n_sets,
   reg_live_length, reg_crosses_call and reg_basic_block.  */

#include <stdio.h>
#include "config.h"
#include "rtl.h"
#include "basic-block.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "flags.h"

/* Get the basic block number of an insn.
   This info should not be expected to remain available
   after the end of life_analysis.  */

#define BLOCK_NUM(INSN)  uid_block_number[INSN_UID (INSN)]

/* This is where the BLOCK_NUM values are really stored.
   This is set up by find_basic_blocks and used there and in life_analysis,
   and then freed.  */

static short *uid_block_number;

/* INSN_VOLATILE (insn) is 1 if the insn refers to anything volatile.  */

#define INSN_VOLATILE(INSN) uid_volatile[INSN_UID (INSN)]
static char *uid_volatile;

/* Number of basic blocks in the current function.  */

int n_basic_blocks;

/* Maximum register number used in this function, plus one.  */

int max_regno;

/* Indexed by n, gives number of basic block that  (REG n) is used in.
   Or gives -2 if (REG n) is used in more than one basic block.
   Or -1 if it has not yet been seen so no basic block is known.
   This information remains valid for the rest of the compilation
   of the current function; it is used to control register allocation.  */

short *reg_basic_block;

/* Indexed by n, gives number of times (REG n) is used or set, each
   weighted by its loop-depth.
   This information remains valid for the rest of the compilation
   of the current function; it is used to control register allocation.  */

short *reg_n_refs;

/* Indexed by n, gives number of times (REG n) is set.
   This information remains valid for the rest of the compilation
   of the current function; it is used to control register allocation.  */

short *reg_n_sets;

/* Indexed by N, gives number of places register N dies.
   This information remains valid for the rest of the compilation
   of the current function; it is used to control register allocation.  */

short *reg_n_deaths;

/* Indexed by N, gives 1 if that reg is live across any CALL_INSNs.
   This information remains valid for the rest of the compilation
   of the current function; it is used to control register allocation.  */

char *reg_crosses_call;

/* Total number of instructions at which (REG n) is live.
   The larger this is, the less priority (REG n) gets for
   allocation in a real register.
   This information remains valid for the rest of the compilation
   of the current function; it is used to control register allocation.

   local-alloc.c may alter this number to change the priority.

   Negative values are special.
   -1 is used to mark a pseudo reg which has a constant or memory equivalent
   and is used infrequently enough that it should not get a hard register.
   -2 is used to mark a pseudo reg for a parameter, when a frame pointer
   is not required.  global-alloc.c makes an allocno for this but does
   not try to assign a hard register to it.  */

int *reg_live_length;

/* Element N is the next insn that uses (hard or pseudo) register number N
   within the current basic block; or zero, if there is no such insn.
   This is valid only during the final backward scan in propagate_block.  */

static rtx *reg_next_use;

/* Size of a regset for the current function,
   in (1) bytes and (2) elements.  */

int regset_bytes;
int regset_size;

/* Element N is first insn in basic block N.
   This info lasts until we finish compiling the function.  */

rtx *basic_block_head;

/* Element N is last insn in basic block N.
   This info lasts until we finish compiling the function.  */

rtx *basic_block_end;

/* Element N is a regset describing the registers live
   at the start of basic block N.
   This info lasts until we finish compiling the function.  */

regset *basic_block_live_at_start;

/* Regset of regs live when calls to `setjmp'-like functions happen.  */

regset regs_live_at_setjmp;

/* Element N is nonzero if control can drop into basic block N
   from the preceding basic block.  Freed after life_analysis.  */

static char *basic_block_drops_in;

/* Element N is depth within loops of basic block number N.
   Freed after life_analysis.  */

static short *basic_block_loop_depth;

/* Element N nonzero if basic block N can actually be reached.
   Vector exists only during find_basic_blocks.  */

static char *block_live_static;

/* Depth within loops of basic block being scanned for lifetime analysis,
   plus one.  This is the weight attached to references to registers.  */

static int loop_depth;

/* Define AUTO_INC_DEC if machine has any kind of incrementing
   or decrementing addressing.  */

#ifdef HAVE_PRE_DECREMENT
#define AUTO_INC_DEC
#endif

#ifdef HAVE_PRE_INCREMENT
#define AUTO_INC_DEC
#endif

#ifdef HAVE_POST_DECREMENT
#define AUTO_INC_DEC
#endif

#ifdef HAVE_POST_INCREMENT
#define AUTO_INC_DEC
#endif

/* Forward declarations */
static void find_basic_blocks ();
static void life_analysis ();
static void mark_label_ref ();
void allocate_for_life_analysis (); /* Used also in stupid_life_analysis */
static void init_regset_vector ();
static void propagate_block ();
static void mark_set_regs ();
static void mark_used_regs ();
static int insn_dead_p ();
static int try_pre_increment ();
static int try_pre_increment_1 ();
static rtx find_use_as_address ();
static int volatile_refs_p ();
void dump_flow_info ();

/* Find basic blocks of the current function and perform data flow analysis.
   F is the first insn of the function and NREGS the number of register numbers
   in use.  */

void
flow_analysis (f, nregs, file)
     rtx f;
     int nregs;
     FILE *file;
{
  register rtx insn;
  register int i;
  register int max_uid = 0;

  /* Count the basic blocks.  Also find maximum insn uid value used.  */

  {
    register RTX_CODE prev_code = JUMP_INSN;
    register RTX_CODE code;

    for (insn = f, i = 0; insn; insn = NEXT_INSN (insn))
      {
	code = GET_CODE (insn);
	if (INSN_UID (insn) > max_uid)
	  max_uid = INSN_UID (insn);
	if (code == CODE_LABEL
	    || (prev_code != INSN && prev_code != CALL_INSN
		&& prev_code != CODE_LABEL
		&& (code == INSN || code == CALL_INSN || code == JUMP_INSN)))
	  i++;
	if (code != NOTE)
	  prev_code = code;
      }
  }

  /* Allocate some tables that last till end of compiling this function
     and some needed only in find_basic_blocks and life_analysis.  */

  n_basic_blocks = i;
  basic_block_head = (rtx *) oballoc (n_basic_blocks * sizeof (rtx));
  basic_block_end = (rtx *) oballoc (n_basic_blocks * sizeof (rtx));
  basic_block_drops_in = (char *) alloca (n_basic_blocks);
  basic_block_loop_depth = (short *) alloca (n_basic_blocks * sizeof (short));
  uid_block_number = (short *) alloca ((max_uid + 1) * sizeof (short));
  uid_volatile = (char *) alloca (max_uid + 1);
  bzero (uid_volatile, max_uid + 1);

  find_basic_blocks (f);
  life_analysis (f, nregs);
  if (file)
    dump_flow_info (file);

  basic_block_drops_in = 0;
  uid_block_number = 0;
  basic_block_loop_depth = 0;
}

/* Find all basic blocks of the function whose first insn is F.
   Store the correct data in the tables that describe the basic blocks,
   set up the chains of references for each CODE_LABEL, and
   delete any entire basic blocks that cannot be reached.  */

static void
find_basic_blocks (f)
     rtx f;
{
  register rtx insn;
  register int i;

  /* Initialize the ref chain of each label to 0.  */
  /* Record where all the blocks start and end and their depth in loops.  */
  /* For each insn, record the block it is in.  */

  {
    register RTX_CODE prev_code = JUMP_INSN;
    register RTX_CODE code;
    int depth = 1;

    for (insn = f, i = -1; insn; insn = NEXT_INSN (insn))
      {
	code = GET_CODE (insn);
	if (code == NOTE)
	  {
	    if (NOTE_LINE_NUMBER (insn) == NOTE_INSN_LOOP_BEG)
	      depth++;
	    else if (NOTE_LINE_NUMBER (insn) == NOTE_INSN_LOOP_END)
	      depth--;
	  }
	else if (code == CODE_LABEL
		 || (prev_code != INSN && prev_code != CALL_INSN
		     && prev_code != CODE_LABEL
		     && (code == INSN || code == CALL_INSN || code == JUMP_INSN)))
	  {
	    basic_block_head[++i] = insn;
	    basic_block_end[i] = insn;
	    basic_block_loop_depth[i] = depth;
	    if (code == CODE_LABEL)
	      LABEL_REFS (insn) = insn;
	  }
	else if (code == INSN || code == CALL_INSN || code == JUMP_INSN)
	  basic_block_end[i] = insn;
	BLOCK_NUM (insn) = i;
	if (code != NOTE)
	  prev_code = code;
      }
  }

  /* Record which basic blocks control can drop in to.  */

  {
    register int i;
    for (i = 0; i < n_basic_blocks; i++)
      {
	register rtx insn = PREV_INSN (basic_block_head[i]);
	/* TEMP1 is used to avoid a bug in Sequent's compiler.  */
	register int temp1;
	while (insn && GET_CODE (insn) == NOTE)
	  insn = PREV_INSN (insn);
	temp1 = insn && GET_CODE (insn) != BARRIER;
	basic_block_drops_in[i] = temp1;
      }
  }

  /* Now find which basic blocks can actually be reached
     and put all jump insns' LABEL_REFS onto the ref-chains
     of their target labels.  */

  if (n_basic_blocks > 0)
    {
      register char *block_live = (char *) alloca (n_basic_blocks);
      register char *block_marked = (char *) alloca (n_basic_blocks);
      int something_marked = 1;

      /* Initialize with just block 0 reachable and no blocks marked.  */

      bzero (block_live, n_basic_blocks);
      bzero (block_marked, n_basic_blocks);
      block_live[0] = 1;
      block_live_static = block_live;

      /* Pass over all blocks, marking each block that is reachable
	 and has not yet been marked.
	 Keep doing this until, in one pass, no blocks have been marked.
	 Then blocks_live and blocks_marked are identical and correct.
	 In addition, all jumps actually reachable have been marked.  */

      while (something_marked)
	{
	  something_marked = 0;
	  for (i = 0; i < n_basic_blocks; i++)
	    if (block_live[i] && !block_marked[i])
	      {
		block_marked[i] = 1;
		something_marked = 1;
		if (i + 1 < n_basic_blocks && basic_block_drops_in[i + 1])
		  block_live[i + 1] = 1;
		insn = basic_block_end[i];
		if (GET_CODE (insn) == JUMP_INSN)
		  mark_label_ref (PATTERN (insn), insn, 0);
	      }
	}

      /* Now delete the code for any basic blocks that can't be reached.
	 They can occur because jump_optimize does not recognize
	 unreachable loops as unreachable.  */

      for (i = 0; i < n_basic_blocks; i++)
	if (!block_live[i])
	  {
	    insn = basic_block_head[i];
	    while (1)
	      {
		if (GET_CODE (insn) == BARRIER)
		  abort ();
		if (GET_CODE (insn) != NOTE)
		  {
		    PUT_CODE (insn, NOTE);
		    NOTE_LINE_NUMBER (insn) = NOTE_INSN_DELETED;
		    NOTE_SOURCE_FILE (insn) = 0;
		  }
		if (insn == basic_block_end[i])
		  {
		    /* BARRIERs are between basic blocks, not part of one.
		       Delete a BARRIER if the preceding jump is deleted.
		       We cannot alter a BARRIER into a NOTE
		       because it is too short; but we can really delete
		       it because it is not part of a basic block.  */
		    if (NEXT_INSN (insn) != 0
			&& GET_CODE (NEXT_INSN (insn)) == BARRIER)
		      delete_insn (NEXT_INSN (insn));
		    break;
		  }
		insn = NEXT_INSN (insn);
	      }
	    /* Each time we delete some basic blocks,
	       see if there is a jump around them that is
	       being turned into a no-op.  If so, delete it.  */

	    if (block_live[i - 1])
	      {
		register int j;
		for (j = i; j < n_basic_blocks; j++)
		  if (block_live[j])
		    {
		      insn = basic_block_end[i - 1];
		      if (GET_CODE (insn) == JUMP_INSN
			  && JUMP_LABEL (insn) != 0
			  && INSN_UID (JUMP_LABEL (insn)) != 0
			  && BLOCK_NUM (JUMP_LABEL (insn)) == j)
			{
			  PUT_CODE (insn, NOTE);
			  NOTE_LINE_NUMBER (insn) = NOTE_INSN_DELETED;
			  NOTE_SOURCE_FILE (insn) = 0;
			  if (GET_CODE (NEXT_INSN (insn)) != BARRIER)
			    abort ();
			  delete_insn (NEXT_INSN (insn));
			}
		      break;
		    }
	      }
	  }
    }
}

/* Check expression X for label references;
   if one is found, add INSN to the label's chain of references.

   CHECKDUP means check for and avoid creating duplicate references
   from the same insn.  Such duplicates do no serious harm but
   can slow life analysis.  CHECKDUP is set only when duplicates
   are likely.  */

static void
mark_label_ref (x, insn, checkdup)
     rtx x, insn;
     int checkdup;
{
  register RTX_CODE code = GET_CODE (x);
  register int i;
  register char *fmt;

  if (code == LABEL_REF)
    {
      register rtx label = XEXP (x, 0);
      register rtx y;
      if (GET_CODE (label) != CODE_LABEL)
	return;
      /* If the label was never emitted, this insn is junk,
	 but avoid a crash trying to refer to BLOCK_NUM (label).
	 This can happen as a result of a syntax error
	 and a diagnostic has already been printed.  */
      if (INSN_UID (label) == 0)
	return;
      CONTAINING_INSN (x) = insn;
      /* if CHECKDUP is set, check for duplicate ref from same insn
	 and don't insert.  */
      if (checkdup)
	for (y = LABEL_REFS (label); y != label; y = LABEL_NEXTREF (y))
	  if (CONTAINING_INSN (y) == insn)
	    return;
      LABEL_NEXTREF (x) = LABEL_REFS (label);
      LABEL_REFS (label) = x;
      block_live_static[BLOCK_NUM (label)] = 1;
      return;
    }

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	mark_label_ref (XEXP (x, i), insn, 0);
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = 0; j < XVECLEN (x, i); j++)
	    mark_label_ref (XVECEXP (x, i, j), insn, 1);
	}
    }
}

/* Determine the which registers are live at the start of each
   basic block of the function whose first insn is F.
   NREGS is the number of registers used in F.
   We allocate the vector basic_block_live_at_start
   and the regsets that it points to, and fill them with the data.
   regset_size and regset_bytes are also set here.  */

static void
life_analysis (f, nregs)
     rtx f;
     int nregs;
{
  register regset tem;
  int first_pass;
  int changed;
  /* For each basic block, a bitmask of regs
     live on exit from the block.  */
  regset *basic_block_live_at_end;
  /* For each basic block, a bitmask of regs
     live on entry to a successor-block of this block.
     If this does not match basic_block_live_at_end,
     that must be updated, and the block must be rescanned.  */
  regset *basic_block_new_live_at_end;
  /* For each basic block, a bitmask of regs
     whose liveness at the end of the basic block
     can make a difference in which regs are live on entry to the block.
     These are the regs that are set within the basic block,
     possibly excluding those that are used after they are set.  */
  regset *basic_block_significant;
  register int i;
  rtx insn;

  max_regno = nregs;

  bzero (regs_ever_live, sizeof regs_ever_live);

  /* Allocate and zero out many data structures
     that will record the data from lifetime analysis.  */

  allocate_for_life_analysis ();

  reg_next_use = (rtx *) alloca (nregs * sizeof (rtx));
  bzero (reg_next_use, nregs * sizeof (rtx));

  /* Set up several regset-vectors used internally within this function.
     Their meanings are documented above, with their declarations.  */

  basic_block_live_at_end = (regset *) alloca (n_basic_blocks * sizeof (regset));
  tem = (regset) alloca (n_basic_blocks * regset_bytes);
  bzero (tem, n_basic_blocks * regset_bytes);
  init_regset_vector (basic_block_live_at_end, tem, n_basic_blocks, regset_bytes);

  basic_block_new_live_at_end = (regset *) alloca (n_basic_blocks * sizeof (regset));
  tem = (regset) alloca (n_basic_blocks * regset_bytes);
  bzero (tem, n_basic_blocks * regset_bytes);
  init_regset_vector (basic_block_new_live_at_end, tem, n_basic_blocks, regset_bytes);

  basic_block_significant = (regset *) alloca (n_basic_blocks * sizeof (regset));
  tem = (regset) alloca (n_basic_blocks * regset_bytes);
  bzero (tem, n_basic_blocks * regset_bytes);
  init_regset_vector (basic_block_significant, tem, n_basic_blocks, regset_bytes);

  /* Record which insns refer to any volatile memory.  */

  for (insn = f; insn; insn = NEXT_INSN (insn))
    if (GET_CODE (insn) == INSN || GET_CODE (insn) == JUMP_INSN
	|| GET_CODE (insn) == CALL_INSN)
      INSN_VOLATILE (insn) = volatile_refs_p (PATTERN (insn));

  if (n_basic_blocks > 0)
#ifdef EXIT_IGNORE_STACK
    if (! (EXIT_IGNORE_STACK) || ! frame_pointer_needed)
#endif
      {
	/* If exiting needs the right stack value,
	   consider the stack pointer live at the end of the function.  */
	basic_block_live_at_end[n_basic_blocks - 1]
	  [STACK_POINTER_REGNUM / REGSET_ELT_BITS]
	    |= 1 << (STACK_POINTER_REGNUM % REGSET_ELT_BITS);
	basic_block_new_live_at_end[n_basic_blocks - 1]
	  [STACK_POINTER_REGNUM / REGSET_ELT_BITS]
	    |= 1 << (STACK_POINTER_REGNUM % REGSET_ELT_BITS);
      }

  /* Propagate life info through the basic blocks
     around the graph of basic blocks.

     This is a relaxation process: each time a new register
     is live at the end of the basic block, we must scan the block
     to determine which registers are, as a consequence, live at the beginning
     of that block.  These registers must then be marked live at the ends
     of all the blocks that can transfer control to that block.
     The process continues until it reaches a fixed point.  */

  first_pass = 1;
  changed = 1;
  while (changed)
    {
      changed = 0;
      for (i = n_basic_blocks - 1; i >= 0; i--)
	{
	  int consider = first_pass;
	  int must_rescan = first_pass;
	  register int j;

	  /* Set CONSIDER if this block needs thinking about at all
	     (that is, if the regs live now at the end of it
	     are not the same as were live at the end of it when
	     we last thought about it).
	     Set must_rescan if it needs to be thought about
	     instruction by instruction (that is, if any additional
	     reg that is live at the end now but was not live there before
	     is one of the significant regs of this basic block).  */

	  for (j = 0; j < regset_size; j++)
	    {
	      register int x = basic_block_new_live_at_end[i][j]
		      & ~basic_block_live_at_end[i][j];
	      if (x)
		consider = 1;
	      if (x & basic_block_significant[i][j])
		{
		  must_rescan = 1;
		  consider = 1;
		  break;
		}
	    }

	  if (! consider)
	    continue;

	  /* The live_at_start of this block may be changing,
	     so another pass will be required after this one.  */
	  changed = 1;

	  if (! must_rescan)
	    {
	      /* No complete rescan needed;
		 just record those variables newly known live at end
		 as live at start as well.  */
	      for (j = 0; j < regset_size; j++)
		{
		  register int x = basic_block_new_live_at_end[i][j]
			& ~basic_block_live_at_end[i][j];
		  basic_block_live_at_start[i][j] |= x;
		  basic_block_live_at_end[i][j] |= x;
		}
	    }
	  else
	    {
	      /* Update the basic_block_live_at_start
		 by propagation backwards through the block.  */
	      bcopy (basic_block_new_live_at_end[i],
		     basic_block_live_at_end[i], regset_bytes);
	      bcopy (basic_block_live_at_end[i],
		     basic_block_live_at_start[i], regset_bytes);
	      propagate_block (basic_block_live_at_start[i],
			       basic_block_head[i], basic_block_end[i], 0,
			       first_pass ? basic_block_significant[i] : 0,
			       i);
	    }

	  {
	    register rtx jump, head;
	    /* Update the basic_block_new_live_at_end's of the block
	       that falls through into this one (if any).  */
	    head = basic_block_head[i];
	    jump = PREV_INSN (head);
	    if (basic_block_drops_in[i])
	      {
		register from_block = BLOCK_NUM (jump);
		register int j;
		for (j = 0; j < regset_size; j++)
		  basic_block_new_live_at_end[from_block][j]
		    |= basic_block_live_at_start[i][j];
	      }
	    /* Update the basic_block_new_live_at_end's of
	       all the blocks that jump to this one.  */
	    if (GET_CODE (head) == CODE_LABEL)
	      for (jump = LABEL_REFS (head);
		   jump != head;
		   jump = LABEL_NEXTREF (jump))
		{
		  register from_block = BLOCK_NUM (CONTAINING_INSN (jump));
		  register int j;
		  for (j = 0; j < regset_size; j++)
		    basic_block_new_live_at_end[from_block][j]
		      |= basic_block_live_at_start[i][j];
		}
	  }
	}
      first_pass = 0;
    }

  /* Process the regs live at the beginning of the function.
     Mark them as not local to any one basic block.  */

  if (n_basic_blocks > 0)
    for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
      if (basic_block_live_at_start[0][i / REGSET_ELT_BITS]
	  & (1 << (i % REGSET_ELT_BITS)))
	reg_basic_block[i] = -2;

  /* Now the life information is accurate.
     Make one more pass over each basic block
     to delete dead stores, create autoincrement addressing
     and record how many times each register is used, is set, or dies.

     To save time, we operate directly in basic_block_live_at_end[i],
     thus destroying it (in fact, converting it into a copy of
     basic_block_live_at_start[i]).  This is ok now because
     basic_block_live_at_end[i] is no longer used past this point.  */

  for (i = 0; i < n_basic_blocks; i++)
    {
      propagate_block (basic_block_live_at_end[i],
		       basic_block_head[i], basic_block_end[i], 1, 0, i);
    }
}

/* Subroutines of life analysis.  */

/* Allocate the permanent data structures that represent the results
   of life analysis.  Not static since used also for stupid life analysis.  */

void
allocate_for_life_analysis ()
{
  register int i;
  register regset tem;

  regset_size = ((max_regno + REGSET_ELT_BITS - 1) / REGSET_ELT_BITS);
  regset_bytes = regset_size * sizeof (*(regset)0);

  reg_n_refs = (short *) oballoc (max_regno * sizeof (short));
  bzero (reg_n_refs, max_regno * sizeof (short));

  reg_n_sets = (short *) oballoc (max_regno * sizeof (short));
  bzero (reg_n_sets, max_regno * sizeof (short));

  reg_n_deaths = (short *) oballoc (max_regno * sizeof (short));
  bzero (reg_n_deaths, max_regno * sizeof (short));

  reg_live_length = (int *) oballoc (max_regno * sizeof (int));
  bzero (reg_live_length, max_regno * sizeof (int));

  reg_crosses_call = (char *) oballoc (max_regno);
  bzero (reg_crosses_call, max_regno);

  reg_basic_block = (short *) oballoc (max_regno * sizeof (short));
  for (i = 0; i < max_regno; i++)
    reg_basic_block[i] = -1;

  basic_block_live_at_start = (regset *) oballoc (n_basic_blocks * sizeof (regset));
  tem = (regset) oballoc (n_basic_blocks * regset_bytes);
  bzero (tem, n_basic_blocks * regset_bytes);
  init_regset_vector (basic_block_live_at_start, tem, n_basic_blocks, regset_bytes);

  regs_live_at_setjmp = (regset) oballoc (regset_bytes);
  bzero (regs_live_at_setjmp, regset_bytes);
}

/* Make each element of VECTOR point at a regset,
   taking the space for all those regsets from SPACE.
   SPACE is of type regset, but it is really as long as NELTS regsets.
   BYTES_PER_ELT is the number of bytes in one regset.  */

static void
init_regset_vector (vector, space, nelts, bytes_per_elt)
     regset *vector;
     regset space;
     int nelts;
     int bytes_per_elt;
{
  register int i;
  register regset p = space;

  for (i = 0; i < nelts; i++)
    {
      vector[i] = p;
      p += bytes_per_elt / sizeof (*p);
    }
}

/* Compute the registers live at the beginning of a basic block
   from those live at the end.

   When called, OLD contains those live at the end.
   On return, it contains those live at the beginning.
   FIRST and LAST are the first and last insns of the basic block.

   FINAL is nonzero if we are doing the final pass which is not
   for computing the life info (since that has already been done)
   but for acting on it.  On this pass, we delete dead stores,
   set up the logical links and dead-variables lists of instructions,
   and merge instructions for autoincrement and autodecrement addresses.

   SIGNIFICANT is nonzero only the first time for each basic block.
   If it is nonzero, it points to a regset in which we store
   a 1 for each register that is set within the block.

   BNUM is the number of the basic block.  */

static void
propagate_block (old, first, last, final, significant, bnum)
     register regset old;
     rtx first;
     rtx last;
     int final;
     regset significant;
     int bnum;
{
  register rtx insn;
  rtx prev;
  regset live;
  regset dead;

  /* The following variables are used only if FINAL is nonzero.  */
  /* This vector gets one element for each reg that has been live
     at any point in the basic block that has been scanned so far.
     SOMETIMES_MAX says how many elements are in use so far.
     In each element, OFFSET is the byte-number within a regset
     for the register described by the element, and BIT is a mask
     for that register's bit within the byte.  */
  register struct foo { short offset; short bit; } *regs_sometimes_live;
  int sometimes_max = 0;
  /* This regset has 1 for each reg that we have seen live so far.
     It and REGS_SOMETIMES_LIVE are updated together.  */
  regset maxlive;

  loop_depth = basic_block_loop_depth[bnum];

  dead = (regset) alloca (regset_bytes);
  live = (regset) alloca (regset_bytes);

  if (final)
    {
      register int i, offset, bit;

      maxlive = (regset) alloca (regset_bytes);
      bcopy (old, maxlive, regset_bytes);
      regs_sometimes_live
	= (struct foo *) alloca (max_regno * sizeof (struct foo));

      /* Process the regs live at the end of the block.
	 Enter them in MAXLIVE and REGS_SOMETIMES_LIVE.
	 Also mark them as not local to any one basic block.  */

      for (offset = 0, i = 0; offset < regset_size; offset++)
	for (bit = 1; bit; bit <<= 1, i++)
	  {
	    if (i == max_regno)
	      break;
	    if (old[offset] & bit)
	      {
		reg_basic_block[i] = -2;
		regs_sometimes_live[sometimes_max].offset = offset;
		regs_sometimes_live[sometimes_max].bit = i % REGSET_ELT_BITS;
		sometimes_max++;
	      }
	  }
    }

  /* Scan the block an insn at a time from end to beginning.  */

  for (insn = last; ; insn = prev)
    {
      prev = PREV_INSN (insn);

      /* If this is a call to `setjmp' et al,
	 warn if any non-volatile datum is live.  */

      if (final && GET_CODE (insn) == NOTE
	  && NOTE_LINE_NUMBER (insn) == NOTE_INSN_SETJMP)
	{
	  int i;
	  for (i = 0; i < regset_size; i++)
	    regs_live_at_setjmp[i] |= old[i];
	}

      /* Update the life-status of regs for this insn.
	 First DEAD gets which regs are set in this insn
	 then LIVE gets which regs are used in this insn.
	 Then the regs live before the insn
	 are those live after, with DEAD regs turned off,
	 and then LIVE regs turned on.  */

      if (GET_CODE (insn) == INSN
	  || GET_CODE (insn) == JUMP_INSN
	  || GET_CODE (insn) == CALL_INSN)
	{
	  register int i;
	  rtx note = find_reg_note (insn, REG_RETVAL, 0);

	  /* If an instruction consists of just dead store(s) on final pass,
	     "delete" it by turning it into a NOTE of type NOTE_INSN_DELETED.
	     We could really delete it with delete_insn, but that
	     can cause trouble for first or last insn in a basic block.  */
	  if (final && insn_dead_p (PATTERN (insn), old, 1)
	      /* Don't delete something that refers to volatile storage!  */
	      && ! INSN_VOLATILE (insn))
	    {
	      PUT_CODE (insn, NOTE);
	      NOTE_LINE_NUMBER (insn) = NOTE_INSN_DELETED;
	      NOTE_SOURCE_FILE (insn) = 0;
	      /* If this insn is copying the return value from a library call,
		 delete the entire library call.  */
	      if (note)
		{
		  rtx first = XEXP (note, 0);
		  rtx prev = insn;
		  while (first->volatil)
		    first = NEXT_INSN (first);
		  while (prev != first)
		    {
		      prev = PREV_INSN (prev);
		      PUT_CODE (prev, NOTE);
		      NOTE_LINE_NUMBER (prev) = NOTE_INSN_DELETED;
		      NOTE_SOURCE_FILE (prev) = 0;
		    }
		}
	      goto flushed;
	    }

	  for (i = 0; i < regset_size; i++)
	    {
	      dead[i] = 0;	/* Faster than bzero here */
	      live[i] = 0;	/* since regset_size is usually small */
	    }

	  /* See if this is an increment or decrement that can be
	     merged into a following memory address.  */
#ifdef AUTO_INC_DEC
	  {
	    register rtx x = PATTERN (insn);
	    /* Does this instruction increment or decrement a register?  */
	    if (final && GET_CODE (x) == SET
		&& GET_CODE (SET_DEST (x)) == REG
		&& (GET_CODE (SET_SRC (x)) == PLUS
		    || GET_CODE (SET_SRC (x)) == MINUS)
		&& XEXP (SET_SRC (x), 0) == SET_DEST (x)
		&& GET_CODE (XEXP (SET_SRC (x), 1)) == CONST_INT
		/* Ok, look for a following memory ref we can combine with.
		   If one is found, change the memory ref to a PRE_INC
		   or PRE_DEC, cancel this insn, and return 1.
		   Return 0 if nothing has been done.  */
		&& try_pre_increment_1 (insn))
	      goto flushed;
	  }
#endif /* AUTO_INC_DEC */

	  /* If this is not the final pass, and this insn is copying the
	     value of a library call and it's dead, don't scan the
	     insns that perform the library call, so that the call's
	     arguments are not marked live.  */
	  if (note && insn_dead_p (PATTERN (insn), old, 1))
	    insn = XEXP (note, 0);
	  else
	    {
	      /* LIVE gets the regs used in INSN; DEAD gets those set by it.  */
	      mark_set_regs (old, dead, PATTERN (insn), final ? insn : 0,
			     significant);
	      mark_used_regs (old, live, PATTERN (insn), final, insn);

	      /* Update OLD for the registers used or set.  */
	      for (i = 0; i < regset_size; i++)
		{
		  old[i] &= ~dead[i];
		  old[i] |= live[i];
		}

	      if (GET_CODE (insn) == CALL_INSN)
		{
		  register int i;

		  /* Each call clobbers all call-clobbered regs.
		     Note that the function-value reg is one of these, and
		     mark_set_regs has already had a chance to handle it.  */
		  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
		    if (call_used_regs[i])
		      old[i / REGSET_ELT_BITS]
			&= ~(1 << (i % REGSET_ELT_BITS));

		  /* The stack ptr is used (honorarily) by a CALL insn.  */
		  old[STACK_POINTER_REGNUM / REGSET_ELT_BITS]
		    |= (1 << (STACK_POINTER_REGNUM % REGSET_ELT_BITS));

		  if (final)
		    {
		      /* Any regs live at the time of a call instruction
			 must not go in a register clobbered by calls.
			 Find all regs now live and record this for them.  */

		      register struct foo *p = regs_sometimes_live;

		      for (i = 0; i < sometimes_max; i++, p++)
			{
			  if (old[p->offset]
			      & (1 << p->bit))
			    reg_crosses_call[p->offset * REGSET_ELT_BITS + p->bit] = 1;
			}
		    }
		}
	    }

	  /* On final pass, add any additional sometimes-live regs
	     into MAXLIVE and REGS_SOMETIMES_LIVE.
	     Also update counts of how many insns each reg is live at.  */

	  if (final)
	    {
	      for (i = 0; i < regset_size; i++)
		{
		  register int diff = live[i] & ~maxlive[i];

		  if (diff)
		    {
		      register int regno;
		      maxlive[i] |= diff;
		      for (regno = 0; diff && regno < REGSET_ELT_BITS; regno++)
			if (diff & (1 << regno))
			  {
			    regs_sometimes_live[sometimes_max].offset = i;
			    regs_sometimes_live[sometimes_max].bit = regno;
			    diff &= ~ (1 << regno);
			    sometimes_max++;
			  }
		    }
		}

	      {
		register struct foo *p = regs_sometimes_live;
		for (i = 0; i < sometimes_max; i++, p++)
		  {
		    if (old[p->offset] & (1 << p->bit))
		      reg_live_length[p->offset * REGSET_ELT_BITS + p->bit]++;
		  }
	      }
	    }
	}
    flushed: ;
      if (insn == first)
	break;
    }
}

/* Return 1 if X (the body of an insn, or part of it) is just dead stores
   (SET expressions whose destinations are registers dead after the insn).
   NEEDED is the regset that says which regs are alive after the insn.  */

static int
insn_dead_p (x, needed, strict_low_ok)
     rtx x;
     regset needed;
     int strict_low_ok;
{
  register RTX_CODE code = GET_CODE (x);
#if 0
  /* Make sure insns to set the stack pointer are never deleted.  */
  needed[STACK_POINTER_REGNUM / REGSET_ELT_BITS]
    |= 1 << (STACK_POINTER_REGNUM % REGSET_ELT_BITS);
#endif

  /* If setting something that's a reg or part of one,
     see if that register's altered value will be live.  */

  if (code == SET)
    {
      register rtx r = SET_DEST (x);
      /* A SET that is a subroutine call cannot be dead.  */
      if (GET_CODE (SET_SRC (x)) == CALL)
	return 0;
      while (GET_CODE (r) == SUBREG
	     || (strict_low_ok && GET_CODE (r) == STRICT_LOW_PART)
	     || GET_CODE (r) == ZERO_EXTRACT
	     || GET_CODE (r) == SIGN_EXTRACT)
	r = SUBREG_REG (r);
      if (GET_CODE (r) == REG)
	{
	  register int regno = REGNO (r);
	  register int offset = regno / REGSET_ELT_BITS;
	  register int bit = 1 << (regno % REGSET_ELT_BITS);
	  return (needed[offset] & bit) == 0;
	}
    }
  /* If performing several activities,
     insn is dead if each activity is individually dead.
     Also, CLOBBERs and USEs can be ignored; a CLOBBER or USE
     that's inside a PARALLEL doesn't make the insn worth keeping.  */
  else if (code == PARALLEL)
    {
      register int i = XVECLEN (x, 0);
      for (i--; i >= 0; i--)
	{
	  rtx elt = XVECEXP (x, 0, i);
	  if (!insn_dead_p (elt, needed, strict_low_ok)
	      && GET_CODE (elt) != CLOBBER
	      && GET_CODE (elt) != USE)
	    return 0;
	}
      return 1;
    }
  /* We do not check CLOBBER or USE here.
     An insn consisting of just a CLOBBER or just a USE
     should not be deleted.  */
  return 0;
}

/* Return 1 if register REGNO was used before it was set.
   In other words, if it is live at function entry.  */

int
regno_uninitialized (regno)
     int regno;
{
  return (basic_block_live_at_start[0][regno / REGSET_ELT_BITS]
	  & (1 << (regno % REGSET_ELT_BITS)));
}

/* 1 if register REGNO was alive at a place where `setjmp' was called
   and was set more than once.  Such regs may be clobbered by `longjmp'.  */

int
regno_clobbered_at_setjmp (regno)
     int regno;
{
  return (reg_n_sets[regno] > 1
	  && (regs_live_at_setjmp[regno / REGSET_ELT_BITS]
	      & (1 << (regno % REGSET_ELT_BITS))));
}

/* Process the registers that are set within X.
   Their bits are set to 1 in the regset DEAD,
   because they are dead prior to this insn.

   If INSN is nonzero, it is the insn being processed
   and the fact that it is nonzero implies this is the FINAL pass
   in propagate_block.  In this case, various info about register
   usage is stored, LOG_LINKS fields of insns are set up.  */

static void mark_set_1 ();

static void
mark_set_regs (needed, dead, x, insn, significant)
     regset needed;
     regset dead;
     rtx x;
     rtx insn;
     regset significant;
{
  register RTX_CODE code = GET_CODE (x);

  if (code == SET || code == CLOBBER)
    mark_set_1 (needed, dead, x, insn, significant);
  else if (code == PARALLEL)
    {
      register int i;
      for (i = XVECLEN (x, 0) - 1; i >= 0; i--)
	{
	  code = GET_CODE (XVECEXP (x, 0, i));
	  if (code == SET || code == CLOBBER)
	    mark_set_1 (needed, dead, XVECEXP (x, 0, i), insn, significant);
	}
    }
}

/* Process a single SET rtx, X.  */

static void
mark_set_1 (needed, dead, x, insn, significant)
     regset needed;
     regset dead;
     rtx x;
     rtx insn;
     regset significant;
{
  register int regno;
  register rtx reg = SET_DEST (x);

  if (reg == 0)
    return;

  if (GET_CODE (reg) == SUBREG)
    {
      /* Modifying just one hardware register
	 of a multi-register value does not count as "setting"
	 for live-dead analysis.  Parts of the previous value
	 might still be significant below this insn.  */
      if (REG_SIZE (SUBREG_REG (reg)) > REG_SIZE (reg))
	return;

      reg = SUBREG_REG (reg);
    }

  if (GET_CODE (reg) == REG
      && (regno = REGNO (reg), regno != FRAME_POINTER_REGNUM)
      && regno != ARG_POINTER_REGNUM)
    /* && regno != STACK_POINTER_REGNUM) -- let's try without this.  */
    {
      register int offset = regno / REGSET_ELT_BITS;
      register int bit = 1 << (regno % REGSET_ELT_BITS);
      int is_needed = 0;

      /* Mark the reg being set as dead before this insn.  */
      dead[offset] |= bit;
      /* Mark it as a significant register for this basic block.  */
      if (significant)
	significant[offset] |= bit;
      /* A hard reg in a wide mode may really be multiple registers.
	 If so, mark all of them just like the first.  */
      if (regno < FIRST_PSEUDO_REGISTER)
	{
	  int n = HARD_REGNO_NREGS (regno, GET_MODE (reg));
	  while (--n > 0)
	    {
	      dead[(regno + n) / REGSET_ELT_BITS]
		|= 1 << ((regno + n) % REGSET_ELT_BITS);
	      if (significant)
		significant[(regno + n) / REGSET_ELT_BITS]
		  |= 1 << ((regno + n) % REGSET_ELT_BITS);
	      is_needed |= (needed[(regno + n) / REGSET_ELT_BITS]
			    & 1 << ((regno + n) % REGSET_ELT_BITS));
	    }
	}
      /* Additional data to record if this is the final pass.  */
      if (insn)
	{
	  register rtx y = reg_next_use[regno];
	  register int blocknum = BLOCK_NUM (insn);

	  /* If this is a hard reg, record this function uses the reg.  */

	  if (regno < FIRST_PSEUDO_REGISTER)
	    {
	      register int i;
	      i = HARD_REGNO_NREGS (regno, GET_MODE (reg));
	      do
		regs_ever_live[regno + --i] = 1;
	      while (i > 0);
	    }

	  /* Keep track of which basic blocks each reg appears in.  */

	  if (reg_basic_block[regno] == -1)
	    reg_basic_block[regno] = blocknum;
	  else if (reg_basic_block[regno] != blocknum)
	    reg_basic_block[regno] = -2;

	  /* Count (weighted) references, stores, etc.  */
	  reg_n_refs[regno] += loop_depth;
	  reg_n_sets[regno]++;
	  /* The next use is no longer "next", since a store intervenes.  */
	  reg_next_use[regno] = 0;
	  /* The insns where a reg is live are normally counted elsewhere,
	     but we want the count to include the insn where the reg is set,
	     and the normal counting mechanism would not count it.  */
	  reg_live_length[regno]++;
	  if ((needed[offset] & bit) || is_needed)
	    {
	      /* Make a logical link from the next following insn
		 that uses this register, back to this insn.
		 The following insns have already been processed.  */
	      if (y && (BLOCK_NUM (y) == blocknum))
		LOG_LINKS (y)
		  = gen_rtx (INSN_LIST, VOIDmode, insn, LOG_LINKS (y));
	    }
	  else
	    {
	      /* Note that dead stores have already been deleted when possible
		 If we get here, we have found a dead store that cannot
		 be eliminated (because the same insn does something useful).
		 Indicate this by marking the reg being set as dying here.  */
	      REG_NOTES (insn)
		= gen_rtx (EXPR_LIST, REG_DEAD,
			   reg, REG_NOTES (insn));
	    }
	}
    }
}

/* Scan expression X and store a 1-bit in LIVE for each reg it uses.
   This is done assuming the registers needed from X
   are those that have 1-bits in NEEDED.

   On the final pass, FINAL is 1.  This means try for autoincrement
   and count the uses and deaths of each pseudo-reg.

   INSN is the containing instruction.  */

static void
mark_used_regs (needed, live, x, final, insn)
     regset needed;
     regset live;
     rtx x;
     rtx insn;
     int final;
{
  register RTX_CODE code;
  register int regno;

 retry:
  code = GET_CODE (x);
  switch (code)
    {
    case LABEL_REF:
    case SYMBOL_REF:
    case CONST_INT:
    case CONST:
    case CC0:
    case PC:
    case CLOBBER:
      return;

#if defined (HAVE_POST_INCREMENT) || defined (HAVE_POST_DECREMENT)
    case MEM:
      /* Here we detect use of an index register which might
	 be good for postincrement or postdecrement.  */
      if (final)
	{
	  rtx addr = XEXP (x, 0);
	  register int size = GET_MODE_SIZE (GET_MODE (x));

	  if (GET_CODE (addr) == REG)
	    {
	      register rtx y;
	      regno = REGNO (addr);
	      /* Is the next use an increment that might make auto-increment? */
	      y = reg_next_use[regno];
	      if (y && GET_CODE (PATTERN (y)) == SET
		  && BLOCK_NUM (y) == BLOCK_NUM (insn)
		  /* Can't add side effects to jumps; if reg is spilled and
		     reloaded, there's no way to store back the altered value.  */
		  && GET_CODE (insn) != JUMP_INSN
		  && (y = SET_SRC (PATTERN (y)),
		      (0
#ifdef HAVE_POST_INCREMENT
		       || GET_CODE (y) == PLUS
#endif
#ifdef HAVE_POST_DECREMENT
		       || GET_CODE (y) == MINUS
#endif
		       )
		      && XEXP (y, 0) == addr
		      && GET_CODE (XEXP (y, 1)) == CONST_INT
		      && INTVAL (XEXP (y, 1)) == size)
		  && dead_or_set_p (reg_next_use[regno], addr))
		{
		  rtx use = find_use_as_address (PATTERN (insn), addr, 0);

		  /* Make sure this register appears only once in this insn.  */
		  if (use != 0 && use != (rtx) 1)
		    {
		      /* We have found a suitable auto-increment:
			 do POST_INC around the register here,
			 and patch out the increment instruction that follows. */
		      XEXP (x, 0)
			= gen_rtx (GET_CODE (y) == PLUS ? POST_INC : POST_DEC,
				   Pmode, addr);
		      /* Record that this insn has an implicit side effect.  */
		      REG_NOTES (insn)
			= gen_rtx (EXPR_LIST, REG_INC, addr, REG_NOTES (insn));

		      /* Modify the old increment-insn to simply copy
			 the already-incremented value of our register.  */
		      y = reg_next_use[regno];
		      SET_SRC (PATTERN (y)) = addr;

		      /* If that makes it a no-op (copying the register
			 into itself) then change it to a simpler no-op
			 so it won't appear to be a "use" and a "set"
			 of this register.  */
		      if (SET_DEST (PATTERN (y)) == addr)
			PATTERN (y) = gen_rtx (USE, VOIDmode, const0_rtx);

		      /* Count an extra reference to the reg for the increment.
			 When a reg is incremented.
			 spilling it is worse, so we want to make that
			 less likely.  */
		      reg_n_refs[regno] += loop_depth;
		      /* Count the increment as a setting of the register,
			 even though it isn't a SET in rtl.  */
		      reg_n_sets[regno]++;
		    }
		}
	    }
	}
      break;
#endif /* HAVE_POST_INCREMENT or HAVE_POST_DECREMENT */

    case REG:
      /* See a register other than being set
	 => mark it as needed.  */

      regno = REGNO (x);
      if (regno != FRAME_POINTER_REGNUM
	  && regno != ARG_POINTER_REGNUM)
	/* && regno != STACK_POINTER_REGNUM) -- let's try without this.  */
	{
	  register int offset = regno / REGSET_ELT_BITS;
	  register int bit = 1 << (regno % REGSET_ELT_BITS);
	  int is_needed = 0;

	  live[offset] |= bit;
	  /* A hard reg in a wide mode may really be multiple registers.
	     If so, mark all of them just like the first.  */
	  if (regno < FIRST_PSEUDO_REGISTER)
	    {
	      int n = HARD_REGNO_NREGS (regno, GET_MODE (x));
	      while (--n > 0)
		{
		  live[(regno + n) / REGSET_ELT_BITS]
		    |= 1 << ((regno + n) % REGSET_ELT_BITS);
		  is_needed |= (needed[(regno + n) / REGSET_ELT_BITS]
				& 1 << ((regno + n) % REGSET_ELT_BITS));
		}
	    }
	  if (final)
	    {
	      register int blocknum = BLOCK_NUM (insn);

	      /* If a hard reg is being used,
		 record that this function does use it.  */

	      if (regno < FIRST_PSEUDO_REGISTER)
		{
		  register int i;
		  i = HARD_REGNO_NREGS (regno, GET_MODE (x));
		  do
		    regs_ever_live[regno + --i] = 1;
		  while (i > 0);
		}

	      /* Keep track of which basic block each reg appears in.  */

	      if (reg_basic_block[regno] == -1)
		reg_basic_block[regno] = blocknum;
	      else if (reg_basic_block[regno] != blocknum)
		reg_basic_block[regno] = -2;

	      /* Record where each reg is used
		 so when the reg is set we know the next insn that uses it.  */

	      reg_next_use[regno] = insn;

	      /* Count (weighted) number of uses of each reg.  */

	      reg_n_refs[regno] += loop_depth;

	      /* Record and count the insns in which a reg dies.
		 If it is used in this insn and was dead below the insn
		 then it dies in this insn.  */

	      if (!(needed[offset] & bit) && !is_needed
		  && ! find_regno_note (insn, REG_DEAD, regno))
		{
		  REG_NOTES (insn)
		    = gen_rtx (EXPR_LIST, REG_DEAD, x, REG_NOTES (insn));
		  reg_n_deaths[regno]++;
		}
	    }
	}
      return;

    case SET:
      {
	register rtx testreg = SET_DEST (x);
	int mark_dest = 0;

	/* Storing in STRICT_LOW_PART is like storing in a reg
	   in that this SET might be dead, so ignore it in TESTREG.
	   but in some other ways it is like using the reg.  */
	/* Storing in a SUBREG or a bit field is like storing the entire
	   register in that if the register's value is not used
	   then this SET is not needed.  */
	while (GET_CODE (testreg) == STRICT_LOW_PART
	       || GET_CODE (testreg) == ZERO_EXTRACT
	       || GET_CODE (testreg) == SIGN_EXTRACT
	       || GET_CODE (testreg) == SUBREG)
	  {
	    /* Modifying a single register in an alternate mode
	       does not use any of the old value.  But these other
	       ways of storing in a register do use the old value.  */
	    if (GET_CODE (testreg) == SUBREG
		&& !(REG_SIZE (SUBREG_REG (testreg)) > REG_SIZE (testreg)))
	      ;
	    else
	      mark_dest = 1;

	    testreg = XEXP (testreg, 0);
	  }

	/* If this is a store into a register,
	   recursively scan the only value being stored,
	   and only if the register's value is live after this insn.
	   If the value being computed here would never be used
	   then the values it uses don't need to be computed either.  */

	if (GET_CODE (testreg) == REG
	    && (regno = REGNO (testreg), regno != FRAME_POINTER_REGNUM)
	    && regno != ARG_POINTER_REGNUM)
	  /*  && regno != STACK_POINTER_REGNUM) -- let's try without this.  */
	  {
	    register int offset = regno / REGSET_ELT_BITS;
	    register int bit = 1 << (regno % REGSET_ELT_BITS);
	    if ((needed[offset] & bit)
		/* If insn refers to volatile, we mustn't delete it,
		   so its inputs are all needed.  */
		|| INSN_VOLATILE (insn))
	      {
		mark_used_regs (needed, live, SET_SRC (x), final, insn);
		if (mark_dest)
		  mark_used_regs (needed, live, SET_DEST (x), final, insn);
	      }
	    return;
	  }
      }
      break;
    }

  /* Recursively scan the operands of this expression.  */

  {
    register char *fmt = GET_RTX_FORMAT (code);
    register int i;
    
    for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
      {
	if (fmt[i] == 'e')
	  {
	    /* Tail recursive case: save a function call level.  */
	    if (i == 0)
	      {
		x = XEXP (x, 0);
		goto retry;
	      }
	    mark_used_regs (needed, live, XEXP (x, i), final, insn);
	  }
	if (fmt[i] == 'E')
	  {
	    register int j;
	    for (j = 0; j < XVECLEN (x, i); j++)
	      mark_used_regs (needed, live, XVECEXP (x, i, j), final, insn);
	  }
      }
  }
}

/* Nonzero if X contains any volatile memory references
   or volatile ASM_OPERANDS expressions.  */

static int
volatile_refs_p (x)
     rtx x;
{
  register RTX_CODE code;

  code = GET_CODE (x);
  switch (code)
    {
    case LABEL_REF:
    case SYMBOL_REF:
    case CONST_INT:
    case CONST:
    case CC0:
    case PC:
    case CLOBBER:
    case REG:
      return 0;

    case CALL:
      return 1;

    case MEM:
    case ASM_OPERANDS:
      if (x->volatil)
	return 1;
    }

  /* Recursively scan the operands of this expression.  */

  {
    register char *fmt = GET_RTX_FORMAT (code);
    register int i;
    
    for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
      {
	if (fmt[i] == 'e')
	  {
	    if (volatile_refs_p (XEXP (x, i)))
	      return 1;
	  }
	if (fmt[i] == 'E')
	  {
	    register int j;
	    for (j = 0; j < XVECLEN (x, i); j++)
	      if (volatile_refs_p (XVECEXP (x, i, j)))
		return 1;
	  }
      }
  }
  return 0;
}

#ifdef AUTO_INC_DEC

static int
try_pre_increment_1 (insn)
     rtx insn;
{
  /* Find the next use of this reg.  If in same basic block,
     make it do pre-increment or pre-decrement if appropriate.  */
  rtx x = PATTERN (insn);
  int amount = ((GET_CODE (SET_SRC (x)) == PLUS ? 1 : -1)
		* INTVAL (XEXP (SET_SRC (x), 1)));
  int regno = REGNO (SET_DEST (x));
  rtx y = reg_next_use[regno];
  if (y != 0
      && BLOCK_NUM (y) == BLOCK_NUM (insn)
      && try_pre_increment (y, SET_DEST (PATTERN (insn)),
			    amount))
    {
      /* We have found a suitable auto-increment
	 and already changed insn Y to do it.
	 So flush this increment-instruction.  */
      PUT_CODE (insn, NOTE);
      NOTE_LINE_NUMBER (insn) = NOTE_INSN_DELETED;
      NOTE_SOURCE_FILE (insn) = 0;
      /* Count a reference to this reg for the increment
	 insn we are deleting.  When a reg is incremented.
	 spilling it is worse, so we want to make that
	 less likely.  */
      reg_n_refs[regno] += loop_depth;
      reg_n_sets[regno]++;
      return 1;
    }
  return 0;
}

/* Try to change INSN so that it does pre-increment or pre-decrement
   addressing on register REG in order to add AMOUNT to REG.
   AMOUNT is negative for pre-decrement.
   Returns 1 if the change could be made.
   This checks all about the validity of the result of modifying INSN.  */

static int
try_pre_increment (insn, reg, amount)
     rtx insn, reg;
     int amount;
{
  register rtx use;

  /* Nonzero if we can try to make a pre-increment or pre-decrement.
     For example, addl $4,r1; movl (r1),... can become movl +(r1),...  */
  int pre_ok = 0;
  /* Nonzero if we can try to make a post-increment or post-decrement.
     For example, addl $4,r1; movl -4(r1),... can become movl (r1)+,...
     It is possible for both PRE_OK and POST_OK to be nonzero if the machine
     supports both pre-inc and post-inc, or both pre-dec and post-dec.  */
  int post_ok = 0;

  /* Nonzero if the opportunity actually requires post-inc or post-dec.  */
  int do_post = 0;

  /* From the sign of increment, see which possibilities are conceivable
     on this target machine.  */
#ifdef HAVE_PRE_INCREMENT
  if (amount > 0)
    pre_ok = 1;
#endif
#ifdef HAVE_POST_INCREMENT
  if (amount > 0)
    post_ok = 1;
#endif

#ifdef HAVE_PRE_DECREMENT
  if (amount < 0)
    pre_ok = 1;
#endif
#ifdef HAVE_POST_DECREMENT
  if (amount < 0)
    post_ok = 1;
#endif

  if (! (pre_ok || post_ok))
    return 0;

  /* It is not safe to add a side effect to a jump insn
     because if the incremented register is spilled and must be reloaded
     there would be no way to store the incremented value back in memory.  */

  if (GET_CODE (insn) == JUMP_INSN)
    return 0;

  use = 0;
  if (pre_ok)
    use = find_use_as_address (PATTERN (insn), reg, 0);
  if (post_ok && (use == 0 || use == (rtx) 1))
    {
      use = find_use_as_address (PATTERN (insn), reg, -amount);
      do_post = 1;
    }

  if (use == 0 || use == (rtx) 1)
    return 0;

  if (GET_MODE_SIZE (GET_MODE (use)) != (amount > 0 ? amount : - amount))
    return 0;

  XEXP (use, 0) = gen_rtx (amount > 0
			   ? (do_post ? POST_INC : PRE_INC)
			   : (do_post ? POST_DEC : PRE_DEC),
			   Pmode, reg);

  /* Record that this insn now has an implicit side effect on X.  */
  REG_NOTES (insn) = gen_rtx (EXPR_LIST, REG_INC, reg, REG_NOTES (insn));
  return 1;
}

#endif /* AUTO_INC_DEC */

/* Find the place in the rtx X where REG is used as a memory address.
   Return the MEM rtx that so uses it.
   If PLUSCONST is nonzero, search instead for a memory address equivalent to
   (plus REG (const_int PLUSCONST)).

   If such an address does not appear, return 0.
   If REG appears more than once, or is used other than in such an address,
   return (rtx)1.  */

static rtx
find_use_as_address (x, reg, plusconst)
     register rtx x;
     rtx reg;
     int plusconst;
{
  enum rtx_code code = GET_CODE (x);
  char *fmt = GET_RTX_FORMAT (code);
  register int i;
  register rtx value = 0;
  register rtx tem;

  if (code == MEM && XEXP (x, 0) == reg && plusconst == 0)
    return x;

  if (code == MEM && GET_CODE (XEXP (x, 0)) == PLUS
      && XEXP (XEXP (x, 0), 0) == reg
      && GET_CODE (XEXP (XEXP (x, 0), 1)) == CONST_INT
      && INTVAL (XEXP (XEXP (x, 0), 1)) == plusconst)
    return x;

  if (code == SIGN_EXTRACT || code == ZERO_EXTRACT)
    {
      /* If REG occurs inside a MEM used in a bit-field reference,
	 that is unacceptable.  */
      if (find_use_as_address (XEXP (x, 0), reg, 0) != 0)
	return (rtx) 1;
    }

  if (x == reg)
    return (rtx) 1;

  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	{
	  tem = find_use_as_address (XEXP (x, i), reg, plusconst);
	  if (value == 0)
	    value = tem;
	  else if (tem != 0)
	    return (rtx) 1;
	}
      if (fmt[i] == 'E')
	{
	  register int j;
	  for (j = XVECLEN (x, i) - 1; j >= 0; j--)
	    {
	      tem = find_use_as_address (XVECEXP (x, i, j), reg, plusconst);
	      if (value == 0)
		value = tem;
	      else if (tem != 0)
		return (rtx) 1;
	    }
	}
    }

  return value;
}

/* Write information about registers and basic blocks into FILE.
   This is part of making a debugging dump.  */

void
dump_flow_info (file)
     FILE *file;
{
  register int i;
  static char *reg_class_names[] = REG_CLASS_NAMES;

  fprintf (file, "%d registers.\n", max_regno);

  for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
    if (reg_n_refs[i])
      {
	enum reg_class class;
	fprintf (file, "\nRegister %d used %d times across %d insns",
		 i, reg_n_refs[i], reg_live_length[i]);
	if (reg_basic_block[i] >= 0)
	  fprintf (file, " in block %d", reg_basic_block[i]);
	if (reg_n_deaths[i] != 1)
	  fprintf (file, "; dies in %d places", reg_n_deaths[i]);
	if (reg_crosses_call[i])
	  fprintf (file, "; crosses calls");
	if (PSEUDO_REGNO_BYTES (i) != UNITS_PER_WORD)
	  fprintf (file, "; %d bytes", PSEUDO_REGNO_BYTES (i));
	class = reg_preferred_class (i);
	if (class != GENERAL_REGS)
	  {
	    if (reg_preferred_or_nothing (i))
	      fprintf (file, "; %s or none", reg_class_names[(int) class]);
	    else
	      fprintf (file, "; pref %s", reg_class_names[(int) class]);
	  }
	if (REGNO_POINTER_FLAG (i))
	  fprintf (file, "; pointer");
	fprintf (file, ".\n");
      }
  fprintf (file, "\n%d basic blocks.\n", n_basic_blocks);
  for (i = 0; i < n_basic_blocks; i++)
    {
      register rtx head, jump;
      register int regno;
      fprintf (file, "\nBasic block %d: first insn %d, last %d.\n",
	       i,
	       INSN_UID (basic_block_head[i]),
	       INSN_UID (basic_block_end[i]));
      /* The control flow graph's storage is freed
	 now when flow_analysis returns.
	 Don't try to print it if it is gone.  */
      if (basic_block_drops_in)
	{
	  fprintf (file, "Reached from blocks: ");
	  head = basic_block_head[i];
	  if (GET_CODE (head) == CODE_LABEL)
	    for (jump = LABEL_REFS (head);
		 jump != head;
		 jump = LABEL_NEXTREF (jump))
	      {
		register from_block = BLOCK_NUM (CONTAINING_INSN (jump));
		fprintf (file, " %d", from_block);
	      }
	  if (basic_block_drops_in[i])
	    fprintf (file, " previous");
	}
      fprintf (file, "\nRegisters live at start:");
      for (regno = 0; regno < max_regno; regno++)
	{
	  register int offset = regno / REGSET_ELT_BITS;
	  register int bit = 1 << (regno % REGSET_ELT_BITS);
	  if (basic_block_live_at_start[i][offset] & bit)
	      fprintf (file, " %d", regno);
	}
      fprintf (file, "\n");
    }
  fprintf (file, "\n");
}
