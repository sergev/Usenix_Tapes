	  title    intrup.asm -- Init and Term. Interrupt vectors and devices
	  page	   60,132
;*****************************************************************************
;     This program is the sole property and confidential information of      *
;     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
;     to any third party, without the written prior consent of Tymnet.	     *
;*****************************************************************************
; -------------------------------------------------------------------------
; Initialization and Termination Code for Interrupt vectors and devices
; filename = p_intrup.asm
;
;  PUBLIC subroutines:
;  -------------------
; load_drv  saves off the current contents of the interrupt vector used for the
;	    the Driver, and replaces it with the X.PC Driver routine address.
; unld_drv  replaces the X.PC Driver address in the Driver interrupt vector
;	    with the values stored by 'load_drv', disconnecting the Driver.
;
; load_tim  moves the current vector value of the 1C timer interrupt to the
;	    vector used by X.PC for a secondary timer, and places the X.PC timer
;	    routine address in the 1Ch vector. The vector values where the
;	    secondary timer is loaded are saved before the 1C values are moved.
; unld_tim  replaces the 1C timer interrupt with the value moved to the X.PC
;	    secondary timer vector, and replaces the secondary timer vector
;	    with the value saved by 'load_tim'.  Timer interrupts left enabled.
;
; load_com  saves off the current vector value for the port number parameter,
;	    loads the vector with the ASYNC interrupt handling routine, and
;	    initializes the device at the address corresponding to the port.
;	    Also creates the table used to access the ports of the 8250, but
;	    DOES NOT enable the device, call init_825 AFTER this to do so.
; unld_com  replaces the vector value for the port number parameter from the
;	    saved value, and de-initializes the device at the corresponding
;	    address.  DOES NOT disable the device, call term_825 BEFORE to do.
;
; init_825  enables the 8259 Programmable Interrupt Controller chip to handle
;	    interrupts from the 8250 Async. Controller, for the current port.
;	    Initializes the 8250 Async. Communication Adapter, with DTR/RTS ON.
; term_825  disables the 8259 PIC chip's ability to handle 8250 interrupts, for
;	    the current port.  The 8250 is disabled entirely, with the modem's
;	    DTR/RTS signals set to OFF.  Call BEFORE calling unld_comm.
; end_intr  Issues a non-specific End-Of-Interrupt (EOI) to the 8259 chip.
; disp_str  displays on the screen the string pointed to by parameter.
;
; ----------------------------------------------------------------------------
;   Date    Change#   by     Reason
; 05/22/84     01    curt    Initial Generation of Code.
; 05/29/84     02    curt    Changed to not enable any interrupts in init_825.
; 08/31/84     03    curt    Remove extra push/pop, for release 1.02.
; 11/05/84     04    curt    Changes due to technical info from National:
;			     all fixes noted with INS-DOC-8250-x are described
;			     in National Document B01361, 'problem-solution.'
;  8/19/85     05    mike    Move curr_port and port_addr from CS to DS.
; -------------------------------------------------------------------------
DISP_STRG EQU	  9		;DOS function
	  subttl  Data and program segment Data Variables
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The routines that take as a parameter a 'port number' are referring to
; a value equal to 1 or 2.  Port 1 is assumed to be on the 0Ch vector,
; using a physical port address of 3F8h.  Port 2 is assumed to be on the
; 0Dh vector, using a physical port address of 2F8h.
; The communication vector routines require a parameter indicating the port to
; be enabled, and the init/term_825 routine must get the corresponding value.
; The following structure maps the stack when the routine parameters are there.
; ----------------------------------------------------------------------------
prm_struc struc
old_bp	  dw	  ?
retn_ip   dw	  ?
param_1   dw	  ?
param_2   dw	  ?
prm_struc ends

; -------------------------------------------------------------------------
; These are the 'data group' memory allocations, including storage for
; saving interrupt vectors and arrays showing the communication port addresses
; and vector values.  Save address as 2 words: 1st = segment, 2nd = offset.
; The vector numbers for Driver and Timers can be found in 'p_intequ.ad'.
; The vector_tb and ptaddr_tb have dummy first value to allow index w/param.
; -------------------------------------------------------------------------
dgroup	  group   data
data	  segment byte public 'data'
	  assume  DS:dgroup
	  public  vector_tb
	  extrn   nest_tim:word 	;nest count reset for timer interrupt
	  public  port_addr, curr_port
curr_port dw	  0			;0 means no port, else com 1 or 2
port_addr dw	  0			;port address 3f8 (1) or 2f8 (2)
sv_drver  dw	  0,0			;2 words for Application Interrupt
sv_timer  dw	  0,0			;2 words for Timer interrupt
sv_async  dw	  0,0			;2 words for ASYNC comm. port routine.
vector_tb db	  00h,	0Ch,  0Bh	;Port DUMMY, Port 1, Port 2
ptaddr_tb dw	  000h, 3F8h, 2F8h	;DUMMY chip, Port 1 chip, Port 2 chip
data	  ends

