*****************************************************************
*								*
*	Boot Prom for CPU-68K and 64K RAM,			*
*	Morrow HDCDMA hard disk controler,			*
*	DISK1 floppy disk controller,				*
*       INTERFACER 4 as primary console,			*
*	MicroAngelo as secondary console.			*
*								*
*****************************************************************


*****************************************************************
*								*
* File wide .equates.						*
*								*
*****************************************************************

false	.equ	0		; Logical false
truee	.equ	-1		; Logical true
test	.equ	false		; True for non-ROM testing,
*				; causes ROM required goodies
*				; to disappear, and includes
*				; a call to set sup mode.
iobase	.equ	$ff0000		; Address of I/O ports
fdport	.equ	$c0		; Base of Disk1, for stopping boot.
ser	.equ	3		; Port in Disk1 for above purpose.
initsr	.equ	$2700		; init status--no interrupts

datasiz	.equ	4*1024		; Reserved RAM for char BIOS data.

*****************************************************************
*								*
* Boot Loader code begins here.					*
*								*
* Global flags are kept in D7 as follows:			*
*	(Initially set from S1 switches on INTER IV,		*
*	 Bit = 1 if true)					*
*								*
*	S1.10 (Bit  0) - 19,200 baud Console port, else 9600.	*
*	S1.9  (Bit  1) - Use MicroAngelo as console.		*
*	S1.8  (Bit  2) - Auto Floppy boot.			*
*	S1.7  (Bit  3) - Auto Hard disk boot.			*
*								*
*	Bit  7 - RAM is good.  If bad code blocks routines	*
*			       that need RAM.			*
*								*
*	Register usage -- RAM-less portions:			*
*	A7 - Stack pointer (unused as such)			*
*		points to End of RAM.				*
*	A6 - 1st return address for psuedo-subroutines.		*
*	A5 - 2nd return address for psuedo-subroutines.		*
*	A4 - Points to dynamic data storage area.		*
*	A3 - Last used address for addressing defaults.		*
*								*
*	Register usage -- RAM portions:				*
*								*
*								*
*								*
*****************************************************************

	ifne test
	move #62,d0		; Set SUPERVISOR state -- for testing only!!!
	trap #2
	endc
	ifeq test
	.dc.l $10000		; Initial stack pointer
	.dc.l entry		; Initial PC
entry:	clr.b iobase+fdport+ser	; Clear DISK1 boot logic just in case,
*				; also resets CPU boot logic.
	endc

	.globl rom
	.xdef exceptn
rom:	move #initsr,sr		; set no interrupts.
	move.l #exceptn,$7c	; Point NMI to appropriate routine.
	move.l #exceptn,$24	; Point TRACE to appropriate routine.

	move.l #iobase,a3
	move.b #4,$17(a3)	; Select user 4
	move.b $10(a3),d7	; & read dipswitch.

	move.b #5,$17(a3)	; Init Aux 2
	move.b #$4E,$12(a3)	; Async, x16, 1 stp 8 data no par.
	move.b #$76,$12(a3)	; 300 baud.
	move.b #$37,$13(a3)	; Rx & Tx on, DTR & RTS, no brk, reset.

	move.b #6,$17(a3)	; Init Aux 1
	move.b #$4E,$12(a3)	; Async, x16, 1 stp 8 data no par.
	move.b #$7E,$12(a3)	; 9600 baud.
	move.b #$37,$13(a3)	; Rx & Tx on, DTR & RTS, no brk, reset.

	move.b #7,$17(a3)	; Init Console
	move.b #$4E,$12(a3)	; Async, x16, 1 stp 8 data no par.
	btst #0,d7		; Which baud rate?
	bne npoint2
	move.b #$7E,$12(a3)	; 9600 baud.
	bra rom1
