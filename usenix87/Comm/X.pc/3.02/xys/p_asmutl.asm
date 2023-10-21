	  title   asmutl.asm  General Purpose Assembler Routines
	  page	  60,132
;*****************************************************************************
;     This program is the sole property and confidential information of      *
;     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
;     to any third party, without the written prior consent of Tymnet.	     *
;*****************************************************************************

; ------------------------------------------------------------------------
;		  General Purpose Assembler Routines
;
; filename = p_asmutl.asm
;
; The routines in this file are general purpose routines: returning registers,
; displaying strings, copying data from an area in the local segment to an
; area in a distant segment of memory, or from an area in a distant segment
; of memory into the local segment.  The 'local' segment is assumed to be
; pointed to by the DS in which all X.PC data is allocated.
;
; get_ds    Returns to the caller the current value of the DS register.
; get_cs    Returns to the caller the current value of the CS register.
;
; mvbyt_to  Accepts the Data Segment/Offset of bytes in a distant data area,
;	    an offset within the local data area, and the number of bytes
;	    that are to be moved. The routine moves the data from the local
;	    data segment's offset "TO" the distant data segment's offset.
;	      'C' sequence:  mvbyt_to (dist_seg, dist_off, local_off, mov_cnt);
; mvbyt_fr  Accepts the Data Segment/Offset of a distant data area, an offset
;	    within the local data area, and the number of bytes that are to
;	    be moved.  The routine moves the data "FROM" the distant data
;	    segment's offset into the local data segment's offset.
;	      'C' sequence:  mvbyt_fr (dist_seg, dist_off, local_off, mov_cnt);
;
; mvwrd_to  Accepts the Data Segment/Offset of a distant data segment word
;	    and a value to store.  The routine moves the value "TO" the
;	    word in the distant segment's word.
;	      'C' sequence:   mvwrd_to (dist_seg, dist_off, int_value);
; mvwrd_fr  Accepts the Data Segment/Offset of a distant data area word, and
;	    the offset within the local segment of a word.  The routine moves
;	    the distant word's contents into the local segment's word.
;	      'C' sequence:  mvwrd_fr (dist_seg, dist_off, local_off);
; gtwrd_fr  Accepts the Data Segment/Offset of a distant data area word, and
;	    returns the word found at that location in AX.
;	      'C' sequence:  loc_word = gtwrd_fr (dist_seg, dist_off);
;
; zero_mem  Accepts an address, and a count.  Sets each byte for COUNT bytes
;	    to the value 0. 'C' sequence:  zero_mem (local_off, count);
;
; int_on    enables interrupts (just a STI instruction).
; int_off   disables interrupts (just a CLI instruction).
; percent   accepts an integer and a value, returns value % of integer.
; ----------------------------------------------------------------------------
;   Date    Change#   by     Reason
; 06/14/84    00     curt    Initial Conversion to PAD code version.
; 07/15/84    01     curt    Made several files into a ASM utility file.
; 08/31/84    01     curt    Clean up for release 1.02 of Driver.
; ------------------------------------------------------------------------
	  subttl  Local File Data Declarations
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; These structures are used to reach the values pushed on the stack by the
; calling routine.  See first page for parameter sequence. 'bytstruc' is for
; byte move routines, 'wrdstruc' is for word move routines.  'getstruc' is for
; the 'gtwrd_fr' routine, which returns the distant word pointed to in AX.
; The structures do not create storage, they only allow map into stack area.
; ----------------------------------------------------------------------------

bytstruc  struc
bold_bp   dw	  ?		  ; for caller's BP value
bretn_ip  dw	  ?		  ; IP for return from call.
bdst_seg  dw	  ?		  ; Distant segment.
bdst_off  dw	  ?		  ; Distant offset.
bloc_off  dw	  ?		  ; Offset of first byte in local area to use.
bmov_cnt  dw	  ?		  ; Number of bytes to transfer.
bytstruc  ends

wrdstruc  struc
wold_bp   dw	  ?		  ; for caller's BP value
wretn_ip  dw	  ?		  ; IP for return from call.
wdst_seg  dw	  ?		  ; Distant segment.
wdst_off  dw	  ?		  ; Distant offset.
wloc_off  dw	  ?		  ; Offset of word (fr), or value to move (to).
wrdstruc  ends

