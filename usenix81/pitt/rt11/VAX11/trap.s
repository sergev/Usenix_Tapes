#
#	RT11 EMULATOR
#	system call traps and
#	compatibility mode support
#
#	Daniel R. Strick
#	January, 1981
#	7/21/81 -- changed the ways in which a bunch of things get into
#		   the address space and declare their locations
#	8/31/81 -- inserted hook to core dump after bad emt
#

	.globl	_emtrap
	.globl	_start
	.globl	_embase			# used to estimate bottom of emulator

	.globl	_compret		# address of completion routine return
	.globl	_comprtn		# address of completion routine
	.globl	_compcsw		# csw for completion routine
	.globl	_compchn		# channel number for completion routine

	.set	SYSVEC,	034		# sys call trap vector
	.set	STARTAD,040		# rt11 starting address word
	.set	ISTACK,	042		# rt11 initial stack top word
	.set	RMONAD,	054		# rt11 monitor address word

	.set	S_PSW,	 4
	.set	S_PC,	16

	.set	compat, 0x83c0		# compat | cur user | prev user
	.set	emtcod, 0104000		# pdp11 emulator trap instruction code
	.set	syscod,	0104400		# pdp11 trap instruction code

	.text
	.align	2
_embase:
illins:
	.word	0x0
#
#  The following code is appropriate for 4BSD vmunix.
#  See below for the 3BSD and 32V incantations.
#
	moval	uw1, S_PC(fp)		# unwind stack to interrupt time
	ret
uw1:
	moval	uw2, S_PC(fp)
	ret
uw2:
	movzwl	(sp)+, r7		# get interrupted pc
	movzwl	(sp)+, r8		# get interrupted ps
#
# The code inside this comment is appropriate for 3BSD and perhaps 32V.
#
#	movzwl	S_PSW(fp), r8		# get interrupted ps and pc
#	movzwl	S_PC(fp), r7
#	moval	uw, S_PC(fp)		# unwind stack to interrupt time
#	ret
#uw:
#
	movzwl	(r7)+, r9		# skip over sys call, just like pdp11
	pushr	$0x1ff			# save regs: r0 - r5, sp, pc, ps
	bicw2	$0xff, r9
	cmpw	$emtcod, r9		# make sure it is right type of call
	beql	emtrap
	cmpw	$syscod, r9
	beql	syscal
	movzwl	-(r7), r9
	clrl	r10			# a hack to get regs in core dump
	movw	r0,(r10)+
	movw	r1,(r10)+
	movw	r2,(r10)+
	movw	r3,(r10)+
	movw	r4,(r10)+
	movw	r5,(r10)+
	movw	r6,(r10)+
	movw	r7,(r10)+
	pushl	r7			# otherwise cause error message
	pushl	r9
	pushal	msg2
	calls	$3, _printf
	calls	$6, _dump

syscal:
	popr	0x1ff
	movw	r8, -(r6)		# fake a trap exception
	movw	r7, -(r6)
	movw	$compat, -(sp)
	movw	r8, -(sp)
	movzwl	*$SYSVEC, -(sp)
	rei

emtrap:
	calls	$0, _emtrap		# lie about arg count to protect regs
	popr	$0x07f			# restore, leaving psw & pc on stack
	movw	$compat, 6(sp)		# set up compatability mode psl
	tstw	_comprtn
	beql	nocomp
	movw	4(sp), -(r6)		# completion routine kludge:
	movw	0(sp), -(r6)		#  -push ps/pc onto user's stack for rtt
	movw	r0, -(r6)		#  -push user's r0, r1 onto his stack
	movw	r1, -(r6)		#  -push addr of rtt routine onto stack
	movw	_compret, -(r6)		#  -load csw,chn into r0,r1 for comp rtn
	movzwl	_compcsw, r0		#  -switch resumption loc to comp rtn
	movzwl	_compchn, r1		#  -clear completion routine request
	movzwl	_comprtn, 0(sp)
	clrw	_comprtn
nocomp:
	rei				# resume user's program


	.align	1
_start:
	.word	0x3ff			# saves r0 - r9
	pushal	illins
	pushl	$4
	calls	$2, _signal
	clrl	r0
	clrl	r1
	clrl	r2
	clrl	r3
	clrl	r4
	clrl	r5
	movzwl	*$ISTACK,r6
	pushl	$0x83c00000
	movzwl	*$STARTAD,-(sp)
	rei
.data
msg2:	.asciz	"bad emt (%o) at pc: %o\n"
