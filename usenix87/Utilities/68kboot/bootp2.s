iobase	.equ $ff0000
xsize	.equ 8			; Exception stack size, 6=68K 8=68010

*****************************************************************
*								*
*	RAM commands						*
*								*
*****************************************************************

	.globl dump
	.xdef prompt
dump	bsr atol		; Dump command, get address & flag.
	move.l d1,a0		; Save address.
	move d0,d3		; Save terminator.
	tst d2
	bne defstrt		; No address, use 1+last dumped.
	movea.l a3,a0
defstrt	move.l a0,a1		; Default end address.
	adda #127,a1
	cmp.b #',',d3		; Is the end addr to be specified?
	bne defend		; No, use default end.
	move.b #',',d0
	bsr konout		; Print a comma.
	bsr atol		; Grab ending value (in d1 till later).
	move.l d1,a1		; The end address.
	move d0,d3		; Save new terminator.
	tst d2
	beq prompt		; No terminating address, abort.
defend	cmp.l a1,a0		; Check to make sure the range is good.
	bhi dumperr		; No good, say so.
	move.l a0,a2		; Make a0 anchor for ASCII dump.
dlnlop	move #15,d4		; 16 bytes across the screen.
	bsr crlf
	move.l a0,d1		; print address.
	bsr paddrsp
dvllop	cmp.b #'W',d3		; print appropriate value.
	bne dqisl
	move.l a2,d1		; check for odd address.
	btst #0,d1
	bne modderr
	move (a2)+,d1
	bsr pwordsp
	subq #1,d4		; if word knock off 1 for term count.
	bra ddidval
dqisl	cmp.b #'L',d3
	bne disbyt
	move.l a2,d1		; check for odd address.
	btst #0,d1
	bne modderr
	move.l (a2)+,d1
	bsr plongsp
	subq #3,d4		; if long knock off 3 for term count.
	bra ddidval
disbyt	move.b (a2)+,d1
	bsr pbytesp
ddidval	dbra d4,dvllop		; If more to go on the line then do them.
	move.b #' ',d0
	bsr konout		; Two more spaces to separate.
	bsr konout
	move #15,d4		; Do 16 ASCII chars from the same place.
dascilp	move.b (a0)+,d0
	and #$7F,d0		; Mask off high bit.
	cmp #$7f,d0		; Don't print deletes or
	beq isnasci		; control chars.
	cmp #$20,d0
	bge isascii
isnasci	move.b #'.',d0
isascii	bsr konout
	cmp #8,d4		; If in the middle
	bne isntmid
	move.b #' ',d0		; Then break it with a space.
	bsr konout
isntmid	dbra d4,dascilp
	bsr konstat
	bne ddone
	cmpa.l a1,a2		; Are we done dumping?
	bhi ddone
	bra dlnlop		; else do another line.
ddone	movea.l a2,a3		; Save for non-specified dump.
	bra prompt

dumperr	lea.l dumpmsg,a1	; Error message if end > start.
	bsr pstr
	bra prompt


	.globl go
	.xdef lioinit,pioinit
go	bsr atol			; Go command, get address & flag.
	move.l d1,a0			; Ready to go.
	cmp.b #$D,d0			; Make sure ended addr with CR.
	bne prompt
	tst d2
	beq prompt			; No address given, quit.
	btst #0,d1			; Check for odd address.
	bne modderr
	move.b d7,oldrd7(a4)		; Save D7 for later.
	bsr lioinit			; Set up I/O structures
	bsr pioinit			; in case prog uses them.
	bsr crlf
	bsr paddr			; Print address
	bsr crlf
	pea.l prompt			; A return address.
	jmp (a0)			; and do it!

	.xdef oddmsg
modderr	lea.l oddmsg,a1			; Error message if Odd address.
	bsr pstr
	bra prompt

	.globl exceptn
	.xdef rom,ilins
exceptn	move #$2700,sr			; No interrupts allowed.
	move.l a0,0			; Do a total save on Exception.
	movea.l 4,a0			; Point into Data block.
	cmp.b #'B',(a0)			; Check for real Data area.
	bne rom				; Cold start if no area or
	cmp.b #'I',1(a0)		; if fence is broken.
	bne rom
	cmp.b #'O',2(a0)
	bne rom
	cmp.b #'S',3(a0)
	bne rom
	lea.l regsav(a0),a0
	movem.l d0-d7/a0-a7,-(a0)	; Save interrupted general regs.
	move usp,a1
	move.l a1,-(a0)
	movea.l 4,a4
	move.l 0,rega0(a4)		; Fix A0 in saved regs.
	movea.l dataend(a4),a7		; Get a real stack now.

	lea.l regsav2(a4),a0
	movea.l rega7(a4),a1		; Grab exception frame.
	move.w 6(a1),-(a0)		; Save old Format/Offset word.
	move.l 2(a1),-(a0)		; Save interrupted PC.
	move.w 0(a1),-(a0)		; Save interrupted SR.
	lea.l xsize(a1),a1		; Fix A7 to before EXCPTN pushed stuff.
	move.l a1,rega7(a4)
	move.w ilins,0			; Repair illegal inst at 0.