getstruc  struc
gold_bp   dw	  ?		  ; for caller's BP value
gretn_ip  dw	  ?		  ; IP for return from call.
gdst_seg  dw	  ?		  ; Distant segment.
gdst_off  dw	  ?		  ; Distant offset.
getstruc  ends

; ----------------------------------------------------------------------------
; Code segment begins here.
; ----------------------------------------------------------------------------
pgroup	  group   prog
prog	  segment byte public 'prog'
	  assume  CS:pgroup

; ----------------------------------------------------------------------------
;    Routine to return current Data Segment register value
; ----------------------------------------------------------------------------

	  public  get_ds
get_ds	  proc	  near
	  mov	  ax,ds
	  ret
get_ds	  endp

	  public  get_cs
get_cs	  proc	  near
	  mov	  ax,cs
	  ret
get_cs	  endp
	  subttl  Local-to-Distant Multi-byte Transfer Routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------------
; This routine will move bytes from a local data area (in the X.PC segment) to
; an area in a distant segment, up to the number of bytes specified.
;
; Want the 'source' to be DS:loc_off, and want the 'destination' to be (use ES)
; dst_seg:dst_off[dst_fst].  Set up registers, do multi-byte move operation.
; ----------------------------------------------------------------------------
	  public  mvbyt_to
mvbyt_to  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es

	  cld			       ; Want to go in forward direction.
	  mov	  ax,[bp].bdst_seg     ; get the segment of the distant area.
	  mov	  es,ax
	  mov	  di,[bp].bdst_off     ; get the offset of the distant area.
	  mov	  si,[bp].bloc_off     ; get offset of local data area.
	  mov	  cx,[bp].bmov_cnt     ; get number of bytes to move.

rep	  movsb 		       ; move bytes from local to distant.

	  pop	  es
	  pop	  bp
	  ret
mvbyt_to  endp

	  subttl  Distant-to-Local Multi-byte Transfer Routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine will move bytes from an area in a distant segment into a local
; data area (in the X.PC segment), up to the number of bytes specified.
;
; Want as 'source' (use ES) dst_seg:dst_off[dst_fst], and want as 'destination'
; DS:loc_off.  Set up registers, then do multi-byte move operation. Because
; the MOVSB instruction uses DS:SI and ES:DI, and want to do DS:DI and ES:SI,
; have to switch the DS and ES registers to fake out the instruction.
; ----------------------------------------------------------------------------
	  public  mvbyt_fr
mvbyt_fr  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es

	  cld			       ; Want to go in forward direction.
	  push	  ds
	  mov	  ax,[bp].bdst_seg     ; get the segment of the distant area.
	  mov	  ds,ax
	  pop	  es		       ; Store old DS in ES for move, save
	  push	  es		       ; value again to restore before exit.
	  mov	  si,[bp].bdst_off     ; get the offset of the distant area.
	  mov	  di,[bp].bloc_off     ; get offset of local data area.
	  mov	  cx,[bp].bmov_cnt     ; get number of bytes to move.

rep	  movsb 		       ; move bytes from distant to local.
	  pop	  ds		       ; restore original DS value.

	  pop	  es
	  pop	  bp
	  ret
mvbyt_fr  endp

	  subttl  Value to Distant-word Move routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine moves an word value into a word in a distant segment/offset.
;
; Set up the extra segment register and index register with address of distant
; word, then move word value from caller into that distant land.
; ----------------------------------------------------------------------------
	  public  mvwrd_to
mvwrd_to  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es

	  mov	  ax,[bp].wdst_seg     ; get the segment of the distant word.
	  mov	  es,ax
	  mov	  di,[bp].wdst_off     ; get the offset of the distant word.
	  mov	  ax,[bp].wloc_off     ; get value to move into distant word.
	  mov	  es:[di],ax	       ; move from local to distant.

	  pop	  es
	  pop	  bp
	  ret
mvwrd_to  endp


	  subttl  Distant-word to local-word move routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine moves a word from an offset in a distant segment into a word
; in the local data segment.
;
; Load the distant location in es:[di], and the local location in ds:[si].
; ----------------------------------------------------------------------------
	  public  mvwrd_fr
