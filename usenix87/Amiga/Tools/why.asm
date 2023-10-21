; Tabs set every 8.
;
;BETTERCLI(tm) "WHY" Command, release 1
;
;BETTERCLI WHY (C)1987 Bryce Nesbitt.  Unlimited free non-exclusive licence
;hereby	granted	to any sentient	being to use or	abuse this code	in any way
;whatsoever provided that this and any other copyright notices remain fully
;attached and are reproduced in	any simultaneously distributed printed matter
;and with the exception	that, without prior written permission,	it not
;be utilized by	any entity that	has been commonly referred to as Robert
;W. Skyles, Skyles Electric Works, Jim Drew, Regie Warren or any organization
;founded by, controlled, employing or profiting	any such entity, it's
;offspring or spouses.	ARP and	FISH use welcome.
;Author	correspondence,	bug or stupidity reports may be	directed to:
;	Bryce Nesbitt
;	1712 Marin Ave.
;	Berkeley, Ca 94707-2206
;
;This program replaces the CLI WHY command, providing faster execution and
;enhanced functionality.  The old WHY command was simply a stub	that called
;'C:FAULT', and thus could not be made properly RESIDENT.  This command was
;to replace FAULT as well, however the BCPL code does something	funky
;when calling FAULT, and I could not discover it's secret.
;
; Return code convention:
;  20  FAIL  -serious error, DOS will not open,	out of memory.
;  10  ERROR -user caused error	(syntax, file not found	etc.)
;	     -less serious than	FAIL.
;  5   WARN  -technicality (file not found on DELETE, for example)
;  0   OK    -command sucessful, operation completed.
;
;This code is REUSABLE,REENTRANT and fully RESIDENT COMPATIBLE.
;Stack 1600 ok.
*************************
jsrlib	macro
	xref	_LVO\1
	jsr	_LVO\1(a6)
	endm
jmplib	macro
	xref	_LVO\1
	jmp	_LVO\1(a6)
	endm
	;INCLUDE 'lib/exec_lib.i'	;You don't have these files
	;INCLUDE 'lib/dos_lib.i'	;so please ignore these lines.
	INCLUDE	'libraries/dosextens.i'
*************************
		CODE
outhand		equr	d6
failcode	equr	d5
proc		equr	a5
cli		equr	a4
;d0-d4 a0-a3 scratch
;-- Setup --
StartUp:	moveq	#20,failcode	;default-SEVERE	failure
		move.l	4,a6		;get exec library base pointer
		lea	dosname(pc),a1
		jsrlib	OldOpenLibrary	;get this... Compatability with	V1.0!
		move.l	d0,outhand	;temp for DOSBase
		beq.s	e_dos		;fail if it will not open
		suba.l	a1,a1		;find THIS task
		jsrlib	FindTask
		move.l	d0,proc
		move.l	pr_CLI(proc),cli
		adda.l	cli,cli		;ban the BPTR!!
		adda.l	cli,cli		;ban the BPTR!!
		move.l	outhand,a6	;DOSBase
		jsrlib	Output
		move.l	d0,outhand
		beq.s	e_output
;--
		lea	mes1s(pc),a0	;'Fail code '
		moveq	#mes1e-mes1s,d3
		bsr.s	DOWRITED
;-- Get	fail code --
		move.l	cli_ReturnCode(cli),d0
		bsr.s	PRINTDECIMAL
;--
		lea	mes2s(pc),a0	;'and error code '
		moveq	#mes2e-mes2s,d3
		bsr.s	DOWRITED
;-- Print error	number --
		move.l	cli_Result2(cli),d0
		bsr.s	PRINTDECIMAL
;--
		lea	mes3s(pc),a0	;';'
		moveq	#mes3e-mes3s,d3
		bsr.s	DOWRITED
;-- Print error	text --
		move.l	cli_Result2(cli),d0
		bsr.s	PCODE
		moveq	#0,failcode
e_error
e_output	move.l	a6,a1
		move.l	4,a6
		jsrlib	CloseLibrary
e_dos		move.l	failcode,d0
rts1		rts
;
;DOWRITED(pointer,length)
;	  a0	  d3
;Aborts	if error
;
DOWRITED	;[d3=length]
		move.l	a0,d2
		move.l	outhand,d1
		jsrlib	Write
		cmp.l	d0,d3
		beq.s	rts1
e_badwrite	addq.l	#4,a7	;pop return off	stack
		jsrlib	IoErr
		move.l	d0,pr_Result2(proc)
		bra.s	e_error	;(return code is set to	FAIL, 20)