*	Setup remainder of the normal registers.
	move.b oldrd7(a4),d7		; Pick up D7 flags.
	move.l regpc(a4),a3		; Default address is saved PC.
	move.b #-1,contflg(a4)		; Assume prog is continuable.
	
* Reset console device here.

*	Test top of old stack at PC to make sure it
*	was in RAM before printing PC address.

	lea.l intmsg,a1			; First print 'Interrupted' message.
	bsr pstr

	move.w regfmt(a4),d1		; Which exception was it?
	andi.w #$fff,d1			; Get Vector offset.
	cmp.w #$7C,d1			; NMI?
	beq abort
	cmp.w #$24,d1			; TRACE?
	beq trace
	bsr pstr			; Not either, use vector number.
	bsr pbyte
	bra dmpregs

trace	lea.l trcmsg,a1			; Acknowledge the TRACE's receipt.
	bsr pstr
	bra dmpregs

abort	lea.l nmimsg,a1			; Acknowledge the NMI's receipt.
	bsr pstr
dmpregs	lea.l int2msg,a1
	bsr pstr
	move.l rega7(a4),a0
	move.l -xsize-2(a0),d0		; Alignment will ALWAYS be good.
	move.l d0,d1			; Test PC loc in stack for RAM.
	not.l d1			; Print it if in RAM, else we
	move.l d1,-xsize-2(a0)		; are most likely lost.
	cmp.l -xsize-2(a0),d1
	beq goodpc
	move.b #0,contflg(a4)		; Not continuable then.
	lea.l nopcmsg,a1		; Bad stack, address is unknown.
	bsr pstr
	lea.l sspmsg,a1
	bra showssp

goodpc	move.l d0,-xsize-2(a0)		; Fix old PC in 'stack'.
	bsr pstr			; Display interrupted PC.
	move.l regpc(a4),d1
	bsr plong
showssp	bsr pstr			; Note, assumes A1 points to next
	move.l rega7(a4),d1		; message in line!
	bsr plong
	bsr pstr
	move.l regusp(a4),d1
	bsr plong
	bsr pstr

	tst.b contflg(a4)		; If PC was bad, so will be SR.
	beq badsr
	move.w -xsize(a0),d0		; Test SR.
	move.w d0,d1
	not.w d1
	move.w d1,-xsize(a0)
	cmp.w -xsize(a0),d1
	beq goodsr
badsr	move.b #0,contflg(a4)		; Not continuable then.
	lea.l nopcmsg,a1		; Bad stack, status is unknown.
	bsr pstr
	bra nocry

goodsr	move.w d0,-xsize(a0)		; Fix old SR in 'stack'.
	bsr pstr
	move.w regsr(a4),d1
	bsr pword
	bsr pstr

	move.w regsr(a4),d2		; ASCII flag messages here.
	btst #15,d2
	beq notrc
	lea.l trcmsg,a1
	bsr pstr
	bsr pstr
notrc	btst #13,d2
	beq nosup
	lea.l supmsg,a1
	bsr pstr
nosup	lea.l imskmsg,a1
	bsr pstr
	move.w d2,d1
	lsr #8,d1
	and.w #7,d1
	bsr ntoa
	bsr pspace
	btst #4,d2
	beq nox
	lea.l xmsg,a1
	bsr pstr
nox	btst #3,d2
	beq noneg
	lea.l negmsg,a1
	bsr pstr
noneg	btst #2,d2
	beq nozer
	lea.l zermsg,a1
	bsr pstr
nozer	btst #1,d2
	beq noovfl
	lea.l ovflmsg,a1
	bsr pstr
noovfl	btst #0,d2
	beq nocry
	lea.l crymsg,a1
	bsr pstr

nocry	lea.l regmsg,a1
	bsr pstr			; Dump 8 data registers.
	lea.l regd0(a4),a0
	move.w #7,d2
regdlop	move.l (a0)+,d1
	bsr plongsp
	cmp.w #4,d2
	bne nodspc
	bsr pspace
nodspc	dbra d2,regdlop

	bsr pstr			; Dump 8 address registers.
	move.w #7,d2
regalop	move.l (a0)+,d1
	bsr plongsp
	cmp.w #4,d2
	bne noaspc
	bsr pspace
noaspc	dbra d2,regalop
	bsr pstr
	bra prompt			; and goto prompt.

	.globl cont
cont	tst.b contflg(a4)		; Continuation allowed?
	bne docont
	lea.l contmsg,a1
	bsr pstr
	bra prompt
