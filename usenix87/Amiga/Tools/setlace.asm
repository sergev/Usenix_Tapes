;tabs every 8
;
;SetLace implementation by Bryce Nesbitt, based on an example by Bob
;Pariseau of C-A. Public Domain.
;
;On most monitors a non-interlaced display will leave black bars between
;each scan line.  This is evident in all the 200 line display modes of the
;Amiga 1000.  For photographs or longer persistence displays this black is
;undesirable.  SetLace instructs the system to set interlace; on each
;successive field the same display data will be painted in between that of 
;the previous field.  This may be a necessity for use of the Amiga with
;some NTSC video gear and broadcast television equipment.  For most computer
;(non-broadcast) applications interlace will flicker unacceptably unless
;the contrast, brightness and room lighting are all turned way down.
;
;CLI Syntax:	SetLace		;Flip (XOR) interlace
;		SetLace 1	;Turn lace on
;		SetLace 0	;Turn lace off
;Workbench:			;Flip (XOR)
;ALL screens will be forced to interlace
;
;Author correspondence, bug or stupidity reports may be directed to:
;	Bryce Nesbitt
;	1712 Marin Ave.
;	Berkeley, Ca 94707-2902
*************************
	NOLIST
	INCLUDE 'exec/types.i'
	INCLUDE 'libraries/dosextens.i'
	INCLUDE 'graphics/display.i'
	INCLUDE 'graphics/gfxbase.i'
	;INCLUDE 'lib/exec_lib.i'	;eliminates link with amiga.lib
	;INCLUDE 'lib/dos_lib.i'	;non-standard, and very speedy!
	;INCLUDE 'lib/intuition_lib.i'	;Can be fabricated from the fd.files
	;INCLUDE 'lib/graphics_lib.i'	;directory. (see exec/exec_lib.i)
	LIST
jsrlib	macro
	xref	_LVO\1			;^Nuke for use as above^
	jsr	_LVO\1(a6)
	endm
jmplib	macro
	xref	_LVO\1
	jmp	_LVO\1(a6)
	endm
**************************
		CODE
startup:	move.l	a0,a4		;CLI line pointer
		move.l	d0,d4		;CLI line length
		move.l	4,a6
		suba.l	a1,a1		;Get THIS task
		jsrlib	FindTask
		move.l	d0,a5
		moveq	#0,d0		;Set zero for later
		move.l	pr_CLI(a5),d1	;Pointer to CLI only structure
		bne.s	fromCLI		;If not zero, then save a zero...
;
; If called from Workbench a message will be sent.  This waits for it,
;  and saves it's pointer to be returned to Workbench later.
;
		lea	pr_MsgPort(a5),a0
		jsrlib	WaitPort
		lea	pr_MsgPort(a5),a0
		jsrlib	GetMsg		;Message pointer in D0
		moveq	#0,d4		;clear CLI line pointer
fromCLI 	move.l	d0,-(a7)	;Save message for later...


******************************** [A6=ExecBase][a5=this task][a4=CLI line]
* [d4=CLI length]
returncode	equr	d7
ibase		equr	d6
gbase		equr	d5
* (d4,a4,a5)

		moveq	#20,returncode	;default to 'severe failure' code
		lea	IntuiName(pc),a1
		moveq	#0,d0
		jsrlib	OpenLibrary
		move.l	d0,ibase	 ;Error check. What a pain! Could we
		beq.s	e_Intui		 ;not agree that Intuition and a few
		lea	Graphname(pc),a1 ;other core libraries will always
		moveq	#0,d0		 ;be openable??
		jsrlib	OpenLibrary
		move.l	d0,gbase
		beq.s	e_Graph
;
;Modifying the system-wide bplcon0.  From here we can change all screens
;to INTERLACE and turn tricks like disabling colorburst.  The Forbid/
;Permit pair is used for saftey while modifying this stuff.
;
		jsrlib	Forbid
		move.l	gbase,a6	;gbase

		subq.l	#2,d4		;0=wb 1=cli no args 2=1 arg
		bmi.s	flip		;wb, or cli flip
		bne.s	toomanychars
		cmpi.b	#'0',(a4)
		beq.s	clear
		cmpi.b	#'1',(a4)
		bne.s	badargs

setlace		ori.w	#INTERLACE,gb_system_bplcon0(a6)	;Set lace
		bra.s	break
clear		andi.w	#$ffff-INTERLACE,gb_system_bplcon0(a6)	;Clear lace
		bra.s	break
flip		eori.w	#INTERLACE,gb_system_bplcon0(a6) 	;Flip lace

break		move.l	ibase,a6
		jsrlib	RemakeDisplay	;The big one! Rethink ALL displays
		moveq	#0,returncode	;Everything is OK!
		bra.s	goodexit
badargs
toomanychars	move.l	#120,pr_Result2(a5) ;'arg line invalid or too long'
		moveq	#10,d7		;return code 10, less serious error
goodexit	move.l	4,a6
		jsrlib	Permit
;
;RemakeDisplay() is spec'ed to do a Forbid() and then Permit() around it's
; lengthy operation. This is just a saftey valve in case it did not...
;
		move.l	gbase,a1
		jsrlib	CloseLibrary
e_Graph		move.l	ibase,a1
		jsrlib	CloseLibrary
e_Intui
******************************** (a7)+=message d7=return code
ExitToDOS	move.l	(a7)+,d6
		beq.s	NotWB	;If saved pointer is zero, exit to CLI...
;
; Return the startup message to the parent Workbench tool. The forbid
; is needed so workbench can't UnLoadSeg() the code too early.
;
		move.l	4,a6
		jsrlib	Forbid
		move.l	d6,a1		;message pointer
		jsrlib	ReplyMsg
NotWB		move.l	d7,d0		;Set "failat" code
		rts
IntuiName	dc.b   'intuition.library',0
Graphname	dc.b  'graphics.library',0
MyName		dc.b 'Bryce Nesbitt'
;-- this is the end --
