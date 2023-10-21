/ driver for SI Fujitsu 160Mb disk
/ using RM03 emulation

/hmcs1 = 176700
hmda  = 176706
/hmcs2 = 176710
hmdc  = 176734

SPT = 32.       / sectors per track
TPC = 10.       / tracks per cylinder

/preset = 021
/clear = 040

/first = .+2     / dirty, but i need the space
/        tst     $0
/        bne     1f
/        mov     $clear,*$hmcs2
/        mov     $preset,*$hmcs1
/        inc     first
/1:
	mov     dska,r1
	clr     r0
	div     $SPT,r0
	mov     r1,-(sp)
	mov     r0,r1
	clr     r0
	div     $TPC,r0
	bisb    r1,1(sp)
	mov     r0,*$hmdc
	mov     $hmda,r1
	mov     (sp)+,(r1)
	mov     ba,-(r1)
	mov     wc,-(r1)
	mov     $iocom,-(r1)
1:
	tstb    (r1)
	bpl     1b
	rts     pc
