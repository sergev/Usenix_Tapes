/ low core

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
	trap; br7+4.		/ power fail
	trap; br7+5.		/ emulator trap
	trap; br7+6.		/ system entry

. = 40^.
.globl	start, dump
1:	jmp	start
	jmp	dump


. = 50^.	/ reserved for software switch register
	173000; 0

	trap; br7+15.		/ 54

. = 60^.
	klin; br4
	klou; br4

	trap; br7+15.		/ 70
	trap; br7+15.		/ 74

. = 100^.
	kwlp; br6
	kwlp; br6

	trap; br7+15.		/ 110

. = 114^.
	trap; br7+7.		/ 11/70 parity

	trap; br7+15.		/ 120
	trap; br7+15.		/ 124
	trap; br7+15.		/ 130
	trap; br7+15.		/ 134
	trap; br7+15.		/ 140
	trap; br7+15.		/ 144
	trap; br7+15.		/ 150
	trap; br7+15.		/ 154
	trap; br7+15.		/ 160
	trap; br7+15.		/ 164
	trap; br7+15.		/ 170
	trap; br7+15.		/ 174
	trap; br7+15.		/ 200
	trap; br7+15.		/ 204
	trap; br7+15.		/ 210
	trap; br7+15.		/ 214

. = 220^.
	rkio; br5

	trap; br7+15.		/ 224
	trap; br7+15.		/ 230
	trap; br7+15.		/ 234

. = 240^.
	trap; br7+7.		/ programmed interrupt
	trap; br7+8.		/ floating point
	trap; br7+9.		/ segmentation violation

	trap; br7+15.		/ 254
	trap; br7+15.		/ 260
	trap; br7+15.		/ 264
	trap; br7+15.		/ 270
	trap; br7+15.		/ 274

/ floating vectors
. = 300^.
	klin; br4+1.
	klou; br4+1.
	lxin; br4+1.
	lxin; br4+1.
	lxin; br4+0.
	lxin; br4+0.

	trap; br7+15.		/ 330
	trap; br7+15.		/ 334

. = 340^.
	dzin; br5+0.
	dzou; br5+0.
	dzin; br5+1.
	dzou; br5+1.

//////////////////////////////////////////////////////
/		interface code to C
//////////////////////////////////////////////////////

.globl	call, trap

.globl	_klrint
klin:	jsr	r0,call; _klrint
.globl	_klxint
klou:	jsr	r0,call; _klxint

.globl	_lxint
lxin:	jsr	r0,call; _lxint

.globl	_dzrint
dzin:	jsr	r0,call; _dzrint
.globl	_dzxint
dzou:	jsr	r0,call; _dzxint

.globl	_clock
kwlp:	jsr	r0,call; _clock


.globl	_rkintr
rkio:	jsr	r0,call; _rkintr
