/ RT simulator, low core module 1
/       two modules req'd to circumvent assembler bug
RMON= 0134000
PCLOC=040
SPLOC=042

	.globl _main, start

.= .+54000
start:  mov     sp,r0
	mov     (r0),-(sp)
	tst     (r0)+
	mov     r0,2(sp)
	jsr     pc,_main        / load .SAV file and catch EMTs
	mov     SPLOC,sp
	jmp     *PCLOC

	.globl _sclear
_sclear:                        / sclear(nwds, stk_arg_loc) : clear off stack args
	mov     2(sp),r0
	asl     r0
	mov     4(sp),r1
	add     r1,r0
1:      mov     -(r1),-(r0)
	cmp     r1,sp
	bhi     1b
	sub     r1,r0
	add     r0,sp
	add     r0,r5
	rts     pc

	.globl _sstuff
_sstuff:                        / sstuff(nwds, stk_arg_loc, buf) : return data on stack
	mov     2(sp),r0
	asl     r0
	mov     4(sp),r2
	mov     sp,r1
	sub     r0,sp
	mov     sp,r0
1:      mov     (r1)+,(r0)+
	cmp     r1,r2
	blo     1b
	sub     r0,r1
	sub     r1,r5
	asr     r1
	mov     6(sp),r2
2:      mov     (r2)+,(r0)+
	sob     r1,2b
	rts     pc

	.globl  _ladd
_ladd:                          / longadd(hi,lo,lp) : add to *lp
	mov     6(sp),r0
	add     2(sp),(r0)+
	add     4(sp),(r0)
	adc     -(r0)
	rts     pc


.= start+266
	RMON; -1; 0; 0; 1003

.= start+314
	-1

.= start+374
	0