; ------------------------------------------------------------------------
; These are the program group and segment declarations needed for the module.
; Also listed are any routines or variables that are PUBLIC or EXTERNAL.
; ------------------------------------------------------------------------
pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup, DS:dgroup
; ----------------------------------------------------------------------------
; These routines are the Interrupt handlers for X.PC.  Their addresses are
; stored into the DOS interrupt vector tables by load routines.
; ----------------------------------------------------------------------------
	  extrn   XPC_DRIVER:far, TIMER_TICK:far, ASYNC_INT:far, XPC_DS:word
	  subttl  Include files
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	  include p_intequ.ad		;Interrupt vector constants.
; ----------------------------------------------------------------------------
; This subroutine just sends an EOI (non-specific) to the 8259 chip.
; ----------------------------------------------------------------------------
	  public  end_intr
end_intr  proc	  near
	  mov	  al,EOI_8259		;set up value,
	  out	  EOI_port,al		;and output it.
	  ret
end_intr  endp
	  subttl  Set up/restore Driver vector table entry
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ------------------------------------------------------------------------
; This routine replaces the current vector table values with the addresses
; of the VTPAD DRIVER routines implemented for X.PC.
; ------------------------------------------------------------------------
	  public  load_drv
load_drv  proc	  near
;	  cli				;no interrupts, altering vector tables.

	  push	  es			;save ES, DOS destroys
	  mov	  ax,GET_VECT*256+DRIVRI_VEC	;report vector table contents
	  int	  DOS_CALL		;please.
	  mov	  sv_drver,es		;store segment returned.
	  mov	  sv_drver+2,bx 	;store offset returned.
	  pop	  es			;restore ES

	  push	  ds			;save DS, we destroy
	  push	  cs
	  pop	  ds			;set up code segment register value.
	  lea	  dx,pgroup:XPC_DRIVER	;point to new routine to execute.
	  mov	  ax,SET_VECT*256+DRIVRI_VEC	;store segment/offset in vector
	  int	  DOS_CALL		;please.
	  pop	  ds			;restore DS

;	  sti				;re-enable interrupts at this point.
	  mov	  nest_tim,-1		;only appl task XPC call can reset this
	  ret
load_drv  endp


; ----------------------------------------------------------------------------
; This routine unloads the vector table entry used for the Driver's address.
; ----------------------------------------------------------------------------
	  public  unld_drv
unld_drv  proc	  near
;	  cli				;please, dear, not while I'm busy!

	  push	  ds			;save DS, we destroy
	  mov	  dx,sv_drver+2 	;recall segment and offset of the
	  mov	  ds,sv_drver		;original routine (DESTROYS DS!!!)
	  mov	  ax,SET_VECT*256+DRIVRI_VEC	;put values into vector table
	  int	  DOS_CALL		;please.
	  pop	  ds			;restore DS

;	  sti				;done altering vector tables.
	  ret
unld_drv  endp
	  subttl  Initialization of Timer vector table entry
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ------------------------------------------------------------------------
; This routine replaces the current vector table values with the addresses
; of the TIMER routines implemented for X.PC.	If a timer is already using
; the 1Ch interrupt, that timer is moved to the TIMER2_VEC interrupt vector.
; The current contents of the vector for the chained-to timer are saved.
; ----------------------------------------------------------------------------
	  public  load_tim
load_tim  proc	  near
	  push	  es			;save ES, DOS destroys
;	  cli				;no interrupts, altering vector tables.

	  mov	  ax,GET_VECT*256+TIMER2_VEC	;report vector table contents
	  int	  DOS_CALL		;please.
	  mov	  sv_timer,es		;store segment returned.
	  mov	  sv_timer+2,bx 	;store offset returned.

	  mov	  ax,GET_VECT*256+TIMER1_VEC	;for original interrupt.
	  int	  DOS_CALL		;pass old vector to SET_VECT
	  push	  ds			;save DS, we destroy
	  push	  es			;
	  pop	  ds			;segment of old timer routine
	  mov	  dx,bx 		;will set up existing as TIMER2_VEC.
	  mov	  ax,SET_VECT*256+TIMER2_VEC	;original timer intr routine
	  int	  DOS_CALL		;please.

	  push	  cs
	  pop	  ds			;set up code segment register value.
	  lea	  dx,pgroup:TIMER_TICK	;point to new routine to execute.
	  mov	  ax,SET_VECT*256+TIMER1_VEC	;for timer interrupt routine.
	  int	  DOS_CALL		;please.
	  pop	  ds			;restore DS

;	  sti				;re-enable interrupts at this point.
	  pop	  es			;restore ES
	  ret
load_tim  endp
	  subttl  Termination of Timer vector table entry
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine unloads the vector table entry used for the Driver's address.
; ----------------------------------------------------------------------------
	  public  unld_tim
