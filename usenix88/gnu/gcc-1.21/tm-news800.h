/* Definitions of target machine for GNU compiler.  SONY NEWS version.
   Copyright (C) 1987 Free Software Foundation, Inc.

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

/* Use the GNU Assembler, because the system's assembler
   has no way to assemble the difference of two labels
   for the displacement in a switch-dispatch instruction.  */

#define USE_GAS

#ifndef USE_GAS
/* This controls conditionals in tm-m68k.h.  */
#define MOTOROLA
#endif

#include "tm-m68k.h"

/* See tm-m68k.h.  7 means 68020 with 68881.  */

#define TARGET_DEFAULT 7

/* Names to predefine in the preprocessor for this target machine.  */

#define CPP_PREDEFINES "-Dunix -Dmc68020 -Dnews800"

/* Override parts of tm-m68k.h to fit Sony's assembler syntax.  */

#undef POINTER_BOUNDARY
#undef BIGGEST_ALIGNMENT
#undef CALL_USED_REGISTERS
#undef FUNCTION_VALUE
#undef LIBCALL_VALUE
#undef FUNCTION_PROFILER

#ifdef MOTOROLA
#undef FUNCTION_PROLOGUE
#undef FUNCTION_EPILOGUE
#undef REGISTER_NAMES
#undef ASM_OUTPUT_DOUBLE
#undef ASM_OUTPUT_SKIP
#undef PRINT_OPERAND
#undef PRINT_OPERAND_ADDRESS
#endif  

#undef ASM_OUTPUT_ALIGN

/* Allocation boundary (in *bits*) for storing pointers in memory.  */
#define POINTER_BOUNDARY 32

/* There is no point aligning anything to a rounder boundary than this.  */
#define BIGGEST_ALIGNMENT 32
  
/* NEWS makes d2, d3, fp2 and fp3 unsaved registers, unlike the Sun system.  */
  
#define CALL_USED_REGISTERS \
 {1, 1, 1, 1, 0, 0, 0, 0, \
  1, 1, 0, 0, 0, 0, 0, 1, \
  1, 1, 1, 1, 0, 0, 0, 0}

/* NEWS returns floats and doubles in fp0, not d0/d1.  */

#define FUNCTION_VALUE(VALTYPE,FUNC) LIBCALL_VALUE (TYPE_MODE (VALTYPE))

#define LIBCALL_VALUE(MODE) \
 gen_rtx (REG, (MODE), ((TARGET_68881 && ((MODE) == SFmode || (MODE) == DFmode)) ? 16 : 0))

#define ASM_OUTPUT_ALIGN(FILE,LOG)	\
  fprintf (FILE, "\t.align %d\n", (LOG))

#ifdef MOTOROLA

#define FUNCTION_PROLOGUE(FILE, SIZE)     \
{ register int regno;						\
  register int mask = 0;					\
  static char *reg_names[] = REGISTER_NAMES;			\
  extern char call_used_regs[];					\
  int fsize = (SIZE);						\
  if (frame_pointer_needed)					\
    { if (TARGET_68020 || fsize < 0x10000)			\
        fprintf (FILE, "\tlink fp,#%d\n", -fsize);		\
      else							\
	fprintf (FILE, "\tlink fp,#0\n\tsub.l #%d,sp\n", fsize); }  \
  for (regno = 16; regno < FIRST_PSEUDO_REGISTER; regno++)	\
    if (regs_ever_live[regno] && ! call_used_regs[regno])	\
       mask |= 1 << (regno - 16);				\
  if (mask != 0)						\
    fprintf (FILE, "\tfmovem.x #0x%x,-(sp)\n", mask & 0xff);    \
  mask = 0;							\
  for (regno = 0; regno < 16; regno++)				\
    if (regs_ever_live[regno] && ! call_used_regs[regno])	\
       mask |= 1 << (15 - regno);				\
  if (frame_pointer_needed)					\
    mask &= ~ (1 << (15-FRAME_POINTER_REGNUM));			\
  if (exact_log2 (mask) >= 0)					\
    fprintf (FILE, "\tmove.l %s,-(sp)\n", reg_names[15 - exact_log2 (mask)]);  \
  else if (mask) fprintf (FILE, "\tmovem.l #0x%x,-(sp)\n", mask); }

