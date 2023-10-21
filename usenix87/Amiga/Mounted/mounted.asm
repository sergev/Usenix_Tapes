;MOUNTED Copyright 1987 Bryce Nesbitt.	Free, revokable licence hereby 
;granted for any entity to use or absue this code in any way they see fit,
;provided that such entity, in the judgement of the author:
;1> Retains this and any other Copyright notices.
;2> Is paid up on any monetary imbalance with author.
;3> Does not have unjustified litigation pending against author.
;Please tell me about any enhacements,changes,bugs or brain-damage in this
;code.	bryce@hoser.berkeley.EDU -or- bryce@cogsci.berkeley.EDU
;-or- ucbvax!hoser!bryce -or- ucbvax!cogsci!bryce -or-
;1712 Marin Ave. Berkeley, Ca 94707-2206
;Some inspirational assistance by Peter da Silva.  FISH use ok.
;
;FUNCTION:
;  MOUNTED can determind if a volume or file is on-line.  It may be used
;for conditional execution in a command file.  If returns OK if the path
;is present, or WARN if it is not.  It does the test "quietly" without
;bringing up a requester.
;
;FAIL  means serious error, like no DOS or memory
;ERROR is less fatal, maybe syntax or "file not found" for a TYPE command.
;WARN  is used for things like "file not found" on a DELETE or here in
;      MOUNTED.  Also used by TYPE when <CTRL><C> is hit.
;
;EXAMPLE:
; mounted assem:c
; if not warn
;  echo #27 "[43m"
;  path assem:c
; endif
;
;BUGS:
;  Because I use the parser code from the ARP.LIBRARY, which I cannot yet
;distribute, no parsing of the line is done.  The entire input line will be
;used, spaces, quotes and all.
;  It would be nice to be able to determine if a physical device such as
;DF0: is present.  As it stands MOUNTED will return a WARN if a connected
;drive is present, but empty.  There are better ways of attaining this
;function than adding a kludge to MOUNTED.
;  The executable is 152 bytes long when assembled with METACOMCO 10.178
;and linked using BLINK 6.5 with the NODEBUG option enabled.

***********************
	NOLIST
	;INCLUDE 'lib/exec_lib.i'
	;INCLUDE 'lib/dos_lib.i'
	INCLUDE 'exec/ables.i'
	INCLUDE 'libraries/dosextens.i'
	LIST
jsrlib	MACRO
	XREF	_LVO\1
	jsr	_LVO\1(a6)
	ENDM
jmplib	macro
	XREF	_LVO\1
	jmp	_LVO\1(a6)
	ENDM
blink	MACRO
	bchg.b	#1,$bfe001
	ENDM
***********************
DOSBase 	equr a5
MyProcess	equr a4
LinePointer	equr a3
returncode	equr d7
WindowSave	equr d6

		CODE
startup:	clr.b	-1(a0,d0)	;cheap way to NULL terminate
		moveq	#5,returncode	;default WARN condition
		move.l	a0,LinePointer

		move.l	4,a6
		lea	DOSName(pc),a1
		jsrlib	OldOpenLibrary	;V1.0 Compatible
		move.l	d0,DOSBase	;Look ma, no error check!
		suba.l	a1,a1
		jsrlib	FindTask	;Process, really
		move.l	d0,MyProcess
;-- Report errors "quietly" --
		move.l	pr_WindowPtr(MyProcess),WindowSave
		moveq	#-1,d0
		move.l	d0,pr_WindowPtr(MyProcess)
;-- Attempt lock --
		move.l	LinePointer,d1
		moveq	#ACCESS_READ,d2
		move.l	DOSBase,a6
		jsrlib	Lock		;Attempt lock
		tst.l	d0
		beq.s	NoLock		;Can't find it, exit code 10
		move.l	d0,d1
		jsrlib	UnLock
		moveq	#0,returncode	;It's there, return zero
;-- Restore environment / "noisy" errors --
NoLock: 	move.l	WindowSave,pr_WindowPtr(MyProcess)
		move.l	DOSBase,a1
		move.l	4,a6
		jsrlib	CloseLibrary
		move.l	returncode,d0
		rts
DOSName:	dc.b	'dos.library',0
		dc.b	'(C)1987 Bryce Nesbitt'
		END
