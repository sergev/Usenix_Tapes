/------------------------------------------------------------------------------
/	This is an interpreter for EM1 programs with no virtual memory
/
/	Author:	Hans van Staveren
/		Vrije Universiteit
/		Amsterdam
/
/	Memory layout:
/
/	|text|data|      bss      |  unused  |          stack             |
/	___________________________________________________________________
/	|            |       |    |          | |    |     |            |  |
/	|            |       |    |          | |    |     |            |  |
/	|      1     |   2   |  3 |          |4|  5 |  6  |      7     | 8|
/	|            |       |    |          | |    |     |            |  |
/	|____________|_______|____|__________|_|____|_____|____________|__|
/
/	1:	Interpreter code+data
/	2:	EM1 hole + const
/	3:	EM1 stack
/	4:	interpreter stack
/	5:	EM1 heap
/	6:	line-profile table
/	7:	EM1 code
/	8:	Arguments to interpreter
/
/	Assembly time flags:
/	.debug:	controls self checking and trace-printout
/	.test : controls checking for undefined variables,nil pointers,
/		array indices, etc....
/	.prof :	controls generation of a runtime profile
/	.opfreq:	controls runtime frequency count per opcode
/	.flow:	controls generation of a flow bitmap
/	.count:	controls generation of a flow count
/
/	Register layout:
/	spx = EM1 stackpointer
/	pcx = EM1 programcounter
/	lb  = EM1 base-address
/	next= address of start of interpreter loop
/
/	The general structure of this interpreter is as follows:
/	The opcode byte of the instruction is placed in r0
/	with sign-extension and multiplied by 2.
/	If .opfreq is nonzero each occurence of each opcode is counted.
/	Then, if .prof is nonzero an estimation of the time required
/	to execute the instruction is added to a counter associated
/	with the source-line number. This estimation is roughly the
/	number of memory-cycles needed. At the end of this accounting
/	r1 points to the loword of the double precision counter.
/	This can be used by individual execution routines to add some
/	more to the counter depending on their operand.
/
/------------------------------------------------------------------------------
/
/------------------------------------------------------------------------------
/	Declaring of some constants
/------------------------------------------------------------------------------

	.debug	=0
	.opfreq	=0

	spx	=r5
	next	=r4
	pcx	=r3
	lb	=r2

	statd	=-8.

	memincr	=400		/additional memory asked on stack overflow
	und	=100000		/undefined memory pattern
	common	=134.		/common opcodes for different contexts
	.bigcomm=  1		/set to 1 if common>127
	signext =177400		/high bits for signextension

/------------------------------------------------------------------------------
/	constants fm_xxx, for xxx some EM1-mnemonic, give the base of
/	the minis for that instruction. If the base is >= 128 the
/	constant signext must be added.
/	n.b. fm stands for "first mini"
/------------------------------------------------------------------------------

	fm_and	=	228.	signext
	fm_beg	=	251.	signext
	fm_dup	=	179.	signext
	fm_ine	=	185.	signext
	fm_inl	=	188.	signext
	fm_ior	=	157.	signext
	fm_lae	=	100.
	fm_lal	=	92.
	fm_ldl	=	148.	signext
	fm_lex	=	140.	signext
	fm_lnc	=	36.
	fm_loc	=	1.
	fm_loe	=	56.
	fm_lof	=	80.
	fm_loi	=	123.
	fm_lol	=	39.
	fm_lop	=	162.	signext
	fm_mrk	=	244.	signext
	fm_mrx	=	248.	signext
	fm_ret	=	199.	signext
	fm_stf	=	209.	signext
	fm_sti	=	214.	signext
	fm_stl	=	217.	signext
/
/------------------------------------------------------------------------------
/	The constants ac_xxx, for xxx some EM1-mnemonic, give the extra
/	number of memory cycles needed for an increase of the parameter
/	of the instruction by one.
/	Usage:	jsr	r5,account;ac_xxx
/------------------------------------------------------------------------------

	ac_loi	=	4
	ac_sti	=	4
	ac_cmu	=	5
	ac_and	=	6
	ac_ior	=	4
	ac_xor	=	5
	ac_com	=	3
	ac_dup	=	4
	ac_bug	=	3
	ac_blm	=	4
	ac_mrk	=	4
	ac_beg	=	3
	ac_ret	=	4
	ac_lex	=	4
	ac_set	=	3

/------------------------------------------------------------------------------
/	Declaring of some instructions unknown to the assembler
/------------------------------------------------------------------------------

	rti	=	2	/ return from interrupt
	stst	=	170300^tst	/ store floating point status
	indir	=	0	/for sys indir
	sleep	=	35.	/for sys sleep

/------------------------------------------------------------------------------
/	External references
/------------------------------------------------------------------------------

	.globl	mesg,_end

/------------------------------------------------------------------------------
/
/------------------------------------------------------------------------------
/	Now the real program starts
/------------------------------------------------------------------------------

startoff:
	mov	$_end,eb	/ set up eb
	mov	(sp)+,argc	/ pass to userprogram later
	mov	sp,argv		/ pass to userprogram later
	mov	$3,r0		/ file descriptor of load file
	mov	r0,r5		/ save filedescriptor
	sys	read;header;16.	/ read header
	jes	badarg		/ failed
	cmp	r0,$16.		/ long enough ?
	jne	badarg		/ no.
	inc	txtsiz		/ make txtsiz even
	bic	$1,txtsiz	/ done now
	sub	txtsiz,sp	/ reserve space for code
	tst	(sp)		/ notify system
	mov	sp,pb		/ set up procedure base
	mov	txtsiz,1f+4	/ set up for read
	mov	sp,1f+2		/ almost done
	mov	r5,r0		/ done !
	sys	indir;1f	/ this is it folks
	.data
1:	sys	read;0;0	/ read call
	.text
	jes	badarg		/ read failed
	cmp	r0,txtsiz	/ all code available ?
	jne	badarg		/ no
	sub	procsiz,sp	/ reserve space for proc table
	tst	(sp)		/ let the system know
	inc	datsiz		/ make evn
	bic	$1,datsiz	/ finish that
	inc	holsiz		/ same for holsiz
	bic	$1,holsiz	/ also finished
	add	holsiz,sybreak+2/ get core for hol and data
	mov	sybreak+2,1b+2	/ address of initialised data
	add	datsiz,sybreak+2/ this is the total
	add	$memincr,sybreak+2 / a bit extra for the stack
	sys	indir;sybreak	/ Ask for the core
	jes	toolarge	/ Too much, sorry
	mov	holsiz,r0;	/ clear hol
	beq	4f		/ no hol
	asr	r0		/ # of words
	mov	$_end,r1	/ start of hol
9:	mov	$und,(r1)+	/ set up hol
	sob	r0,9b		/ as long as there is some.
	clr	*eb		/ clear line number
4:	mov	datsiz,1b+4	/ setting up to read data
	mov	r5,r0		/ ready for it
	sys	indir;1b	/ do it now
	bes	badarg		/ no comment
	cmp	r0,datsiz	/ ditto
	bne	badarg		/ lots of things can go wrong
	mov	sp,1b+2		/ setting up to read procs
	mov	procsiz,1b+4	/ count
	mov	r5,r0		/ filedescriptor
	sys	indir;1b	/ do it now.
	bes	badarg
	cmp	r0,procsiz
	bne	badarg		/ no comment
	mov	sp,pd		/ set up procedure descriptor register
	asr	r0		/ divide by 2
	asr	r0		/ again
	mov	sp,r1		/ relocate offset's to absolute offsets
9:	tst	(r1)+		/ skip parameter field
	add	pb,(r1)+	/ relocate
	sob	r0,9b		/ as long as there are procs
	mov	relsiz,r2	/ do datarelocation
	beq	3f		/ no relocation
	asr	r2		/ words at a time
	mov	$2,1b+4		/ read one word at a time
	mov	$2f,1b+2	/ put at next data word
9:	mov	r5,r0
	sys	indir;1b	/ read word
	bes	badarg
	mov	2f,r1		/ this is index in eb[]
	add	$_end,_end(r1)	/ change offset in address
	sob	r2,9b
	.bss
2:	.=.+2
	.text
3:	mov	r5,r0		/ end of file
	sys	close		/ close file

.if .prof
	mov	nlines,r0
	inc	r0
	asl	r0
	asl	r0
	sub	r0,sp
	tst	(sp)
	mov	r0,profsiz
	mov	sp,ltime	/ set up pointer
.endif
.if .flow
	mov	nlines,r0
	ash	$-3,r0		/ divide by 8
	add	$2,r0
	bic	$1,r0		/ Rounded up to an integral number of words
	sub	r0,sp
	tst	(sp)
	mov	r0,flowsiz
	mov	sp,lflow
.endif
.if .count
	mov	nlines,r0
	inc	r0
	ash	$2,r0		/ multiply by 4
	sub	r0,sp
	tst	(sp)
	mov	r0,countsiz
	mov	sp,lcount
.endif

	mov	sybreak+2,spx	/ set up spx
	sub	$memincr,spx	/ ok now
	mov	spx,initstck	/ for testing
	mov	$-1,(spx)+	/ this link will trap
	mov	$-1,(spx)+	/ ditto
	clr	(spx)+		/ space for return address(ho ho)
	clr	(spx)+		/ dummy line number
	mov	argc,(spx)+	/ argument for .main
	mov	argv,(spx)+	/ next argument
	mov	sp,hp		/ set up heap-pointer
	mov	sp,ml		/ set up memory limit
	dec	ml		/ end of it
	sys	signal;11.;segviol	/ catch segmentation violation
.if	.softfp
	.globl	fptrap
	sys	signal;4;fptrap
.endif
.if	.float
	ldfps	$7400
	sys	signal;8.;fppflt	/ catch floating exception
.endif
	mov	entry.,r0	/ procnumber of .main
	jbr	cal

badarg:	mov	$1f,r0;	jsr	pc,fatal
toolarge:mov	$2f,r0;	jsr	pc,fatal
	.data
1:	.byte 86.;	<load file error\0>
2:	.byte	87.;	<program too large\0>
	.even
	.text
/------------------------------------------------------------------------------
/
/------------------------------------------------------------------------------
/	Main loop of interpreter starts here
/------------------------------------------------------------------------------

