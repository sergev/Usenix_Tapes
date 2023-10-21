/
/ inverted text-data / stack initialization routine
/
/ This initialization routine creates a full 32K word address space
/ by expanding the stack to a full 4K word segment and extending
/ the data segment break to meet the bottom of the stack.
/
/ The data segement locations above this initialization routine and
/ below _edata are copied into the stack.  Addresses are modified as
/ required for the code to be executed in its new location.
/ The stack is repositioned below the moved code and initialized data.
/
/ The argument count and pointer list address are stacked as required
/ by a standard C language main program which is then called as a
/ subroutine by the initialization code.  If the main routine returns,
/ the returned value is passed to the system with the exit system call.
/
/ The address relocation procedure requires that the relocation bits
/ in the executable file are retained and appended to the data segment
/ (the a.out file is not quite standard).
/
/ Daniel R. Strick
/ April 1979
/	7/21/81 -- now sets up the addresses of things like the VAX version
/	7/21/81 -- defined exit to make this run under v7
/
TOP4K	= -[4 * 2048.]
SINCR	= 20. * 64.		/ This may vary from system to system
exit	= 1

.globl start, _edata, _end, _main, _brk, _sbrk

start:
	setd
	mov	sp,r4		/ grow stack segment to exactly 4K word
	mov	$TOP4K+SINCR,sp
	tst	(sp)
	mov	r4,sp

	mov	$TOP4K,-(sp)	/ extend data segment to stack segment
	jsr	pc,_brk
	tst	(sp)+

	mov	$_end,r2
	mov	$_edata,r1
1:
	clr	-(sp)		/ clear new bss
	tst	-(r2)
	cmp	r2,r1
	bhi	1b

	mov	r1,r2		/ locate top of relocation bits
	asl	r2
	mov	sp,r3		/ compute relocation constant
	sub	r1,r3
1:
	mov	-(r1),-(sp)	/ copy text+data into stack
	mov	-(r2),r0
	bic	$!7,r0		/ isolate relocation bits
	beq	3f
	cmp	$1,r0
	bne	2f
	sub	r3,(sp)		/ pc relative ref to absolute info
2:
	bit	$1,r0
	bne	3f
	add	r3,(sp)		/ absolute ref to relocated info
3:
	cmp	r1,$2f
	bhi	1b

	clr	-(sp)		/ fixup the data segment break address
	jsr	pc,_sbrk
	mov	r0,(sp)
	add	$_brk,r3
	jsr	pc,*r3
/	tst	(sp)+

/	tst	-(sp)		/ set up arg count and vector for main program
	mov	(r4),-(sp)
	tst	(r4)+
	mov	r4,2(sp)
	jmp	4(sp)
2:
	jsr	pc,_main	/ invoke main program
	sys	exit
