/* Definitions to make GDB run on a vax under 4.2bsd.
   Copyright (C) 1986, 1987 Free Software Foundation, Inc.

GDB is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GDB General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GDB,
but only under the conditions described in the GDB General Public
License.  A copy of this license is supposed to have been given to you
along with GDB so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GDB, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

#ifndef vax
#define vax
#endif

/* Get rid of any system-imposed stack limit if possible.  */

#define SET_STACK_LIMIT_HUGE

/* Define this if the C compiler puts an underscore at the front
   of external names before giving them to the linker.  */

#define NAMES_HAVE_UNDERSCORE

/* Debugger information will be in DBX format.  */

#define READ_DBX_FORMAT

/* Offset from address of function to start of its code.
   Zero on most machines.  */

#define FUNCTION_START_OFFSET 2

/* Advance PC across any function entry prologue instructions
   to reach some "real" code.  */

#define SKIP_PROLOGUE(pc)   \
{ register int op = read_memory_integer (pc, 1);	\
  if (op == 0x11) pc += 2;  /* skip brb */		\
  if (op == 0x31) pc += 3;  /* skip brw */		\
}

/* Immediately after a function call, return the saved pc.
   Can't always go through the frames for this because on some machines
   the new frame is not set up until the new function executes
   some instructions.  */

#define SAVED_PC_AFTER_CALL(frame) FRAME_SAVED_PC(frame)

/* This is the amount to subtract from u.u_ar0
   to get the offset in the core file of the register values.  */

#define KERNEL_U_ADDR (0x80000000 - (UPAGES * NBPG))

/* Address of end of stack space.  */

#define STACK_END_ADDR (0x80000000 - (UPAGES * NBPG))

/* Stack grows downward.  */

#define INNER_THAN <

/* Sequence of bytes for breakpoint instruction.  */

#define BREAKPOINT {3}

/* Amount PC must be decremented by after a breakpoint.
   This is often the number of bytes in BREAKPOINT
   but not always.  */

#define DECR_PC_AFTER_BREAK 0

/* Nonzero if instruction at PC is a return instruction.  */

#define ABOUT_TO_RETURN(pc) (read_memory_integer (pc, 1) == 04)

/* Return 1 if P points to an invalid floating point value.
   LEN is the length in bytes -- not relevant on the Vax.  */

#define INVALID_FLOAT(p, len) ((*(short *) p & 0xff80) == 0x8000)

/* Say how long (ordinary) registers are.  */

#define REGISTER_TYPE long

/* Number of machine registers */

#define NUM_REGS 17

/* Initializer for an array of names of registers.
   There should be NUM_REGS strings in this initializer.  */

#define REGISTER_NAMES {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "ap", "fp", "sp", "pc", "ps"}

/* Register numbers of various important registers.
   Note that some of these values are "real" register numbers,
   and correspond to the general registers of the machine,
   and some are "phony" register numbers which are too large
   to be actual register numbers as far as the user is concerned
   but do serve to get the desired values when passed to read_register.  */

#define AP_REGNUM 12
#define FP_REGNUM 13		/* Contains address of executing stack frame */
#define SP_REGNUM 14		/* Contains address of top of stack */
#define PC_REGNUM 15		/* Contains program counter */
#define PS_REGNUM 16		/* Contains processor status */

#define REGISTER_U_ADDR(addr, blockend, regno)		\
{ addr = blockend - 0110 + regno * 4;			\
  if (regno == PC_REGNUM) addr = blockend - 8;		\
  if (regno == PS_REGNUM) addr = blockend - 4;		\
  if (regno == FP_REGNUM) addr = blockend - 0120;	\
  if (regno == AP_REGNUM) addr = blockend - 0124;	\
  if (regno == SP_REGNUM) addr = blockend - 20; }

/* Total amount of space needed to store our copies of the machine's
   register state, the array `registers'.  */
#define REGISTER_BYTES (17*4)

/* Index within `registers' of the first byte of the space for
   register N.  */

#define REGISTER_BYTE(N) ((N) * 4)

/* Number of bytes of storage in the actual machine representation
   for register N.  On the vax, all regs are 4 bytes.  */

#define REGISTER_RAW_SIZE(N) 4

/* Number of bytes of storage in the program's representation
   for register N.  On the vax, all regs are 4 bytes.  */

#define REGISTER_VIRTUAL_SIZE(N) 4

/* Largest value REGISTER_RAW_SIZE can have.  */

#define MAX_REGISTER_RAW_SIZE 4

/* Largest value REGISTER_VIRTUAL_SIZE can have.  */

#define MAX_REGISTER_VIRTUAL_SIZE 4

/* Nonzero if register N requires conversion
   from raw format to virtual format.  */

#define REGISTER_CONVERTIBLE(N) 0

/* Convert data from raw format for register REGNUM
   to virtual format for register REGNUM.  */

