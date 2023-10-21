/ C library -- empty: rand addition; returns 1 if pipe is empty, else 0.

/ error =  empty(file);

.globl  _empty, csv, cret, cerror

_empty:
	jsr     r5,csv
	mov     4(r5),r0
	sys	empty
	bec	1f
	jmp	cerror
1:
	jmp     cret
empty=63.