loop:
.if .debug
	cmp	trace,$3;blt 1f
	mov	-6(spx),-(sp);	mov	-4(spx),-(sp);	mov	-2(spx),-(sp)
	clr	-(sp);	bisb	(pcx),(sp)
	mov	pcx,-(sp)
	mov	$0f,-(sp);jsr pc,printf
	add	$12.,sp
	.data
0:	<At %5l instruction %3d\t>
	<Stack:\t%6o\t%6o\t%6o\n\0>
	.even
	.text
1:
	cmp	spx,initstck		/ stack underflow ?
	bhis	1f			/ no
	jsr	r5,mesg			/ yes
		<\nStack underflow - Core dump will be forced\n\0>;.even
	tst	1			/ force core dump
1:
	mov	pcx,r0			/ is program counter allright ?
	sub	pb,r0
	cmp	r0,txtsiz
	blo	1f			/ ok
	jsr	r5,mesg			/ bad program counter
		<\nProgram counter out of range - Core dump will be forced\n\0>
		.even
	tst	1			/ force core dump
1:
.endif					/ end of .if .debug

/ Now up to the real work

	movb	(pcx)+,r0		/ pickup opcode + sign extend
9:	asl	r0			/ opcode now -256 .. 254 & even

.if	.opfreq
	mov	r0,r1;	asl	r1	/ multiply by two again
	add	$1,counttab+512.(r1)	/ cannot be inc
	adc	counttab+514.(r1)	/ double precision counters
.endif
.if	.prof
	mov	*eb,r1			/ line number in r1
	asl	r1;	asl	r1	/ multiply by 4
	add	ltime,r1		/ make it point into the table
	add	timeinf(r0),2(r1)
	adc	(r1)			/ double precision
.endif
	jmp	*dispat(r0)		/ goto execution routine

/ two byte opcodes come here for decoding of second byte

escape:
.if .debug
	cmp	trace,$3;blt 1f
	mov	-6(spx),-(sp);	mov	-4(spx),-(sp);	mov	-2(spx),-(sp)
	clr	-(sp);	bisb	(pcx),(sp)
	mov	pcx,-(sp)
	mov	$0f,-(sp);jsr pc,printf
	add	$12.,sp
	.data
0:	<At %5l escaped ins %3d\t>
	<Stack:\t%6o\t%6o\t%6o\n\0>
	.even
	.text
1:
	cmp	spx,initstck		/ stack underflow ?
	bhis	1f			/ no
	jsr	r5,mesg			/ yes
		<\nStack underflow - Core dump will be forced\n\0>;.even
	tst	1			/ force core dump
1:
	mov	pcx,r0			/ is program counter allright ?
	sub	pb,r0
	cmp	r0,txtsiz
	blo	1f			/ ok
	jsr	r5,mesg			/ bad program counter
		<\nProgram counter out of range - Core dump will be forced\n\0>
		.even
	tst	1			/ force core dump
1:
.endif					/ end of .if .debug
	movb	(pcx)+,r0		/ fetch second byte and sign extend
	asl	r0			/ -256.. 254 & even

.if	.opfreq
	mov	r0,r1;	asl	r1	/ multiply by two again
	add	$1,counttab+1536.(r1)	/ cannot be inc
	adc	counttab+1538.(r1)	/ double precision counters
.endif
.if	.prof
	mov	*eb,r1			/ line number in r1
	asl	r1;	asl	r1	/ multiply by 4
	add	ltime,r1		/ make it point into the table
	add	time2inf(r0),2(r1)
	adc	(r1)			/ double precision
.endif
	jmp	*disp2(r0)

/alternate decoding context comes here

loop1:

.if .debug
	cmp	trace,$3;blt 1f
	mov	-6(spx),-(sp);	mov	-4(spx),-(sp);	mov	-2(spx),-(sp)
	clr	-(sp);	bisb	(pcx),(sp)
	mov	pcx,-(sp)
	mov	$0f,-(sp);jsr pc,printf
	add	$12.,sp
	.data
0:	<At %5l INSTRUCTION %3d\t>
	<Stack:\t%6o\t%6o\t%6o\n\0>
	.even
	.text
1:
	cmp	spx,initstck		/ stack underflow ?
	bhis	1f			/ no
	jsr	r5,mesg			/ yes
		<\nStack underflow - Core dump will be forced\n\0>;.even
	tst	1			/ force core dump
1:
	mov	pcx,r0			/ is program counter allright ?
	sub	pb,r0
	cmp	r0,txtsiz
	blo	1f			/ ok
	jsr	r5,mesg			/ bad program counter
		<\nProgram counter out of range - Core dump will be forced\n\0>
		.even
	tst	1			/ force core dump
1:
.endif					/ end of .if .debug
.if .bigcomm-1
	clr	r0
	bisb	(pcx)+,r0		/ no sign extension now
	cmp	r0,$common		/ codes 0 to common -1 not special
	jlt	9b
	sub	$common,r0
.endif
.if .bigcomm
	movb	(pcx)+,r0		/ sign extend
	cmpb	r0,$common		/ codes 0 to common -1 not special
	jlo	9b
	sub	$[common+signext],r0
.endif
	jbr	cal

/------------------------------------------------------------------------------
illins:lsa.s0:lsa.l:
.if	1-.float
fad.z:	fsb.z:	fmu.z:	fdv.z:	cif.z:	cfi.z:	cdf.z:	cfd.z:	cmf.z:
.endif
.if	1-.ext
zzz.z:
.endif
/These instructions are not implemented(yet)
	mov	$1f,r0
	jsr	pc,fatal
	.data
1:	.byte 66.;	<illegal instruction\0>
	.even
	.text
/
/------------------------------------------------------------------------------
/	dispatch tables, first the unescaped opcodes
/
/	name convention is as follows:
/	each execution routine has as a name the name of the instruction
/	followed by a dot and a suffix.
/	suffix can be an integer, an 's' followed by an integer, an 'l'
/	or a 'z'.
/	loc.1		routine to execute loc 1
/	lae.s2		routine to execute lae 1024 thru lae 1535
/	loe.l		routine to execute all loe instructions
/	add.z		routine to execute instruction without operand
/------------------------------------------------------------------------------
	.data


loi.s0;	add.z;	sub.z;	mul.z;	div.z;	inc.z;	dec.z;	lar.s0
lar.s1;	lar.l;	aar.s0;	aar.s1;	aar.l;	lex.1;	lex.2;	rck.s0
rck.l;	lde.s0;	lde.s1;	lde.l;	ldl.0;	ldl.1;	ldl.2;	ldl.3
ldl.4;	ldl.5;	ldl.6;	ldl.s0;	adi.s0;	cif.z;	ior.1;	ior.2
ior.3;	ior.4;	lop.0;	lop.1;	lop.s0;	beq.s0;	bge.s0;	bgt.s0
ble.s0;	blt.s0;	bne.s0;	brb.s0;	brf.s0;	blm.s0;	cal.s0;	cfi.z
cmf.z;	cmi.z;	cse.s0;	cse.l;	dup.1;	fad.z;	fsb.z;	fmu.z
fdv.z;	ine.0;	ine.s0;	ine.l;	inl.0;	inl.1;	inl.2;	inl.3
inl.4;	inl.s0;	inn.s0;	lin.s0;	lin.l;	mod.z;	neg.z;	ret.0
ret.1;	sar.s0;	sar.s1;	sar.l;	sde.s0;	sdl.s0;	shl.z;	ste.s0
ste.s1;	ste.l;	stf.1;	stf.2;	stf.3;	stf.s0;	sti.0;	sti.1
sti.s0;	stl.0;	stl.1;	stl.2;	stl.3;	stl.4;	stl.5;	stl.6
stl.7;	stl.8;	stl.9;	stl.s0;	stp.s0;	and.1;	teq.z;	tge.z
tgt.z;	tle.z;	tlt.z;	tne.z;	zeq.s0;	zge.s0;	zgt.s0;	zle.s0
zlt.s0;	zne.s0;	zre.s0;	zrl.s0;	mrk.0;	mrk.1;	mrk.2;	mrk.3
mrx.0;	mrx.1;	mrx.2;	mrx.3;	beg.1;	beg.2;	beg.3;	beg.s0

dispat:			/ Dispatch table for unescaped opcodes

escape;	loc.0;	loc.1;	loc.2;	loc.3;	loc.4;	loc.5;	loc.6
loc.7;	loc.8;	loc.9;	loc.10;	loc.11;	loc.12;	loc.13;	loc.14
loc.15;	loc.16;	loc.17;	loc.18;	loc.19;	loc.20;	loc.21;	loc.22
loc.23;	loc.24;	loc.25;	loc.26;	loc.27;	loc.28;	loc.29;	loc.30
loc.31;	loc.32;	loc.s0;	loc.s1;	loc.l;	lnc.1;	lnc.s0;	lol.0
lol.1;	lol.2;	lol.3;	lol.4;	lol.5;	lol.6;	lol.7;	lol.8
lol.9;	lol.10;	lol.11;	lol.12;	lol.13;	lol.14;	lol.15;	lol.s0
loe.0;	loe.1;	loe.2;	loe.3;	loe.4;	loe.5;	loe.6;	loe.7
loe.8;	loe.9;	loe.10;	loe.11;	loe.12;	loe.13;	loe.14;	loe.15
loe.16;	loe.17;	loe.18;	loe.19;	loe.s0;	loe.s1;	loe.s2;	loe.s3
loe.l;	lof.1;	lof.2;	lof.3;	lof.4;	lof.5;	lof.6;	lof.7
lof.8;	lof.9;	lof.s0;	lof.l;	lal.0;	lal.1;	lal.2;	lal.3
lal.4;	lal.5;	lal.6;	lal.s0;	lae.0;	lae.1;	lae.2;	lae.3
lae.4;	lae.5;	lae.6;	lae.7;	lae.8;	lae.9;	lae.10;	lae.11
lae.12;	lae.s0;	lae.s1;	lae.s2;	lae.s3;	lae.s4;	lae.s5;	lae.s6
lae.s7;	lae.s8;	lae.l;	loi.0;	loi.1;	loi.2;	loi.3;	loi.4

/------------------------------------------------------------------------------
/
/------------------------------------------------------------------------------
/	now dispatch table for escaped opcodes
/------------------------------------------------------------------------------

illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins

disp2:			/ Dispatch table for escaped opcodes