#define FUNCTION_PROFILER(FILE, LABEL_NO) \
   fprintf (FILE, "\tmove.l #LP%d,d0\n\tjsr mcount\n", (LABEL_NO));

#define FUNCTION_EPILOGUE(FILE, SIZE) \
{ register int regno;						\
  register int mask, fmask;					\
  register int nregs;						\
  int offset, foffset;						\
  extern char call_used_regs[];					\
  static char *reg_names[] = REGISTER_NAMES;			\
  extern int current_function_pops_args;			\
  extern int current_function_args_size;			\
  int fsize = (SIZE);						\
  int big = 0;							\
  nregs = 0;  fmask = 0;					\
  for (regno = 16; regno < FIRST_PSEUDO_REGISTER; regno++)	\
    if (regs_ever_live[regno] && ! call_used_regs[regno])	\
      { nregs++; fmask |= 1 << (23 - regno); }			\
  foffset = nregs * 12;						\
  nregs = 0;  mask = 0;						\
  if (frame_pointer_needed) regs_ever_live[FRAME_POINTER_REGNUM] = 0; \
  for (regno = 0; regno < 16; regno++)				\
    if (regs_ever_live[regno] && ! call_used_regs[regno])	\
      { nregs++; mask |= 1 << regno; }				\
  offset = foffset + nregs * 4;					\
  if (offset + fsize >= 0x8000 && frame_pointer_needed)		\
    { fprintf (FILE, "\tmove.l #%d,a0\n", -fsize);		\
      fsize = 0, big = 1; }					\
  if (exact_log2 (mask) >= 0) {					\
    if (big)							\
      fprintf (FILE, "\tmove.l (-%d,fp,a0.l),%s\n",		\
	       offset + fsize, reg_names[exact_log2 (mask)]);	\
    else if (! frame_pointer_needed)				\
      fprintf (FILE, "\tmove.l (sp)+,%s\n",			\
	       reg_names[exact_log2 (mask)]);			\
    else							\
      fprintf (FILE, "\tmove.l (-%d,fp),%s\n",			\
	       offset + fsize, reg_names[exact_log2 (mask)]); }	\
  else if (mask) {						\
    if (big)							\
      fprintf (FILE, "\tmovem.l (-%d,fp,a0.l),#0x%x\n",		\
	       offset + fsize, mask);				\
    else if (! frame_pointer_needed)				\
      fprintf (FILE, "\tmovem.l (sp)+,#0x%x\n", mask);		\
    else							\
      fprintf (FILE, "\tmovem.l (-%d,fp),#0x%x\n",		\
	       offset + fsize, mask); }				\
  if (fmask) {							\
    if (big)							\
      fprintf (FILE, "\tfmovem.x (-%d,fp,a0.l),#0x%x\n",	\
	       foffset + fsize, fmask);				\
    else if (! frame_pointer_needed)				\
      fprintf (FILE, "\tfmovem.x (sp)+,#0x%x\n", fmask);	\
    else							\
      fprintf (FILE, "\tfmovem.x (-%d,fp),#0x%x\n",		\
	       foffset + fsize, fmask); }			\
  if (frame_pointer_needed)					\
    fprintf (FILE, "\tunlk fp\n");				\
  if (current_function_pops_args && current_function_args_size)	\
    fprintf (FILE, "\trtd #%d\n", current_function_args_size);	\
  else fprintf (FILE, "\trts\n"); }

/* Difference from tm-m68k.h is in `fp' instead of `a6'.  */

#define REGISTER_NAMES \
{"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",	\
 "a0", "a1", "a2", "a3", "a4", "a5", "fp", "sp",	\
 "fp0", "fp1", "fp2", "fp3", "fp4", "fp5", "fp6", "fp7"}
  
#define ASM_OUTPUT_DOUBLE(FILE,VALUE)  \
  fprintf (FILE, "\t.double 0d%.20e\n", (VALUE))

#define ASM_OUTPUT_SKIP(FILE,SIZE)  \
  fprintf (FILE, "\t.space %d\n", (SIZE))

