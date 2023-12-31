	  TITLE   comutl.asm  Communication Utility Routines
	  page	  60,132
;*****************************************************************************
;     This program is the sole property and confidential information of      *
;     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
;     to any third party, without the written prior consent of Tymnet.	     *
;*****************************************************************************
;
; filename = p_comutl.asm
;
; All the routines in this file use the 'curr_port' and 'port_addr' values
; generated by the 'load_com' routine, so that routine must be invoked FIRST.
; The routines in the module include:
;
; end_intr  Sends an 'End of Interrupt' command to the 8259 device.
;
; xmit_enb  After this routine has executed, 'Transmitter Buffer Empty' ints
;	    will cause the 8250 to indicate an interrupt to the 8259.
; xmit_dsb  After this routine has executed, 'Transmitter Buffer Empty' ints
;	    will be ignored by the 8250, and subsequently by the 8259.
; recv_enb  After this routine has executed, 'Data Ready' and 'Line Error' ints
;	    will cause the 8250 to indicate an interrupt to the 8259.
; recv_dsb  After this routine has executed, 'Data Ready' and 'Line Error' ints
;	    will be ignored by the 8250, and subsequently by the 8259.
;
; xmit_on   returns TRUE if transmit interrupts are enabled, FALSE if not.
; recv_on   returns TRUE if receive interrupts are enabled, FALSE if not.
;
; xmit_byt  Accepts a word as parameter, and moves the low byte of that word
;	    into the output port of the 8250 Async. Device.
; ------------------------------------------------------------------------
;   Date     by     Reason
; 05/22/84  curt    Initial Generation of Code.
; 05/29/84  curt    'recv_enb/dsb' now also enable/disable line errors.
; 06/01/84  curt    'xmit_enb' and 'xmit_byt' working better.
; 06/12/84  curt    'xmit_byt' needed to wait for xmit buffer empty.
; 08/31/84  curt    removed extra push/pop, clean up for Release 1.02.
; 10/31/84  curt    Finally got interrupt problems fixed in 8250.
;		    All fixes noted with INS-DOC-8250-x are described
;		    in National Document B01361, 'problem-solution'.
; 12/19/84  Mike    ASYNC_INT add AND to prevent wild jump
;  1/10/84  Mike    Remove dis/re-enable on every interrupt.  Only receive
;		    errors do it; unless xmit is active, which waits for
;		    next xmit interrupt.
;  1/16/84  Mike    Insert delay after IN and OUT instructions for IBM PC/AT
; ------------------------------------------------------------------------

	  subttl  Local File declarations and Include files
; ----------------------------------------------------------------------------
; This object file has a public character variable, which is where the byte
; input from the comm. line on an Async. interrupt is stored.
; ----------------------------------------------------------------------------
dgroup	  group   data
data	  segment word public 'data'
	  assume  DS:dgroup
	  extrn   port_addr:word, curr_port:word
	  extrn   ring_bot:byte, ring_fill:word, ring_empty:word
	  extrn   lcb:word		;Link Control Block
	  extrn   nest_com:word 	;nested comm interrupt count
	  extrn   nest_inp:word 	;nested input interrupt count
	  extrn   vector_tb:word	;vector interrupt table
	  public  mdm_mask		;used by comint's xmit_ready mdm_change
mdm_mask  dw	  0		;init to no bits, after logon A0 (DSR,CD)
fix_data  db	  8 dup (0)	;fix_xmit puts comm regs, if no xmit buff empty
data	  ends


; ----------------------------------------------------------------------------
; The current port and register addresses variables are declared within the
; 'pgroup' segment, so declare pgroup before include files and extrn declares.
; ----------------------------------------------------------------------------
pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup
	  extrn   is_enb:near
	  extrn   abort:near

	  include p_intequ.ad		;EQUs and MACROs for interrupt proc.
	  include p_lnkmdm.ae		;Link and Modem routine declarations

	  subttl  8259/8250 chip initialization
; -------------------------------------------------------------------------
; This routine initializes the 8259 and 8250 chips for the hardware interrupt
; portion of X.PC (the comm. port).  The 8250 is not enabled, just initialized.
; The sequence of inputs of various registers is suggested by INS-DOC-8250-x.
; -------------------------------------------------------------------------
	  public  init_825
