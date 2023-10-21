
;------------------------------------------------------------------------------
;	Talking console device driver for the AT&T PC6300.
;	Written by Karl Dahlke, September 1986.
;	Property of AT&T Bell Laboratories, all rights reserved.
;	This software is in the public domain and may be freely
;	distributed to anyone, provided this notice is included.
;	It may not, in whole or in part, be incorporated in any commercial
;	product without AT&T's explicit permission.
;------------------------------------------------------------------------------

;	parms.h: parameters for the talking console device driver

;	Several important real time functions are controlled by
;	CPU loops.  Therefore, loop limits are determined by the CPU clock rate.
CLKRATE equ 8 ; in cycles per microsecond

DELAY	equ CLKRATE*81 ; slow down output stream, so sounds carry information

FIFOLEN	equ 32 ; length of real time event fifo

WDFIXLEN	equ	2048 ; length of word correction table

WDLEN	equ 20 ; longest possible word

KBSIZ	equ 120 ; length  of type ahead keyboard buffer

ADDR	equ offset PGROUP

