/	m40 -- machine language assist for PDP-11 without separate I & D space

/	last edit: 04-Apr-1981	D A Gwyn

/  1) installed FP11 support; set `.fpp' to enable;
/  2) installed vectors to "trap" at unused interrupt locations;
/  3) fixed FP11 load/store precision bug;
/  4) fixed backup for (rn)+ on 11/34;
/  5) upgraded "cret" to permit return of long value;
/  6) changed "call" to return ("mkconf" utility also changed), for pure code;
/  7) added light register display; set `.lights' to enable;
/  8) added support for MTPS/MFPS for 11/34 & 11/23;
/  9) "idle" re-enables the clock & console (IE bit dies due to hardware bug).

/ Set exactly one of the following CPU type flags to 1 and the others to 0:
.cpu23	= 0			/ LSI-11/23
.cpu34	= 1			/ PDP-11/34
.cpu40	= 0			/ PDP-11/40

/ Set to 1 each of the following hardware options you want supported:
.fpp	= 1			/ FP11 floating-point processor
				/ (if you set this, an FP11 must be present)
.lights	= 0			/ light register (only if display desired)
				/ (if you set this, lights must be present)

/ Note that a switch register is required only if you set `.lights';
/ a clock register (KW11-L or KW11-P) and memory parity registers are
/ tested for in "main".


/ PDP-11 instructions not understood by assembler:
mfpi	= 6500^tst		/ same as mfpd on these CPUs
mtpi	= 6600^tst		/ same as mtpd on these CPUs
.if .cpu23+.cpu34
mfps	= 106700^tst
mtps	= 106400^tst
.endif
.if .fpp
ldfps	= 170100^tst
stfps	= 170200^tst
.endif
wait	= 1
rtt	= 6
reset	= 5


/		interrupt linkage to C procedures


.globl	trap, call
.globl	_trap, _runrun, _swtch

/	trap -- linkage for internal-interrupt handler
/
/ execution begins with previous PSW & PC on Kernel stack, priority = 7

trap:
	mov	*$PS,-4(sp)	/ condition-code bits encode trap type
	tst	nofault		/ address of "err" during "fuiword", etc.
	bne	1f

	/ here if normal internal interrupt

	mov	*$SSR0,ssr	/ save MMU registers for instruction restart
	mov	*$SSR2,ssr+4
	mov	$1,*$SSR0	/ re-enable MMU
	jsr	r0,call1	/ call C "trap" routine via more linkage code
	jmp	_trap

	/ here if error during "fuiword", etc.
1:
	mov	$1,*$SSR0	/ re-enable MMU
	mov	nofault,(sp)	/ overwrite address from "gword"
	rtt			/ execute "err" instead of "gword"

call1:				/ r0 stacked for compatibility with "call"
	tst	-(sp)		/ new PSW already stacked
.if .cpu23+.cpu34
	mtps	$0
.endif
.if .cpu40
	bic	$340,*$PS	/ priority = 0 to permit "real" interrupts
.endif
	br	1f

/	call -- linkage for external-interrupt handlers
/
/	jsr	r0,call
/	jmp	_XXintr		/ invoke C procedure: "XXintr( subdev );"

call:
	mov	*$PS,-(sp)	/ condition-code bits encode subdevice

	/ common code for both "trap" and "call":
1:
	mov	r1,-(sp)	/ save Kernel R1
	mfpi	sp		/ save previous SP (in case prev = User)
	mov	4(sp),-(sp)
	bic	$!37,(sp)	/ isolate subdevice (or trap type)
	bit	$30000,*$PS	/ test previous mode
	beq	1f

	/ here if previous mode = User

.if .fpp
	/ The FPU runs asynchronously with the CPU but is not used by Kernel
	mov	$20,*$_u+4	/ FP-11 maint mode bit not used
.endif
	jsr	pc,(r0)		/ ... which will jump to C procedure

	/ here after C interrupt handler finishes
2:
.if .cpu23+.cpu34
	mtps	$340
.endif
.if .cpu40
	bis	$340,*$PS	/ priority = 7 (critical region)
.endif
	tstb	_runrun		/ test for higher-priority runnable process
	beq	2f

	/ here if another process should be run first

.if .cpu23+.cpu34
	mtps	$0
.endif
.if .cpu40
	bic	$340,*$PS	/ priority = 0 (end critical region)
.endif
.if .fpp
	jsr	pc,_savfp	/ save FPU registers
.endif
	jsr	pc,_swtch	/ switch processes
	br	2b		/ test again in locked critical region

	/ here with priority = 7 if current process is highest priority
2:
.if .fpp
	mov	$_u+4,r1	/ FPU save area for process
	bit	$20,(r1)
	bne	2f		/ test whether saved

	/ here if FPU state was saved

	mov	(r1)+,r0	/ previous FP status
	setd			/ force 64-bit load
	movf	(r1)+,fr0	/ restore floating registers
	movf	(r1)+,fr1
	movf	fr1,fr4
	movf	(r1)+,fr1
	movf	fr1,fr5
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	ldfps	r0		/ restore previous FP status
2:
.endif
	tst	(sp)+		/ pop "subdev" argument
	mtpi	sp		/ restore User SP
	br	2f

	/ here if previous mode = Kernel
1:
	bis	$30000,*$PS	/ pretend previous mode was User
	jsr	pc,(r0)		/ ... which will jump to C procedure

	/ here after C interrupt handler finishes

	cmp	(sp)+,(sp)+	/ pop "dev" argument and previous SP

	/ common code for both possible previous modes:
2:
	mov	(sp)+,r1	/ restore Kernel R1
	tst	(sp)+		/ pop new PSW
	mov	(sp)+,r0	/ restore Kernel R0
	rtt			/ restore previous PC & PSW, execute at least
				/ one instruction before trace trap can occur


/		floating-point processor register save routine


.globl	_savfp

/	savfp -- save FPU registers in per-process data area
/
/	savfp();

_savfp:
.if .fpp
	mov	$_u+4,r1	/ FPU save area for process
	bit	$20,(r1)
	beq	1f		/ test whether we are going to save state

	/ here if we were running in User mode when trap occurred

	stfps	(r1)+		/ save FPU status
	setd			/ force 64-bit store
	movf	fr0,(r1)+	/ save FPU data registers
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


/		light-register display routine


.globl	_display

.if .lights
CSW	=	177570
.bss
dispdly:.=.+2
.text
.endif

/	display -- display contents of address given in switch register
/		   if LSB set, displays User space
/
/	display();

_display:
.if .lights
	dec     dispdly
	bge     2f		/ if delaying after error, skip

	clr     dispdly
.if .cpu23+.cpu34
	mfps	-(sp)
	mtps	$340
.endif
.if .cpu40
	mov     *$PS,-(sp)	/ save priority
	mov     $340,*$PS	/ priority = 7 (critical region)
.endif
	mov	*$CSW,r1	/ get desired address
	bit     $1,r1
	beq     1f		/ see which address space is desired

	bis     $30000,*$PS	/ User mode
	dec     r1		/ make address even

1:
	jsr     pc,fuword	/ try to get data
	mov     r0,*$CSW	/ display result
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov     (sp)+,*$PS	/ restore priority (end critical region)
.endif
	cmp     r0,$-1
	bne     2f		/ if -1, assume access error

	mov     $120.,dispdly	/ 2-second delay after access error
2:
.endif
	rts     pc


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
	mfpi	(r1)
	inc	(sp)
	mtpi	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc


/		character-list manipulation routines


.globl	_getc, _putc
.globl	_cfreelist

/	getc -- get character from clist
/
/	(int)nextc = getc( &clist );	// ???

_getc:
	mov	2(sp),r1
.if .cpu23+.cpu34
	mfps	-(sp)
.endif
.if .cpu40
	mov	*$PS,-(sp)	/ save priority
.endif
	mov	r2,-(sp)
.if .cpu23+.cpu34
	mtps	$300
.endif
.if .cpu40
	bis	$340,*$PS	/ (critical region)
	bic	$40,*$PS	/ spl 6
.endif
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
3:
	mov	(sp)+,r2
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority (end critical region)
.endif
	rts	pc
9:
	clr	4(r1)
	mov	$-1,r0
	mov	(sp)+,r2
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority (end critical region)
.endif
	rts	pc

/	putc -- put character into clist
/
/	(int)status = putc( &clist );	// ???

_putc:
	mov	2(sp),r0
	mov	4(sp),r1
.if .cpu23+.cpu34
	mfps	-(sp)
.endif
.if .cpu40
	mov	*$PS,-(sp)	/ save priority
.endif
	mov	r2,-(sp)
	mov	r3,-(sp)
.if .cpu23+.cpu34
	mtps	$300
.endif
.if .cpu40
	bis	$340,*$PS	/ (critical region)
	bic	$40,*$PS	/ spl 6
.endif
	mov	4(r1),r2	/ last ptr
	bne	1f
	mov	_cfreelist,r2
	beq	9f
	mov	(r2),_cfreelist
	clr	(r2)+
	mov	r2,2(r1)	/ first ptr
	br	2f
1:
	bit	$7,r2
	bne	2f
	mov	_cfreelist,r3
	beq	9f
	mov	(r3),_cfreelist
	mov	r3,-10(r2)
	mov	r3,r2
	clr	(r2)+
2:
	movb	r0,(r2)+
	mov	r2,4(r1)
	inc	(r1)		/ count
	clr	r0
	mov	(sp)+,r3
	mov	(sp)+,r2
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority (end critical region)
.endif
	rts	pc
9:
	mov	pc,r0
	mov	(sp)+,r3
	mov	(sp)+,r2
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority (end critical region)
.endif
	rts	pc


.globl	_backup
.globl	_regloc
_backup:
	mov	2(sp),ssr+2
	mov	r2,-(sp)
	jsr	pc,backup
	mov	r2,ssr+2
	mov	(sp)+,r2
	movb	jflg,r0
	bne	2f
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

/ hard part - simulate the SSR1 register missing on these CPUs

backup:
	clr	r2		/ backup register ssr1
	mov	$1,bflg		/ clrs jflg
	mov	ssr+4,r0
	jsr	pc,fetch
	mov	r0,r1
	ash	$-11.,r0
	bic	$!36,r0
	jmp	*0f(r0)
.data
0:		t00; t01; t02; t03; t04; t05; t06; t07
		t10; t11; t12; t13; t14; t15; t16; t17
.text

t00:
	clrb	bflg

t10:
	mov	r1,r0
	swab	r0
	bic	$!16,r0
	jmp	*0f(r0)
.data
0:		u0; u1; u2; u3; u4; u5; u6; u7
.text

u6:	/ single op, m[tf]pi, sxt, illegal
	bit	$400,r1
	beq	u5		/ all but m[tf], sxt
	bit	$200,r1
	beq	1f		/ mfpi
	bit	$100,r1
	bne	u5		/ sxt

/ simulate mtpi with double (sp)+,dd
	bic	$4000,r1	/ turn instr into (sp)+
	br	t01

/ simulate mfpi with double ss,-(sp)
1:
	ash	$6,r1
	bis	$46,r1		/ -(sp)
	br	t01

u4:	/ jsr
	mov	r1,r0
	jsr	pc,setreg	/ assume no fault
	bis	$173000,r2	/ -2 from sp
	rts	pc

t07:	/ EIS
	clrb	bflg

u0:	/ jmp, swab
u5:	/ single op
	mov	r1,r0
	br	setreg

t01:	/ mov
t02:	/ cmp
t03:	/ bit
t04:	/ bic
t05:	/ bis
t06:	/ add
t16:	/ sub
	clrb	bflg

t11:	/ movb
t12:	/ cmpb
t13:	/ bitb
t14:	/ bicb
t15:	/ bisb
	mov	r1,r0
	ash	$-6,r0
	jsr	pc,setreg
	swab	r2
	mov	r1,r0
	jsr	pc,setreg

/ if delta(dest) is zero, no need to fetch source

	bit	$370,r2
	beq	1f

/ if mode(source) is R, no fault is possible

	bit	$7000,r1
	beq	1f

/ if reg(source) is reg(dest), too bad.

	mov	r2,-(sp)
	bic	$174370,(sp)
	cmpb	1(sp),(sp)+
	beq	t17

/ start source cycle
/ pick up value of reg

	mov	r1,r0
	ash	$-6,r0
	bic	$!7,r0
	movb	_regloc(r0),r0
	asl	r0
	add	ssr+2,r0
	mov	(r0),r0

/ if reg has been incremented, must decrement it before fetch

	bit	$174000,r2
	ble	2f
	dec	r0
	bit	$10000,r2
	beq	2f
	dec	r0
2:

/ if mode is 6,7 fetch and add X(R) to R

	bit	$4000,r1
	beq	2f
	bit	$2000,r1
	beq	2f
	mov	r0,-(sp)
	mov	ssr+4,r0
	add	$2,r0
	jsr	pc,fetch
	add	(sp)+,r0
2:

/ fetch operand
/ if mode is 3,5,7 fetch *

	jsr	pc,fetch
	bit	$1000,r1
	beq	1f
	bit	$6000,r1
	bne	fetch
1:
	rts	pc

t17:	/ illegal
u1:	/ br
u2:	/ br
u3:	/ br
u7:	/ illegal
	incb	jflg
	rts	pc

setreg:
	mov	r0,-(sp)
	bic	$!7,r0
	bis	r0,r2
	mov	(sp)+,r0
	ash	$-3,r0
	bic	$!7,r0
	movb	0f(r0),r0
	tstb	bflg
	beq	1f
	bit	$2,r2
	beq	2f
	bit	$4,r2
	beq	2f
1:
	cmp	r0,$20
	beq	2f
	cmp	r0,$-20
	beq	2f
	asl	r0
2:
	bisb	r0,r2
	rts	pc

/ It is hard to fix up mode 3 = @(rn)+ for an 11/34, so watch out!
/ At least this fixes mode 2 = (rn)+.  Credit to Clem Cole for the fix.
.if .cpu34
0:	.byte	0,0,0,20,-10,-20,0,0
.endif
.if .cpu23+.cpu40
0:	.byte	0,0,10,20,-10,-20,0,0
.endif

fetch:
	bic	$1,r0
	mov	nofault,-(sp)
	mov	$1f,nofault
	mfpi	(r0)
	mov	(sp)+,r0
	mov	(sp)+,nofault
	rts	pc

1:
 	mov	(sp)+,nofault
	clrb	r2			/ clear out dest on fault
	mov	$-1,r0
	rts	pc

.bss
bflg:	.=.+1
jflg:	.=.+1
.text


/		routines to fetch/store data in User I/D space


.globl	_fubyte, _subyte, _fuibyte, _suibyte
.globl	_fuword, _suword, _fuiword, _suiword

_fuibyte:
_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
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

/	fuiword -- fetch word from User I space
/	fuword -- fetch word from User D space
/
/	(int)contents = fuiword( (unsigned)address );
/	(int)contents = fuword( (unsigned)address );
/	returns -1 on error (bus timeout or segmentation violation)

_fuiword:
_fuword:			/ I & D space coincident on these CPUs
	mov	2(sp),r1	/ address
fuword:
	jsr	pc,gword
	rts	pc		/ return value or -1

gword:
.if .cpu23+.cpu34
	mfps	-(sp)
	mtps	$340
.endif
.if .cpu40
	mov	*$PS,-(sp)	/ save priority
	bis	$340,*$PS	/ priority = 7 (critical region)
.endif
	mov	nofault,-(sp)	/ save possibly useful current value
	mov	$err,nofault	/ used to switch to "err" in "trap"; see above
	mfpi	(r1)		/ try to get word from User I/D space;
				/ if this fails, "trap" is executed
	mov	(sp)+,r0	/ return value (`mfpi' succeeded)
	br	1f