init_825  proc	  near
;	  cli				;no interrupts during this process!!
	  BLD_DXP INT_ENBLE		;disable 8250 entirely, by turning off
	  xor	  al,al 		;all mask bits in the interrupt enable
	  out	  dx,al 		;register.
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 		;SCS, Sleepy Chip Syndrome...
	  INC_DXP INT_ENBLE, MDM_CNTRL	;want to talk to Modem Control Reg.
	  mov	  al,10h		;want to turn on the OUT1 (loopback)
	  out	  dx,al 		;line so will catch random output.
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 		;SCS, Sleepy Chip Syndrome...
	  INC_DXP MDM_CNTRL, LIN_STAT	;want to talk to line status register,
	  in	  al,dx 		;input to clear register contents.
	  INC_DXP LIN_STAT, MDM_STAT	;want to talk to the modem status reg,
	  in	  al,dx 		;input to clear register contents.
	  DEC_DXP MDM_STAT, IO_PORT	;back up to input port, and try input.
	  in	  al,dx 		;This done per INS-DOC-8250-x notes.
	  INC_DXP IO_PORT, MDM_CNTRL	;want to talk to Modem Control Reg.
	  mov	  al,0Bh		;DTR, RTS, OUT1, OUT2 are NOT bits
	  out	  dx,al 		;OUT1 is selected by 0 in its bit posit
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 		;SCS, Sleepy Chip Syndrome...
;
; Tricky stuff here to build a mask that can be used to set the int enable bit
; OFF in the Interrupt Mask Register of 8259.  For vector N, shift '1' N times
; to left to align value in dh with 8259 vector pos.  To enable, set bit to 0.
; This routine assumes that the vector used for 8250 is >= 0Ch and <= 0Fh.
;
	  mov	  di,curr_port		;get number of active port (1 or 2)
	  mov	  dx,1			;bit mask for AND operation.
	  mov	  cl,byte ptr vector_tb[di]	;get int vector number used.
	  shl	  dx,cl 		;shift the 1 bit over to the left,
	  not	  dh			;make a reversed mask of mostly 1's.
	  in	  al,IMR_port
	  and	  al,dh 		;want enable current + async. #1 or 2.
	  out	  IMR_port,al
	  mov	  nest_com,-1		;only appl task XPC call can reset this
	  mov	  nest_inp,-1		;only appl task XPC call can reset this
	  mov	  mdm_mask,0		;set CTS req to xmit (prevented)
	  lea	  ax,ring_bot
	  mov	  ring_fill,ax		;initialize the input ring pointers
	  mov	  ring_empty,ax
;	  sti				;interrupts OK now, thank you.
	  ret
init_825  endp

	  subttl  8259/8250 chip termination and cleanup
; ------------------------------------------------------------------------
; This routine disconnects the 8259 and 8250 chips from the system, by turning
; off all interrupts from the 8250, and by dropping the DTR, RTS, and OUT2
; signals.  The IRQ line to the 8259 for that vector is also disabled.
;
; Tricky stuff here to build a mask that can be used to set the int enable bit
; ON in the Interrupt Mask Register of 8259.  For vector N, shift '1' N times
; to left to align value in dh with 8259 vector pos.  To disable, set bit to 1.
; ----------------------------------------------------------------------------
	  public  term_825
term_825  proc	  near
;	  cli
	  mov	  di,curr_port		;get number of active port (1 or 2)
	  mov	  dx,1			;setup bit mask for AND operation.
	  mov	  cl,byte ptr vector_tb[di]	;get vector used for port.
	  shl	  dx,cl 		;shift the 1 bit over to the left.
	  in	  al,IMR_port		;get current contents of 8259,
	  or	  al,dh 		;OR in bit set on to disable async.
	  out	  IMR_port,al		;output to chip, use port 21h.
	  BLD_DXP MDM_CNTRL		;need to set all modem control signals
	  xor	  al,al 		;OFF, dropping DTR/RTS, OUT2 signal
	  out	  dx,al 		;drop causes disconnect from 8259.
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 		;SCS, Sleepy Chip Syndrome...
	  DEC_DXP MDM_CNTRL, INT_ENBLE	;back up, talk to interrupt enable reg,
	  out	  dx,al 		;inform chip no interrupts allowed.
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 		;SCS, Sleepy Chip Syndrome...
;	  sti
	  ret
term_825  endp

	  subttl  Xmitter Empty Interrupt Enable/Disable
; ----------------------------------------------------------------------------
; This routine enables the 'transmitter buffer empty' interrupts.  The first
; interrupt to start the chain is caused by the setting of the bit that says
; that the interrupts are enabled.  Once STI occurs, probably lose control to
; ASYNC_INT.  Wait for xmitter empty before enable, as noted in INS-DOC-8250-x.
; ----------------------------------------------------------------------------
	  public  xmit_enb
xmit_enb  proc	  near
;	  cli
	  xor	  cx,cx 	       ; loop maximum of 64k times???
	  BLD_DXP LIN_STAT	       ; need to first check for empty buffer.
emptychk: in	  al,dx
	  test	  al,20h	       ; see if bit #5 is on: buffer empty.
	  jnz	  short okempty        ; if NOT ZERO, 'empty' flag is on.
	  call	  fix_xmit	       ; fix the transmitter
	  loop	  emptychk