#define PRINT_OPERAND(FILE, X, CODE)  \
{ if (CODE == '.') fprintf (FILE, ".");					\
  else if (CODE == '#') fprintf (FILE, "#");				\
  else if (CODE == '-') fprintf (FILE, "-(sp)");			\
  else if (CODE == '+') fprintf (FILE, "(sp)+");			\
  else if (CODE == 's') fprintf (FILE, "(sp)");				\
  else if (CODE == '!') fprintf (FILE, "ccr");				\
  else if (GET_CODE (X) == REG)						\
    fprintf (FILE, "%s", reg_name [REGNO (X)]);				\
  else if (GET_CODE (X) == MEM)						\
    output_address (XEXP (X, 0));					\
  else if (GET_CODE (X) == CONST_DOUBLE && GET_MODE (X) == SFmode)	\
    { union { double d; int i[2]; } u;					\
      union { float f; int i; } u1;					\
      u.i[0] = XINT (X, 0); u.i[1] = XINT (X, 1);			\
      u1.f = u.d;							\
      if (CODE == 'f')							\
        fprintf (FILE, "#0f%.9e", u1.f);				\
      else								\
        fprintf (FILE, "#0x%x", u1.i); }				\
  else if (GET_CODE (X) == CONST_DOUBLE)				\
    { union { double d; int i[2]; } u;					\
      u.i[0] = XINT (X, 0); u.i[1] = XINT (X, 1);			\
      fprintf (FILE, "#0d%.20e", u.d); }				\
  else if (CODE == 'b') output_addr_const (FILE, X);			\
  else { putc ('#', FILE); output_addr_const (FILE, X); }}

