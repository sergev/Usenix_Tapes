| exchj.s -- Machine-dependent co-routine support for OOPS processes
|
| Author:
|	K. E. Gorlen
|	Bg. 12A, Rm. 2017
|	Computer Systems Laboratory
|	Division of Computer Research and Technology
|	National Institutes of Health
|	Bethesda, Maryland 20892
|	Phone: (301) 496-5363
|	uucp: {decvax!}seismo!elsie!cecil!keith
|
| Function:
|	
| Process::create(void** stack) initializes the specified stack so that a
| subsequent call to Process::exchj() will start the new Process.
| Process::exchj() performs a coroutine call by saving all required
| registers, switching to a new Process stack, restoring the registers
| saved there, and returning to the new Process.
|
| Modification History:
|	
| NOTE: These functions currently do not save the floating point co-
| processor's registers.
|
| C Stack Layout on the MASSCOMP 500:
|
|		arg n
|		...
|		arg 2
|		arg 1
|		return link
|FP (a6) ->	frame link
|		auto 1
|		auto 2
|		...
|		auto n
|		saved reg n
|		...
|		saved reg 2
|SP (a7) ->	saved reg 1
|
	.text
REGMASK = 0x3ffe		| save d1-d7, a0-a5
REGSAVE = 64			| # words needed to save registers

| void Process::create(void** stack) -- Initialize a Process stack
|
| Called from inline Process::Process to initialize a stack for a new
| Process object.  First argument is address of Process object, second
| argument is base address of stack.
|
	.globl	__Process_create
__Process_create:
	link	a6,#-REGSAVE	| reserve space on stack for saving registers
	moveml	#REGMASK,a7@	| save registers
	movl	a6@(12),a5	| a5 = stack base address
	addql	#4,a5		| point to word just before stack
	movl	a6@,a2		| FP of derived class's constructor
	movl	a2@,a2		| FP of caller of derived class's constructor
	movl	a2,d7		| calculate # of words of stack to copy
	subl	a7,d7		| FP - SP of caller's caller
	asrl	#2,d7
	jra	L2
L1:				| this will copy enough of the current
	movl	a2@-,a5@-	| stack to the new stack to be certain
L2:				| that the new Process will get the
	dbra	d7,L1		| arguments it was created with.
	addw	#REGSAVE,a5	| point to frame link on new stack
	movl	a5,d7
	subl	a6,d7		| new FP - current FP
	addl	d7,a5@		| adjust frame link on new stack
	movl	a6@(8),a0	| pointer to new Process instance
	movl	a5,a0@(8)	| save new FP in this->saved_fp
	moveml	a6@(-REGSAVE),#REGMASK	| restore registers
	unlk	a6		| restore FP
	rts

| Process::exchj() -- Process exchange jump (co--routine call)
|
| Transfer control from the active process to a new process.
|
	.text
	.globl	__Process_exchj
__Process_exchj:
	link	a6,#-REGSAVE	| reserve space on stack for saving registers
	moveml	#REGMASK,a7@	| save registers
	movl	a6@(8),a5	| a5 = address of destination Process object
	movl	_scheduler+8,a0	| a0 = address of active Process object
	movl	a6,a0@(8)	| scheduler->previous_process->saved_fp = FP
	movl	a5@(8),a6	| FP = this->saved_fp
	moveml	a6@(-REGSAVE),#REGMASK	| restore registers
	unlk	a6		| restore FP
	rts
