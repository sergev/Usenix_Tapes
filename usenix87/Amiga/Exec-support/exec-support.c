;**** EXEC_SUPPORT ****
;
;Support routines to provide some of the same functions	as the exec_support
;library that C	programmers may	draw upon.  Slightly specialized, but
;easy to modify.  Other	stuff tossed in	to use more disk space :-)
;Hastily assembled by Bryce Nesbitt.  Public domain.
;bryce@cogsci.berkeley.EDU
;
;CREATEPORTE	;Create	a MsgPort
;DELETEPORTE
;CREATESTDIOE	;Create	a IOReq	of STD size
;DELETESTDIOE	 ;can easily be	modified for ANY size
;CHECKBREAK	;Check for <CTRL><C> or	the BREAK command
;CLEARBREAK	;Clear the status of the above flag
;SLEEPTILLBREAK	;
;PRINTLONGHEX	;Prints	LONG from D0. Needs a Ouput (ala CLI) but nothing
;		 ;else,	saves all registers.
;
;Each routine has a "ifeq 1" in front of it.  Change this to "ifeq 0" to
;assemble that routine.
;
jsrlib	MACRO
	XREF	\1
	JSR	\1(a6)
	ENDM
jmplib	MACRO
	XREF	\1
	JMP	\1(a6)
	ENDM
 NOLIST
 IFND EXEC_TYPES_I
 INCLUDE 'exec/types.i'     ;macros, lots of stuff
 ENDC
;IFND LIB_EXEC_LIB_I	    ;Non-standard extension that allows
;INCLUDE 'lib/exec_lib.i'   ;elimination of the usual link with
;ENDC			    ;amiga.lib.	 Much faster!
 IFND EXEC_MEMORY_I
 INCLUDE 'exec/memory.i'    ;MEMF_CLEAR MEMF_PUBLIC
 ENDC
 IFND EXEC_PORTS_I
 INCLUDE 'exec/ports.i'     ;lots of stuff
 ENDC
 IFND EXEC_IO_I
 INCLUDE 'exec/io.i'
 ENDC
 IFND LIBRARIES_DOS_I
 INCLUDE 'libraries/dos.i'  ;SIGBREAKF_CTRL_C SIGBREAKB_CTRL_C
 ENDC
 LIST
;
; port=CREATEPORTE(),exec
; d0		     a6
; z=error
;
;FUNCTION: Create a nameless message port, 0 priority.
;RESULT: The port pointer or Z=1 if an error occured.
;REGISTERS: A6 must contain exec!
;EXAMPLE:	bsr	CREATEPORTE
;		beq.s	noport	;Not enough memory (or signals)
;
		ifeq	1
		;xref	CREATEPORTE
CREATEPORTE	move.l	a2,-(a7)
		move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
		moveq	#MP_SIZE,d0
		jsrlib	_LVOAllocMem
		move.l	d0,a2
		tst.l	d0
		beq.s	cp_nomemory
		moveq	#-1,d0
		jsrlib	_LVOAllocSignal	    ;d0=return
		moveq	#-1,d1
		cmp.l	d0,d1	;-1 indicates bad signal
		bne.s	cp_sigok
		move.l	a2,a1
		moveq	#MP_SIZE,d0
		jsrlib	_LVOFreeMem
cp_nomemory	move.l	(a7)+,a2
		moveq	#0,d0
		rts

cp_sigok	move.b	d0,MP_SIGBIT(a2)
		move.b	#PA_SIGNAL,MP_FLAGS(a2)
		move.b	#NT_MSGPORT,LN_TYPE(a2)
		clr.b	LN_PRI(a2)
		suba.l	a1,a1		   ;a1=0/Find this task
		jsrlib	_LVOFindTask	   ;[d0=this task]
		move.l	d0,MP_SIGTASK(a2)
		lea	MP_MSGLIST(a2),a0  ;Point to list header
		NEWLIST	a0		;Init new list macro
		move.l	a2,d0
		move.l	(a7)+,a2	;cc's NOT affected
		rts
		endc

;
;DELETEPORTE(port),exec
;	     a1	   a6
;
;FUNCTION:  Deletes the	port by	first setting some
; fields to illegal values then	calling	FreeMem.
;RESULT: none
;REGISTERS: A6 must contain exec!
;
		ifeq	1
		;xref	DELETEPORTE
