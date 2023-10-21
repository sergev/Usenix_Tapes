	 TITLE	 comint.asm  Asynchronous Interrupt Handling Routines
	 page	 60,132
	 subttl  Local File declarations and Include files
;*****************************************************************************
;     This program is the sole property and confidential information of      *
;     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
;     to any third party, without the written prior consent of Tymnet.	     *
;*****************************************************************************
; ------------------------------------------------------------------------
;	 Asynchronous Interrupt Handling Routines
;
; filename = p_comint.asm
;
; The routines in this module handle the Interrupts generated by the 8250
; Asynchronous Communication chip.  The main Interrupt handler is ASYNC_INT,
; it is invoked when the 8259a PIC receives an indication from the 8250.
; The routines in the module include:
;
; ASYNC_INT is invoked each time the 8259a device receives an interrupt from
;	    8250 attached to the current port.	Handles line and modem status
;	    changes, inputs bytes waiting in the receive port, and outputs
;	    bytes to the transmitter port.  This routine's address is placed
;	    in the DOS interrupt vector tables by the 'load_com' routine.
; ------------------------------------------------------------------------
;   Date     by     Reason
; 10/31/84  curt    Finally got interrupt problems fixed in 8250.
;		    All fixes noted with INS-DOC-8250-x are described
;		    in National Document B01361, 'problem-solution'.
; 12/19/84  Mike    ASYNC_INT add AND to prevent wild jump
;  1/10/85  Mike    Remove dis/re-enable on every interrupt.  Only receive
;		    errors do it; unless xmit is active, which waits for
;		    next xmit interrupt.
;  1/16/85  Mike    Insert delay after IN and OUT instructions for IBM PC/AT
; ------------------------------------------------------------------------
; This object file has a public character variable, which is where the byte
; input from the comm. line on an Async. interrupt is stored.
; ----------------------------------------------------------------------------
dgroup	 group	 data
data	 segment word public 'data'
	 assume  DS:dgroup
	 extrn	 mdm_mask:word		;modem status required to send
	 extrn	 port_addr:word
	 public  nest_com		;used to initialize comm interrupts
	 public  nest_inp		;used to initialize input interrupts
	 public  ring_bot, ring_fill, ring_empty	;used by comm init
ring_bot  db	 256 dup ('r')          ;ring buffer (at 9600 baud, 26.6 ms)
ring_top  dw	 ring_top		;pointer to top of ring
ring_fill dw	 ring_bot		;pointer to input ring for filling
ring_empty dw	 ring_bot		;pointer to input ring for emptying
xpc_stack dw	 252 dup ('ss')         ;XPC's stack for PC or PC/XT
stk_top   dw	 0			;XPC's stack for PC/AT (different push)
nest_com  dw	 -1			;nested interrupt count for comm
nest_inp  dw	 -1			;nested interrupt count for comm input
xout_cnt  dw	 0			;retry count if xmit status bad
reg_ss	  dw	 0
reg_sp	  dw	 0			;to save or restore stack pointer
reg_dx	  dw	 0
reg_ax	  dw	 0			;to save oe restore on new stack
dumb_int  dw	 0
real_int  dw	 0			;count real interrupts
data	 ends
; ----------------------------------------------------------------------------
; The current port and register addresses variables are declared within the
; 'pgroup' segment, so declare pgroup before include files and extrn declares.
; ----------------------------------------------------------------------------
pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup
; ----------------------------------------------------------------------------
; These extrns are used by the asynchronous routines to handle interrupts.
; ----------------------------------------------------------------------------
	  extrn   XPC_DS:word		;DS register for interrupt-driven code.
	  extrn   strt_ouframe:near	;C routine to process end-of-output
	  extrn   byte_in:near		;C routine accepts byte read from line.
	  extrn   byte_out:near 	;C routine returns byte to send.
	  extrn   pmdm_chg:near 	;C routine gets modem changes.
	  extrn   plnk_chg:near 	;C routine gets Line changes.
	  extrn   xmit_byt:near 	;asm routine sends a byte to comm port
	  extrn   fix_xmit:near 	;asm routine to get xmit buffer empty
	  extrn   xmit_fail:near	;C routine tells PAD layer of failure
	  extrn   abort:near		;asm routine stops if input ring fills

	  include p_intequ.ad		;EQUs and MACROs for interrupt proc.
	  include p_lnkmdm.ae		;Link and Modem routine declarations
	  subttl  Asynchronous I/O Interrupt Handler
	  page
; ----------------------------------------------------------------------------
; This routine is invoked when the 8250 tells the 8259 something has happened
; at the I/O ports, or the line or modem status has changed since last read.
; ----------------------------------------------------------------------------
inp_struc struc 			;comm intr base pointer for XPC stack
proc_enbl dw	?			;1 if input in ring, 4 if output complt
inp_struc ends
;
	  public  ASYNC_INT
	  public  ring_in		;used by p_chario.c for pseudo receive
