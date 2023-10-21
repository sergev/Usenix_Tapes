/ C library -- pause

/ pause()

pause = 29.

.globl _pause, cerror

_pause:
	sys	pause
	rts	pc