_suiword:
_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

pword:
.if .cpu23+.cpu34
	mfps	-(sp)
	mtps	$340
.endif
.if .cpu40
	mov	*$PS,-(sp)	/ save priority
	bis	$340,*$PS	/ priority = 7 (critical region)
.endif
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)

	/ common code to restore and return from fetch/store U word routines:
1:
	mov	(sp)+,nofault	/ restore clobbered value
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority (end critical region)
.endif
	rts	pc		/ return (value in R0)

	/ here upon `rtt' in "trap" (implies that `mfpi' failed)
err:
	mov	(sp)+,nofault	/ restore clobbered value
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority (end critical region)
.endif
	tst	(sp)+		/ pop return to fetch/store routine itself
	mov	$-1,r0
	rts	pc		/ return -1 to indicate error


.globl	_copyin, _copyout
_copyin:
	jsr	pc,copsu
1:
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	2f

_copyout:
	jsr	pc,copsu
1:
	mov	(r0)+,-(sp)
	mtpi	(r1)+
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


/		idle routine


.globl	_idle, _waitloc
.globl	_lks

/	idle -- wait for external event
/
/	idle();			// returns after interrupt has been serviced

_idle:
.if .cpu23+.cpu34
	mfps	-(sp)
	mtps	$0
.endif
.if .cpu40
	mov	*$PS,-(sp)	/ save priority
	bic	$340,*$PS	/ priority = 0, to permit interrupts
