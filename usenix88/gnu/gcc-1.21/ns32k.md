;; BUGS:
;; Insert no-op between an insn with memory read-write operands
;;   following by a scale-indexing operation.
;; The Sequent assembler does not allow addresses to be used
;;   except in insns which explicitly compute an effective address.
;;   I.e., one cannot say "cmpd _p,@_x"
;; Implement unsigned multiplication??

;;- Machine descrption for GNU compiler
;;- ns32000 Version
;;   Copyright (C) 1988 Free Software Foundation, Inc.
;;   Contributed by Michael Tiemann (tiemann@mcc.com)

;; This file is part of GNU CC.

;; GNU CC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY.  No author or distributor
;; accepts responsibility to anyone for the consequences of using it
;; or for whether it serves any particular purpose or works at all,
;; unless he says so in writing.  Refer to the GNU CC General Public
;; License for full details.

;; Everyone is granted permission to copy, modify and redistribute
;; GNU CC, but only under the conditions described in the
;; GNU CC General Public License.   A copy of this license is
;; supposed to have been given to you along with GNU CC so you
;; can know your rights and responsibilities.  It should be in a
;; file named COPYING.  Among other things, the copyright notice
;; and this notice must be preserved on all copies.


;;- Instruction patterns.  When multiple patterns apply,
;;- the first one in the file is chosen.
;;-
;;- See file "rtl.def" for documentation on define_insn, match_*, et. al.
;;-
;;- cpp macro #define NOTICE_UPDATE_CC in file tm.h handles condition code
;;- updates for most instructions.

; tstsi is first test insn so that it is the one to match
; a constant argument.

