	.title	at.sml	-   assembler/translator system macros

	.ident	/10may4/

	.macro	always		;all files of macro

mk.symbol=1			;one to make symbols, 0 otherwise
;x40=	0
pdpv45=	0
$timdf=	8
;xfltg=	0		;define to assmbl out floating hardware

;ft.id=	0			;define for d-space only
;idtype=0			;define for harv type i+d


.if ndf ft.unx
	ft.unx=	1		;default is for unix
.endc

.if gt ft.unx
	.if ndf ft.id		;if this is for unix, default is for i&d
		ft.id=	0
	.endc
	.if ndf idtype		;default to bell
		idtype = 1
	.endc
.endc

	.nlist	bex



tab=	11
lf=	12
vt=	13
ff=	14
cr=	15
space=	40

bpmb	=	20		;bytes per macro block


	.mcall	(at)defsec,entsec,xitsec,call  ,return
	.mcall	(at)setovr,entovr,xitovr,jmpovr
*** note:  the following line should be activated (remove ;) for non-40/45's***
;	.mcall	(at)mul,div,sob

	defsec		;invoke the default psects

	.macro	always
	.nlist	bex
	.endm	always

	.endm	always
	.macro	entsec	name	;init a section
	.psect	name	con
	.endm

	.macro	xitsec
	.psect	main
	.endm

	.macro	defsec		;set up initial psects
	.psect	main	con, shr
	.psect	xctprg	con, shr
	.psect	xctpas	con, shr
	.psect	xctlin	con, shr
	.psect	xctlix	con, shr
	.psect	dpure	con, shr
	.psect	mixed	con
	.psect	impure	con, bss
	.psect	imppas	con, bss
	.psect	implin	con, bss
	.psect	swtsec	con, shr
	.endm

	.macro	call	address
	jsr	%7,address
	.endm

	.macro	return
	rts	%7
	.endm


	.macro	setovr	name
	.endm
	.macro	entovr	name
	.endm

	.macro	xitovr	inline
	.endm

	.macro	jmpovr	adrpnt
	jmp	@adrpnt
	.endm


	.macro	param	mne,	value	;define default parameters
	.iif ndf mne,	mne=	value
	.list
mne=	mne
	.nlist
	.endm
	.macro	putkb	addr	;list to kb
	mov	addr,r0
	call	putkb
	.endm

	.macro	putlp	addr	;list to lp
	mov	addr,r0
	call	putlp
	.endm

	.macro	putkbl	addr	;list to kb and lp
	mov	addr,r0
	call	putkbl
	.endm

	.macro	putlin	addr	;use listing flags
	.if dif	<addr><r0>
	mov	addr,r0
	.endc
	call	putlin
	.endm

	.macro	xmit	wrdcnt	;move small # of words
	.globl	xmit0
	call	xmit0-<wrdcnt*2>
	.endm	xmit


;the macro "genswt" is used to specify  a command
;string switch (1st argument) and the address of
;the routine to be called when encountered (2nd arg).
; the switch is made upper-case.

	.macro	genswt	mne,addr,?label
	entsec	swtsec
label:	.irpc	x,mne
	.if ge ''x-141
		.if le ''x-172
			.byte ''x-40
		.iff
			.byte ''x
		.endc
	.iff
	.byte ''x
	.endc
	.endr
	.iif ne <.-label&1>,	.byte	0
	.word	addr
	xitsec
	.endm
	.macro	zread	chan
	.globl	zread
	mov	#chan'chn,r0
	call	zread
	.endm	zread

	.macro	zwrite	chan
	.globl	zwrite
	mov	#chan'chn,r0
	call	zwrite
	.endm	zwrite
	.macro	genedt	mne,subr	;gen enable/disable table
	entsec	edtsec
	.rad50	/mne/
	.if nb	subr
	.word	subr
	.iff
	.word	cpopj
	.endc
	.word	ed.'mne
	xitsec
	.endm	genedt


;the macro "gencnd" is used to specify conditional
;arguments.  it takes two or three arguments:

;	1-	mnemonic
;	2-	subroutine to be called
;	3-	if non-blank, complement condition

	.macro	gencnd	mne,subr,toggle	;generate conditional
	entsec	cndsec
	.rad50	/mne/
	.if b	<toggle>
	.word	subr
	.iff
	.word	subr+1
	.endc
	xitsec
	.endm
	.macro	ch.mne