#define REGISTER_CONVERT_TO_VIRTUAL(REGNUM,FROM,TO)	\
  bcopy ((FROM), (TO), 4);

/* Convert data from virtual format for register REGNUM
   to raw format for register REGNUM.  */

#define REGISTER_CONVERT_TO_RAW(REGNUM,FROM,TO)	\
  bcopy ((FROM), (TO), 4);

/* Return the GDB type object for the "standard" data type
   of data in register N.  */

#define REGISTER_VIRTUAL_TYPE(N) builtin_type_int

/* Extract from an array REGBUF containing the (raw) register state
   a function return value of type TYPE, and copy that, in virtual format,
   into VALBUF.  */

#define EXTRACT_RETURN_VALUE(TYPE,REGBUF,VALBUF) \
  bcopy (REGBUF, VALBUF, TYPE_LENGTH (TYPE))

/* Write into appropriate registers a function return value
   of type TYPE, given in virtual format.  */

#define STORE_RETURN_VALUE(TYPE,VALBUF) \
  write_register_bytes (0, VALBUF, TYPE_LENGTH (TYPE))

/* Extract from an array REGBUF containing the (raw) register state
   the address in which a function should return its structure value,
   as a CORE_ADDR (or an expression that can be used as one).  */

#define EXTRACT_STRUCT_VALUE_ADDRESS(REGBUF) (*(int *)(REGBUF))

/* Describe the pointer in each stack frame to the previous stack frame
   (its caller).  */

/* FRAME_CHAIN takes a frame's nominal address
   and produces the frame's chain-pointer.

   FRAME_CHAIN_COMBINE takes the chain pointer and the frame's nominal address
   and produces the nominal address of the caller frame.

   However, if FRAME_CHAIN_VALID returns zero,
   it means the given frame is the outermost one and has no caller.
   In that case, FRAME_CHAIN_COMBINE is not used.  */

/* In the case of the Vax, the frame's nominal address is the FP value,
   and 12 bytes later comes the saved previous FP value as a 4-byte word.  */

#define FRAME_CHAIN(thisframe)  (read_memory_integer (thisframe + 12, 4))

#define FRAME_CHAIN_VALID(chain, thisframe) \
  (chain != 0 && (FRAME_SAVED_PC (thisframe) >= first_object_file_end))

#define FRAME_CHAIN_COMBINE(chain, thisframe) (chain)

/* Define other aspects of the stack frame.  */

#define FRAME_SAVED_PC(frame) (read_memory_integer (frame + 16, 4))

/* Cannot find the AP register value directly from the FP value.
   Must find it saved in the frame called by this one, or in the AP register
   for the innermost frame.  */
#define FRAME_ARGS_ADDRESS(fi) \
 (((fi).next_frame                                  \
   ? read_memory_integer ((fi).next_frame + 8, 4)   \
   : read_register (AP_REGNUM)))

#define FRAME_LOCALS_ADDRESS(fi) (fi).frame

/* Return number of args passed to a frame.
   Can return -1, meaning no way to tell.  */

#define FRAME_NUM_ARGS(numargs, fi)  \
{ numargs = (0xff & read_memory_integer (FRAME_ARGS_ADDRESS (fi), 1)); }

/* Return number of bytes at start of arglist that are not really args.  */

#define FRAME_ARGS_SKIP 4

/* Put here the code to store, into a struct frame_saved_regs,
   the addresses of the saved registers of frame described by FRAME_INFO.
   This includes special registers such as pc and fp saved in special
   ways in the stack frame.  sp is even more special:
   the address we return for it IS the sp for the next frame.  */

#define FRAME_FIND_SAVED_REGS(frame_info, frame_saved_regs) \
{ register int regnum;     \
  register int regmask = read_memory_integer ((frame_info).frame+4, 4) >> 16; \
  register CORE_ADDR next_addr;     \
  bzero (&frame_saved_regs, sizeof frame_saved_regs);     \
  next_addr = (frame_info).frame + 16;     \
  /* Regmask's low bit is for register 0,     \
     which is the first one that would be pushed.  */     \
  for (regnum = 0; regnum < 12; regnum++, regmask >>= 1)  \
    (frame_saved_regs).regs[regnum] = (regmask & 1) ? (next_addr += 4) : 0;  \
  (frame_saved_regs).regs[SP_REGNUM] = next_addr + 4;  \
  if (read_memory_integer ((frame_info).frame + 4, 4) & 0x20000000)   \
    (frame_saved_regs).regs[SP_REGNUM] += 4 + 4 * read_memory_integer (next_addr + 4, 4);  \
  (frame_saved_regs).regs[PC_REGNUM] = (frame_info).frame + 16;  \
  (frame_saved_regs).regs[FP_REGNUM] = (frame_info).frame + 12;  \
  (frame_saved_regs).regs[AP_REGNUM] = (frame_info).frame + 8;  \
  (frame_saved_regs).regs[PS_REGNUM] = (frame_info).frame + 4;  \
}

