# exchj.s -- Machine-dependent co-routine support for OOPS processes
#
# Author:
#	K. E. Gorlen
#	Bg. 12A, Rm. 2017
#	Computer Systems Laboratory
#	Division of Computer Research and Technology
#	National Institutes of Health
#	Bethesda, Maryland 20892
#	Phone: (301) 496-5363
#	uucp: {decvax!}seismo!elsie!cecil!keith
#
# Function:
#	
# Process::create(void** stack) initializes the specified stack so that a
# subsequent call to Process::exchj() will start the new Process.
# Process::exchj() performs a coroutine call by saving all required
# registers, switching to a new Process stack, restoring the registers
# saved there, and returning to the new Process.
#
# Modification History:
#	
#
# C Stack Layout on the VAX:
#
#		arg n
#		...
#		arg 2
#		arg 1
# AP ->		# args
#		saved R11
#		...
#		saved R2
#		saved PC
#		saved FP
#		saved AP
#		saved mask/PSW
# FP ->		condition handler (0)
#		auto 1
#		auto 2
#		...
# SP ->		auto n
#
	.text
	.align	1
	.set	REGMASK,0xffc	# save r2 - r11

# void Process::create(void** stack) -- Initialize a Process stack
#
# Called from inline Process::Process to initialize a stack for a new
# Process object.  First argument is address of Process object, second
# argument is base address of stack.
#
	.globl	__Process_create
__Process_create:
	.word	REGMASK
	movl	8(ap),r5	# r5 = stack base address
	tstl	(r5)+		# point to word just before stack
	movl	8(fp),r2	# AP of derived class's constructor
	movzbl	(r2),r3		# no. of args to derived class's constructor
	moval	(r2)[r3],r4	# r4 = address of first argument
	tstl	(r4)+		# r4 = address before first argument
	subl3	fp,r4,r7	# calculate # words of stack to copy
	ashl	$-2,r7,r7
L1:			
	movl	-(r4),-(r5)	# copy arguments from current
	sobgtr	r7,L1		# stack to new stack
	subl3	fp,r5,r7	# new FP - current FP
	addl2	r7,8(r5)	# adjust saved AP on new stack
	addl2	r7,12(r5)	# adjust saved FP on new stack
	movl	4(ap),r6	# pointer to new Process instance
	movl	r5,8(r6)	# save new FP in this->saved_fp
	ret

# Process::exchj() -- Process exchange jump (co--routine call)
#
# Transfer control from the active process to a new process.
#
	.text
	.align	1
	.globl	__Process_exchj
__Process_exchj:
	.word	REGMASK
	movl	4(ap),r2	# r2 = address of destination Process object
	movl	_scheduler+8,r3	# r3 = address of active Process object
	movl	fp,8(r3)	# scheduler->previous_process->saved_fp = FP
	movl	8(r2),fp	# FP = this->saved_fp
	ret