.endif
	/ turn on interrupt-enable bits on flaky console & clock:
	/ (rumor has it that the problem results from static electricity)
	mov	$100,r0		/ interrupt enable bit
	mov	_lks,r1		/ is there a clock register?
	beq	1f

	bis	r0,(r1)		/ poke clock register
1:
	bis	r0,*$RCSR	/ poke console
	bis	r0,*$XCSR

	wait			/ prepare for interrupt
_waitloc:
.if .cpu23+.cpu34
	mtps	(sp)+
.endif
.if .cpu40
	mov	(sp)+,*$PS	/ restore priority
.endif
	rts	pc


.globl	_savu, _retu, _aretu
_savu:
.if .cpu23+.cpu34
	mtps	$340
.endif
.if .cpu40
	bis	$340,*$PS	/ priority = 7 (critical region)
.endif
	mov	(sp)+,r1
	mov	(sp),r0
	mov	sp,(r0)+
	mov	r5,(r0)+
.if .cpu23+.cpu34
	mtps	$0
.endif
.if .cpu40
	bic	$340,*$PS	/ priority = 0 (end critical region)
.endif
	jmp	(r1)

_aretu:
.if .cpu23+.cpu34
	mtps	$340
.endif
.if .cpu40
	bis	$340,*$PS	/ priority = 7 (critical region)
