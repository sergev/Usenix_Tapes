;
; Speeddir By Bryce Nesbitt. 
; Disclaimer: Guaranteed defective.  If it works, lucky you!  If you improve
;  on it please send a copy back to the author at:
;  1712 Marin Ave.  Berkeley, Ca 94707-2206.  bryce@cogsci.Berkeley.EDU
;
 NOLIST
 INCLUDE 'exec/types.i'
 INCLUDE 'exec/memory.i'
 INCLUDE 'exec/ports.i'     ;lots of stuff
 INCLUDE 'exec/io.i'
 INCLUDE 'libraries/dos.i'
 INCLUDE 'libraries/dosextens.i'
 INCLUDE 'devices/trackdisk.i'
;INCLUDE 'lib/exec_lib.i'   ;You don't have these.  So *you*
;INCLUDE 'lib/dos_lib.i'    ;need bother with the linker. :-)
blink macro
 move.l d7,-(a7)
 move.b $bfe001,d7
 eori.b #2,d7
 move.b d7,$bfe001
 move.l (a7)+,d7
 endm
jsrlib MACRO
 xref _LVO\1
 jsr _LVO\1(a6)
 ENDM
jmplib MACRO	
 xref _LVO\1
 jmp _LVO\1(a6)
 ENDM
 LIST
*******************************
DEBUG	equ 1
	NOLIST
	INCLUDE 'libraries/filehandler.i'
	LIST
**************************
DOS_TRUE	equ 0
DOS_FALSE	equ -1
myprocport	equr a5
packet		equr a4

;--Get ready...--
		move.l	4,a6
		lea	DOSName(pc),a1
		jsrlib	OldOpenLibrary
		move.l	d0,a6
		move.l	d0,a5
		lea	Volume,a0
		move.l	a0,d1
		jsrlib	DeviceProc
		tst.l	d0
		beq	noway
		move.l	d0,a4
		lea	MyWait,a0
		move.l	a0,pr_PktWait-TC_SIZE(a4)
		move.l	4,a6
lplplp		;move.l  #SIGBREAKF_CTRL_C,d0
		moveq	#0,d0
		jsrlib	Wait
		;jsrlib  Forbid
		;clr.l	 pr_PktWait-TC_SIZE(a4)
		;move.l  a5,a6
		;move.l  #150,d1
		;jsrlib  Delay
		;move.l  4,a6
		;jsrlib  Permit
noway		moveq	#21,d0
		rts
;
;DOSpacket=GETAPACKETE()
;a0
;Waits for then gets a packet
;
MyWait		movem.l a2/a6,-(a7)
MyWaitLp	move.l	4,a6
		suba.l	a1,a1
		jsrlib	FindTask
		add.l	#TC_SIZE,d0
		move.l	d0,a0
		move.l	d0,a2
		jsrlib	WaitPort ;!!Signal 8??
		move.l	a2,a0
		jsrlib	GetMsg
		move.l	d0,a0

		move.l	LN_NAME(a0),a1
		move.l	a2,a0
		cmp.w	#$18,dp_Type+2(a1)
		beq.s	ExTheNext
		movem.l (a7)+,a2/a6
		rts
*******************************
;
;success=ExNext(lock,FileInfoBlock)
;d0		d1   d2
;
fib_KOB 	equ  fib_FileName+96	;* Kludge-O-Buffer
fib_HashChain	equ  fib_FileName+100	;* Where to chain to
fib_ChainRoot	equ  fib_FileName+104	;* Where chain started from
HASHSIZE	equ  72
KOB_SIZE	equ  HASHSIZE*4
ST.USERDIR	equ  2
ST.FILE 	equ  -3
ST.ROOT 	equ  1
T.SHORT 	equ  2
T.LIST		equ  16
T.DATA		equ  8
exfib		equr a4
blockbuffer	equr a3

