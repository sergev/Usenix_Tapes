#include "syscall.h"
	.globl cerror
	.globl _remoteoff
	.align 2
_remoteoff:
	###entry###
	movw	pr0,tr1
	movw	$/**/SYS_remoteoff, tr0
	callk $0
	cmpw	$0,tr0
	bne	err
	movw	tr1,pr0
	ret
err:
	jump cerror
