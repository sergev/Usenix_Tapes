/
/ copyright 1972 bell telephone laboratories inc.
/

/ a21 -- pdp-11 assembler pass 2 

indir	= 0

main:
	sys	signal; 2; 1
	ror	r0
	bcs	1f
	sys	signal; 2; aexit
1:
	jmp	start

/ set up sizes and origins

go:

/ read in symbol table

	mov	$usymtab,r1
1:
	jsr	pc,getw
	bvs	1f
	add	$14,symsiz		/ count symbols
	jsr	pc,getw
	jsr	pc,getw
	jsr	pc,getw
	jsr	pc,getw
	mov	r4,r0
	bic	$!37,r0
	cmp	r0,$2			/text
	blo	2f
	cmp	r0,$3			/data
	bhi	2f
	add	$31,r4			/mark "estimated"
	mov	r4,(r1)+
	jsr	pc,getw
	mov	r4,(r1)+
	br	3f
2:
	clr	(r1)+
	clr	(r1)+
	jsr	pc,getw
3:
	jsr	pc,setbrk
	br	1b
1:

/ read in f-b definitions

	mov	r1,fbbufp
	movb	fbfil,fin
	clr	ibufc
1:
	jsr	pc,getw
	bvs	1f
	add	$31,r4			/ "estimated"
	mov	r4,(r1)+
	jsr	pc,getw
	mov	r4,(r1)+
	jsr	pc,setbrk
	br	1b
1:
	mov	r1,endtable
	mov	$100000,(r1)+

/ set up input text file; initialize f-b table

	jsr	pc,setup
/ do pass 1

	jsr	pc,assem

/ prepare for pass 2
	cmp	outmod,$755
	beq	1f
	jmp	aexit
1:
	clr	dot
	mov	$2,dotrel
	mov	$..,dotdot
	clr	brtabp
	movb	fin,r0
	sys	close
	jsr	r5,ofile; a.tmp1
	movb	r0,fin
	clr	ibufc
	jsr	pc,setup
	incb	passno
	inc	bsssiz
	bic	$1,bsssiz
	mov	txtsiz,r1
	inc	r1
	bic	$1,r1
	mov	r1,txtsiz
	mov	datsiz,r2
	inc	r2
	bic	$1,r2
	mov	r2,datsiz
/
/	following this comment a lot of code was added by Hans van Staveren
/
/	This was necessary due to the error in this assembler that
/	assumed a.out files would never grow larger than 2^16 bytes
/	The contrary has however been experienced.
/
/	Since I did not understand all of the assembler I did the following:
/	I created the a.out file, opened it 3 times more for writing,
/	and once and for all seeked those file-descriptors to the place
/	where they should point.
/	I do not consider the change very nice but it seemed the only
/	way, considering the dirty programming of the people who 
/	wrote this assembler.
/

	clr	r4
	mov	r1,r5
	add	$20,r5
	adc	r4
	mov	datap+6,r0
	jsr	pc,doseek	/seek data-filedescriptor

	clr	r4
	mov	r2,r5
	add	$20,r5
	adc	r4
	add	r1,r5
	adc	r4
	mov	trelp+6,r0
	jsr	pc,doseek	/seek text-reloc-filedescriptor

	clr	r4
	mov	r2,r5
	add	$20,r5
	adc	r4
	add	r1,r5
	adc	r4
	add	r1,r5
	adc	r4
	mov	drelp+6,r0
	jsr	pc,doseek	/seek data-reloc-filedescriptor

	br	1f

doseek:	mov	r0,r3		/save fildes in r3
	div	$1000,r4	/now r4=blockoffset,r5=byteoffset
	mov	r4,syssk+2
	mov	$3,syssk+4
	sys	indir;syssk	/seek blocks
	bes	skerr
	mov	r5,syssk+2
	mov	$1,syssk+4
	mov	r3,r0
	sys	indir;syssk	/seek bytes
	bes	skerr
	rts	pc


skerr:	jsr	r5,filerr;a.outp
	/ no return

	.data
syssk:	sys	seek;..;..	/for seeks
	.text