ASYNC_INT proc	  far
	  cli				;no interrupt until status/data in/out
	  push	  ds			;*	interrupted task's stack
	  mov	  ds,cs:XPC_DS		;load addr of XPC_DS variable
	  mov	  reg_dx,dx
	  mov	  reg_ax,ax
;enable 8259 controller interrupts, BEFORE input of source.
	  mov	  al,EOI_8259		;Send an EOI command to the 8259,
	  out	  EOI_port,al		;release it to handle other ints.
	  BLD_DXP INT_SRCE		;DX points to Interrupt Source reg.
	  in	  al,dx 		;find out about interrupt
	  test	  al,1			;is there an interrupt source?
	  jz	  aproc_int		;yes, process async interrupt
	  inc	  dumb_int		;DEBUG!  Why is this happening?
async_ex: mov	  ax,reg_ax
	  mov	  dx,reg_dx
	  pop	  ds			;*	interrupted task's stack
	  iret				;return, reload flags to enable interr
ASYNC_INT endp
;
; Jump table for source of interrupt
;					      PRIORITY
ASYNC_JMP dw	  pgroup:MDM_STATUS	; 0   lowest
	  dw	  pgroup:XMIT_READY	; 2
	  dw	  pgroup:RECV_READY	; 4
	  dw	  pgroup:LINE_ERROR	; 6   highest
;
; Process async interrupt source(s).  8259 is enabled, but are interrupts off
;
aproc_int proc	  near			;already saved: DS, DX, AX
	  inc	  real_int
	  mov	  reg_sp,sp		;save old SP -- ax, dx unchgd --+
	  mov	  reg_ss,ss		;save old SS			|
	  inc	  nest_com		;was XPC interrupted?		|
	  jnz	  sameseg		;yes, use same stack		|
	  mov	  ss,cs:XPC_DS		;load addr of XPC_DS variable	|
	  lea	  sp,stk_top		;      and new SP stack 	|
	  page
sameseg:  push	  reg_ss		;*	old SS			|
	  push	  reg_sp		;**	old SP			|
	  push	  reg_dx		;***				|
	  push	  reg_ax		;****				|
	  push	  bx			;*****				|
	  push	  cx			;****** 			|
	  push	  si			;*******			|
	  push	  di			;********			|
	  push	  es			;*********			|
	  push	  bp			;**********			|
	  sub	  sp,2			;***********	  proc_enbl	|
	  mov	  bp,sp 		;set the stack base addr	|
	  mov	  proc_enbl[bp],0	;set bit to process after enable|
					;				|
dup_srce: and	  ax,6			;insurance --- ax, dx unchgd ---+
	  mov	  bx,ax 		;use bx for indexed call
	  push	  dx			;************
	  call	  word ptr pgroup:ASYNC_JMP[bx] ;call handler at address
	  pop	  dx			;************
	  in	  al,dx 		;find out about interrupt: if al=1,
	  test	  al,1			;is another interrupt type avail?
	  jz	  dup_srce		;yes, process another
	  sti				;all interrupt-sources are processed
	  test	  proc_enbl[bp],4	;end of output to process after enable?
	  jz	  not_out		;no - try input
	  call	  strt_ouframe		;process output, when end-of-frame
not_out:  test	  proc_enbl[bp],1	;CRC1/2 input to process after enable?
	  jz	  not_inp		;no - return
	  call	  recv_byte		;process input, for every byte

not_inp:  add	  sp,2			;***********
	  cli				;don't let interrupt change reg_xx
	  pop	  bp			;**********
	  pop	  es			;*********
	  pop	  di			;********
	  pop	  si			;*******
	  pop	  cx			;******
	  pop	  bx			;*****
	  pop	  reg_ax		;****
	  pop	  reg_dx		;***
	  pop	  reg_sp		;**	old SP
	  pop	  ss			;*	old SS
	  mov	  sp,reg_sp		;old stack pointer
	  dec	  nest_com		;use XPC data segment
	  jmp	  async_ex		;exit.	reload DS, DX, AX later
	  subttl  Line error, Byte Input/Output Routines
	  page
; ----------------------------------------------------------------------------
; This reads the Modem Status Reg., and calls Driver routine to update fields.
; ----------------------------------------------------------------------------
MDM_STATUS:
	  call	  rep_mdst		;put in AX the modem status reg.
	  push	  ax
	  and	  ax,0A0h		;use only CD,DSR (must not go off)
	  or	  mdm_mask,ax		;set them when they come on, for xmit
	  call	  pmdm_chg		;call update routine.
	  pop	  ax
	  ret

; ----------------------------------------------------------------------------
; This does immediate STI for the byte_out routine, calls handler, then EOI.
; ----------------------------------------------------------------------------
XMIT_READY:
	  INC_DXP INT_SRCE, MDM_STAT	;need to check modem-status register
	  in	  al,dx
	  and	  ax,mdm_mask		;use none, DSR and CD after logon
	  cmp	  ax,mdm_mask		;are all bits on?
	  je	  mdm_ok		;yes, send to modem
	  inc	  xout_cnt		;add 1 to failed xmit output
	  cmp	  xout_cnt,1		;is this the first time?
	  jne	  only1 		;no - don't report again
	  call	  xmit_fail		;yes, tell C-language that xmit failed
