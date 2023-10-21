
|
| Gprolog 1.4/1.5
|
| Barry Brachman
| Dept. of Computer Science
| Univ. of British Columbia
| Vancouver, B.C. V6T 1W5
|
| .. {ihnp4!alberta, uw-beaver}!ubc-vision!ubc-cs!brachman
| brachman@cs.ubc.cdn
| brachman%ubc.csnet@csnet-relay.arpa
| brachman@ubc.csnet
|

|
| pushargs(func,argvec,arity,argtypes)
| int func();
| Mixed argvec[MAXARGS];
| int arity;
| char argtypes[MAXARGS];
|
| - checks elements in argvec against types found in argtypes
| - each element in the union argvec takes up 8 bytes
| - an element in argtypes indicates how many bytes the corresponding
|   element in argvec requires
|

.globl _pushargs

_pushargs:
	link	a6,#0		| a6 gets stacked
|
| First we get a pointer to the end of argvec
| so that the arguments can be stacked in the correct sequence
|
	movl	a6@(16),d0	| get arity
	movl	a6@(12),a0	| get address of argvec
	movl	d0,d1
	asll	#3,d1		| each entry always uses 8 bytes
	addl	d1,a0
|
| Now we stack the args, in the correct sequence
| and considering the size of each
|
	movl	a6@(20),a1	| get address of argtype vector
	addl	d0,a1		| this must also point to end of list!
	jra	1f
2:
	subql	#8,a0		| field width is 8 bytes
	movb	a1@-,d1		| get an element from argtype vector
	cmpb	#4,d1		| just 4 bytes?
	beq	3f
	cmpb	#8,d1
	beq	4f
	movb	a0@,sp@-	| push a character
	jra	1f
4:
	movl	a0@(4),sp@-	| assume 8 bytes for now
	movl	a0@(0),sp@-
	jra	1f
3:
	movl	a0@,sp@-	| push vector element
1:
	dbra	d0,2b

	movl	a6@(8),a0	| get address of func
	jsr	a0@		| call func
	unlk	a6
	rts