hlt.z;	aas.z;	adi.l;	ads.z;	and.s0;	and.l;	ans.z;	beg.l
beq.l;	bge.l;	bgt.l;	ble.l;	blt.l;	bne.l;	brf.l;	brb.l
bes.z;	blm.l;	bls.z;	cal.l;	cas.z;	cdi.z;	cid.z;	cmd.z
cms.z;	cmu.s0;	cmu.l;	com.s0;	com.l;	cos.z;	dad.z;	dsb.z
dmu.z;	ddv.z;	dee.s0;	dee.l;	del.s0;	del.l;	dup.s0;	dup.l
dus.z;	exg.z;	inl.l;	inn.l;	ins.z;	ior.s0;	ior.l;	ios.z
lal.l;	las.z;	ldf.s0;	ldf.l;	ldl.l;	lex.s0;	lex.l;	loi.l
lol.l;	lop.l;	lor.s0;	los.z;	mon.z;	mrk.s0;	mrk.l;	mrx.s0
mrx.l;	mrs.z;	mxs.z;	ret.s0;	ret.l;	rol.z;	ror.z;	sas.z
sde.l;	sdf.s0;	sdf.l;	sdl.l;	ses.z;	set.s0;	set.l;	stf.l
sti.l;	stl.l;	str.s0;	sts.z;	swi.z;	swp.z;	xor.s0;	xor.l
xos.z;	nop.z;	not.z;	shr.z;	zre.l;	zrl.l;	zeq.l;	zge.l
zgt.l;	zle.l;	zlt.l;	zne.l;	cdf.z;	cfd.z;	dmd.z;	res.z
lsa.s0;	lsa.l;	lor.l;	str.l;	lnc.l;	stp.l;	zzz.z;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins
illins;	illins;	illins;	illins;	illins;	illins;	illins;	illins

/------------------------------------------------------------------------------

	.text

/
/------------------------------------------------------------------------------
/	timeinf tables, first the unescaped opcodes
/	these tables are parallel to the tables dispat and disp2
/	Each entry contains the # of memory-cycles needed to
/	execute that instruction.
/	The table timeinf also contains, added to each entry,
/	the number of memory-cycles needed to decode the instruction.
/	This number is currently 6
/------------------------------------------------------------------------------

.if	.prof
33.;	11.;	11.;	13.;	16.;	10.;	10.;	44.
46.;	50.;	37.;	40.;	44.;	11.;	14.;	21.
30.;	20.;	22.;	39.;	17.;	17.;	17.;	17.
17.;	17.;	17.;	18.;	14.;	16.;	11.;	29.
33.;	37.;	12.;	13.;	16.;	14.;	14.;	14.
14.;	14.;	14.;	11.;	12.;	18.;	26.;	20.
24.;	14.;	34.;	43.;	11.;	22.;	22.;	23.
23.;	11.;	16.;	25.;	11.;	11.;	11.;	11.
11.;	11.;	30.;	00.;	00.;	16.;	10.;	15.
17.;	45.;	47.;	51.;	21.;	19.;	15.;	17.
19.;	26.;	15.;	15.;	15.;	16.;	13.;	12.
14.;	12.;	12.;	12.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	15.;	16.;	13.;	12.;	12.
12.;	12.;	12.;	12.;	13.;	13.;	13.;	13.
13.;	13.;	16.;	14.;	13.;	16.;	19.;	22.
15.;	18.;	21.;	24.;	10.;	13.;	16.;	12.

timeinf:

12.;	 9.;	12.;	12.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	12.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	12.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	12.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	14.;	20.;	10.;	13.;	12.
12.;	12.;	12.;	12.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	12.;	12.;	12.;	12.;	15.
14.;	14.;	14.;	14.;	14.;	14.;	14.;	14.
14.;	14.;	14.;	14.;	14.;	14.;	14.;	14.
14.;	14.;	14.;	14.;	17.;	19.;	19.;	19.
26.;	15.;	15.;	15.;	15.;	15.;	15.;	15.
15.;	15.;	16.;	25.;	12.;	12.;	12.;	12.
12.;	12.;	12.;	14.;	15.;	15.;	15.;	15.
15.;	15.;	15.;	15.;	15.;	15.;	15.;	15.
15.;	16.;	18.;	18.;	18.;	18.;	18.;	18.
18.;	18.;	27.;	14.;	11.;	21.;	25.;	29.

/------------------------------------------------------------------------------
/
/------------------------------------------------------------------------------
/	time2inf table for escaped opcodes
/	cycles necessary for decoding is already accounted for in timeinf
/------------------------------------------------------------------------------

time2inf:

00.;	31.;	18.;	 5.;	14.;	23.;	17.;	15.
 9.;	 9.;	 9.;	 9.;	 9.;	 9.;	14.;	14.
 9.;	24.;	12.;	29.;	20.;	11.;	 6.;	15.
19.;	16.;	25.;	 7.;	16.;	10.;	12.;	12.
/ Next numbers have no meaning at all
/ If you are a (memory) cycle freak, here is your big chance
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
10.;	10.;	10.;	10.;	10.;	10.;	10.;	10.
.endif

/------------------------------------------------------------------------------
/
/------------------------------------------------------------------------------
/	Load constant, load local, store local
/------------------------------------------------------------------------------

loc.0:	clr	(spx)+;		jmp	(next)
loc.1:	loc.2:	loc.3:	loc.4:	loc.5:	loc.6:	loc.7:	loc.8:
loc.9:	loc.10:	loc.11:	loc.12:	loc.13:	loc.14:	loc.15:	loc.16:
loc.17:	loc.18:	loc.19:	loc.20:	loc.21:	loc.22:	loc.23:	loc.24:
loc.25:	loc.26:	loc.27:	loc.28:	loc.29:	loc.30:	loc.31:	loc.32:
/	all these share the same routine
	asr	r0	/ make multiplication undone
	sub	$fm_loc,r0
	mov	r0,(spx)+
	jmp	(next)
loc.s1:	mov	$400,r0;	br	2f
loc.s0:	clr	r0;	2:bisb	(pcx)+,r0
	mov	r0,(spx)+;	jmp	(next)
loc.l:	jsr	pc,wrdoff;	mov	r0,(spx)+;	jmp	(next)
lnc.1:	mov	$-1,(spx)+;	jmp	(next);
lnc.l:	jsr	pc,wrdoff;	br	1f
lnc.s0:	clr	r0;		bisb	(pcx)+,r0;
1:	neg	r0
	mov	r0,(spx)+;	jmp	(next)


lol.0:	lol.1:	lol.2:	lol.3:	lol.4:	lol.5:	lol.6:	lol.7:	
lol.8:	lol.9:	lol.10:	lol.11:	lol.12:	lol.13:	lol.14:	lol.15:
	add	lb,r0
	mov	-2*fm_lol(r0),(spx)+
	jmp	(next)
lol.l:	jsr	pc,wrdoff;	br	1f
lol.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
	mov	(r0),(spx)+
	jmp	(next)


stl.0:	stl.1:	stl.2:	stl.3:	stl.4:	stl.5:	stl.6:	stl.7:	stl.8:	stl.9:
	add	lb,r0;		mov	-(spx),-2*fm_stl(r0);	jmp	(next)
stl.l:	jsr	pc,wrdoff;	br	1f
stl.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
	mov	-(spx),(r0);	jmp	(next)

ldl.0:	ldl.1:	ldl.2:	ldl.3:	ldl.4:	ldl.5:	ldl.6:
	sub	$2*fm_ldl,r0
	br	3f
ldl.l:	jsr	pc,wrdoff;	br	1f
ldl.s0:	clr	r0;		bisb	(pcx)+,r0;
1:	asl	r0;		3:add	lb,r0
	mov	(r0)+,(spx)+
	mov	(r0),(spx)+
	jmp	(next)

sdl.l:	jsr	pc,wrdoff;	br	1f
sdl.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
	mov	-(spx),2(r0);	mov	-(spx),(r0);	jmp	(next)
/
/------------------------------------------------------------------------------
/	Load external,store external
/------------------------------------------------------------------------------

loe.0:	loe.1:	loe.2:	loe.3:	loe.4:	loe.5:	loe.6:	loe.7:	loe.8:	loe.9:
loe.10:	loe.11:	loe.12:	loe.13:	loe.14:	loe.15:	loe.16:	loe.17:	loe.18:	loe.19:
	add eb,r0
	mov	-2*fm_loe(r0),(spx)+
	jmp	(next)
loe.l:	jsr	pc,wrdoff;	br	1f
loe.s1:	mov	$400,r0;	br	2f
loe.s2:	mov	$1000,r0;	br	2f
loe.s3:	mov	$1400,r0;	br	2f
loe.s0:	clr	r0;		2:bisb	(pcx)+,r0
1:	asl	r0;	add eb,r0;	mov	(r0),(spx)+;
	jmp	(next)

ste.l:	jsr	pc,wrdoff;	br	1f
ste.s1:	mov	$400,r0;	br	2f
ste.s0:	clr	r0;		2:bisb	(pcx)+,r0
1:	asl	r0;	add eb,r0;	mov	-(spx),(r0)
	jmp	(next)

lde.l:	jsr	pc,wrdoff;	br	1f
lde.s1:	mov	$400,r0;	br	2f
lde.s0:	clr	r0;		2:bisb	(pcx)+,r0
1:	asl	r0;	add eb,r0;	mov	(r0)+,(spx)+
	mov	(r0),(spx)+
	jmp	(next)

sde.l:	jsr	pc,wrdoff;	br	1f
sde.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;	add eb,r0;	mov	-(spx),2(r0)
	mov	-(spx),(r0)
	jmp	(next)
/
/------------------------------------------------------------------------------
/	Clear local,clear external
/------------------------------------------------------------------------------

zrl.l:	jsr	pc,wrdoff;	br	1f
zrl.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0;
	clr	(r0);		jmp	(next)

zre.l:	jsr	pc,wrdoff;	br	1f
zre.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;	add eb,r0;	clr	(r0);	jmp	(next)
/
/------------------------------------------------------------------------------
/	increments and decrements
/------------------------------------------------------------------------------

inc.z:
.if	.test
	cmp	-2(spx),$und;	jeq	undef
.endif
	inc	-2(spx)
.if	.test
	jvs	ovf
.endif
	jmp	(next)
dec.z:
.if	.test
	cmp	-2(spx),$und;	jeq	undef
.endif
	dec	-2(spx)
.if	.test
	jvs	ovf
.endif
	jmp	(next)

