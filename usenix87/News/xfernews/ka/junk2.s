	.file	"junk2.c"
	.data
	.text
	.set	.F1,4
	.align	4
	.def	main;	.val	main;	.scl	2;	.type	044;	.endef
	.globl	main
	.set	.R1,0
main:
	save	&.R1,&1
	movw	&_iob+16,0(%fp)
	pushw	0(%fp)
	pushw	&0
	call	&2,setbuf
	pushw	&1
	call	&1,close
	subw2	&1,*0(%fp)
	jge	.L9999
	pushzb	&88
	pushw	0(%fp)
	call	&2,_flsbuf
	jmp	.L9998
.L9999:
	movw	0(%fp),%r0
	movw	4(%r0),%r1
	movb	&88,0(%r1)
	addw2	&1,4(%r0)
	movzbw	0(%r1),%r0
.L9998:
	cmpw	%r0,&-1
	jne	.L31
	pushw	&_iob+32
	pushw	&.L33
	jmp	..0
.L31:
	pushw	&_iob+32
	pushw	&.L35
..0:
	call	&2,fprintf
	movw	0(%fp),%r0
	bitb	&32,12(%r0)
	jz	.L36
	pushw	&_iob+32
	pushw	&.L37
	call	&2,fprintf
.L36:
	ret	&.R1
	.def	main;	.val	.;	.scl	-1;	.endef
	.data
.L33:
	.byte	0x70,0x75,0x74,0x63,0x20,0x72,0x65,0x74
	.byte	0x75,0x72,0x6e,0x65,0x64,0x20,0x45,0x4f
	.byte	0x46,0xa,0x0
.L35:
	.byte	0x70,0x75,0x74,0x63,0x20,0x64,0x69,0x64
	.byte	0x20,0x6e,0x6f,0x74,0x20,0x72,0x65,0x74
	.byte	0x75,0x72,0x6e,0x20,0x45,0x4f,0x46,0xa
	.byte	0x0
.L37:
	.byte	0x66,0x65,0x72,0x72,0x6f,0x72,0x20,0x73
	.byte	0x65,0x74,0xa,0x0
	.text
