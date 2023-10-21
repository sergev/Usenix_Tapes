	  title   APLINT.ASM  Application's Interrupt to PAD routine
	  page	  60,132
; ------------------------------------------------------------------------
;		  Application's Interrupt to PAD routine
;
; filename = p_aplint.asm
;
; xpcinvok    accepts a pointer within the current data segment.  It places
;	      the pointer in BX, and places the current data segment in ES.
;	      It then invokes the X.PC PAD driver.
; xpccheck    If the X.PC vector is zero, it returns FALSE to the caller.
; ------------------------------------------------------------------------
;   Date    Change#   by     Reason
; 06/14/84    00     curt    Initial generation of code.
; 11/28/84    01     curt    Split xpccheck out from xpcinvok routine.
; ------------------------------------------------------------------------
DRIVER_VEC  EQU  7Ah
DOS	    equ  21h
GET_VECT    equ  35h		;DOS	  function

pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup

	  public  xpcinvok
xpcinvok  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es
	  push	  bx

	  mov	  ax,ds
	  mov	  es,ax
	  mov	  bx,[bp+4]		;only one param, above bp,ip.
	  int	  DRIVER_VEC		;call XPC "driver"

	  pop	  bx
	  pop	  es
	  pop	  bp
	  sti				;make sure those INTs are on!!
	  ret
xpcinvok  endp

	  subttl  vector check routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  public  xpccheck
xpccheck  proc	  near
	  push	  es

	  mov	  ax,get_vect*256+DRIVER_VEC
	  int	  DOS
	  mov	  ax,es
	  cmp	  ax,0			;has xpc "driver" been loaded?
	  je	  done			;if vector address is zero, FALSE.
	  mov	  ax,1			;address non-zero, return TRUE.

done:	  pop	  es
	  ret
xpccheck  endp


; ----------------------------------------------------------------------------
;    Routine to return current Data Segment register value
; ----------------------------------------------------------------------------

	  public  get_ds
get_ds	  proc	  near
	  mov	  ax,ds
	  ret
get_ds	  endp

prog	  ends
          end