npoint2	move.b #$7F,$12(a3)	; 19200 baud.
rom1	move.b #$37,$13(a3)	; Rx & Tx on, DTR & RTS, no brk, reset.

	move.b #1,$ff00f1	; Reset MicroAngelo.
	move.b #0,$ff00f1

	clr.l a3		; Default address for dump et al.
	lea.l rom2,a6		; Figure out mem size and set stack
	bra msize		; at top (if any).
rom2	btst #7,d7
	beq rom3
	move.b #0,contflg(a4)	; If we have RAM then continuation
	lea.l regusp(a4),a0	; isn't possible.  (Ain't anyhow).
	move.w #regsav-regusp-1,d0	; Clear saved reg bank.
regclr	clr.b (a0)+
	dbra d0,regclr
	clr.w (a0)+		; Wipe last two.
	clr.l (a0)

rom3	lea.l signon,a1		; Say hello to the world...
	btst #1,d7
	beq rom4
	lea.l mamsg,a1
rom4	lea.l prompt,a6
	bra pstr

	.xdef go,dump,cont,dotrace
	.xdef fboot,hboot
	.globl prompt

prompt	lea.l promptst,a1	; Print prompt.
	lea.l prompt2,a6
	bra pstr
prompt2	btst #7,d7		; RAM needed for auto boots.
	beq prompt3
	btst #2,d7		; Auto Floppy boot?
	bne fboot
	btst #3,d7		; Auto Hard boot?
	bne hboot
prompt3	lea.l prompt4,a5
	bra konin		; Get command.
prompt4	cmpi.b #'?',d0		; Help?
	beq help
	bclr #5,d0		; Else mask case.
	cmpi.b #'@',d0		; Check range, @..Z allowed.
	blo prompt2
	cmpi.b #'Z',d0
	bhi prompt2
	lea.l qisa,a5		; Echo command letter.
	bra konout
qisa	cmp.b #'A',d0		; Alternate console?
	bne qisat
	bchg #1,d7
	bra prompt
qisat	cmp.b #'@',d0		; Mem set?
	bne qist
	bra at
qist	cmp.b #'M',d0		; Mem Test (size)?
	bne qisr
	bra mtest
qisr	cmp.b #'R',d0		; Read Scope loop?
	bne qisw
	bra reads
qisw	cmp.b #'W',d0		; Write Scope loop?
	bne qisinp
	bra writs
qisinp	cmpi.b #'I',d0		; Input from port?
	bne qisout
	bra input
qisout	cmpi.b #'O',d0		; Output to port?
	bne qisg
	bra output
qisg	btst #7,d7		; RAM commands follow from here,
	beq mberr		; so make sure there is some.
	cmp.b #'G',d0		; Go?
	bne qiscon
	bra go
qiscon	cmp.b #'C',d0		; Breakpoint continue?
	bne qistr
	bra cont
qistr	cmp.b #'T',d0		; Trace?
	bne qish
	bra dotrace
qish	cmpi.b #'H',d0		; Hard disk boot?
	bne qisd
	bra hboot
qisd	cmpi.b #'F',d0		; Floppy disk boot?
	bne qisdu
	bra fboot
qisdu	cmpi.b #'D',d0		; Dump memory?
	bne iserr
	bra dump
iserr	lea.l errmsg,a1		; Else an error, go round again.
	lea.l prompt,a6
	bra pstr

mberr	lea.l mbmsg,a1
	bra help2
help	lea.l helpmsg,a1
help2	lea.l prompt,a6
	bra pstr

*****************************************************************
*								*
*	Start of Commands					*
*								*
*****************************************************************


***************************************************************

at	lea.l at1,a6		; RAM-less set memory command.
	bra atol		; Get address in d1
at1	move.l d1,a0		; and a0.
	move.b d0,d3		; save terminating character.
	tst d2			; No start specified, use default.
	bne atloop
	movea.l a3,a0
atloop	lea.l at2,a6
	bra crlf
at2	move.l a0,d1
	lea.l atword,a6
	bra paddr		; print address.