/* Things needed for making the inferior call functions.  */

/* Push an empty stack frame, to record the current PC, etc.  */

#define PUSH_DUMMY_FRAME \
{ register CORE_ADDR sp = read_register (SP_REGNUM);\
  register int regnum;				    \
  sp = push_word (sp, 0); /* arglist */		    \
  for (regnum = 11; regnum >= 0; regnum--)	    \
    sp = push_word (sp, read_register (regnum));    \
  sp = push_word (sp, read_register (PC_REGNUM));   \
  sp = push_word (sp, read_register (FP_REGNUM));   \
  sp = push_word (sp, read_register (AP_REGNUM));   \
  sp = push_word (sp, (read_register (PS_REGNUM) & 0xffef)   \
		      + 0x2fff0000);		    \
  sp = push_word (sp, 0); 			    \
  write_register (SP_REGNUM, sp);		    \
  write_register (FP_REGNUM, sp);		    \
  write_register (AP_REGNUM, sp + 17 * sizeof (int)); }

/* Discard from the stack the innermost frame, restoring all registers.  */

#define POP_FRAME  \
{ register CORE_ADDR fp = read_register (FP_REGNUM);		 \
  register int regnum;						 \
  register int regmask = read_memory_integer (fp + 4, 4);	 \
  write_register (PS_REGNUM, 					 \
		  (regmask & 0xffff)				 \
		  | (read_register (PS_REGNUM) & 0xffff0000));	 \
  write_register (PC_REGNUM, read_memory_integer (fp + 16, 4));  \
  write_register (FP_REGNUM, read_memory_integer (fp + 12, 4));  \
  write_register (AP_REGNUM, read_memory_integer (fp + 8, 4));   \
  fp += 16;							 \
  for (regnum = 0; regnum < 12; regnum++)			 \
    if (regmask & (0x10000 << regnum))				 \
      write_register (regnum, read_memory_integer (fp += 4, 4)); \
  fp = fp + 4 + ((regmask >> 30) & 3);				 \
  if (regmask & 0x20000000)					 \
    { regnum = read_memory_integer (fp, 4);			 \
      fp += (regnum + 1) * 4; }					 \
  write_register (SP_REGNUM, fp);				 \
  set_current_frame (read_register (FP_REGNUM)); }

/* This sequence of words is the instructions
     calls #69, @#32323232
     bpt
   Note this is 8 bytes.  */

#define CALL_DUMMY {0x329f69fb, 0x03323232}

#define CALL_DUMMY_START_OFFSET 0  /* Start execution at beginning of dummy */

/* Insert the specified number of args and function address
   into a call sequence of the above form stored at DUMMYNAME.  */

#define FIX_CALL_DUMMY(dummyname, fun, nargs)   \
{ *((char *) dummyname + 1) = nargs;		\
  *(int *)((char *) dummyname + 3) = fun; }

/* Interface definitions for kernel debugger KDB.  */

/* Map machine fault codes into signal numbers.
   First subtract 0, divide by 4, then index in a table.
   Faults for which the entry in this table is 0
   are not handled by KDB; the program's own trap handler
   gets to handle then.  */

#define FAULT_CODE_ORIGIN 0
#define FAULT_CODE_UNITS 4
#define FAULT_TABLE    \
{ 0, SIGKILL, SIGSEGV, 0, 0, 0, 0, 0, \
  0, 0, SIGTRAP, SIGTRAP, 0, 0, 0, 0, \
  0, 0, 0, 0, 0, 0, 0, 0}

/* Start running with a stack stretching from BEG to END.
   BEG and END should be symbols meaningful to the assembler.
   This is used only for kdb.  */

#define INIT_STACK(beg, end)  \
{ asm (".globl end");         \
  asm ("movl $ end, sp");      \
  asm ("clrl fp"); }

/* Push the frame pointer register on the stack.  */
#define PUSH_FRAME_PTR        \
  asm ("pushl fp");

/* Copy the top-of-stack to the frame pointer register.  */
#define POP_FRAME_PTR  \
  asm ("movl (sp), fp");

/* After KDB is entered by a fault, push all registers
   that GDB thinks about (all NUM_REGS of them),
   so that they appear in order of ascending GDB register number.
   The fault code will be on the stack beyond the last register.  */

#define PUSH_REGISTERS        \
{ asm ("pushl 8(sp)");        \
  asm ("pushl 8(sp)");        \
  asm ("pushal 0x14(sp)");    \
  asm ("pushr $037777"); }

/* Assuming the registers (including processor status) have been
   pushed on the stack in order of ascending GDB register number,
   restore them and return to the address in the saved PC register.  */

#define POP_REGISTERS      \
{ asm ("popr $037777");    \
  asm ("subl2 $8,(sp)");   \
  asm ("movl (sp),sp");    \
  asm ("rei"); }