;--We can do the job--
ExTheNext	movem.l (a7)+,a2/a6
		movem.l d2-d3/a0-a4/a6,-(a7)
		move.l	4,a6
		move.l	dp_Arg2(a1),d0
		asl.l	#2,d0
		move.l	d0,exfib
		move.l	#512,d0
		move.l	#MEMF_CLEAR+MEMF_CHIP,d1
		jsrlib	AllocMem
		tst.l	d0	    ;grrr...
		beq	e_memory
		move.l	d0,blockbuffer
;--Check if this is the first time 'round--
		btst	#7,fib_Protection(exfib)  ;!!Byte test!!
		bne.s	exthenext
;--This is a new request. Allocate some kludg-o ram--
		move.l	#KOB_SIZE,d0
		move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
		jsrlib	AllocMem
		move.l	d0,fib_KOB(exfib)
		beq	e_memory
		clr.l	fib_HashChain(exfib)
		clr.l	fib_ChainRoot(exfib)
		move.l	(exfib),d0	;fib_DiskKey
		move.l	blockbuffer,a0
		bsr	READBLOCK
		cmp.l	#T.SHORT,(blockbuffer)
		bne	e_errorreading
		moveq	#HASHSIZE-1,d0
		move.l	blockbuffer,a0
		adda.l	#24,a0
		move.l	fib_KOB(exfib),a1
KOBfill 	move.l	(a0)+,(a1)+
		dbra	d0,KOBfill
		clr.l	(exfib)
;--Check if we should chain--
exthenext	tst.l	fib_KOB(exfib)	;This time they simply
		beq	e_endagain	;went too far...
		move.l	fib_HashChain(exfib),d0
		bne.s	getit		;Follow the chain
;--Derive next block from hash table--
ScanList	moveq	#-1,d0
		moveq	#-1,d1
		move.l	fib_ChainRoot(exfib),d2 ;fib_DiskKey
		moveq	#71,d3
		move.l	fib_KOB(exfib),a0

scanlp		;move.l  d0,-(a7)
		;move.l  d3,d0
		;bsr	 PRINTLONGHEX
		;move.l  (a7)+,d0
		cmp.l	(a0)+,d2
		dbcs	d3,scanlp	;Until end or d0 is LOWER
		bcc.s	endscan
		move.l	-4(a0),a1
		cmp.l	a1,d0		;compare with last try
		bls.s	far		;new one is farther
		moveq	#0,d1
		move.l	a1,d0
	       ;bsr	PRINTLONGHEX
far		dbra	d3,scanlp	;DBCS does not decrement count

endscan 	tst.l	d1
		bmi	e_nomore
;--Got it, so get it--
getit		;[block in d0]
		;bsr	 PRINTLONGHEX
		move.l	blockbuffer,a0
		bsr	READBLOCK
		bmi	e_memory
		bne	e_errorreading
;--Valid User Directory in (blockbuffer). Fill in exfib--
		move.l	fib_HashChain(exfib),d0
		move.l	512-16(blockbuffer),fib_HashChain(exfib)
		tst.l	d0
		bne.s	thisisachain
		move.l	4(blockbuffer),fib_ChainRoot(exfib)
thisisachain	move.l	4(blockbuffer),(exfib)	;fib_DiskKey
		;compare with expected
		;compare type
		moveq	#ST.USERDIR,d1
		move.l	512-4(blockbuffer),d0	;Secondary type
		cmp.l	d0,d1
		beq.s	STValid
		moveq	#ST.FILE,d1
		cmp.l	d0,d1
		bne	e_errorreading
STValid 	move.l	d1,fib_DirEntryType(exfib)
;--Filename-- -80
		lea	fib_FileName(exfib),a1
		lea	512-80(blockbuffer),a0
		moveq	#0,d0		;Clear upper half
		move.b	(a0)+,d0
		move.b	d0,(a1)+
	       ; beq.s	 noname
	       ; subq.w  #1,d0		 ;Compensate for dbra