atword	cmp.b #'W',d3		; was it a WORD modify?
	bne atlong
	move.l a0,d0		; make sure an even address.
	btst #0,d0
	bnz oddaddr
	move (a0),d1
	lea.l atwd1,a6
	bra pword
atwd1	lea.l atwd2,a6		; Get input word.
	bra atol
atwd2	tst d2
	beq atwd3
	move d1,(a0)+		; Stash it and loop.
	bra atloop
atwd3	cmp.b #$0a,d0		; Check for other commands.
	bne atwd4
	subq.l #2,a0		; Was a backup, do so and goto loop.
	bra atloop
atwd4	cmp.b #'.',d0		; Quit?
	beq atend
	cmp.b #$0d,d0		; NOP
	bne atend
	addq.l #2,a0		; Increment addr and loop.
	bra atloop
atlong	cmp.b #'L',d3		; was it a LONG modify?
	bne atbyte
	move.l a0,d0		; make sure an even address.
	btst #0,d0
	bnz oddaddr
	move.l (a0),d1
	lea.l atlng1,a6
	bra plong		; Print contents.
atlng1	lea.l atlng2,a6		; Get input long.
	bra atol
atlng2	tst d2
	beq atlng3
	move.l d1,(a0)+		; Stash it and loop.
	bra atloop
atlng3	cmp.b #$0a,d0		; Check for other commands.
	bne atlng4
	subq.l #4,a0		; Was a backup, do so and goto loop.
	bra atloop
atlng4	cmp.b #'.',d0		; Quit?
	beq atend
	cmp.b #$0d,d0		; NOP
	bne atend
	addq.l #4,a0		; Increment addr and loop.
	bra atloop
atbyte	move.b (a0),d1
	lea.l atbyt1,a6
	bra pbyte
atbyt1	lea.l atbyt2,a6		; Get input byte.
	bra atol
atbyt2	tst d2
	beq atbyt3
	move.b d1,(a0)+		; Stash it and loop.
	bra atloop
atbyt3	cmp.b #$0a,d0		; Check for other commands.
	bne atbyt4
	subq.l #1,a0		; Was a backup, do so and goto loop.
	bra atloop
atbyt4	cmp.b #'.',d0		; Quit?
	beq atend
	cmp.b #$0d,d0		; NOP
	bne atend
	addq.l #1,a0		; Increment addr and loop.
	bra atloop
atend	movea.l a0,a3		; Save last addr for default.
	bra prompt

oddaddr	lea.l oddmsg,a1
	lea.l prompt,a6
	bra pstr


***************************************************************

input	lea.l input1,a6		; RAM-less port read command.
	bra atol
input1	and.l #$ff,d1		; Mask to I/O address range.
	add.l #iobase,d1	; Map to I/O space
	move.l d1,a0		; got address.
	move.b d0,d3		; Save terminating character.
	tst d2			; No start specified, use default.
	bne inloop
	movea.l a3,a0
inloop	lea.l input2,a6
	bra crlf
input2	move a0,d1
	lea.l inword,a6
	bra pbyte		; print port address.
inword	cmp.b #'W',d3		; was it a WORD read?
	bne inbyte
	move a0,d0		; make sure an even address.
	btst #0,d0
	bnz oddaddr
	move (a0),d1
	lea.l inwd1,a6
	bra pword
inwd1	lea.l inwd2,a6		; Get input "word".
	bra atol
inwd2	cmp.b #$0d,d0		; Loop?
	beq inloop
	bra prompt
inbyte	move.b (a0),d1
	lea.l inwd1,a6
	bra pbyte


***************************************************************

output	lea.l output1,a6	; RAM-less port write command.
	bra atol
output1	and.l #$ff,d1		; Mask to I/O port range.
	add.l #iobase,d1	; Map to I/O port space.
	move.l d1,a0		; got address.
	move.b d0,d3		; Save terminating character.
	tst d2			; No start specified, use default.
	bne outloop
	movea.l a3,a0
