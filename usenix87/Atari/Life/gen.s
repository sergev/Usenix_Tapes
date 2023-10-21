*
* Fast monochrome life for Atari ST.
*
* Written by
*   Jan Gray
*   300 Regina St. N Apt. 2-905
*   Waterloo, Ontario
*   N2J 4H2
*   Canada
*   (jsgray@watrose.UUCP)
*
* Copyright (C) 1987 Jan Gray.
* This program may be freely redistributed if this notice is retained.
*

*
* gen -- compute the next generation of cells
*
* This code is a transliteration of the the pdp-11 code presented in
* "Life Algorithms" by Mark Niemiec, Byte, 4:1, January 1979.
*
* gen currently does about 400,000 cell-generations/second.  It can go faster.
*
* Forgive the hardwired-in constants, and the terse comments.  It's a hack.
* Think of the fun you'll have figuring out how it works.
*
.globl _gen
.text
_gen:
	movem.l	d0-d7/a0-a2,-(sp)

	move.l	_Screen,a0
	move.l	a0,a2
	add.l	#80,a0
	add.l	#31920,a2
	move.l	#_NextScreen+80,a1

* 1 2 3
* 7 * 8
* 4 5 6

* (d1,d0) = neighbours 1+2
again:
	move.l	-80(a0),d0
	move.l	d0,d1
	move.l	d0,d2
	move.b	-81(a0),d7
	roxr.b	#1,d7
	roxr.l	#1,d0
	eor.l	d1,d0
	or.l	d0,d1
	eor.l	d0,d1

* (d1,d0) = neighbours 1+2+3

	move.b	-76(a0),d7
	roxl.b	#1,d7
	roxl.l	#1,d2
	eor.l	d2,d0
	or.l	d0,d2
	eor.l	d0,d2
	or.l	d2,d1

* (d3,d2) = neighbours 4+5

	move.l	80(a0),d2
	move.l	d2,d3
	move.l	d2,d4
	move.b	79(a0),d7
	roxr.b	#1,d7
	roxr.l	#1,d2
	eor.l	d3,d2
	or.l	d2,d3
	eor.l	d2,d3

* (d3,d2) = neighbours 4+5+6

	move.b	84(a0),d7
	roxl.b	#1,d7
	roxl.l	#1,d4
	eor.l	d4,d2
	or.l	d2,d4
	eor.l	d2,d4
	or.l	d4,d3

* (d2,d1,d0) = neighbours 1+2+3+4+5+6

	eor.l	d2,d0
	or.l	d0,d2
	eor.l	d0,d2
	eor.l	d2,d1
	or.l	d1,d2
	eor.l	d1,d2
	eor.l	d3,d1
	or.l	d1,d3
	eor.l	d1,d3
	or.l	d3,d2

* (d4,d3) = neighbours 7+8

	move.l	(a0),d3
	move.l	d3,d4
	move.b	-1(a0),d7
	roxr.b	#1,d7
	roxr.l	#1,d3
	move.b 	4(a0),d7
	roxl.b	#1,d7
	roxl.l	#1,d4
	eor.l	d4,d3
	or.l	d3,d4
	eor.l	d3,d4

* (d2,d1,d0) = neighbours 1+2+3+4+5+6+7+8

	eor.l	d3,d0
	or.l	d0,d3
	eor.l	d0,d3
	eor.l	d3,d1
	or.l	d1,d3
	eor.l	d1,d3
	eor.l	d4,d1
	or.l	d1,d4
	eor.l	d1,d4
	or.l	d3,d2
	or.l	d4,d2

* next generation

	or.l	(a0)+,d0
	not.l	d2
	and.l	d2,d0
	and.l	d1,d0
	move.l	d0,(a1)+

	cmp.l	a2,a0
	blt	again
	movem.l	(sp)+,d0-d7/a0-a2
	rts