.endif
	mov	(sp)+,r1
	mov	(sp),r0
	br	1f

_retu:
.if .cpu23+.cpu34
	mtps	$340
.endif
.if .cpu40
	bis	$340,*$PS	/ priority = 7 (critical region)
.endif
	mov	(sp)+,r1
	mov	(sp),*$KISA6
	mov	$_u,r0
1:
	mov	(r0)+,sp
	mov	(r0)+,r5
.if .cpu23+.cpu34
	mtps	$0
.endif
.if .cpu40
	bic	$340,*$PS	/ priority = 0 (end critical region)
.endif
	jmp	(r1)


/		routines to set CPU priority level


.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7

/	spl? -- set CPU priority to level ?
/
/	spl?();			// where ? = one of: 0, 1, 4, 5, 6, 7

_spl0:
.if .cpu23+.cpu34
	mtps	$0
.endif
.if .cpu40
	bic	$340,*$PS
.endif
	rts	pc

_spl1:
.if .cpu23+.cpu34
	mtps	$40
.endif
.if .cpu40
	bis	$40,*$PS
	bic	$300,*$PS
.endif
	rts	pc

_spl4:
.if .cpu23+.cpu34
	mtps	$200
.endif
.if .cpu40
	bis	$200,*$PS
	bic	$140,*$PS
