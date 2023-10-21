	  title   drvint.asm  Application (INT 7A) interface to XPC driver
	  page	  60,132
;*****************************************************************************
;     This program is the sole property and confidential information of      *
;     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
;     to any third party, without the written prior consent of Tymnet.	     *
;*****************************************************************************
; ------------------------------------------------------------------------
;
;		  X.PC Driver -- ASM interface to 'C'
;
; filename = p_drvint.asm
;
; This file contains the routine XPC_DRIVER, whose address is put into the
; DOS interrupt vector tables.	When an application program issues the
; proper interrupt, this routine is executed.  The routine copies in the area
; pointed to by the ES register value (Extra Segment of caller's X.PC Parameter
; pointer block) and the BX register value (offset within ES of X.PC Parameter
; pointer Block).  The Parameter pointer block contains the addresses of the
; caller's word variables that define the function to be performed. Once the
; Parameter Pointer Block address is set up, the X.PC Driver is called to
; handle the request.  When the Driver returns, finish the interrupt, return.
;
; ------------------------------------------------------------------------
;   Date    Change#   by     Reason
; 06/13/84    00     curt    Initial Generation of Code.
; 08/31/84    01     curt    Released Driver version 1.02.
; ------------------------------------------------------------------------
	  subttl  Local File Declarations
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup
	  extrn   x_driver:near
	  extrn   XPC_DS:word
	  subttl  Routine to call Driver on Application Interrupt
	  page
; ----------------------------------------------------------------------------
; This routine saves the current state of the 8088, sets up the segment regs
; as required by XPC, and then invokes the 'C' driver module.  All registers
; are saved except CS (already done by INT), and SS/SP (borrow callers stack).
; ----------------------------------------------------------------------------
	  public  XPC_DRIVER
XPC_DRIVER proc   far
	  sti
	  push	  ax			;*
	  push	  cx			;**
	  push	  dx			;***
	  push	  si			;****
	  push	  di			;*****
	  push	  ds			;******
	  push	  bp			;*******
	  mov	  bp,sp 		;set stack base
	  mov	  ds,cs:XPC_DS		;set up Data Segment of XPC

	  push	  bx			;********  2nd argument: data offset
	  push	  es			;********* 1st argument: data segment
	  call	  x_driver		;invoke 'C' routine for XPC
	  pop	  es			;*********
	  pop	  bx			;********

	  pop	  bp			;*******
	  pop	  ds			;******
	  pop	  di			;*****
	  pop	  si			;****
	  pop	  dx			;***
	  pop	  cx			;**
	  pop	  ax			;*
	  iret
XPC_DRIVER endp
prog	  ends
	  end