inl.0:	inl.1:	inl.2:	inl.3:	inl.4:	inl.5:
	add	lb,r0
.if	.test
	cmp	-2*fm_inl(r0),$und;	jeq	undef
.endif
	inc	-2*fm_inl(r0)
.if	.test
	jvs	ovf
.endif
	jmp	(next)
inl.l:	jsr	pc,wrdoff;	br	1f
inl.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
.if	.test
	cmp	(r0),$und;	jeq	undef
.endif
	inc	(r0)
.if	.test
	jvs	ovf
.endif
	jmp	(next)

del.l:	jsr	pc,wrdoff;	br	1f
del.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
.if	.test
	cmp	(r0),$und;	jeq	undef
.endif
	dec	(r0)
.if	.test
	jvs	ovf
.endif
	jmp	(next)

ine.0:
.if	.test
	cmp	*eb,$und;	jeq	undef
	cmp	*eb,nlines;	jhis	linovf
.endif
	inc	*eb
.if	.test
	jvs	ovf
.endif
.if	.flow
	jsr	pc,seeflow
.endif
.if	.count
	jsr	pc,seecount
.endif
	jmp	(next)
ine.l:	jsr	pc,wrdoff;	br	1f
ine.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	eb,r0
.if	.test
	cmp	(r0),$und;	jeq	undef
.endif
	inc	(r0)
.if	.test
	jvs	ovf
.endif
	jmp	(next)

dee.l:	jsr	pc,wrdoff;	br	1f
dee.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	eb,r0
.if	.test
	cmp	(r0),$und;	jeq	undef
.endif
	dec	(r0)
.if	.test
	jvs	ovf
.endif
	jmp	(next)
/
/------------------------------------------------------------------------------
/	load-addresses, indirect, offsetted
/------------------------------------------------------------------------------

lal.0:	lal.1:	lal.2:	lal.3:	lal.4:	lal.5:	lal.6:
	sub	$2*fm_lal,r0
	add	lb,r0
	mov	r0,(spx)+
	jmp	(next)
lal.l:	jsr	pc,wrdoff;	br	1f
lal.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
	mov	r0,(spx)+;	jmp	(next)

lae.0:	lae.1:	lae.2:	lae.3:	lae.4:	lae.5:
lae.6:	lae.7:	lae.8:	lae.9:	lae.10:	lae.11:	lae.12:
	sub	$2*fm_lae,r0
	br	3f
lae.l:	jsr	pc,wrdoff;	br	1f
lae.s1:	mov	$400,r0;	br	2f
lae.s2:	mov	$1000,r0;	br	2f
lae.s3:	mov	$1400,r0;	br	2f
lae.s4:	mov	$2000,r0;	br	2f
lae.s5:	mov	$2400,r0;	br	2f
lae.s6:	mov	$3000,r0;	br	2f
lae.s7:	mov	$3400,r0;	br	2f
lae.s8:	mov	$4000,r0;	br	2f
lae.s0:	clr	r0;		2:bisb	(pcx)+,r0
1:	asl	r0;		3:add	eb,r0
.if	.test
	cmp	r0,initstck;	bhis	badlae
.endif
	mov	r0,(spx)+;	jmp	(next)
badlae:	mov	$1f,r0;	jsr	pc,fatal
	.data
1:	.byte 88.;	<load illegal address\0>
	.even
	.text

lop.0:
.if	.test
	mov	(lb),-(sp);	jsr	pc,chkptr
.endif
	mov	*(lb),(spx)+;	jmp	(next)
lop.1:
.if	.test
	mov	2(lb),-(sp);	jsr	pc,chkptr
.endif
	mov	*2(lb),(spx)+;	jmp	(next)
lop.l:	jsr	pc,wrdoff;	br	1f
lop.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
.if	.test
	mov	(r0),-(sp);	jsr	pc,chkptr
.endif
	mov	*(r0)+,(spx)+;	jmp	(next)

stp.l:	jsr	pc,wrdoff;	br	1f
stp.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	lb,r0
.if	.test
	mov	(r0),-(sp);	jsr	pc,chkptr
.endif
	mov	-(spx),*(r0)+;	jmp	(next)

lof.1:	lof.2:	lof.3:	lof.4:	lof.5:	lof.6:	lof.7:	lof.8:	lof.9:
	sub	$2*fm_lof,r0
	br	3f
lof.l:	jsr	pc,wrdoff;	br	1f
lof.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		3:add	-(spx),r0
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptr
.endif
	mov	(r0),(spx)+;	jmp	(next)

ldf.l:	jsr	pc,wrdoff;	br	1f
ldf.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	-(spx),r0
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptr
.endif
	mov	(r0)+,(spx)+;	mov	(r0),(spx)+;	jmp	(next)

sdf.l:	jsr	pc,wrdoff;	br	1f
sdf.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	-(spx),r0
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptr
.endif
	mov	-(spx),2(r0);	mov	-(spx),(r0);	jmp	(next)

stf.1:	stf.2:	stf.3:
	sub	$2*fm_stf,r0
	br	3f
stf.l:	jsr	pc,wrdoff;	br	1f
stf.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		3:add	-(spx),r0
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptr
.endif
	mov	-(spx),(r0);	jmp	(next)

lex.1:	mov	statd(lb),(spx)+;	jmp	(next)
lex.2:	mov	statd(lb),r0;	mov	statd(r0),(spx)+;	jmp	(next)
lex.l:	jsr	pc,wrdoff;	tst	r0;	bgt	1f
lex.0:	mov	lb,(spx)+;		jmp	(next)
lex.s0:	clr	r0;		bisb	(pcx)+,r0;	beq	lex.0
1:
.if	.prof
	jsr	r5,account;ac_lex
.endif
	mov	lb,r1
2:	mov	statd(r1),r1;	sob	r0,2b
	mov	r1,(spx)+;	jmp	(next)

loi.1:
.if	.test
	mov	-2(spx),-(sp);	jsr	pc,chkptr
.endif
	mov	*-(spx),(spx)+;	jmp	(next)
loi.2:	loi.3:	loi.4:
	sub	$2*fm_loi,r0
	asr	r0
	br	3f
los.z:	mov	-(spx),r0
.if	.test
	jeq	illzero
.endif
	cmp	r0,$1;		beq	lib
	asr	r0
.if	.test
	jcs	illodd
.endif
	br	1f
loi.l:	jsr	pc,wrdoff;	br	1f
loi.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_loi
.endif
3:
	mov	-(spx),r1
.if	.test
	mov	r1,-(sp);	jsr	pc,chkptr
.endif
2:	mov	(r1)+,(spx)+;	sob	r0,2b
	jmp	(next)

sti.1:	mov	-(spx),r0
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptr
.endif
	mov	-(spx),(r0);	jmp	(next)
sts.z:	mov	-(spx),r0
.if	.test
	jeq	illzero
.endif
	cmp	r0,$1;		beq	sib
	asr	r0
.if	.test
	jcs	illodd
.endif
	br	1f
sti.l:	jsr	pc,wrdoff;	br	1f
sti.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_sti
.endif
	mov	-(spx),r1
.if	.test
	mov	r1,-(sp);	jsr	pc,chkptr
.endif
	add	r0,r1;		add	r0,r1
2:	mov	-(spx),-(r1);	sob	r0,2b
	jmp	(next)

loi.0:
lib:	mov	-(spx),r0
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptrb
.endif
	clr	r1;		bisb	(r0),r1
	mov	r1,(spx)+;	jmp	(next)

sti.0:
sib:	mov	-(spx),r0;	mov	-(spx),r1
.if	.test
	mov	r0,-(sp);	jsr	pc,chkptrb
.endif
	movb	r1,(r0);	jmp	(next)
/
/------------------------------------------------------------------------------
/	test and branch group
/------------------------------------------------------------------------------

tlt.z:	tst	-(spx);	blt	true;	clr	(spx)+;	jmp	(next)
tle.z:	tst	-(spx);	ble	true;	clr	(spx)+;	jmp	(next)
teq.z:	tst	-(spx);	beq	true;	clr	(spx)+;	jmp	(next)
tne.z:	tst	-(spx);	bne	true;	clr	(spx)+;	jmp	(next)
tge.z:	tst	-(spx);	bge	true;	clr	(spx)+;	jmp	(next)
tgt.z:	tst	-(spx);	bgt	true;	clr	(spx)+;	jmp	(next)

zlt.s0:	tst	-(spx); blt	yesbr2;	br	nobr2
zlt.l:	tst	-(spx);	blt	yesbr3;	br	nobr3
zle.s0:	tst	-(spx); ble	yesbr2;	br	nobr2
zle.l:	tst	-(spx);	ble	yesbr3;	br	nobr3
zeq.s0:	tst	-(spx); beq	yesbr2;	br	nobr2
zeq.l:	tst	-(spx);	beq	yesbr3;	br	nobr3
zne.s0:	tst	-(spx); bne	yesbr2;	br	nobr2
zne.l:	tst	-(spx);	bne	yesbr3;	br	nobr3
zge.s0:	tst	-(spx); bge	yesbr2;	br	nobr2
zge.l:	tst	-(spx);	bge	yesbr3;	br	nobr3
zgt.s0:	tst	-(spx); bgt	yesbr2;	br	nobr2
zgt.l:	tst	-(spx);	bgt	yesbr3;	br	nobr3
great:
true:	mov	$1,(spx)+;	jmp	(next)

blt.s0:	cmp	-(spx),-(spx); bgt	yesbr2;	br	nobr2
blt.l:	cmp	-(spx),-(spx);	bgt	yesbr3;	br	nobr3
ble.s0:	cmp	-(spx),-(spx); bge	yesbr2;	br	nobr2
ble.l:	cmp	-(spx),-(spx);	bge	yesbr3;	br	nobr3
beq.s0:	cmp	-(spx),-(spx); beq	yesbr2;	br	nobr2
beq.l:	cmp	-(spx),-(spx);	beq	yesbr3;	br	nobr3
bne.s0:	cmp	-(spx),-(spx); bne	yesbr2;	br	nobr2
bne.l:	cmp	-(spx),-(spx);	bne	yesbr3;	br	nobr3
bge.s0:	cmp	-(spx),-(spx); ble	yesbr2;	br	nobr2
bge.l:	cmp	-(spx),-(spx);	ble	yesbr3;	br	nobr3
bgt.s0:	cmp	-(spx),-(spx); blt	yesbr2;	br	nobr2
bgt.l:	cmp	-(spx),-(spx);	blt	yesbr3;	br	nobr3