.endif
	rts	pc

_spl5:
.if .cpu23+.cpu34
	mtps	$240
.endif
.if .cpu40
	bis	$240,*$PS
	bic	$100,*$PS
.endif
	rts	pc

_spl6:
.if .cpu23+.cpu34
	mtps	$300
.endif
.if .cpu40
	bis	$300,*$PS
	bic	$40,*$PS
.endif
	rts	pc

_spl7:
.if .cpu23+.cpu34
	mtps	$340
.endif
.if .cpu40
	bis	$340,*$PS
.endif
	rts	pc


.globl	_copyseg
_copyseg:
	mov	*$PS,-(sp)	/ save PSW
	mov	*$UISA0,-(sp)
	mov	*$UISA1,-(sp)
	mov	$30340,*$PS
	mov	10(sp),*$UISA0
	mov	12(sp),*$UISA1
	mov	*$UISD0,-(sp)
	mov	*$UISD1,-(sp)
	mov	$6,*$UISD0
	mov	$6,*$UISD1
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2
1:
	mfpi	(r0)+
	mtpi	(r1)+
	sob	r2,1b
	mov	(sp)+,r2
	mov	(sp)+,*$UISD1
	mov	(sp)+,*$UISD0
	mov	(sp)+,*$UISA1
	mov	(sp)+,*$UISA0
	mov	(sp)+,*$PS	/ restore PSW
	rts	pc