;
;
;
PCODE		moveq	#10,d1
		lea	table(pc),a1
checknext	move.l	a1,a0		;Save pointer to this line
		moveq	#-1,d3
findend		cmp.b	(a1)+,d1	;count length
		dbeq	d3,findend	;68010 loop mode!
		neg.l	d3		;compensate for	backwards count
		subq.l	#1,d3		;tweak
		cmp.b	#$ff,(a0)
		beq.s	gotit		;end of	list, use default
		cmp.b	(a0),d0
		bne.s	checknext
gotit		addq.l	#1,a0		;skip error #
		bra.s	DOWRITED	;[a0-string|d3-len]
;
;Bloated, slow, kludg-o 32 bit signed print decimal routine.
;destroys D2,D3
;
countvar	equr	d1
tableoffset	equr	d2

PRINTDECIMAL	moveq	#-4,tableoffset
		subq.l	#6,a7
		subq.l	#6,a7	;reserve 12 bytes for number
		move.l	a7,a0
		tst.l	d0
		beq.s	zero
		bpl.s	positive
		move.b	#'-',(a0)+
		neg.l	d0
positive	addq.l	#4,tableoffset
		cmp.l	table2(pc,tableoffset.l),d0
		bcs.s	positive	;BLO

doloop		move.l	table2(pc,tableoffset.w),countvar
		beq.s	endofnum
		moveq	#-1,countvar
subloop		addq.l	#1,countvar
		sub.l	table2(pc,tableoffset.w),d0
		bcc.s	subloop
		add.l	table2(pc,tableoffset.w),d0 ;Ooops! Too	far!!
		ori.b	#$30,countvar
		move.b	countvar,(a0)+
		addq.l	#4,tableoffset
		bra.s	doloop

zero		move.b	#$30,(a0)+
endofnum	suba.l	a7,a0
		move.l	a0,d3
		move.l	a7,d2
		move.l	outhand,d1
		jsrlib	Write
		addq.l	#6,a7
		addq.l	#6,a7
		cmp.l	d0,d3
		bne.s	e_badwrite
		rts

table2		dc.l	1000000000
		dc.l	100000000
		dc.l	10000000
		dc.l	1000000
		dc.l	100000
		dc.l	10000
		dc.l	1000
		dc.l	100
		dc.l	10
		dc.l	1
		dc.l	0
dosname		dc.b 'dos.library',0
mes1s		dc.b 'Fail code '
mes1e
mes2s		dc.b ', and error code '
mes2e
mes3s		dc.b '; '
mes3e
		dc.b '(C)1987 Bryce Nesbitt'
;
;Error message table. Feel free	to translate into other	languages such as
;German, Italian, or even English. Error numbers 200 and 201
;are unauthorized 'enhancments' to the standard.
;
table	dc.b 103,'insufficient free store',10
	dc.b 105,'task table full',10           ;!!Eliminate need!!
	dc.b 120,'argument line invalid or too long',10
	dc.b 121,'file is not an object module',10
	dc.b 122,'invalid resident library during load',10
       dc.b 200,'internal error',10
       dc.b 201,'newer operating system required',10
	dc.b 202,'object in use',10
	dc.b 203,'object already exists',10
	dc.b 204,'directory not found',10
	dc.b 205,'object not found',10
	dc.b 206,'invalid window description',10
       ;dc.b 207,'object too large',10              ;obsolete
	dc.b 209,'packet request type unknown',10
	dc.b 210,'stream name component invalid',10 ;what??
	dc.b 211,'invalid object lock',10
	dc.b 212,'object not of required type',10
	dc.b 213,'disk not validated',10
	dc.b 214,'disk write-protected',10
	dc.b 215,'rename across devices attempted',10
	dc.b 216,'directory not empty',10
       ;dc.b 217,'too many levels;,10               ;obsolete
	dc.b 218,'device (or volume) not mounted',10
	dc.b 219,'seek failure',10
	dc.b 220,'comment too big',10
	dc.b 221,'disk full',10
	dc.b 222,'file is protected from deletion',10
	dc.b 223,'file is write protected',10
	dc.b 224,'file is read protected',10
	dc.b 225,'not a valid DOS disk',10
	dc.b 226,'no disk in drive',10
	;dc.b 227,'can',$27,'t open window',10
	;dc.b 228,'can',$27,'t open screen',10
	dc.b 232,'no more entries in directory',10
	dc.b 255,'no error message',10
;-- end --
