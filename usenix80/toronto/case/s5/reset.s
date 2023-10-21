/	setexit and reset, version ? from
/		unix newsletter (;login:) of aug. 1977
/	author Tom Duff NYIT

/ C library -- reset, setexit

/ reset(val)
/ will generate a "return" from
/ the last call to
/ setexit()
/ by restoring sp, r5 and doing a return.
/ "val" is returned to the caller of setexit.
/ setexit itself returns 0.
/
/ useful for going back to the main loop
/ after a horrible error in a lowlevel
/ routine.

.globl _setexit
.globl _reset
.globl csv,cret

_setexit:
	mov r5,sr5
	mov sp,ssp
	mov (sp),spc
	clr r0
	rts pc

_reset:
	jsr r5,csv
	mov 4(r5),r0
1:
	cmp (r5),sr5
	beq 1f
	mov (r5),r5
	bne 1b
/ panic -- r2 - r4 lost
	br 2f
1:
	mov -(r5),r4
	mov -(r5),r3
	mov -(r5),r2
2:
	mov sr5,r5
	mov ssp,sp
	mov spc,(sp)
	rts pc
.bss
sr5: .=.+2
spc: .=.+2
ssp: .=.+2