DELETEPORTE	move.l	a1,-(a7)
		moveq	#-1,d0
		move.b	d0,LN_TYPE(a1)
		move.l	d0,MP_MSGLIST+LH_HEAD(a1)
		moveq	#0,d0	;Clear upper 3/4 of d0
		move.b	MP_SIGBIT(a1),d0
		jsrlib	_LVOFreeSignal
		move.l	(a7)+,a1
		moveq	#MP_SIZE,d0
		jmplib	_LVOFreeMem
		endc

;
;ioStdReq=CREATESTDIOE(ioReplyPort),exec
;  a1		       d0	    a6
;
;Function: Allocate a IoRequest	block of standard size.
;
;Example:	move.l	MyPort,d0
;		bsr	CREATESTDIOE
;		beq.s	badnews
;
		ifeq	1
		;xref	CREATESTDIOE
CREATESTDIOE	move.l	d0,-(a7)
		moveq	#IOSTD_SIZE,d0		;!!SIZE	of IO!!
		move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
		jsrlib	_LVOAllocMem
		move.l	d0,a1
		tst.l	d0
		beq.s	nomem
		move.b	#NT_MESSAGE,LN_TYPE(a1)
		move.l	(a7)+,MN_REPLYPORT(a1)
nomem		rts
		endc

;
;DELETESTDIOE(ioStdReq),exec
;	      a1	a6
;
;FUNCTION:  Deletes an ioStdIO request by setting some
; fields to illegal values then	calling	FreeMem.
;RESULT: none
;
		ifeq	1
		;xref	DELETESTDIOE
DELETESTDIOE	moveq	#-1,d0	;Set fields to illegal value
		move.b	d0,LN_TYPE(a1)
		move.l	d0,IO_DEVICE(a1)
		move.l	d0,IO_UNIT(a1)
		moveq	#IOSTD_SIZE,d0	   ;!!SIZE of IO!!
		jmplib	_LVOFreeMem
		endc

;
;result=CHECKBREAK()
;z
;
;Example:   bsr	    CHECKBREAK
;	    beq.s   break_not_hit
;
		ifeq	1
		;xref	CHECKBREAK
CHECKBREAK	move.l	a6,-(a7)
		move.l	4,a6
		moveq	#0,d0
		moveq	#0,d1
		jsrlib	_LVOSetSignal
		move.l	(a7)+,a6
		btst	#SIGBREAKB_CTRL_C,d0
		rts
		endc
;
;CLEARBREAK()
;
		ifeq	1
CLEARBREAK	move.l	a6,-(a7)
		move.l	4,a6
		moveq	#0,d0
		move.l	#SIGBREAKF_CTRL_C,d1
		jsrlib	_LVOSetSignal
		move.l	(a7)+,a6
		rts
		endc
;
;SLEEPTILLBREAK()
;
		ifeq	1
		move.l	a6,-(a7)
		move.l	4,a6
		move.l	#SIGBREAKF_CTRL_C,d0
		jsrlib	_LVOWait
		move.l	(a7)+,a6
		rts
		endc
;
;PRINTLONGHEX(value)
;	      d0
;
		ifeq	1
PRINTLONGHEX	movem.l	d0-d3/a0-a1/a6,-(a7)
		subq.l	#5,a7	;Take 10 bytes from stack
		subq.l	#5,a7	;(it's smaller...)
		move.l	a7,a0
		moveq	#7,d2

1$		rol.l	#4,d0
		move.l	d0,d1
		andi.w	#$f,d1
		move.b	hextab2(pc,d1.w),(a0)+
		dbra	d2,1$

		move.b	#10,(a0)
		move.l	4,a6
		lea	DOSName(pc),a1
		jsrlib	OldOpenLibrary	;V1.0 compatible :-)
		tst.l	d0		;grrr...
		beq.s	er_lib
		move.l	d0,a6
		jsrlib	Output
		move.l	d0,d1
		beq.s	er_output
		move.l	a7,d2
		moveq	#9,d3
		jsrlib	Write		;skip error check
		move.l	a6,a1
		move.l	4,a6
		jsrlib	CloseLibrary
		addq.l	#5,a7
		addq.l	#5,a7
er_output
er_lib		movem.l	(a7)+,d0-d3/a0-a1/a6
		rts

hextab2		dc.b	'0123456789ABCDEF'
		endc
		end

