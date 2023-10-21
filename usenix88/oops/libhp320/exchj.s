# Author:
#
#	Tim O'Konski
#	Software Engineering Lab, Bld. 26U
#	Hewlett-Packard
#	3500 Deer Creek Road
#	Palo Alto, CA 94304-1317
#	(415) 857-3999
#	uunet!hplabs!hpcea!hpcesea!tim

	text
	set	REGMASK,0x3ffe		# save d1-d7, a0-a5
	set	REGSAVE,64		# # words needed to save registers
					# Note this save space for all 16
					# registers -- we're not really
					# saving all of them.

# void Process::create(void** stack) -- Initialize a Process stack
#
# Called from inline Process::Process to initialize a stack for a new
# Process object.  First argument is address of Process object, second
# argument is base address of stack.
#
	global	__Process_create
__Process_create:
	link	%a6,&-REGSAVE	# reserve space on stack for saving registers
	movm.l	&REGMASK,(%sp)	# save registers
	mov.l	12(%a6),%a5	# a5 = stack base address
	addq.l	&4,%a5		# point to word just before stack
	mov.l	(%a6),%a2		# FP of derived class's constructor
	mov.l	(%a2),%a2		# FP of caller of derived class's constructor
	mov.l	%a2,%d7		# calculate # of words of stack to copy
	sub.l	%a7,%d7		# FP - SP of caller's caller
	asr.l	&2,%d7
	bra.b	L2
L1:				# this will copy enough of the current
	mov.l	-(%a2),-(%a5)	# stack to the new stack to be certain
L2:				# that the new Process will get the
	dbra	%d7,L1		# arguments it was created with.
	add.w	&REGSAVE,%a5	# point to frame link on new stack
	mov.l	%a5,%d7
	sub.l	%a6,%d7		# new FP - current FP
	add.l	%d7,(%a5)		# adjust frame link on new stack
	mov.l	8(%a6),%a0	# pointer to new Process instance
	mov.l	%a5,8(%a0)	# save new FP in this->saved_fp
	movm.l	-REGSAVE(%a6),&REGMASK	# restore registers
	unlk	%a6		# restore FP
	rts

# Process::exchj() -- Process exchange jump (co--routine call)
#
# Transfer control from the active process to a new process.
#
	text
	global	__Process_exchj
__Process_exchj:
	link	%a6,&-REGSAVE	# reserve space on stack for saving registers
	movm.l	&REGMASK,(%sp)	# save registers
	mov.l	8(%a6),%a5	# a5 = address of destination Process object
	mov.l	_scheduler+8,%a0	# a0 = address of active Process object
	mov.l	%a6,8(%a0)	# scheduler->previous_process->saved_fp = FP
	mov.l	8(%a5),%a6	# FP = this->saved_fp
	movm.l	-REGSAVE(%a6),&REGMASK	# restore registers
	unlk	%a6		# restore FP
	rts
