/       _envsave saves an environment pointer (actually a ptr into the
/               stack in envir.
/       _envrest restores the stack pointer, and registers r5-r2 to their
/               values at the time of the call to _envsave.
/
/
/                       C calling sequence:
/
/                         envsave(&envir);                                   /
/                         int envir;
/                              .
/                              .
/                              .
/                         envrest(&envir);
/
/
/       After calls to _envsave or _envrest, the stack appears as follows:
/
/               sp ->   xxx (uninitialized)
/                       saved r2
/                       saved r3
/                       saved r4
/            envir ->   saved r5
/                       address of location following _envsave call
/                       address of argument (&envir)
/
/
.globl  _envsave
.globl  _envrest
.globl  csv

_envsave:
	jsr r5,csv      /save regs 5 (by jsr), 4, 3, and 2
	mov r5,*4(r5)   /pass environment ptr to caller in arg1
	mov (r5),r5     /restore the caller's r5, leaving regs on stack
	mov $1,r0       /return 1
	jmp *10.(sp)

_envrest:
	mov *2(sp),r1   /fetch environment pointer
	mov (r1),r5     /restore registers as before _envsave call
	mov -(r1),r4
	mov -(r1),r3
	mov -(r1),r2
	tst -(r1)
	mov r1,sp       /stack pointer after _envsave
	clr r0          /return 0
	jmp *10.(sp)
