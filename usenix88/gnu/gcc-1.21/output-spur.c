/* Subroutines for insn-output.c for Motorola 68000 family.
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

static rtx find_addr_reg ();

char *
output_compare (operands, opcode, exchange_opcode)
     rtx *operands;
     char *opcode;
     char *exchange_opcode;
{
  static char buf[40];
  operands[2] = operands[0];
  if (GET_CODE (cc_prev_status.value1) == CONST_INT)
    {
      operands[1] = cc_prev_status.value1;
      operands[0] = cc_prev_status.value2;
      opcode = exchange_opcode;
    }
  else
    {
      operands[0] = cc_prev_status.value1;
      operands[1] = cc_prev_status.value2;
    }
  sprintf (buf, "cmp_br_delayed %s,%%0,%%1,%%l2\n\tnop", opcode);
  return buf;
}

/* Return the best assembler insn template
   for moving operands[1] into operands[0] as a fullword.  */

static char *
singlemove_string (operands)
     rtx *operands;
{
  if (GET_CODE (operands[0]) == MEM)
    return "st_32 %r1,%0";
  if (GET_CODE (operands[1]) == MEM)
    return "ld_32 %0,%1\n\tnop";
  return "add_nt %0,r0,%1";
}

/* Output assembler code to perform a doubleword move insn
   with operands OPERANDS.  */

char *
output_move_double (operands)
     rtx *operands;
{
  enum { REGOP, OFFSOP, MEMOP, PUSHOP, POPOP, CNSTOP, RNDOP } optype0, optype1;
  rtx latehalf[2];
  rtx addreg0 = 0, addreg1 = 0;

  /* First classify both operands.  */

  if (REG_P (operands[0]))
    optype0 = REGOP;
  else if (offsetable_memref_p (operands[0]))
    optype0 = OFFSOP;
  else if (GET_CODE (operands[0]) == MEM)
    optype0 = MEMOP;
  else
    optype0 = RNDOP;

  if (REG_P (operands[1]))
    optype1 = REGOP;
  else if (CONSTANT_P (operands[1])
	   || GET_CODE (operands[1]) == CONST_DOUBLE)
    optype1 = CNSTOP;
  else if (offsetable_memref_p (operands[1]))
    optype1 = OFFSOP;
  else if (GET_CODE (operands[1]) == MEM)
    optype0 = MEMOP;
  else
    optype1 = RNDOP;

  /* Check for the cases that the operand constraints are not
     supposed to allow to happen.  Abort if we get one,
     because generating code for these cases is painful.  */

  if (optype0 == RNDOP || optype1 == RNDOP)
    abort ();

  /* If an operand is an unoffsettable memory ref, find a register
     we can increment temporarily to make it refer to the second word.  */

  if (optype0 == MEMOP)
    addreg0 = find_addr_reg (operands[0]);

  if (optype1 == MEMOP)
    addreg1 = find_addr_reg (operands[1]);

  /* Ok, we can do one word at a time.
     Normally we do the low-numbered word first,
     but if either operand is autodecrementing then we
     do the high-numbered word first.

     In either case, set up in LATEHALF the operands to use
     for the high-numbered word and in some cases alter the
     operands in OPERANDS to be suitable for the low-numbered word.  */

  if (optype0 == REGOP)
    latehalf[0] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
  else if (optype0 == OFFSOP)
    latehalf[0] = adj_offsetable_operand (operands[0], 4);
  else
    latehalf[0] = operands[0];

  if (optype1 == REGOP)
    latehalf[1] = gen_rtx (REG, SImode, REGNO (operands[1]) + 1);
  else if (optype1 == OFFSOP)
    latehalf[1] = adj_offsetable_operand (operands[1], 4);
  else if (optype1 == CNSTOP)
    {
      if (CONSTANT_P (operands[1]))
	latehalf[1] = const0_rtx;
      else if (GET_CODE (operands[1]) == CONST_DOUBLE)
	{
	  latehalf[1] = gen_rtx (CONST_INT, VOIDmode, XINT (operands[1], 1));
	  operands[1] = gen_rtx (CONST_INT, VOIDmode, XINT (operands[1], 0));
	}
    }
  else
    latehalf[1] = operands[1];

  /* If the first move would clobber the source of the second one,
     do them in the other order.  This happens only for registers;
     such overlap can't happen in memory unless the user explicitly
     sets it up, and that is an undefined circumstance.  */

  if (optype0 == REGOP && optype1 == REGOP
      && REGNO (operands[0]) == REGNO (latehalf[1]))
    {
      /* Make any unoffsetable addresses point at high-numbered word.  */
      if (addreg0)
	output_asm_insn ("add_nt %0,%0,$4", &addreg0);
      if (addreg1)
	output_asm_insn ("add_nt %0,%0,$4", &addreg1);

      /* Do that word.  */
      output_asm_insn (singlemove_string (latehalf), latehalf);

      /* Undo the adds we just did.  */
      if (addreg0)
	output_asm_insn ("add_nt %0,%0,$-4", &addreg0);
      if (addreg1)
	output_asm_insn ("add_nt %0,%0,$-4", &addreg0);

      /* Do low-numbered word.  */
      return singlemove_string (operands);
    }

  /* Normal case: do the two words, low-numbered first.  */

  output_asm_insn (singlemove_string (operands), operands);

  /* Make any unoffsetable addresses point at high-numbered word.  */
  if (addreg0)
    output_asm_insn ("add_nt %0,%0,$4", &addreg0);
  if (addreg1)
    output_asm_insn ("add_nt %0,%0,$4", &addreg1);

  /* Do that word.  */
  output_asm_insn (singlemove_string (latehalf), latehalf);

  /* Undo the adds we just did.  */
  if (addreg0)
    output_asm_insn ("add_nt %0,%0,$-4", &addreg0);
  if (addreg1)
    output_asm_insn ("add_nt %0,%0,$-4", &addreg1);

  return "";
}

