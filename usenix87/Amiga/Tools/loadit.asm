;
; LoadIt (C)1987 Bryce Nesbitt.  Revokable, free, non-exclusive licence
; hereby granted to any sentient being to use or abuse this code in any way
; whatsoever provided that this and any other copyright notices remain fully
; attached, and with the exception that, without prior written permission,
; it not be utilized by any entity that has been commonly referred to as
; Robert W. Skyles, Skyles Electric Works, Jim Drew, Regie Warren or any
; organization founded by, controlled, employing or profiting any such
; entity, it's offspring or spouses.  FISH use ok.
;
; Author correspondence, bug or stupidity reports may be directed to:
; Bryce Nesbitt 	;bryce@cogsci.Berkeley.EDU
; 1712 Marin Ave.
; Berkeley, Ca 96034-9412
;
; USAGE:     LoadIt <file>
;
; FUNCTION:  Loads a file into memory, and reports the address.  The
;	     file remains at that address until a re-boot.
;
; EXAMPLES:  LoadIt c:install
;	     LoadIt Read Me    ;No parsing is done. Quotes not needed
;
; BUGS:      If there is an error during the read the allocated memory will
;	     not be returned.
;
; Proper size when assembled with the Metacompost assembler
; and BLINK'ed is 424 bytes
;
; RETURN CODE CONVENTION:
;
; FAIL	(20) - serious error, no DOS, memory etc.
; ERROR (10) - user caused error. (file not found, syntax)
; WARN	(5)  - technicality such as file not found on DELETE.
;
**************************
	NOLIST
	INCLUDE 'libraries/dosextens.i'
	;INCLUDE 'lib/exec_lib.i'        ;eliminates link with amiga.lib
	;INCLUDE 'lib/dos_lib.i'         ;non-standard, but very speedy!
	LIST
jsrlib	macro
	xref	_LVO\1
	jsr	_LVO\1(a6)
	endm
blink	macro
	bchg.b #1, $bfe001
	endm
**************************
	CODE
;
reserved1	equr a7
reserved2	equr a6
lock		equr a5
handle		equr a4
block		equr a3
reserved3	equr a2

returncode	equr d7
resultcode	equr d6
size		equr d5
inline		equr d4
reserved4	equr d3
reserved5	equr d2

;-- "parse" the string --
		clr.b	-1(a0,d0)	;null terminate string
		move.l	a0,inline	;the quick and dirty way.
;-- Set defaults --
		suba.l	lock,lock
		suba.l	handle,handle
		moveq	#10,returncode	;default ERROR code
;-- Open DOS --
		move.l	4,a6
		lea	DOSName(pc),a1
		jsrlib	OldOpenLibrary	;1.0 compatible
		move.l	d0,a6		;Look Ma, no error check!
;-- Get a lock --
		move.l	inline,d1
		moveq	#ACCESS_READ,d2
		jsrlib	Lock
		move.l	d0,d1
		move.l	d0,lock
		beq.s	e_lock
;-- Examine locked object [d1-lock] --
		suba.l	#fib_SIZEOF,a7
		move.l	a7,d2
		;[lock in d1]
		jsrlib	Examine
		tst.l	d0	;grrr
		beq.s	e_examine
		move.l	fib_Size(a7),d0
		adda.l	#fib_SIZEOF,a7
		move.l	d0,size
		beq	e_nosize
;-- Get enough memory to fit [d0-size] --
		move.l	a6,-(a7)
		;[size in d0]
		moveq	#0,d1	;Any type of memory
		move.l	4,a6
		jsrlib	AllocMem
		move.l	(a7)+,a6
		move.l	d0,block
		tst.l	d0	;grrr
		beq.s	e_memory
;-- Open object --
		move.l	inline,d1
		move.l	#MODE_OLDFILE,d2
		jsrlib	Open
		move.l	d0,handle
		move.l	d0,d1
		beq.s	e_open
;-- Read entire thing --
		;[handle in d1]
		move.l	block,d2
		move.l	size,d3
		jsrlib	Read
		cmp.l	d0,d3
		bne.s	e_read
;-- Play leapfrog with deallocation --
		moveq	#0,returncode	;OK
e_read
e_open
e_examine
e_lock
e_stuff 	jsrlib	IoErr
		move.l	d0,resultcode
errorentry	move.l	handle,d1
		beq.s	noclose
		jsrlib	Close
noclose 	move.l	lock,d1
		beq.s	nounlock
		jsrlib	UnLock
nounlock	tst.l	returncode
		beq.s	noerrors
;-- print error --
		lea	nocans(pc),a2
		moveq	#nocane-nocans,d3
		bsr.s	DOWRITE      ;error handling?
		bra.s	closeup
;-- print stats --
noerrors	lea	itsats(pc),a2
		moveq	#itsate-itsats,d3
		bsr.s	DOWRITE      ;error handling?
		move.l	block,d0
		moveq	#',',d3
		bsr.s	PRINTLONGHEX ;error handling?
		lea	sizes(pc),a2
		moveq	#sizee-sizes,d3
		bsr.s	DOWRITE      ;error handling?
		move.l	size,d0
		moveq	#10,d3
		bsr.s	PRINTLONGHEX ;error handling?
;-- Close up --
closeup 	move.l	a6,a1	;a6 had DosBase
		move.l	4,a6
		jsrlib	CloseLibrary
		suba.l	a1,a1	;zero out a1
		jsrlib	FindTask	;Find THIS task
		move.l	d0,a0
		move.l	resultcode,pr_Result2(a0) ;set WHY (or RESULT) code
		move.l	returncode,d0
		rts
;-- Handle out of memory --
e_memory	moveq	#20,returncode	;upgrade to FAIL
		moveq	#ERROR_NO_FREE_STORE,resultcode
		bra.s	errorentry
;-- Set misleading error message for zero byte length file --
e_nosize	moveq	#ERROR_FILE_NOT_OBJECT,resultcode
		bra.s	errorentry
;
;error=DOWRITE(buffer,length),dos
; z	       a2     d3<>0
;
DOWRITE 	jsrlib	Output
		move.l	d0,d1
		beq.s	e_noout
		move.l	a2,d2
		;[length in d3]
		jsrlib	Write
e_noout 	;cmp.l	 d0,d3	 ;check for mismatch
		rts
;
;PRINTLONGHEX(value,terminator),dos
;	      d0    d3.b
;
PRINTLONGHEX	;movem.l d0-d3/a0-a1,-(a7)
		subq.l	#5,a7	;Take 10 bytes from stack
		subq.l	#5,a7	;(it's smaller...)
		move.l	a7,a0
		moveq	#7,d2

1$		rol.l	#4,d0
		move.l	d0,d1
		andi.w	#$f,d1
		move.b	hextab2(pc,d1.w),(a0)+
		dbra	d2,1$

		move.b	d3,(a0)
		jsrlib	Output
		move.l	d0,d1
		beq.s	er_output
		move.l	a7,d2
		moveq	#9,d3
		jsrlib	Write
		addq.l	#5,a7
		addq.l	#5,a7
er_output
er_lib		;movem.l (a7)+,d0-d3/a0-a1
		rts

DOSName 	dc.b 'dos.library',0
nocans		dc.b 'Can',39,'t load file',10
nocane
itsats		dc.b 'File loaded at address $'
itsate
sizes		dc.b ' size $'
sizee
hextab2 	dc.b '0123456789ABCDEF'
		dc.b '(C)1987 Bryce Nesbitt'
		dc.b 1	     ;release 1
;-- That's all folks --