brf.s0:
yesbr2: clr	r0;	bisb	(pcx)+,r0;	add	r0,pcx;	jmp	(next)
brf.l:
yesbr3:	jsr	pc,wrdoff;	add	r0,pcx;	jmp	(next)

brb.s0:	clr	r0;	bisb	(pcx)+,r0;	sub	r0,pcx;	jmp	(next)
brb.l:	jsr	pc,wrdoff;			sub	r0,pcx;	jmp	(next)
nobr2:	tstb	(pcx)+;	jmp	(next)
nobr3:	cmpb	(pcx)+,(pcx)+;	jmp	(next)

.if	.float
cmf.z:	movf	-(spx),fr0;	cmpf	-(spx),fr0;	cfcc
	blt	less;		bgt	great
	clr	(spx)+;		jmp	(next)
.endif
less:	mov	$-1,(spx)+;	jmp	(next)

cmi.z:	cmp	-(spx),-(spx);	bgt	less;	blt	great
	clr	(spx)+;		jmp	(next)

cmd.z:	sub	$10,spx
	cmp	(spx),4(spx);	blt	less;	bgt	great
	cmp	2(spx),6(spx);	blo	less;	bhi	great
	clr	(spx)+;		jmp	(next)

cmu.1:	cmp	-(spx),-(spx)	
	bhi	less;		blo	great
	clr	(spx)+;		jmp	(next)
cms.z:	mov	-(spx),r0;	jeq	illzero
	asr	r0;		jcs	illodd
	br	1f
cmu.l:	jsr	pc,wrdoff;	br	1f
cmu.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_cmu
.endif
	mov	r2,-(sp);	asl	r0
	sub	r0,spx;		mov	spx,r1
	sub	r0,spx;		mov	spx,r2
	asr	r0
1:	cmp	(r1)+,(r2)+;	bne	2f
	sob	r0,1b;		mov	(sp)+,r2
	clr	(spx)+;		jmp	(next)
2:	bhi	3f;		mov	(sp)+,r2;	br	great
3:				mov	(sp)+,r2;	br	less
/
/------------------------------------------------------------------------------
/	aritmetic group
/------------------------------------------------------------------------------

add.z:
.if	.test
	mov	-(spx),r0;	cmp	r0,$und;	jeq	undef
	mov	-(spx),r1;	cmp	r1,$und;	jeq	undef
	add	r1,r0;		jvs	ovf
	mov	r0,(spx)+
.endif
.if	1-.test
	add	-(spx),-2(spx)
.endif
	jmp	(next)

sub.z:
.if	.test
	mov	-(spx),r0;	cmp	r0,$und;	jeq	undef
	mov	-(spx),r1;	cmp	r1,$und;	jeq	undef
	sub	r0,r1;		jvs	ovf
	mov	r1,(spx)+
.endif
.if	1-.test
	sub	-(spx),-2(spx)
.endif
	jmp	(next)

mul.z:
.if	.test
	mov	-(spx),r0;	cmp	r0,$und;	jeq	undef
	mov	-(spx),r1;	cmp	r1,$und;	jeq	undef
	clc;			mul	r0,r1;		jcs	ovf
	mov	r1,(spx)+;
.endif
.if	1-.test
	mov	-(spx),r1;	mul	-(spx),r1;	mov	r1,(spx)+
.endif
	jmp	(next)

div.z:
.if	.test
	mov	r2,-(sp);
	mov	-(spx),r2;	cmp	r2,$und;	jeq	undef
	mov	-(spx),r1;	sxt	r0;
	cmp	r1,$und;	jeq	undef
	div	r2,r0;		jcs	zerodivisor
	mov	r0,(spx)+;	mov	(sp)+,r2;	jmp	(next)
.endif
.if	1-.test
	mov	-4(spx),r1;	sxt	r0;
	div	-(spx),r0;	mov	r0,-2(spx);	jmp	(next)
.endif

mod.z:
.if	.test
	mov	r2,-(sp);
	mov	-(spx),r2;	cmp	r2,$und;	jeq	undef
	mov	-(spx),r1;	sxt	r0;
	cmp	r1,$und;	jeq	undef
	div	r2,r0;		jcs	zerodivisor
	mov	r1,(spx)+;	mov	(sp)+,r2;	jmp	(next)
.endif
.if	1-.test
	mov	-4(spx),r1;	sxt	r0;
	div	-(spx),r0;	mov	r1,-2(spx);	jmp	(next)
.endif

neg.z:
.if	.test
	cmp	-2(spx),$und;	jeq	undef
.endif
	neg	-2(spx)
.if	.test
	jvs	ovf
.endif
	jmp	(next)

undef:	mov	$1f,r0;	jsr	pc,fatal
ovf:	mov	$2f,r0;	jsr	pc,fatal
zerodivisor:mov	$3f,r0;	jsr	pc,fatal
illbool:
rangeerr:mov	$4f,r0;	jsr	pc,fatal
cseerr:	mov	$5f,r0;	jsr	pc,fatal
arerr:	mov	$6f,r0;	jsr	pc,fatal
illzero:
illodd:	mov	$7f,r0;	jsr	pc,fatal
linovf:	mov	$8f,r0;	jsr	pc,fatal
	.data
1:	.byte 78.;	<undefined integer\0>
2:	.byte 72.;	<integer overflow\0>
3:	.byte 76.;	<divide by 0\0>
4:	.byte 71.;	<range bound error\0>
5:	.byte 68.;	<case error\0>
6:	.byte 70.;	<array bound error\0>
7:	.byte 67.;	<odd or zero byte count\0>
8:	.byte 91.;	<argument of lin too high\0>
	.even
	.text

shl.z:	mov	-4(spx),r0;	ash	-(spx),r0;
	mov	r0,-2(spx);	jmp	(next)

shr.z:	neg	-(spx);		mov	-(spx),r0
	ash	2(spx),r0;	mov	r0,(spx)+;	jmp	(next)

rol.z:	mov	-4(spx),r1;	ashc	-(spx),r1
	mov	r1,-2(spx);	jmp	(next)

ror.z:	neg	-(spx);		mov	-(spx),r1;	ashc	2(spx),r1
	mov	r1,(spx)+;	jmp	(next)
/
/------------------------------------------------------------------------------
/	double precision arithmetic
/------------------------------------------------------------------------------

dad.z:	add	-(spx),-4(spx)		/ add low order word
	adc	-6(spx)			/ add carry
.if	.test
	jvs	ovf
.endif
	add	-(spx),-4(spx)		/ add high order word
.if	.test
	jvs	ovf
.endif
	jmp	(next)

dsb.z:	sub	-(spx),-4(spx)		/ sub low order word
	sbc	-6(spx)			/ subtract carry
.if	.test
	jvs	ovf
.endif
	sub	-(spx),-4(spx)		/ sub high order word
.if	.test
	jvs	ovf
.endif
	jmp	(next)

dmu.z:	setl;	setd
	movif	-(spx),fr0
	movif	-(spx),fr1
	mulf	fr1,fr0
	movfi	fr0,(spx)+
	setf;	seti
	jmp	(next)

ddv.z:	setl;	setd
	movif	-(spx),fr0
	movif	-(spx),fr1
	divf	fr0,fr1
	movfi	fr1,(spx)+
	setf;	seti
	jmp	(next)

dmd.z:	setl;	setd
	movif	-(spx),fr0
	movif	-(spx),fr1
	movf	fr1,fr2
	divf	fr0,fr2
	modf	$040200,fr2
	mulf	fr0,fr3
	subf	fr3,fr1
	movfi	fr1,(spx)+
	setf;	seti
	jmp	(next)
/
/------------------------------------------------------------------------------
/	conversion group
/------------------------------------------------------------------------------

cid.z:	mov	-(spx),r0;	sxt	(spx)+
	mov	r0,(spx)+;	jmp	(next)
cdi.z:	mov	-(spx),r0;	sxt	r1
	cmp	-(spx),r1;	beq	0f
	mov	$3f,r0;		jsr	pc,fatal
0:	mov	r0,(spx)+;	jmp	(next)
	.data
3:	.byte 83.;	<longint-\>int error\0>;	.even
	.text

.if	.float
cif.z:	movif	-(spx),fr0;	movf	fr0,(spx)+;	jmp	(next)
cfi.z:	movf	-(spx),fr0;	movfi	fr0,(spx)+;	jmp	(next)

cdf.z:	setl;			movif	-(spx),fr0;	movf	fr0,(spx)+;
	seti;			jmp	(next)
cfd.z:	setl;			movf	-(spx),fr0;	movfi	fr0,(spx)+;
	seti;			jmp	(next)
.endif
/
/------------------------------------------------------------------------------
/	boolean group
/------------------------------------------------------------------------------

and.1:	mov	-(spx),r0;	com	r0
	bic	r0,-2(spx);	jmp	(next)
ans.z:	mov	-(spx),r0;	beq	illzero
	asr	r0;		bcs	illodd
	br	1f
and.l:	jsr	pc,wrdoff;	br	1f
and.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_and
.endif
	sub	r0,spx;		sub	r0,spx
	mov	spx,r1;		mov	r2,-(sp)
	mov	spx,r2;		sub	r0,r2
	sub	r0,r2;
2:	com	(r1);		bic	(r1)+,(r2)+
	sob	r0,2b
	mov	(sp)+,r2;	jmp	(next)

ior.1:	bis	-(spx),-2(spx);	jmp	(next)
ior.2:	ior.3:	ior.4:
	sub	$2*fm_ior,r0
	asr	r0
	br	3f
ios.z:	mov	-(spx),r0;	jeq	illzero
	asr	r0;		jcs	illodd
	br	1f
ior.l:	jsr	pc,wrdoff;	br	1f
ior.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_ior
.endif
3:
	sub	r0,spx;		sub	r0,spx
	mov	spx,r1;		mov	r2,-(sp)
	mov	spx,r2;		sub	r0,r2
	sub	r0,r2
2:	bis	(r1)+,(r2)+;	sob	r0,2b
	mov	(sp)+,r2;	jmp	(next)

xos.z:	mov	-(spx),r0;	jeq	illzero
	asr	r0;		jcs	illodd
	br	1f