outloop	lea.l output2,a6
	bra crlf
output2	move a0,d1
	lea.l outword,a6
	bra pbyte		; print port address.
outword	cmp.b #'W',d3		; was it a WORD modify?
	bne outbyte
	move a0,d0		; make sure an even address.
	btst #0,d0
	bnz oddaddr
	move (a0),d1
	lea.l outwd1,a6
	bra pword
outwd1	lea.l outwd2,a6		; Get input word.
	bra atol
outwd2	tst d2			; Nothing? default or done.
	beq outwd4
	move d1,d4		; Save for default.
outwd3	move d1,(a0)		; Stash it and loop.
	bra outloop
outwd4	cmp.b #$0d,d0		; Default?
	bne prompt		; No, quit.
	move d4,d1		; Restore default.
	lea.l outwd5,a6		; print it.
	bra pword
outwd5	move d4,d1		; (restore)
	bra outwd3		; and poke it.

outbyte	move.b (a0),d1
	lea.l outbyt1,a6
	bra pbyte
outbyt1	lea.l outbyt2,a6	; Get input byte.
	bra atol
outbyt2	tst d2			; Nothing? default or done.
	beq outbyt4
	move.b d1,d4		; Save for default.
outbyt3	move.b d1,(a0)		; Stash it and loop.
	bra outloop
outbyt4	cmp.b #$0d,d0		; Default?
	bne prompt		; No, quit.
	move.b d4,d1		; Restore default.
	lea.l outbyt5,a6	; print it.
	bra pbyte
outbyt5	move.b d4,d1		; (restore)
	bra outbyt3		; and poke it.


***************************************************************

mtest	lea.l mtest2,a6		; Monitor Mem size command.
	bra msize
mtest2	lea.l mtest3,a6		; (Assumes RAM comes in 512 increments)
	bra crlf
mtest3	lea.l prompt,a6
	move.l a7,d1
	bra paddr		; print end-of-mem


***************************************************************

reads	lea.l read2,a6		; Monitor Read Scope loop
	bra atol
read2	tst d2
	beq prompt
	move.l d1,a0
	move.b d0,d3		; save terminator
	lea.l read3,a6
	bra crlf
read3	lea.l read4,a6
	bra paddr
read4	cmp.b #'W',d3
	beq readw
	lea.l readb1,a5
readb	tst.b (a0)		; four times so recognizable
	tst.b (a0)		; on the bus.
	tst.b (a0)
	tst.b (a0)
	bra konstat
readb1	beq readb
	bra prompt

readw	lea.l readw2,a5
readw1	tst.w (a0)		; four times so recognizable
	tst.w (a0)		; on the bus.
	tst.w (a0)
	tst.w (a0)
	bra konstat
readw2	beq readw1
	bra prompt


***************************************************************

writs	lea.l writ2,a6		; Monitor Write Scope loop
	bra atol
writ2	tst d2
	beq prompt
	move.l d1,a0
	move.b d0,d3		; save terminator
	lea.l writ3,a6
	bra crlf
writ3	lea.l writ4,a6
	bra paddr
writ4	clr.w d0		; for writing
	cmp.b #'W',d3
	beq writw
	lea.l writb1,a5
writb	move.b d0,(a0)		; four times so recognizable
	move.b d0,(a0)		; on the bus.
	move.b d0,(a0)
	move.b d0,(a0)
	bra konstat
writb1	beq writb
	bra prompt

writw	lea.l writw2,a5
writw1	move.w d0,(a0)		; four times so recognizable
	move.w d0,(a0)		; on the bus.
	move.w d0,(a0)
	move.w d0,(a0)
	bra konstat
writw2	beq writw1
	bra prompt


msize	move.l #0,a7		; Find memory size in 512 byte increments.
	clr d0			; 0 pages...
	bclr #7,d7		; mark memory as bad until further notice