#define PRINT_OPERAND_ADDRESS(FILE, ADDR)  \
{ register rtx reg1, reg2, breg, ireg;					\
  register rtx addr = ADDR;						\
  rtx offset;								\
  switch (GET_CODE (addr))						\
    {									\
    case REG:								\
      fprintf (FILE, "(%s)", reg_name [REGNO (addr)]);			\
      break;								\
    case PRE_DEC:							\
      fprintf (FILE, "-(%s)", reg_name [REGNO (XEXP (addr, 0))]);	\
      break;								\
    case POST_INC:							\
      fprintf (FILE, "(%s)+", reg_name [REGNO (XEXP (addr, 0))]);	\
      break;								\
    case PLUS:								\
      reg1 = 0;	reg2 = 0;						\
      ireg = 0;	breg = 0;						\
      offset = 0;							\
      if (CONSTANT_ADDRESS_P (XEXP (addr, 0)))				\
	{								\
	  offset = XEXP (addr, 0);					\
	  addr = XEXP (addr, 1);					\
	}								\
      else if (CONSTANT_ADDRESS_P (XEXP (addr, 1)))			\
	{								\
	  offset = XEXP (addr, 1);					\
	  addr = XEXP (addr, 0);					\
	}								\
      if (GET_CODE (addr) != PLUS) ;					\
      else if (GET_CODE (XEXP (addr, 0)) == SIGN_EXTEND)		\
	{								\
	  reg1 = XEXP (addr, 0);					\
	  addr = XEXP (addr, 1);					\
	}								\
      else if (GET_CODE (XEXP (addr, 1)) == SIGN_EXTEND)		\
	{								\
	  reg1 = XEXP (addr, 1);					\
	  addr = XEXP (addr, 0);					\
	}								\
      else if (GET_CODE (XEXP (addr, 0)) == MULT)			\
	{								\
	  reg1 = XEXP (addr, 0);					\
	  addr = XEXP (addr, 1);					\
	}								\
      else if (GET_CODE (XEXP (addr, 1)) == MULT)			\
	{								\
	  reg1 = XEXP (addr, 1);					\
	  addr = XEXP (addr, 0);					\
	}								\
      else if (GET_CODE (XEXP (addr, 0)) == REG)			\
	{								\
	  reg1 = XEXP (addr, 0);					\
	  addr = XEXP (addr, 1);					\
	}								\
      else if (GET_CODE (XEXP (addr, 1)) == REG)			\
	{								\
	  reg1 = XEXP (addr, 1);					\
	  addr = XEXP (addr, 0);					\
	}								\
      if (GET_CODE (addr) == REG || GET_CODE (addr) == MULT		\
	  || GET_CODE (addr) == SIGN_EXTEND)				\
	{ if (reg1 == 0) reg1 = addr; else reg2 = addr; addr = 0; }	\
      if (offset != 0) { if (addr != 0) abort (); addr = offset; }	\
      if ((reg1 && (GET_CODE (reg1) == SIGN_EXTEND			\
		    || GET_CODE (reg1) == MULT))			\
	  || (reg2 != 0 && REGNO_OK_FOR_BASE_P (REGNO (reg2))))		\
	{ breg = reg2; ireg = reg1; }					\
      else if (reg1 != 0 && REGNO_OK_FOR_BASE_P (REGNO (reg1)))		\
	{ breg = reg1; ireg = reg2; }					\
      if (ireg != 0 && breg == 0 && GET_CODE (addr) == LABEL_REF)	\
        { int scale = 1;						\
	  if (GET_CODE (ireg) == MULT)					\
	    { scale = INTVAL (XEXP (ireg, 1));				\
	      ireg = XEXP (ireg, 0); }					\
	  if (GET_CODE (ireg) == SIGN_EXTEND)				\
	    fprintf (FILE, "(L%d-LI%d.b,pc,%s.w",			\
		     CODE_LABEL_NUMBER (XEXP (addr, 0)),		\
		     CODE_LABEL_NUMBER (XEXP (addr, 0)),		\
		     reg_name[REGNO (XEXP (ireg, 0))]); 		\
	  else								\
	    fprintf (FILE, "(L%d-LI%d.b,pc,%s.l",			\
		     CODE_LABEL_NUMBER (XEXP (addr, 0)),		\
		     CODE_LABEL_NUMBER (XEXP (addr, 0)),		\
		     reg_name[REGNO (ireg)]);				\
	  if (scale != 1) fprintf (FILE, "*%d", scale);			\
	  putc (')', FILE);						\
	  break; }							\
      if (ireg != 0 || breg != 0)					\
	{ int scale = 1;						\
	  if (breg == 0)						\
	    abort ();							\
	  fprintf (FILE, "(");						\
	  if (addr != 0) {						\
	    output_addr_const (FILE, addr);				\
	    putc (',', FILE); }						\
	  fprintf (FILE, "%s", reg_name[REGNO (breg)]);			\
	  if (ireg != 0)						\
	    putc (',', FILE);						\
	  if (ireg != 0 && GET_CODE (ireg) == MULT)			\
	    { scale = INTVAL (XEXP (ireg, 1));				\
	      ireg = XEXP (ireg, 0); }					\
	  if (ireg != 0 && GET_CODE (ireg) == SIGN_EXTEND)		\
	    fprintf (FILE, "%s.w", reg_name[REGNO (XEXP (ireg, 0))]);	\
	  else if (ireg != 0)						\
	    fprintf (FILE, "%s.l", reg_name[REGNO (ireg)]);		\
	  if (scale != 1) fprintf (FILE, "*%d", scale);			\
	  putc (')', FILE);						\
	  break;							\
	}								\
      else if (reg1 != 0 && GET_CODE (addr) == LABEL_REF)		\
	{ fprintf (FILE, "(L%d-LI%d.b,pc,%s:l)",			\
		   CODE_LABEL_NUMBER (XEXP (addr, 0)),			\
		   CODE_LABEL_NUMBER (XEXP (addr, 0)),			\
		   reg_name[REGNO (reg1)]);				\
	  break; }							\
    default:								\
      if (GET_CODE (addr) == CONST_INT					\
	  && INTVAL (addr) < 0x8000					\
	  && INTVAL (addr) >= -0x8000)					\
	fprintf (FILE, "%d.w", INTVAL (addr));				\
      else								\
        output_addr_const (FILE, addr);					\
    }}

#else /* Using GAS, which uses the MIT assembler syntax, like a Sun.  */

#define FUNCTION_PROFILER(FILE, LABEL_NO) \
   fprintf (FILE, "\tmovl #LP%d,d0\n\tjsr mcount\n", (LABEL_NO));

#endif /* MOTOROLA */