/	end of added code here
/	many instructions that follow now are probably useless
/	I didn't dare to remove them.
/
1:
	mov	r1,r3
	mov	r3,datbase	/ txtsiz
	mov	r3,savdot+2
	add	r2,r3
	mov	r3,bssbase	/ txtsiz+datsiz
	mov	r3,savdot+4
	asl	r3
	add	$20,r3
	mov	r3,symseek	/ 2*txtsiz+2*datsiz+20
	sub	r2,r3
	mov	r3,drelseek	/ 2*txtsiz+datsiz
	sub	r1,r3
	mov	r3,trelseek	/ txtsiz+datsiz+20
	sub	r2,r3
	mov	r3,datseek	/ txtsiz+20
	mov	$usymtab,r1
1:
	jsr	pc,doreloc
	add	$4,r1
	cmp	r1,endtable
	blo	1b
	clr	r0
	jsr	r5,oset; textp
	mov	trelseek,r0
	jsr	r5,oset; trelp
/	code added
	mov	datseek,r0
	jsr	r5,oset;datap
	mov	drelseek,r0
	jsr	r5,oset;drelp
/	end of added code
	mov	$8.,r2
	mov	$txtmagic,r1
1:
	mov	(r1)+,r0
	jsr	r5,putw; txtp
	dec	r2
	bne	1b
	jsr	pc,assem

/polish off text and relocation

	jsr	r5,flush; textp
	jsr	r5,flush; trelp
	jsr	r5,flush; datap
/	don't flush data-relocation, symbols are written after it

/ append full symbol table

	mov	symf,r0
	mov	r0,fin
	sys	seek; 0; 0;
	clr	ibufc
	mov	$drelp,relp	/write "data-relocation" now
	mov	$usymtab,r1
1:
	jsr	pc,getw
	bvs	1f
	mov	r4,r0
	jsr	r5,putw; relp
	jsr	pc,getw
	mov	r4,r0
	jsr	r5,putw; relp
	jsr	pc,getw
	mov	r4,r0
	jsr	r5,putw; relp
	jsr	pc,getw
	mov	r4,r0
	jsr	r5,putw; relp
	mov	(r1)+,r0
	jsr	r5,putw; relp
	mov	(r1)+,r0
	jsr	r5,putw; relp
	jsr	pc,getw
	jsr	pc,getw
	br	1b
1:
	jsr	r5,flush; drelp
	jmp	aexit

	.data
aexit:
	mov	a.tmp1,0f
	sys	unlink; 0:..
	mov	a.tmp2,0f
	sys	unlink; 0:..
	mov	a.tmp3,0f
	sys	unlink; 0:..
	mov	a.outp,0f
	sys	chmod; 0: ..; outmod: 755
	movb	errflg,r0
	sys	exit
	.text

filerr:
	mov	*(r5),r3
1:
	movb	(r3)+,ch
	beq	1f
	mov	$1,r0
	sys	write; ch; 1
	br	1b
1:
	mov	$1,r0
	sys	write; qnl; 2
	movb	$-1,errflg
	jmp	aexit

doreloc:
	movb	(r1),r0
	bne	1f
	bisb	defund,(r1)
1:
	bic	$!37,r0
	cmp	r0,$5
	bhis	1f
	cmp	r0,$3
	blo	1f
	beq	2f
	add	bssbase,2(r1)
	rts	pc
2:
	add	datbase,2(r1)
1:
	rts	pc

setbrk:
	mov	r1,-(sp)
	add	$20,r1
	cmp	r1,0f
	blo	1f
	add	$512.,0f
	sys	indir; 9f
	.data
9:	sys	break; 0: end
	.text
1:
	mov	(sp)+,r1
	rts	pc

setup:
	mov	$curfb,r4
1:
	clr	(r4)+
	cmp	r4,$curfb+40.
	blo	1b
	mov	txtfil,fin
	clr	ibufc
	clr	r4
1:
	jsr	pc,fbadv
	tstb	(r4)+
	cmp	r4,$10.
	blt	1b
	rts	pc

ofile:
	mov	*(r5),0f
	sys	indir; 9f
	.data
9:	sys	open; 0:..; 0
	.text
	bes	1f
	tst	(r5)+
	rts	r5
1:
	jmp	filerr
