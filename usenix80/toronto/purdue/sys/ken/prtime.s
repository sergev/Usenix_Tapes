.globl _prtime

_prtime:
	jsr	r5,csv
	mov	_time,r0	/get current time
	mov	_time+2,r1
	sub	$18000.,r1	/convert GMT to EST
	sbc	r0

	div	$28800.,r0	/break sec. into sec + 8-hr chunks
	mov	r0,r2		/save 8-hr chunks

	clr	r0	
	div	$60.,r0		/get seconds
	mov	r1,seconds

	mov	r0,r1
	clr	r0
	div	$60.,r0		/get min
	mov	r1,min

	mov	r0,hours	/r0 = hours (mod 8)
	clr	r0
	mov	r2,r1		/ get hr*8
	div	$3,r0	
	mul	$8.,r1		/ convert to hours
	add	r1,hours


	mov	4(r5),r0
	mov	hours,(r0)+
	mov	min,(r0)+
	mov	seconds,(r0)+
	jmp	cret

fmt:	<%d:%d:%d.  \0>

.bss
hours:	. = .+2
min:	. = .+2
seconds:. = .+2
.text
