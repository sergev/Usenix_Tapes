;;- Machine description for SPUR chip for GNU C compiler
;;   Copyright (C) 1988 Free Software Foundation, Inc.

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


;;- See file "rtl.def" for documentation on define_insn, match_*, et. al.

;;- cpp macro #define NOTICE_UPDATE_CC in file tm.h handles condition code
;;- updates for most instructions.

;;- Operand classes for the register allocator:

;; Compare instructions.
;; This pattern is used for generating an "insn"
;; which does just a compare and sets a (fictitious) condition code.

;; The actual SPUR insns are compare-and-conditional-jump.
;; The define_peephole's below recognize the combinations of
;; compares and jumps, and output each pair as a single assembler insn.

;; This controls RTL generation and register allocation.
(define_insn "cmpsi"
  [(set (cc0)
	(minus (match_operand:SI 0 "nonmemory_operand" "rK")
	       (match_operand:SI 1 "nonmemory_operand" "rK")))]
  ""
  "*
{
  cc_status.value1 = operands[0], cc_status.value2 = operands[1];
  return \"\";
}")

;; We have to have this because cse can optimize the previous pattern
;; into this one.

(define_insn "tstsi"
  [(set (cc0)
	(match_operand:SI 0 "register_operand" "r"))]
  ""
  "*
{
  cc_status.value1 = operands[0], cc_status.value2 = const0_rtx;
  return \"\";
}")


;; These control RTL generation for conditional jump insns
;; and match them for register allocation.

(define_insn "beq"
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"eq\", \"eq\"); ")

(define_insn "bne"
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"ne\", \"ne\"); ")

(define_insn "bgt"
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"gt\", \"lt\"); ")

(define_insn "bgtu"
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"ugt\", \"ult\"); ")

(define_insn "blt"
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"lt\", \"gt\"); ")

(define_insn "bltu"
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"ult\", \"ugt\"); ")

(define_insn "bge"
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"ge\", \"le\"); ")

(define_insn "bgeu"
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"uge\", \"ule\"); ")

(define_insn "ble"
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"le\", \"ge\"); ")

(define_insn "bleu"
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "* return output_compare (operands, \"ule\", \"uge\"); ")

;; These match inverted jump insns for register allocation.

(define_insn ""
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"ne\", \"ne\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"eq\", \"eq\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"le\", \"ge\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"ule\", \"uge\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"ge\", \"le\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"uge\", \"ule\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"lt\", \"gt\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"ult\", \"ugt\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"gt\", \"lt\"); ")

(define_insn ""
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "* return output_compare (operands, \"ugt\", \"ult\"); ")

;; Move instructions

(define_insn "movsi"
  [(set (match_operand:SI 0 "general_operand" "=r,m")
	(match_operand:SI 1 "general_operand" "rmi,rJ"))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == MEM)
    return \"st_32 %r1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    return \"ld_32 %0,%1\;nop\";
  if (GET_CODE (operands[1]) == REG)
    return \"add_nt %0,%1,$0\";
  if (GET_CODE (operands[1]) == SYMBOL_REF && operands[1]->unchanging)
    return \"add_nt %0,r24,$(%1-0b)\";
  return \"add_nt %0,r0,%1\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(mem:SI (plus:SI (match_operand:SI 1 "register_operand" "r")
			 (match_operand:SI 2 "register_operand" "r"))))]
  ""
  "ld_32 %0,%1,%2\;nop")

;; Generate insns for moving single bytes.

