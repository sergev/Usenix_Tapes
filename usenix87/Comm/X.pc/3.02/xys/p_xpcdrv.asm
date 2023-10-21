	  title   xpcdrv.asm  Loading module to setup Driver for X.PC device
	  page	  60,132
;*****************************************************************************
;     This program is the sole property and confidential information of      *
;     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
;     to any third party, without the written prior consent of Tymnet.	     *
;*****************************************************************************
;
;	   Loading module to setup Driver for X.PC device
;
; filename = p_xpcdrv.asm
;
; This module is the startup code required to load a X.PC device driver
; into memory, perform initialization tasks required to make it accessible
; from a seperately running program, and then exit to the DOS monitor (while
; remaining loaded in memory).
;
;  'pgroup' PUBLIC data elements:
;
; XPC_DS    dw			       ; value of segment registers required,
;
;   Date    Change#   by     Reason
; 05/13/84     00    curt    Initial Generation of code.
; 06/14/84     01    curt    Version updated for release.
; 09/18/86     02    Bobc    Deal with any old value of DRIVRI_VEC interrupt
;****************************************************************************

	    include p_intequ.ad ; For Application interrupt and related values
	    extrn   ver_string:far

TERM_RES    EQU     27h 	; DOS interrupt to terminate and stay resident
;
;
; The group statements control the order in which the segments are placed in
; memory. base, prog, and tail are code segments; data and stack are data segs.
;
pgroup	  group   base,prog,tail
dgroup	  group   data,stack
	  assume  cs:pgroup, ds:dgroup, ss:dgroup

; ----------------------------------------------------------------------------
; The following code segment serves only to force "pgroup" lower in memory.
; NOTE that the size of this variable must be taken into account with the ORG.
; ----------------------------------------------------------------------------
base	  segment 'prog'
base_org  label   near
base	  ends



; ----------------------------------------------------------------------------
; The data segment is declared here to make the group instruction happy.
; All 'C' code data items will exist in the data segment, as part of dgroup.
; ----------------------------------------------------------------------------
data	  segment para public 'data'
	  public  unused
unused	  label   word
data	  ends
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The stack segment defines the base (lowest address) of the stack.  Note
; that in this module it is NOT defined to be of the STACK combining type,
; thus allowing a .COM file to be made of the resulting .EXE file.  All calls
; made to the driver through interrupts will use the caller's stack.
; ----------------------------------------------------------------------------
stack	  segment para 'data'
STK_WORDS EQU	  1028
sbase	  dw	  'st'
stack	  ends



; ----------------------------------------------------------------------------
; The tail segment allows this module to determine the paragraph at which the
; data segment begins, without requiring use of a relocatable value.
; The 'XPC_DS' variable will be used by interrupt-driver routines to find DS.
; ----------------------------------------------------------------------------
tail	  segment word 'prog'
	  public  XPC_DS
XPC_DS	  dw	  0
last	  db	  0		  ;last byte in PGROUP
tail	  ends


; ----------------------------------------------------------------------------
; The prog segment consists of all of the assembled and compiled code that
; implements the X.PC system.  'drv_init' is 'C' code that controls loading.
; ----------------------------------------------------------------------------
prog	  segment byte public 'prog'
	  public  XPCMAIN
	  extrn   drv_init:near
	  public  abort 		;bufque aborts if duplicate queue entry
	  org	  0			;Program Segment Prefix ===> addr 0
abort	  label   near			;contains DOS int 20 (terminate)
	  org	  100h			;to satisfy 'exe2bin'   ===> addr 100h
	  subttl  Main X.PC Routine for Load Phase
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The main routine sets up segment and stack registers for load routine, which
; initializes the X.PC driver.	The program terminates, buts stays resident.
; First set up segment registers, and create the stack for the load logic.
; Stack will be in memory beyond program space, will be truncated after load.
; ----------------------------------------------------------------------------
XPCMAIN   proc	  far
	  cli
	  push	  ds

	  mov	  ax,GET_VECT*256+DRIVRI_VEC	;Get 7A vector table contents
	  int	  DOS_CALL			;  "please"    (in ES and BX)
	  mov	  ax,es 			;  and look for any old ones.
	  and	  bx,ax 			;Assume none if either is =0.
	  jz	  no_apvec

	  mov	  si,offset ver_string
	  mov	  di,si
	  mov	  cx,10h			;for a selected length...
	  push	  cs				;address the strings we will
	  pop	  ds				;try to match up due to vector
	  repz	  cmpsb
	  jcxz	  ys_drver
	  jmp	  no_drver			;If mismatch, driver is needed

drvr_msg  db	"Interrupt vector 7Ah is already in use."
	  db	 07h,0Dh,0Ah
	  db	"Driver is not loaded."
	  db	 0Dh,0Ah,24h

no_drver  label   near
	  mov	  dx,offset drvr_msg   ;String mismatch w/this version of
	  mov	  ah,09 	       ;code so send an error message, quit.
	  int	  DOS_CALL

ys_drver  label   near
	  pop	  ds		       ;No loading is to occur anyway.
	  int	  20h		       ;Go away forever(per this .COM)

no_apvec  label   near
	  pop	  ds
	  lea	  ax,pgroup:last       ; get offset of last byte.
	  add	  ax,16
	  mov	  cl,4
	  shr	  ax,cl 	       ; number of paragraphs in PGROUP.
	  mov	  bx,cs
	  add	  ax,bx 	       ; base of DGROUP
	  mov	  ds,ax 	       ; initialize DS and SS, following CS.
	  mov	  ss,ax

	  lea	  bx,dgroup:sbase
	  add	  bx,(STK_WORDS*2)     ; add room for n word stack.
	  mov	  sp,bx 	       ; set stack pointer
	  sti
;
; Now that have set up segments, save off DS for interrupting routines.
; Call loader module to do "C" level initialization of the Driver.
;
	  mov	  ax,ds 	       ; save off DS value.
	  lea	  bx,pgroup:XPC_DS
	  mov	  pgroup:[bx],ax       ; in pgroup variable.
	  call	  drv_init	       ; call main module for Setup of driver.
;
; The 'terminate and stay resident' logic must tell DOS the Code Segment offset
; of first byte beyond X.PC driver.  Find out how many bytes are in use in
; the code segment, then add to that the address of the 'sbase' variable.  The
; 'sbase' variable will be the first byte on a paragraph boundary beyond the
; end of the Data Segment (due to 'para' alignment).  The stack area is no
; longer used by the Driver, so the first byte of that area is first available.
;
	  mov	  dx,cs 	       ; get segment of code
	  mov	  ax,ds 	       ; get segment of data
	  sub	  ax,dx 	       ; find difference in segments
	  mov	  cl,4
	  shl	  ax,cl 	       ; treat diff. as number of bytes in CS.
	  lea	  dx,dgroup:sbase      ; find start of stack, no longer needed.
	  add	  dx,ax 	       ; add number of bytes in Code Segment.
	  int	  TERM_RES	       ; terminate and stay resident.
	  ret			       ; return to MS-DOS ???

XPCMAIN   endp
prog	  ends
	  end	  XPCMAIN