xor.l:	jsr	pc,wrdoff;	br	1f
xor.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_xor
.endif
	sub	r0,spx;		sub	r0,spx
	mov	spx,r1;		mov	r2,-(sp)
	mov	spx,r2;		sub	r0,r2
	sub	r0,r2;		mov	r3,-(sp)
2:	mov	(r1)+,r3;	xor	r3,(r2)+;	sob	r0,2b
	mov	(sp)+,r3;	mov	(sp)+,r2;	jmp	(next)

cos.z:	mov	-(spx),r0;	jeq	illzero
	asr	r0;		jcs	illodd
	br	1f
com.l:	jsr	pc,wrdoff;	br	1f
com.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_com
.endif
	mov	spx,r1;		sub	r0,r1;	sub	r0,r1
2:	com	(r1)+;		sob	r0,2b
	jmp	(next)

not.z:	mov	-(spx),r0;	jeq	great
	cmp	r0,$1;		jne	illbool
	clr	(spx)+;		jmp	(next)
/
/------------------------------------------------------------------------------
/	miscellaneous
/------------------------------------------------------------------------------

exg.z:	mov	-(spx),r0;	mov	-(spx),r1
	mov	r0,(spx)+;	mov	r1,(spx)+
	jmp	(next)

dup.1:	mov	-2(spx),(spx)+;	jmp	(next)
dus.z:	mov	-(spx),r0;	jeq	illzero
	asr	r0;		jcs	illodd
	br	1f
dup.l:	jsr	pc,wrdoff;	br	1f
dup.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_dup
.endif
	mov	spx,r1;		sub	r0,r1;		sub	r0,r1
2:	mov	(r1)+,(spx)+;	sob	r0,2b
	jmp	(next)

ads.z:	add	-(spx),-2(spx);	jmp	(next)
adi.l:	jsr	pc,wrdoff;	br	1f
adi.s0:	clr	r0;		bisb	(pcx)+,r0
1:	add	r0,-2(spx)
	/ fall through to nop-return

nop.z:	jmp	(next)

lin.l:	jsr	pc,wrdoff;	br	1f
lin.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.test
	cmp	r0,nlines;	jhi	linovf
.endif
	mov	r0,*eb
.if	.flow
	jsr	pc,seeflow
.endif
.if	.count
	jsr	pc,seecount
.endif
	jmp	(next)

beg.3:	mov	$und,(spx)+
beg.2:	mov	$und,(spx)+
beg.1:	mov	$und,(spx)+
	jmp	(next)
bes.z:	mov	-(spx),r0;	jeq	nobeg
	asr	r0;		jcs	illodd;		br	2f
beg.l:	jsr	pc,wrdoff;	tst	r0;		beq	nobeg
2:	bgt	1f;		add	r0,spx;		add	r0,spx
	jmp	(next)
beg.s0:	clr	r0;		bisb	(pcx)+,r0;	beq	nobeg
1:
.if	.prof
	jsr	r5,account;ac_beg
.endif
	mov	$und,r1
2:	mov	r1,(spx)+;	sob	r0,2b
nobeg:	jmp	(next)

bls.z:	mov	-(spx),r0;		br	1f
blm.l:	jsr	pc,wrdoff;		br	1f
blm.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_blm
.endif
	mov	r2,-(sp);	mov	-(spx),r2;	mov	-(spx),r1
.if	.test
	mov	r1,-(sp);	jsr	pc,chkptr
	mov	r2,-(sp);	jsr	pc,chkptr
.endif
1:	movb	(r1)+,(r2)+;	sob	r0,1b
	mov	(sp)+,r2;	jmp	(next)

rck.l:	jsr	pc,wrdoff;	br	1f
rck.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	eb,r0;	
	mov	-2(spx),r1;	cmp	(r0)+,r1;	jgt	rangeerr
	cmp	r1,(r0);	jgt	rangeerr;	jmp	(next)

cse.l:	jsr	pc,wrdoff;	br	1f
cse.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0;		add	eb,r0
	mov	-(spx),r1;	sub	(r0)+,r1
	cmp	r1,(r0)+;	bhi	outofrange;	tst	(r0)+
	asl	r1;		add	r1,r0;	
	mov	(r0),pcx;	bne	1f;	jmp	cseerr
outofrange:
	mov	(r0),pcx;	jeq	cseerr
1:	add	pb,pcx;		jmp	(next)

lor.l:	jsr	pc,wrdoff;	br	1f
lor.s0:	clr	r0;		bisb	(pcx)+,r0
1:	cmp	r0,$4;		jhi	illins
	asl	r0;		jmp	1f(r0)
1:	br	2f;	br	3f;	br	4f
	br	5f;	br	6f
2:	mov	pcx,(spx);	sub	pb,(spx)+;	jmp	(next)
3:	mov	pd,(spx)+;	jmp	(next)
4:	mov	lb,(spx)+;	jmp	(next)
5:	mov	spx,(spx);	sub	$2,(spx)+;	jmp	(next)
6:	mov	hp,(spx)+;	jmp	(next)

str.l:	jsr	pc,wrdoff;	br	1f
str.s0:	clr	r0;	bisb	(pcx)+,r0
1:	cmp	r0,$4;	jhi	illins
	asl	r0;	jmp	1f(r0)
1:	br	2f;	br	3f;	br	4f
	br	5f;	br	6f
2:	mov	-(spx),pcx;	add	pb,pcx;		jmp	(next)
3:	mov	-(spx),pd;	jmp	(next)
4:	mov	-(spx),lb;	jmp	(next)
5:	mov	-(spx),spx;	tst	(spx)+;	jmp	(next)
6:	mov	-(spx),hp;	mov	hp,sp;
	tst	(sp);		jmp	(next)

/
/------------------------------------------------------------------------------
/	array group
/------------------------------------------------------------------------------

lar.l:	jsr	pc,arad.l;
7:	bcs	lar1b		/if carry set load 1 byte
	mov	(r0),r0		/size in r0
	asr	r0		/size must be in words
1:	mov	(r1)+,(spx)+;	sob	r0,1b
	jmp	(next)
lar1b:	clr	r0		/avoid signextension !
	bisb	(r1),r0
	mov	r0,(spx)+
	jmp	(next)
lar.s0:	jsr	pc,arad.s0;	br	7b
lar.s1:	jsr	pc,arad.s1;	br	7b
las.z:	jsr	pc,arad.z;	br	7b

aar.l:	jsr	pc,arad.l;	mov	r1,(spx)+;	jmp	(next)
aar.s0:	jsr	pc,arad.s0;	mov	r1,(spx)+;	jmp	(next)
aar.s1:	jsr	pc,arad.s1;	mov	r1,(spx)+;	jmp	(next)
aas.z:	jsr	pc,arad.z;	mov	r1,(spx)+;	jmp	(next)

sar.l:	jsr	pc,arad.l
7:	bcs	sar1b
	mov	(r0),r0
	add	r0,r1
	asr	r0
1:	mov	-(spx),-(r1);	sob	r0,1b
	jmp	(next)
sar1b:	mov	-(spx),r0
	movb	r0,(r1)
	jmp	(next)
sar.s0:	jsr	pc,arad.s0;	br	7b
sar.s1:	jsr	pc,arad.s1;	br	7b
sas.z:	jsr	pc,arad.z;	br	7b
/
/------------------------------------------------------------------------------
/	set group
/------------------------------------------------------------------------------

ins.z:	mov	-(spx),r0;	br	2f
inn.l:	jsr	pc,wrdoff;	br	1f
inn.s0:	clr	r0;		bisb	(pcx)+,r0
1:	asl	r0		/ convert to bytes
2:
.if	.debug
	tst	r0;		jle	illins
.endif
	mov	-(spx),r1			/bitnumber to test
.if	.test
	ash	$-4,r1;		cmp	r1,r0
	bhis	notinset;	mov	(spx),r1
.endif
	sub	r0,spx		/ reduce stack
	clr	r0
	div	$8.,r0		/ r0=byte offset; r1=bit offset
	add	spx,r0		/ r0=address of byte
	bitb	bits(r1),(r0)	/ test bit
	bne	1f		/ bit was set
	clr	(spx)+		/ push 0 and return
	jmp	(next)
1:	mov	$1,(spx)+	/ push 1 and return
	jmp	(next)
notinset:mov	$1f,r0;	jsr	pc,fatal
	.data
1:	.byte 69.;	<set bound error\0>;	.even
	.text

ses.z:	mov	-(spx),r0;	asr	r0
.if	.test
	jcs	illodd
.endif
	br	1f
set.l:	jsr	pc,wrdoff;	br	1f
set.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_set
.endif
	mov	-(spx),r1	/ bitnumber in r1
.if	.test
	ash	$-4,r1;	cmp	r1,r0
	bhis	notinset;	mov	(spx),r1
.endif
	mov	spx,-(sp)	/ save spx (base of set)
1:	clr	(spx)+		/ make empty set
	sob	r0,1b		/ of right size
	/now r0 == 0, so clear not necessary
	div	$8.,r0		/ r0=byte offset; r1=bit offset
	add	(sp)+,r0	/ r0=byte address
	bisb	bits(r1),(r0)	/ set bit on
	jmp	(next)		/ that was all

	.data
bits:	.byte	1,	2,	4,	10,	20,	40,	100,	200
	.text
/
/------------------------------------------------------------------------------
/	floating arithmetic
/------------------------------------------------------------------------------


.if	.float

fad.z:	movf	-(spx),fr0;	addf	-(spx),fr0
	movf	fr0,(spx)+;	jmp	(next)
fsb.z:	movf	-10(spx),fr0;	subf	-(spx),fr0
	movf	fr0,-4(spx);	jmp	(next)
fmu.z:	movf	-(spx),fr0;	mulf	-(spx),fr0
	movf	fr0,(spx)+;	jmp	(next)
fdv.z:	movf	-10(spx),fr0;	divf	-(spx),fr0
	movf	fr0,-4(spx);	jmp	(next)

.endif

/
/------------------------------------------------------------------------------
/	mrk and cal group
/------------------------------------------------------------------------------
/
/	Markblock format on the stack is:
/	|-------------------------|
/	| Saved line number       |
/	|-------------------------|
/	| Return address          |
/	|-------------------------|
/	| Dynamic link            |
/	|-------------------------|
/	| Static link             |
/	|-------------------------|
/	|                         |
/	|-------------------------|
/	|                         |
/
/	Both links are set by mrk or mrs
/	The return address is filled in by cal or cas
/	So is the line number
/------------------------------------------------------------------------------

