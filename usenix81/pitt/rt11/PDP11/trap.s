/
/	RT11 EMULATOR
/	system call traps and
/	RT11 environment support
/
/	Daniel R. Strick
/	April, 1979
/	7/21/81  -- changed the ways in for a bunch of things get into
/		    the address space and declare their locations
/	7/21/81  -- defined "signal" to make this work under v7
/	10/15/81 -- put back code to restore traps before resuming rt11 program
/		    (which somehow got lost during VAXification)
/

SYSVEC	= 34	/ sys call trap vector
STARTAD	= 40	/ contains starting address of rt11 program
ISTACK	= 42	/ contains initial stack top for rt11 program
RMONAD	= 54	/ contains rt11 monitor address

/
/ EMT trapping for run time systems
/

.globl _emtrap, _embase
/.globl	_emtsig

SIGEMT	= 7	/ emulator trap signal number
rtt	= 6	/ return from trap with trace inhibited
signal	= 48.

_embase:			/ used to estimate bottom of emulator

/
/ invoke _emtsig to set the emulator trap
/
_emtsig:
	sys	signal; SIGEMT; emtrap
	bec	1f
	mov	$-1,r0
1:
	rts	pc
/
/ emulator traps begin here
/
emtrap:
	mov	r0,-(sp)	/ save user's registers
	mov	_rtstack,r0
	mov	(sp)+,-(r0)		/ his r0
	mov	r1,-(r0)		/ his r1
	mov	(sp)+,-(r0)		/ his pc
	mov	(sp)+,-(r0)		/ his ps
	mov	sp,-(r0)		/ his sp
	mov	r0,sp		/ exchange stacks
	jsr	pc,_emtrap	/ call: _emtrap(sp,ps,pc,r1,r0)
	jsr	pc,_emtsig	/ restore programmed request traps
	jsr	pc,_syssig
	mov	sp,r0		/ exchange stacks and restore his regs
	mov	(r0)+,sp		/ his sp
	mov	(r0)+,-(sp)		/ his ps
	mov	(r0)+,-(sp)		/ his pc
	mov	(r0)+,r1		/ his r1
	mov	(r0)+,r0		/ his r0

	tst	_comprtn	/ nonzero if completion routine pending
	beq	1f
	clr	_comprtn	/ make sure it is called only once
	mov	r0,-(sp)	/ save user's r0 and r1 in his stack
	mov	r1,-(sp)
	mov	_compcsw,r0	/ provide info to completion routine
	mov	_compchn,r1
	jsr	pc,*_comprtn
	mov	(sp)+,r1	/ restore user's r0 and r1
	mov	(sp)+,r0
1:
	rtt

///////////////////////////////////////
/
/ system call trapping for run-time systems
/

SIGTRP	= 18.	/ sys call trap signal number

/.globl	_syssig

_syssig:		/ set system call trap
	tst	_systset
	bne	1f
	sys	signal; SIGTRP; systrp
	inc	_systset
1:
	rts	pc

/
/ sys calls trap here
/

systrp:
	cmp	(sp),_embase
	blo	1f
	clr	_systset
	sub	$2,(sp)
	rtt
1:
	sys	signal; SIGTRP; systrp
	jmp	*SYSVEC

///////////////////////////////////////
/
/ begin the user program
/

.globl	_start

_start:
	jsr	pc,_emtsig
	jsr	pc,_syssig
	mov	sp,_rtstack
	mov	ISTACK,sp
	jmp	*STARTAD

.bss

.globl	_comprtn, _compcsw, _compchn

_rtstack: . = .+2	/ monitor stack top when user is running
_comprtn: . = .+2	/ completion routine address or zero
_compcsw: . = .+2	/ channel status word for completion routine
_compchn: . = .+2	/ number of channel on which I/O completed
_systset: . = .+2	/ nonzero if sys call trap set
