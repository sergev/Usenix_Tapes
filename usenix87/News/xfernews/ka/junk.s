	.file	"junk.c"
	.data
	.globl	nextdir
	.bss	nextdir,44,4
	.globl	batfp
	.bss	batfp,4,4
	.globl	nfiles
	.bss	nfiles,4,4
	.text
	.set	.F1,0
	.align	4
	.def	addfile;	.val	addfile;	.scl	2;	.type	044;	.endef
	.globl	addfile
	.set	.R1,1
addfile:
	save	&.R1,&0
	subw2	&1,*batfp
	jge	.L9999
	movtwb	0(%ap),%r0
	pushw	%r0
	pushw	batfp
	call	&2,_flsbuf
	jmp	.L9998
..0:
	movtwb	%r8,%r0
	pushw	%r0
	pushw	batfp
	call	&2,_flsbuf
	jmp	.L9996
.L9999:
	movtwb	0(%ap),%r0
	movw	batfp,%r1
	movw	4(%r1),%r2
	movtwb	%r0,0(%r2)
	addw2	&1,4(%r1)
.L9996:
.L9998:
	movzbw	%r0,%r0
	cmpw	%r0,&-1
	jne	.L54
.L49:
	pushw	&.L51
	call	&1,fatal
.L54:
	subw2	&1,*4(%ap)
	jge	.L9995
	pushw	4(%ap)
	call	&1,_filbuf
	movw	%r0,%r8
	jmp	.L9994
.L9995:
	movw	4(%ap),%r0
	movzbw	*4(%r0),%r8
	addw2	&1,4(%r0)
	movw	%r8,%r0
.L9994:
	cmpw	%r0,&-1
	jne	.L55
	subw2	&1,*batfp
	jge	.L9993
	pushzb	&131
	pushw	batfp
	call	&2,_flsbuf
	jmp	.L9992
.L55:
	subw2	&1,*batfp
	jl	..0
	movtwb	%r8,%r0
	movw	batfp,%r1
	movw	4(%r1),%r2
	movtwb	%r0,0(%r2)
	addw2	&1,4(%r1)
	jmp	.L9996
.L9993:
	movw	batfp,%r0
	movw	4(%r0),%r1
	movb	&131,0(%r1)
	addw2	&1,4(%r0)
	movzbw	0(%r1),%r0
.L9992:
	cmpw	%r0,&-1
	je	.L49
	addw2	&1,nfiles
	ret	&.R1
	.def	addfile;	.val	.;	.scl	-1;	.endef
	.data
	.text
	.set	.F2,0
	.align	4
	.def	raddfile;	.val	raddfile;	.scl	2;	.type	044;	.endef
	.globl	raddfile
	.set	.R2,2
raddfile:
	save	&.R2,&0
	movw	4(%ap),%r8
	subw2	&1,*batfp
	jge	.L9991
	movtwb	0(%ap),%r0
	pushw	%r0
	pushw	batfp
	call	&2,_flsbuf
	jmp	.L9990
..1:
	movtwb	%r7,%r0
	pushw	%r0
	pushw	batfp
	call	&2,_flsbuf
	jmp	.L9988
.L9991:
	movtwb	0(%ap),%r0
	movw	batfp,%r1
	movw	4(%r1),%r2
	movtwb	%r0,0(%r2)
	addw2	&1,4(%r1)
.L9988:
.L9990:
	movzbw	%r0,%r0
	cmpw	%r0,&-1
	jne	.L64
.L61:
	pushw	&.L62
	call	&1,fatal
.L64:
	subw2	&1,0(%r8)
	jge	.L9987
	pushw	%r8
	call	&1,_filbuf
	movw	%r0,%r7
	jmp	.L9986
.L9987:
	movzbw	*4(%r8),%r7
	addw2	&1,4(%r8)
	movw	%r7,%r0
.L9986:
	cmpw	%r0,&-1
	jne	.L65
	subw2	&1,*batfp
	jge	.L9985
	pushzb	&131
	pushw	batfp
	call	&2,_flsbuf
	jmp	.L9984
.L65:
	subw2	&1,*batfp
	jl	..1
	movtwb	%r7,%r0
	movw	batfp,%r1
	movw	4(%r1),%r2
	movtwb	%r0,0(%r2)
	addw2	&1,4(%r1)
	jmp	.L9988
.L9985:
	movw	batfp,%r0
	movw	4(%r0),%r1
	movb	&131,0(%r1)
	addw2	&1,4(%r0)
	movzbw	0(%r1),%r0
.L9984:
	cmpw	%r0,&-1
	je	.L61
	addw2	&1,nfiles
	ret	&.R2
	.def	raddfile;	.val	.;	.scl	-1;	.endef
	.data
.L51:
	.byte	0x62,0x61,0x74,0x63,0x68,0x20,0x66,0x69
	.byte	0x6c,0x65,0x20,0x77,0x72,0x69,0x74,0x65
	.byte	0x20,0x65,0x72,0x72,0x6f,0x72,0x0
.L62:
	.byte	0x62,0x61,0x74,0x63,0x68,0x20,0x66,0x69
	.byte	0x6c,0x65,0x20,0x77,0x72,0x69,0x74,0x65
	.byte	0x20,0x65,0x72,0x72,0x6f,0x72,0x0
	.text
