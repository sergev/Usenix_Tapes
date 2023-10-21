/ C library -- sfork    (rand:rbg)

/ pid = fork();
/
/ pid == 0 in child process; pid == -1 means error return
/ in child, parents id is in par_uid if needed

	sfork = 62.

.globl  _sfork, cerror, _par_uid

_sfork:
	mov	r5,-(sp)
	mov	sp,r5
	sys     sfork
		br 1f
	bec	2f
	jmp	cerror
1:
	mov	r0,_par_uid
	clr	r0
2:
	mov	(sp)+,r5
	rts	pc
.bss
_par_uid: .=.+2
