/ C library -- alarm

/ time_left = alarm(time)

alarm = 27.

.globl _alarm

_alarm:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	alarm
	mov	(sp)+,r5
	rts	pc