mvwrd_fr  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es

	  mov	  ax,[bp].wdst_seg     ; get the segment of the distant area.
	  mov	  es,ax
	  mov	  di,[bp].wdst_off     ; get the offset of word in dist. seg.
	  mov	  si,[bp].wloc_off     ; get the offset of word in local seg.

	  mov	  ax,es:[di]	       ; move the word in from distant seg.,
	  mov	  ds:[si],ax	       ; and store at the local offset.

	  pop	  es
	  pop	  bp
	  ret
mvwrd_fr  endp

	  subttl  Distant-word to AX (return value) routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine moves a word from an offset in a distant segment into AX.
; ----------------------------------------------------------------------------
	  public  gtwrd_fr
gtwrd_fr  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es
;
; Load the distant location in es:[di], and move word value found into AX.
;
	  mov	  ax,[bp].gdst_seg     ; get the segment of the distant area.
	  mov	  es,ax
	  mov	  di,[bp].gdst_off     ; get the offset of word in dist. seg.
	  mov	  ax,es:[di]	       ; move the word in from distant seg.,

	  pop	  es
	  pop	  bp
	  ret
gtwrd_fr  endp
	  subttl  Zero Memory routine
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; The structure allows the zero memory routine to map into the parameter list.
; ----------------------------------------------------------------------------
zerostruc struc
zold_bp   dw	  ?		  ; for caller's BP value
zretn_ip  dw	  ?		  ; IP for return from call.
byteaddr  dw	  ?		  ; Address in DS that we should start at.
bytecount dw	  ?		  ; The number of bytes from address to zero.
zerostruc ends

; ----------------------------------------------------------------------------
; This routine zeros bytes of memory.  It starts with the byte pointed to by
; the 'byteaddr' parameter, and does 'bytecount' bytes from there.
; ----------------------------------------------------------------------------;
	  public  zero_mem
zero_mem  proc	  near
	  push	  bp
	  mov	  bp,sp
	  push	  es
	  mov	  ax,ds
	  mov	  es,ax 		    ; DI defaults to using ES register.

	  mov	  di,[bp].byteaddr	    ; get address into ES:DI.
	  mov	  cx,[bp].bytecount	    ; set up loop count for REP.
	  xor	  ax,ax 		    ; fill each byte with value 0.

;	  cli				    ; INTERRUPTS OFF BY CALLER
	  cld				    ; going in a forward direction,
rep	  stosb 			    ; repeat store byte operation.
;	  sti				    ; DO NOT TURN ON!

	  pop	  es
	  pop	  bp
	  ret
zero_mem  endp
	  subttl  Int Enable/Disable, Message Display
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This routine just enables interrupt-recoginition for the CPU.
; ----------------------------------------------------------------------------
	  public  int_on
int_on	  proc	  near
	  sti			  ; enable interrupts.
	  ret
int_on	  endp


; ----------------------------------------------------------------------------
; This routine just disables interrupt-recoginition for the CPU.
; ----------------------------------------------------------------------------
	  public  int_off
int_off   proc	  near
	  cli			  ; disable interrupts.
	  ret
int_off   endp



	  subttl  Percent calculation
;	  page	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
; ----------------------------------------------------------------------------
; This subroutine does 32-bit arithmetic: returning the value that equals the
; param_1 value times param_2 percent.	NOTE that deals with SIGNED numbers.
; ----------------------------------------------------------------------------
prm_struc struc
old_bp	  dw	  ?
retn_ip   dw	  ?
param_1   dw	  ?
param_2   dw	  ?
prm_struc ends

	  public  percent
percent   proc	  near
	  push	  bp
	  mov	  bp,sp

	  mov	  ax,[bp].param_1	    ; total value.
	  imul	  [bp].param_2		    ; times percentage (integer), then
	  mov	  bx,100		    ; divide by 100 to get percent,
	  idiv	  bx			    ; leaving answer in AX.

	  pop	  bp
	  ret
percent   endp

; ----------------------------------------------------------------------------
; Test the CPU's status byte for Interrupt Enabled bit
; ----------------------------------------------------------------------------
	  public  is_enb
is_enb	  proc	  near
	  pushf 			;*
	  pop	  ax			;*
	  and	  ax,0200h		;is interrupt flag on?
	  ret				;return
is_enb	  endp
prog	  ends
	  end
