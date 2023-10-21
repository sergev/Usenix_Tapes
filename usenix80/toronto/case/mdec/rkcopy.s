/This program does disk to disk transfers for the RK11-D and RK11-E
/type moving head disk.  The program is assembled with tty.s and
/runs stand-alone using fsboot.s.

/Written by S.J.Leffler. 10/19/77.

	mov	$2000,sp
	mov	$trvect,r5
info:
	jsr	pc,mesg
	<\r\nTo halt copying at any time during input hit ^C\0>
	.even
gets:
	mov 	$trvect,r5
	jsr	pc,mesg
	<\r\n\0>
	.even
	jsr	pc,mesg
	<Source Drive Selection:\0>	/input source's disk drive
	.even
	jsr	pc,getc
	cmp	r0,$003			/halt program on @C
	jeq	halt
	sub	$60,r0			/convert ASCII to integer
	mov	r0,sdsa
	jsr	pc,mesg
	<\r\n\0>
	.even
getd:
	jsr	pc,mesg
	<Destination Drive Selection:\0>
	.even
	jsr	pc,getc
	cmp	r0,$003			/halt program on @C
	jeq	halt
	sub	$60,r0			/convert to integer
	mov	r0,ddsa
	jsr 	pc,mesg
	<\r\n\0>
	.even
verify:
	jsr	pc,mesg
	<Transfer disk \0>
	.even
	mov	sdsa,r0
	add	$60,r0			/convert integer back to ASCII
	mov	r0,1f			
	jsr	pc,mesg
1:	.byte 0,0			/reserve word for sdsa
	.even
	jsr	pc,mesg
	< to disk \0>
	.even
	mov	ddsa,r0
	add	$60,r0
	mov	r0,2f
	jsr	pc,mesg
2:	.byte 0,0			/reserve word for ddsa
	.even
	jsr	pc,mesg
	<\r\n\0>
	.even
	jsr	pc,mesg
	<  [Confirm]\0>
	.even
	jsr	pc,getc
	cmp	r0,$'\n
	bne	gets
	jsr	pc,mesg
	<\r\nTransfer Beginning\r\n\0>
	.even
transfer:
rkcs = 777404
rkwc = 777406
rkda = 777412
rker = 777402
rkbs = 777410
rkds = 777400
1:
	clr 	r1			/r1 counts the number of blocks of 60000 (octal) words  transferred
	clr	*$rkda
	clr	*$rkcs			/set status blank before loop
	mov	sdsa,r0			/initialize read pointer
	ash	$13.,r0
	mov	ddsa,r3
	ash	$13.,r3
	mov	r3,*$rkda		/initialize write pointer
2:
	cmp	r1,$50.
	beq	last
	inc	r1
	mov	$-60000,*$rkwc
	mov	$6000,*$rkbs		/set core buffer at starting address
	mov	*$rkda,r2		/save old write pointer
	mov	r0,*$rkda		/set read pointer
	mov	$5,*$rkcs		/load read and go
5:
	bit	$000200,*$rkcs		/check ready bit
	beq	5b
3:					/entry for normal write loop
	mov	*$rkda,r0		/save old read pointer
	mov	$-60000,*$rkwc
	mov	$6000,*$rkbs		/set core buffer at starting address
	mov	r2,*$rkda		/set write pointer
	bit	$40,*$rkds		/check if write protected disk
	beq	4f			/no write  protect - continue loop
	jsr	pc,mesg
	<Write Protect On Destination Disk!\r\nReset And cr\0>
	.even
	jsr	pc,getc
4:
	mov	$3.,*$rkcs		/load write and go
5:
	bit	$000200,*$rkcs		/check ready bit
	beq	5b
	jmp	2b
last:					/entry for final 72 sector to be transferred
	mov	$-18432.,*$rkwc
	mov	$6000,*$rkbs
	mov	*$rkda,r2
	mov	r0,*$rkda
	mov	$5,*$rkcs		/do last read
5:
	bit	$000200,*$rkcs
	beq	5b
	mov	$-18432.,*$rkwc
	mov	$6000,*$rkbs
	mov	r2,*$rkda
	mov	$3,*$rkcs		/do last write
5:
	bit	$000200,*$rkcs
	beq	5b
done:
	jsr	pc,mesg
	<Transfer Completed\r\n\0>
	.even
	jmp	gets			/loop to transfer more disks
halt:	jsr	pc,mesg
	<\r\ndcopy halting!!\r\n\0>
	.even
	.byte	0,0
sdsa:	.byte	0,0			/loc. of source drive select address
ddsa:	.byte	0,0			/loc. of destination drive select