(define_expand "movqi"
  [(set (match_operand:QI 0 "general_operand" "")
	(match_operand:QI 1 "general_operand" ""))]
  ""
  "
{
  if (GET_CODE (operands[0]) == MEM && GET_CODE (operands[1]) == MEM)
    operands[1] = copy_to_reg (operands[1]);

  if (GET_CODE (operands[1]) == MEM)
    {
      rtx tem = gen_reg_rtx (SImode);
      rtx addr = force_reg (SImode, XEXP (operands[1], 0));
      emit_move_insn (tem, gen_rtx (MEM, SImode, addr));
      emit_insn (gen_rtx (SET, VOIDmode,
			  gen_rtx (SUBREG, SImode, operands[0], 0),
			  gen_rtx (ZERO_EXTRACT, SImode, tem,
				   gen_rtx (CONST_INT, VOIDmode, 8),
				   addr)));
    }
  else if (GET_CODE (operands[0]) == MEM)
    {
      rtx tem = gen_reg_rtx (SImode);
      rtx addr = force_reg (SImode, XEXP (operands[0], 0));
      emit_move_insn (tem, gen_rtx (MEM, SImode, addr));
      if (! CONSTANT_ADDRESS_P (operands[1]))
	operands[1] = gen_rtx (SUBREG, SImode, operands[1], 0);
      emit_insn (gen_rtx (SET, VOIDmode,
			  gen_rtx (ZERO_EXTRACT, SImode, tem,
				   gen_rtx (CONST_INT, VOIDmode, 8),
				   addr),
			  operands[1]));
      emit_move_insn (gen_rtx (MEM, SImode, addr), tem);
    }
  else
    {
      emit_insn (gen_rtx (SET, VOIDmode, operands[0], operands[1]));
    }
  DONE;
}")

;; Recognize insns generated for moving single bytes.

(define_insn ""
  [(set (match_operand:QI 0 "general_operand" "=r,m")
	(match_operand:QI 1 "general_operand" "rmi,r"))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == MEM)
    return \"st_32 %1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    return \"ld_32 %0,%1\;nop\";
  if (GET_CODE (operands[1]) == REG)
    return \"add_nt %0,%1,$0\";
  return \"add_nt %0,r0,%1\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(zero_extract:SI (match_operand:SI 1 "register_operand" "r")
			 (const_int 8)
			 (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "extract %0,%1,%2")

(define_insn ""
  [(set (zero_extract:SI (match_operand:SI 0 "register_operand" "+r")
			 (const_int 8)
			 (match_operand:SI 1 "nonmemory_operand" "rI"))
	(match_operand:SI 2 "nonmemory_operand" "ri"))]
  ""
  "wr_insert %1\;insert %0,%0,%2")

;; Constant propagation can optimize the previous pattern into this pattern.
;[Not any more.  It could when the position-operand contains a MULT.]

;(define_insn ""
;  [(set (zero_extract:QI (match_operand:SI 0 "register_operand" "+r")
;			 (const_int 8)
;			 (match_operand:SI 1 "immediate_operand" "I"))
;	(match_operand:QI 2 "register_operand" "r"))]
;  "GET_CODE (operands[1]) == CONST_INT
;   && INTVAL (operands[1]) % 8 == 0
;   && (unsigned) INTVAL (operands[1]) < 32"
;  "*
;{
;  operands[1] = gen_rtx (CONST_INT, VOIDmode, INTVAL (operands[1]) / 8);
;  return \"wr_insert 0,0,%1\;insert %0,%0,%2\";
;}")

;; The three define_expand patterns on this page
;; serve as subroutines of "movhi".

;; Generate code to fetch an aligned halfword from memory.
;; Operand 0 is the destination register (HImode).
;; Operand 1 is the memory address (SImode).
;; Operand 2 is a temporary (SImode).
;; Operand 3 is a temporary (SImode).
;; Operand 4 is a temporary (QImode).

(define_expand "loadhi"
  [(set (match_operand:SI 2 "register_operand" "")
	(mem:SI (match_operand:SI 1 "register_operand" "")))
   ;; Extract the low byte.
   (set (subreg:SI (match_dup 5) 0)
	(zero_extract:SI (match_dup 2) (const_int 8) (match_dup 1)))
   ;; Form address of high byte.
   (set (match_operand:SI 3 "register_operand" "")
	(plus:SI (match_dup 1) (const_int 1)))
   ;; Extract the high byte.
   (set (subreg:SI (match_operand:QI 4 "register_operand" "") 0)
	(zero_extract:SI (match_dup 2) (const_int 8) (match_dup 3)))
   ;; Put the high byte in with the low one.
   (set (zero_extract:SI (match_dup 5) (const_int 8) (const_int 1))
	(subreg:SI (match_dup 4) 0))
   (set (match_operand:HI 0 "register_operand" "") (match_dup 5))]
  ""
  "operands[5] = gen_reg_rtx (HImode);")

;; Generate code to store an aligned halfword into memory.
;; Operand 0 is the destination address (SImode).
;; Operand 1 is the source register (HImode, not constant).
;; Operand 2 is a temporary (SImode).
;; Operand 3 is a temporary (SImode).
;; Operand 4 is a temporary (QImode).

(define_expand "storehi"
  [(set (match_operand:SI 2 "register_operand" "")
	(mem:SI (match_operand:SI 0 "register_operand" "")))
   ;; Insert the low byte.
   (set (zero_extract:SI (match_dup 2) (const_int 8) (match_dup 0))
	(subreg:SI (match_operand:HI 1 "register_operand" "") 0))
   ;; Form address of high byte.
   (set (match_operand:SI 3 "register_operand" "")
	(plus:SI (match_dup 0) (const_int 1)))
   ;; Extract the high byte from the source.
   (set (subreg:SI (match_operand:QI 4 "register_operand" "") 0)
	(zero_extract:SI (match_dup 1) (const_int 8) (const_int 1)))
   ;; Store high byte into the memory word
   (set (zero_extract:SI (match_dup 2) (const_int 8) (match_dup 3))
	(subreg:SI (match_dup 4) 0))
   ;; Put memory word back into memory.
   (set (mem:SI (match_dup 0))
	(match_dup 2))]
  ""
  "")

;; Like storehi but operands[1] is a CONST_INT.

(define_expand "storeinthi"
  [(set (match_operand:SI 2 "register_operand" "")
	(mem:SI (match_operand:SI 0 "register_operand" "")))
   ;; Insert the low byte.
   (set (zero_extract:SI (match_dup 2) (const_int 8) (match_dup 0))
	(match_dup 5))
   ;; Form address of high byte.
   (set (match_operand:SI 3 "register_operand" "")
	(plus:SI (match_dup 0) (const_int 1)))
   ;; Store high byte into the memory word
   (set (zero_extract:SI (match_dup 2) (const_int 8) (match_dup 3))
	(match_dup 6))
   ;; Put memory word back into memory.
   (set (mem:SI (match_dup 0))
	(match_dup 2))]
  ""
  " operands[5] = gen_rtx (CONST_INT, VOIDmode, INTVAL (operands[1]) & 255);
    operands[6] = gen_rtx (CONST_INT, VOIDmode,
			   (INTVAL (operands[1]) >> 8) & 255);
")

;; Main entry for generating insns to move halfwords.

(define_expand "movhi"
  [(set (match_operand:HI 0 "general_operand" "")
	(match_operand:HI 1 "general_operand" ""))]
  ""
  "
{
  if (GET_CODE (operands[0]) == MEM && GET_CODE (operands[1]) == MEM)
    operands[1] = copy_to_reg (operands[1]);
  
  if (GET_CODE (operands[1]) == MEM)
    {
      rtx insn =
	emit_insn (gen_loadhi (operands[0],
			       force_reg (SImode, XEXP (operands[1], 0)),
			       gen_reg_rtx (SImode), gen_reg_rtx (SImode),
			       gen_reg_rtx (QImode)));
      /* Tell cse what value the loadhi produces, so it detect duplicates.  */
      REG_NOTES (insn) = gen_rtx (EXPR_LIST, REG_EQUAL, operands[1], 0);
    }
  else if (GET_CODE (operands[0]) == MEM)
    {
      if (GET_CODE (operands[1]) == CONST_INT)
	emit_insn (gen_storeinthi (force_reg (SImode, XEXP (operands[0], 0)),
			           operands[1],
				   gen_reg_rtx (SImode), gen_reg_rtx (SImode),
				   gen_reg_rtx (QImode)));
      else
	{
	  if (CONSTANT_P (operands[1]))
            operands[1] = force_reg (HImode, operands[1]);
	  emit_insn (gen_storehi (force_reg (SImode, XEXP (operands[0], 0)),
				  operands[1],
				  gen_reg_rtx (SImode), gen_reg_rtx (SImode),
				  gen_reg_rtx (QImode)));
	}
    }
  else
    emit_insn (gen_rtx (SET, VOIDmode, operands[0], operands[1]));
  DONE;
}")

;; Recognize insns generated for moving halfwords.
;; (Note that the extract and insert patterns for single-byte moves
;;  are also involved in recognizing some of the insns used for this purpose.)

(define_insn ""
  [(set (match_operand:HI 0 "general_operand" "=r,m")
	(match_operand:HI 1 "general_operand" "rmi,r"))]
  ""
  "*
{
  if (GET_CODE (operands[0]) == MEM)
    return \"st_32 %1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    return \"ld_32 %0,%1\;nop\";
  if (GET_CODE (operands[1]) == REG)
    return \"add_nt %0,%1,$0\";
  return \"add_nt %0,r0,%1\";
}")

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(zero_extract:SI (match_operand:HI 1 "register_operand" "r")
			 (const_int 8)
			 (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "extract %0,%1,%2")

(define_insn ""
  [(set (zero_extract:SI (match_operand:HI 0 "register_operand" "+r")
			 (const_int 8)
			 (match_operand:SI 1 "nonmemory_operand" "rI"))
	(match_operand:SI 2 "nonmemory_operand" "ri"))]
  ""
  "wr_insert %1\;insert %0,%0,%2")

;; Constant propagation can optimize the previous pattern into this pattern.

;(define_insn ""
;  [(set (zero_extract:QI (match_operand:HI 0 "register_operand" "+r")
;			 (const_int 8)
;			 (match_operand:SI 1 "immediate_operand" "I"))
;	(match_operand:QI 2 "register_operand" "r"))]
;  "GET_CODE (operands[1]) == CONST_INT
;   && INTVAL (operands[1]) % 8 == 0
;   && (unsigned) INTVAL (operands[1]) < 32"
;  "*
;{
;  operands[1] = gen_rtx (CONST_INT, VOIDmode, INTVAL (operands[1]) / 8);
;  return \"wr_insert 0,0,%1\;insert %0,%0,%2\";
;}")

;; This pattern forces (set (reg:DF ...) (const_double ...))
;; to be reloaded by putting the constant into memory.
;; It must come before the more general movdf pattern.
(define_insn ""
  [(set (match_operand:DF 0 "general_operand" "=r,f,o")
	(match_operand:DF 1 "" "mG,m,G"))]
  "GET_CODE (operands[1]) == CONST_DOUBLE"
  "*
{
  if (FP_REG_P (operands[0]))
    return output_fp_move_double (operands);
  if (operands[1] == dconst0_rtx && GET_CODE (operands[0]) == REG)
    {
      operands[1] = gen_rtx (REG, SImode, REGNO (operands[0]) + 1);
      return \"add_nt %0,r0,$0\;add_nt %1,r0,$0\";
    }
  if (operands[1] == dconst0_rtx && GET_CODE (operands[0]) == MEM)
    {
      operands[1] = adj_offsetable_operand (operands[0], 4);
      return \"st_32 r0,%0\;st_32 r0,%1\";
    }
  return output_move_double (operands);
}
")
  
(define_insn "movdf"
  [(set (match_operand:DF 0 "general_operand" "=r,m,?f,?rm")
	(match_operand:DF 1 "general_operand" "rm,r,rfm,f"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]) || FP_REG_P (operands[1]))
    return output_fp_move_double (operands);
  return output_move_double (operands);
}
")

(define_insn "movdi"
  [(set (match_operand:DI 0 "general_operand" "=r,m,?f,?rm")
	(match_operand:DI 1 "general_operand" "rm,r,rfm,f"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]) || FP_REG_P (operands[1]))
    return output_fp_move_double (operands);
  return output_move_double (operands);
}
")

(define_insn "movsf"
  [(set (match_operand:SF 0 "general_operand" "=rf,m")
	(match_operand:SF 1 "general_operand" "rfm,rf"))]
  ""
  "*
{
  if (FP_REG_P (operands[0]))
    {
      if (FP_REG_P (operands[1]))
	return \"fmov %0,%1\";
      if (GET_CODE (operands[1]) == REG)
	{
	  rtx xoperands[2];
	  int offset = - get_frame_size () - 8;
	  xoperands[1] = operands[1];
	  xoperands[0] = gen_rtx (CONST_INT, VOIDmode, offset);
	  output_asm_insn (\"st_32 %1,r25,%0\", xoperands);
	  xoperands[1] = operands[0];
	  output_asm_insn (\"ld_sgl %1,r25,%0\;nop\", xoperands);
	  return \"\";
	}
      return \"ld_sgl %0,%1\;nop\";
    }
  if (FP_REG_P (operands[1]))
    {
      if (GET_CODE (operands[0]) == REG)
	{
	  rtx xoperands[2];
	  int offset = - get_frame_size () - 8;
	  xoperands[0] = gen_rtx (CONST_INT, VOIDmode, offset);
	  xoperands[1] = operands[1];
	  output_asm_insn (\"st_sgl %1,r25,%0\", xoperands);
	  xoperands[1] = operands[0];
	  output_asm_insn (\"ld_32 %1,r25,%0\;nop\", xoperands);
	  return \"\";
	}
      return \"st_sgl %1,%0\";
    }
  if (GET_CODE (operands[0]) == MEM)
    return \"st_32 %r1,%0\";
  if (GET_CODE (operands[1]) == MEM)
    return \"ld_32 %0,%1\;nop\";
  if (GET_CODE (operands[1]) == REG)
    return \"add_nt %0,%1,$0\";
  return \"add_nt %0,r0,%1\";
}")

;;- truncation instructions
;; I think these are unnecessary, since subreg will be used instead.
;(define_insn "truncsiqi2"
;  [(set (match_operand:QI 0 "register_operand" "=r")
;	(truncate:QI
;	 (match_operand:SI 1 "register_operand" "r")))]
;  ""
;  "add_nt, %0,%1,$0")
;
;(define_insn "trunchiqi2"
;  [(set (match_operand:QI 0 "register_operand" "=r")
;	(truncate:QI
;	 (match_operand:HI 1 "register_operand" "r")))]
;  ""
;  "add_nt, %0,%1,$0")
;
;(define_insn "truncsihi2"
;  [(set (match_operand:HI 0 "register_operand" "=r")
;	(truncate:HI
;	 (match_operand:SI 1 "register_operand" "r")))]
;  ""
;  "add_nt, %0,%1,$0")

;;- zero extension instructions

;; Note that the one starting from HImode comes before those for QImode
;; so that a constant operand will match HImode, not QImode.
(define_expand "zero_extendhisi2"
  [(set (match_operand:SI 0 "register_operand" "")
	(and:SI (subreg:SI (match_operand:HI 1 "register_operand" "") 0)
		;; This constant is invalid, but reloading will handle it.
		;; It's useless to generate here the insns to construct it
		;; because constant propagation would simplify them anyway.
		(match_dup 2)))]
  ""
  "  operands[2] = force_reg (SImode, gen_rtx (CONST_INT, VOIDmode, 65535)); ")

(define_insn "zero_extendqihi2"
  [(set (match_operand:HI 0 "register_operand" "=r")
	(zero_extend:HI
	 (match_operand:QI 1 "register_operand" "r")))]
  ""
  "extract %0,%1,$0")

(define_insn "zero_extendqisi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(zero_extend:SI
	 (match_operand:QI 1 "register_operand" "r")))]
  ""
  "extract %0,%1,$0")

;;- sign extension instructions
;; Note that the one starting from HImode comes before those for QImode
;; so that a constant operand will match HImode, not QImode.

(define_expand "extendhisi2"
  [(set (match_dup 2)
	(and:SI (subreg:SI (match_operand:HI 1 "register_operand" "") 0)
		(match_dup 4)))
   (set (match_dup 3) (plus:SI (match_dup 2) (match_dup 5)))
   (set (match_operand:SI 0 "register_operand" "")
	(xor:SI (match_dup 3) (match_dup 5)))]
  ""
  "
{
  operands[2] = gen_reg_rtx (SImode);
  operands[3] = gen_reg_rtx (SImode);
  operands[4] = force_reg (SImode, gen_rtx (CONST_INT, VOIDmode, 65535));
  operands[5] = force_reg (SImode, gen_rtx (CONST_INT, VOIDmode, -32768));
}")

(define_expand "extendqihi2"
  [(set (match_dup 2)
	(and:HI (subreg:HI (match_operand:QI 1 "register_operand" "") 0)
		(const_int 255)))
   (set (match_dup 3)
	(plus:SI (match_dup 2) (const_int -128)))
   (set (match_operand:HI 0 "register_operand" "")
	(xor:SI (match_dup 3) (const_int -128)))]
  ""
  "
{
  operands[2] = gen_reg_rtx (HImode);
  operands[3] = gen_reg_rtx (HImode);
}")

(define_expand "extendqisi2"
  [(set (match_dup 2)
	(and:SI (subreg:SI (match_operand:QI 1 "register_operand" "") 0)
		(const_int 255)))
   (set (match_dup 3) (plus:SI (match_dup 2) (const_int -128)))
   (set (match_operand:SI 0 "register_operand" "")
	(xor:SI (match_dup 3) (const_int -128)))]
  ""
  "
{
  operands[2] = gen_reg_rtx (SImode);
  operands[3] = gen_reg_rtx (SImode);
}")

;;- arithmetic instructions

(define_insn "addsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(plus:SI (match_operand:SI 1 "nonmemory_operand" "%r")
		 (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "add %0,%1,%2")

(define_insn "subsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(minus:SI (match_operand:SI 1 "register_operand" "r")
		  (match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "sub %0,%1,%2")

(define_insn "andsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(and:SI (match_operand:SI 1 "nonmemory_operand" "%r")
		(match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "and %0,%1,%2")

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ior:SI (match_operand:SI 1 "nonmemory_operand" "%r")
		(match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "or %0,%1,%2")

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(xor:SI (match_operand:SI 1 "nonmemory_operand" "%r")
		(match_operand:SI 2 "nonmemory_operand" "rI")))]
  ""
  "xor %0,%1,%2")

(define_insn "negsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(neg:SI (match_operand:SI 1 "nonmemory_operand" "rI")))]
  ""
  "sub %0,r0,%1")

(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "register_operand" "=r")
	(not:SI (match_operand:SI 1 "register_operand" "r")))]
  ""
  "xor %0,%1,$-1")

;; Floating point arithmetic instructions.

(define_insn "adddf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(plus:DF (match_operand:DF 1 "register_operand" "f")
		 (match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fadd %0,%1,%2")

(define_insn "addsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(plus:SF (match_operand:SF 1 "register_operand" "f")
		 (match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fadd %0,%1,%2")

(define_insn "subdf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(minus:DF (match_operand:DF 1 "register_operand" "f")
		  (match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fsub %0,%1,%2")

(define_insn "subsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(minus:SF (match_operand:SF 1 "register_operand" "f")
		  (match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fsub %0,%1,%2")

(define_insn "muldf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(mult:DF (match_operand:DF 1 "register_operand" "f")
		 (match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fmul %0,%1,%2")

(define_insn "mulsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(mult:SF (match_operand:SF 1 "register_operand" "f")
		 (match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fmul %0,%1,%2")

(define_insn "divdf3"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(div:DF (match_operand:DF 1 "register_operand" "f")
		(match_operand:DF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fdiv %0,%1,%2")

(define_insn "divsf3"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(div:SF (match_operand:SF 1 "register_operand" "f")
		(match_operand:SF 2 "register_operand" "f")))]
  "TARGET_FPU"
  "fdiv %0,%1,%2")

(define_insn "negdf2"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(neg:DF (match_operand:DF 1 "nonmemory_operand" "f")))]
  "TARGET_FPU"
  "fneg %0,%1")

(define_insn "negsf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(neg:SF (match_operand:SF 1 "nonmemory_operand" "f")))]
  "TARGET_FPU"
  "fneg %0,%1")

(define_insn "absdf2"
  [(set (match_operand:DF 0 "register_operand" "=f")
	(abs:DF (match_operand:DF 1 "nonmemory_operand" "f")))]
  "TARGET_FPU"
  "fabs %0,%1")

(define_insn "abssf2"
  [(set (match_operand:SF 0 "register_operand" "=f")
	(abs:SF (match_operand:SF 1 "nonmemory_operand" "f")))]
  "TARGET_FPU"
  "fabs %0,%1")

;; Shift instructions

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ashift:SI (match_operand:SI 1 "register_operand" "r")
		   (match_operand:SI 2 "immediate_operand" "I")))]
  "GET_CODE (operands[2]) == CONST_INT
   && (unsigned) INTVAL (operands[2]) <= 3"
  "sll %0,%1,%2")

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(ashiftrt:SI (match_operand:SI 1 "register_operand" "r")
		     (const_int 1)))]
  ""
  "sra %0,%1,$1")

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=r")
	(lshiftrt:SI (match_operand:SI 1 "register_operand" "r")
		     (const_int 1)))]
  ""
  "srl %0,%1,$1")

(define_expand "ashlsi3"
  [(set (match_operand:SI 0 "register_operand" "")
	(ashift:SI (match_operand:SI 1 "register_operand" "")
		   (match_operand:SI 2 "nonmemory_operand" "")))]
  ""
  "
{
  if (GET_CODE (operands[2]) != CONST_INT
      || (unsigned) INTVAL (operands[2]) > 3)
    FAIL;
}")

(define_expand "lshlsi3"
  [(set (match_operand:SI 0 "register_operand" "")
	(ashift:SI (match_operand:SI 1 "register_operand" "")
		   (match_operand:SI 2 "nonmemory_operand" "")))]
  ""
  "
{
  if (GET_CODE (operands[2]) != CONST_INT
      || (unsigned) INTVAL (operands[2]) > 3)
    FAIL;
}")

(define_expand "ashrsi3"
  [(set (match_operand:SI 0 "register_operand" "")
	(ashiftrt:SI (match_operand:SI 1 "register_operand" "")
		     (match_operand:SI 2 "nonmemory_operand" "")))]
  ""
  "
{
  if (GET_CODE (operands[2]) != CONST_INT
      || (unsigned) INTVAL (operands[2]) > 1)
    FAIL;
}")

(define_expand "lshrsi3"
  [(set (match_operand:SI 0 "register_operand" "")
	(lshiftrt:SI (match_operand:SI 1 "register_operand" "")
		     (match_operand:SI 2 "nonmemory_operand" "")))]
  ""
  "
{
  if (GET_CODE (operands[2]) != CONST_INT
      || (unsigned) INTVAL (operands[2]) > 1)
    FAIL;
}")

;; Unconditional and other jump instructions
(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "jump %l0\;nop")

(define_insn "tablejump"
  [(set (pc) (match_operand:SI 0 "register_operand" "r"))
   (use (label_ref (match_operand 1 "" "")))]
  ""
  "jump_reg r0,%0\;nop")

;;- jump to subroutine
(define_insn "call"
  [(call (match_operand:SI 0 "memory_operand" "m")
	 (match_operand:SI 1 "general_operand" "g"))]
  ;;- Don't use operand 1 for most machines.
  ""
  "add_nt r9,%0\;call .+8\;jump_reg r0,r9\;nop")

(define_insn "call_value"
  [(set (match_operand 0 "" "g")
	(call (match_operand:SI 1 "memory_operand" "m")
	      (match_operand:SI 2 "general_operand" "g")))]
  ;;- Don't use operand 1 for most machines.
  ""
  "add_nt r9,%1\;call .+8\;jump_reg r0,r9\;nop")

;; A memory ref with constant address is not normally valid.
;; But it is valid in a call insns.  This pattern allows the
;; loading of the address to combine with the call.
(define_insn ""
  [(call (mem:SI (match_operand:SI 0 "" "i"))
	 (match_operand:SI 1 "general_operand" "g"))]
  ;;- Don't use operand 1 for most machines.
  "GET_CODE (operands[0]) == SYMBOL_REF"
  "call %0\;nop")

(define_insn ""
  [(set (match_operand 0 "" "g")
	(call (mem:SI (match_operand:SI 1 "" "i"))
	      (match_operand:SI 2 "general_operand" "g")))]
  ;;- Don't use operand 1 for most machines.
  "GET_CODE (operands[1]) == SYMBOL_REF"
  "call %1\;nop")

;;- Local variables:
;;- mode:emacs-lisp
;;- comment-start: ";;- "
;;- eval: (set-syntax-table (copy-sequence (syntax-table)))
;;- eval: (modify-syntax-entry ?[ "(]")
;;- eval: (modify-syntax-entry ?] ")[")
;;- eval: (modify-syntax-entry ?{ "(}")
;;- eval: (modify-syntax-entry ?} "){")
;;- End:
