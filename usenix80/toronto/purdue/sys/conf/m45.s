/ machine language assist
/ for 11/45 or 11/70 CPUs

.fpp = 1
.pump = 1	/ hooks for pump
STATS = 1	/ hooks for monitoring code in clock.c
XBUF  = 1	/ run with Large buffer pool out of kernel space
POWERFAIL = 1	/ recover from power failures - must have all core mem.
UMRfirst = 170300 /addr of first Unibus map reg to save on power fail
UMRcnt	= 16.	/number of UMR's*2 to save on powerfail.
MX	= 1	/ run with MX host

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
spl	= 230
ldfps	= 170100^tst
stfps	= 170200^tst
wait	= 1
halt	= 0
rtt	= 6
reset	= 5

HIPRI	= 300
HIGH	= 6

/ Mag tape dump
/ save registers in low core and
/ write all core onto mag tape.
/ entry is thru 44 abs

.data
.globl	dump
dump:
	bit	$1,SSR0
	bne	dump

/ save regs r0,r1,r2,r3,r4,r5,r6,KIA6
/ starting at abs location 4

	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	KDSA6,(r0)+

/ dump all of core (ie to first mt error)
/ onto mag tape. (9 track or 7 track 'binary')
/ this code is  for TU16 magtape, 9 track drive  -ghg 11/17/76

	mov	$MTC,r0
	clr	4(r0)		/zero buss address
	mov	$1300,32(r0)	/set 800 bpi odd parity
1:
	mov	$-256.,2(r0)	/set 256 word write
	mov	$-512.,6(r0)	/set byte  count
	movb	$61,(r0)	/start write
2:
	tstb	12(r0)		/wait for drdy in mtds
	bpl	2b
	bit	$40000,(r0)		/error, probably  nexm
	beq	1b		/no, get more
	reset
	mov	$27,(r0)	/write end of file
	br	.		/hang

.if	POWERFAIL
.globl	pwrtrp, _sureg, _powerup, pfail, _getuer, _setum
.globl savreg,fpsav,savsp,savr0,savka6
pwrtrp:				/power up/power down traps to here
	mov	r0,savr0
	mov	pfs,r0		/dispatch on state of machine
	rol	r0
	add	$pftab,r0
	jmp	*(r0)

pftab:
	pfsav			/ 0 - going down - save registers
	pfail			/ 1 - Fatal, regs not saved
	pfup			/ 2 - attempt to bring machine up
	pfrec			/ 3 - power fail during recovery

pfsav:				/ Power down - save registers
	mov	$1,pfs		/ state = power down in progress
	mov	$40,*$170710	/ ** KLUDGE TO KILL SI 
	mov	$savreg,r0	/ save CPU registers
	mov	savr0,(r0)+
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,savsp
	bis	$30000,PS	/set prev mode = user to get his sp
	mfpd	sp		/save user sp
	mov	(sp)+,(r0)+
	mov	KDSA5,(r0)+
	mov	SISA0,(r0)+	/ supervisor regs
	mov	SISA1,(r0)+
	mov	SISD0,(r0)+
	mov	SISD1,(r0)+
	mov	UISA0,(r0)+	/  dev/mem.c uses this one
	mov	UISD0,(r0)+	/   "	"	"
	mov	KDSA1,(r0)+	/ _sufet/m45.s uses this
	mov	KDSA6,savka6
	.if	.fpp
	mov	$fpsav,r0	/save floating point
	stfps	(r0)+
	movf	fr0,(r0)+
	movf	fr4,fr0
	movf	fr0,(r0)+
	movf	fr5,fr0
	movf	fr0,(r0)+
	movf	fr1,(r0)+
	movf	fr2,(r0)+
	movf	fr3,(r0)+
	.endif
	cmp	_cputype,$70.	/ save part of Unibus map if 11/70
	bne	pfsv4		/ must be 11/45
	mov	$umsav,r0
	mov	$UMRfirst,r1	/ first UMR to save
	mov	$UMRcnt,r2	/ number of UMR words to save
pfsv2:
	mov	(r1)+,(r0)+
	sob	r2,pfsv2

pfsv4:
	jsr	pc,*$_getuer	/save u.u_error
	mov	r0,saverr
	mov	$2,pfs		/power down register save complete
1:
	reset
	br	1b		/die

pfup:				/ power up recovery routine
	mov	$3,pfs		/ state = power up recovery in progress
	mov	$stk+2,sp	/ temp stack in case of another trap
	spl	6		/ allow another power fail trap.
	mov	$2000.,r0	/ wait for awhile to let things settle
1:
	reset
	sob	r0,1b

	jbr	startx		/ setup kernel segmentation registers

pfup1:
	mov	savka6,KDSA6
	mov	savsp,sp	/ setup real sp
	cmp	_cputype,$70.	/ load Unibus map regs if 11/70
	bne	pfup4		/ must be 11/45
	jsr	pc,*$_setum	/ set up "known" Unibus mapping regs if 70
	mov	$umsav,r0	/ reload saved Unibus map regs if 70
	mov	$UMRfirst,r1
	mov	$UMRcnt,r2
pfup3:
	mov	(r0)+,(r1)+
	sob	r2,pfup3

pfup4:
	mov	saverr,-(sp)	/ powerup restors u.u_error
	jsr	pc,*$_powerup	/ bring up devices
	tst	(sp)+
	jsr	pc,*$_sureg	/ set user segmentation registers
	.if	.fpp
	mov	$fpsav,r1	/ restore floating point registers
	mov	(r1)+,r0
	ldfps	r0
	movf	(r1)+,fr0
	movf	(r1)+,fr1
	movf	fr1,fr4
	movf	(r1)+,fr1
	movf	fr1,fr5
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	ldfps	r0

	.endif
	mov	$savreg+2,r0	/ load CPU registers
	mov	(r0)+,r1
	mov	(r0)+,r2
	mov	(r0)+,r3
	mov	(r0)+,r4
	mov	(r0)+,r5
	mov	(r0)+,-(sp)	/ load user sp
	bis	$30000,PS	/ set prev mode = user to store his sp
	mtpd	sp
	mov	(r0)+,KDSA5
	mov	(r0)+,SISA0
	mov	(r0)+,SISA1
	mov	(r0)+,SISD0
	mov	(r0)+,SISD1
	mov     (r0)+,UISA0     /  dev/mem.c uses this one
	mov     (r0)+,UISD0     /   "   "       "
	mov     (r0)+,KDSA1     / _sufet/m45.s uses this
	mov	savreg,r0
	spl	7
	clr	pfs		/ cross fingers and pray!
	rtt			/ here we go!

pfrec:				/ Power failure during power up recovery
	mov	$2,pfs		/ state = down with regs saved
	mov	$stk+2,sp	/ just in case
	mov	$1000.,r1
1:
	mov	$12525,r0	/ for lights display
	reset
	sob	r1,1b

/	If this loop ever terminates, then we are "lost". We
/	obviously got here during a power-up sequence instead of
/	a power-down sequence.
/	This is due to the way DEC hardware handles power fail traps,
/	both power up and power down trapping to 24.  The software
/	has to figure out whether it is going up or down.  If after
/	a power-up trap, another power fail trap occurs before
/	the first 6 instructions are executed, then the software 
/	"gets lost" since it did not correctly record the power-up.
/	In this case the code at "pfup" will execute and the machine
/	will die during the reset loop in pfup.  When the next power
/	up trap occures, the software will think a power-down is in
/	progress and come here to hang and die.  However if the 10 
/	second reset loop completes, it was obviously not a power down
/	trap and we will attempt to restart again.
/
	jmp	*24		/fake a power up trap

savreg:	.=.+30.
	.if	.fpp
fpsav:	.=.+50.
	.endif
umsav:	.=.+[UMRcnt*2]	/ Unibus map register save area on pwr fail
savsp:	0
savka6:	0
savr0:	0
saverr:	0			/save of u.u_error (clobbered by powerup)
.globl	pfs
pfs:	0			/ Machine state during powerfail
	.endif


.globl	start, _end, _edata, _etext, _main

/ 11/45 and 11/70 startup.
/ entry is thru 0 abs.
/ since core is shuffled,
/ this code can be executed but once

start:
	inc	$-1
	bne	.
	reset
	clr	PS

/ set KI0 to physical 0

startx:
	mov	$77406,r3
	mov	$KISA0,r0
	mov	$KISD0,r1
	clr	(r0)+
	mov	r3,(r1)+

/ set KI1-6 to eventual text resting place

	mov	$_end+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
.if	XBUF
	.globl	_xstart,_xend
	.globl	_bufsiz,_bufaddr
	.globl	_clsiz,_claddr	/clist is out there too.
	.globl	_prsiz,_praddr	/and the proc table
	.if	MX
	.globl	_ntsiz,_ntaddr
	.endif

	mov	r2,_xstart	/save start of extended area for main

	mov	r2,_bufaddr	/save for binit()
	add	_bufsiz,r2	/Allocate buffers after bss but
				/before text. bufsiz in words*32.
	mov	r2,_claddr	/allocate clist after buffers
	add	_clsiz,r2

	mov	r2,_praddr	/ throw in the proc table too
	add	_prsiz,r2

	.if	MX
	mov	r2,_ntaddr	/ and the network
	add	_ntsiz,r2

	.endif
	mov	r2,_xend	/ end of extended area for main() to clear
.endif
1:
	mov	r2,(r0)+
	mov	r3,(r1)+
	add	$200,r2
	cmp	r0,$KISA7
	blos	1b

/ set KI7 to IO seg for escape

	mov	$IO,-(r0)

/ set KD0-7 to physical

	mov	$KDSA0,r0
	mov	$KDSD0,r1
	clr	r2
1:
	mov	r2,(r0)+
	mov	r3,(r1)+
	add	$200,r2
	cmp	r0,$KDSA7
	blos	1b

/ initialization
/ get a temp (1-word) stack
/ turn on segmentation
/ copy text to I space
/ clear bss in D space

	mov	$stk+2,sp
	mov	$65,SSR3		/ 22-bit,  K+U sep
	bit	$20,SSR3
	beq	1f
	mov	$70.,_cputype
1:
	inc	SSR0
	.if	POWERFAIL
	tstb	pfs			/check for power fail restart
	bne	2f			/don't shovel text around
	.endif
	mov	$_etext,r0
	mov	$_edata,r1
	add	$_etext-8192.,r1
1:
	mov	-(r1),-(sp)
	mtpi	-(r0)
	cmp	r1,$_edata
	bhi	1b
1:
	clr	(r1)+
	cmp	r1,$_end
	blo	1b

/ use KI escape to set KD7 to IO seg
/ set KD6 to first available core

	.if	POWERFAIL
2:
	.endif
	mov	$IO,-(sp)
	mtpi	*$KDSA7
	mov	$_etext-8192.+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	add	KISA1,r2
	mov	r2,KDSA6
	.if	POWERFAIL
	tstb	pfs
	jne	pfup1			/ continue power fail recovery
	.endif

/ set up supervisor D registers

	mov	$6,SISD0
	mov	$6,SISD1

/ set up real sp
/ clear user block

	mov	$_u+[usize*64.],sp
	mov	$_u,r0
	mov	r0,SL
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
/	jsr	pc,_isprof

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt

.globl	trap, call
.globl	_trap

/ all traps and interrupts are
/ vectored thru this routine.

trap:
	mov	PS,saveps
	tst	sp	/ check red stack violation
	jeq	4f	/ death
	tst	nofault
	bne	1f
	mov	SSR0,ssr
	mov	SSR1,ssr+2
	mov	SSR2,ssr+4
	mov	$1,SSR0
	jsr	r0,call1; _trap
	/ no return
1:
	mov	$1,SSR0
	mov	nofault,(sp)
	rtt
.text

.globl	_runrun, _qswtch
.globl death
call1:
	mov	saveps,-(sp)
	cmp	$340,PS			/ Kernel mode bus error?
	beq	1f			/ dont drop cpu pri
	spl	0
	br	1f

call:
	mov	PS,-(sp)
1:
	mov	r1,-(sp)
	mfpd	sp
	mov	4(sp),-(sp)
	bic	$!37,(sp)
	bit	$30000,PS
	beq	1f
.if .fpp
	mov	$20,_u+4		/ FP maint mode
.endif
	cmp	sp,$_u+460	/o well.. check stack overflow
	bpl	3f
4:	mov	$stkmsg,r0
	jmp	death

3:
	jsr	pc,*(r0)+
2:
	spl	HIGH
	tstb	_runrun
	beq	2f
	spl	0
	jsr	pc,_savfp
	jsr	pc,_qswtch
	br	2b
2:
.if .fpp
	mov	$_u+4,r1
	bit	$20,(r1)
	bne	2f
	mov	(r1)+,r0
	ldfps	r0
	movf	(r1)+,fr0
	movf	(r1)+,fr1
	movf	fr1,fr4
	movf	(r1)+,fr1
	movf	fr1,fr5
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	ldfps	r0
2:
.endif
	tst	(sp)+
	mtpd	sp
	br	2f
1:
	bis	$30000,PS
	jsr	pc,*(r0)+
	cmp	(sp)+,(sp)+
2:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rtt

.data
stkmsg:	<Panic - Stack violation\r\n\0>
	.even
.text
.globl	_savfp
_savfp:
.if .fpp
	mov	$_u+4,r1
	bit	$20,(r1)
	beq	1f
	stfps	(r1)+
	movf	fr0,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
1:
.endif
	rts	pc

.globl	_incupc
_incupc:
	mov	r2,-(sp)
	mov	6(sp),r2	/ base of prof with base,leng,off,scale
	mov	4(sp),r0	/ pc
	sub	4(r2),r0	/ offset
	clc
	ror	r0
	mul	6(r2),r0	/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)	/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpd	(r1)
	inc	(sp)
	mtpd	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc

.globl	_display
_display:
	dec	dispdly
	bge	2f
	clr	dispdly
	mov	PS,-(sp)
	mov	$HIPRI,PS
	mov	CSW,r1
	bit	$1,r1
	beq	1f
	bis	$30000,PS
	dec	r1
1:
	jsr	pc,fuword
	mov	r0,CSW
	mov	(sp)+,PS
	cmp	r0,$-1
	bne	2f
	mov	$120.,dispdly		/ 2 sec delay after CSW fault
2:
	rts	pc

/ Character list get/put

.globl	_getc, _putc
.globl	_cfreelist
.globl	_ccount
.globl	_cblkct

_getc:
	mov	PS,-(sp)
	spl	6
	mov	4(sp),r1
	.if	XBUF
	mov	KDSA5,-(sp)
	mov	_claddr,KDSA5	/map ka5 to point to clist
	.endif
	mov	r2,-(sp)
	mov	2(r1),r2	/ first ptr
	beq	9f		/ empty
	movb	(r2)+,r0	/ character
	bic	$!377,r0
	mov	r2,2(r1)
	dec	(r1)+		/ count
	bne	1f
	clr	(r1)+
	clr	(r1)+		/ last block
	br	2f
1:
	bit	$7,r2
	bne	3f
	mov	-10(r2),(r1)	/ next block
	add	$2,(r1)
2:
	dec	r2
	bic	$7,r2
	mov	_cfreelist,(r2)
	mov	r2,_cfreelist
/ stats -ghg
	dec	_cblkct
/......................
3:
/ statistics code -ghg
	dec	_ccount
/ ................
	mov	(sp)+,r2
	.if	XBUF
	mov	(sp)+,KDSA5	/restore to previous mapping
	.endif
	mov	(sp)+,PS
	rts	pc
9:
	clr	4(r1)
	mov	$-1,r0
	mov	(sp)+,r2
	.if	XBUF
	mov	(sp)+,KDSA5	/restore to previous mapping
	.endif
	mov	(sp)+,PS
	rts	pc

_putc:
	mov	PS,-(sp)
	spl	6
	mov	4(sp),r0
	mov	6(sp),r1
	.if	XBUF
	mov	KDSA5,-(sp)
	mov	_claddr,KDSA5	/map ka5 to point to clist
	.endif
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	4(r1),r2	/ last ptr
	bne	1f
	mov	_cfreelist,r2
	beq	9f
	mov	(r2),_cfreelist
/ statistics -ghg
	inc	_cblkct
/ ..................................
	clr	(r2)+
	mov	r2,2(r1)	/ first ptr
	br	2f
1:
	bit	$7,r2
	bne	2f
	mov	_cfreelist,r3
	beq	9f
	mov	(r3),_cfreelist
/ stats -ghg
	inc	_cblkct
/ ...............................
	mov	r3,-10(r2)
	mov	r3,r2
	clr	(r2)+
2:
	movb	r0,(r2)+
	mov	r2,4(r1)
	inc	(r1)		/ count
/ statistics -ghg
	inc	_ccount		/ count of active chars in clist
/ ...................
	clr	r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	.if	XBUF
	mov	(sp)+,KDSA5	/restore to previous mapping
	.endif
	mov	(sp)+,PS
	rts	pc
9:
	inc	_coverf
	mov	pc,r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	.if	XBUF
	mov	(sp)+,KDSA5	/restore to previous mapping
	.endif
	mov	(sp)+,PS
	rts	pc

.globl	_backup
.globl	_regloc
_backup:
	mov	2(sp),r0
	movb	ssr+2,r1
	jsr	pc,1f
	movb	ssr+3,r1
	jsr	pc,1f
	movb	_regloc+7,r1
	asl	r1
	add	r0,r1
	mov	ssr+4,(r1)
	clr	r0
2:
	rts	pc
1:
	mov	r1,-(sp)
	asr	(sp)
	asr	(sp)
	asr	(sp)
	bic	$!7,r1
	movb	_regloc(r1),r1
	asl	r1
	add	r0,r1
	sub	(sp)+,(r1)
	rts	pc


.globl	_fubyte, _subyte
.globl	_fuword, _suword
.globl	_fuibyte, _suibyte
.globl	_fuiword, _suiword
_fuibyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	br	2f

_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword

2:
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,piword
	clr	r0
	rts	pc

_subyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,pword
	clr	r0
	rts	pc

_fuiword:
	mov	2(sp),r1
fuiword:
	jsr	pc,giword
	rts	pc

_fuword:
	mov	2(sp),r1
fuword:
	jsr	pc,gword
	rts	pc

giword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpi	(r1)
	mov	(sp)+,r0
	br	1f

gword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpd	(r1)
	mov	(sp)+,r0
	br	1f

_suiword:
	mov	2(sp),r1
	mov	4(sp),r0
suiword:
	jsr	pc,piword
	rts	pc

_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

piword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
	br	1f

pword:
	mov	PS,-(sp)
	spl	HIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpd	(r1)
1:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	rts	pc

err:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	tst	(sp)+
	mov	$-1,r0
	rts	pc

.if	.pump

/ support code for nonblocking read to user buffer at interrupt time

	.globl	_sufet
_sufet:
	mov	6(sp),r0	/ fet offset from PAR
	add	$20000,r0	/ map through PAR 1
	mov	r0,r1
	mov	PS,-(sp)	/ must lock out all interrupts
	mov	KDSA1,-(sp)	/ save Kernel I PAR 1
	spl	7
	mov	10(sp),KDSA1	/ point PAR 1 to user buffer
	add	(r0),r1		/ r1 has addr of next input char
	inc	(r0)		/ advance user buffer pointer (in)
	movb	6(sp),(r1)	/ store character
	cmp	(r0)+,(r0)	/ check for buffer wrap around
	bne	2f		/ no wrap
	mov	$6,-2(r0)	/ set in = start of buffer
2:
	cmp	-(r0),4(r0)	/ return true if buffer overrun
	beq	3f
	clr	r0
3:
	mov	(sp)+,KDSA1	/ restore Mem-Mgmnt PAR 1
	mov	(sp)+,PS
	rts	pc