okempty:  DEC_DXP LIN_STAT, INT_ENBLE  ; Want to talk to Interrupt enable reg.
	  in	  al,dx 	       ; Input current register contents.
	  or	  al,XMIT_BIT	       ; Set bit #1 to 1 regardless.
	  out	  dx,al 	       ; Reset with new, watch out!!!
	  jmp	  short $+2	       ; delay for IBM PC/AT
	  out	  dx,al 	       ; SCS, Sleepy Chip Syndrome...
;	  sti
	  ret
xmit_enb  endp

; ----------------------------------------------------------------------------
; This routine disables the 'transmitter buffer empty' interrupts.
; ----------------------------------------------------------------------------
	  public  xmit_dsb
xmit_dsb  proc	  near
;	  cli
	  BLD_DXP INT_SRCE		;point to Interrupt Source reg.
	  in	  al,dx 		;find out about interrupt source
	  cmp	  al,6			;line error change?
	  jnz	  noop			;no
	  push	  dx
	  INC_DXP INT_SRCE, LIN_STAT	;yes
	  in	  al,dx 		;read it
	  pop	  dx
noop:	  cmp	  al,0			;modem status change?
	  jnz	  cant			;no
	  INC_DXP INT_SRCE, MDM_STAT	;yes
	  in	  al,dx 		;read it
; OBSERVED: When xmit failed and intr source reg was 6, input line_status reg
;	    MADE OUTPUT START UP!!!
cant:	  BLD_DXP INT_ENBLE	       ; Want to talk to Interrupt enable reg.
	  in	  al,dx 	       ; Get current settings.
	  and	  al,NOT XMIT_BIT      ; Leave all as is except bit #1.
	  out	  dx,al 	       ; output new settings.
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 	       ; SCS, Sleepy Chip Syndrome...
;	  sti
	  ret
xmit_dsb  endp

	  subttl  Receive Character Waiting Interrupt Enable/Disable
; ----------------------------------------------------------------------------
; This mask is a shorthand for all bits turned on when receive is enabled.
; This routine enables the 'received byte waiting' and 'line error' interrupts.
; ----------------------------------------------------------------------------
ENBLE_RECV  EQU  (RECV_BIT OR LINE_BIT OR MODM_BIT)
	  public  recv_enb
recv_enb  proc	  near
;	  cli
	  BLD_DXP INT_ENBLE	       ; Want to talk to Interrupt enable reg.
	  in	  al,dx
	  or	  al,ENBLE_RECV
	  out	  dx,al
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 	       ; SCS, Sleepy Chip Syndrome...
;	  sti
	  ret
recv_enb  endp

; ----------------------------------------------------------------------------
; This routine disables the 'received byte waiting' and 'line error' interrupts.
; ----------------------------------------------------------------------------
	  public  recv_dsb
recv_dsb  proc	  near
;	  cli
	  BLD_DXP INT_ENBLE	       ; Want to talk to Interrupt enable reg.
	  in	  al,dx
	  and	  al,NOT ENBLE_RECV
	  out	  dx,al
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 	       ; SCS, Sleepy Chip Syndrome...
;	  sti
	  ret
recv_dsb  endp

	  subttl  Xmit/Recieve Enabled? test routines
; ----------------------------------------------------------------------------
; This returns TRUE if the xmit interrupts are enabled, FALSE if not enabled.
; ----------------------------------------------------------------------------
	  public  xmit_on
xmit_on   proc	  near
;	  cli
	  BLD_DXP INT_ENBLE	       ; Want to talk to Interrupt enable reg.
	  xor	  ah,ah
	  in	  al,dx 	       ; Get current settings.
	  and	  al,XMIT_BIT	       ; See if the bit is set to 1.
	  jz	  short xmt_done       ; no matching bits, not enabled.
	  mov	  ax,1		       ; matching bit on, must be enabled.
xmt_done:
;	  sti
	  ret
xmit_on   endp

; ----------------------------------------------------------------------------
; This returns TRUE if the recv interrupts are enabled, FALSE if not enabled.
; ----------------------------------------------------------------------------
	  public  recv_on
recv_on   proc	  near
;	  cli
	  BLD_DXP INT_ENBLE	       ; Want to talk to Interrupt enable reg.
	  xor	  ah,ah
	  in	  al,dx 	       ; Get current settings.
	  and	  al,RECV_BIT	       ; see if the bit is set to 1.
	  jz	  short rcv_done       ; no matching bits, not enabled.
	  mov	  ax,1		       ; matching bit on, must be enabled.
rcv_done:
;	  sti
	  ret
recv_on   endp

	  subttl  Special Routines for Input/Output Processing