unld_tim  proc	  near
	  push	  es			;save ES, DOS destroys
;	  cli				;please, dear, not while I'm busy!

	  mov	  ax,GET_VECT*256+TIMER2_VEC	;report vector table contents
	  int	  DOS_CALL		;please.

	  push	  ds			;save DS, we destroy
	  push	  es			;move segment returned in es into ds.
	  pop	  ds			;recover CS:IP of chained-to, setup
	  mov	  dx,bx 		;original vector (1Ch) with address.
	  mov	  ax,SET_VECT*256+TIMER1_VEC	;store segment/offset in vector
	  int	  DOS_CALL		;please.
	  pop	  ds			;retrieve proper ds value.

	  push	  ds
	  mov	  dx,sv_timer+2 	;recall stored segment and offset
	  mov	  ds,sv_timer		;of the original routine (DESTROYS DS!)
	  mov	  ax,SET_VECT*256+TIMER2_VEC	;put values into vector table
	  int	  DOS_CALL		;please.
	  pop	  ds			;restore DS

;	  sti				;done altering vector tables.
	  pop	  es			;restore ES
	  ret
unld_tim  endp

	  subttl  Initialization of Communication vector table entry
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The load_com routine gets a parameter (value = 1 or 2).  This indicates the
; entry to use from the 'vector_tb' and the 'ptaddr_tb' to use to build the
; 'port_addr' tables, as used by all ASYNC interrupt routines.
; Save off current ASYNC vector contents, then replace with X.PC routine addr.
; ----------------------------------------------------------------------------
	  public  load_com
load_com  proc	  near
	  push	  bp			;*
	  mov	  bp,sp
	  push	  es			;**
;	  cli				;no interrupts, altering vector tables.

	  mov	  di,[bp].param_1	;get port number to set up.
	  mov	  al,byte ptr vector_tb[di]	; chip interrupt.
	  mov	  ah,GET_VECT		;report vector table contents for comm.
	  int	  DOS_CALL		;please.
	  mov	  sv_async,es		;store segment returned.
	  mov	  sv_async+2,bx 	;store offset returned.

	  mov	  al,byte ptr vector_tb[di]	; NEED DS FOR THIS ACTION!!
	  lea	  dx,pgroup:ASYNC_INT	;point to our ASYNC interrupt routine.
	  push	  ds			;*** do some tricky junk with DS here.
	  push	  cs			;****
	  pop	  ds			;**** set up code segment register value.
	  mov	  ah,SET_VECT		;store segment/offset into table?
	  int	  DOS_CALL		;please.
	  pop	  ds			;*** restore DS one last time.

;	  sti				;re-enable interrupts at this point.
; Initialize 'dgroup:port_addr' array.  Filling words 0-6 with port addresses.
	  mov	  curr_port,di		;store current port number (1 or 2)
	  shl	  di,1			;double index to get to word.
	  mov	  ax,word ptr ptaddr_tb[di]	;get address for this port.
filloop:  mov	  word ptr port_addr,ax ;store at address+index.
	  pop	  es			;**
	  pop	  bp			;*
	  ret
load_com  endp
	  subttl  Termination of Communication vector table entry
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine unloads the vector contents, and sets the port address array=0.
; ----------------------------------------------------------------------------
	  public  unld_com
unld_com  proc	  near
;	  cli				;please, not while I'm busy!
	  push	  bp
	  mov	  bp,sp
; Replace address pointing to X.PC routine, put back contents from load time.
	  mov	  di,[bp].param_1	;get port number to set up.
	  mov	  al,byte ptr vector_tb[di]	;for asynch. I/O routine.
	  push	  ds
	  mov	  dx,sv_async+2 	;recall offset and segment of
	  mov	  ds,sv_async		;orig. routine (DESTROYS DS!)
	  mov	  ah,SET_VECT		;put values into vector table
	  int	  DOS_CALL		;please.
	  pop	  ds			;**
;	  sti				;done altering vector tables.
; Fill the current port and address variables with zero to indicate not active.
	  mov	  curr_port,0		;zero current port number.
zeroloop: mov	  word ptr port_addr,0	;store at address+index.
	  pop	  bp
	  ret
unld_com  endp


; ----------------------------------------------------------------------------
; This routine accepts a pointer to a string, and asks DOS to display it.
; Called by drv_init in file p_initdr.c, when the XPC driver is loaded.
; ----------------------------------------------------------------------------
	  public  disp_str
disp_str  proc	  near
	  push	  bp
	  mov	  bp,sp

	  mov	  dx,[bp].param_1	    ; parameter is address of string.
	  mov	  ah,DISP_STRG		    ; display to standard output.
	  int	  DOS_CALL		    ; please, DOS, pretty please???

	  sti				    ; sometimes DOS is interrupt jerk.
	  pop	  bp
	  ret
disp_str  endp
prog	  ends
	  end