only1:	  xor	  ax,ax 		;zero char
	  push	  ax
	  call	  xmit_byt		;write a NULL to the bit bucket
	  pop	  ax
	  jmp	  short no_send 	;common return
mdm_ok:   DEC_DXP MDM_STAT, LIN_STAT	;need to check line-status register
	  in	  al,dx 		;read line status
	  and	  al,20h		;is TX holding register empty?
	  jz	  no_send		;no - clear by read Intr Id Reg (done)
	  call	  byte_out		;yes, clear by xmit a byte, or disable
	  or	  proc_enbl[bp],ax	;output to process after enable int
no_send:  ret
;---------------- MODEM is not ready!!!

;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine reads the Line Status Register, and informs C routine of change.
; ----------------------------------------------------------------------------
LINE_ERROR:
	  call	  rep_lkst	       ; put Line Status Register in AX.
	  push	  ax
	  call	  plnk_chg	       ; call update routine to handle it.
	  pop	  ax
	  ret

; ----------------------------------------------------------------------------
; line_error's plnk_chg calls ring_in to store white char into the input ring.
; INTERRUPTS ARE DISABLED.
; ----------------------------------------------------------------------------
cstruct   struc
	  dw	  ?			;proc_enbl = 1 if input (not nested)
	  dw	  ?			;old_bp base pointer to stack
	  dw	  ?			;old_ip return address
param	  dw	  ?			;parameter is char "received"
cstruct   ends
;
ring_in:  push	  bp			;get SP into BP to reference stack
	  sub	  sp,2			;proc_enbl
	  mov	  bp,sp
	  mov	  ax,param[bp]		;load C language parameter
	  call	  ring_sto		;store char "received" into ring
	  add	  sp,2			;proc_enbl
	  dec	  nest_inp		;turn off this flag set by ring_sto
	  pop	  bp
	  ret
; ----------------------------------------------------------------------------
; This routine reads the byte, stores it in the Input Ring.  After EOI to
; 8259 interrupt controller, move from the Input Ring to the Receive Buffer.
; INTERRUPTS ARE DISABLED.
; ----------------------------------------------------------------------------
RECV_READY:
	  DEC_DXP INT_SRCE, IO_PORT	;DX ok, decr to back up to I/O port.
	  in	  al,dx 		;input next char
ring_sto: mov	  si,ring_fill		;load the ring fill pointer
	  mov	  0[si],al		;put the byte into the input ring
	  inc	  si			;increment out index
	  cmp	  si,ring_top		;are we beyond top of ring buffer?
	  jne	  ok_fill		;no - keep pointer
	  lea	  si,ring_bot		;yes, wrap pointer
ok_fill:  mov	  ring_fill,si		;store the "incremented" pointer
	  cmp	  si,ring_empty 	;is the ring completely full?
	  jne	  not_full		;no - empty pointer is keeping up
	  call	  abort 		;yes, quit and use a larger ring
;nested interrupts must not take data from input ring, only primary interrupt
not_full: inc	  nest_inp		;is this a nested input interrupt?
	  jg	  is_nested		;yes, process the byte in primary intr
	  or	  proc_enbl[bp],1	;no - process after enabling interrupts
is_nested:ret

; ----------------------------------------------------------------------------
; This is done AFTER interrupts are enabled.  Load all bytes received into
; the input ring, and put them into Receive bufferlets.
; INTERRUPTS ARE ENABLED.
; ----------------------------------------------------------------------------
recv_byte:cli				;no interrupts, please
	  mov	  si,ring_empty 	;load the ring empty pointer
	  cmp	  si,ring_fill		;is anything there?
	  je	  no_data		;no - empty.  EXIT WITH INTRRUPTS OFF!
	  mov	  al,0[si]		;yes, get the byte from the input ring
	  inc	  si			;increment out index
	  cmp	  si,ring_top		;are we beyond top of ring buffer?
	  jne	  ok_empty		;no - keep pointer
	  lea	  si,ring_bot		;yes, wrap pointer
ok_empty: mov	  ring_empty,si 	;store the "incremented" pointer
; process the byte from the ring buffer
	  sti				;set interrupts enabled
	  xor	  ah,ah 		;clear upper half of ax-reg
	  push	  ax			;C Language expects arguement on stack
	  call	  byte_in		;will enque data, no process end of frm
	  pop	  ax
	  jmp	  recv_byte		;process all bytes in ring.
no_data:  mov	  nest_inp,-1		;clear (can't decrm, line_error sets it)
	  sti				;enable interrupts
	  ret				;return

aproc_int endp
prog	  ends
	  end