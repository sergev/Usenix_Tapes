/ low core
XSWAP = 1	/ run with primary and sec swap devs
POWERFAIL = 1	/ assemble powerfail code

br4 = 200
br5 = 240
br6 = 300
br7 = 340

. = 0^.
	br	1f
	4

/ trap vectors
	trap; br7+0.		/ bus error
	trap; br7+1.		/ illegal instruction
	trap; br7+2.		/ bpt-trace trap
	trap; br7+3.		/ iot trap
	.if	POWERFAIL
	.globl	pwrtrp
	pwrtrp; br7+4.
	.endif
	.if	POWERFAIL-1
	pfail; br7+4.		/ power fail
	.endif
	trap; br7+5.		/ emulator trap
	trap; br7+6.		/ system entry

. = 40^.
.globl	start, dump
1:	jmp	start
	jmp	dump


. = 50^.
.globl 	_proc,_praddr
_praddr: _proc

. = 52^.	/last char read from diagnostic tty port
.globl	_diagc
_diagc:	0

. = 60^.
	klin; br4
	klou; br4

/. = 70^.
/	pcin; br4
/	pcou; br4

. = 100^.
	kwlp; br6
	kwlp; br6

. = 114^.
	trap; br7+7.		/ 11/70 parity


. = 124^.
	daio; br6+0.		/ DA-11 DMA unibus link (DR-11b)

///////////////////////////////////////////////////////////////
/  UNIX low core pointers and variables                       /
/  vector space between 150 and 200 is currently unused by dec/
/  -ghg 1/1/77                                                /
///////////////////////////////////////////////////////////////

.globl	_rootdev, _swapdev, _swplo, _nswap, _stsp

. = 150^.			/ start of system pointers
_rootdev: .byte 0.,6.		/ major + minor of rootdev (6/0 hp0)
. = 152^.
_swapdev: .byte 0.,1.		/ major + minor dev of swap dev (1/0 sw)
. = 154^.
_swplo:	1.			/ first block of swap area (can't be 0)
. = 156^.
_nswap:	8489.			/ number of blocks in swap area

/ _rootdev to _nswap were moved from "/usr/sys/conf/conf.c"
/ to  low core so they can be patched at boot up time
/ incase of a hardware failure of the root/swap devices.
/ After boot-up, the cpu should be halted just after the 22-bit
/ (18-bit if 11/45) addressing light comes on.  The locations
/ should be patched up with the front panel switches and the
/ cpu continued.

	.if	XSWAP
.globl  _pswapdev, _sswapdev, _pswplo, _sswplo, _paddrx, _ptimx
. = 160^.
_pswapdev:	.byte 8.,5.	/* primary swap (5/8 hs0)
_sswapdev:	.byte 6,6	/ sec swap (6/6 hp6)
_pswplo:	1500.		/start of primary swap (cant be 0)
_sswplo:	1		/start of sec swap (can't be 0)
_paddrx:	547.		/first logical swap addr on sec swap dev
_ptimx:		45.		/idle time before staged to sec swp
	.endif
. = 174^.
.globl _pipedev
_pipedev:	.byte 8.,5.	/ pipe device
. = 176^.
_stsp:	0			/ pointer to system statistics block

////////////////////////////////////////////////////////////////////
/  end  of UNIX low core pointers and variables                    /
////////////////////////////////////////////////////////////////////

. = 200^.
	lpou; br5

. = 204^.
	hsio; br5

. = 214^.			/ DEC tapes
	tcio; br6

. = 224^.
	htio; br5

. = 240^.
	pirq; br7		/ programmed interrupt
	trap; br7+8.		/ floating point
	trap; br7+9.		/ segmentation violation

. = 254^.
	hpio; br5

/ floating vectors
. = 300^.
	emin; br4+0.		/ DM11-BB #0 (serviced by ep handler)
. = 304^.
	emin; br4+1.		/ DM11-BB #1 (serviced by ep handler)
. = 310^.
	emin; br4+2.		/ DM11-BB #2 (serviced by ep handler)
. = 320^.	/1st dh-11
	dhin; br5+0.		/ DH11-0 receive int
	dhou; br5+0.		/ DH11-0 xmit int

. = 330^.	/2nd dh-11
	dhin; br5+1.		/ DH11-1 receive interrupt
	dhou; br5+1.		/ DH11-1 xmit interrupt

. = 340^.	/3rd dh-11
	dhin; br5+2.		/ DH11-2 receive int
	dhou; br5+2.		/ DH11-2 xmit int

. = 350^.	/4th dh-11
	dhin; br5+3.		/ DH11-3 receive int
	dhou; br5+3.		/ DH11-3 xmit int

. = 360^.	/5th dh-11
	dhin; br5+4.		/ DH11-4 receive int
	dhou; br5+4.		/ DH11-4 xmit int

. = 370^.	/DMC-11 to Potter
	mpin; br5
	mpou; br5

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.globl	call, trap

.globl	_klrint
klin:	jsr	r0,call; _klrint
.globl	_klxint
klou:	jsr	r0,call; _klxint

.globl	_pirqint
pirq:	jsr	r0,call; _pirqint

/.globl	_pcrint
/pcin:	jsr	r0,call; _pcrint
/.globl	_pcpint
/pcou:	jsr	r0,call; _pcpint

.globl	_clock
kwlp:	jsr	r0,call; _clock


.globl	_lpint
lpou:	jsr	r0,call; _lpint

.globl	_hsintr
hsio:	jsr	r0,call; _hsintr

.globl	_htintr
htio:	jsr	r0,call; _htintr

.globl	_hpintr
hpio:	jsr	r0,call; _hpintr

.globl	_dhrint
dhin:	jsr	r0,call; _dhrint
.globl	_dhxint
dhou:	jsr	r0,call; _dhxint

.globl	_tcintr
tcio:	jsr	r0,call; _tcintr

.globl	_emintr		/Regular DM11-BB handler not used (no dialup)
emin:	jsr	r0,call; _emintr

.globl	_daintr
daio:	jsr	r0,call; _daintr

.globl	_mprintr, _mpxintr
mpin:	jsr	r0,call; _mpxintr
mpou:	jsr	r0,call; _mprintr

	.globl	pfail
pfail:			/Power failure - print crash message
	reset
	clr	r0	/wait awhile
	sob	r0,.
	clr	r0
	sob	r0,.
	clr	r0
	sob	r0,.
reset=5
halt=0
csxsr=177564
csbuf=177566		/Decwriter console address
	mov	$pfm,r0	/address of power fail message
pf1:	tstb	csxsr	/wait for DL-11 to go ready
	bpl	pf1	/not ready yet
	movb	(r0)+,csbuf	/stuff character to DL-11
	bne	pf1
	halt
	br	pfail

pfm:	<> / let decwriter warm up
	<\r\n\r\n PANIC CRASH! - Power Fail - Reboot and recover\r\n\0>
	.even
