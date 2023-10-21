/ C library -- wait1

/ pid = wait1(&status);
/
/ pid == -1 if error
/ status indicates fate of process, if given
/
/	this routine included for seperated I/D programs

.globl	_wait1, cerror

_wait1:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	tst	(sp)+
	jmp	cerror
1:
	mov	r1,*4(r5)	/ status return
	mov	(sp)+,r5
	rts	pc