msloop	move.b (a7),d1		; get test loc
	move.b d1,d2
	not.b d2
	move.b d2,(a7)		; stuff complement of orig value.
	move.b (a7),d2
	move.b d1,(a7)		; restore original value.
	not.b d2
	cmp.b d1,d2		; All bits work?
	bne msend
	adda.l #512,a7		; Bump to next page
	inc d0			; and increment count.
	bra msloop
msend	cmp #128,d0		; Mark RAM good if at least 64k.
	blt msng		; Leave stack at end+1 of RAM.
	bset #7,d7
msng	movea.l a7,a4		; pointer to Data area.
	suba.l #datasiz,a4
	move.l a7,dataend(a4)	; End of RAM pointer in Data block.
	move.l #'BIOS',marker(a4)	; Put in Fence.
	move.l a4,4		; Stash pointer at RESET PC vector.
	move.w ilins,0		; Put an ILLEGAL at RESET SSP vector.
	jmp (a6)
	.globl ilins
ilins	illegal
	
*****************************************************************
*								*
*	Miscellaneous Support routines				*
*								*
*****************************************************************

	.globl pstr
pstr:	move.b (a1)+,d0		; Print string -> A1. Stop on NULL
	beq pstr2
	lea.l pstr,a5		; Konout uses a5 for ret addr
	bra konout
pstr2	jmp (a6)

crlf	lea.l crlf1,a5		; Print CR-LF pair.
	move.b #10,d0
	bra konout
crlf1	lea.l crlf2,a5
	move.b #13,d0
	bra konout
crlf2	jmp (a6)

konout	btst #1,d7
	bnz mkonout
	move.b #7,iobase+$17	; Select User 7 on INTERFACER IV
konout1	btst #0,iobase+$11
	beq konout1
	move.b d0,iobase+$10
konret	jmp (a5)
mkonout	btst #0,iobase+$f1
	bnz mkonout
	move.b d0,iobase+$f0
	jmp (a5)

konin	move.b #7,iobase+$17	; Select User 7 on INTERFACER IV
konin1	btst #1,iobase+$11
	beq konin1
	move.b iobase+$10,d0
	jmp (a5)

konstat	move.b #7,iobase+$17	; Select User 7 on INTERFACER IV
	btst #1,iobase+$11	; returns Z if nothing there.
	jmp (a5)

pbyte	move.l #$20018,d0	; 2 nybbles, 24 bit shift first.
	bra pdigits
pword	move.l #$40010,d0	; 4 nybbles, 16 bit shift first.
	bra pdigits
paddr	move.l #$60008,d0	; 6 nybbles, 8 bit shift first.
	bra pdigits
plong	move.l #$80000,d0	; 8 nybbles, no shift first.
pdigits	rol.l d0,d1		; do shift.
	bra pdigent
pdiglop	swap d0			; save nybble count.
	rol.l #4,d1		; Print variable in d1.
	lea.l pdigent,a5
	bra ntoa
pdigent	swap d0			; get nybble count.
	dbra d0,pdiglop
	move.b #' ',d0
	lea.l pdigend,a5
	bra konout
pdigend	jmp (a6)

ntoa	move.b d1,d0		; nybble in d0 to ascii, then output
	and #$f,d0
	cmp #$a,d0
	blt htoa2
	add.b #'A'-'9'-1,d0
htoa2	add.b #'0',d0
	bra konout

atol	clr.l d1		; ascii to long, stops on invalid hex char.
	clr d2			; returns long in d1, terminator char in d0,
atol1	lea.l atol2,a5		; d2=1 if any chars entered before terminator.
	bra konin
atol2	cmp.b #$40,d0
	blo atol3
	and #$5F,d0		; mask to upper case.
atol3	cmpi.b #'0',d0		; check range (0..9,A..F).
	blo atolend
	cmpi.b #'F',d0
	bhi atolend
	cmpi.b #'9',d0
	bls atol4
	cmpi.b #'A',d0
	bhs atol4
	bra atolend