; ----------------------------------------------------------------------------
; This structure is used to map the stack area inside a called routine.
; ----------------------------------------------------------------------------
stkstruc  struc
oldbp	  dw	  ?
retn_ip   dw	  ?
param	  dw	  ?
stkstruc  ends
; ----------------------------------------------------------------------------
; This routine takes the byte received as a parameter, and outputs it.	It
; first makes sure the buffer is empty, because it might not be empty yet.
; ----------------------------------------------------------------------------
	  public  xmit_byt
xmit_byt  proc	  near
	  push	  bp
	  mov	  bp,sp
	  BLD_DXP IO_PORT		;back up to the I/O port for output.
	  mov	  ax,[bp].param 	;xmit buffer is now empty.
	  out	  dx,al 		;output the character.
	  jmp	  short $+2
	  out	  dx,al 		;output the character.
	  pop	  bp
	  ret
xmit_byt  endp

; ----------------------------------------------------------------------------
; The National Semi 8250 chip loses transmit interrupts, so send fails!!!
; Disable and re-enable the 8250 chip "between accesses" says the National Semi
; application note.  We run full dupex, so there is no such state; however, we
; learned that if it is done on every interrupt, then receive fails!!!
; If it has no effect on send (except to keep interrupts working), then we can
; do this on every receive interrupt.
; ----------------------------------------------------------------------------
	  public  re_enable
re_enable proc	  near
	  RET				;XMIT INTERRUPT IS FIXED
	  push	  dx			;*
	  BLD_DXP INT_SRCE		;point to Interrupt Source reg.
	  push	  dx
	  in	  al,dx 		;find out about interrupt: if al=1,
; OBSERVED: When xmit failed and intr source reg was 6,
;	    reading line_status reg MADE OUTPUT START UP!!!
	  cmp	  al,6			;line error change?
	  jnz	  nope			;no
	  INC_DXP INT_SRCE, LIN_STAT	;yes
	  in	  al,dx 		;read it
	  jmp	  short cont
nope:	  cmp	  al,0			;modem status change?
	  jnz	  cont			;no
	  INC_DXP INT_SRCE, MDM_STAT	;yes
	  in	  al,dx 		;read it
; OBSERVED: When xmit failed, the xmit enable bit was off, so...
;	    TURN IT BACK ON!!!
cont:	  pop	  dx			;point to Interrupt Source reg.
	  DEC_DXP INT_SRCE, INT_ENBLE
	  in	  al,dx 		;read the current enable reg
	  or	  al,XMIT_BIT		;set bit #1 on regardless.
	  out	  dx,al 		;Turn on all interrupt lines,
	  jmp	  short $+2		;delay for IBM PC/AT
	  out	  dx,al 		;SCS, Sleepy Chip Syndrome...
	  jmp	  short $+2		;delay for IBM PC/AT
	  pop	  dx			;*
	  ret				;return
re_enable endp

; ----------------------------------------------------------------------------
; This is called by xmit_enb and asycn_int (xmit_ready) when the xmit enable bit
; is off.  This routine will attempt to turn it on by allowing receive and line_
; error interrupts which are at a higher priority than xmit.  (dx)=LIN_STAT
; INTERRUPTS ARE DISABLED
; ----------------------------------------------------------------------------
	  public  fix_xmit
fix_xmit  proc	  near
	  push	 ax
	  call	 is_enb 		;are interrupt enabled?
	  jz	 allright		;no - proceed
	  call	 abort			;yes, abort
; THE FOLLOWING IS FOR ANALYSIS ONLY, MAY SCREWUP INTERRUPTS !!!!!!!!!!
	  push	 dx
	  DEC_DXP LIN_STAT IO_PORT
	  in	 al,dx			;read i/o data port
	  mov	 fix_data+0,al
	  INC_DXP IO_PORT INT_ENBLE
	  in	 al,dx			;read intr enable port
	  mov	 fix_data+1,al
	  INC_DXP INT_ENBLE INT_SRCE
	  in	 al,dx			;read intr source port
	  mov	 fix_data+2,al
	  INC_DXP INT_SRCE LIN_CNTRL
	  in	 al,dx			;read line control port
	  mov	 fix_data+3,al
	  INC_DXP LIN_CNTRL MDM_CNTRL
	  in	 al,dx			;read modem control port
	  mov	 fix_data+4,al
	  INC_DXP MDM_CNTRL LIN_STAT
	  in	 al,dx			;read line status port
	  mov	 fix_data+5,al
	  INC_DXP LIN_STAT MDM_STAT
	  in	 al,dx			;read modem status port
	  mov	 fix_data+6,al
	  pop	 dx
; THE ABOVE IS FOR ANALYSIS ONLY, MAY SCREWUP INTERRUPTS !!!!!!!!!!
allright: pop	 ax
	  sti				;allow an interrupt (receive or error)
	  cli
	  ret
fix_xmit  endp
prog	  ends
	  end