(define_insn "tstsi"
  [(set (cc0)
	(match_operand:SI 0 "general_operand" "rmn"))]
  ""
  "*
{ cc_status.flags |= CC_REVERSED; return \"cmpqd %$0,%0\"; }")

(define_insn "tsthi"
  [(set (cc0)
	(match_operand:HI 0 "general_operand" "g"))]
  ""
  "*
{ cc_status.flags |= CC_REVERSED; return \"cmpqw %$0,%0\"; }")

(define_insn "tstqi"
  [(set (cc0)
	(match_operand:QI 0 "general_operand" "g"))]
  ""
  "*
{ cc_status.flags |= CC_REVERSED; return \"cmpqb %$0,%0\"; }")

(define_insn "tstdf"
  [(set (cc0)
	(match_operand:DF 0 "general_operand" "fmF"))]
  "TARGET_32081"
  "*
{ cc_status.flags |= CC_REVERSED; return \"cmpl %$0d0.0,%0\"; }")

(define_insn "tstsf"
  [(set (cc0)
	(match_operand:SF 0 "general_operand" "fmF"))]
  "TARGET_32081"
  "*
{ cc_status.flags |= CC_REVERSED; return \"cmpf %$0f0.0,%0\"; }")

(define_insn "cmpdf"
  [(set (cc0)
	(minus (match_operand:DF 0 "general_operand" "fmF")
	       (match_operand:DF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "cmpl %0,%1")

(define_insn "cmpsf"
  [(set (cc0)
	(minus (match_operand:SF 0 "general_operand" "fmF")
	       (match_operand:SF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "cmpf %0,%1")

(define_insn "cmpsi"
  [(set (cc0)
	(minus (match_operand:SI 0 "general_operand" "rmn")
	       (match_operand:SI 1 "general_operand" "rmn")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      int i = INTVAL (operands[1]);
      if (i <= 7 && i >= -8)
	{
	  cc_status.flags |= CC_REVERSED;
	  return \"cmpqd %1,%0\";
	}
    }
  cc_status.flags &= ~CC_REVERSED;
  if (GET_CODE (operands[0]) == CONST_INT)
    {
      int i = INTVAL (operands[0]);
      if (i <= 7 && i >= -8)
	return \"cmpqd %0,%1\";
    }
  return \"cmpd %0,%1\";
}")

(define_insn "cmphi"
  [(set (cc0)
	(minus (match_operand:HI 0 "general_operand" "g")
	       (match_operand:HI 1 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      short i = INTVAL (operands[1]);
    if (i <= 7 && i >= -8)
      {
	cc_status.flags |= CC_REVERSED;
	if (INTVAL (operands[1]) > 7)
	  operands[1] = gen_rtx(CONST_INT, VOIDmode, i);
	return \"cmpqw %1,%0\";
      }
    }
  cc_status.flags &= ~CC_REVERSED;
  if (GET_CODE (operands[0]) == CONST_INT)
    {
      short i = INTVAL (operands[0]);
      if (i <= 7 && i >= -8)
	{
	  if (INTVAL (operands[0]) > 7)
	    operands[0] = gen_rtx(CONST_INT, VOIDmode, i);
	  return \"cmpqw %0,%1\";
	}
    }
  return \"cmpw %0,%1\";
}")

(define_insn "cmpqi"
  [(set (cc0)
	(minus (match_operand:QI 0 "general_operand" "g")
	       (match_operand:QI 1 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      char i = INTVAL (operands[1]);
      if (i <= 7 && i >= -8)
	{
	  cc_status.flags |= CC_REVERSED;
	  if (INTVAL (operands[1]) > 7)
	    operands[1] = gen_rtx(CONST_INT, VOIDmode, i);
	  return \"cmpqb %1,%0\";
	}
    }
  cc_status.flags &= ~CC_REVERSED;
  if (GET_CODE (operands[0]) == CONST_INT)
    {
      char i = INTVAL (operands[0]);
      if (i <= 7 && i >= -8)
	{
	  if (INTVAL (operands[0]) > 7)
	    operands[0] = gen_rtx(CONST_INT, VOIDmode, i);
	  return \"cmpqb %0,%1\";
	}
    }
  return \"cmpb %0,%1\";
}")

(define_insn "movdf"
  [(set (match_operand:DF 0 "general_operand" "=fg<")
	(match_operand:DF 1 "general_operand" "fFg"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]) || GET_CODE (operands[1]) == CONST_DOUBLE)
	return \"movl %1,%0\";
      if (REG_P (operands[1]))
	{
	  rtx xoperands[2];
	  xoperands[1] = gen_rtx (REG, SImode, REGNO (operands[1]) + 1);
	  output_asm_insn (\"movd %1,tos\", xoperands);
	  output_asm_insn (\"movd %1,tos\", operands);
	  return \"movl tos,%0\";
	}
      return \"movl %1,%0\";
    }
  else if (FP_REG_P (operands[1]))
    {
      if (REG_P (operands[0]))
	{
	  output_asm_insn (\"movl %1,tos\;movd tos,%0\", operands);
	  operands[0] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
	  return \"movd tos,%0\";
	}
      else
        return \"movl %1,%0\";
    }
  return output_move_double (operands);
}")

(define_insn "movsf"
  [(set (match_operand:SF 0 "general_operand" "=fg<")
	(match_operand:SF 1 "general_operand" "fFg"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]))
    {
      if (GET_CODE (operands[1]) == REG && REGNO (operands[1]) < 8)
	return \"movd %1,tos\;movf tos,%0\";
      else
	return \"movf %1,%0\";
    }
  else if (FP_REG_P (operands[1]))
    {
      if (REG_P (operands[0]))
	return \"movf %1,tos\;movd tos,%0\";
      return \"movf %1,%0\";
    }
  else if (GET_CODE (operands[1]) == CONST_DOUBLE)
    {
      /* Is there a better machine-independent way to to this?  */
      operands[1] = gen_rtx (CONST_INT, VOIDmode, INTVAL (operands[1]));
      return \"movd %1,%0\";
    }
  else return \"movd %1,%0\";
}")

(define_insn ""
  [(set (match_operand:TI 0 "memory_operand" "=m")
	(match_operand:TI 1 "memory_operand" "m"))]
  ""
  "movmd %1,%0,4")

(define_insn "movdi"
  [(set (match_operand:DI 0 "general_operand" "=g<")
	(match_operand:DI 1 "general_operand" "g"))]
  ""
  "* return output_move_double (operands); ")

(define_insn "movsi"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(match_operand:SI 1 "general_operand" "gx"))]
  ""
  "*
{ if (GET_CODE (operands[1]) == CONST_INT)
    {
      int i = INTVAL (operands[1]);
      if (i <= 7 && i >= -8)
	return \"movqd %1,%0\";
      if (i < 0x4000 && i >= -0x4000)
	return \"addr @%1,%0\";
      return \"movd %1,%0\";
    }
  else if (GET_CODE (operands[1]) == REG)
    if (REGNO (operands[1]) < 16)
      return \"movd %1,%0\";
    else if (REGNO (operands[1]) == 16)
      return \"addr 0(fp),%0\";
    else return \"addr tos,%0\";
  else if (GET_CODE (operands[1]) == MEM)
    return \"movd %1,%0\";
  return \"addr %a1,%0\";
}")

(define_insn "movhi"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(match_operand:HI 1 "general_operand" "g"))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      short i = INTVAL (operands[1]);
      if (i <= 7 && i >= -8)
	{
	  if (INTVAL (operands[1]) > 7)
	    operands[1] =
	      gen_rtx (CONST_INT, VOIDmode, i);
	  return \"movqw %1,%0\";
	}
    }
  return \"movw %1,%0\";
}")

(define_insn "movstricthi"
  [(set (strict_low_part (match_operand:HI 0 "general_operand" "+r"))
	(match_operand:HI 1 "general_operand" "g"))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL(operands[1]) <= 7 && INTVAL(operands[1]) >= -8)
    return \"movqw %1,%0\";
  return \"movw %1,%0\";
}")

(define_insn "movqi"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(match_operand:QI 1 "general_operand" "g"))]
  ""
  "*
{ if (GET_CODE (operands[1]) == CONST_INT)
    {
      char char_val = (char)INTVAL (operands[1]);
      if (char_val <= 7 && char_val >= -8)
	{
	  if (INTVAL (operands[1]) > 7)
	    operands[1] =
	      gen_rtx (CONST_INT, VOIDmode, char_val);
	  return \"movqb %1,%0\";
	}
    }
  return \"movb %1,%0\";
}")

(define_insn "movstrictqi"
  [(set (strict_low_part (match_operand:QI 0 "general_operand" "+r"))
	(match_operand:QI 1 "general_operand" "g"))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL(operands[1]) < 8 && INTVAL(operands[1]) > -9)
    return \"movqb %1,%0\";
  return \"movb %1,%0\";
}")

(define_insn ""
  [(set (reg:SI 17)
	(match_operand:SI 0 "general_operand" "g"))]
  ""
  "lprd sp,%0")

;; The definition of this insn does not really explain what it does,
;; but it should suffice
;; that anything generated as this insn will be recognized as one
;; and that it won't successfully combine with anything.
(define_insn "movstrsi"
  [(set (match_operand:BLK 0 "general_operand" "=g")
	(match_operand:BLK 1 "general_operand" "g"))
   (use (match_operand:SI 2 "general_operand" "rmn"))
   (clobber (reg:SI 0))
   (clobber (reg:SI 1))
   (clobber (reg:SI 2))]
  ""
  "*
{
  if (GET_CODE (operands[0]) != MEM || GET_CODE (operands[1]) != MEM)
    abort ();
  operands[0] = XEXP (operands[0], 0);
  operands[1] = XEXP (operands[1], 0);
  if (GET_CODE (operands[0]) == MEM)
    if (GET_CODE (operands[1]) == MEM)
      output_asm_insn (\"movd %0,r2\;movd %1,r1\", operands);
    else
      output_asm_insn (\"movd %0,r2\;addr %a1,r1\", operands);
  else if (GET_CODE (operands[1]) == MEM)
    output_asm_insn (\"addr %a0,r2\;movd %1,r1\", operands);
  else
    output_asm_insn (\"addr %a0,r2\;addr %a1,r1\", operands);

  if (GET_CODE (operands[2]) == CONST_INT && (INTVAL (operands[2]) & 0x3) == 0)
    {
      operands[2] = gen_rtx (CONST_INT, VOIDmode, INTVAL (operands[2]) >> 2);
      if ((unsigned) INTVAL (operands[2]) <= 7)
	return \"movqd %2,r0\;movsd\";
      else 
	return \"movd %2,r0\;movsd\";
    }
  else
    {
      return \"movd %2,r0\;movsb\";
    }
}")

;; Extension and truncation insns.
;; Those for integer source operand
;; are ordered widest source type first.

(define_insn "truncsiqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(truncate:QI (match_operand:SI 1 "general_operand" "g")))]
  ""
  "movb %1,%0")

(define_insn "truncsihi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(truncate:HI (match_operand:SI 1 "general_operand" "g")))]
  ""
  "movw %1,%0")

(define_insn "trunchiqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(truncate:QI (match_operand:HI 1 "general_operand" "g")))]
  ""
  "movb %1,%0")

(define_insn "extendhisi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(sign_extend:SI (match_operand:HI 1 "general_operand" "g")))]
  ""
  "movxwd %1,%0")

(define_insn "extendqihi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(sign_extend:HI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "movxbw %1,%0")

(define_insn "extendqisi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(sign_extend:SI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "movxbd %1,%0")

(define_insn "extendsfdf2"
  [(set (match_operand:DF 0 "general_operand" "=fm<")
	(float_extend:DF (match_operand:SF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "movfl %1,%0")

(define_insn "truncdfsf2"
  [(set (match_operand:SF 0 "general_operand" "=fm<")
	(float_truncate:SF (match_operand:DF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "movlf %1,%0")

(define_insn "zero_extendhisi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(zero_extend:SI (match_operand:HI 1 "general_operand" "g")))]
  ""
  "movzwd %1,%0")

(define_insn "zero_extendqihi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(zero_extend:HI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "movzbw %1,%0")

(define_insn "zero_extendqisi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(zero_extend:SI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "movzbd %1,%0")

;; Fix-to-float conversion insns.
;; Note that the ones that start with SImode come first.
;; That is so that an operand that is a CONST_INT
;; (and therefore lacks a specific machine mode).
;; will be recognized as SImode (which is always valid)
;; rather than as QImode or HImode.

;; Rumor has it that the National part does not correctly convert
;; constant ints to floats.  This conversion is therefore disabled.
;; A register must be used to perform the conversion.

(define_insn "floatsisf2"
  [(set (match_operand:SF 0 "general_operand" "=fm<")
	(float:SF (match_operand:SI 1 "general_operand" "rm")))]
  "TARGET_32081"
  "movdf %1,%0")

(define_insn "floatsidf2"
  [(set (match_operand:DF 0 "general_operand" "=fm<")
	(float:DF (match_operand:SI 1 "general_operand" "rm")))]
  "TARGET_32081"
  "movdl %1,%0")

(define_insn "floathisf2"
  [(set (match_operand:SF 0 "general_operand" "=fm<")
	(float:SF (match_operand:HI 1 "general_operand" "rm")))]
  "TARGET_32081"
  "movwf %1,%0")

(define_insn "floathidf2"
  [(set (match_operand:DF 0 "general_operand" "=fm<")
	(float:DF (match_operand:HI 1 "general_operand" "rm")))]
  "TARGET_32081"
  "movwl %1,%0")

(define_insn "floatqisf2"
  [(set (match_operand:SF 0 "general_operand" "=fm<")
	(float:SF (match_operand:QI 1 "general_operand" "rm")))]
  "TARGET_32081"
  "movbf %1,%0")

(define_insn "floatqidf2"
  [(set (match_operand:DF 0 "general_operand" "=fm<")
	(float:DF (match_operand:QI 1 "general_operand" "rm")))]
  "TARGET_32081"
  "movbl %1,%0")

;; Float-to-fix conversion insns.
;; The sequent compiler always generates "trunc" insns.

(define_insn "fixsfqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(fix:QI (fix:SF (match_operand:SF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncfb %1,%0")

(define_insn "fixsfhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(fix:HI (fix:SF (match_operand:SF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncfw %1,%0")

(define_insn "fixsfsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(fix:SI (fix:SF (match_operand:SF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncfd %1,%0")

(define_insn "fixdfqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(fix:QI (fix:DF (match_operand:DF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "trunclb %1,%0")

(define_insn "fixdfhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(fix:HI (fix:DF (match_operand:DF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "trunclw %1,%0")

(define_insn "fixdfsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(fix:SI (fix:DF (match_operand:DF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncld %1,%0")

;; Unsigned

(define_insn "fixunssfqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(unsigned_fix:QI (fix:SF (match_operand:SF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncfb %1,%0")

(define_insn "fixunssfhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(unsigned_fix:HI (fix:SF (match_operand:SF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncfw %1,%0")

(define_insn "fixunssfsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(unsigned_fix:SI (fix:SF (match_operand:SF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncfd %1,%0")

(define_insn "fixunsdfqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(unsigned_fix:QI (fix:DF (match_operand:DF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "trunclb %1,%0")

(define_insn "fixunsdfhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(unsigned_fix:HI (fix:DF (match_operand:DF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "trunclw %1,%0")

(define_insn "fixunsdfsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(unsigned_fix:SI (fix:DF (match_operand:DF 1 "general_operand" "fm"))))]
  "TARGET_32081"
  "truncld %1,%0")

;;; These are not yet used by GCC
(define_insn "fix_truncsfqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(fix:QI (match_operand:SF 1 "general_operand" "fm")))]
  "TARGET_32081"
  "truncfb %1,%0")

(define_insn "fix_truncsfhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(fix:HI (match_operand:SF 1 "general_operand" "fm")))]
  "TARGET_32081"
  "truncfw %1,%0")

(define_insn "fix_truncsfsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(fix:SI (match_operand:SF 1 "general_operand" "fm")))]
  "TARGET_32081"
  "truncfd %1,%0")

(define_insn "fix_truncdfqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(fix:QI (match_operand:DF 1 "general_operand" "fm")))]
  "TARGET_32081"
  "trunclb %1,%0")

(define_insn "fix_truncdfhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(fix:HI (match_operand:DF 1 "general_operand" "fm")))]
  "TARGET_32081"
  "trunclw %1,%0")

(define_insn "fix_truncdfsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(fix:SI (match_operand:DF 1 "general_operand" "fm")))]
  "TARGET_32081"
  "truncld %1,%0")

;;- All kinds of add instructions.

(define_insn "adddf3"
  [(set (match_operand:DF 0 "general_operand" "=fm")
	(plus:DF (match_operand:DF 1 "general_operand" "%0")
		 (match_operand:DF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "addl %2,%0")


(define_insn "addsf3"
  [(set (match_operand:SF 0 "general_operand" "=fm")
	(plus:SF (match_operand:SF 1 "general_operand" "%0")
		 (match_operand:SF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "addf %2,%0")

(define_insn ""
  [(set (reg:SI 17)
	(plus:SI (reg:SI 17)
		 (match_operand:SI 0 "immediate_operand" "i")))]
  "GET_CODE (operands[0]) == CONST_INT"
  "*
{
  if (INTVAL(operands[0]) < 64 && INTVAL(operands[0]) > -64)
    return \"adjspb %$%n0\";
  else if (INTVAL (operands[0]) < 8192 && INTVAL (operands[0]) >= -8192)
    return \"adjspw %$%n0\";
  return \"adjspd %$%n0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(plus:SI (reg:SI 16)
		 (match_operand:SI 1 "immediate_operand" "i")))]
  "GET_CODE (operands[1]) == CONST_INT"
  "addr %1(fp),%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(plus:SI (reg:SI 17)
		 (match_operand:SI 1 "immediate_operand" "i")))]
  "GET_CODE (operands[1]) == CONST_INT"
  "addr %1(sp),%0")

(define_insn "addsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(plus:SI (match_operand:SI 1 "general_operand" "%0")
		 (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      int i = INTVAL (operands[2]);

      if (i <= 7 && i >= -8)
	return \"addqd %2,%0\";
      else if (GET_CODE (operands[0]) == REG
	       && i < 0x4000 && i >= -0x4000)
	return \"addr %2(%0),%0\";
    }
  return \"addd %2,%0\";
}")

(define_insn "addhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(plus:HI (match_operand:HI 1 "general_operand" "%0")
		 (match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    {
      int i = INTVAL (operands[2]);
      if (i <= 7 && i >= -8)
	return \"addqw %2,%0\";
    }
  return \"addw %2,%0\";
}")

(define_insn ""
  [(set (strict_low_part (match_operand:HI 0 "general_operand" "=r"))
	(plus:HI (match_operand:HI 1 "general_operand" "0")
		 (match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL (operands[1]) >-9 && INTVAL(operands[1]) < 8)
    return \"addqw %1,%0\";
  return \"addw %1,%0\";
}")

(define_insn "addqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(plus:QI (match_operand:QI 1 "general_operand" "%0")
		 (match_operand:QI 2 "general_operand" "g")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    {
      int i = INTVAL (operands[2]);
      if (i <= 7 && i >= -8)
	return \"addqb %2,%0\";
    }
  return \"addb %2,%0\";
}")

(define_insn ""
  [(set (strict_low_part (match_operand:QI 0 "general_operand" "=r"))
	(plus:QI (match_operand:QI 1 "general_operand" "0")
		 (match_operand:QI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL (operands[1]) >-9 && INTVAL(operands[1]) < 8)
    return \"addqb %1,%0\";
  return \"addb %1,%0\";
}")

;;- All kinds of subtract instructions.

(define_insn "subdf3"
  [(set (match_operand:DF 0 "general_operand" "=fm")
	(minus:DF (match_operand:DF 1 "general_operand" "0")
		  (match_operand:DF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "subl %2,%0")

(define_insn "subsf3"
  [(set (match_operand:SF 0 "general_operand" "=fm")
	(minus:SF (match_operand:SF 1 "general_operand" "0")
		  (match_operand:SF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "subf %2,%0")

(define_insn ""
  [(set (reg:SI 17)
	(minus:SI (reg:SI 17)
		  (match_operand:SI 0 "immediate_operand" "i")))]
  "GET_CODE (operands[0]) == CONST_INT"
  "*
{
  if (GET_CODE(operands[0]) == CONST_INT && INTVAL(operands[0]) < 64
      && INTVAL(operands[0]) > -64)
    return \"adjspb %0\";
  return \"adjspd %0\";
}")

(define_insn "subsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(minus:SI (match_operand:SI 1 "general_operand" "0")
		  (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    {
      int i = INTVAL (operands[2]);

      if (i <= 8 && i >= -7)
        return \"addqd %$%n2,%0\";
    }
  return \"subd %2,%0\";
}")

(define_insn "subhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(minus:HI (match_operand:HI 1 "general_operand" "0")
		  (match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    {
      int i = INTVAL (operands[2]);

      if (i <= 8 && i >= -7)
        return \"addqw %$%n2,%0\";
    }
  return \"subw %2,%0\";
}")

(define_insn ""
  [(set (strict_low_part (match_operand:HI 0 "general_operand" "=r"))
	(minus:HI (match_operand:HI 1 "general_operand" "0")
		  (match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL (operands[1]) >-8 && INTVAL(operands[1]) < 9)
    return \"addqw %$%n1,%0\";
  return \"subw %1,%0\";
}")

(define_insn "subqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(minus:QI (match_operand:QI 1 "general_operand" "0")
		  (match_operand:QI 2 "general_operand" "g")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    {
      int i = INTVAL (operands[2]);

      if (i <= 8 && i >= -7)
	return \"addqb %$%n2,%0\";
    }
  return \"subb %2,%0\";
}")

(define_insn ""
  [(set (strict_low_part (match_operand:QI 0 "general_operand" "=r"))
	(minus:QI (match_operand:QI 1 "general_operand" "0")
		  (match_operand:QI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && INTVAL (operands[1]) >-8 && INTVAL(operands[1]) < 9)
    return \"addqb %$%n1,%0\";
  return \"subb %1,%0\";
}")

;;- Multiply instructions.

(define_insn "muldf3"
  [(set (match_operand:DF 0 "general_operand" "=fm")
	(mult:DF (match_operand:DF 1 "general_operand" "%0")
		 (match_operand:DF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "mull %2,%0")

(define_insn "mulsf3"
  [(set (match_operand:SF 0 "general_operand" "=fm")
	(mult:SF (match_operand:SF 1 "general_operand" "%0")
		 (match_operand:SF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "mulf %2,%0")

(define_insn "mulsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(mult:SI (match_operand:SI 1 "general_operand" "%0")
		 (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "muld %2,%0")

(define_insn "mulhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(mult:HI (match_operand:HI 1 "general_operand" "%0")
		 (match_operand:HI 2 "general_operand" "g")))]
  ""
  "mulw %2,%0")

(define_insn "mulqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(mult:QI (match_operand:QI 1 "general_operand" "%0")
		 (match_operand:QI 2 "general_operand" "g")))]
  ""
  "mulb %2,%0")

(define_insn "umulsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(umult:SI (match_operand:SI 1 "general_operand" "%0")
		  (match_operand:SI 2 "general_operand" "g")))]
  ""
  "muld %2,%0")

(define_insn "umulhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(umult:HI (match_operand:HI 1 "general_operand" "%0")
		  (match_operand:HI 2 "general_operand" "g")))]
  ""
  "mulw %2,%0")

(define_insn "umulqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(umult:QI (match_operand:QI 1 "general_operand" "%0")
		  (match_operand:QI 2 "general_operand" "g")))]
  ""
  "mulb %2,%0")

(define_insn "umulsidi3"
  [(set (match_operand:DI 0 "general_operand" "=g")
	(umult:DI (match_operand:SI 1 "general_operand" "0")
		  (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "meid %2,%0")

;;- Divide instructions.

(define_insn "divdf3"
  [(set (match_operand:DF 0 "general_operand" "=fm")
	(div:DF (match_operand:DF 1 "general_operand" "0")
		(match_operand:DF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "divl %2,%0")

(define_insn "divsf3"
  [(set (match_operand:SF 0 "general_operand" "=fm")
	(div:SF (match_operand:SF 1 "general_operand" "0")
		(match_operand:SF 2 "general_operand" "fmF")))]
  "TARGET_32081"
  "divf %2,%0")

(define_insn "divsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(div:SI (match_operand:SI 1 "general_operand" "0")
		(match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "quod %2,%0")

(define_insn "divhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(div:HI (match_operand:HI 1 "general_operand" "0")
		(match_operand:HI 2 "general_operand" "g")))]
  ""
  "quow %2,%0")

(define_insn "divqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(div:QI (match_operand:QI 1 "general_operand" "0")
		(match_operand:QI 2 "general_operand" "g")))]
  ""
  "quob %2,%0")

(define_insn "udivsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(udiv:SI (subreg:SI (match_operand:DI 1 "general_operand" "0") 0)
		 (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{
  operands[1] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
  return \"deid %2,%0\;movd %1,%0\";
}")

(define_insn "udivhi3"
  [(set (match_operand:HI 0 "register_operand" "=r")
	(udiv:HI (subreg:HI (match_operand:DI 1 "general_operand" "0") 0)
		 (match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{
  operands[1] = gen_rtx (REG, HImode, REGNO (operands[0]) + 1);
  return \"deiw %2,%0\;movw %1,%0\";
}")

(define_insn "udivqi3"
  [(set (match_operand:QI 0 "register_operand" "=r")
	(udiv:QI (subreg:QI (match_operand:DI 1 "general_operand" "0") 0)
		 (match_operand:QI 2 "general_operand" "g")))]
  ""
  "*
{
  operands[1] = gen_rtx (REG, QImode, REGNO (operands[0]) + 1);
  return \"deib %2,%0\;movb %1,%0\";
}")

;; Remainder instructions.

(define_insn "modsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(mod:SI (match_operand:SI 1 "general_operand" "0")
		(match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "remd %2,%0")

(define_insn "modhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(mod:HI (match_operand:HI 1 "general_operand" "0")
		(match_operand:HI 2 "general_operand" "g")))]
  ""
  "remw %2,%0")

(define_insn "modqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(mod:QI (match_operand:QI 1 "general_operand" "0")
		(match_operand:QI 2 "general_operand" "g")))]
  ""
  "remb %2,%0")

(define_insn "umodsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(umod:SI (subreg:SI (match_operand:DI 1 "general_operand" "0") 0)
		 (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "deid %2,%0")

(define_insn "umodhi3"
  [(set (match_operand:HI 0 "register_operand" "=r")
	(umod:HI (subreg:HI (match_operand:DI 1 "general_operand" "0") 0)
		 (match_operand:HI 2 "general_operand" "g")))]
  ""
  "deiw %2,%0")

(define_insn "umodqi3"
  [(set (match_operand:QI 0 "register_operand" "=r")
	(umod:QI (subreg:QI (match_operand:DI 1 "general_operand" "0") 0)
		 (match_operand:QI 2 "general_operand" "g")))]
  ""
  "deib %2,%0")

; This isn't be usable in its current form.
;(define_insn "udivmoddisi4"
;  [(set (subreg:SI (match_operand:DI 0 "general_operand" "=r") 1)
;	(udiv:SI (match_operand:DI 1 "general_operand" "0")
;		 (match_operand:SI 2 "general_operand" "rmn")))
;   (set (subreg:SI (match_dup 0) 0)
;	(umod:SI (match_dup 1) (match_dup 2)))]
;  ""
;  "deid %2,%0")

;;- Logical Instructions: AND

(define_insn "andsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(and:SI (match_operand:SI 1 "general_operand" "%0")
		(match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT) {
    if ((INTVAL (operands[2]) | 0xff) == 0xffffffff)
      if (INTVAL (operands[2]) == 0xffffff00)
        return \"movqb %$0,%0\";
      else {
        INTVAL (operands[2]) &= 0xff;
        return \"andb %2,%0\";
      }
    if ((INTVAL (operands[2]) | 0xffff) == 0xffffffff)
      if (INTVAL (operands[2]) == 0xffff0000)
        return \"movqw %$0,%0\";
      else {
        INTVAL (operands[2]) &= 0xffff;
        return \"andw %2,%0\";
      }
  }
  return \"andd %2,%0\";
}")

(define_insn "andhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(and:HI (match_operand:HI 1 "general_operand" "%0")
		(match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT
      && (INTVAL (operands[2]) | 0xff) == 0xffffffff)
    if (INTVAL (operands[2]) == 0xffffff00)
      return \"movqb %$0,%0\";
    else {
      INTVAL (operands[2]) &= 0xff;
      return \"andb %2,%0\";
    }
  return \"andw %2,%0\";
}")

(define_insn "andqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(and:QI (match_operand:QI 1 "general_operand" "%0")
		(match_operand:QI 2 "general_operand" "g")))]
  ""
  "andb %2,%0")

(define_insn "andcbsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(and:SI (match_operand:SI 1 "general_operand" "0")
		(not:SI (match_operand:SI 2 "general_operand" "rmn"))))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT)
    {
      if ((INTVAL (operands[2]) & 0xffffff00) == 0)
	return \"bicb %2,%0\";
      if ((INTVAL (operands[2]) & 0xffff0000) == 0)
	return \"bicw %2,%0\";
    }
  return \"bicd %2,%0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(and:SI (not:SI (match_operand:SI 1 "general_operand" "rmn"))
		(match_operand:SI 2 "general_operand" "0")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT)
    {
      if ((INTVAL (operands[1]) & 0xffffff00) == 0)
	return \"bicb %1,%0\";
      if ((INTVAL (operands[1]) & 0xffff0000) == 0)
	return \"bicw %1,%0\";
    }
  return \"bicd %1,%0\";
}")

(define_insn "andcbhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(and:HI (match_operand:HI 1 "general_operand" "%0")
		(not:HI (match_operand:HI 2 "general_operand" "g"))))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT
      && (INTVAL (operands[2]) & 0xffffff00) == 0)
    return \"bicb %2,%0\";
  return \"bicw %2,%0\";
}")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g")
	(and:HI (not:HI (match_operand:HI 1 "general_operand" "g"))
		(match_operand:HI 2 "general_operand" "0")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == CONST_INT
      && (INTVAL (operands[1]) & 0xffffff00) == 0)
    return \"bicb %1,%0\";
  return \"bicw %1,%0\";
}")

(define_insn "andcbqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(and:QI (match_operand:QI 1 "general_operand" "%0")
		(not:QI (match_operand:QI 2 "general_operand" "g"))))]
  ""
  "bicb %2,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(and:QI (not:QI (match_operand:QI 1 "general_operand" "g"))
		(match_operand:QI 2 "general_operand" "0")))]
  ""
  "bicb %1,%0")

;;- Bit set instructions.

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(ior:SI (match_operand:SI 1 "general_operand" "%0")
		(match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT) {
    if ((INTVAL (operands[2]) & 0xffffff00) == 0)
      return \"orb %2,%0\";
    if ((INTVAL (operands[2]) & 0xffff0000) == 0)
      return \"orw %2,%0\";
  }
  return \"ord %2,%0\";
}")

(define_insn "iorhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(ior:HI (match_operand:HI 1 "general_operand" "%0")
		(match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE(operands[2]) == CONST_INT &&
      (INTVAL(operands[2]) & 0xffffff00) == 0)
    return \"orb %2,%0\";
  return \"orw %2,%0\";
}")

(define_insn "iorqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(ior:QI (match_operand:QI 1 "general_operand" "%0")
		(match_operand:QI 2 "general_operand" "g")))]
  ""
  "orb %2,%0")

;;- xor instructions.

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(xor:SI (match_operand:SI 1 "general_operand" "%0")
		(match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{
  if (GET_CODE (operands[2]) == CONST_INT) {
    if ((INTVAL (operands[2]) & 0xffffff00) == 0)
      return \"xorb %2,%0\";
    if ((INTVAL (operands[2]) & 0xffff0000) == 0)
      return \"xorw %2,%0\";
  }
  return \"xord %2,%0\";
}")

(define_insn "xorhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(xor:HI (match_operand:HI 1 "general_operand" "%0")
		(match_operand:HI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE(operands[2]) == CONST_INT &&
      (INTVAL(operands[2]) & 0xffffff00) == 0)
    return \"xorb %2,%0\";
  return \"xorw %2,%0\";
}")

(define_insn "xorqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(xor:QI (match_operand:QI 1 "general_operand" "%0")
		(match_operand:QI 2 "general_operand" "g")))]
  ""
  "xorb %2,%0")

(define_insn "negdf2"
  [(set (match_operand:DF 0 "general_operand" "=fm<")
	(neg:DF (match_operand:DF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "negl %1,%0")

(define_insn "negsf2"
  [(set (match_operand:SF 0 "general_operand" "=fm<")
	(neg:SF (match_operand:SF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "negf %1,%0")

(define_insn "negsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(neg:SI (match_operand:SI 1 "general_operand" "rmn")))]
  ""
  "negd %1,%0")

(define_insn "neghi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(neg:HI (match_operand:HI 1 "general_operand" "g")))]
  ""
  "negw %1,%0")

(define_insn "negqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(neg:QI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "negb %1,%0")

(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(not:SI (match_operand:SI 1 "general_operand" "rmn")))]
  ""
  "comd %1,%0")

(define_insn "one_cmplhi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(not:HI (match_operand:HI 1 "general_operand" "g")))]
  ""
  "comw %1,%0")

(define_insn "one_cmplqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(not:QI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "comb %1,%0")

;; arithmetic left and right shift operations

(define_insn "ashlsi3"
  [(set (match_operand:SI 0 "general_operand" "=g,g")
	(ashift:SI (match_operand:SI 1 "general_operand" "r,0")
		   (match_operand:SI 2 "general_operand" "I,rmn")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT
      && INTVAL (operands[2]) > 0
      && INTVAL (operands[2]) <= 3)
    if (GET_CODE (operands[0]) == REG)
      {
	if (GET_CODE (operands[1]) == REG)
	  {
	    if (REGNO (operands[0]) == REGNO (operands[1]))
	      {
		if (operands[2] == const1_rtx)
		  return \"addd %0,%0\";
		else if (INTVAL (operands[2]) == 2)
		  return \"addd %0,%0\;addd %0,%0\";
	      }
	    operands[1] = gen_indexed_expr (const0_rtx, operands[1], operands[2]);
	    return \"addr %a1,%0\";
	  }
	else if (operands[2] == const1_rtx)
	  return \"movd %1,%0\;addd %0,%0\";
      }
    else if (GET_CODE (operands[1]) == REG)
      {
	operands[1] = gen_indexed_expr (const0_rtx, operands[1], operands[2]);
	return \"addr %a1,%0\";
      }
    else if (INTVAL (operands[2]) == 1
	     && GET_CODE (operands[1]) == MEM
	     && rtx_equal_p (operands [0], operands[1]))
      {
	rtx temp = XEXP (operands[1], 0);

	if (GET_CODE (temp) == REG
	    || (GET_CODE (temp) == PLUS
		&& GET_CODE (XEXP (temp, 0)) == REG
		&& GET_CODE (XEXP (temp, 1)) == CONST_INT))
	  return \"addd %0,%0\";
      }
  return \"ashd %2,%0\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(ashift:SI (match_operand:SI 1 "general_operand" "0")
		   (and:SI (match_operand:SI 2 "general_operand" "rmn")
			   (const_int 31))))]
  ""
  "ashd %2,%0")

(define_insn "ashlhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(ashift:HI (match_operand:HI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    if (INTVAL (operands[2]) == 1)
      return \"addw %1,%0\";
    else if (INTVAL (operands[2]) == 2)
      return \"addw %1,%0\;addw %0,%0\";
  return \"ashw %2,%0\";
}")

(define_insn "ashlqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(ashift:QI (match_operand:QI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    if (INTVAL (operands[2]) == 1)
      return \"addb %1,%0\";
    else if (INTVAL (operands[2]) == 2)
      return \"addb %1,%0\;addb %0,%0\";
  return \"ashb %2,%0\";
}")

;; logical shift instructions

(define_insn "lshlsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(lshift:SI (match_operand:SI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "lshd %2,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(lshift:SI (match_operand:SI 1 "general_operand" "0")
		   (and:SI (match_operand:SI 2 "general_operand" "rmn")
			   (const_int 31))))]
  ""
  "lshd %2,%0")

(define_insn "lshlhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(lshift:HI (match_operand:HI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "lshw %2,%0")

(define_insn "lshlqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(lshift:QI (match_operand:QI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "lshb %2,%0")

;; Rotate instructions

(define_insn "rotlsi3"
  [(set (match_operand:SI 0 "general_operand" "=g")
	(rotate:SI (match_operand:SI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "rotd %2,%0")

(define_insn "rotlhi3"
  [(set (match_operand:HI 0 "general_operand" "=g")
	(rotate:HI (match_operand:HI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "rotw %2,%0")

(define_insn "rotlqi3"
  [(set (match_operand:QI 0 "general_operand" "=g")
	(rotate:QI (match_operand:QI 1 "general_operand" "0")
		   (match_operand:SI 2 "general_operand" "rmn")))]
  ""
  "rotb %2,%0")

;;- load or push effective address 
;; These come after the move, add, and multiply patterns
;; because we don't want pushl $1 turned into pushad 1.

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(match_operand:QI 1 "address_operand" "p"))]
  ""
  "addr %a1,%0")

;;; Index insns.  These are about the same speed as multiply-add counterparts.
;;; but slower then using power-of-2 shifts if we can use them
;
;(define_insn ""
;  [(set (match_operand:SI 0 "register_operand" "=r")
;	(plus:SI (match_operand:SI 1 "general_operand" "rmn")
;		 (mult:SI (match_operand:SI 2 "register_operand" "0")
;			  (plus:SI (match_operand:SI 3 "general_operand" "rmn") (const_int 1)))))]
;  "GET_CODE (operands[3]) != CONST_INT || INTVAL (operands[3]) > 8"
;  "indexd %0,%3,%1")
;
;(define_insn ""
;  [(set (match_operand:SI 0 "register_operand" "=r")
;	(plus:SI (mult:SI (match_operand:SI 1 "register_operand" "0")
;			  (plus:SI (match_operand:SI 2 "general_operand" "rmn") (const_int 1)))
;		 (match_operand:SI 3 "general_operand" "rmn")))]
;  "GET_CODE (operands[2]) != CONST_INT || INTVAL (operands[2]) > 8"
;  "indexd %0,%2,%3")

;; Set, Clear, and Invert bit

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(ior:SI
	 (ashift:SI (const_int 1)
		    (match_operand:SI 1 "general_operand" "rmn"))
	 (match_dup 0)))]
  ""
  "sbitd %1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(ior:SI
	 (match_dup 0)
	 (ashift:SI (const_int 1)
		    (match_operand:SI 1 "general_operand" "rmn"))))]
  ""
  "sbitd %1,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(ior:QI
	 (subreg:QI
	  (ashift:SI (const_int 1)
		     (match_operand:QI 1 "general_operand" "rmn")) 0)
	 (match_dup 0)))]
  ""
  "sbitb %1,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(ior:QI
	 (match_dup 0)
	 (subreg:QI
	  (ashift:SI (const_int 1)
		     (match_operand:QI 1 "general_operand" "rmn")) 0)))]
  ""
  "sbitb %1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(and:SI
	 (not:SI
	  (ashift:SI (const_int 1)
		     (match_operand:SI 1 "general_operand" "rmn")))
	 (match_dup 0)))]
  ""
  "cbitd %1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(and:SI
	 (match_dup 0)
	 (not:SI
	  (ashift:SI (const_int 1)
		     (match_operand:SI 1 "general_operand" "rmn")))))]
  ""
  "cbitd %1,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(and:QI
	 (subreg:QI
	  (not:SI
	   (ashift:SI (const_int 1)
		      (match_operand:QI 1 "general_operand" "rmn"))) 0)
	 (match_dup 0)))]
  ""
  "cbitb %1,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(and:QI
	 (match_dup 0)
	 (subreg:QI
	  (not:SI
	   (ashift:SI (const_int 1)
		      (match_operand:QI 1 "general_operand" "rmn"))) 0)))]
  ""
  "cbitb %1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(xor:SI
	 (ashift:SI (const_int 1)
		    (match_operand:SI 1 "general_operand" "rmn"))
	 (match_dup 0)))]
  ""
  "ibitd %1,%0")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g")
	(xor:SI
	 (match_dup 0)
	 (ashift:SI (const_int 1)
		    (match_operand:SI 1 "general_operand" "rmn"))))]
  ""
  "ibitd %1,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(xor:QI
	 (subreg:QI
	  (ashift:SI (const_int 1)
		     (match_operand:QI 1 "general_operand" "rmn")) 0)
	 (match_dup 0)))]
  ""
  "ibitb %1,%0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g")
	(xor:QI
	 (match_dup 0)
	 (subreg:QI
	  (ashift:SI (const_int 1)
		     (match_operand:QI 1 "general_operand" "rmn")) 0)))]
  ""
  "ibitb %1,%0")

;; Recognize jbs and jbc instructions.

(define_insn ""
  [(set (cc0)
	(zero_extract (match_operand:SI 0 "general_operand" "rmn")
		      (const_int 1)
		      (match_operand:SI 1 "general_operand" "rmn")))]
  ""
  "*
{ cc_status.flags = CC_Z_IN_F;
  return \"tbitd %1,%0\";
}")

(define_insn ""
  [(set (cc0)
	(minus (zero_extract (match_operand:SI 0 "general_operand" "rmn")
			     (const_int 1)
			     (match_operand:SI 1 "general_operand" "rmn"))
	       (const_int 1)))]
  ""
  "*
{ cc_status.flags = CC_Z_IN_NOT_F;
  return \"tbitd %1,%0\";
}")

(define_insn ""
  [(set (cc0)
	(zero_extract (match_operand:HI 0 "general_operand" "g")
		      (const_int 1)
		      (match_operand:HI 1 "general_operand" "g")))]
  ""
  "*
{ cc_status.flags = CC_Z_IN_F;
  return \"tbitw %1,%0\";
}")

(define_insn ""
  [(set (cc0)
	(minus (zero_extract (match_operand:HI 0 "general_operand" "g")
			     (const_int 1)
			     (match_operand:HI 1 "general_operand" "rmn"))
	       (const_int 1)))]
  ""
  "*
{ cc_status.flags = CC_Z_IN_NOT_F;
  return \"tbitw %1,%0\";
}")

(define_insn ""
  [(set (cc0)
	(zero_extract (match_operand:QI 0 "general_operand" "g")
		      (const_int 1)
		      (match_operand:QI 1 "general_operand" "g")))]
  ""
  "*
{ cc_status.flags = CC_Z_IN_F;
  return \"tbitb %1,%0\";
}")

(define_insn ""
  [(set (cc0)
	(minus (zero_extract:SI (match_operand:QI 0 "general_operand" "g")
				(const_int 1)
				(match_operand:QI 1 "general_operand" "rmn"))
	       (const_int 1)))]
  ""
  "*
{ cc_status.flags = CC_Z_IN_NOT_F;
  return \"tbitb %1,%0\";
}")

(define_insn ""
  [(set (cc0)
	(and:SI (match_operand:SI 0 "general_operand" "rmn")
		(match_operand:SI 1 "immediate_operand" "i")))]
  "GET_CODE (operands[1]) == CONST_INT
   && exact_log2 (INTVAL (operands[1])) >= 0"
  "*
{
  operands[1]
    = gen_rtx (CONST_INT, VOIDmode, exact_log2 (INTVAL (operands[1])));
  cc_status.flags = CC_Z_IN_F;
  return \"tbitd %1,%0\";
}")

;; extract(base, width, offset)
;; Signed bitfield extraction is not supported in hardware on the
;; NS 32032.  It is therefore better to let GCC figure out a
;; good strategy for generating the proper instruction sequence
;; and represent it as rtl.

(define_insn "extzv"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(zero_extract:SI (match_operand:SI 1 "general_operand" "rmn")
			 (match_operand:SI 2 "const_int" "i")
			 (match_operand:SI 3 "general_operand" "ri")))]
  ""
  "*
{ if (GET_CODE (operands[3]) == CONST_INT)
    return \"extsd %1,%0,%3,%2\";
  else return \"extd %3,%1,%0,%2\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(zero_extract:SI (match_operand:HI 1 "general_operand" "g")
			 (match_operand:SI 2 "const_int" "i")
			 (match_operand:SI 3 "general_operand" "ri")))]
  ""
  "*
{ if (GET_CODE (operands[3]) == CONST_INT)
    return \"extsd %1,%0,%3,%2\";
  else return \"extd %3,%1,%0,%2\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(zero_extract:SI (match_operand:QI 1 "general_operand" "g")
			 (match_operand:SI 2 "const_int" "i")
			 (match_operand:SI 3 "general_operand" "ri")))]
  ""
  "*
{ if (GET_CODE (operands[3]) == CONST_INT)
    return \"extsd %1,%0,%3,%2\";
  else return \"extd %3,%1,%0,%2\";
}")

(define_insn "insv"
  [(set (zero_extract:SI (match_operand:SI 0 "general_operand" "=g")
			 (match_operand:SI 1 "const_int" "i")
			 (match_operand:SI 2 "general_operand" "ri"))
	(match_operand:SI 3 "general_operand" "rmn"))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    if (INTVAL (operands[1]) <= 8)
      return \"inssb %3,%0,%2,%1\";
    else if (INTVAL (operands[1]) <= 16)
      return \"inssw %3,%0,%2,%1\";
    else
      return \"inssd %3,%0,%2,%1\";
  else return \"insd %2,%3,%0,%1\";
}")

(define_insn ""
  [(set (zero_extract:SI (match_operand:HI 0 "general_operand" "=g")
			 (match_operand:SI 1 "const_int" "i")
			 (match_operand:SI 2 "general_operand" "ri"))
	(match_operand:SI 3 "general_operand" "rmn"))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    if (INTVAL (operands[1]) <= 8)
      return \"inssb %3,%0,%2,%1\";
    else if (INTVAL (operands[1]) <= 16)
      return \"inssw %3,%0,%2,%1\";
    else
      return \"inssd %3,%0,%2,%1\";
  else return \"insd %2,%3,%0,%1\";
}")

(define_insn ""
  [(set (zero_extract:SI (match_operand:QI 0 "general_operand" "=g")
			 (match_operand:SI 1 "const_int" "i")
			 (match_operand:SI 2 "general_operand" "ri"))
	(match_operand:SI 3 "general_operand" "rmn"))]
  ""
  "*
{ if (GET_CODE (operands[2]) == CONST_INT)
    if (INTVAL (operands[1]) <= 8)
      return \"inssb %3,%0,%2,%1\";
    else if (INTVAL (operands[1]) <= 16)
      return \"inssw %3,%0,%2,%1\";
    else
      return \"inssd %3,%0,%2,%1\";
  else return \"insd %2,%3,%0,%1\";
}")


(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "br %l0")

(define_insn "beq"
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"bfc %l0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"bfs %l0\";
  else return \"beq %l0\";
}")

(define_insn "bne"
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"bfs %l0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"bfc %l0\";
  else return \"bne %l0\";
}")

(define_insn "bgt"
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bgt %l0")

(define_insn "bgtu"
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bhi %l0")

(define_insn "blt"
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "blt %l0")

(define_insn "bltu"
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "blo %l0")

(define_insn "bge"
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bge %l0")

(define_insn "bgeu"
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bhs %l0")

(define_insn "ble"
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "ble %l0")

(define_insn "bleu"
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "bls %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"bfs %l0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"bfc %l0\";
  else return \"bne %l0\";
}")

(define_insn ""
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"bfc %l0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"bfs %l0\";
  else return \"beq %l0\";
}")

(define_insn ""
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "ble %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "bls %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "bge %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "bhs %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "blt %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "blo %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "bgt %l0")

(define_insn ""
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "bhi %l0")

;; Subtract-and-jump and Add-and-jump insns.
;; These can actually be used for adding numbers in the range -8 to 7

(define_insn ""
  [(set (pc)
	(if_then_else
	 (ne (minus:SI (match_operand:SI 0 "general_operand" "+g")
		       (match_operand:SI 1 "general_operand" "i"))
	     (const_int 0))
	 (label_ref (match_operand 2 "" ""))
	 (pc)))
  (set (match_dup 0)
       (minus:SI (match_dup 0)
		 (match_dup 1)))]
  "GET_CODE (operands[1]) == CONST_INT
   && INTVAL (operands[1]) > -8 && INTVAL (operands[1]) <= 8"
  "acbd %$%n1,%0,%l2")

(define_insn ""
  [(set (pc)
	(if_then_else
	 (ne (plus:SI (match_operand:SI 0 "general_operand" "+g")
		      (match_operand:SI 1 "general_operand" "i"))
	     (const_int 0))
	 (label_ref (match_operand 2 "" ""))
	 (pc)))
  (set (match_dup 0)
       (plus:SI (match_dup 0)
		(match_dup 1)))]
  "GET_CODE (operands[1]) == CONST_INT
   && INTVAL (operands[1]) >= -8 && INTVAL (operands[1]) < 8"
  "acbd %1,%0,%l2")

;; Reversed

(define_insn ""
  [(set (pc)
	(if_then_else
	 (eq (minus:SI (match_operand:SI 0 "general_operand" "+g")
		       (match_operand:SI 1 "general_operand" "i"))
	     (const_int 0))
	 (pc)
	 (label_ref (match_operand 2 "" ""))))
  (set (match_dup 0)
       (minus:SI (match_dup 0)
		 (match_dup 1)))]
  "GET_CODE (operands[1]) == CONST_INT
   && INTVAL (operands[1]) > -8 && INTVAL (operands[1]) <= 8"
  "acbd %$%n1,%0,%l2")

(define_insn ""
  [(set (pc)
	(if_then_else
	 (eq (plus:SI (match_operand:SI 0 "general_operand" "+g")
		      (match_operand:SI 1 "general_operand" "i"))
	     (const_int 0))
	 (pc)
	 (label_ref (match_operand 2 "" ""))))
  (set (match_dup 0)
       (plus:SI (match_dup 0)
		(match_dup 1)))]
  "GET_CODE (operands[1]) == CONST_INT
   && INTVAL (operands[1]) >= -8 && INTVAL (operands[1]) < 8"
  "acbd %1,%0,%l2")

(define_insn "call"
  [(call (match_operand:QI 0 "general_operand" "g")
	 (match_operand:QI 1 "general_operand" "g"))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == MEM)
    {
      if (CONSTANT_ADDRESS_P (XEXP (operands[0], 0)))
	return \"bsr %a0\";
      if (GET_CODE (XEXP (operands[0], 0)) == REG)
        return \"jsr %a0\";
    }
  return \"jsr %0\";
}")

(define_insn "call_value"
  [(set (match_operand 0 "" "fg")
	(call (match_operand:QI 1 "general_operand" "g")
	      (match_operand:QI 2 "general_operand" "g")))]
  ""
  "*
{
  if (GET_CODE (operands[1]) == MEM)
    {
      if (CONSTANT_ADDRESS_P (XEXP (operands[1], 0)))
	return \"bsr %a1\";
      if (GET_CODE (XEXP (operands[1], 0)) == REG)
        return \"jsr %a1\";
    }
  return \"jsr %1\";
}")

(define_insn "return"
  [(return)]
  ""
  "ret 0")

(define_insn "abssf2"
  [(set (match_operand:SF 0 "general_operand" "=fm<")
	(abs:SF (match_operand:SF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "absf %1,%0")

(define_insn "absdf2"
  [(set (match_operand:DF 0 "general_operand" "=fm<")
	(abs:DF (match_operand:DF 1 "general_operand" "fmF")))]
  "TARGET_32081"
  "absl %1,%0")

(define_insn "abssi2"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(abs:SI (match_operand:SI 1 "general_operand" "rmn")))]
  ""
  "absd %1,%0")

(define_insn "abshi2"
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(abs:HI (match_operand:HI 1 "general_operand" "g")))]
  ""
  "absw %1,%0")

(define_insn "absqi2"
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(abs:QI (match_operand:QI 1 "general_operand" "g")))]
  ""
  "absb %1,%0")

;;(define_insn "tablejump"
;;  [(set (pc)
;;	(plus:SI (match_operand:SI 0 "general_operand" "g")
;;		 (pc)))]
;;  ""
;;  "cased %0")

(define_insn "tablejump"
  [(set (pc)
	(plus:SI (pc) (match_operand:HI 0 "general_operand" "g")))
   (use (label_ref (match_operand 1 "" "")))]
  ""
  "*
{
  ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, \"LI\",
			     CODE_LABEL_NUMBER (operands[1]));
  return \"casew %0\";
}")

;;(define_insn ""
;;  [(set (pc)
;;	(plus:SI (match_operand:QI 0 "general_operand" "g")
;;		 (pc)))]
;;  ""
;;  "caseb %0")

;; Scondi instructions
(define_insn "seq"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(eq (cc0) (const_int 0)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"sfcd %0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"sfsd %0\";
  else return \"seqd %0\";
}")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(eq (cc0) (const_int 0)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"sfcw %0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"sfsw %0\";
  else return \"seqw %0\";
}")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(eq (cc0) (const_int 0)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"sfcb %0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"sfsb %0\";
  else return \"seqb %0\";
}")

(define_insn "sne"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(ne (cc0) (const_int 0)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"sfsd %0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"sfcd %0\";
  else return \"sned %0\";
}")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(ne (cc0) (const_int 0)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"sfsw %0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"sfcw %0\";
  else return \"snew %0\";
}")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(ne (cc0) (const_int 0)))]
  ""
  "*
{ if (cc_prev_status.flags & CC_Z_IN_F)
    return \"sfsb %0\";
  else if (cc_prev_status.flags & CC_Z_IN_NOT_F)
    return \"sfcb %0\";
  else return \"sneb %0\";
}")

(define_insn "sgt"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(gt (cc0) (const_int 0)))]
  ""
  "sgtd %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(gt (cc0) (const_int 0)))]
  ""
  "sgtw %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(gt (cc0) (const_int 0)))]
  ""
  "sgtb %0")

(define_insn "sgtu"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(gtu (cc0) (const_int 0)))]
  ""
  "shid %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(gtu (cc0) (const_int 0)))]
  ""
  "shiw %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(gtu (cc0) (const_int 0)))]
  ""
  "shib %0")

(define_insn "slt"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(lt (cc0) (const_int 0)))]
  ""
  "sltd %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(lt (cc0) (const_int 0)))]
  ""
  "sltw %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(lt (cc0) (const_int 0)))]
  ""
  "sltb %0")

(define_insn "sltu"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(ltu (cc0) (const_int 0)))]
  ""
  "slod %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(ltu (cc0) (const_int 0)))]
  ""
  "slow %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(ltu (cc0) (const_int 0)))]
  ""
  "slob %0")

(define_insn "sge"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(ge (cc0) (const_int 0)))]
  ""
  "sged %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(ge (cc0) (const_int 0)))]
  ""
  "sgew %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(ge (cc0) (const_int 0)))]
  ""
  "sgeb %0")

(define_insn "sgeu"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(geu (cc0) (const_int 0)))]
  ""
  "shsd %0")  

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(geu (cc0) (const_int 0)))]
  ""
  "shsw %0")  

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(geu (cc0) (const_int 0)))]
  ""
  "shsb %0")  

(define_insn "sle"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(le (cc0) (const_int 0)))]
  ""
  "sled %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(le (cc0) (const_int 0)))]
  ""
  "slew %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(le (cc0) (const_int 0)))]
  ""
  "sleb %0")

(define_insn "sleu"
  [(set (match_operand:SI 0 "general_operand" "=g<")
	(leu (cc0) (const_int 0)))]
  ""
  "slsd %0")

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=g<")
	(leu (cc0) (const_int 0)))]
  ""
  "slsw %0")

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=g<")
	(leu (cc0) (const_int 0)))]
  ""
  "slsb %0")

;;- Local variables:
;;- mode:emacs-lisp
;;- comment-start: ";;- "
;;- eval: (set-syntax-table (copy-sequence (syntax-table)))
;;- eval: (modify-syntax-entry ?[ "(]")
;;- eval: (modify-syntax-entry ?] ")[")
;;- eval: (modify-syntax-entry ?{ "(}")
;;- eval: (modify-syntax-entry ?} "){")
;;- End:
