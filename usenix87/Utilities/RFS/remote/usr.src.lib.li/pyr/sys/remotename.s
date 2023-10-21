#include "syscall.h"
	.globl cerror
	.globl _remotename
	.align 2
_remotename:
	###entry###
	movw	pr0,tr1
	movw	pr1,tr2
	movw	pr2,tr3
	movw	pr3,tr4
	movw	pr4,tr5
	movw	$/**/SYS_remotename, tr0
	callk $0
	cmpw	$0,tr0
	bne	err
	movw	tr1,pr0
	ret
err:
	jump cerror