.globl	_clearseg
_clearseg:
	mov	*$PS,-(sp)	/ save PSW
	mov	*$UISA0,-(sp)
	mov	$30340,*$PS
	mov	6(sp),*$UISA0
	mov	*$UISD0,-(sp)
	mov	$6,*$UISD0
	clr	r0
	mov	$32.,r1
1:
	clr	-(sp)
	mtpi	(r0)+
	sob	r1,1b
	mov	(sp)+,*$UISD0
	mov	(sp)+,*$UISA0
	mov	(sp)+,*$PS	/ restore PSW
	rts	pc


/		Support for double-precision arithmetic


.globl	_dpadd, _dpcmp

/	dpadd -- add int to long
/
/	dpadd( &long , int );

_dpadd:
	mov	2(sp),r0	/ -> MSW
	add	4(sp),2(r0)	/ add int to LSW
	adc	(r0)		/ possible carry to MSW
	rts	pc

/	dpcmp -- compare two longs
/
/	diff = dpcmp( longa , longb );	// returns (int)(longa - longb)
/					// clipped to [-512,+512]

_dpcmp:
	mov	2(sp),r0	/ longa MSW
	mov	4(sp),r1	/ longa LSW
	sub	6(sp),r0	/ subtract longb MSW
	sub	8(sp),r1	/ subtract longb LSW
	sbc	r0		/ possible borrow from MSW
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


/		Standalone core dump utility (start CPU @ 044)


.globl	dump

dump:
	bit	$1,*$SSR0
	bne	dump		/ loop if UNIX running

/ save regs r0,r1,r2,r3,r4,r5,r6,KIA6 starting at abs location 4

	mov	r0,*$4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	*$KISA6,(r0)

/ dump all of core (i.e., to first mt error) onto magtape.

	mov	$MTC,r0
	mov	$60004,(r0)+	/ 800bpi 9trk write
	clr	2(r0)		/ MTCMA (current memory address)
1:
	mov	$-512.,(r0)	/ MTBRC (byte record count)
	inc	-(r0)		/ "go"
2:
	tstb	(r0)
	bge	2b		/ wait for cu ready
	tst	(r0)+		/ error => done (memory fault)
	bge	1b		/ MTCMA points to next block already
	reset			/ clear error

	mov	$60007,-(r0)	/ write end of file
	br	.		/ loop while operation completes


/		UNIX startup from standalone boot


