/ eofp -- C library    rand:bobg

/       eofp(fd)
/       int fd;

.globl  _eofp, cerror

eofp = 60.

_eofp:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4(r5),r0
	sys     eofp
	bec	1f
	jmp	cerror
1:      mov     (sp)+,r5
	rts     pc