atol4	moveq #1,d2		; Valid characters entered, flag it.
	lea.l atol5,a5		; Echo the valid char.
	bra konout
atol5	sub.b #'0',d0
	cmp.b #$9,d0
	bls atol6
	sub.b #'A'-'9'-1,d0
atol6	ext d0			; to long.
	ext.l d0
	asl.l #4,d1		; tack it onto d1.
	add.l d0,d1
	bra atol1
atolend	jmp (a6)

	.data
mamsg	.dc.b $80,$E8,$BA,1,$83,10	; Inits MicroAngelo
	ifne test
signon	.dc.b 12,'CPU-68K Monitor version TEST',10,13,0
	endc
	ifeq test
signon	.dc.b 12,'CPU-68K Monitor version 2.0',10,13,0
	endc

promptst .dc.b 10,13,':',0
errmsg	.dc.b ' Unknown command, use "?" for help.',0
mbmsg	.dc.b 10,13,' Function unavailable without memory!',0
	.globl oddmsg
oddmsg	.dc.b ' Invalid (odd) memory address!',0

helpmsg	.dc.b '?',10,13
	.dc.b '               ---  RAM-less Commands ---',10,13
	.dc.b '    @ - Set Memory (B,W,L)     R - Read Scope loop (B,W)',10,13
	.dc.b '    M - Test Memory Size       W - Write Scope loop (B,W)',10,13  
	.dc.b '    I - Input from port (B,W)  O - Output to port (B,W)',10,13
	.dc.b '    A - Alternate console',10,13
	.dc.b 10,13
	.dc.b '                   --- RAM Commands ---',10,13
	.dc.b '    D - Dump memory (B,W,L)    G - Go to address',10,13
	.dc.b '    T - Trace                  C - Continue execution',10,13
	.dc.b '    F - Floppy disk boot       H - Hard disk boot',10,13
	.dc.b 10,13
	.dc.b " The RAM-less commands don't require RAM to function,",10,13
	.dc.b ' the RAM commands must have RAM to function.',10,13
	.dc.b ' RAM is tested upon reset and with the "M" command.',10,13
	.dc.b ' All commands that take an address and can operate on',10,13
	.dc.b ' words or longs use a "W" or "L" as an address',10,13
	.dc.b ' terminator to change to that mode.',10,13
	.dc.b 0

*				; Dynamic Data storage area.
	offset 0
marker	ds.l 1			; If corrupt, data area may be too.
dataend	ds.l 1			; Points to end of this block.

conovec	ds.w 1			; Console out physical device vector
conivec ds.w 1			; Console in physical device vector
ax1ovec ds.w 1			; Aux 1 out pdv
ax1ivec ds.w 1			; Aux 1 in pdv
ax2ovec ds.w 1			; Aux 2 out pdv
ax2ivec ds.w 1			; Aux 2 in pdv
lstovec	ds.w 1			; List out pdv

oldrd7	ds.b 1			; Saved D7 for flags.
contflg	ds.b 1			; Continuation allowed flag.
regusp	ds.l 1			; Register save storage.
regd0	ds.l 1
regd1	ds.l 1
regd2	ds.l 1
regd3	ds.l 1
regd4	ds.l 1
regd5	ds.l 1
regd6	ds.l 1
regd7	ds.l 1
rega0	ds.l 1
rega1	ds.l 1
rega2	ds.l 1
rega3	ds.l 1
rega4	ds.l 1
rega5	ds.l 1
rega6	ds.l 1
rega7	ds.l 1
regsav	ds.b 0
regsr	ds.w 1
regpc	ds.l 1
regfmt	ds.w 1
regsav2	ds.b 0

outtab	ds.l 32			; Pointers to output handlers & A6 data areas
outstab	ds.l 32			; Pointers for output status
intab	ds.l 32			; Pointers for input
instab	ds.l 32			; Pointers for input status

	.end