docont	bclr #7,regsr(a4)		; Clear TRACE in saved regs.
restore	move.b d7,oldrd7(a4)		; Save D7 flags for re-entry.
	bsr crlf
	move.l rega7(a4),a7		; Build exception frame.
	move.w regfmt(a4),-(sp)		; Short exception word.
	move.l regpc(a4),-(sp)		; PC.
	move.w regsr(a4),-(sp)		; SR.
	move.l a7,0			; Save SP at 0.
	lea.l regusp(a4),a7		; Restore all other registers.
	move.l (sp)+,a0
	move a0,usp
	movem.l (sp)+,d0-d7/a0-a6
	move.l 0,a7			; Get proper SSP back.
	move.w ilins,0			; Fix illegal at 0.
	rte

	.globl dotrace
dotrace	tst.b contflg(a4)		; Tracing allowed?
	beq notrace
	bset #7,regsr(a4)		; Set TRACE in saved regs.
	move.l #exceptn,$24		; Reset trace vector.
	bra restore

notrace	lea.l tracmsg,a1
	bsr pstr
	bra prompt

*****************************************************************
*								*
*	Miscellaneous Support routines				*
*								*
*****************************************************************

pstr	move.b (a1)+,d0		; Print string -> A1. Stop on NULL
	beq pstr2
	bsr konout
	bra pstr
pstr2	rts

crlf	move.b #10,d0
	bsr konout
	move.b #13,d0
	bsr konout
crlf2	rts

konout	btst #1,d7
	bnz mkonout
	move.b #7,iobase+$17	; Select User 7 on INTERFACER IV
konout1	btst #0,iobase+$11
	beq konout1
	move.b d0,iobase+$10
konret	rts
mkonout	btst #0,iobase+$f1
	bnz mkonout
	move.b d0,iobase+$f0
	rts

konin	move.b #7,iobase+$17	; Select User 7 on INTERFACER IV
konin1	btst #1,iobase+$11
	beq konin1
	move.b iobase+$10,d0
	rts

konstat	move.b #7,iobase+$17	; Select User 7 on INTERFACER IV
	btst #1,iobase+$11	; returns Z if nothing there.
	rts

pbytesp	bsr pbyte		; Same as below but with trailing space.
	bra pspace
pwordsp	bsr pword
	bra pspace
paddrsp	bsr paddr
	bra pspace
plongsp	bsr plong
pspace	move.b #' ',d0
	bra konout

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
	bsr ntoa
pdigent	swap d0			; get nybble count.
	dbra d0,pdiglop
	rts

ntoa	move.b d1,d0		; nybble in d0 to ASCII, then output
	and #$f,d0
	cmp #$a,d0
	blt htoa2
	add.b #'A'-'9'-1,d0
htoa2	add.b #'0',d0
	bra konout

atol	clr.l d1		; ASCII to long, stops on invalid hex char.
	clr d2			; Returns long in d1, terminator char in d0,
atol1	bsr konin		; d2=1 if any chars entered before terminator.
	cmp.b #$40,d0
	blo atol2
	and #$5F,d0		; Mask to upper case.
atol2	cmpi.b #'0',d0		; Check range (0..9,A..F).
	blo atolend
	cmpi.b #'F',d0
	bhi atolend
	cmpi.b #'9',d0
	bls atol3
	cmpi.b #'A',d0
	bhs atol3
	bra atolend
atol3	moveq #1,d2		; Valid characters entered, flag it.
	bsr konout		; Echo the valid char.
	sub.b #'0',d0
	cmp.b #$9,d0
	bls atol4
	sub.b #'A'-'9'-1,d0
atol4	ext d0			; to long.
	ext.l d0
	asl.l #4,d1		; tack it onto d1.
	add.l d0,d1
	bra atol1
atolend	rts

	.data
dumpmsg	.dc.b 10,13,'  Ending address > starting address.',0
contmsg .dc.b 10,13,'Cannot continue.',0
tracmsg	.dc.b 10,13,'Cannot trace.',0

intmsg	.dc.b 10,13,'Program Interrupted (',0
	.dc.b '$',0
nmimsg	.dc.b 'NMI',0
int2msg	.dc.b ')  -----  PC = ',0
	.dc.b '$',0
sspmsg	.dc.b 10,13,'SSP = $',0
	.dc.b '  USP = $',0
	.dc.b '  SR = ',0
	.dc.b '$',0
	.dc.b ': ',0
nopcmsg	.dc.b 'UNKNOWN',0

trcmsg	.dc.b 'TRACE',0
	.dc.b ' ',0
supmsg	.dc.b 'SUP ',0
imskmsg	.dc.b 'IMASK=',0
xmsg	.dc.b 'X ',0
negmsg	.dc.b 'NEG ',0
zermsg	.dc.b 'ZERO ',0
ovflmsg	.dc.b 'OVR ',0
crymsg	.dc.b 'CARRY ',0
regmsg	.dc.b 10,13,'D0-7 ',0
	.dc.b 10,13,'A0-7 ',0
	.dc.b 10,13,0

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

	end