loop1		move.b	(a0)+,(a1)+
		dbra	d0,loop1	;68010 Loop mode
noname	       ; clr.b	 (a1)		 ;Null terminate string
;--Protection bits-- -192
		move.l	512-192(blockbuffer),d0
		bset	#31,d0		;Set we've been here flag
		move.l	d0,fib_Protection(exfib)
;--Size in bytes-- -188
		move.l	512-188(blockbuffer),d0
		move.l	d0,fib_Size(exfib)
		beq.s	zeroblocks
;--Calculate # blocks in file--
;--Do *NOT* read in list blocks from disk!--
;$1E7FE18/$1E8=$FFFF
		divu	#488,d0 ;!!Ok for files <= 31981080 bytes long
		trapv
		move.w	d0,d1
		ext.l	d0	;extend result
		swap	d1
		tst.w	d1		;Test remainder
		beq.s	zeroblocks	;Even multiply
		addq.l	#1,d0		;Tweak block count
		;nop
zeroblocks	move.l	d0,fib_NumBlocks(exfib)
;--DateStamp-- -92
		lea	512-92(blockbuffer),a0
		lea	fib_DateStamp(exfib),a1
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+
;--Comment-- 184
		lea	fib_Comment(exfib),a1
		lea	512-184(blockbuffer),a0
		moveq	#0,d0		;Clear upper part
		move.b	(a0)+,d0
		move.b	d0,(a1)+
	       ; beq.s	 nocomment
	       ; subq.w  #1,d0		 ;Compensate for dbra
loop2		move.b	(a0)+,(a1)+
		dbra	d0,loop2
nocomment      ; clr.b	 (a1)
;--Exit ok--
		suba.l	a2,a2
		bra.s	ExitA2
e_errorreading	move.w	#ERROR_SEEK_ERROR,a2
		bra.s	ExitA2
e_memory	move.w	#ERROR_NO_FREE_STORE,a2
		bra.s	ExitA2
e_nomore	move.l	fib_KOB(exfib),a1
		move.l	#KOB_SIZE,d0
		jsrlib	FreeMem
e_endagain	move.w	#ERROR_NO_MORE_ENTRIES,a2
ExitA2		move.l	#512,d0
		move.l	blockbuffer,a1
		jsrlib	FreeMem
;--Place error code in d0 where IOErr can find it--
		suba.l	a1,a1
		jsrlib	FindTask
		move.l	d0,a0
		move.l	a2,pr_Result2(a0)
		move.l	a2,d1
		movem.l (a7)+,d2-d3/a0-a4/a6
		bne.s	anerror
		moveq	#-1,d0
		bra.s	noerror
anerror 	moveq	#0,d0
;== Return the packet ==
noerror 	movem.l a2/a6,-(a7)
		blink
		move.l	4,a6
		move.l	d0,dp_Res1(a1)
		move.l	d1,dp_Res2(a1)
		move.l	dp_Port(a1),d0
		move.l	a0,dp_Port(a1)
		move.l	dp_Link(a1),a1
		move.l	d0,a0
		jsrlib	PutMsg
		bra	MyWaitLp
;--------------------------------
;
;PRINTLONGHEX(value)
;	      d0
;
		ifeq	1
PRINTLONGHEX	movem.l d0-d3/a0-a1/a6,-(a7)
		subq.l	#5,a7	;Take 10 bytes from stack
		subq.l	#5,a7	;(it's smaller...)
		move.l	a7,a0
		moveq	#7,d2

hexlp		rol.l	#4,d0
		move.l	d0,d1
		andi.w	#$f,d1
		move.b	hextab2(pc,d1.w),(a0)+
		dbra	d2,hexlp

		move.b	#10,(a0)
		move.b	#32,(a0)
		move.l	4,a6
		lea	DOSName,a1
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
er_lib		movem.l (a7)+,d0-d3/a0-a1/a6
		rts
hextab2 	dc.b	'0123456789ABCDEF'
		endc
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
		;xref	CREATEPORTE
