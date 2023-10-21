	  title   timint.asm  Timer Interrupt -- Clock-Driven ASM code
	  page	  60,132
	  subttl  Local File Data Declarations (within PGROUP)
; This module contains the routine required to implement the timer and
; timeouts for the X.PC system.  The primary routine is TIMER_TICK, a FAR
; routine whose address is placed in the DOS interrupt vector tables by the
; INIT_VEC routine, thereby causing it to be invoked each system timer tick.
;
; TIMER_TICK  is called by ROM BIOS' INT 1C, which happens 18.2 times per sec.
;	      It simply sets up the stack to return to xpc_timer, which runs
;	      as foreground code, with interrupts enabled.
; xpc_timer   is invoked each time the ROM BIOS does and IRET (after timer_
;	      tick's stack modification) and calls 'timevent' for X.PC timers.

dgroup	  group   data
data	  segment byte public 'data'
	  assume  DS:dgroup
	  public  nest_tim		;used to initialize time interrupts
xpc_stack dw	  254 dup ('tt')        ;XPC's stack for PC or PC/XT
stk_top   dw	  0			;XPC's stack for PC/AT (different push)
nest_tim  dw	  -1			;nested interrupt count for timer/comm
iret_ip   dw	  0		;any_ip <--- iret to task; repl by foreground
iret_cs   dw	  0		;any_cs
iret_flg  dw	  0		;any_flg
reg_ds	  dw	 0			;to save or restore 3 registers
reg_ss	  dw	 0
reg_sp	  dw	 0			;to save or restore stack pointer
data	  ends

pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup
	  public  TIMER_TICK
; ------------------------------------------------------------------------
; timevent is 'C' code invoked when timeout unit elapses.
; The XPC_DS word is allocated within PGROUP, must override segment.
; ------------------------------------------------------------------------
	  extrn   XPC_DS:word
	  extrn   timevent:near
	  include p_intequ.ad		    ; contains vector # defines.
	  subttl  Foreground Timer Routine (ROM BIOS returns here)
	  page
; xpc_timer   is invoked each time the ROM BIOS does and IRET (after timer_
;	      tick's stack modification) and calls 'timevent' for X.PC timers.
; TIMER_TICK is invoked each time the DOS system timer is triggered (about
; 18.2 times per second), it sets up its own stack, calls the original int 1C,
; calls X.PC's timevent and returns to the interrupted task.
;
xpc_timer proc	  far			;intr OFF by timer tick
	  push	  ds
	  mov	  ds,cs:XPC_DS		;load addr of XPC_DS variable
	  pop	  reg_ds
	  push	  iret_flg		;from the ROM BIOS timer interrupt...
	  push	  iret_cs		;task's PSW put back on the stack
	  push	  iret_ip		;
	  push	  reg_ds		;*	interrupted task's stack
	  mov	  reg_sp,sp		;save old SP
	  mov	  reg_ss,ss		;save old SS
	  inc	  nest_tim		;was XPC interrupted?
	  jnz	  sameseg		;yes, use same stack
	  mov	  ss,cs:XPC_DS		;load addr of XPC_DS variable
	  lea	  sp,stk_top		;      and new SP stack
sameseg:  push	  reg_ss		;*	old SS
	  push	  reg_sp		;**	old SP
	  push	  dx			;***
	  push	  ax			;****
	  push	  bx			;*****
	  push	  cx			;******
	  push	  si			;*******
	  push	  di			;********
	  push	  es			;*********
	  push	  bp			;**********
	  mov	  bp,sp 		;set the stack base address
	  sti				;OK to enable interrupts
				;using XPC's stack, call the old INT 21 timeout
	  int	  TIMER2_VEC		;run chained-to timer: if is only
	  call	  timevent		;handles XPC's sys service timer

	  cli				;don't let interrupt change reg_xx
	  pop	  bp			;**********
	  pop	  es			;*********
	  pop	  di			;********
	  pop	  si			;*******
	  pop	  cx			;******
	  pop	  bx			;*****
	  pop	  ax			;****
	  pop	  dx			;***
	  pop	  reg_sp		;**	old SP
	  pop	  ss			;*	old SS
	  mov	  sp,reg_sp		;old stack pointer
	  dec	  nest_tim		;use XPC data segment
	  pop	  ds			;*	interrupted task's stack
	  iret				;return to original task
xpc_timer endp
	  subttl  Timer Tick Routine (called by ROM BIOS)
	  page
; TIMER_TICK  is called by ROM BIOS' INT 1C, which happens 18.2 times per sec.
;	      It saves the interrupted task's PSW, then replaces it with XPC's
;	      foreground PSW (interrupts must be off).	Forground will push
;	      the interrupted task's PSW on to the stack for a subsequent iret.
;	      Thus, this code returns to BIOS, which returns to XPC foreground,
;	      which returns to the interrupted task.  And foreground runs with
;	      interrupts ON, unlike conventional timeout logic.
;
stkstruc  struc 		;original stack
irom_ip   dw	  ?		;	<--- iret to ROM BIOS
irom_cs   dw	  ?		;
irom_flg  dw	  ?		;
regs_	  dw	  ?		;	<--- regs DX, AX, DS
	  dw	  ?		;
	  dw	  ?		;
task_ip   dw	  ?		;any_ip <--- iret to task; repl by foreground
task_cs   dw	  ?		;any_cs
task_flg  dw	  ?		;any_flg
stkstruc  ends

TIMER_TICK proc   far
; ROM BIOS has saved DS, AX and DX	;***
	  mov	  dx,bp 		;save bp in dx
	  mov	  bp,sp 		;set new bp
	  mov	  ds,cs:XPC_DS		;load DS address
; replace ROM BIOS' return address, to make it go to XPC's xpc_timer
	  lea	  ax,pgroup:xpc_timer	;put xpc_timer address...
	  xchg	  ax,task_ip[bp]	;into ROM BIOS' return, and
	  mov	  iret_ip,ax		;save for foreground.
	  mov	  ax,cs 		;put xpc_timer code segment...
	  xchg	  ax,task_cs[bp]	;into ROM BIOS' return, and
	  mov	  iret_cs,ax		;save for foreground.
	  mov	  ax,0F002h		;foreground intr OFF until stack ready..
	  xchg	  ax,task_flg[bp]	;into ROM BIOS return, and
	  mov	  iret_flg,ax		;save for foreground.
	  mov	  ax,NOT 0200h		;BIOS ROM interrupts off (in flags)..
	  and	  irom_flg[bp],ax	;to prevent interrupt using DOS stack
; restore registers and return
	  mov	  bp,dx 		;restore bp from dx
; ROM BIOS will restore DS, AX and DX	;***
	  iret				;return to forground xpc_time
TIMER_TICK endp
prog	  ends
	  end