static char *
output_fp_move_double (operands)
     rtx *operands;
{
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]))
	return "fmov %0,%1";
      if (GET_CODE (operands[1]) == REG)
	{
	  rtx xoperands[2];
	  int offset = - get_frame_size () - 8;
	  xoperands[1] = gen_rtx (REG, SImode, REGNO (operands[1]) + 1);
	  xoperands[0] = gen_rtx (CONST_INT, VOIDmode, offset + 4);
	  output_asm_insn ("st_32 %1,r25,%0", xoperands);
	  xoperands[1] = operands[1];
	  xoperands[0] = gen_rtx (CONST_INT, VOIDmode, offset);
	  output_asm_insn ("st_32 %1,r25,%0", xoperands);
	  xoperands[1] = operands[0];
	  output_asm_insn ("ld_dbl %1,r25,%0\n\tnop", xoperands);
	  return "";
	}
      return "ld_dbl %0,%1\n\tnop";
    }
  else if (FP_REG_P (operands[1]))
    {
      if (GET_CODE (operands[0]) == REG)
	{
	  rtx xoperands[2];
	  int offset = - get_frame_size () - 8;
	  xoperands[0] = gen_rtx (CONST_INT, VOIDmode, offset);
	  xoperands[1] = operands[1];
	  output_asm_insn ("st_dbl %1,r25,%0", xoperands);
	  xoperands[1] = operands[0];
	  output_asm_insn ("ld_32 %1,r25,%0\n\tnop", xoperands);
	  xoperands[1] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
	  xoperands[0] = gen_rtx (CONST_INT, VOIDmode, offset + 4);
	  output_asm_insn ("ld_32 %1,r25,%0\n\tnop", xoperands);
	  return "";
	}
      return "st_dbl %1,%0";
    }
}

/* Return a REG that occurs in ADDR with coefficient 1.
   ADDR can be effectively incremented by incrementing REG.  */

static rtx
find_addr_reg (addr)
     rtx addr;
{
  while (GET_CODE (addr) == PLUS)
    {
      if (GET_CODE (XEXP (addr, 0)) == REG)
	addr = XEXP (addr, 0);
      if (GET_CODE (XEXP (addr, 1)) == REG)
	addr = XEXP (addr, 1);
      if (CONSTANT_P (XEXP (addr, 0)))
	addr = XEXP (addr, 1);
      if (CONSTANT_P (XEXP (addr, 1)))
	addr = XEXP (addr, 0);
    }
  if (GET_CODE (addr) == REG)
    return addr;
  return 0;
}