.endif
.globl	_copyin, _copyout
.globl	_copyiin, _copyiout
_copyiin:
	jsr	pc,copsu
1:
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyin:
	jsr	pc,copsu
1:
	mfpd	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyiout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpi	(r1)+
	sob	r2,1b
	br	2f

_copyout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpd	(r1)+
	sob	r2,1b
2:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	clr	r0
	rts	pc

copsu:
	mov	(sp)+,r0
	mov	r2,-(sp)
	mov	nofault,-(sp)
	mov	r0,-(sp)
	mov	10(sp),r0
	mov	12(sp),r1
	mov	14(sp),r2
	asr	r2
	mov	$1f,nofault
	rts	pc

1:
	mov	(sp)+,nofault
	mov	(sp)+,r2
	mov	$-1,r0
	rts	pc

.globl	_idle
_idle:
	mov	PS,-(sp)
	spl	0
/ ************ kludge for SI drive testing ********************
	cmp	*$177570,$1	/ SIDUMP() if swr == 1
	bne	2f
.globl _sidump
	jsr	pc,*$_sidump
2:
/***************************** end kludge **********************
	wait
	.if	STATS
.globl	_waitloc
_waitloc:
	.endif
	mov	(sp)+,PS
	rts	pc

.globl	_halt
_halt:
	halt
	rts	pc

.globl  _savu, _retu, _aretu, _panic
_savu:
	spl	HIGH
	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
	spl	0
	jmp	(r1)

_aretu:
	spl	7
	mov	(sp)+,r1
	mov	(sp),r0
	br	1f

_retu:
	spl	7
	mov	(sp)+,r1
	mov	(sp),KDSA6
	mov	$_u,r0
1:
	cmp     (r0),$_u        /check for fishy looking new sp
	blos    2f
	cmp     (r0),$_u+[usize*64.]
	bhi     2f
	mov	(r0)+,sp
	mov	(r0)+,r5
	spl	0
	jmp	(r1)

2:      mov     $crmsg,-(sp)
	jsr     pc,*$_panic     /bye....
.data
crmsg:  <swtch() to trashed proc\n\0>; .even
.text

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7
_spl0:
	spl	0
	rts	pc

_spl1:
	spl	1
	rts	pc

_spl4:
	spl	4
	rts	pc

_spl5:
	spl	5
	rts	pc

_spl6:
	spl	6
	rts	pc

_spl7:
	spl	HIGH
	rts	pc

.globl	_copyseg
_copyseg:
	mov	PS,-(sp)
	mov	4(sp),SISA0
	mov	6(sp),SISA1
	mov	$10000+HIPRI,PS
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2
1:
	mfpd	(r0)+
	mtpd	(r1)+
	sob	r2,1b
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

.globl	_clearseg
_clearseg:
	mov	PS,-(sp)
	mov	4(sp),SISA0
	mov	$10000+HIPRI,PS
	clr	r0
	mov	$32.,r1
1:
	clr	-(sp)
	mtpd	(r0)+
	sob	r1,1b
	mov	(sp)+,PS
	rts	pc

.globl	_dpadd
_dpadd:
	mov	2(sp),r0
	add	4(sp),2(r0)
	adc	(r0)
	rts	pc

.globl	_dpadd2	/same as above, but words reversed (for mapalloc())
_dpadd2:
	mov	2(sp),r0
	add	4(sp),(r0)
	adc	2(r0)
	rts	pc

.globl	_dpcmp
_dpcmp:
	mov	2(sp),r0
	mov	4(sp),r1
	sub	6(sp),r0
	sub	8(sp),r1
	sbc	r0
	bge	1f
	cmp	r0,$-1
	bne	2f
	cmp	r1,$-512.
	bhi	3f
2:
	mov	$-512.,r0
	rts	pc
1:
	bne	2f
	cmp	r1,$512.
	blo	3f
2:
	mov	$512.,r1
3:
	mov	r1,r0
	rts	pc

.globl	_dpdiv
_dpdiv:
	mov	2(sp),r0
	mov	4(sp),r1
	div	6(sp),r0
	rts	pc

.globl	_dprem
_dprem:
	mov	2(sp),r0
	mov	4(sp),r1
	div	6(sp),r0
	mov	r1,r0
	rts	pc

.globl	_ldiv
_ldiv:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	rts	pc

.globl	_lrem
_lrem:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	mov	r1,r0
	rts	pc

.globl	_lshift
_lshift:
	mov	2(sp),r1
	mov	(r1)+,r0
	mov	(r1),r1
	ashc	4(sp),r0
	mov	r1,r0
	rts	pc

.globl	csv
csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

.globl cret
cret:
	mov	r5,r1
	mov	-(r1),r4
	mov	-(r1),r3
	mov	-(r1),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

.if	XBUF
.globl	_b
_b	= 120000
.globl	_cfree
_cfree = 120000		/clist now mapped like extended buffers
.globl	_proc
_proc	= 120000	/throw in the  proc table too
			/it can have max length of 4096-32 words.
	.if	MX
	.globl	_ntbuf
_ntbuf	= 120000	/Network buffer pool
	.endif
.endif

.globl	_u
_u	= 140000
usize	= 16.

CSW	= 177570
PS	= 177776
SL	= 177774
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA7	= 172356
KISD0	= 172300
KDSA0	= 172360
KDSA1	= 172362
KDSA5	= 172372
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
MTC	=	172440		/TU16 csr
SISA0	= 172240
SISA1	= 172242
SISD0	= 172200
SISD1	= 172202
IO	= 177600
UISA0	= 177640
UISD0	= 177600

.data
.globl	_ka6
.globl	_cputype

_ka6:	KDSA6
_cputype:45.
stk:	0
.if	XBUF
.globl	_ka5
_ka5:	KDSA5
_bufaddr:	0
_claddr:	0	/force to Data instead of Bss.
/_praddr:	0	/addr of proc table (in low.s at addr 50)

	.if	MX
_ntaddr:	0	/start addr/64 of net buffers
	.endif

_xstart:	0	/start addr/64 of extended area
_xend:		0	/end of extended area (main() clears it)
.endif

.bss
.globl	nofault, ssr, _coverf
nofault:.=.+2
ssr:	.=.+6
dispdly:.=.+2
saveps:	.=.+2
_coverf: .=.+2

.text
/ system profiler
/
/rtt	= 6
/CCSB	= 172542
/CCSR	= 172540
/PS	= 177776
/
/.globl	_sprof, _xprobuf, _probuf, _probsiz, _mode
/_probsiz = 7500.+2048.
/
/_isprof:
/	mov	$_sprof,104	/ interrupt
/	mov	$340,106	/ pri
/	mov	$100.,CCSB	/ count set = 100
/	mov	$113,CCSR	/ count down, 10kHz, repeat
/	rts	pc
/
/_sprof:
/	mov	r0,-(sp)
/	mov	PS,r0
/	ash	$-11.,r0
/	bic	$!14,r0
/	add	$1,_mode+2(r0)
/	adc	_mode(r0)
/	cmp	r0,$14		/ user
/	beq	done
/	mov	2(sp),r0	/ pc
/	asr	r0
/	asr	r0
/	bic	$140001,r0
/	cmp	r0,$_probsiz
/	blo	1f
/	inc	_outside
/	br	done
/1:
/	inc	_probuf(r0)
/	bne	done
/	mov	r1,-(sp)
/	mov	$_xprobuf,r1
/2:
/	cmp	(r1)+,r0
/	bne	3f
/	inc	(r1)
/	br	4f
/3:
/	tst	(r1)+
/	bne	2b
/	sub	$4,r1
/	mov	r0,(r1)+
/	mov	$1,(r1)+
/4:
/	mov	(sp)+,r1
/done:
/	mov	(sp)+,r0
/	mov	$113,CCSR
/	rtt
/
/.bss
/_xprobuf:	.=.+512.
/_probuf:.=.+_probsiz
/_mode:	.=.+16.
/_outside: .=.+2