.globl	start
.globl	_end, _edata, _main

/	start -- start from scratch

start:
	bit	$1,*$SSR0
	bne	start		/ loop if UNIX already running
	reset

/ Set unused vectors to trap to system, in case of hardware glitch

	clr	r0		/ -> loc. 0 (force)
1:
	mov	$trap,(r0)	/ use "trap"
	mov	$340+15.,2(r0)	/ with priority 7, trap type 15
2:
	add	$4,r0		/ -> next higher slot
	cmp	r0,$1000	/ done?
	bhis	1f		/ yes

	tst	(r0)		/ something there?
	bne	2b		/ yes, try next
	tst	2(r0)		/ (for safety)
	bne	2b

	br	1b		/ no, set up catcher
1:

/ initialize systems segments

	mov	$KISA0,r0
	mov	$KISD0,r1
	mov	$200,r4
	clr	r2
	mov	$6,r3
1:
	mov	r2,(r0)+
	mov	$77406,(r1)+		/ 4k rw
	add	r4,r2
	sob	r3,1b

/ initialize user segment

	mov	$_end+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	mov	r2,(r0)+		/ ksr6 = sysu
	mov	$usize-1\<8|6,(r1)+

/ initialize io segment

	mov	$IO,(r0)+
	mov	$77406,(r1)+		/ rw 4k

/ get a sp and start segmentation

	mov	$_u+[usize*64.],sp
	inc	*$SSR0

/ clear bss

	mov	$_edata,r0
1:
	clr	(r0)+
	cmp	r0,$_end
	blo	1b

/ clear user block

	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,*$PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt


/		Support for unsigned arithmetic


.globl	_ldiv, _lrem, _lshift

/	ldiv -- divide unsigned by int
/
/	quot = ldiv( uns , int );	// returns int quotient

_ldiv:
	clr	r0		/ 0 MSW
	mov	2(sp),r1	/ uns LSW
	div	4(sp),r0	/ divide by int (quot in r0)
	rts	pc

/	lrem -- remainder of unsigned divided by int
/
/	rem = lrem( uns , int );	// returns int remainder

_lrem:
	clr	r0		/ 0 MSW
	mov	2(sp),r1	/ uns LSW
	div	4(sp),r0	/ divide by int (rem in r1)
	mov	r1,r0
	rts	pc

/	lshift -- shift long quantity int places
/
/	result = lshift( &long , int );	// returns (int)(long << int)

_lshift:
	mov	2(sp),r1	/ -> long MSW
	mov	(r1)+,r0	/ long MSW
	mov	(r1),r1		/ long LSW
	ashc	4(sp),r0	/ long-shift by int
	mov	r1,r0		/ LSW of result
	rts	pc


/		C procedure entry/exit linkage


.globl	csv, cret

/	csv -- save registers
/
/	jsr	r5,csv

csv:
	mov	r5,r0		/ return point from "csv"
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

/	cret -- restore registers and return
/
/	jmp	cret

cret:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2	/ this works on all PDP-11s (amazing!)
	mov	r5,sp
	mov	(sp)+,r5	/ restore old frame pointer
	rts	pc


.globl	_u
_u	= 140000		/ per-process data area (swaps with process)
usize	= 16.			/ size of _u in clicks

PS	= 177776
UISA1	= 177642
UISA0	= 177640
UISD1	= 177602
UISD0	= 177600
SSR2	= 177576
SSR0	= 177572
XCSR	= 177564		/ console transmitter CSR
RCSR	= 177560		/ console receiver CSR
MTC	= 172522		/ magtape control reg address
KISA6	= 172354
KISA0	= 172340
KISD0	= 172300
IO	= 7600


/		Machine-specific data


.data
.globl	_ka6, _cputype

_ka6:	KISA6			/ kernel segmentation reg addr for _u
_cputype:	40.		/ all these CPUs are similar

.bss
.globl	nofault, ssr

nofault:.=.+2			/ when non-zero, contains routine for "trap"
ssr:	.=.+6			/ storage for SSR0..2
