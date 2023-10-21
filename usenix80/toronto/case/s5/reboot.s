//C library -- reboot

// error = reboot(string)

reboot = 58.

.globl _reboot,cerror
_reboot:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	reboot
	jmp	cerror
