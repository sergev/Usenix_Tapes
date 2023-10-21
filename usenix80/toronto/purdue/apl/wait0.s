/ C library -- wait0

/ pid = wait0();
/
/ pid == -1 if error
/ no status is returned
/
/	this routine included for seperated I/D programs

.globl	_wait0, cerror

_wait0:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	tst	(sp)+
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