CREATEPORTE	move.l	a2,-(a7)
		move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
		moveq	#MP_SIZE,d0
		jsrlib	AllocMem
		move.l	d0,a2
		tst.l	d0
		beq.s	cp_nomemory
		moveq	#-1,d0
		jsrlib	AllocSignal	;d0=return
		moveq	#-1,d1
		cmp.l	d0,d1	;-1 indicates bad signal
		bne.s	cp_sigok
		move.l	a2,a1
		moveq	#MP_SIZE,d0
		jsrlib	FreeMem
cp_nomemory	move.l	(a7)+,a2
		moveq	#0,d0
		rts

cp_sigok	move.b	d0,MP_SIGBIT(a2)
		move.b	#PA_SIGNAL,MP_FLAGS(a2)
		move.b	#NT_MSGPORT,LN_TYPE(a2)
		clr.b	LN_PRI(a2)
		suba.l	a1,a1		   ;a1=0/Find this task
		jsrlib	FindTask       ;[d0=this task]
		move.l	d0,MP_SIGTASK(a2)
		lea	MP_MSGLIST(a2),a0  ;Point to list header
		NEWLIST a0		;Init new list macro
		move.l	a2,d0
		move.l	(a7)+,a2	;cc's NOT affected
		rts
;
;DELETEPORTE(port),exec
;	     a1    a6
;
;FUNCTION:  Deletes the port by first setting some
; fields to illegal values then calling FreeMem.
;RESULT: none
;REGISTERS: A6 must contain exec!
;
		;xref	DELETEPORTE
DELETEPORTE	move.l	a1,-(a7)
		moveq	#-1,d0
		move.b	d0,LN_TYPE(a1)
		move.l	d0,MP_MSGLIST+LH_HEAD(a1)
		moveq	#0,d0	;Clear upper 3/4 of d0
		move.b	MP_SIGBIT(a1),d0
		jsrlib	FreeSignal
		move.l	(a7)+,a1
		moveq	#MP_SIZE,d0
		jmplib	FreeMem
;
;READBLOCK(buffer,block)
;	   a0	  d0
;
READBLOCK	movem.l d2/a3/a6,-(a7)
		move.l	4,a6
		move.l	d0,d2
		move.l	a0,a3
		suba.l	#IOTD_SIZE,a7
		moveq	#IOTD_SIZE-1,d0 ;clear the new IORequest (slowly)
		move.l	a7,a0
clearit 	clr.b	(a0)+
		dbra	d0,clearit
		bsr	CREATEPORTE
		beq.s	e_TDport
		move.l	d0,MN_REPLYPORT(a7)
		move.b	#NT_MESSAGE,LN_TYPE(a7)
		lea	TrackName,a0
		move.l	a7,a1
		moveq	#1,d0
		moveq	#0,d1
		jsrlib	OpenDevice  ;[a0-name|d0=unit|a1-IO|d1-flags]
		tst.l	d0
		bne.s	e_TDopen
		move.w	#CMD_READ,IO_COMMAND(a7) ;ETD!!
		move.l	#512,IO_LENGTH(a7)
		moveq	#9,d0
		asl.l	d0,d2	;shift d2 by d0
		move.l	d2,IO_OFFSET(a7)
		move.l	a3,IO_DATA(a7)
		move.l	a7,a1
		jsrlib	DoIO	;[a1-IO]
		move.l	MN_REPLYPORT(a7),a1
		bsr	DELETEPORTE
		move.b	IO_ERROR(a7),d0    ;cc's set
		adda.l	#IOTD_SIZE,a7	   ;cc's not
		movem.l (a7)+,d2/a3/a6	   ;cc's not
		rts
e_TDopen
e_TDport	moveq	#-1,d0
		rts

DOSName 	dc.b 'dos.library',0
Volume		dc.b 'df1:',0
TrackName	dc.b 'trackdisk.device',0
		dc.b 0,0
		END