mrx.0:	mov	$loop1,next		/ change context
mrk.0:	mov	lb,(spx)+		/ push static link
mrs.z:	mov	lb,(spx)+		/ push dynamic link
	clr	(spx)+			/ space for return address
	mov	*eb,(spx)+		/ save line number
.if .debug
	cmp	trace,$2;blt 8f
	jsr	r5,mesg;	<After mrk:\0>
	.even
	jsr	pc,prregs
8:
.endif
	jmp	(next)			/ ready ...

mrx.1:	mov	$loop1,next
mrk.1:	mov	statd(lb),(spx)+;	br	mrs.z
mrx.2:	mov	$loop1,next
mrk.2:	mov	statd(lb),r0;	mov	statd(r0),(spx)+;	br	mrs.z
mrx.3:	mov	$loop1,next
mrk.3:	mov	statd(lb),r0;	mov	statd(r0),r0
	mov	statd(r0),(spx)+;	br	mrs.z

mrx.l:	mov	$loop1,next
mrk.l:	jsr	pc,wrdoff;	br	1f
mrx.s0:	mov	$loop1,next
mrk.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_mrk
.endif
	mov	lb,r1		/ go make static link
2:	mov	statd(r1),r1;	sob	r0,2b
	mov	r1,(spx)+	/ push static link
	br	mrs.z		/ finish the job elsewhere

mxs.z:	mov	$loop1,next	/ change context
	br	mrs.z		/ the rest is identical to mrs
/
cas.z:	mov	-(spx),r0;	br	1f	/ proc-number on the stack
cal.l:	jsr	pc,wrdoff;	br	1f	/ long proc-number
cal.s0:	clr	r0;		bisb	(pcx)+,r0/short proc-number
cal:	/ main loop jumps here from alternate context sometimes
1:	asl	r0;	asl	r0	/ proc-descriptors 4 bytes long
	add	pd,r0			/ r0 now points to proc-descriptor
	mov	spx,r1			/ r1 has to point to new lb
	sub	(r0)+,r1		/ it now does just that
.if	.test
	tst	-4(r1)			/ Is this ok ?
	bne	calerr
.endif
	mov	pcx,-4(r1)		/ fill in return-address
	mov	(r0),pcx		/ put base of proc in program counter
	mov	r1,lb			/ set lb
	mov	$loop,next		/ set normal context
.if	.debug
	cmp	trace,$2;blt 8f
	jsr	r5,mesg;	<After cal:\0>
	.even
	jsr	pc,prregs
8:
.endif
	jmp	(next)			/ cal finished


calerr:	mov	$1f,r0;	jsr	pc,fatal
	.data
1:	.byte 89.;	<parameter size error\0>;	.even
	.text
/------------------------------------------------------------------------------


/ ret.0 and ret.1 are separated from the rest for efficiency

ret.0:	mov	lb,r1		/ lb points just above markblock
	mov	-(r1),*eb	/ reset line number
	mov	-(r1),pcx	/ reset programcounter
	mov	-(r1),lb	/ reset lb
	tst	-(r1)		/ skip static link
	mov	r1,spx		/ reset stackpointer
	br	9f
ret.1:	mov	lb,r1
	mov	-(r1),*eb	/ reset line number
	mov	-(r1),pcx
	mov	-(r1),lb
	mov	-(spx),-2(r1)	/ move return value to its place
	mov	r1,spx
	br	9f

res.z:	mov	-(spx),r0;	asr	r0;	jcc	1f;	jmp	illodd
ret.l:	jsr	pc,wrdoff;	br	1f
ret.s0:	clr	r0;		bisb	(pcx)+,r0
1:
.if	.prof
	jsr	r5,account;ac_ret
.endif
	mov	lb,next		/ use next as scratch
	mov	-(next),*eb
	mov	-(next),pcx
	mov	-(next),lb
	tst	-(next)
	mov	spx,r1
	sub	r0,r1
	sub	r0,r1
	mov	next,spx
2:	mov	(r1)+,(spx)+
	sob	r0,2b
	mov	$loop,next	/ after ret always normal context
9:
.if	.debug
	cmp	trace,$2;blt	8f
	jsr	r5,mesg;	<After ret:\0>
	.even
	jsr	pc,prregs
8:
.endif
	jmp	(next)
.if	.debug

prregs:	mov	-8(spx),-(sp)
	mov	-6(spx),-(sp)
	mov	-4(spx),-(sp)
	mov	-2(spx),-(sp)
	mov	spx,-(sp)
	mov	lb,-(sp)
	mov	pcx,-(sp)
	mov	$1f,-(sp)
	jsr	pc,printf
	add	$20,sp
	rts	pc

	.data
1:	<pc=%5l,lb=%5l,spx=%5l,stack: %5l %5l %5l %5l\n\0>
	.even
	.text
.endif

swi.z:
.if	.debug
	dec	trace
	jsr	r5,mesg;	<*** *** Trace decremented\n\0>;.even
.endif
	jmp	(next)
swp.z:
.if	.debug
	inc	trace
	jsr	r5,mesg;	<*** *** Trace incremented\n\0>;.even
.endif
	jmp	(next)
/
/------------------------------------------------------------------------------
/	Exit instruction, with all it's crud
/------------------------------------------------------------------------------

hlt.z:				/ In fast mode no work
.if	.opfreq			/ Write out counttab
	sys	creat;1f;644	/ Create em1:opfreqtab
	sys	write;counttab;2048.	/ Write it out
	.data
1:	<em1:opfreqtab\0>;	.even
	.text
.endif
.if	.prof
	sys	creat;1f;644	/ Create em1:profile
	mov	ltime,2f+2
	mov	profsiz,2f+4
	sys	indir;2f	/ Write
	.data
1:	<em1:profile\0>;	.even
2:	sys	write;0;0
	.text
.endif
.if	.flow
	sys	creat;1f;644
	mov	lflow,2f+2
	mov	flowsiz,2f+4
	sys	indir;2f
	.data
1:	<em1:flowtab\0>;	.even
2:	sys	write;0;0
	.text
.endif
.if	.count
	sys	creat;1f;644
	mov	lcount,2f+2
	mov	countsiz,2f+4
	sys	indir;2f
	.data
1:	<em1:counttab\0>;	.even
2:	sys	write;0;0
	.text
.endif
	sys	exit
/
/------------------------------------------------------------------------------
/	System call interface routine
/------------------------------------------------------------------------------
R1_IN	=	0200
R0_IN	=	0100
R0_OUT	=	040
R1_OUT	=	020
CBIT	=	010


mon.z:	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	-(spx),r0
	bic	$177700,r0
	movb	r0,call
	movb	tab(r0),r3
	mov	r3,r2
	rolb	r3		/ R1_IN
	bcc	1f
	mov	-(spx),r1
1:	rolb	r3		/ R0_IN
	bcc	1f
	mov	-(spx),r0
1:	asl	r2		/ NARGS
	bic	$177761,r2
	add	$call+2,r2
1:	cmp	$call+2,r2
	beq	1f
	mov	-(spx),-(r2)
	br	1b
1:	clr	r2
	cmpb	call,$fork
	bne	1f
	clr	r1
	sys	fork
	inc	r1
	br	2f
1:	sys	indir;call
2:	bcc	1f
	mov	r0,r2
1:	rolb	r3		/ R0_OUT
	bcc	1f
	mov	r0,(spx)+
1:	rolb	r3		/ R1_OUT
	bcc	1f
	mov	r1,(spx)+
1:	rolb	r3		/ CBIT
	bcc	1f
	mov	r2,(spx)+
1:	mov	(sp)+,r3
	mov	(sp)+,r2
	jmp	(next)

.data

call:	sys 0;	0;	0;	0;	0

tab:
	.byte 0						/  0 = indir
	.byte 0		+R0_IN				/  1 = exit
	.byte 0			+R0_OUT	+R1_OUT	+CBIT	/  2 = fork
	.byte 2		+R0_IN	+R0_OUT		+CBIT	/  3 = read
	.byte 2		+R0_IN	+R0_OUT		+CBIT	/  4 = write
	.byte 2			+R0_OUT		+CBIT	/  5 = open
	.byte 0		+R0_IN			+CBIT	/  6 = close
	.byte 0			+R0_OUT	+R1_OUT	+CBIT	/  7 = wait
	.byte 2			+R0_OUT		+CBIT	/  8 = creat
	.byte 2					+CBIT	/  9 = link
	.byte 1					+CBIT	/ 10 = unlink
	.byte 2					+CBIT	/ 11 = exec
	.byte 1					+CBIT	/ 12 = chdir
	.byte 0			+R0_OUT	+R1_OUT		/ 13 = time
	.byte 3					+CBIT	/ 14 = mknod
	.byte 2					+CBIT	/ 15 = chmod
	.byte 2					+CBIT	/ 16 = chown
	.byte 1					+CBIT	/ 17 = break
	.byte 2					+CBIT	/ 18 = stat
	.byte 2		+R0_IN			+CBIT	/ 19 = seek
	.byte 0			+R0_OUT			/ 20 = getpid
	.byte 3					+CBIT	/ 21 = mount
	.byte 1					+CBIT	/ 22 = umount
	.byte 0		+R0_IN			+CBIT	/ 23 = setuid
	.byte 0			+R0_OUT			/ 24 = getuid
	.byte 0	+R1_IN	+R0_IN			+CBIT	/ 25 = stime
	.byte 3		+R0_IN	+R0_OUT		+CBIT	/ 26 = ptrace
	.byte 0		+R0_IN	+R0_OUT			/ 27 = alarm
	.byte 1		+R0_IN			+CBIT	/ 28 = fstat
	.byte 0						/ 29 = pause
	.byte 1						/ 30 = smdate; inoperative
	.byte 1		+R0_IN			+CBIT	/ 31 = stty
	.byte 1		+R0_IN			+CBIT	/ 32 = gtty
	.byte 2					+CBIT	/ 33 = access
	.byte 0		+R0_IN			+CBIT	/ 34 = nice
	.byte 0		+R0_IN				/ 35 = sleep
	.byte 0						/ 36 = sync
	.byte 1		+R0_IN			+CBIT	/ 37 = kill
	.byte 0			+R0_OUT			/ 38 = switch
	.byte 0						/ 39 = setpgrp (not in yet)
	.byte 0		+R0_IN	+R0_OUT	+R1_OUT	+CBIT	/ 40 = tell
	.byte 0		+R0_IN	+R0_OUT		+CBIT	/ 41 = dup
	.byte 0			+R0_OUT	+R1_OUT	+CBIT	/ 42 = pipe
	.byte 1						/ 43 = times
	.byte 4						/ 44 = prof
	.byte 0						/ 45 = tiu
	.byte 0		+R0_IN			+CBIT	/ 46 = setgid
	.byte 0			+R0_OUT			/ 47 = getgid
	.byte 2			+R0_OUT		+CBIT	/ 48 = sig
	.byte 0						/ 49 = reserved for USG
	.byte 0						/ 50 = reserved for USG
	.byte 1					+CBIT	/ 51 = turn acct off/on
	.byte 0						/ 52 = x
	.byte 0						/ 53 = x
	.byte 0						/ 54 = x
	.byte 0						/ 55 = x
	.byte 0						/ 56 = x
	.byte 0						/ 57 = x
	.byte 0						/ 58 = x
	.byte 0						/ 59 = x
	.byte 0						/ 60 = x
	.byte 0						/ 61 = x
	.byte 0						/ 62 = x
	.byte 0						/ 63 = x

	.text