ch.ior=	'!
ch.qtm=	'"
ch.hsh=	'#
ch.dol=	'$
ch.pct=	'%
ch.and=	'&
ch.xcl=	''

ch.lp=	'(
ch.rp=	')
ch.mul=	'*
ch.add=	'+
ch.com=	',
ch.sub=	'-
ch.dot=	'.
ch.div=	'/

ch.col=	':
ch.smc=	';
ch.lab=	'<
ch.equ=	'=
ch.rab=	'>
ch.qm=	'?

ch.ind=	'@
ch.bsl=	'\
ch.uar=	'^

let.a=	'a&^c40
let.b=	'b&^c40
let.c=	'c&^c40
let.d=	'd&^c40
let.e=	'e&^c40
let.f=	'f&^c40
let.g=	'g&^c40
let.o=	'o&^c40
let.p=	'p&^c40
let.z=	'z&^c40

dig.0=	'0
dig.9=	'9
	.macro	ch.mne
	.endm	ch.mne
	.endm	ch.mne
.if ndf pdpv45
	.macro	mul	src,dst
	.iif dif <src>,<r3>,	mov	src,r3
	.iif dif <dst>,<r0>,	.error	;illegal mul args
	.globl	mul
	call	mul
	.endm

	.macro	div	src,dst
	.iif dif <src>,<r3>,	mov	src,r3
	.iif dif <dst>,<r0>,	.error	;illegal div args
	.globl	div
	call	div
	.endm

	.macro	sob	reg,addr
	dec	reg
	bne	addr
	.endm

.endc

	.macro	error	arg
	.globl	err.'arg,	proerr
	jsr	r5,proerr
	 .word	err.'arg
	.endm	error

	.macro	setnz	addr	;set addr to non-zero for t/f flags
	mov	sp,addr
	.endm
				;roll handler calls

	.macro	search	rolnum	;binary search
	mov	#rolnum,r0
	.globl	search
	call	search
	.endm

	.macro	scan	rolnum	;linear scan
	mov	#rolnum,r0
	.globl	scan
	call	scan
	.endm

	.macro	scanw	rolnum	;linear scan, one word
	mov	#rolnum,r0
	.globl	scanw
	call	scanw
	.endm

	.macro	next	rolnum	;fetch next entry
	mov	#rolnum,r0
	.globl	next
	call	next
	.endm

	.macro	append	rolnum	;append to end of roll
	mov	#rolnum,r0
	.globl	append
	call	append
	.endm

	.macro	zap	rolnum	;clear roll
	mov	#rolnum,r0
	.globl	zap
	call	zap
	.endm

;	call	insert		;insert (must be preceded by one 
				;of the above to set pointers)
;	call	setrol		;save and set regs for above
;flags used in symbol table mode

	.macro	st.flg

.if le ft.unx

ovrflg=	000004		;overlay (psect only)
defflg=	000010		;defined
relflg=	000040		;relocatable
glbflg=	000100		;global

.endc

.if gt ft.unx

			; ****** these should not be changed!! ******
shrflg=	000001		;shareable (psect only)
.if gt ft.id
insflg=	shrflg*2	;instruction space (psect only)
bssflg=	insflg*2	;blank section (psect only)
m.idf=	shrflg!insflg!bssflg	;mask to turn them off
.iff
bssflg=	shrflg*2
m.idf=	shrflg!bssflg
.endc
b.idf=	1		;shift count to make above bits word offset
			; ***********************************
defflg=	000010		;defined
ovrflg=	000020		;overlay (psect only)
relflg=	000040		;relocatable
glbflg=	000100		;global

.endc

regflg=	000001		;register
lblflg=	000002		;label
mdfflg=	000004		;multilpy defined
	.macro	st.flg
	.endm
	.endm	st.flg



	.macro	ct.mne
	.globl	cttbl
ct.eol	=	000		; eol
ct.com	=	001		; comma
ct.tab	=	002		; tab
ct.sp	=	004		; space
ct.pcx	=	010		; printing character
ct.num	=	020		; numeric
ct.alp	=	040		; alpha, dot, dollar
ct.lc	=	100		; lower case alpha
ct.smc	=	200		; semi-colon (sign bit)

ct.pc	=	ct.com!ct.smc!ct.pcx!ct.num!ct.alp
	.macro	ct.mne
	.endm
	.endm	ct.mne


	.end