/
/------------------------------------------------------------------------------
/	External routines
/------------------------------------------------------------------------------
.if	.ext
zzz.z:
	mov	-(spx),r0
	asl	r0
	cmp	r0,$endxtab-xtab
	jhis	illins
	cmp	r0,$endxtab-xtab-4
	jhis	xcvt
	movf	-(spx),fr0
	mov	r0,saver0
	jsr	pc,*xtab(r0)
	bcc	0f
	mov	saver0,r0
	mov	xerr(r0),r0
	jsr	pc,fatal
0:	movf	fr0,(spx)+
	jmp	(next)
xcvt:	mov	r2,-(sp)	/ save r2
	mov	-(spx),1f	/ address of buffer
	mov	-(spx),_ndigit	/ # of digits to be produced
	mov	-(spx),1f+2	/ some other address
	mov	-(spx),1f+4	/ ditto
	movf	-(spx),fr0	/ real argument
	jsr	pc,*xtab(r0)	/ call it now
	mov	r2,*1f+2
	mov	r1,*1f+4
	mov	1f,r1
2:	movb	(r0)+,(r1)+
	bne	2b
	mov	(sp)+,r2
	jmp	(next)

	.data
.globl	sin,	cos,	atan,	exp,	log,	sqrt,	ecvt,	fcvt,	_ndigit
xtab:	sin;	cos;	atan;	exp;	log;	sqrt;	ecvt;	fcvt
endxtab:
xerr:	0f;1f;2f;3f;4f;5f
0:	.byte 129.;	<error in sin\0>
1:	.byte 130.;	<error in cos\0>
2:	.byte 134.;	<error in arctan\0>
3:	.byte 131.;	<error in exp\0>
4:	.byte 132.;	<error in ln\0>
5:	.byte 133.;	<error in sqrt\0>
	.even
	.bss
1:	.=.+6
	.text
.endif
/
/------------------------------------------------------------------------------
/	General subroutines
/------------------------------------------------------------------------------

.if	.debug
	.globl	_printf
printf:	mov	(sp)+,0f	/save return address
	mov	r0,0f+2		/save r0
	mov	r1,0f+4		/save r1
	jsr	pc,_printf	/_printf messes around with r0 and r1
	mov	0f+4,r1
	mov	0f+2,r0		/restore registers
	jmp	*0f		/return
	.bss
0:	.=.+6			/ storage for registers and return address
	.text
.endif

wrdoff:	movb	(pcx)+,r0	/get first byte
	swab	r0		/put it in high byte
	clrb	r0		/clear low byte of r0
	bisb	(pcx)+,r0	/"or" second byte in
	rts	pc		/done

.if	.flow
seeflow:
	mov	*eb,r1		/ line number in r1
	clr	r0		/ get ready for divide
	div	$8.,r0		/ r0 = byte offset; r1 = bit offset
	add	lflow,r0	/ r0 is byte pointer now
	bisb	bits(r1),(r0)	/ set bit
	rts	pc		/ that was all
.endif
.if	.count
seecount:
	mov	*eb,r1		/ line number in r1
	ash	$2,r1		/ multiply by 4
	add	lcount,r1	/ r1 is pointer to long integer now
	add	$1,2(r1)	/ cannot be inc
	adc	(r1)		/ that was all
	rts	pc
.endif

segviol:mov	r0,segr0	/save r0
.if .debug
	jsr	r5,mesg;	<*** *** Caught segmentation violation\n\0>
	.even
.endif
	cmp	spx,sybreak+2	/was it a trap due to stack overflow ?
	blo	1f		/no
	mov	spx,sybreak+2
	add	$memincr,sybreak+2
	sys	indir;sybreak	/ask for more core
	bes	2f		/no core
	sys	signal;11.;segviol
1:	mov	segr0,r0
	rti
2:	mov	$4f,r0
	jsr	pc,fatal
	.data
4:	.byte 64.;	<Stack overflow\0>;	.even
	.even
sybreak:sys	break;_end	/For indirect calling of break
	.bss
segr0:	.=.+2
	.text
.if	.float
fppflt:	mov	r0,saver0
	mov	$e7,fpperr+6
	stfps	r0
	bit	$100,r0
	beq	1f
	mov	$e11,fpperr+6
1:	stst	r0
	mov	fpperr(r0),r0
	jsr	pc,fatal
	.data
fpperr:	e5; e5; e6; e7; e8; e9; e10; e5
e5:	.byte 84.;	<fpp error\0>
e6:	.byte 77.;	<divide by 0.0\0>
e7:	.byte 81.;	<real-\>int error\0>
e8:	.byte 74.;	<real overflow\0>
e9:	.byte 75.;	<real underflow\0>
e10:	.byte 80.;	<real undefined\0>
e11:	.byte 82.;	<real-\>long error\0>
	.text
.endif

arad.z:	mov	-(spx),r0;	br	3f
arad.s1:mov	$400,r0;	br	2f
arad.s0:clr	r0;		2:bisb	(pcx)+,r0;	br	1f
arad.l:	jsr	pc,wrdoff;	/get address of arraydescriptor in r0
1:	asl	r0		/ get word address
	add	eb,r0		/ make hard address
3:	mov	-(spx),r1
	sub	(r0)+,r1	/normalize to 0..n
	cmp	r1,(r0)+	/is it too big?
.if	.test
	jhi	arerr
.endif
	cmp	(r0),$1
	beq	1f
	mul	(r0),r1
	add	-(spx),r1
	clc
	rts	pc
1:	add	-(spx),r1
	sec
	rts	pc

.if	.test
/
/	Calling sequence:	mov	???,-(sp);	jsr	pc,chkptr(b)
/	This will trap a bad pointer. In case of chkptrb, odd pointers are
/	allowed.
/
chkptr:	mov	r0,saver0;	mov	2(sp),r0
	bit	$1,r0;		bne	ptrerr;		br	1f
chkptrb:mov	r0,saver0;	mov	2(sp),r0
1:
	cmp	r0,eb;	blo	ptrerr
	cmp	r0,spx;	blo	ptrok
	cmp	r0,hp;	blo	ptrerr
	cmp	r0,ml;	blos	ptrok
	cmp	r0,argv;blo	ptrerr
ptrok:	mov	(sp)+,r0
	mov	r0,(sp)
	mov	saver0,r0
	rts	pc
ptrerr:	mov	$1f,r0;	jsr	pc,fatal
	.data
1:	.byte 90.;	<bad pointer\0>;	.even
	.text
.endif

.if	.prof
/
/	Calling sequence:	jsr	r5,account;ac_opc
/	This procedure adds r0*ac_opc to the double precision counter
/	that has r1 pointing to its high-order word.
/
account:mov	r3,-(sp)	/ get yourself an odd register
	mov	r0,r3		/ get ready for multiply
	mul	(r5)+,r3	/ now r3 contains product
	add	r3,2(r1)	/ make first part of addition
	adc	(r1)		/ addition finished
	mov	(sp)+,r3	/ restore register
	rts	r5		/ exit, stage left
.endif

fatal:	mov	r0,r1
	mov	r0,saver0
	mov	$2,r0
	sys	write;1f;5
	.data
1:	<em1: >;	.even
	.text
	inc	r1		/ skip over first byte
	mov	r1,r0
1:	tstb	(r1)+;	bne	1b
	mov	r0,1f+2
	sub	r0,r1
	mov	r1,1f+4
	mov	$2,r0
	sys	indir;1f
	.data
1:	sys	write;0;0
	.text
	tst	*eb
	beq	9f
	mov	$2,r0
	sys	write;1f;16.
	.data
1:	< on source line >;	.even
	.text
	mov	r3,-(sp)
	mov	$1f+5,r3
	mov	*eb,r1
2:	clr	r0
	div	$10.,r0
	bisb	r1,-(r3)
	mov	r0,r1
	bne	2b
	mov	r3,2f+2
	sub	$1f+5,r3
	neg	r3
	mov	r3,2f+4
	mov	(sp)+,r3
	mov	$2,r0
	sys	indir;2f
	.data
1:	<00000>;	.even
2:	sys	write;0;0
	.text
9:	mov	$2,r0
	sys	write;1f;1
	.data
1:	<\n>;	.even
	.text
	mov	$5,r0
	sys	sleep
	movb	*saver0,r0
	sys	exit

	.data
.if	.debug
trace:	3
.endif


	.bss
eb:	.=.+2
pb:	.=.+2
pd:	.=.+2
hp:	.=.+2
ml:	.=.+2
initstck:.=.+2
.if	.prof
ltime:	.=.+2
profsiz:.=.+2
.endif
.if	.flow
lflow:	.=.+2
flowsiz:.=.+2
.endif
.if	.count
lcount:	.=.+2
countsiz:.=.+2
.endif
.if	.opfreq
counttab:
	.=.+2048.
.endif
saver0:	.=.+2

header:
	txtsiz:	.=.+2
	datsiz:	.=.+2
	holsiz:	.=.+2
	procsiz:.=.+2
	relsiz:	.=.+2
	entry.:	.=.+2
	nlines:	.=.+2
		.=.+2	/ unused
argc:	.=.+2
argv:	.=.+2
